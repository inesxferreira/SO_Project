#include <sys/time.h> // in order to obtain the current time stamp

#define CLIENT_TO_SERVER "tmp/clientToserver"
#define SERVER_TO_CLIENT "tmp/serverToclient"

//struct que define um no
typedef struct pedido
{
    int pid;
    struct timeval  initial_timestamp; // tempo inicial em segundos
}PEDIDO;

// struct que guarda os pedidos em execução num dado momento
typedef struct pedidos_execucao{
    char *nome_programa;
    int pid;
    struct timeval initial_timestamp; // tempo inicial em segundos
} *PEDIDOSEXECUCAO;







