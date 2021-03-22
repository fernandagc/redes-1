#include "commands.h"

#include <time.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <linux/if.h>
#include <sys/types.h>
#include <sys/ioctl.h>
#include <sys/socket.h>
#include <net/ethernet.h>
#include <linux/if_packet.h>


int main()
{
    time_t inicio, limite;
    short int timeOut, error = 0;
    char *Conteudo, *param1, *param2, *param3, *ParametroEdit;
    long int tam = 0;
    int local = 0, Socket = 0, Sequencia = 0, multSeq = 0, tamIni = 0, tamFim = 0;
                
    Mensagem enviada, recebida;

    Conteudo = initPort(TAM_MSG);
    param1 = initPort(TAM_MSG);
    param2 = initPort(TAM_MSG);
    param3 = initPort(TAM_MSG);
    
    ParametroEdit = initPort(1024);

    recebida.Inicio = 0;
    enviada.Inicio = 0;

    Socket = rawSocket();
    if (Socket == -1)
    {
        perror("Erro no socket.\n");
        exit(1);
    } else {
    	perror("Interface servidor conectada.");
	};

    setvbuf (stdout, 0, _IONBF, 0);
    

    while (1)
    {
        while ((((recebida.Inicio != INITMSG)) || ( recebida.Tipo != 0x0 && recebida.Tipo != 0x2 && recebida.Tipo != 0x4 && recebida.Tipo != 0x5 && recebida.Tipo != 0x6 && recebida.Tipo != 0x7 )  || ((int)recebida.Sequencia != Sequencia)))  
        {
            recebida = receiveMsg(Socket, recebida);
        }
        

        if (!checkParity(recebida) && timeOut == 0)
            sendNACK(Servidor, Cliente, Socket, Sequencia, recebida, enviada);                
            
        switch (recebida.Tipo)
        {
            case 0x0: //cd
                strcpy(param1,(char *)recebida.Dados);
                
                cd(&error, param1);
                if (error == 0)
                {
                    sendACK(Servidor, Cliente, Socket, Sequencia, ACK, recebida, enviada);
                    Sequencia++;
                }
                else
                {
                    sendERR(Servidor, Cliente, Socket, Sequencia, 0xF, error, enviada); 
                    break;
                }
                
                lcd(param1);
                break;        
                    
            case 0x2: //ls - list 
                ls(&error);

                if (error == 0)
                {
                    sendACK(Servidor, Cliente, Socket, Sequencia, 0xb, recebida, enviada);
                    Sequencia++;
                }
                else
                {
                    sendERR(Servidor, Cliente, Socket, Sequencia, 0xF, error, enviada); 
                    break;
                }
                recebida.Inicio = 0;
                enviada.Inicio = 0;

                time(&inicio);
                while ( (recebida.Inicio != INITMSG) || ( (recebida.Tipo != ACK && recebida.Tipo != NACK) || (int)recebida.Sequencia != Sequencia) )
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
                
                if (recebida.Tipo == 0x9)
                    recebida = ttNACK(Socket, Sequencia, recebida, enviada);
                
                if (recebida.Tipo == 0x8) // ls
                {
                    Conteudo = list(local, &tam);
                    enviada = newMsg(Servidor, Cliente, Conteudo, 0xb, Sequencia);
                    sendMsg(Socket, enviada);
                    Sequencia++;
                }
                local = 1;
                while (tam >= local)
                {
                    recebida.Inicio = 0;
                    enviada.Inicio = 0;

                    time(&inicio);
                    while ( (recebida.Inicio != INITMSG) || ( (recebida.Tipo != ACK && recebida.Tipo != NACK) || (int)recebida.Sequencia != Sequencia) )
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
                                    
                    if (recebida.Tipo == 0x9)
                        recebida = ttNACK(Socket, Sequencia, recebida, enviada);
                    
                    if(recebida.Tipo == ACK)
                    {
                        Conteudo = list(local, &tam);
                        enviada = newMsg(Servidor, Cliente, Conteudo, 0xb, Sequencia);
                        sendMsg(Socket, enviada);
                        Sequencia++;
                        if (Sequencia == 256)
                        {
                            Sequencia = 0;
                            multSeq++;
                        }
                        local ++;
                        if((int)enviada.Tamanho == 14)
                        {
                            tam++;
                        }
                    }
                }

                if (timeOut == 1)
                    break;        
                
                time(&inicio);
                
                while ( (recebida.Inicio != INITMSG) || ( (recebida.Tipo != ACK && recebida.Tipo != NACK) || (int)recebida.Sequencia != Sequencia) )
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
                
                if (recebida.Tipo == 0x9)
                    recebida = ttNACK(Socket, Sequencia, recebida, enviada);
                
                if (recebida.Tipo == 0x8)
                {   
                    sendACK(Servidor, Cliente, Socket, Sequencia, 0xd, recebida, enviada);
                }
        
            break;

            case 0x4:   // ver
                strcpy(param1, (char *)recebida.Dados);
                
                verArquivo(&error, param1);

                if (error == 0)
                {
                    sendACK(Servidor, Cliente, Socket, Sequencia, 0xc, recebida, enviada);
                    Sequencia++;
                }
                else
                {
                    sendERR(Servidor, Cliente, Socket, Sequencia, 0xF, error, enviada); 
                    break;
                }

                recebida.Inicio = 0;
                enviada.Inicio = 0;

                time(&inicio);
                while ( (recebida.Inicio != INITMSG) || ( (recebida.Tipo != ACK && recebida.Tipo != NACK) || (int)recebida.Sequencia != Sequencia) )
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
            
                if (recebida.Tipo == 0x9)
                    recebida = ttNACK(Socket, Sequencia, recebida, enviada);
                
                if (recebida.Tipo == 0x8)
                {
                    Conteudo = ver(param1, &tam, local);
                    
                    enviada = newMsg(Servidor, Cliente, Conteudo, 0xc, Sequencia);
                    
                    sendMsg(Socket, enviada);

                }    
                Sequencia++;             
                
                while(tam > local)
                {
                    recebida.Inicio = 0;
                    enviada.Inicio = 0;
                    
                    time(&inicio);
                    
                    while ( (recebida.Inicio != INITMSG) || ( (recebida.Tipo != ACK && recebida.Tipo != NACK) || (int)recebida.Sequencia != Sequencia) )
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

                    if (recebida.Tipo == 0x9)
                        recebida = ttNACK(Socket, Sequencia, recebida, enviada);
                    
                    if (recebida.Tipo == 0x8)
                    {
                        local += TAM_MSG;
                        Conteudo = ver(param1, &tam, local);
                        enviada = newMsg(Servidor, Cliente, Conteudo, 0xc, Sequencia);
                        sendMsg(Socket, enviada);
                        Sequencia++;
                        if (Sequencia == 256)
                        {
                            Sequencia = 0;
                            multSeq++;
                        }
                    }
                }
                
                
                if (timeOut == 1)
                    break;
                
                time(&inicio);
                
                while ( (recebida.Inicio != INITMSG) || ( (recebida.Tipo != ACK && recebida.Tipo != NACK) || (int)recebida.Sequencia != Sequencia) )
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
                
                if (recebida.Tipo == 0x9)
                    recebida = ttNACK(Socket, Sequencia, recebida, enviada);
                
                if (recebida.Tipo == 0x8)
                {    
                    sendACK(Servidor, Cliente, Socket, Sequencia, 0xd, recebida, enviada);
                }    
                break;

            case 0x5: //linha
                strcpy(param2, (char *)recebida.Dados);
                
                verArquivo(&error, param2);

                if (error == 0)
                {
                    sendACK(Servidor, Cliente, Socket, Sequencia, ACK, recebida, enviada);
                    Sequencia++;
                }
                else
                {
                    sendERR(Servidor, Cliente, Socket, Sequencia, 0xF, error, enviada);
                    break;
                }
                
                recebida.Inicio = 0;
                enviada.Inicio = 0;

                time(&inicio);
                while ( (recebida.Inicio != INITMSG) || ( (recebida.Tipo != 0xa) || (int)recebida.Sequencia != Sequencia) )
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
                  
                tamIni = 0;
                if (recebida.Tipo == 0xa)
                {
                    if (!checkParity(recebida))
                        sendNACK(Servidor, Cliente, Socket, Sequencia, recebida, enviada);

                    strcpy(param1, (char *)recebida.Dados);
                    tamIni = atoi(param1);

                    enviada = newMsg(Servidor, Cliente, Conteudo, 0xc, Sequencia);                
    
                    sendMsg(Socket, enviada);
                    Sequencia++;             
                }    
                
                recebida.Inicio = 0;
                enviada.Inicio = 0;

                time(&inicio);
                while ( (recebida.Inicio != INITMSG) || ( (recebida.Tipo != ACK && recebida.Tipo != NACK) || (int)recebida.Sequencia != Sequencia) )
                {    
                recebida = receiveMsg(Socket, recebida);
                time(&limite);
                    if (difftime(limite, inicio) > 1)
                    {
                        timeOut = 1;
                        break;
                    }
                }

                if (recebida.Tipo == 0x9)
                    recebida = ttNACK(Socket, Sequencia, recebida, enviada);
                
                if (recebida.Tipo == 0x8)
                {
                    Conteudo = linha(tamIni, param2, &tam, local);
                    enviada = newMsg(Servidor, Cliente, Conteudo, 0xc, Sequencia);
                 
                    sendMsg(Socket, enviada);
                    Sequencia++;             
                }    
                
                while(tam - TAM_MSG > local)
                {
                    recebida.Inicio = 0;
                    enviada.Inicio = 0;
                    
                    time(&inicio);
                    while ( (recebida.Inicio != INITMSG) || ( (recebida.Tipo != ACK && recebida.Tipo != NACK) || (int)recebida.Sequencia != Sequencia) )
                    {
                        recebida = receiveMsg(Socket, recebida);
                        time(&limite);
                        if (difftime(limite, inicio) > 1)
                        {
                            timeOut = 1;
                            break;
                        }
                    }   
                    if (recebida.Tipo == 0x9)
                        recebida = ttNACK(Socket, Sequencia, recebida, enviada);
                    
                    if (recebida.Tipo == 0x8)
                    {
                        local += 14;
                        Conteudo = linha(tamIni, param2, &tam, local);
                        enviada = newMsg(Servidor, Cliente, Conteudo, 0xc, Sequencia);

                        sendMsg(Socket, enviada);
                        Sequencia++;
                        if (Sequencia == 256)
                        {
                            Sequencia = 0;
                            multSeq++;
                        }
                    }
                }
                
                time(&inicio);
                while ( (recebida.Inicio != INITMSG) || ( (recebida.Tipo != ACK && recebida.Tipo != NACK) || (int)recebida.Sequencia != Sequencia) )
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
                
                if (recebida.Tipo == 0x9)
                    recebida = ttNACK(Socket, Sequencia, recebida, enviada);
                
                if (recebida.Tipo == 0x8)
                {    
                    sendACK(Servidor, Cliente, Socket, Sequencia, 0xd, recebida, enviada);
                }    
                break;

            case 0x6: //linhas
                strcpy(param3, (char *)recebida.Dados);
                
                verArquivo(&error, param3);

                if (error == 0)
                {
                    sendACK(Servidor, Cliente, Socket, Sequencia, ACK, recebida, enviada);
                    Sequencia++;
                }
                else
                {
                    sendERR(Servidor, Cliente, Socket, Sequencia, 0xF, error, enviada); 
                    break;
                }
                
                recebida.Inicio = 0;
                enviada.Inicio = 0;

                time(&inicio);
                while ( (recebida.Inicio != INITMSG) || ( (recebida.Tipo != 0xa) || (int)recebida.Sequencia != Sequencia) )
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

                tamIni = 0;
                tamFim = 0;
                
                if (recebida.Tipo == 0xa)
                {
                    if (!checkParity(recebida))
                        sendNACK(Servidor, Cliente, Socket, Sequencia, recebida, enviada);

                    short int flag = 0;
                    int k = 0;
                    for (int i = 0; i < (int)recebida.Tamanho; i++)
                    {
                        if (recebida.Dados[i] == '\n')
                        {
                            flag = 1;
                            continue;
                        }
                        if (flag == 0)
                        {
                            param1[i] = recebida.Dados[i]; 
                        }
                        else
                        {
                            param2[k] = recebida.Dados[i];
                            k++;
                        }
                        
                    }
                    tamIni = atoi(param1);
                    tamFim = atoi(param2);
                    enviada = newMsg(Servidor, Cliente, Conteudo, 0xc, Sequencia);           
                    sendMsg(Socket, enviada);
                    Sequencia++;             
                }    
                
                while (tamIni <= tamFim)
                {
                
                    recebida.Inicio = 0;
                    enviada.Inicio = 0;

                    time(&inicio);
                    while ( (recebida.Inicio != INITMSG) || ( (recebida.Tipo != ACK && recebida.Tipo != NACK) || (int)recebida.Sequencia != Sequencia) )
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

                    if (recebida.Tipo == 0x9)
                        recebida = ttNACK(Socket, Sequencia, recebida, enviada);
                    
                    if (recebida.Tipo == 0x8)
                    {
                        Conteudo = linha(tamIni, param3, &tam, local);
                        enviada = newMsg(Servidor, Cliente, Conteudo, 0xc, Sequencia);
                        sendMsg(Socket, enviada);
                        Sequencia++;
                        if (Sequencia == 256)
                        {
                            Sequencia = 0;
                            multSeq++;
                        }             
                    }    
                    
                    while(tam - TAM_MSG > local)
                    {
                        recebida.Inicio = 0;
                        enviada.Inicio = 0;
                        time(&inicio);
                        while ( (recebida.Inicio != INITMSG) || ( (recebida.Tipo != ACK && recebida.Tipo != NACK) || (int)recebida.Sequencia != Sequencia) )
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
                        
                        if (recebida.Tipo == 0x9)
                            recebida = ttNACK(Socket, Sequencia, recebida, enviada);
                        
                        if (recebida.Tipo == 0x8)
                        {
                            local += 14;
                            Conteudo = linha(tamIni, param3, &tam, local);                        
                            enviada = newMsg(Servidor, Cliente, Conteudo, 0xc, Sequencia);
                            sendMsg(Socket, enviada);
                            Sequencia++;
                            if (Sequencia == 256)
                            {
                                Sequencia = 0;
                                multSeq++;
                            }
                        }
                    }
                        local = 0;
                        tamIni++;
                }
                if (timeOut == 1)
                    break;
                time(&inicio);
                while ( (recebida.Inicio != INITMSG) || ( (recebida.Tipo != ACK && recebida.Tipo != NACK) || (int)recebida.Sequencia != Sequencia) )
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
                
                if (recebida.Tipo == 0x9)
                    recebida = ttNACK(Socket, Sequencia, recebida, enviada);
                
                if (recebida.Tipo == 0x8)
                {    
                    sendACK(Servidor, Cliente, Socket, Sequencia, 0xd, recebida, enviada);
                }
                break; 

            case 0x7:  //edit
                tamIni = 0;
                int tamConteudo = 0;
                local = 0;            
                int i;
                strcpy(param2, (char *)recebida.Dados);
                verArquivo(&error, param2);
                if (error == 0)
                {
                    sendACK(Servidor, Cliente, Socket, Sequencia, ACK, recebida, enviada);
                    Sequencia++;
                }
                else
                {
                    sendERR(Servidor, Cliente, Socket, Sequencia, 0xF, error, enviada); 
                    break;
                }
                
                recebida.Inicio = 0;
                enviada.Inicio = 0;

                time(&inicio);
                while ( (recebida.Inicio != INITMSG) || ( (recebida.Tipo != 0xa) || (int)recebida.Sequencia != Sequencia) )
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

                if (recebida.Tipo == 0xa)
                {
                    if (!checkParity(recebida))
                        sendNACK(Servidor, Cliente, Socket, Sequencia, recebida, enviada);

                    strcpy(param1, (char *)recebida.Dados);
                    
                    showLine(&error, param1, param2);
                    
                    if (error == 0)
                    {  
                        tamIni = atoi(param1);
                        enviada = newMsg(Servidor, Cliente, Conteudo, ACK, Sequencia);                
                        sendMsg(Socket, enviada);
                        Sequencia++;             
                    }
                    else
                    {
                        sendERR(Servidor, Cliente, Socket, Sequencia, 0xF, error, enviada);
                        break;
                    }
                }
                
                while(recebida.Tipo != 0xd)
                {
                    recebida.Inicio = 0;
                    enviada.Inicio = 0;
                    time(&inicio);
                    while ( (recebida.Inicio != INITMSG) || ( (recebida.Tipo != 0xc && recebida.Tipo != 0xd && recebida.Tipo != NACK) ||(int) recebida.Sequencia != Sequencia))
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
                    
                    if(recebida.Tipo == NACK)
                    {
                        recebida = ttNACK(Socket, Sequencia, recebida, enviada);
                    }
                    if (recebida.Tipo == 0xc)
                    {
                        if (!checkParity(recebida))
                            sendNACK(Servidor, Cliente, Socket, Sequencia, recebida, enviada);
                        sendACK(Servidor, Cliente, Socket, Sequencia, 0x8, recebida, enviada);
                    }
                    
                    Sequencia++;
                    if (Sequencia == 256)
                    {
                        Sequencia = 0;
                        multSeq++;
                    }    
                    for (i = 0; i < (int)recebida.Tamanho; i++)
                    {
                        ParametroEdit[i+local] = recebida.Dados[i];
                    }
                
                    tamConteudo+=i;
                    local += TAM_MSG;
                } 

                
                if (timeOut == 1)
                    break;

                edit(tamIni, param2, ParametroEdit, tamConteudo);
                printf("Edit efetuado.\n");
                break;              
            
                default:
                printf("Comando inexistente.\n");
                break;
        }
        
        strcpy(Conteudo, "");
        strcpy(param1, "");
        strcpy(param2, "");
        strcpy(param3, "");
        strcpy(ParametroEdit, "");
        
        tam = 0;
        local = 0;
        error = 0;
        multSeq = 0;
        timeOut = 0;
        Sequencia = 0;
        recebida.Inicio = 0;
        enviada.Inicio = 0;
    }
    
    return 0;
}

