#include "commands.h"

#include <fcntl.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>  
#include <sys/stat.h>
#include <linux/if.h>
#include <sys/types.h>
#include <arpa/inet.h>
#include <sys/ioctl.h>
#include <sys/socket.h>
#include <net/ethernet.h>
#include <linux/if_packet.h>
#include <stdbool.h>

int ConexaoRawSocket()
{
  int soquete;
  struct ifreq ir;
  struct sockaddr_ll endereco;
  struct packet_mreq mr;

  soquete = socket(AF_PACKET, SOCK_RAW, htons(ETH_P_ALL));  	/*cria socket*/
  if (soquete == -1) {
    printf("Erro no Socket\n");
    exit(-1);
  }

  memset(&ir, 0, sizeof(struct ifreq));  	/*dispositivo lo*/
  memcpy(ir.ifr_name, "lo", sizeof("lo"));
  if (ioctl(soquete, SIOCGIFINDEX, &ir) == -1) {
    printf("Erro no ioctl\n");
    exit(-1);
  }
	

  memset(&endereco, 0, sizeof(endereco)); 	/*IP do dispositivo*/
  endereco.sll_family = AF_PACKET;
  endereco.sll_protocol = htons(ETH_P_ALL);
  endereco.sll_ifindex = ir.ifr_ifindex;
  if (bind(soquete, (struct sockaddr *)&endereco, sizeof(endereco)) == -1) {
    printf("Erro no bind\n");
    exit(-1);
  }


  memset(&mr, 0, sizeof(mr));          /*Modo Promiscuo*/
  mr.mr_ifindex = ir.ifr_ifindex;
  mr.mr_type = PACKET_MR_PROMISC;
  if (setsockopt(soquete, SOL_PACKET, PACKET_ADD_MEMBERSHIP, &mr, sizeof(mr)) == -1)	{
    printf("Erro ao fazer setsockopt\n");
    exit(-1);
  }
 
  return soquete;
}

char* initPort(int tam)
{
    char *porta;
    
    porta = NULL;     
    
    if (tam == 0)
        porta = malloc(BUFFER * sizeof(char));
    else               
        porta = malloc(tam * sizeof(char));
    return porta;
}

int cmd(char* cmd_, char* Dados, char* param1, char* param2, char* param3)
{
    int tipo;

    if (!strcmp(cmd_, "cd"))
    {
        tipo = 0x0;
        strcpy(Dados, param1);
    }
    else if (!strcmp(cmd_, "lcd"))
    {
        tipo = 0x1;
        strcpy(Dados, param1);
    }
     if (!strcmp(cmd_, "ls"))
    {
        tipo = 0x2;
        Dados = NULL;
    }
    else if (!strcmp(cmd_, "lls"))
    {
        tipo = 0x3;
        Dados = NULL;
    }                
    else if (!strcmp(cmd_, "ver"))
    {
        tipo = 0x4;
        strcpy(Dados, param1);    
    }    
    else if (!strcmp(cmd_, "linha"))
    {   
        tipo = 0x5;
        strcpy(Dados, param2);
    }    
    else if (!strcmp(cmd_, "linhas"))
    {   
        tipo = 0x6;
        strcpy(Dados, param3);
    }
    else if (!strcmp(cmd_, "edit"))
    {
        tipo = 0x7;
        strcpy(Dados, param2);
    }
    else if (!strcmp(cmd_, "ack"))
    {
        tipo = 0x8;
        Dados = NULL;
    }    
    else if (!strcmp(cmd_, "nack"))
    {
        tipo = 0x9;
        Dados = NULL;
    }
    else if (!strcmp(cmd_, "err"))
    {
        tipo = 0xF;
        Dados = NULL;
    }
    return tipo;
}

void cd(short int *error, char *diretorio)
{
    int status;
    
    if (strcmp(diretorio, ".."))
    {
        status = chdir(diretorio);
        if (status == AT_EACCESS)
        {
            *error = 1;
        }
        if (status == -1)
        {
            *error = 2;
        }
        else
        {
            *error = 0;
        }    
    }
    else
        *error = 0;
    
}

