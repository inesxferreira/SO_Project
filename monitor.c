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
#include "time.h"
#include <sys/time.h> // in order to obtain the current time stamp
#define PIPE_NAME "fifo"
#define LOG_FILE "status_log.txt"

//programa servidor com o qual o cliente deve interagir


void addNovoPedido(PEDIDOSEXECUCAO **head, int pid, char nome_programa[], struct timeval time_initial){
    PEDIDOSEXECUCAO *add = (PEDIDOSEXECUCAO*)malloc(sizeof(PEDIDOSEXECUCAO));
    add->pid=pid;
    strcpy(add->nome_programa, nome_programa);
    add->prox = NULL;
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

struct timeval time_from_buffer(const char* buffer) {
    long int seconds = strtol(buffer + 2, NULL, 10);
    struct timeval tv;
    tv.tv_sec = seconds;
    tv.tv_usec = 0;
    return tv;
}

void status_log_file(PEDIDOSEXECUCAO *head){
    int log_file;
    char log_content [512];
    PEDIDOSEXECUCAO *atual=head;
    log_file = open(LOG_FILE,  O_WRONLY | O_CREAT | O_APPEND, 0644);
   if (log_file == -1){
        perror("Erro ao abrir o ficheiro de log");
        exit(EXIT_FAILURE);}
    while(atual!=NULL){
        struct timeval start_time, end_time;
        start_time = atual->initial_timestamp;
        gettimeofday(&end_time, NULL);
        long process_time = (end_time.tv_sec - start_time.tv_sec) * 1000 + 
                   (end_time.tv_usec - start_time.tv_usec) / 1000;
        sprintf(log_content, "%d %s %ld\n", atual->pid, atual->nome_programa, process_time);
        write(log_file, log_content, strlen(log_content));
        atual = atual->prox;
    }
    close(log_file);
}



int main(int argc, char const *argv[]){
    // criação dos fifos
    char buffer_pedido[512];
    int fd_serverToClient;
    int fd_clientToServer;
    int bytes_read;
    mkfifo("/tmp/clientToServer",0666); // client -> server
    mkfifo("/tmp/serverToClient",0666); // server -> client
    PEDIDOSEXECUCAO p;
    PEDIDOSEXECUCAO* head = NULL;
    int i=1;
    
    while (i){
    printf("novo pedido");
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
    pid_t pid = fork();
    printf("novo pedido");
    bytes_read = read(fd_clientToServer, buffer_pedido, sizeof(buffer_pedido));
    if(bytes_read == -1){
            perror("Erro ao ler o pedido do cliente");
            _exit(EXIT_FAILURE);
        }
    if(pid == -1){
        perror("Erro ao criar um  processo filho no servidor");
        _exit(EXIT_FAILURE);
    }
    else  if (pid == 0){
        if (strcmp(buffer_pedido,"status") == 0){
                FILE *log_status= fopen(LOG_FILE, "r"); //abrimos o ficheiro log para leitura
                if(log_status == NULL){
                    perror("Erro ao abrir o ficheiro de log.");
                    _exit(EXIT_FAILURE);
                }
                char log_buffer [512];
                while(fgets(log_buffer,sizeof(log_buffer),log_status)!=NULL){
                    write(fd_serverToClient,log_buffer,strlen(log_buffer));
                }
                fclose(log_status);}

        else{
            if (strstr(buffer_pedido,"Ended in")!= NULL){
                int pid_a_remover= atoi(&buffer_pedido[0]);
                removePedido(&head,pid_a_remover);
            }
            else{
                int pid_a_adicionar= atoi(&buffer_pedido[0]);
                char* nome_programa = &buffer_pedido[1];
                struct timeval tempo = time_from_buffer(buffer_pedido);
                addNovoPedido(&head,pid_a_adicionar,nome_programa,tempo);
            }
            buffer_pedido[bytes_read] ='\0';
            char buffer_resposta[] = "O servidor já guardou a informação fornecida";
            write(fd_serverToClient, buffer_resposta, sizeof(buffer_resposta));}
    _exit(EXIT_SUCCESS);  
    } }

    close(fd_clientToServer);
    close(fd_serverToClient);
    unlink("/tmp/clientToServer");
    unlink("/tmp/serverToClient");
    return 0;}
