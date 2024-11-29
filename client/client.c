
#include "../protocol/protocol.h"
#include "../core/core_utils.h"
#include <getopt.h>
#include <semaphore.h>
#include <string.h>
#include <unistd.h>
#include <signal.h>
#include <stdio.h>
#include <pthread.h>
#include <stdlib.h>

CLIENT_SETS client_sets;

void destroy_client_resources(CLIENT_SETS *client_sets) {
    // Destruir recursos al finalizar
    sem_destroy(&((*client_sets).buf_ref)->session_sem);
    sem_destroy(&((*client_sets).buf_ref)->client_reading);
    sem_destroy(&((*client_sets).buf_ref)->server_reading);
    sem_destroy(&((*client_sets).buf_ref)->check_response);
    munmap(((*client_sets).buf_ref), sizeof(session_packet));
    close((*client_sets).tempid);
    shm_unlink((*client_sets).channel);
}

void handler_exit() {
    printf("[ + ] Se liberaron los recursos con éxito\n");
    printf("[ BYE! ] Cliente en decadencia\n");

    destroy_client_resources(&client_sets);
    exit(0);
}

void change_user_type(CLIENT_SETS *client_sets) {
    char input_buffer[10];
    int new_type;
    printf("Ingrese el nuevo tipo de usuario (0 para pre-pago, 1 para pos-pago): ");

    // Usar fgets para leer la entrada del usuario
    fgets(input_buffer, sizeof(input_buffer), stdin);
    // Remover el carácter de nueva línea si está presente
    input_buffer[strcspn(input_buffer, "\n")] = '\0';
    // Convertir la entrada a entero y validar
    if (sscanf(input_buffer, "%d", &new_type) == 1 && (new_type == PRE_PAGO || new_type == POS_PAGO)) {
        // Enviar solicitud al servidor
        sem_wait(&(client_sets->buf_ref->session_sem));
        client_sets->buf_ref->status = get_char_by_code(CT);
        client_sets->buf_ref->buffer[0] = new_type; 
        sem_post(&(client_sets->buf_ref->session_sem));
        sem_post(&(client_sets->buf_ref->client_reading));

        // Esperar confirmación del servidor
        wait_for_read(client_sets->buf_ref);

        // Actualizar el modo de usuario en el cliente
        client_sets->user_mode = new_type;

        printf("Tipo de usuario actualizado a %s-pago.\n", new_type == PRE_PAGO ? "pre" : "pos");
    } else {
        printf("Entrada inválida. Por favor, ingrese 0 para pre-pago o 1 para pos-pago.\n");
    }

    
}



void *send_messages(void *arg) {
    pthread_setcancelstate(PTHREAD_CANCEL_ENABLE, NULL);
    pthread_setcanceltype(PTHREAD_CANCEL_ASYNCHRONOUS, NULL);
    
    CLIENT_SETS *client_sets = (CLIENT_SETS *)arg;
    char msg_buffer[1024];

    while (1) {
        printf("Ingrese su mensaje (o 'cambiar' para cambiar de tipo de usuario): \n");
        fgets(msg_buffer, sizeof(msg_buffer), stdin);
        msg_buffer[strcspn(msg_buffer, "\n")] = '\0';

        if (strcmp(msg_buffer, "cambiar") == 0) { 
            change_user_type(client_sets);
            continue;
        }

        // Enviar el mensaje al servidor
        sem_wait(&(((*client_sets).buf_ref))->server_reading);
        sem_wait(&(((*client_sets).buf_ref))->session_sem);

        client_sets->buf_ref->status = get_char_by_code(SM);
        strcpy(client_sets->buf_ref->buffer, msg_buffer);

        sem_post(&(((*client_sets).buf_ref))->session_sem);
        sem_post(&(((*client_sets).buf_ref))->client_reading);

        // Esperar respuesta del servidor
        wait_for_read(client_sets->buf_ref);
        


        // Verificar si hubo un error
        if (client_sets->buf_ref->status == 'E') {
            printf("[ Servidor ] %s\n", client_sets->buf_ref->buffer);
        } else {
            printf("[ Enviado ] Mensaje enviado: %s\n", msg_buffer);
        }

        memset(msg_buffer, 0, sizeof(msg_buffer));
    }

    return NULL;
}

int main(int argc, char **argv) {
    // Manejar la salida con Ctrl+C
    signal(SIGINT, handler_exit);

    // Recibir los parámetros por memoria
    char *SYNC_MEMORY = NULL;
    char c;
    int user_mode = PRE_PAGO;

    while ((c = getopt(argc, argv, "n:p:")) != -1) {
        switch (c) {
            case 'n':
                SYNC_MEMORY = optarg;
                printf("[ + ] Usted inicia con nombre: %s\n", SYNC_MEMORY);
                break;
            case 'p':
                user_mode = atoi(optarg);
                printf("[ + ] Usted inicia con el modo: %d\n", user_mode);
                break;
            default:
                printf("No existe tal argumento\n");
                exit(EXIT_FAILURE);
        }
    }

    if (SYNC_MEMORY == NULL) {
        printf("Debe especificar un nombre de sesión con -n\n");
        exit(EXIT_FAILURE);
    }

    // Abrir conexión mapeando la memoria
    CORE_SETS cs = get_default_core_sets();
    client_sets = open_conn(&cs, SYNC_MEMORY);
    client_sets.user_mode = user_mode;
    printf("[ + ] Se estableció la conexión, estás en la sesión de sincronización\n");

    // Establecer sesión
    get_session_id(&client_sets, user_mode);

    // Asegurarse de que el ID es válido
    if (client_sets.buf_ref->id == 999) {
        printf("[ - ] Error al obtener el ID del servidor\n");
        exit(EXIT_FAILURE);
    }

    printf("[ + ] Se le asignó el ID: %d\n", client_sets.buf_ref->id);

    // Mostrar información del usuario
    print_user(&cs, client_sets.buf_ref->id);

    // Iniciar hilo para enviar mensajes
    pthread_t send_thread;
    pthread_create(&send_thread, NULL, send_messages, &client_sets);

    
    pthread_join(send_thread, NULL);

    return 0;
}
