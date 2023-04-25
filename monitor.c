#include <stdio.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <stdlib.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include "structs.h"
#include "string.h"
#include <sys/time.h> // in order to obtain the current time stamp
#define PIPE_NAME "fifo"
#define LOG_FILE "status_log.txt"

//programa servidor com o qual o cliente deve interagir

// retorna o tempo em segundos decorrido
long getFinalTimeStamp(long initial_timestamp){
    time_t current_time;
    current_time=time(NULL);
    long now = (long) current_time;
    long before = initial_timestamp;
    long final_time = now-before;
    return final_time;
}

void status_log_file(PEDIDOSEXECUCAO *head){
    PEDIDOSEXECUCAO *atual=head;
    FILE* log_file = fopen(LOG_FILE, "a");
    if (log_file == NULL) {
        perror("Erro ao abrir o arquivo de log");
        exit(EXIT_FAILURE);}
    while(atual!=NULL){
        long tempodeexecucaoateaomomento= getFinalTimeStamp(atual->initial_timestamp);
        fprintf(log_file, "%d %s %ld\n", atual->pid, atual->nome_programa, tempodeexecucaoateaomomento);
        atual=atual->prox;

    }
    fclose(log_file);
}

void addNovoPedido(PEDIDOSEXECUCAO **head, int pid, char nome_programa[],long time_initial){
    PEDIDOSEXECUCAO *add = (PEDIDOSEXECUCAO*)malloc(sizeof(PEDIDOSEXECUCAO));
    add->pid=pid;
    strcpy(add->nome_programa, nome_programa);
    add->prox = NULL;
    add->pid=pid;
    add->initial_timestamp=time_initial;
    if(*head == NULL){
        *head = add;
        return;
    }
    PEDIDOSEXECUCAO*aux=*head;
    while(aux->prox!=NULL){
        aux=aux->prox;
    }
    aux->prox=add;
}

void removePedido(PEDIDOSEXECUCAO **head, int pid){
    if (*head == NULL){
        return;
    }
    //se o pedido a remover for a cabeça da lista, temos de fazer update da cabeça
    if((*head)->pid == pid){
        PEDIDOSEXECUCAO *aux = *head;
        *head = (*head)->prox;
        free(aux);
        return;}
    //caso contrário, temos de percorrer a struct até encontrar o elemento a ser removido
    PEDIDOSEXECUCAO *atual=*head;
    PEDIDOSEXECUCAO *ant=NULL;
    while(atual!= NULL && atual->pid!=pid){
        ant=atual;
        atual=atual->prox;
    }
    //se o elemento for encontrado, removemo-lo da struct
    if(atual!=NULL){
        ant->prox=atual->prox;
        free(atual);
    }
    }



int main(int argc, char const *argv[]){
    // criação dos fifos
    char buffer_pedido[512];
    char buffer_resposta[512];
    int fd_serverToClient;
    int fd_clientToServer;
    int bytes_read;
    mkfifo("/tmp/clientToServer",0666); // client -> server
    mkfifo("/tmp/serverToClient",0666); // server -> client
    
    //abre o fifo para receber o pedido do cliente
    fd_clientToServer = open("/tmp/clientToServer", O_RDONLY);
    if (fd_clientToServer == -1){
        perror("Erro ao abrir o pedido do cliente");
        _exit(EXIT_FAILURE);
    }

    //abre o fifo para enviar a resposta ao cliente
    fd_serverToClient = open("/tmp/serverToClient", O_WRONLY);
    if (fd_serverToClient == -1){
        perror("Erro ao enviar a resposta ao cliente");
        _exit(EXIT_FAILURE);
    }

    //lê os comandos que vêm no fifo do client
    while((bytes_read == read(fd_clientToServer, buffer_pedido, strlen(buffer_pedido)-1))>0){
        if(bytes_read == -1){
            perror("Erro ao ler o pedido do cliente");
            _exit(EXIT_FAILURE);
        }
        if (strcmp(buffer_pedido,"status") == 0){
            FILE *log_status= fopen(LOG_FILE, "r"); //abrimos o ficheiro log para leitura
            char log_buffer [512];
            while(fgets(log_buffer,512,log_status)!=NULL){
                write(fd_serverToClient,log_buffer,strlen(log_buffer));
            }
            fclose(log_status);}

        else{
        buffer_pedido[bytes_read] ='\0';
        FILE *executefile;
        char buffer_dos_comandos[512];
        char buffer_dos_resultados[512];
        sprintf(buffer_dos_comandos, "%s 2>&1", buffer_pedido);
        executefile = open(buffer_dos_comandos, "r");
        if (executefile == NULL){
            sprintf(buffer_dos_resultados, "Erro ao executar os comandos\n");}
        else{
            while(fgets(buffer_dos_resultados, sizeof(buffer_dos_resultados)-1, executefile)!= NULL){
                strcat(buffer_resposta, buffer_dos_resultados); }
            pclose(executefile);
        }
        write(fd_serverToClient, buffer_resposta, strlen(buffer_resposta));}
    }

    close(fd_clientToServer);
    close(fd_serverToClient);
    unlink("/tmp/clientToServer");
    unlink("/tmp/serverToClient");
    return 0;
}