#include <stdio.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <stdlib.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <string.h>
#include "structs.h"
#include "time.h"
#include <sys/time.h> // in order to obtain the current time stamp

// cliente
int main(int argc, char const *argv[])
{
    // Verificar se foi passado o nome do programa a executar
    char buffer_pedido[512];
    memset(buffer_pedido, 0, sizeof(buffer_pedido));
    char buffer_resposta[512];
    memset(buffer_resposta, 0, sizeof(buffer_resposta));
    int fd_serverToClient;
    int fd_clientToServer;
    const char *status = "status";
    int bytes_read;
    char **tokens;
    PEDIDO pdido;
    pid_t pid = getpid();
    pdido.pid= pid;
    //clock_t start_time, end_time;
    //double time_taken;
    //start_time=clock();
    struct timeval start_time, end_time;
    long process_time; 
    gettimeofday(&start_time, NULL);
    pdido.initial_timestamp =  start_time;
    
    if (argc < 2)
    { // se o comando tiver errado (tem que ter pelo menos 2 argumentos para o status)
        _exit(EXIT_FAILURE);
    }
    if (strcmp(argv[1], "execute") == 0){
        if (strcmp(argv[2], "-u") == 0){
                char *args[256];
                int i = 0;
                char *token = strtok(argv[3], " ");
                while (token != NULL) {
                    args[i++] = token;
                    token = strtok(NULL, " ");
                }
                args[i] = NULL;
            if ((pid = fork()) == 0) {
                //abre o fd do cliente
                fd_clientToServer = open(CLIENT_TO_SERVER, O_WRONLY);
                if (fd_clientToServer == -1)
                { // não conseguiu abrir
                    perror("Erro ao abrir o fifo do cliente");
                    _exit(EXIT_FAILURE);
                }
                sprintf(buffer_pedido, "Add %d %s %ld ms\n", pdido.pid,argv[3],start_time.tv_sec*1000+start_time.tv_usec/1000); // guardamos no buffer o nome do programa
                //o cliente envia para o servidor a info do pedido a executar
                write(fd_clientToServer, buffer_pedido, strlen(buffer_pedido));
                
                char pid_string [30];
                sprintf(pid_string,"Running PID %d\n",pdido.pid);
                
                //o cliente informa o utilizador via standard output do pedido a executar
                write(STDOUT_FILENO,pid_string,strlen(pid_string));
                // o cliente executa o programa
                execvp(args[0], args);
                close(fd_clientToServer);

                fd_serverToClient = open(SERVER_TO_CLIENT,O_RDONLY);
                if (fd_serverToClient == -1){
                // não conseguiu abrir
                    perror("Erro ao abrir a resposta do servidor (1)");
                    _exit(EXIT_FAILURE);
                }
                int bytes_read=0;
                char buffer[1024];
                while((bytes_read = read(fd_serverToClient,buffer,1024))>0){
                int flag=1;
                while(flag){
                    if(strcmp(buffer,"O servidor já adicionou a informação fornecida\n") == 0){
                        flag=0;
                    }  
                }
                }
                close(fd_serverToClient);
                return 0;}
           
           else { // se for pai
                sleep(20);
                fd_clientToServer = open(CLIENT_TO_SERVER, O_WRONLY);
                if (fd_clientToServer == -1)
                { // não conseguiu abrir
                    perror("Erro ao abrir o fifo do cliente");
                    _exit(EXIT_FAILURE);
                }
                int statuss;
                waitpid(pid,&statuss,0);
                gettimeofday(&end_time, NULL);
                process_time = (end_time.tv_sec - start_time.tv_sec) * 1000 + 
                   (end_time.tv_usec - start_time.tv_usec) / 1000;
                //o cliente informa o servidor do pedido executado
                sprintf(buffer_pedido, "%d Ended in %ld ms\n", pdido.pid,process_time); // guardamos no buffer o nome do programa
                write(fd_clientToServer, buffer_pedido, strlen(buffer_pedido));
                char final_time[30];
                sprintf(final_time,"Ended in %ld ms\n", process_time);
                //o cliente informa o utilizador via standard output, do tempo de execução (em milisegundos) utilizado pelo programa
                write(STDOUT_FILENO,final_time,strlen(final_time));
                close(fd_clientToServer);

                fd_serverToClient = open(SERVER_TO_CLIENT,O_RDONLY);
                if (fd_serverToClient == -1){
                // não conseguiu abrir
                    perror("Erro ao abrir a resposta do servidor (1)");
                    _exit(EXIT_FAILURE);
                }
                int bytes_read=0;
                char buffer[1024];
                while((bytes_read = read(fd_serverToClient,buffer,1024))>0){
                int flag=1;
                while(flag){
                    if(strcmp(buffer,"O servidor já removeu a informação fornecida\n") == 0){
                        flag=0;
                    }  
               }
                }
                close(fd_serverToClient);
                return 0;}
            
        }}
    else if (strcmp(argv[1], "status") == 0){
             fd_clientToServer = open(CLIENT_TO_SERVER, O_WRONLY);
             if (fd_clientToServer == -1)
                { // não conseguiu abrir
                    perror("Erro ao abrir o fifo do cliente");
                    _exit(EXIT_FAILURE);
                }
             write(fd_clientToServer,"status", strlen("status"));
             close(fd_clientToServer);
             fd_serverToClient = open(SERVER_TO_CLIENT,O_RDONLY);
            if (fd_serverToClient == -1){
                // não conseguiu abrir
                    perror("Erro ao abrir a resposta do servidor (1)");
                    _exit(EXIT_FAILURE);
                }
                int bytes_read=0;
                char buffer[1024];
                while((bytes_read = read(fd_serverToClient,buffer,1024))>0){
                    write(1,buffer,sizeof(buffer));
                    }  
                close(fd_serverToClient);
                return 0;
        }

    return 0;
}