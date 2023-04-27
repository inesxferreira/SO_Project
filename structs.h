//struct que define os dados de um novo pedido adicionado pelo user
typedef struct pedido
{
    int pid;
    long initial_timestamp; // tempo inicial em segundos
}PEDIDO;

// struct que guarda os pedidos em execução num dado momento
typedef struct pedidos_execucao{
    char nome_programa[512];
    int pid;
    long initial_timestamp; // tempo inicial em segundos
    struct pedidosexecucao *prox;
}PEDIDOSEXECUCAO;


