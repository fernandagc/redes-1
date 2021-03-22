#include "operations.h"

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
    time_t inicio, fim;
    short int timeOut, error = 0;
    char *Conteudo, *Parametro1, *Parametro2, *Parametro3, *ParametroEdit;
    long int tam = 0;
    int local = 0, Socket = 0, Sequencia = 0, multSeq = 0, tamIni = 0, tamFim = 0;
                
    Mensagem mEnviado, mRecebido;

    Conteudo = initPort(15);
    Parametro1 = initPort(15);
    Parametro2 = initPort(15);
    Parametro3 = initPort(15);
    
    ParametroEdit = initPort(1024);

    mRecebido.Inicio = 0;
    mEnviado.Inicio = 0;

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
        while ((((mRecebido.Inicio != INITMSG)) || ( mRecebido.Tipo != 0x0 && mRecebido.Tipo != 0x2 && mRecebido.Tipo != 0x4 && mRecebido.Tipo != 0x5 && mRecebido.Tipo != 0x6 && mRecebido.Tipo != 0x7 )  || ((int)mRecebido.Sequencia != Sequencia)))  
        {
            mRecebido = receiveMsg(Socket, mRecebido);
        }
        

        if (!checkParity(mRecebido) && timeOut == 0)
            enviaNACK(Servidor, Cliente, Socket, Sequencia, mRecebido, mEnviado);                
            
        switch (mRecebido.Tipo)
        {
//--------------CD----------------------------------------------------------------------------------------------------------------
            case 0x0:
                strcpy(Parametro1,(char *)mRecebido.Dados);
                
                cd(&error, Parametro1);
                if (error == 0)
                {
                    enviaACK(Socket, Sequencia, ACK, mRecebido, mEnviado);
                    Sequencia++;
                }
                else
                {
                    enviaERR(Socket, Sequencia, 0xF, error, mEnviado); 
                    break;
                }
                
                lcd(Parametro1);
                break;        
                    
//----------------LIST----------------------------------------------------------------------------------------------------------------------------
            case 0x2: 
                ls(&error);

                if (error == 0)
                {
                    enviaACK(Socket, Sequencia, 0xb, mRecebido, mEnviado);
                    Sequencia++;
                }
                else
                {
                    enviaERR(Socket, Sequencia, 0xF, error, mEnviado); 
                    break;
                }
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
                
                //começa o ls
                if (mRecebido.Tipo == 0x8)
                {
                    Conteudo = list(local, &tam);
                
                    mEnviado = newMsg(Servidor, Cliente, Conteudo, 0xb, Sequencia);
                    sendMsg(Socket, mEnviado);
                    Sequencia++;
                }
                local = 1;
                while (tam >= local)
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
                    
                    if(mRecebido.Tipo == ACK)
                    {
                        Conteudo = list(local, &tam);
                        mEnviado = newMsg(Servidor, Cliente, Conteudo, 0xb, Sequencia);
                        sendMsg(Socket, mEnviado);
                        Sequencia++;
                        if (Sequencia == 256)
                        {
                            Sequencia = 0;
                            multSeq++;
                        }
                        local ++;
                        if((int)mEnviado.Tamanho == 14)
                        {
                            tam++;
                        }

                    }
                }

                
                if (timeOut == 1)
                    break;        
                
                time(&inicio);
                //envia a mensagem final
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

                    enviaACK(Socket, Sequencia, 0xd, mRecebido, mEnviado);
                }
        
            break;

//----------------VER-----------------------------------------------------------------------------------------------------------------------------
            case 0x4:  
                strcpy(Parametro1, (char *)mRecebido.Dados);
                
                verArquivo(&error, Parametro1);

                if (error == 0)
                {
                    enviaACK(Socket, Sequencia, 0xc, mRecebido, mEnviado);
                    Sequencia++;
                }
                else
                {
                    enviaERR(Socket, Sequencia, 0xF, error, mEnviado); 
                    break;
                }

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
                    Conteudo = ver(Parametro1, &tam, local);
                    
                    mEnviado = newMsg(Servidor, Cliente, Conteudo, 0xc, Sequencia);
                    
                    sendMsg(Socket, mEnviado);

                }    
                Sequencia++;             
                
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
                        local += 15;
                        Conteudo = ver(Parametro1, &tam, local);
                        mEnviado = newMsg(Servidor, Cliente, Conteudo, 0xc, Sequencia);
                        sendMsg(Socket, mEnviado);
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
                    enviaACK(Socket, Sequencia, 0xd, mRecebido, mEnviado);
                }    
                break;

