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

// programa servidor com o qual o cliente deve interagir
PEDIDOSEXECUCAO *array_processos_running[MAX];
int num_processos_running = 0;

void remove_from_processos_running(int pid_a_remover)
{
    int index_to_remove = 0;
    for (int i = 0; i < num_processos_running; i++)
    {
        if (array_processos_running[i]->pid == pid_a_remover)
        {
            index_to_remove = i;
            for (int j = index_to_remove; j < num_processos_running; j++)
                array_processos_running[j] = array_processos_running[j + 1];
        }
    }
    num_processos_running--;
}

int main(int argc, char const *argv[])
{
    char *log_file_path = malloc(10);
    // criação dos fifos
    mkfifo(CLIENT_TO_SERVER, 0666);
    char buffer_pedido[512];
    memset(buffer_pedido, 0, sizeof(buffer_pedido));
    char buffer_resposta[1024];
    memset(buffer_resposta, 0, sizeof(buffer_resposta));
    int fd_clientToServer;
    int fd_serverToClient;

    write(1, "entrei na main\n", strlen("entrei na main\n"));

    int bytes_read;
    while (fd_clientToServer = open(CLIENT_TO_SERVER, O_RDONLY))
    {
        write(1, "recebi pedido\n", strlen("recebi pedido\n"));

        if (fd_clientToServer == -1)
        {
            perror("Erro ao abrir o pedido do cliente");
            _exit(EXIT_FAILURE);
        }
        bytes_read = read(fd_clientToServer, buffer_pedido, sizeof(buffer_pedido));
        if (bytes_read == -1)
        {
            perror("Erro ao ler o pedido do cliente");
            _exit(EXIT_FAILURE);
        }
        close(fd_clientToServer);
        write(1, buffer_pedido, sizeof(buffer_pedido));

        if (strstr(buffer_pedido, "status") != NULL)
        {
            char *token = strtok(buffer_pedido, " ");
            int pid_ = 0;
            char *pipe_name = malloc(10);
            sprintf(pipe_name, token, strlen(token));
            fd_serverToClient = open(pipe_name, O_WRONLY);
            if (num_processos_running == 0)
            {
                write(fd_serverToClient, "Não há processos a correr\n", strlen("Não há processos a correr\n"));
            }
            write(1, "\nin of status\n", strlen("\nin of status\n"));
            for (int i = 0; i < num_processos_running; i++)
            {
                if (array_processos_running[i]->pid != 0)
                {
                    char line_status[50];
                    for (int i = 0; i < sizeof(line_status); i++)
                    {
                        line_status[i] = 0;
                    }
                    memset(line_status, 0, sizeof(line_status));
                    int current_pid = array_processos_running[i]->pid;
                    char *prog_name = array_processos_running[i]->nome_programa;
                    struct timeval end_time;
                    long start_time;
                    start_time = array_processos_running[i]->initial_timestamp;
                    gettimeofday(&end_time, NULL);
                    long process_time = (end_time.tv_sec * 1000 + end_time.tv_usec / 1000) - start_time;
                    snprintf(line_status, sizeof(line_status), "%d %s %ld ms\n", current_pid, prog_name, process_time);
                    strcat(buffer_resposta, line_status);
                }
            }

            write(fd_serverToClient, buffer_resposta, sizeof(buffer_resposta));
            close(fd_serverToClient);
            write(1, "\nout of status\n", strlen("\nout of status\n"));
            for (int i = 0; i < sizeof(buffer_resposta); i++)
            {
                buffer_resposta[i] = 0;
            }
        }

        if (strstr(buffer_pedido, "Add") != NULL) // se um novo pedido chegar
        {
            write(1, "entrou added\n", strlen("entrou added\n"));
            int fd_serverToClient;
            char *token = strtok(buffer_pedido, ";"); // é o Add
            PEDIDOSEXECUCAO *p = malloc(sizeof(PEDIDOSEXECUCAO));
            token = strtok(NULL, ";"); // é o pid
            if (token != NULL)
            {
                int pid = atoi(token);
                p->pid = pid;
            }
            token = strtok(NULL, ";"); // é o nome do programa
            if (token != NULL)
            {
                strcpy(p->nome_programa, token);
            }

            token = strtok(NULL, ";"); // é o tempo
            if (token != NULL)
            {
                char *endptr;
                long tempo_inicial = strtol(token, &endptr, 10);
                p->initial_timestamp = tempo_inicial;
            }
            array_processos_running[num_processos_running] = p; // adicionamos o pedido à lista dos pedidos em execução
            num_processos_running = num_processos_running + 1;  // incrementamos o número de pedidos em execução
            write(1, "added\n", strlen("added\n"));
        }
        if (strstr(buffer_pedido, "Ended in") != NULL) // se o cliente informar que terminou um pedido
        {
            write(1, "entrou removed\n", strlen("entrou removed\n"));
            char *token = strtok(buffer_pedido, " "); // é o pid
            char nome_programa_fin[512];
            long tempo_inicial_programa_fin;
            int pid_a_remover = 0;
            int index_to_remove = -1;
            if (token != NULL)
            {
                pid_a_remover = atoi(token);
            }
            for (int i = 0; i < num_processos_running; i++)
            {
                if (array_processos_running[i]->pid == pid_a_remover)
                {

                    index_to_remove = i;
                    strcpy(nome_programa_fin, array_processos_running[i]->nome_programa);
                    tempo_inicial_programa_fin = array_processos_running[i]->initial_timestamp;
                    break;
                }
            }
            if (index_to_remove >= 0)
            {

                for (int j = index_to_remove; j < num_processos_running - 1; j++)
                {
                    array_processos_running[j] = array_processos_running[j + 1];
                }
                num_processos_running--;
            }
            if (argc == 2)
            {
                log_file_path = argv[1];
                char *log_file_pid_name = malloc(10);
                sprintf(log_file_pid_name, "%s/%d.txt", log_file_path, pid_a_remover);
                int log_fd = open(log_file_pid_name, O_CREAT | O_WRONLY, S_IRUSR | S_IWUSR);
                if (log_fd == -1)
                {
                    return 1;
                }
                struct timeval end_time;
                long start_time;
                start_time = tempo_inicial_programa_fin;
                char buffer_log[512];
                memset(buffer_log, 0, sizeof(buffer_log));
                gettimeofday(&end_time, NULL);
                long process_time = (end_time.tv_sec * 1000 + end_time.tv_usec / 1000) - start_time;
                snprintf(buffer_log, sizeof(buffer_log), "%d %s %ld ms\n", pid_a_remover, nome_programa_fin, process_time);
                write(log_fd, buffer_log, strlen(buffer_log));
                num_processos_running = num_processos_running - 1;
                write(1, "\nremoved\n", strlen("\nremoved\n"));
                close(log_fd);
            }
            else
            {
                struct timeval end_time;
                long start_time;
                start_time = tempo_inicial_programa_fin;
                char buffer_log[512];
                memset(buffer_log, 0, sizeof(buffer_log));
                gettimeofday(&end_time, NULL);
                long process_time = (end_time.tv_sec * 1000 + end_time.tv_usec / 1000) - start_time;
                snprintf(buffer_log, sizeof(buffer_log), "%d %s %ld ms\n", pid_a_remover, nome_programa_fin, process_time);
                num_processos_running = num_processos_running - 1;
                write(1, "\nremoved\n", strlen("\nremoved\n"));
            }
        }
    }
    return 0;
}
