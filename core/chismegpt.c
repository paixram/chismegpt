

#include <stdio.h>
#include <stdlib.h>
#include "core_utils.h"
#include <pthread.h>
#include "../protocol/protocol.h"

int main() {
    
    // Primero el programa incia la memoria compartida
    CORE_SETS core = init_memory(); 
    
    open_server(&core);

    // Cuando se destruye el server se destruyen todos los recursos
    destroy_all_resources(&core);
    
    return 1;
}