//----------------LINHA----------------------------------------------------------------------------------------------------------------------------
            case 0x5: 
                strcpy(Parametro2, (char *)mRecebido.Dados);
                
                verArquivo(&error, Parametro2);

                if (error == 0)
                {
                    enviaACK(Socket, Sequencia, ACK, mRecebido, mEnviado);
                    Sequencia++;
                }
                else
                {
                    enviaERR(Socket, Sequencia, 0xF, error, mEnviado);
                    break;
                }
                
                mRecebido.Inicio = 0;
                mEnviado.Inicio = 0;

                time(&inicio);
                while ( (mRecebido.Inicio != INITMSG) || ( (mRecebido.Tipo != 0xa) || (int)mRecebido.Sequencia != Sequencia) )
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
                  
                tamIni = 0;
                if (mRecebido.Tipo == 0xa)
                {
                    if (!checkParity(mRecebido))
                        enviaNACK(Servidor, Cliente, Socket, Sequencia, mRecebido, mEnviado);

                    strcpy(Parametro1, (char *)mRecebido.Dados);
                    tamIni = atoi(Parametro1);

                    mEnviado = newMsg(Servidor, Cliente, Conteudo, 0xc, Sequencia);                
    
                    sendMsg(Socket, mEnviado);
                    Sequencia++;             
                }    
                
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

                if (mRecebido.Tipo == 0x9)
                    mRecebido = trataNACK(Socket, Sequencia, mRecebido, mEnviado);
                
                if (mRecebido.Tipo == 0x8)
                {
                    Conteudo = linha(tamIni, Parametro2, &tam, local);
                    mEnviado = newMsg(Servidor, Cliente, Conteudo, 0xc, Sequencia);
                 
                    sendMsg(Socket, mEnviado);
                    Sequencia++;             
                }    
                
                while(tam - 15 > local)
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
                    if (mRecebido.Tipo == 0x9)
                        mRecebido = trataNACK(Socket, Sequencia, mRecebido, mEnviado);
                    
                    if (mRecebido.Tipo == 0x8)
                    {
                        local += 14;
                        Conteudo = linha(tamIni, Parametro2, &tam, local);
                        mEnviado = newMsg(Servidor, Cliente, Conteudo, 0xc, Sequencia);

                        sendMsg(Socket, mEnviado);
                        Sequencia++;
                        if (Sequencia == 256)
                        {
                            Sequencia = 0;
                            multSeq++;
                        }
                    }
                }
                
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
                    enviaACK(Socket, Sequencia, 0xd, mRecebido, mEnviado);
                }    
                break;
//-----------------LINHAS---------------------------------------------------------------------------------------------------------------------------
            case 0x6:
                strcpy(Parametro3, (char *)mRecebido.Dados);
                
                verArquivo(&error, Parametro3);

                if (error == 0)
                {
                    enviaACK(Socket, Sequencia, ACK, mRecebido, mEnviado);
                    Sequencia++;
                }
                else
                {
                    enviaERR(Socket, Sequencia, 0xF, error, mEnviado); 
                    break;
                }
                
                mRecebido.Inicio = 0;
                mEnviado.Inicio = 0;

                time(&inicio);
                while ( (mRecebido.Inicio != INITMSG) || ( (mRecebido.Tipo != 0xa) || (int)mRecebido.Sequencia != Sequencia) )
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

                tamIni = 0;
                tamFim = 0;
                
                if (mRecebido.Tipo == 0xa)
                {
                    if (!checkParity(mRecebido))
                        enviaNACK(Servidor, Cliente, Socket, Sequencia, mRecebido, mEnviado);

                    short int flag = 0;
                    int k = 0;
                    for (int i = 0; i < (int)mRecebido.Tamanho; i++)
                    {
                        if (mRecebido.Dados[i] == '\n')
                        {
                            flag = 1;
                            continue;
                        }
                        if (flag == 0)
                        {
                            Parametro1[i] = mRecebido.Dados[i]; 
                        }
                        else
                        {
                            Parametro2[k] = mRecebido.Dados[i];
                            k++;
                        }
                        
                    }
                    tamIni = atoi(Parametro1);
                    tamFim = atoi(Parametro2);
                    mEnviado = newMsg(Servidor, Cliente, Conteudo, 0xc, Sequencia);           
                    sendMsg(Socket, mEnviado);
                    Sequencia++;             
                }    
                
                while (tamIni <= tamFim)
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
                        Conteudo = linha(tamIni, Parametro3, &tam, local);
                        mEnviado = newMsg(Servidor, Cliente, Conteudo, 0xc, Sequencia);
                        sendMsg(Socket, mEnviado);
                        Sequencia++;
                        if (Sequencia == 256)
                        {
                            Sequencia = 0;
                            multSeq++;
                        }             
                    }    
                    
                    while(tam - 15 > local)
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
                            local += 14;
                            Conteudo = linha(tamIni, Parametro3, &tam, local);                        
                            mEnviado = newMsg(Servidor, Cliente, Conteudo, 0xc, Sequencia);
                            sendMsg(Socket, mEnviado);
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
                    enviaACK(Socket, Sequencia, 0xd, mRecebido, mEnviado);
                }
                break; 

