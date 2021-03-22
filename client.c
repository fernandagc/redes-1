#include "commands.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <signal.h>
#include <time.h>
#include <linux/if.h>
#include <sys/types.h>
#include <sys/ioctl.h>
#include <sys/socket.h>
#include <net/ethernet.h>
#include <linux/if_packet.h>

int main()
{
    Mensagem enviada, recebida;
    enviada.Inicio = 0;
    recebida.Inicio = 0;
    char *param1, *param2, *param3;
    char *CMDCLIENT, *cmd_, *dadosEnviados;
    int Tipo;
	int Sequencia = 0, multSeq = 0, local = 0, tam = 0;
    long int sizeParam;
    time_t inicio, limite;
    short int timeOut, error;
    setvbuf (stdout, 0, _IONBF, 0);
    
	int Socket;
	Socket = rawSocket();
    if (Socket == -1)
    {
        perror("Erro no socket.\n");
        exit(1);
    } else {
    	perror("Interface cliente conectada.");
	};

    CMDCLIENT = initPort(0); 
    cmd_ = initPort(0);

    dadosEnviados = initPort(TAM_MSG);

    param1 = initPort(0);
    param2 = initPort(0);
    param3 = initPort(0);
    
    while (1)
    {
    	printf("Digite o comando desejado: \n");
        fgets(CMDCLIENT, BUFFER, stdin);          
        strcut(CMDCLIENT, cmd_, " ");
        strcut(CMDCLIENT, param1, " ");
        strcut(CMDCLIENT, param2, " ");

        if (strcmp(cmd_, "edit"))
            strcut(CMDCLIENT, param3, " ");
        else {
            strcut(CMDCLIENT, param3, "\"");                                            
            strcut(CMDCLIENT, param3, "\"");                        
        }
        
        Tipo = cmd(cmd_, dadosEnviados, param1, param2, param3);
        
        if (strlen(dadosEnviados) > TAM_MSG){
            printf("Parametro passado possui mais do que TAM_MSG bytes\n");
            continue;
        } else {
            enviada = newMsg(Cliente, Servidor, dadosEnviados, Tipo, 0x0);
            if(enviada.Tipo != 0x1 && enviada.Tipo != 0x3)
                sendMsg(Socket, enviada);
            switch (enviada.Tipo) {
                case 0x0: //cd   
					enviada.Inicio = 0;  
                    recebida.Inicio = 0;
                    time(&inicio);
                    while (((recebida.Tipo != ACK && recebida.Tipo != NACK && recebida.Tipo != 0xF) || (int)recebida.Sequencia != Sequencia) || (recebida.Inicio != INITMSG)) {
                        recebida = receiveMsg(Socket, recebida);
                        time(&limite);
                        if (difftime(limite, inicio) > 1)                        {
                            timeOut = 1;
                            break;
                        }
                    } 
                    if (timeOut == 1)
                        break;
                    if (recebida.Tipo == NACK)
                        recebida = ttNACK(Socket, Sequencia, recebida, enviada);
                    if(recebida.Tipo == 0xF) {  
                        if (recebida.Dados[0] == '1')
                            printf("Permissao negada");
                        if (recebida.Dados[0] == '2')
                            printf("Diretorio nao existe");
                        error = 1;
                    }
                    if (error == 1) {    
                        printf("\n");
                        break;
                    }
                    break;

                case 0x2://ls
                    enviada.Inicio = 0;
					recebida.Inicio = 0;
                    time(&inicio);
                    while ( (recebida.Inicio != INITMSG) || ( (recebida.Tipo != 0xb && recebida.Tipo != NACK && recebida.Tipo != 0xF) || (int)recebida.Sequencia != Sequencia) )
                    {
                        recebida = receiveMsg(Socket, recebida);
                        time(&limite);
                        if (difftime(limite, inicio) > 1)
                        {
                            timeOut = 1;
                            break;
                        }
                    } 
                    if (timeOut == 1)
                        break;
                    if(recebida.Tipo == 0x9) {
                        recebida = ttNACK(Socket, Sequencia, recebida, enviada);
                    }
                    if(recebida.Tipo == 0xb) {
                        if(!checkParity(recebida)) {
                            sendNACK(Cliente, Servidor, Socket, Sequencia, recebida, enviada);
                        }
                        Sequencia++;
                        sendACK(Cliente, Servidor, Socket, Sequencia, 0x8, recebida, enviada);
                    }
                    if(recebida.Tipo == 0xF) {  
                        if (recebida.Dados[0] == '1')
                            printf("Sem permissao");
                        error = 1;
                    }
                    if (error == 1) {    
                        printf("\n");
                        break;
                    }
                    while(recebida.Tipo != 0xd) {
                        recebida.Inicio = 0;
                        enviada.Inicio = 0;
                        time(&inicio);
                        while ( (recebida.Inicio != INITMSG) || ( (recebida.Tipo != 0xb && recebida.Tipo != 0xd && recebida.Tipo != NACK) ||(int) recebida.Sequencia != Sequencia)) {
                            recebida = receiveMsg(Socket, recebida);
                            time(&limite);
                            if (difftime(limite, inicio) > 1) {
                                timeOut = 1;
                                break;
                            }
                        }    
                        if (timeOut == 1)
                            break;
                        Sequencia++;
                        if (Sequencia == 256) {
                            Sequencia = 0;
                            multSeq++;
                        }
                        if(recebida.Tipo == NACK) {
                            recebida = ttNACK(Socket, Sequencia, recebida, enviada);
                        }
                        if (recebida.Tipo == 0xb) {
                            if (!checkParity(recebida))
                                sendNACK(Cliente, Servidor, Socket, Sequencia, recebida, enviada);
                            sendACK(Cliente, Servidor, Socket, Sequencia, 0x8, recebida, enviada);
                        }
                        if (recebida.Sequencia == 1)
                            continue;
                        
                        for (int i = 0; i < (int)recebida.Tamanho; i++) {
                            if (recebida.Dados[i] != '\n')
                                printf("%c",recebida.Dados[i]);
                            else
                                printf(" ");
                        }
                    }
                    printf("\n");

                    break;

                case 0x4: //ver
                	enviada.Inicio = 0;
                    recebida.Inicio = 0;
                    time(&inicio);
                    while (( (recebida.Tipo != 0xc && recebida.Tipo != NACK && recebida.Tipo != 0xF) || (int)recebida.Sequencia != Sequencia)  || (recebida.Inicio != INITMSG)) {
                        recebida = receiveMsg(Socket, recebida);
                        time(&limite);
                        if (difftime(limite, inicio) > 1)
                        {
                            timeOut = 1;
                            break;
                        }
                    } 
                    if (timeOut == 1)
                        break;

                    if(recebida.Tipo == 0x9) {
                        recebida = ttNACK(Socket, Sequencia, recebida, enviada);
                    }
                    
                    if(recebida.Tipo == 0xc)
                    {
                        if(!checkParity(recebida))
                        {
                            sendNACK(Cliente, Servidor, Socket, Sequencia, recebida, enviada);
                        }
                        Sequencia++;
                        sendACK(Cliente, Servidor, Socket, Sequencia, 0x8, recebida, enviada);
                    }
                    
                    if(recebida.Tipo == 0xF)  {  
                        if (recebida.Dados[0] == '3')
                            printf("Arquivo inexistente");
                        error = 1;
                    }

                    if (error == 1) {    
                        printf("\n");
                        break;
                    }
                    
                    while(recebida.Tipo != 0xd) {
                        recebida.Inicio = 0;
                        enviada.Inicio = 0;
                        time(&inicio);
                        while ( (recebida.Inicio != INITMSG) || ( (recebida.Tipo != 0xc && recebida.Tipo != 0xd && recebida.Tipo != NACK) ||(int) recebida.Sequencia != Sequencia)){
                            recebida = receiveMsg(Socket, recebida);
                            time(&limite);
                            if (difftime(limite, inicio) > 1)
                            {
                                timeOut = 1;
                                break;
                            }
                        } 
                        
                        if (timeOut == 1)
                            break;
                        
                        Sequencia++;
                        if (Sequencia == 256)
                        {
                            Sequencia = 0;
                            multSeq++;
                        }
                        if(recebida.Tipo == NACK)
                        {
                            recebida = ttNACK(Socket, Sequencia, recebida, enviada);
                        }
                        if (recebida.Tipo == 0xc)
                        {
                            if (!checkParity(recebida))
                                sendNACK(Cliente, Servidor, Socket, Sequencia, recebida, enviada);
                            sendACK(Cliente, Servidor, Socket, Sequencia, 0x8, recebida, enviada);
                        }
                        
                        for (int i = 0; i < (int)recebida.Tamanho; i++)
                            printf("%c",recebida.Dados[i]);
                    }
                    printf("\n");
                    break;

                case 0x5: //linha
                    recebida.Inicio = 0;
                    enviada.Inicio = 0;
                    
                    time(&inicio);
                    while ( (recebida.Inicio != INITMSG) || ( (recebida.Tipo != ACK && recebida.Tipo != NACK && recebida.Tipo != 0xF) || (int)recebida.Sequencia != Sequencia) )
                    {
                        recebida = receiveMsg(Socket, recebida);
                        time(&limite);
                        if (difftime(limite, inicio) > 1)
                        {
                            timeOut = 1;
                            break;
                        }
                    } 
        
                    if (timeOut == 1)
                        break;

                    
                    if(recebida.Tipo == 0x9)
                    {
                        recebida = ttNACK(Socket, Sequencia, recebida, enviada);
                    }
                    
                    if(recebida.Tipo == ACK)
                    {
                        Sequencia++;
                        enviada = newMsg(Cliente, Servidor, param1, 0xa, Sequencia);
                        sendMsg(Socket, enviada);
                    }
                    if(recebida.Tipo == 0xF)
                    {  
                        if (recebida.Dados[0] == '3')
                            printf("Arquivo inexistente");
                        error = 1;
                    }

                    if (error == 1)
                    {    
                        printf("\n");
                        break;
                    }
                    
                    time(&inicio);
                    while ( (recebida.Inicio != INITMSG) || ( (recebida.Tipo != 0xc && recebida.Tipo != NACK) || (int)recebida.Sequencia != Sequencia) )
                    {
                        recebida = receiveMsg(Socket, recebida);
                        time(&limite);
                        if (difftime(limite, inicio) > 1)
                        {
                            timeOut = 1;
                            break;
                        }
                    } 
                    if (timeOut == 1)
                            break;
                    
                    if(recebida.Tipo == 0x9){
                        recebida = ttNACK(Socket, Sequencia, recebida, enviada);
                    }
                    
                    if(recebida.Tipo == 0xc){
                        if(!checkParity(recebida)){
                            sendNACK(Cliente, Servidor, Socket, Sequencia, recebida, enviada);
                        }
                        Sequencia++;
                        sendACK(Cliente, Servidor, Socket, Sequencia, 0x8, recebida, enviada);
                    }
                    

                    while(recebida.Tipo != 0xd){
                        recebida.Inicio = 0;
                        enviada.Inicio = 0;

                        time(&inicio);
                        while ( (recebida.Inicio != INITMSG) || ( (recebida.Tipo != 0xc && recebida.Tipo != 0xd && recebida.Tipo != NACK) ||(int) recebida.Sequencia != Sequencia)) {
                            recebida = receiveMsg(Socket, recebida);
                            time(&limite);
                            if (difftime(limite, inicio) > 1)
                            {   
                                timeOut = 1;
                                break;
                            }
                        } 
                        
                        if (timeOut == 1)
                            break;
                        
                        Sequencia++;
                        if (Sequencia == 256){
                            Sequencia = 0;
                            multSeq++;
                        }
                        if(recebida.Tipo == NACK){
                            recebida = ttNACK(Socket, Sequencia, recebida, enviada);
                        }
                        if (recebida.Tipo == 0xc){
                            if (!checkParity(recebida))
                                sendNACK(Cliente, Servidor, Socket, Sequencia, recebida, enviada);
                            sendACK(Cliente, Servidor, Socket, Sequencia, 0x8, recebida, enviada);
                        }
                        for (int i = 0; i < (int)recebida.Tamanho; i++)
                            printf("%c",recebida.Dados[i]);
                    }
                    printf("\n");   
                    break;

                case 0x6: //linhas
                    recebida.Inicio = 0;
                    enviada.Inicio = 0;
                    
                    time(&inicio);
                    while (((recebida.Tipo != ACK && recebida.Tipo != NACK && recebida.Tipo != 0xF) || (int)recebida.Sequencia != Sequencia) || (recebida.Inicio != INITMSG))
                    {
                        recebida = receiveMsg(Socket, recebida);
                        time(&limite);
                        if (difftime(limite, inicio) > 1){
                            timeOut = 1;
                            break;
                        }
                    } 
        
                    if (timeOut == 1)
                        break;

                    if(recebida.Tipo == 0x9) {
                        recebida = ttNACK(Socket, Sequencia, recebida, enviada);
                    }
                    
                    if(recebida.Tipo == ACK) {
                        Sequencia++;
                        strcat(param1, "\n");  
                        strcat(param1, param2);
                        enviada = newMsg(Cliente, Servidor, param1, 0xa, Sequencia);
                        sendMsg(Socket, enviada);
                    }
                    if(recebida.Tipo == 0xF) {  
                        if (recebida.Dados[0] == '3')
                            printf("Arquivo inexistente");
                        error = 1;
                    }

                    if (error == 1){    
                        printf("\n");
                        break;
                    }

                    time(&inicio);
                    while (((recebida.Tipo != 0xc && recebida.Tipo != NACK) || (int)recebida.Sequencia != Sequencia) || (recebida.Inicio != INITMSG) ){
                        recebida = receiveMsg(Socket, recebida);
                        time(&limite);
                        if (difftime(limite, inicio) > 1)
                        {
                            timeOut = 1;
                            break;
                        }
                    } 
                    if (timeOut == 1)
                        break;
                    if(recebida.Tipo == 0x9){
                        recebida = ttNACK(Socket, Sequencia, recebida, enviada);
                    }
                    
                    if(recebida.Tipo == 0xc) {
                        if(!checkParity(recebida))
                        {
                            sendNACK(Cliente, Servidor, Socket, Sequencia, recebida, enviada);
                        }

                        Sequencia++;

                        sendACK(Cliente, Servidor, Socket, Sequencia, 0x8, recebida, enviada);
                    }
    
                    while(recebida.Tipo != 0xd) {
                        recebida.Inicio = 0;
                        enviada.Inicio = 0;

                        time(&inicio);
                        while (((recebida.Tipo != 0xc && recebida.Tipo != 0xd && recebida.Tipo != NACK) ||(int) recebida.Sequencia != Sequencia) || (recebida.Inicio != INITMSG))   {
                            recebida = receiveMsg(Socket, recebida);
                            time(&limite);
                            if (difftime(limite, inicio) > 1)  {
                                timeOut = 1;
                                break;
                            }
                        } 
                        
                        if (timeOut == 1)
                            break;
                        
                        Sequencia++;
                        if (Sequencia == 256) {
                            Sequencia = 0;
                            multSeq++;
                        }
                        if(recebida.Tipo == NACK) {
                            recebida = ttNACK(Socket, Sequencia, recebida, enviada);
                        }
                        if (recebida.Tipo == 0xc) {
                            if (!checkParity(recebida))
                                sendNACK(Cliente, Servidor, Socket, Sequencia, recebida, enviada);
                            sendACK(Cliente, Servidor, Socket, Sequencia, 0x8, recebida, enviada);
                        }
                        
                        for (int i = 0; i < (int)recebida.Tamanho; i++)
                            printf("%c",recebida.Dados[i]);
                    }
                    printf("\n");   

                    break;

                case 0x7: //edit
                    
                    recebida.Inicio = 0;
                    enviada.Inicio = 0;
                    
                    time(&inicio);
                    while (((recebida.Tipo != ACK && recebida.Tipo != NACK && recebida.Tipo != 0xF) || (int)recebida.Sequencia != Sequencia) || (recebida.Inicio != INITMSG))
                    {
                        recebida = receiveMsg(Socket, recebida);
                        time(&limite);
                        if (difftime(limite, inicio) > 1) {
                            timeOut = 1;
                            break;
                        }
                    } 
        
                    if (timeOut == 1)
                        break;
                    
                    
                    if(recebida.Tipo == 0x9) {
                        recebida = ttNACK(Socket, Sequencia, recebida, enviada);
                    }
                    
                    if(recebida.Tipo == ACK) {
                        Sequencia++;
                        enviada = newMsg(Cliente, Servidor, param1, 0xa, Sequencia);
                        sendMsg(Socket, enviada);
                    }

                    if(recebida.Tipo == 0xF) {  
                        if (recebida.Dados[0] == '3')
                            printf("Arquivo inexistente");
                        error = 1;
                    }

                    if (error == 1) {    
                        printf("\n");
                        break;
                    }
                    
                    time(&inicio);
                    while (((recebida.Tipo != ACK && recebida.Tipo != NACK && recebida.Tipo != 0xF) || (int)recebida.Sequencia != Sequencia) || (recebida.Inicio != INITMSG))
                    {
                        recebida = receiveMsg(Socket, recebida);
                        time(&limite);
                        if (difftime(limite, inicio) > 1) {
                            timeOut = 1;
                            break;
                        }
                    } 
        
                    if (timeOut == 1)
                        break;


                    if(recebida.Tipo == 0xF) {  
                        if (recebida.Dados[0] == '4')
                            printf("Linha inexistente");
                        error = 1;
                    }

                    if (error == 1) {    
                        printf("\n");
                        break;
                    }
 
                    if(recebida.Tipo == NACK) {
                        recebida = ttNACK(Socket, Sequencia, recebida, enviada);
                    }
                    
                    if(recebida.Tipo == ACK) {
                        if(!checkParity(recebida)) {
                            sendNACK(Cliente, Servidor, Socket, Sequencia, recebida, enviada);
                        }
                        Sequencia++;
                        sendACK(Cliente, Servidor, Socket, Sequencia, 0xc, recebida, enviada); 
                    }
                    
                    tam = strlen(param3);
                    local = 0;

                    while(tam > local) {
                        recebida.Inicio = 0;
                        enviada.Inicio = 0;

                        time(&inicio);
                        while (((recebida.Tipo != ACK && recebida.Tipo != NACK) || (int)recebida.Sequencia != Sequencia) || (recebida.Inicio != INITMSG))  {
                            recebida = receiveMsg(Socket, recebida);
                            time(&limite);
                            if (difftime(limite, inicio) > 1)  {
                                timeOut = 1;
                                break;
                            }
                        }   
                        
                        if (timeOut == 1)
                            break;
                        
                        if (recebida.Tipo == 0x9)
                            recebida = ttNACK(Socket, Sequencia, recebida, enviada);
                        
                        if (recebida.Tipo == 0x8) {
                            strbcut(param3, dadosEnviados, local, tam);
                            Sequencia++;
                            if (Sequencia == 256) {
                                Sequencia = 0;
                                multSeq++;
                            }
                            enviada = newMsg(Cliente, Servidor, dadosEnviados, 0xc, Sequencia);
                            sendMsg(Socket, enviada);
                            
                            local += TAM_MSG;
                        } 
                        if (timeOut == 1)
                            break;
                    }
                    
                    if (timeOut == 1)
                        break;

                    time(&inicio);
                    while (((recebida.Tipo != ACK && recebida.Tipo != NACK) || (int)recebida.Sequencia != Sequencia) || (recebida.Inicio != INITMSG))  {
                        recebida = receiveMsg(Socket, recebida);
                        time(&limite);
                        if (difftime(limite, inicio) > 1) {
                            timeOut = 1;
                            break;
                        }
                    }
                    if (timeOut == 1)
                        break;

                    if (recebida.Tipo == 0x9)
                        recebida = ttNACK(Socket, Sequencia, recebida, enviada);
                    
                    if (recebida.Tipo == 0x8) {    
                        Sequencia++;
                        if (Sequencia == 256) {
                            Sequencia = 0;
                            multSeq++;
                        }
                        sendACK(Cliente, Servidor, Socket, Sequencia, 0xd, recebida, enviada);
                    }
                    break;
                           
                case 0x1: //lcd
                    lcd(param1);
                    printf("\n");
                    break;

                case 0x3: //lls
                    local = 0;
                    dadosEnviados = lls(local, &sizeParam);
                    while (sizeParam - 1 > local)  {
                        dadosEnviados = lls(local, &sizeParam);
                        for (int i = 0; i < strlen(dadosEnviados); i++) {
                            if (dadosEnviados[i] != '\n')
                                printf("%c", dadosEnviados[i]);
                            else
                                printf(" ");
                        }
                        local++;
                    }
                    printf("\n");
                    break;
                default:
                    printf("Comando inexistente\n");
                    break;
            }
        }    

        tam = 0;
        local = 0;
        multSeq = 0;
        Sequencia = 0;
        sizeParam = 0;
        error = 0;
        enviada.Inicio = 0;
        recebida.Inicio = 0;
        if (timeOut == 1)
            printf("TIMEOUT!\n");
        timeOut = 0;  
        strcpy(CMDCLIENT, "");
        strcpy(cmd_, "");
        strcpy(param1, "");
        strcpy(param2, "");        
        strcpy(param3, "");
        strcpy(dadosEnviados, "");
    }
    return 0;
}

