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


void status_log_file(PEDIDOSEXECUCAO *head){
    PEDIDOSEXECUCAO *atual=head;
    FILE* log_file = fopen(LOG_FILE, "a");
    if (log_file == NULL) {
        perror("Erro ao abrir o arquivo de log");
        exit(EXIT_FAILURE);}
    while(atual!=NULL){
        struct timeval start_time, end_time;
        start_time = atual->initial_timestamp;
        gettimeofday(&end_time, NULL);
        long process_time = (end_time.tv_sec - start_time.tv_sec) * 1000 + 
                   (end_time.tv_usec - start_time.tv_usec) / 1000;
        fprintf(log_file, "%d %s %ld\n", atual->pid, atual->nome_programa, process_time);
        atual=atual->prox;

    }
    fclose(log_file);
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
    while((bytes_read == read(fd_clientToServer, buffer_pedido, sizeof(buffer_pedido)))>0){
        if(bytes_read == -1){
            perror("Erro ao ler o pedido do cliente");
            _exit(EXIT_FAILURE);
        }
        if (strcmp(buffer_pedido,"status") == 0){
            printf("i am in status\n");
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
        buffer_pedido[bytes_read] ='\0';
        printf("i am in execute\n");
        FILE *executefile;
        char buffer_dos_comandos[512];
        char buffer_dos_resultados[512];
        sprintf(buffer_dos_comandos, "%s 2>&1", buffer_pedido);
        executefile = open(buffer_dos_comandos, "r");
        if (executefile == NULL){
            sprintf(buffer_dos_resultados, "Erro ao executar os comandos\n");}
        else{
            memset(buffer_resposta, 0, sizeof(buffer_resposta));
            while(fgets(buffer_dos_resultados, sizeof(buffer_dos_resultados)-1, executefile)!= NULL){
                strcat(buffer_resposta, buffer_dos_resultados); 
                }
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