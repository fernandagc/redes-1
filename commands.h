#include <stdio.h>
#include <string.h>

#ifndef __COMMANDS__
#define __COMMANDS__
#define INITMSG 0x3a
#define TAM_MSG 15
#define Cliente 10
#define Servidor 01
#define BUFFER 256
#define ACK 0x8
#define NACK 0x9

typedef struct Mensagem
{
    unsigned char Inicio;  					// inicio da mensagem
    unsigned char Tamanho : 4;              // tamanho da mensagem
    unsigned char Origem : 2;				// end de origem
    unsigned char Destino: 2;				// end de destino
    unsigned char Sequencia;                // ordem de sequencia da mensagem
    unsigned char Tipo : 4;                 // tipo da mensagem (CD, Ack, (...), ls)
    unsigned char Dados[TAM_MSG];                // dados a serem enviados da mensagem
    unsigned char Paridade;                 // paridade da mensagem
} Mensagem;

int rawSocket();

char* initPort(int tam);

int cmd(char* cmd_, char* Dados, char* param1, char* param2, char* param3);

void cd(short int *error, char *diretorio);

void lcd(char *path);

void ls(short int *error);

char* lls(int index, long int *sizeParam);

void verArquivo(short int *error, char *arquivo);

char* ver(char *nomeArquivo,long int *tamArquivo, int local);

void edit(int numeroLinha,char *nomeArquivo, char *txt, int tamConteudo);

void sendMsg(int socket, Mensagem m);

Mensagem receiveMsg(int socket, Mensagem ultMensagem);

int comparar(Mensagem priMsg, Mensagem ultMsg);

Mensagem newMsg(int Origem, int Destino, char *Dados, int Tipo, int Sequencia);

void sendACK(int Origem, int Destino, int Soquete, int Sequencia, int Tipo, Mensagem recebida, Mensagem enviada);

void sendERR(int Origem, int Destino, int Soquete, int Sequencia, int Tipo, short int  error, Mensagem enviada);

Mensagem ttNACK(int Soquete, int Sequencia, Mensagem recebida, Mensagem enviada);

void sendNACK(int Origem, int Destino, int Socket, int Sequencia, Mensagem recebida, Mensagem enviada);

int checkParity(Mensagem mensagemRecebida);

void showLine(short int *error, char *linha, char* arquivo);

void strcut(char *Cortado, char *Resultado, char* Cortador);

char* strbcut(char *param3, char *Conteudo, int local, int tam);

char* list(int index, long int *sizeParam);

char* linha(int numeroLinha, char *nomeArquivo, long int *tamLinha, int local);

#endif
