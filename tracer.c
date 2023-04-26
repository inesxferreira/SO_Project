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

void parser(char *str, char ***tokens)
{
    char *token;
    int i;

    // contar o número de tokens
    int *num_tokens = 0;
    for (i = 0; str[i] != '\0'; i++)
    {
        if (str[i] == ' ')
        {
            (*num_tokens)++;
        }
    }
    (*num_tokens)++; // contar o último comando

    // alocar memória para o array de tokens
    *tokens = (char **)malloc((*num_tokens) * sizeof(char *));
    if (*tokens == NULL)
    {
        perror("Falha na alocação de memória");
        exit(EXIT_FAILURE);
    }

    // preencher o array de tokens
    i = 0;
    token = strtok(str, " ");
    while (token != NULL)
    {
        (*tokens)[i] = strdup(token); // duplicar a string
        if ((*tokens)[i] == NULL)
        {
            perror("Falha na alocação de memória");
            exit(EXIT_FAILURE);
        }
        i++;
        token = strtok(NULL, " ");
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
    parser(argv[3], &tokens);
    struct timespec tempo_atual;
    PEDIDO p;
    pid_t pid = getpid();
    p.pid = pid;
    time_t current_time;
    current_time = time(NULL);
    p.initial_timestamp = (long)current_time;
    if (argc < 2)
    { // se o comando tiver errado (tem que ter pelo menos 2 argumentos para o status)
        _exit(EXIT_FAILURE);
    }
    // char *fifo_str [20];
    // sprintf(fifo_str , "tmp/%d", p.pid);
    // p.pipe_id= fifo_str;
    fd_clientToServer = open("/tmp/clientToServer", O_WRONLY);

    if (strcmp(argv[1], "execute") == 0)
    {

        if (strcmp(argv[2], "-u") == 0)
        {
            if ((pid = fork()) == 0)
            {

                if (fd_clientToServer == -1)
                { // não conseguiu abrir
                    perror("Erro ao abrir o fifo do cliente");
                    _exit(EXIT_FAILURE);
                }
                // definir o tempo antes da execução do programa
                clock_gettime(CLOCK_MONOTONIC, &tempo_atual);
                current_time = tempo_atual.tv_sec * 1000 + tempo_atual.tv_nsec / 1000000;
                // manda o pedido ao servidor
                strcpy(p.nome_programa, argv[3]);      // guardamos o nome do programa à struct
                sprintf(buffer_pedido, "%s", argv[3]); // guardamos no buffer o nome do programa
                write(fd_clientToServer, buffer_pedido, strlen(buffer_pedido));
                _exit(1);
            }
            else
            { // se for pai
                int statuss;
                pid_t pid = wait(&statuss);
                printf("o processo [%d] terminou.\n\n", pid);
                // executa o programa
                execvp(tokens[0], argv); // acho que com tokens[0] tou a aceder ao nome do programa
                if (WEXITSTATUS(statuss))
                {
                    // enviar para o servidor
                    clock_gettime(CLOCK_MONOTONIC, &tempo_atual);
                    current_time = tempo_atual.tv_sec * 1000 + tempo_atual.tv_nsec / 1000000;
                    write(fd_clientToServer, buffer_pedido, strlen(buffer_pedido));
                };
            }
        }
        if (strcmp(argv[1], "status") == 0)
        {
            // manda o pedido ao servidor
            // write(fd_clientToServer, status, strlen(status));
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
    }

    // fechar os fifos
    close(fd_clientToServer);
    close(fd_serverToClient);
    return 0;
}