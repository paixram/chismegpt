
#ifndef CORE_UTILS_H
#define CORE_UTILS_H
#include <sys/mman.h>
#include <semaphore.h>
#include "queue.h"

// core utils
#define MAX_CLIENTS 100 // Establecer el numero de clientes simultaneos que puede tener el programa
                       //
                       // // Se definen los tipos de usuario de la plataforma

// Las conexiones no se limitaran, pueden ser cuantos clientes quieran, bajo esta premisa no deberia existir "client client_id[MAX_CLIENTS];"


typedef struct {
    int id;
    char nombre[12];
    USER_PRIV priority;
    sem_t client_sync;
    sem_t server_sync;
} client;


typedef struct { // Structura para implementar la sala de espera o de sincronizacion
    sem_t sync_room_sem;
    char room_name[12];
} sync_room;

typedef struct {
    sem_t mutex;
    //sem_t clien t_sems[MAX_CLIENTS];
    //sem_t server_sems[MAX_CLIENTS];
    /*
    *      * Estructura de los mensajes definidos (priority queue)
    *           * */
                    
    MessagesQueue server_messages;
    sync_room synchro_rooms;
    client client_id[MAX_CLIENTS];
} memory_data;


// core.h
#define MEMORY_DATA_MUTEX_INITIALIZED 0x11110000
#define MESSAGES_QUEUE_INITIALIZED 0X00001111
#define SEMAPHORES_ERROR_MASK 0x00000000

typedef enum {
    mutex_sem = MEMORY_DATA_MUTEX_INITIALIZED,
    messages_sem = MESSAGES_QUEUE_INITIALIZED,
    errors = (mutex_sem & SEMAPHORES_ERROR_MASK) || (messages_sem & SEMAPHORES_ERROR_MASK),
    success = mutex_sem | messages_sem,
} SEMCHECK;

#define MEMORY_KEY "/chismegpt_key" // Memory key for use in other process

typedef struct {
    int shimd;
    char* name;
    memory_data *memory_ptr;
} CORE_SETS;

CORE_SETS get_default_core_sets();

void init_local_clients_store_memory(CORE_SETS *core_settings);

SEMCHECK init_program_semaphores(memory_data *mem);
CORE_SETS init_memory();
void create_shared_memory(CORE_SETS *core_settings);
int destroy_all_resources(CORE_SETS *core_settings);

int temp_memory(char* name);

#endif