//------------------------EDIT-----------------------------------------------------------------------------------------------------------------------------
            case 0x7: 
                tamIni = 0;
                int tamConteudo = 0;
                local = 0;            
                int i;
                strcpy(Parametro2, (char *)mRecebido.Dados);
                verArquivo(&error, Parametro2);
                if (error == 0)
                {
                    enviaACK(Socket, Sequencia, ACK, mRecebido, mEnviado);
                    Sequencia++;
                }
                else
                {
                    enviaERR(Socket, Sequencia, 0xF, error, mEnviado); 
                    break;
                }
                
                mRecebido.Inicio = 0;
                mEnviado.Inicio = 0;

                time(&inicio);
                while ( (mRecebido.Inicio != INITMSG) || ( (mRecebido.Tipo != 0xa) || (int)mRecebido.Sequencia != Sequencia) )
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

                if (mRecebido.Tipo == 0xa)
                {
                    if (!checkParity(mRecebido))
                        enviaNACK(Servidor, Cliente, Socket, Sequencia, mRecebido, mEnviado);

                    strcpy(Parametro1, (char *)mRecebido.Dados);
                    
                    showLine(&error, Parametro1, Parametro2);
                    
                    if (error == 0)
                    {  
                        tamIni = atoi(Parametro1);
                        mEnviado = newMsg(Servidor, Cliente, Conteudo, ACK, Sequencia);                
                        sendMsg(Socket, mEnviado);
                        Sequencia++;             
                    }
                    else
                    {
                        enviaERR(Socket, Sequencia, 0xF, error, mEnviado);
                        break;
                    }
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
                    
                    if(mRecebido.Tipo == NACK)
                    {
                        mRecebido = trataNACK(Socket, Sequencia, mRecebido, mEnviado);
                    }
                    if (mRecebido.Tipo == 0xc)
                    {
                        if (!checkParity(mRecebido))
                            enviaNACK(Servidor, Cliente, Socket, Sequencia, mRecebido, mEnviado);
                        enviaACK(Socket, Sequencia, 0x8, mRecebido, mEnviado);
                    }
                    
                    Sequencia++;
                    if (Sequencia == 256)
                    {
                        Sequencia = 0;
                        multSeq++;
                    }    
                    for (i = 0; i < (int)mRecebido.Tamanho; i++)
                    {
                        ParametroEdit[i+local] = mRecebido.Dados[i];
                    }
                
                    tamConteudo+=i;
                    local += 15;
                } 

                
                if (timeOut == 1)
                    break;

                edit(tamIni, Parametro2, ParametroEdit, tamConteudo);
                printf("Edit operado com sucesso\n");
                break;              
            
                default:
                printf("Comando errado\n");
                break;
        }
        
        strcpy(Conteudo, "");
        strcpy(Parametro1, "");
        strcpy(Parametro2, "");
        strcpy(Parametro3, "");
        strcpy(ParametroEdit, "");
        
        tam = 0;
        local = 0;
        error = 0;
        multSeq = 0;
        timeOut = 0;
        Sequencia = 0;
        mRecebido.Inicio = 0;
        mEnviado.Inicio = 0;
    }
    
    return 0;
}

