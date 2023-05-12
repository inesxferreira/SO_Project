#include <sys/time.h> // in order to obtain the current time stamp

#define CLIENT_TO_SERVER "tmp/clientToserver"

//struct que define um novo pedido do cliente
typedef struct pedido
{
    int pid;
    struct timeval  initial_timestamp; 
}PEDIDO;

// struct que guarda os pedidos em execução num dado momento
typedef struct pedidos_execucao{
    char nome_programa[512];
    int pid;
    long initial_timestamp; 
} PEDIDOSEXECUCAO;







