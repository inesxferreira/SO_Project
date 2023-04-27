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

void addNovoPedido(PEDIDOSEXECUCAO **head, int pid, char nome_programa[],long time_initial){
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

int main(int argc, char *argv[])
{
    // Verificar se foi passado o nome do programa a executar
    char buffer_pedido[512];
    char buffer_resposta[512];
    int fd_serverToClient;
    int fd_clientToServer;
    const char *status = "status";
    int bytes_read;
    char **tokens;
    PEDIDO pdido;
    pid_t pid = getpid();
    pdido.pid= pid;
    clock_t start_time, end_time;
    double time_taken;
    start_time=clock();
    pdido.initial_timestamp = (double)start_time/CLOCKS_PER_SEC*1000;
    if (argc < 2)
    { // se o comando tiver errado (tem que ter pelo menos 2 argumentos para o status)
        _exit(EXIT_FAILURE);
    }
    fd_clientToServer = open("/tmp/clientToServer", O_WRONLY);
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
                if (fd_clientToServer == -1)
                { // não conseguiu abrir
                    perror("Erro ao abrir o fifo do cliente");
                    _exit(EXIT_FAILURE);
                }
                // definir o tempo antes da execução do programa
                PEDIDOSEXECUCAO p;
                PEDIDOSEXECUCAO* head = NULL;
                p.pid = pdido.pid;
                // o cliente informa o servidor do pedido a executar 
                addNovoPedido(&head, pid,argv[3], (double)start_time/CLOCKS_PER_SEC*1000);
                sprintf(buffer_pedido, "%d\n%s\n%ld\n", pdido.pid,argv[3],pdido.initial_timestamp); // guardamos no buffer o nome do programa
                write(fd_clientToServer, buffer_pedido, strlen(buffer_pedido));
                char pid_string [30];
                sprintf(pid_string,"Running PID %d\n",pdido.pid);
                //o cliente informa o utilizador via standard output do pedido a executar
                write(STDOUT_FILENO,pid_string,strlen(pid_string));
                // o cliente executa o programa
                 execvp(args[0], args);
                return 1;
                }
           else { // se for pai
                int statuss;
                waitpid(pid,&statuss,0);
                end_time= clock();
                time_taken = ((double)(end_time - start_time)) / CLOCKS_PER_SEC * 1000;
                //o cliente informa o servidor do pedido executado
                sprintf(buffer_pedido, "%d\n%.2f\n", pid,time_taken); // guardamos no buffer o nome do programa
                write(fd_clientToServer, buffer_pedido, strlen(buffer_pedido));
                char final_time[30];
                sprintf(final_time,"Ended in %.2f ms\n", time_taken);
                //o cliente informa o utilizador via standard output, do tempo de execução (em milisegundos) utilizado pelo programa
                write(STDOUT_FILENO,final_time,strlen(final_time));
        
                }
            }
        }
    else if (strcmp(argv[1], "status") == 0){
            // manda o pedido ao servidor
             write(fd_clientToServer, status, strlen(status));
        }

        // recebe a resposta do servdor

        fd_serverToClient = open("/tmp/serverToClient", O_RDONLY);
        if (fd_serverToClient == -1)
        {
            perror("Erro ao abrir o fifo do servidor");
            _exit(EXIT_FAILURE);
        }
        bytes_read = read(fd_serverToClient, buffer_resposta, sizeof(buffer_resposta) - 1);
        if (bytes_read == -1)
        {
            perror("Erro ao ler o fifo do servidor");
            _exit(EXIT_FAILURE);
        }
        buffer_resposta[bytes_read] = '\0';
        // printa no terminal
        write(STDOUT_FILENO, buffer_resposta, bytes_read);
    

    // fechar os fifos
    close(fd_clientToServer);
    close(fd_serverToClient);
    return 0;
}