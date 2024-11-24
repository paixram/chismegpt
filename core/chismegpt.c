

//#include <csignal>
#include <stdio.h>
#include <stdlib.h>
#include "core_utils.h"
#include <pthread.h>
#include "../protocol/protocol.h"
#include <signal.h>
#include <unistd.h>


CORE_SETS core;

void handler_exit() {
    printf("[ + ] Se liberaron los recursos con exito\n");
    printf("[ BYE! ] Servidor en decadencia\n");

    destroy_all_resources(&core);
    exit(0);
}

int main() {
    
    // manejar la salida con ctrl c 
    signal(SIGINT, handler_exit);


    // Primero el programa incia la memoria compartida
    core = init_memory();  

    open_server(&core);
    

   
    // Cuando se destruye el server se destruyen todos los recursos
    destroy_all_resources(&core);
    
    return 1;
}
