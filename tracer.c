#include <stdio.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <stdlib.h>

// cliente

int main(int argc, char *argv[])
{
    // Verificar se foi passado o nome do programa a executar
    if (argc < 2)
    {
        printf("Uso: %s <nome_do_programa> [argumentos...]\n", argv[0]);
        exit(1);
    }

    // Obter o nome do programa e os argumentos
    char *program_name = argv[1];
    char **program_args = &argv[1];

    // Fork para criar um novo processo para executar o programa
    pid_t pid = fork();

    if (pid < 0)
    {
        // Erro ao criar o processo filho
        perror("Erro ao criar processo filho");
        exit(1);
    }
    else if (pid == 0)
    {
        // Processo filho - executar o programa
        execvp(program_name, program_args);
        // Se execvp retornar, houve um erro na execução do programa
        perror("Erro ao executar programa");
        exit(1);
    }
    else
    {
        // Processo pai - notificar o servidor e aguardar a conclusão do programa
        printf("Programa em execução com PID: %d\n", pid);

        // Notificar o servidor do início da execução do programa (enviar informações via pipe, por exemplo)

        // Aguardar a conclusão do programa
        int status;
        waitpid(pid, &status, 0);

        // Notificar o servidor da conclusão do programa (enviar informações via pipe, por exemplo)

        // Calcular o tempo de execução do programa
        double runtime = (double)(clock() - start_time) / CLOCKS_PER_SEC * 1000;
        printf("Tempo de execução: %.2f ms\n", runtime)
    }

    return 0;
}
