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

//programa servidor com o qual o cliente deve interagir

/**
 * para já temos 2 tipos de operações:
 * execute -u e status, caso o cliente introduza algo diferente deve dar erro (type-1)
*/
int operationType(char arg1[]){
    int type;
    if (strcmp(arg1,"execute -u")==0)
        type=1;
    if (strcmp(arg1,"status")==0)
        type=2;
    return type-1; 
}
// retorna o tempo em segundos decorrido
long getFinalTimeStamp(PEDIDO p){
    time_t current_time;
    current_time=time(NULL);
    long now = (long) current_time;
    long before = p.initial_timestamp;
    long final_time = now-before;
    return final_time;
}

int main(int argc, char const *argv[])
{
    int fd;
    char *serverfifo = "/tmp/serverfifo";
    mkfifo(serverfifo,0644);
    fd  = open(serverfifo,O_RDONLY);
    while(1){

    }
    return 0;
}
