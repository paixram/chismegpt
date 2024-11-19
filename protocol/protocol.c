


#include "protocol.h"
#include "../core/core_utils.h"
#include <bits/pthreadtypes.h>
#include <semaphore.h>
#include <sys/mman.h>
#include <stdio.h>
#include <stdlib.h>
#include <fcntl.h>
#include <pthread.h>
#include <unistd.h>

void *chismegpt_listen(void *core_settings) {
    CORE_SETS *csts = (CORE_SETS *)core_settings;
    while(1) {
            
        // TODO: CREAR EL SEMAFORO SINMCHRONIZE PARA SINCRONIZAR LOS NUEVOS CLIENTES
        
    } 

}

void *synchronize_user(void *core_settings) {
    CORE_SETS *csts = (CORE_SETS *)core_settings;
    while(1) {
        // Todo: capturar la estructura synchronize y asignar id al cliente;
    }
}

void open_server(CORE_SETS *core_settings) {
    // Inicia el proceso de ver los semaforos
    pthread_t listen_id, synchronize;
    pthread_create(&synchronize, NULL, synchronize_user, core_settings);
    pthread_create(&listen_id, NULL, chismegpt_listen, core_settings);
    pthread_join(synchronize, NULL);
    pthread_join(listen_id, NULL);

};
void stablish_settings();
void handshake();


// Client_def

int open_conn(CORE_SETS *core_settings, char* channel) {
    int shimd = shm_open(core_settings->name, O_RDWR, 0666);
    if(shimd == -1) {
        printf("[ - ] Error al abrir el obtener el decriptor de memoria compartida\n");
        return -1;
    }
    core_settings->shimd =  shimd;

    core_settings->memory_ptr = mmap(NULL, sizeof(memory_data), PROT_READ | PROT_WRITE, MAP_SHARED, shimd, 0);
    
    printf("[ + ] El cliente ha obtenido la memoria compartida\n");

    // Start handshaking process
    // Create temp share memory
    int temp_id = temp_memory(channel);
    if(temp_id == -1) {
        printf("[ - ] Error al crear la memoria compartida\n");
        return -1;
    }; 
    // Comunicar al servidor que ya esta creada la memoria temporal
    sem_wait(&(core_settings->memory_ptr->synchro_rooms.sync_room_sem));
    
    // Crear mi buffer de sincronizacion
    char buffer[12];

    // Asignar codigo de sesion
    assign_session_code(&(buffer[0]), TP);

    // Escribir en memoria
    write_in_buffer(&(buffer[0]), &(channel[0]));

    return 1;
}

char get_char_by_code(HANDSHAKE_CODE code) {
    switch (code) {
        case TP: return 'T';
        case WR: return 'V';
        case GC: return 'G';
        case CT: return 'C';
        default: return '?'; // Valor por defecto
    }
}

void assign_session_code(char *data, HANDSHAKE_CODE hc) {
    switch(hc) {
        case TP:
        case WR:
        case GC:
        case CT:
            data[0] = get_char_by_code(hc);
            break;
        default:
            data[0] = '?';
            printf("[ - ] HANDSHAKE CODE no asociado");
   }

}

// Formato de los paquetes de la sesion de espera
// 1 Byte ----> Para indicar el HANDSHAKE CODE
// 11 Byte ----> Para indicar el cuerpo de los datos
BUFFER_OP_STATUS write_in_buffer(char *buffer, char *data) { // data no puedo exceder los 11 bits
   if (buffer == NULL || data == NULL) return WRITE_ERROR;

    int len = strlen(data);
    len = (len > 11) ? 11 : len;


    // write in data
    for(int i = 0; i < len; i++) {
        *(buffer + (i + 1)) = *(data + i);
    }

    return WRITE_OK;
}


