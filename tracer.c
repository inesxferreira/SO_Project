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
#define SIZE_OF_BUFFER 256

int main(int argc, char const *argv[])
{
    // Verificar se foi passado o nome do programa a executar
    char buffer_pedido[512];
    memset(buffer_pedido, 0, sizeof(buffer_pedido));
    char buffer_resposta[512];
    memset(buffer_resposta, 0, sizeof(buffer_resposta));

    int fd_serverToClient;
    int fd_clientToServer;

    PEDIDO pdido;
    pid_t pid = getpid();
    pdido.pid = pid;

    char *pipe_name = malloc(10);
    sprintf(pipe_name, "tmp/%d", pdido.pid);
    mkfifo(pipe_name, 0666);

    struct timeval start_time, end_time;
    long process_time;
    gettimeofday(&start_time, NULL);
    pdido.initial_timestamp = start_time;

    if (argc < 2)
    { // se o comando tiver errado (tem que ter pelo menos 2 argumentos para o status)
        _exit(EXIT_FAILURE);
    }
    if (strcmp(argv[1], "execute") == 0)
    {
        if (strcmp(argv[2], "-u") == 0)
        {
            char *args[256];
            int i = 0;
            char *token = strtok(argv[3], " ");
            while (token != NULL)
            {
                args[i++] = token;
                token = strtok(NULL, " ");
            }
            args[i] = NULL;

            if ((pid = fork()) == 0)
            {
                char buffer_pedido[512];
                memset(buffer_pedido, 0, sizeof(buffer_pedido));
                // abre o fd do cliente
                fd_clientToServer = open(CLIENT_TO_SERVER, O_WRONLY);
                if (fd_clientToServer == -1)
                { // não conseguiu abrir
                    perror("Erro ao abrir o fifo do cliente");
                    _exit(EXIT_FAILURE);
                }
                sprintf(buffer_pedido, "Add;%d;%s;%ld;\n", pdido.pid, args[0], start_time.tv_sec * 1000 + start_time.tv_usec / 1000); // guardamos no buffer o nome do programa
                // o cliente envia para o servidor a info do pedido a executar
                write(fd_clientToServer, buffer_pedido, strlen(buffer_pedido));

                // o cliente informa o utilizador via standard output do pedido a executar
                char pid_string[30];
                sprintf(pid_string, "Running PID %d\n", pdido.pid);
                write(STDOUT_FILENO, pid_string, strlen(pid_string));
                // o cliente executa o programa
                execvp(args[0], args);
                close(fd_clientToServer);
                return 0;
            }

            else
            { // se for pai
                char buffer_pedido[512];
                memset(buffer_pedido, 0, sizeof(buffer_pedido));
                sleep(20);
                fd_clientToServer = open(CLIENT_TO_SERVER, O_WRONLY);
                if (fd_clientToServer == -1)
                { // não conseguiu abrir
                    perror("Erro ao abrir o fifo do cliente");
                    _exit(EXIT_FAILURE);
                }
                int statuss;
                waitpid(pid, &statuss, 0);
                gettimeofday(&end_time, NULL);
                long time_at_the_end= end_time.tv_sec*1000+end_time.tv_usec/1000;
                process_time = (end_time.tv_sec - start_time.tv_sec) * 1000 +
                               (end_time.tv_usec - start_time.tv_usec) / 1000;

                // o cliente informa o servidor do pedido executado
                sprintf(buffer_pedido, "%d Ended in %ld \n", pdido.pid, time_at_the_end); // guardamos no buffer o nome do programa
                write(fd_clientToServer, buffer_pedido, strlen(buffer_pedido));
                char final_time[30];
                sprintf(final_time, "Ended in %ld ms\n", process_time);

                // o cliente informa o utilizador via standard output, do tempo de execução (em milisegundos) utilizado pelo programa
                write(STDOUT_FILENO, final_time, strlen(final_time));
                close(fd_clientToServer);
                return 0;
            }
        }
        else if (strcmp(argv[2], "-p") == 0)
        {
            //"prog-a arg-1 (...) arg-n | prog-b arg-1 (...) arg-n | prog-c arg-1 (...) arg-n"
            char *token = strtok(argv[3], "|");

            int p = 0;
            char **progs = malloc(256 * sizeof(char *));  // lista de string
            char ***args = malloc(256 * sizeof(char **)); // lista de listas de strings
            int j;
            while (token != NULL)
            {
                progs[p] = token;
                p++;
                // Obtém a próxima substring separada por '|'
                token = strtok(NULL, "|");
            }
            for (int i = 0; i < p; i++)
            {
                j = 0;
                char *arg = strtok(progs[i], " ");
                args[i] = malloc(256 * sizeof(char *));

                while (arg != NULL)
                {
                    args[i][j] = arg;
                    j++;
                    arg = strtok(NULL, " ");
                }
            }

            int pipo[p - 1][2];
            char buffer_pedido[512];
            memset(buffer_pedido, 0, sizeof(buffer_pedido));
            fd_clientToServer = open(CLIENT_TO_SERVER, O_WRONLY);
            if (fd_clientToServer == -1)
            { // não conseguiu abrir
                perror("Erro ao abrir o fifo do cliente");
                _exit(EXIT_FAILURE);
            }
            sprintf(buffer_pedido, "Add;%d;%s;%ld;\n", pdido.pid, args[0][0], start_time.tv_sec * 1000 + start_time.tv_usec / 1000); // guardamos no buffer o nome do programa
            // o cliente envia para o servidor a info do pedido a executar
            write(fd_clientToServer, buffer_pedido, strlen(buffer_pedido));
            char pid_string[30];
            sprintf(pid_string, "Running PID %d\n", pdido.pid);
            // o cliente notifica o utilizador do pedido a executar
            write(STDOUT_FILENO, pid_string, strlen(pid_string));
            close(fd_clientToServer);

            for (int n = 0; n < p; n++)
            {

                if (n == 0)
                {
                    pipe(pipo[0]);

                    if (fork == 0)
                    {                        // preparar ambiente para o programa ser executado
                        dup2(pipo[0][n], 1); // processo filho redireciona a saída padrão para o lado de escrita do pipe,
                        close(pipo[0][0]);
                        close(pipo[0][1]);
                        // o cliente executa o programa
                        execvp(args[0][0], args[0]);
                        perror("Exec failed");
                        _exit(1);
                    }
                    close(pipo[0][1]);
                }
                else if (n < p - 1)
                {

                    pipe(pipo[n]);
                    if (fork() == 0)
                    {
                        // Redireciona a entrada padrão do processo filho para a
                        //  extremidade de leitura do pipe da etapa anterior.
                        dup2(pipo[n - 1][0], 0);
                        // o de escrita anterior está fechado
                        close(pipo[n - 1][0]);

                        dup2(pipo[n][1], 1);
                        close(pipo[n][0]);
                        close(pipo[n][1]);

                        // enviar programa para monitor
                        execvp(args[n][0], args[n]);

                        perror("Exec failed");
                        _exit(1);
                    }
                    close(pipo[n - 1][0]);
                    close(pipo[n][1]);
                }
                else
                {
                    if (fork() == 0)
                    {

                        dup2(pipo[n - 1][0], 0);
                        close(pipo[n - 1][1]);
                        close(pipo[n - 1][0]);

                        // enviar programa para
                        execvp(args[n][0], args[n]);

                        perror("Exec failed");
                        _exit(1);
                    }
                    close(pipo[n - 1][0]);
                }
            }
            for (int n = 0; n < p; n++)
            {
                wait(NULL);
            }
            fd_clientToServer = open(CLIENT_TO_SERVER, O_WRONLY);
            if (fd_clientToServer == -1)
            { // não conseguiu abrir
                perror("Erro ao abrir o fifo do cliente");
                _exit(EXIT_FAILURE);
            }
            int statuss;
            waitpid(pid, &statuss, 0);
            gettimeofday(&end_time, NULL);
            process_time = (end_time.tv_sec - start_time.tv_sec) * 1000 +
                           (end_time.tv_usec - start_time.tv_usec) / 1000;

            // o cliente informa o servidor do pedido executado
            sprintf(buffer_pedido, "%d Ended in %ld \n", pdido.pid, process_time); // guardamos no buffer o nome do programa
            write(fd_clientToServer, buffer_pedido, strlen(buffer_pedido));
            char final_time[30];
            sprintf(final_time, "Ended in %ld ms\n", process_time);

            // o cliente informa o utilizador via standard output, do tempo de execução (em milisegundos) utilizado pelo programa
            write(STDOUT_FILENO, final_time, strlen(final_time));
            close(fd_clientToServer);
        }
    }
    else if (strcmp(argv[1], "status") == 0)
    {
        char buffer_pedido[512];
        memset(buffer_pedido, 0, sizeof(buffer_pedido));
        fd_clientToServer = open(CLIENT_TO_SERVER, O_WRONLY);
        if (fd_clientToServer == -1)
        { // não conseguiu abrir
            perror("Erro ao abrir o fifo do cliente");
            _exit(EXIT_FAILURE);
        }
        sprintf(buffer_pedido, "%s status\n", pipe_name);
        write(fd_clientToServer, buffer_pedido, strlen(buffer_pedido));
        close(fd_clientToServer);
        fd_serverToClient = open(pipe_name, O_RDONLY);
        if (fd_serverToClient == -1)
        {
            // não conseguiu abrir
            perror("Erro ao abrir a resposta do servidor (1)");
            _exit(EXIT_FAILURE);
        }
        int bytes_read = 0;
        char b;
        char *buffer = malloc(SIZE_OF_BUFFER * sizeof(char));
        int size_bytes_read = 0;
        while (bytes_read = read(fd_serverToClient, &b, 1) && size_bytes_read < SIZE_OF_BUFFER - 1)
        {
            if (b != '\0')
            {
                buffer[size_bytes_read++] = b;
            }
        }

        write(1, buffer, size_bytes_read);
        close(fd_serverToClient);
        return 0;
    }
}