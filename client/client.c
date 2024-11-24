


#include "../protocol/protocol.h"
#include "../core/core_utils.h"
#include <bits/getopt_core.h>
#include <semaphore.h>
#include <string.h>
#include <unistd.h>
#include <signal.h>
#include <stdio.h>

//#define SYNC_MEMORY "/sync_memm"

CLIENT_SETS client_sets;

void destroy_client_resources(CLIENT_SETS *client_sets) {
    // Destruir recursos al finalzar
    sem_destroy(&((*client_sets).buf_ref)->session_sem);
    sem_destroy(&((*client_sets).buf_ref)->client_reading);
    sem_destroy(&((*client_sets).buf_ref)->server_reading);
    sem_destroy(&((*client_sets).buf_ref)->check_response);
    munmap(((*client_sets).buf_ref), sizeof(session_packet));
    close((*client_sets).tempid);
    shm_unlink((*client_sets).channel);

}

void handler_exit() {
    printf("[ + ] Se liberaron los recursos con exito\n");
    printf("[ BYE! ] Cliente en decadencia\n");

    destroy_client_resources(&client_sets);
    exit(0);
}



int main(int argc, char **argv) {
    // manejar la salida con ctrl c 
    signal(SIGINT, handler_exit);
    
    // Recibir los parametros por memoria
    char *SYNC_MEMORY, c;
    int user_mode;
    while((c = getopt(argc, argv, "n:p:")) != -1) {
        switch(c) {
            case 'n':
                SYNC_MEMORY = optarg;
                printf("[ + ] Usted inicia con nombre: %s\n", SYNC_MEMORY);
                break;
            case 'p':
                user_mode = *optarg - '0';
                printf("[ + ] Usted inicia con el modo: %d\n", user_mode);
                break;
            default:
                printf("No existe tal argumento\n");

        }
    }

   // Abrir conexion mapeando la memoria
   
    CORE_SETS cs = get_default_core_sets();
    client_sets = open_conn(&cs, SYNC_MEMORY);
    printf("[ + ] Se establecio la conexion, estas en la sesion de sincronizacion\n");

    // TODO: ESTABLECER TIPO DE USUARIOS DEL CLIENTE, ETC. Haciendo login
    
    get_session_id(&client_sets, user_mode);

    // Luego de establecer sesion se tiene que escribir en memoria
    print_user(&cs, client_sets.buf_ref->id);
    
    // Ahora si escribir los mensajes
    write_in(&cs, &client_sets); 
    return 1;
}


