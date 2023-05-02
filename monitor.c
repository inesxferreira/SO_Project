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

#define MAX 1024


//programa servidor com o qual o cliente deve interagir
PEDIDOSEXECUCAO array_processos_running[MAX];
int num_processos_running=0;
int fd_serverToClient;

void remove_from_processos_running(int pid_a_remover){
    int index_to_remove = 0;
    for (int i = 0; i < num_processos_running; i++) {
            if (array_processos_running[i]->pid == pid_a_remover ) {
                index_to_remove = i;
                for(int j=index_to_remove; j<num_processos_running-1;j++)
                    array_processos_running[j]=array_processos_running[j+1];
            }
        }
    num_processos_running--;
    }


struct timeval time_from_buffer(const char* buffer) {
    long int seconds = strtol(buffer + 2, NULL, 10);
    struct timeval tv;
    tv.tv_sec = seconds;
    tv.tv_usec = 0;
    return tv;
}


int main(int argc, char const *argv[]){
    // criação dos fifos
    mkfifo(CLIENT_TO_SERVER,0666);
    mkfifo(SERVER_TO_CLIENT,0666);
    char buffer_pedido[512];
    memset(buffer_pedido, 0, sizeof(buffer_pedido));
    char buffer_resposta[512];
    memset(buffer_resposta, 0, sizeof(buffer_resposta));
    int fd_clientToServer;
    int bytes_read;
    while(fd_clientToServer= open(CLIENT_TO_SERVER,O_RDONLY)){
    fd_serverToClient = open(SERVER_TO_CLIENT, O_WRONLY);
     if(fd_clientToServer == -1){
        perror("Erro ao abrir o pedido do cliente");
         _exit(EXIT_FAILURE);
    }
    bytes_read = read(fd_clientToServer, buffer_pedido, sizeof(buffer_pedido));
    if(bytes_read == -1){
            perror("Erro ao ler o pedido do cliente");
            _exit(EXIT_FAILURE);
        }
    close(fd_clientToServer);
    write(1,buffer_pedido,sizeof(buffer_pedido));
    write(1,"finished\n",strlen("finished\n"));
    if (strstr(buffer_pedido,"status")!= NULL){
                write(1,"i am in status", strlen("i am in status"));
                char sizess[2];
                sprintf(sizess,"Runnig processes %d\n",num_processos_running);
                write(1,sizess,strlen(sizess));
                for (int i=0;i<num_processos_running;i++){
                    write(1,"i am in in", strlen("i am in in"));
                    int current_pid = array_processos_running[i]->pid;
                    char* prog_name = array_processos_running[i]->nome_programa;
                    write(1,prog_name,strlen(prog_name));
                    struct timeval start_time, end_time;
                    start_time = array_processos_running[i]->initial_timestamp;
                    gettimeofday(&end_time, NULL);
                    long process_time = (end_time.tv_sec - start_time.tv_sec) * 1000 + 
                            (end_time.tv_usec - start_time.tv_usec) / 1000;
                    sprintf(buffer_resposta,"%d %s %ld ms\n",current_pid,prog_name,process_time);
                    write(1,buffer_resposta,sizeof(buffer_resposta));
                    write(fd_serverToClient,buffer_resposta,sizeof(buffer_resposta));
                    close(fd_serverToClient);
        }
                }
    else if (strstr(buffer_pedido,"Ended in")!= NULL){
                int pid_a_remover= atoi(&buffer_pedido[0]);
                remove_from_processos_running(pid_a_remover);
                char buffer_resposta[] = "O servidor já guardou a informação fornecida\n";
                write(fd_serverToClient, buffer_resposta, sizeof(buffer_resposta));
                close(fd_serverToClient);
            }
    else{
                write(1,"i am in add", strlen("i am in add"));
                PEDIDOSEXECUCAO p = malloc(sizeof(struct pedidos_execucao));
                p->pid = atoi(&buffer_pedido[0]);
                p->nome_programa = malloc(strlen(&buffer_pedido[1]) + 1); // +1 for null terminator
                strcpy(p->nome_programa, &buffer_pedido[1]); // copy string to 
                p->initial_timestamp = time_from_buffer(buffer_pedido);
                array_processos_running[num_processos_running++]=p; 
                char buffer_resposta[] = "O servidor já guardou a informação fornecida\n";
                write(fd_serverToClient, buffer_resposta, sizeof(buffer_resposta));
                close(fd_serverToClient);
                }}
    return 0;

            }
            
