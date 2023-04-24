#include <stdio.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <stdlib.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include "structs.h"
#include <sys/time.h> // in order to obtain the current time stamp
#define PIPE_NAME "fifo"
#define LOG_FILE "log.txt"

//programa servidor com o qual o cliente deve interagir

// retorna o tempo em segundos decorrido
long getFinalTimeStamp(PEDIDO p){
    time_t current_time;
    current_time=time(NULL);
    long now = (long) current_time;
    long before = p.initial_timestamp;
    long final_time = now-before;
    return final_time;
}

void log_file(PEDIDO p){
    FILE* log_file = fopen(LOG_FILE, "a");
    if (log_file == NULL) {
        perror("Erro ao abrir o arquivo de log");
        exit(EXIT_FAILURE);
    }
    fprintf(log_file, "%d %s %ld\n", p.pid, p.nome_programa, p.initial_timestamp);
    fclose(log_file);
}


int main(int argc, char const *argv[])
{
    int fd_server_pipe, log_fd;
    char buffer[1024];
    char *serverfifo = "/tmp/serverfifo";
    // Verifica se o fifo foi criado com sucesso
    if (mkfifo(PIPE_NAME, 0666) == -1)
    {
        perror("Erro ao criar o fifo");
        exit(1);
    }
    // Abre o pipe com nome para leitura
    if ((fd_server_pipe = open(PIPE_NAME, O_RDONLY)) == -1)
    {
        perror("Erro ao abrir o fifo");
        exit(1);
    }
    // Abre o arquivo de log para escrita
    if ((log_fd = open(LOG_FILE, O_WRONLY | O_CREAT | O_APPEND, 0666)) == -1)
    {
        perror("Erro ao abrir o arquivo de log");
        exit(1);
    }

    while (1)
    {
         // Aguardar por pedidos dos clientes (ler informações via pipe, por exemplo)

        // Processar os pedidos concorrentemente

        // Verificar se um programa foi concluído e atualizar as estatísticas

        // Responder aos pedidos dos clientes (enviar informações via pipe, por exemplo)
    
        int bytes_read= read(fd_server_pipe, buffer, sizeof(buffer));
        if (bytes_read == -1){
            perror("Erro ao ler");
            exit(1);
        }
        if (bytes_read == 0){
            break; // Não tem nada a ler, termina 
        }
        fwrite(buffer, sizeof(char), bytes_read, log_fd);
    }
    close(fd_server_pipe);
    fclose(log_fd);
    unlink("serverfifo");
    
    return 0;

       
}