void lcd(char *path)
{
    chdir(path);
}

void ls(short int *error)
{
    FILE *fp = popen("ls", "r");

    if (fp == NULL)
    {
        *error = 1;
    }

}

char* lls(int index, long int *sizeParam)
{
    FILE *fp;
    int status;
    char *Conteudo;

    Conteudo = initPort(1024);
    char* Lixo = initPort(1024);
    
    if (*sizeParam != 0)
    { 
        fp = popen("ls", "r");

       for(int i = 0; i < index; i++)
        {
            fgets(Lixo, 1024, fp);
            i++;
        }    
        
        fgets(Conteudo, 1024, fp);
        
        status = pclose(fp);
        if (status == -1) 
        {
            perror("Erro no pclose");
            exit(1);
        }
    }
    else
    {
        fp = popen("ls | wc -l", "r");
        fgets(Conteudo, 1024, fp); 
        *sizeParam = atoi(Conteudo) + 1;
        status = pclose(fp);
        if (status == -1) 
        {
            perror("Erro no pclose");
            exit(1);
        }
    }     
    
    return Conteudo;
}

void verArquivo(short int *error, char *arquivo)
{
    FILE *arq = fopen(arquivo, "r");

    if (!arq)
    {
        *error = 3;
    }
    else 
    {
        *error = 0;
    }
}

char* ver(char *nomeArquivo,long int *tamArquivo, int local)
{
    FILE* arq;
    char* Conteudo = NULL;
    arq = fopen(nomeArquivo, "r");
    fseek (arq, 0, SEEK_END);
    *tamArquivo = ftell(arq);
    rewind(arq);

    Conteudo = malloc(TAM_MSG * sizeof(char));
    fseek(arq, local, SEEK_SET);
    fread(Conteudo, sizeof(char), TAM_MSG, arq);

    *tamArquivo -= TAM_MSG;

    fclose(arq);
    
    return Conteudo;
}

void edit(int numeroLinha,char *nomeArquivo, char *txt, int tamConteudo)
{
	FILE* novo;
    FILE* backup;
    int i = 1;
    char* linha = NULL;
    linha = malloc(1024 * sizeof(char));
    
    creat("newArquivo.txt", S_IRUSR | S_IWUSR | S_IROTH | S_IWOTH | S_IRGRP | S_IWGRP);
    chmod("newArquivo.txt", S_IRUSR | S_IWUSR | S_IWOTH | S_IROTH | S_IRGRP | S_IWGRP);    
    
    backup = fopen(nomeArquivo, "r+");
    novo = fopen("newArquivo.txt", "w");
    
    while (!feof(backup))
    {
        if(numeroLinha == i)
        {
            fgets (linha, 1024, backup);
            for (int j = TAM_MSG; j < tamConteudo+TAM_MSG; j++)
            {
                putc(txt[j], novo);
            }
            fwrite("\n", sizeof(char), 1, novo);
        }
        fgets (linha, 1024, backup);
        fwrite(linha, sizeof(char), strlen(linha), novo);
        i++;    
    }
 
    fclose(backup);
    fclose(novo);

    remove(nomeArquivo);
    rename("newArquivo.txt", nomeArquivo);
}

void sendMsg(int socket, Mensagem m)
{
    if(send(socket, &m, sizeof(Mensagem),  0) == -1) 
    {
        perror("Erro ao enviar mensagem ou mensagem vazia.\n");
        exit(1);
    }
}

Mensagem receiveMsg(int socket, Mensagem ultMensagem)
{
    Mensagem m;
    
    if (recv(socket, &m, sizeof(Mensagem), 0) == -1)
    {
        perror("Erro ao receber mensagem ou mensagem de tamanho 0.\n");
        exit(1);
    }    
    else 
    {
        if ((m.Inicio == INITMSG) && !(comparar(m, ultMensagem)) )
        {
            return(m);
        }
        else
        {
            m.Inicio = 0;
            return (m);
        }
    }
}

