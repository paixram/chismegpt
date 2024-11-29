

//#include <csignal>
#include <stdio.h>
#include <stdlib.h>
#include "core_utils.h"
#include <pthread.h>
#include "../protocol/protocol.h"
#include <signal.h>
#include <unistd.h>


CORE_SETS core;

int max_concurrent_messages = 0;

void handler_exit() {
    printf("[ + ] Se liberaron los recursos con exito\n");
    printf("[ BYE! ] Servidor cerrado\n");

    destroy_all_resources(&core);
    exit(0);
}

int main(int argc, char **argv) {
    // Manejar la salida con Ctrl+C
    signal(SIGINT, handler_exit);

    // Obtener y verificar el parámetro del número máximo de mensajes concurrentes
    if (argc < 2) {
        printf("Uso: %s <número_máximo_mensajes_concurrentes>\n", argv[0]);
        exit(EXIT_FAILURE);
    }
    max_concurrent_messages = atoi(argv[1]);
    if (max_concurrent_messages <= 0) {
        printf("El número máximo de mensajes debe ser un entero positivo.\n");
        exit(EXIT_FAILURE);
    }

    // Inicializar la memoria compartida del servidor
    core = init_memory();

    // Pasar el valor a la estructura core
    core.max_concurrent_messages = max_concurrent_messages;

    // **Aquí se llama a initialize_protocol**
    initialize_protocol(&core);

    // Iniciar el servidor
    open_server(&core);

    // Destruir recursos al finalizar
    destroy_all_resources(&core);

    return 0;
}
