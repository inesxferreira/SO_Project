//strcut que define os dados de um novo pedido adicionado pelo user
typedef struct pedido
{
    char nome_programa[1024]; //O nome do programa a executar ((p.ex., cat, grep, wc))
    int args; // Os argumentos do programa, caso existam
    int pid;
    char pipe_id; // 
    long initial_timestamp; // tempo inicial em segundos
}PEDIDO;

// struct que guarda os pedidos em execução num dado momento
typedef struct pedidosexecucao{
    char nome_programa[1024];
    int number; //número de entrada 
    int pid;
    struct pedidosexecucão *prox;
}PEDIDOSEXECUCAO;


