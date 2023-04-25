//struct que define os dados de um novo pedido adicionado pelo user
typedef struct pedido
{
    char nome_programa[512]; //O nome do programa a executar ((p.ex., cat, grep, wc)) 
    int pid;
    char pipe_id; 
    long initial_timestamp; // tempo inicial em segundos
}PEDIDO;

// struct que guarda os pedidos em execução num dado momento
typedef struct pedidos_execucao{
    PEDIDO pedido;
    struct pedidosexecucao *prox;
}PEDIDOSEXECUCAO;