int comparar(Mensagem priMsg, Mensagem ultMsg)
{
    int resultado;
    //VERIFICAR ENDERE?OS AQUI
    if ( (priMsg.Tamanho == ultMsg.Tamanho) && (priMsg.Sequencia == ultMsg.Sequencia) && (priMsg.Tipo == ultMsg.Tipo) && !(strcmp((char *)priMsg.Dados, (char *)ultMsg.Dados)) && (priMsg.Paridade == ultMsg.Paridade) )
        resultado = 1;
    else
        resultado = 0;
    
    return resultado;
}

Mensagem newMsg(int Origem, int Destino, char *Dados, int Tipo, int Sequencia) 
{
    Mensagem m;
    m.Inicio = INITMSG;
    m.Tamanho = (unsigned char)strlen(Dados);
    m.Origem = (unsigned char)Origem;
    m.Destino = (unsigned char)Destino;
    m.Sequencia = (unsigned char)Sequencia;
    m.Tipo = (unsigned char)Tipo;
    strcpy((char *)m.Dados, Dados);
    m.Paridade = m.Tamanho ^ m.Sequencia ^ m.Tipo;
    for(int i = 0; i < strlen(Dados); i++)
    {
        m.Paridade ^= m.Dados[i];
    }
    
    return m;
}

void sendACK(int Origem, int Destino, int Socket, int Sequencia, int Tipo, Mensagem recebida, Mensagem enviada)
{
    enviada = newMsg(Origem, Destino, "\0", Tipo, Sequencia);
    sendMsg(Socket, enviada);
}

void sendERR(int Origem, int Destino, int Socket, int Sequencia, int Tipo, short int  error, Mensagem enviada)
{
    switch (error)
    {
        case 1:
            enviada = newMsg(Origem, Destino, "1", Tipo, Sequencia);
            break;
        case 2:
            enviada = newMsg(Origem, Destino, "2", Tipo, Sequencia);
            break;
        case 3:
            enviada = newMsg(Origem, Destino,"3", Tipo, Sequencia);
            break;
        case 4:
            enviada = newMsg(Origem, Destino,"4", Tipo, Sequencia);
            break;   
        default:
            break;
    }
    sendMsg(Socket, enviada);
}

Mensagem ttNACK(int Socket, int Sequencia, Mensagem recebida, Mensagem enviada)
{
    recebida.Inicio = 0;
    enviada.Inicio = 0;
    
    while(recebida.Tipo == 0x9)
    {
        sendMsg(Socket, enviada);
    
        while ((recebida.Inicio == 0) || comparar(recebida, enviada))
        {
            recebida = receiveMsg(Socket, recebida);
        } 
    
        recebida.Inicio = 0;
        enviada.Inicio = 0;
    }

    return recebida;
}

void sendNACK(int Origem, int Destino, int Socket, int Sequencia, Mensagem recebida, Mensagem enviada)
{        
    
    recebida.Inicio = 0;
    enviada.Inicio = 0;
    
    while(!checkParity(recebida))
    {
        enviada = newMsg(Origem, Destino, "\0", 0x9, Sequencia);
        sendMsg(Socket, enviada);    

        while ((recebida.Inicio == 0) || comparar(recebida, enviada))
        {
            recebida = receiveMsg(Socket, recebida);
        }
        
        recebida.Inicio = 0;
        enviada.Inicio = 0;
    }
}

int checkParity(Mensagem mensagemRecebida)
{
    int paridade, resultado;

    paridade = mensagemRecebida.Tamanho ^ mensagemRecebida.Sequencia ^ mensagemRecebida.Tipo;
    for(int i = 0; i < mensagemRecebida.Tamanho; i++)
    {
        paridade ^= mensagemRecebida.Dados[i];
    }
    
    if (mensagemRecebida.Paridade == paridade)
        resultado = 1;
    else
        resultado = 0;
    
    return (resultado);
}

