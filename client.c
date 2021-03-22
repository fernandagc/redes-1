#include <stdio.h>
#include "operations.h"

#include <time.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <signal.h>
#include <linux/if.h>
#include <sys/types.h>
#include <sys/ioctl.h>
#include <sys/socket.h>
#include <net/ethernet.h>
#include <linux/if_packet.h>

int main()
{
    time_t inicio, fim;
    short int timeOut, error;
    char *CMDUSER, *Comando, *Parametro1, *Parametro2, *Parametro3, *DadosEnviado;
    int Tipo, Sequencia = 0, multSeq = 0, local = 0, tam = 0;
    long int tamanhoLS;
    int Socket;
    Mensagem mEnviado, mRecebido;
    
    mEnviado.Inicio = 0;
    mRecebido.Inicio = 0;

    setvbuf (stdout, 0, _IONBF, 0);
    
    Socket = rawSocket("lo");
    if (Socket == -1)
    {
        perror("Erro no socket.\n");
        exit(1);
    } else {
    	perror("Interface cliente conectada.");
	};

    CMDUSER = initPort(0); 
    Comando = initPort(0);
    Parametro1 = initPort(0);
    Parametro2 = initPort(0);
    Parametro3 = initPort(0);
    DadosEnviado = initPort(15);

    while (1)
    {
        fgets(CMDUSER, BUFFER, stdin);          
        
        strcut(CMDUSER, Comando, " ");
        strcut(CMDUSER, Parametro1, " ");
        strcut(CMDUSER, Parametro2, " ");

        if (strcmp(Comando, "edit"))
            strcut(CMDUSER, Parametro3, " ");
        else
        {
            strcut(CMDUSER, Parametro3, "\"");                                            
            strcut(CMDUSER, Parametro3, "\"");                        
        }
        
        Tipo = defineTipo(Comando, DadosEnviado, Parametro1, Parametro2, Parametro3);
        

        if (strlen(DadosEnviado) > 15)
        {
            printf("Erro, parametro passado tem mais de 15 bytes\n");
            continue;
        }
        else 
        {
            mEnviado = newMsg(Cliente, Servidor, DadosEnviado, Tipo, 0x0);
            if(mEnviado.Tipo != 0x1 && mEnviado.Tipo != 0x3)
                sendMsg(Socket, mEnviado);
            
            switch (mEnviado.Tipo)
            {
//--------------CD----------------------------------------------------------------------------------------------------------------
                case 0x0:
                    
                    mRecebido.Inicio = 0;
                    mEnviado.Inicio = 0;
                    
                    time(&inicio);
                    while ((mRecebido.Inicio != INITMSG) || ( (mRecebido.Tipo != ACK && mRecebido.Tipo != NACK && mRecebido.Tipo != 0xF) || (int)mRecebido.Sequencia != Sequencia))
                    {
                        mRecebido = receiveMsg(Socket, mRecebido);
                        time(&fim);
                        if (difftime(fim, inicio) > 1)
                        {
                            timeOut = 1;
                            break;
                        }
                    } 
                    
                    if (timeOut == 1)
                        break;

                    if (mRecebido.Tipo == NACK)
                        mRecebido = trataNACK(Socket, Sequencia, mRecebido, mEnviado);
                    
                    if(mRecebido.Tipo == 0xF)
                    {  
                        if (mRecebido.Dados[0] == '1')
                            printf("Erro permissao negada");
                        
                        if (mRecebido.Dados[0] == '2')
                            printf("Erro diretorio nao existe");
                        
                        error = 1;
                    }

                    if (error == 1)
                    {    
                        printf("\n");
                        break;
                    }
                    
                    break;

//----------------LIST---------------------------------------------------------------------------------------------------------------------------- 
                case 0x2:
                    
                    mRecebido.Inicio = 0;
                    mEnviado.Inicio = 0;
                    
                    time(&inicio);
                    while ( (mRecebido.Inicio != INITMSG) || ( (mRecebido.Tipo != 0xb && mRecebido.Tipo != NACK && mRecebido.Tipo != 0xF) || (int)mRecebido.Sequencia != Sequencia) )
                    {
                        mRecebido = receiveMsg(Socket, mRecebido);
                        time(&fim);
                        if (difftime(fim, inicio) > 1)
                        {
                            timeOut = 1;
                            break;
                        }
                    } 

                    if (timeOut == 1)
                        break;

                    //caso nack
                    if(mRecebido.Tipo == 0x9)
                    {
                        mRecebido = trataNACK(Socket, Sequencia, mRecebido, mEnviado);
                    }
                    
                    if(mRecebido.Tipo == 0xb)
                    {
                        if(!checkParity(mRecebido))
                        {
                            enviaNACK(Cliente, Servidor, Socket, Sequencia, mRecebido, mEnviado);
                        }
                        Sequencia++;
                        enviaACK(Cliente, Servidor, Socket, Sequencia, 0x8, mRecebido, mEnviado);
                    }
                    
                    if(mRecebido.Tipo == 0xF)
                    {  
                        if (mRecebido.Dados[0] == '1')
                            printf("Acesso proibido/sem permissao");
                        error = 1;
                    }

                    if (error == 1)
                    {    
                        printf("\n");
                        break;
                    }

                    while(mRecebido.Tipo != 0xd)
                    {
                        mRecebido.Inicio = 0;
                        mEnviado.Inicio = 0;
                        time(&inicio);
                        while ( (mRecebido.Inicio != INITMSG) || ( (mRecebido.Tipo != 0xb && mRecebido.Tipo != 0xd && mRecebido.Tipo != NACK) ||(int) mRecebido.Sequencia != Sequencia))
                        {
                            mRecebido = receiveMsg(Socket, mRecebido);
                            time(&fim);
                            if (difftime(fim, inicio) > 1)
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
                        if(mRecebido.Tipo == NACK)
                        {
                            mRecebido = trataNACK(Socket, Sequencia, mRecebido, mEnviado);
                        }
                        if (mRecebido.Tipo == 0xb)
                        {
                            if (!checkParity(mRecebido))
                                enviaNACK(Cliente, Servidor, Socket, Sequencia, mRecebido, mEnviado);
                            enviaACK(Cliente, Servidor, Socket, Sequencia, 0x8, mRecebido, mEnviado);
                        }
                        
                        if (mRecebido.Sequencia == 1)
                            continue;
                        
                        for (int i = 0; i < (int)mRecebido.Tamanho; i++)
                        {
                            if (mRecebido.Dados[i] != '\n')
                                printf("%c",mRecebido.Dados[i]);
                            else
                                printf(" ");
                        }
                    }
                    printf("\n");

                    break;

//----------------VER-----------------------------------------------------------------------------------------------------------------------------
                case 0x4:
                    mRecebido.Inicio = 0;
                    mEnviado.Inicio = 0;
                    
                    time(&inicio);
                    while ( (mRecebido.Inicio != INITMSG) || ( (mRecebido.Tipo != 0xc && mRecebido.Tipo != NACK && mRecebido.Tipo != 0xF) || (int)mRecebido.Sequencia != Sequencia) )
                    {
                        mRecebido = receiveMsg(Socket, mRecebido);
                        time(&fim);
                        if (difftime(fim, inicio) > 1)
                        {
                            timeOut = 1;
                            break;
                        }
                    } 
        
                    if (timeOut == 1)
                        break;

                    //caso nack
                    if(mRecebido.Tipo == 0x9)
                    {
                        mRecebido = trataNACK(Socket, Sequencia, mRecebido, mEnviado);
                    }
                    
                    if(mRecebido.Tipo == 0xc)
                    {
                        if(!checkParity(mRecebido))
                        {
                            enviaNACK(Cliente, Servidor, Socket, Sequencia, mRecebido, mEnviado);
                        }
                        Sequencia++;
                        enviaACK(Cliente, Servidor, Socket, Sequencia, 0x8, mRecebido, mEnviado);
                    }
                    
                    if(mRecebido.Tipo == 0xF)
                    {  
                        if (mRecebido.Dados[0] == '3')
                            printf("Erro arquivo inexistente");
                        error = 1;
                    }

                    if (error == 1)
                    {    
                        printf("\n");
                        break;
                    }
                    
                    while(mRecebido.Tipo != 0xd)
                    {
                        mRecebido.Inicio = 0;
                        mEnviado.Inicio = 0;
                        time(&inicio);
                        while ( (mRecebido.Inicio != INITMSG) || ( (mRecebido.Tipo != 0xc && mRecebido.Tipo != 0xd && mRecebido.Tipo != NACK) ||(int) mRecebido.Sequencia != Sequencia))
                        {
                                                
                            mRecebido = receiveMsg(Socket, mRecebido);
                            time(&fim);
                            if (difftime(fim, inicio) > 1)
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
                        if(mRecebido.Tipo == NACK)
                        {
                            mRecebido = trataNACK(Socket, Sequencia, mRecebido, mEnviado);
                        }
                        if (mRecebido.Tipo == 0xc)
                        {
                            if (!checkParity(mRecebido))
                                enviaNACK(Cliente, Servidor, Socket, Sequencia, mRecebido, mEnviado);
                            enviaACK(Cliente, Servidor, Socket, Sequencia, 0x8, mRecebido, mEnviado);
                        }
                        
                        for (int i = 0; i < (int)mRecebido.Tamanho; i++)
                            printf("%c",mRecebido.Dados[i]);
                    }
                    printf("\n");
                    break;

//----------------LINHA----------------------------------------------------------------------------------------------------------------------------
                case 0x5:
                    mRecebido.Inicio = 0;
                    mEnviado.Inicio = 0;
                    
                    time(&inicio);
                    while ( (mRecebido.Inicio != INITMSG) || ( (mRecebido.Tipo != ACK && mRecebido.Tipo != NACK && mRecebido.Tipo != 0xF) || (int)mRecebido.Sequencia != Sequencia) )
                    {
                        mRecebido = receiveMsg(Socket, mRecebido);
                        time(&fim);
                        if (difftime(fim, inicio) > 1)
                        {
                            timeOut = 1;
                            break;
                        }
                    } 
        
                    if (timeOut == 1)
                        break;

                    //caso nack
                    if(mRecebido.Tipo == 0x9)
                    {
                        mRecebido = trataNACK(Socket, Sequencia, mRecebido, mEnviado);
                    }
                    
                    if(mRecebido.Tipo == ACK)
                    {
                        Sequencia++;
                        mEnviado = newMsg(Cliente, Servidor, Parametro1, 0xa, Sequencia);
                        sendMsg(Socket, mEnviado);
                    }
                    if(mRecebido.Tipo == 0xF)
                    {  
                        if (mRecebido.Dados[0] == '3')
                            printf("Erro arquivo inexistente");
                        error = 1;
                    }

                    if (error == 1)
                    {    
                        printf("\n");
                        break;
                    }
                    
                    time(&inicio);
                    while ( (mRecebido.Inicio != INITMSG) || ( (mRecebido.Tipo != 0xc && mRecebido.Tipo != NACK) || (int)mRecebido.Sequencia != Sequencia) )
                    {
                        mRecebido = receiveMsg(Socket, mRecebido);
                        time(&fim);
                        if (difftime(fim, inicio) > 1)
                        {
                            timeOut = 1;
                            break;
                        }
                    } 
                    if (timeOut == 1)
                            break;
                    //caso nack
                    if(mRecebido.Tipo == 0x9)
                    {
                        mRecebido = trataNACK(Socket, Sequencia, mRecebido, mEnviado);
                    }
                    
                    if(mRecebido.Tipo == 0xc)
                    {
                        if(!checkParity(mRecebido))
                        {
                            enviaNACK(Cliente, Servidor, Socket, Sequencia, mRecebido, mEnviado);
                        }
                        Sequencia++;
                        enviaACK(Cliente, Servidor, Socket, Sequencia, 0x8, mRecebido, mEnviado);
                    }
                    

                    while(mRecebido.Tipo != 0xd)
                    {
                        mRecebido.Inicio = 0;
                        mEnviado.Inicio = 0;

                        time(&inicio);
                        while ( (mRecebido.Inicio != INITMSG) || ( (mRecebido.Tipo != 0xc && mRecebido.Tipo != 0xd && mRecebido.Tipo != NACK) ||(int) mRecebido.Sequencia != Sequencia))
                        {
                            mRecebido = receiveMsg(Socket, mRecebido);
                            time(&fim);
                            if (difftime(fim, inicio) > 1)
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
                        if(mRecebido.Tipo == NACK)
                        {
                            mRecebido = trataNACK(Socket, Sequencia, mRecebido, mEnviado);
                        }
                        if (mRecebido.Tipo == 0xc)
                        {
                            if (!checkParity(mRecebido))
                                enviaNACK(Cliente, Servidor, Socket, Sequencia, mRecebido, mEnviado);
                            enviaACK(Cliente, Servidor, Socket, Sequencia, 0x8, mRecebido, mEnviado);
                        }
                        
                        for (int i = 0; i < (int)mRecebido.Tamanho; i++)
                            printf("%c",mRecebido.Dados[i]);
                    }
                    printf("\n");   
                    break;

//----------------LINHAS---------------------------------------------------------------------------------------------------------------------------
                case 0x6:
                    mRecebido.Inicio = 0;
                    mEnviado.Inicio = 0;
                    
                    time(&inicio);
                    while ( (mRecebido.Inicio != INITMSG) || ( (mRecebido.Tipo != ACK && mRecebido.Tipo != NACK && mRecebido.Tipo != 0xF) || (int)mRecebido.Sequencia != Sequencia) )
                    {
                        mRecebido = receiveMsg(Socket, mRecebido);
                        time(&fim);
                        if (difftime(fim, inicio) > 1)
                        {
                            timeOut = 1;
                            break;
                        }
                    } 
        
                    if (timeOut == 1)
                        break;

                    //caso nack
                    if(mRecebido.Tipo == 0x9)
                    {
                        mRecebido = trataNACK(Socket, Sequencia, mRecebido, mEnviado);
                    }
                    
                    if(mRecebido.Tipo == ACK)
                    {
                        Sequencia++;
                        strcat(Parametro1, "\n");  
                        strcat(Parametro1, Parametro2);
                        
 
                        mEnviado = newMsg(Cliente, Servidor, Parametro1, 0xa, Sequencia);
                        sendMsg(Socket, mEnviado);
                    }
                    if(mRecebido.Tipo == 0xF)
                    {  
                        if (mRecebido.Dados[0] == '3')
                            printf("Erro arquivo inexistente");
                        error = 1;
                    }

                    if (error == 1)
                    {    
                        printf("\n");
                        break;
                    }

                    time(&inicio);
                    while ( (mRecebido.Inicio != INITMSG) || ( (mRecebido.Tipo != 0xc && mRecebido.Tipo != NACK) || (int)mRecebido.Sequencia != Sequencia) )
                    {
                        mRecebido = receiveMsg(Socket, mRecebido);
                        time(&fim);
                        if (difftime(fim, inicio) > 1)
                        {
                            timeOut = 1;
                            break;
                        }
                    } 
        
                    if (timeOut == 1)
                        break;

                    //caso nack
                    if(mRecebido.Tipo == 0x9)
                    {
                        mRecebido = trataNACK(Socket, Sequencia, mRecebido, mEnviado);
                    }
                    
                    if(mRecebido.Tipo == 0xc)
                    {
                        if(!checkParity(mRecebido))
                        {
                            enviaNACK(Cliente, Servidor, Socket, Sequencia, mRecebido, mEnviado);
                        }

                        Sequencia++;

                        enviaACK(Cliente, Servidor, Socket, Sequencia, 0x8, mRecebido, mEnviado);
                    }
                    

                    while(mRecebido.Tipo != 0xd)
                    {
                        mRecebido.Inicio = 0;
                        mEnviado.Inicio = 0;

                        time(&inicio);
                        while ( (mRecebido.Inicio != INITMSG) || ( (mRecebido.Tipo != 0xc && mRecebido.Tipo != 0xd && mRecebido.Tipo != NACK) ||(int) mRecebido.Sequencia != Sequencia))
                        {
                            mRecebido = receiveMsg(Socket, mRecebido);
                            time(&fim);
                            if (difftime(fim, inicio) > 1)
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
                        if(mRecebido.Tipo == NACK)
                        {
                            mRecebido = trataNACK(Socket, Sequencia, mRecebido, mEnviado);
                        }
                        if (mRecebido.Tipo == 0xc)
                        {
                            if (!checkParity(mRecebido))
                                enviaNACK(Cliente, Servidor, Socket, Sequencia, mRecebido, mEnviado);
                            enviaACK(Cliente, Servidor, Socket, Sequencia, 0x8, mRecebido, mEnviado);
                        }
                        
                        for (int i = 0; i < (int)mRecebido.Tamanho; i++)
                            printf("%c",mRecebido.Dados[i]);
                    }
                    printf("\n");   

                    break;

//----------------EDIT-----------------------------------------------------------------------------------------------------------------------------
                case 0x7:
                    
                    mRecebido.Inicio = 0;
                    mEnviado.Inicio = 0;
                    
                    time(&inicio);
                    while ( (mRecebido.Inicio != INITMSG) || ( (mRecebido.Tipo != ACK && mRecebido.Tipo != NACK && mRecebido.Tipo != 0xF) || (int)mRecebido.Sequencia != Sequencia) )
                    {
                        mRecebido = receiveMsg(Socket, mRecebido);
                        time(&fim);
                        if (difftime(fim, inicio) > 1)
                        {
                            timeOut = 1;
                            break;
                        }
                    } 
        
                    if (timeOut == 1)
                        break;
                    
                    //caso nack
                    if(mRecebido.Tipo == 0x9)
                    {
                        mRecebido = trataNACK(Socket, Sequencia, mRecebido, mEnviado);
                    }
                    
                    if(mRecebido.Tipo == ACK)
                    {
                        Sequencia++;
                        mEnviado = newMsg(Cliente, Servidor, Parametro1, 0xa, Sequencia);
                        sendMsg(Socket, mEnviado);
                    }

                    if(mRecebido.Tipo == 0xF)
                    {  
                        if (mRecebido.Dados[0] == '3')
                            printf("Erro arquivo inexistente");
                        error = 1;
                    }

                    if (error == 1)
                    {    
                        printf("\n");
                        break;
                    }
                    
                    time(&inicio);
                    while ( (mRecebido.Inicio != INITMSG) || ( (mRecebido.Tipo != ACK && mRecebido.Tipo != NACK && mRecebido.Tipo != 0xF) || (int)mRecebido.Sequencia != Sequencia) )
                    {
                        mRecebido = receiveMsg(Socket, mRecebido);
                        time(&fim);
                        if (difftime(fim, inicio) > 1)
                        {
                            timeOut = 1;
                            break;
                        }
                    } 
        
                    if (timeOut == 1)
                        break;


                    if(mRecebido.Tipo == 0xF)
                    {  
                        if (mRecebido.Dados[0] == '4')
                            printf("Erro linha inexistente");
                        error = 1;
                    }

                    if (error == 1)
                    {    
                        printf("\n");
                        break;
                    }

                    //caso nack
                    if(mRecebido.Tipo == NACK)
                    {
                        mRecebido = trataNACK(Socket, Sequencia, mRecebido, mEnviado);
                    }
                    
                    if(mRecebido.Tipo == ACK)
                    {
                        if(!checkParity(mRecebido))
                        {
                            enviaNACK(Cliente, Servidor, Socket, Sequencia, mRecebido, mEnviado);
                        }
                        Sequencia++;
                        enviaACK(Cliente, Servidor, Socket, Sequencia, 0xc, mRecebido, mEnviado);

                    }
                    
                    tam = strlen(Parametro3);
                    local = 0;

                    while(tam > local)
                    {
                        mRecebido.Inicio = 0;
                        mEnviado.Inicio = 0;

                        time(&inicio);
                        while ( (mRecebido.Inicio != INITMSG) || ( (mRecebido.Tipo != ACK && mRecebido.Tipo != NACK) || (int)mRecebido.Sequencia != Sequencia) )
                        {
                            mRecebido = receiveMsg(Socket, mRecebido);
                            time(&fim);
                            if (difftime(fim, inicio) > 1)
                            {
                                timeOut = 1;
                                break;
                            }
                        }   
                        
                        if (timeOut == 1)
                            break;
                        
                        if (mRecebido.Tipo == 0x9)
                            mRecebido = trataNACK(Socket, Sequencia, mRecebido, mEnviado);
                        
                        if (mRecebido.Tipo == 0x8)
                        {
                            strbcut(Parametro3, DadosEnviado, local, tam);
                            Sequencia++;
                            if (Sequencia == 256)
                            {
                                Sequencia = 0;
                                multSeq++;
                            }
                            mEnviado = newMsg(Cliente, Servidor, DadosEnviado, 0xc, Sequencia);
                            sendMsg(Socket, mEnviado);
                            
                            local += 15;
                        }
                    
                        if (timeOut == 1)
                            break;
                    }
                    
                    if (timeOut == 1)
                        break;

                    time(&inicio);
                    while ( (mRecebido.Inicio != INITMSG) || ( (mRecebido.Tipo != ACK && mRecebido.Tipo != NACK) || (int)mRecebido.Sequencia != Sequencia) )
                    {
                        mRecebido = receiveMsg(Socket, mRecebido);
                        time(&fim);
                        if (difftime(fim, inicio) > 1)
                        {
                            timeOut = 1;
                            break;
                        }
                    }
                    if (timeOut == 1)
                        break;

                    if (mRecebido.Tipo == 0x9)
                        mRecebido = trataNACK(Socket, Sequencia, mRecebido, mEnviado);
                    
                    if (mRecebido.Tipo == 0x8)
                    {    
                        Sequencia++;
                        if (Sequencia == 256)
                        {
                            Sequencia = 0;
                            multSeq++;
                        }
                        enviaACK(Cliente, Servidor, Socket, Sequencia, 0xd, mRecebido, mEnviado);
                    }
                    break;
                
                
                
//----------------L-CHANGE-DIRECTORY----------------------------------------------------------------------------------------------------------------
                case 0x1:
                    lcd(Parametro1);
                    printf("\n");
                    break;
//----------------L-LIST---------------------------------------------------------------------------------------------------------------------------- 
                case 0x3:
                    local = 0;
                    DadosEnviado = lls(local, &tamanhoLS);
                    while (tamanhoLS - 1 > local)
                    {
                        DadosEnviado = lls(local, &tamanhoLS);
                        for (int i = 0; i < strlen(DadosEnviado); i++)
                        {
                            if (DadosEnviado[i] != '\n')
                                printf("%c", DadosEnviado[i]);
                            else
                                printf(" ");
                        }
                        local++;
                    }
                    printf("\n");
                    break;
                default:
                    printf("Comando Errado\n");
                    break;
            }
        }    

        tam = 0;
        local = 0;
        multSeq = 0;
        Sequencia = 0;
        tamanhoLS = 0;
        error = 0;
        mEnviado.Inicio = 0;
        mRecebido.Inicio = 0;
        if (timeOut == 1)
            printf("Timeout\n");
        timeOut = 0;  
        strcpy(CMDUSER, "");
        strcpy(Comando, "");
        strcpy(Parametro1, "");
        strcpy(Parametro2, "");        
        strcpy(Parametro3, "");
        strcpy(DadosEnviado, "");
    }

    return 0;
}