void showLine(short int *error, char *linha, char *arquivo)
{
    FILE* arq;
    char* Conteudo = NULL;
    int i = 0, numeroLinha = atoi(linha);

    Conteudo = malloc(1024 * sizeof(char));

    arq = fopen(arquivo, "r");
    
    while (!feof(arq))
    {
        if(numeroLinha == i+1)
        {
            *error = 0;
            break;
        }
        fgets (Conteudo, 1024, arq);
        i++;    
    }
 
    if (numeroLinha > i + 1 || numeroLinha < 0)
        *error = 4;

    fclose(arq);
}

//////ACHAR FUNCAO
void strcut(char *Cortado, char *Resultado, char* Cortador)
{
    int tam = strlen(Cortado), j = 0;
    bool flag = false;
    char *Aux = NULL;
    Aux = malloc(sizeof(BUFFER)); 
    
    for (int i = 0; i <= tam-1; i++)
    {   
        if ((Cortado[i] == Cortador[0]) && !(flag))
        {
            flag = true;    
            continue;
        }
        
        if (strcmp(Cortado, "\n"))
        {
            if (!flag)
            {
                Resultado[i] = Cortado[i];
            }
            else
            {   
                Aux[j] = Cortado[i];
                j++;
            }        
        }
    }
    
    Aux[j] = '\0';

    tam = i-j-1;
    if (tam < 0)
        tam *= -1;

    Resultado[tam] = '\0'; 

    strcpy(Cortado, Aux);
    free(Aux);
}
/////ACHAR FUNCAO
char* strbcut(char *param3, char *Conteudo, int local, int tam)
{    
    int i = 0; 
    while (i < TAM_MSG && i < tam)
    {
        Conteudo[i] = param3[local + i];    
        i++;
    }
    Conteudo[i] = '\0';
    return Conteudo;
}
////ACHAR FUNCAO
char* list(int index, long int *sizeParam)
{
    FILE *fp;
    char *Conteudo = initPort(TAM_MSG);
	int status, i = 0;
    
    if (*sizeParam != 0)
    { 
        fp = popen("ls", "r");
        if (fp == NULL)
        {
            perror("Erro no popen");
            exit(1);
        }   
        fgets(Conteudo, TAM_MSG, fp);
        i++; 
        while( (Conteudo != NULL) && (i < index)) 
        {
            fgets(Conteudo, TAM_MSG, fp); 
            i++;
        }
        status = pclose(fp);
        if (status == -1) 
        {
            perror("Erro no pclose");
            exit(1);
        }
    }
    else
    {
        fp = popen("ls | wc -l", "r");
        fgets(Conteudo, 1024, fp); 
        *sizeParam = atoi(Conteudo);
    
        status = pclose(fp);
        if (status == -1) 
        {
            perror("Erro no pclose");
            exit(1);
        }
    }     
    return Conteudo;
}

//ACHAR FUNCAO
char* linha(int numeroLinha, char *nomeArquivo, long int *tamLinha, int local)
{    
    FILE* arq;
    arq = fopen(nomeArquivo, "r");
    char* Conteudo = NULL, 
	char* linha = NULL;
    Conteudo = malloc(TAM_MSG * sizeof(char));
    linha = malloc(1024 * sizeof(char));
    
    long int IniLinha;
    int i = 0;
    
    while (!feof(arq))
    {
        if(numeroLinha == i+1)
        {
            IniLinha = ftell(arq);
            fgets (linha, 1024, arq);
            *tamLinha = strlen(linha);
            fseek(arq, IniLinha + local, SEEK_SET);
            fgets (Conteudo, TAM_MSG, arq);
            break;
        }
        
        fgets (linha, 1024, arq);
        i++;    
    }
 
    fclose(arq);

    return Conteudo;
}


