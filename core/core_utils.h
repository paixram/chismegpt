
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
    int priority;
    int message_count;
    sem_t client_sync;
    sem_t server_sync;
} client;


typedef struct { // Structura para implementar la sala de espera o de sincronizacion
    sem_t sync_room_sem;
    sem_t packet_ready;
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
    sem_t server_messages_access;
    client client_id[MAX_CLIENTS];
    int client_count;
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
    int max_concurrent_messages;
} CORE_SETS;

CORE_SETS get_default_core_sets();

void init_local_clients_store_memory(CORE_SETS *core_settings);

SEMCHECK init_program_semaphores(memory_data *mem);
CORE_SETS init_memory();
void create_shared_memory(CORE_SETS *core_settings);
int destroy_all_resources(CORE_SETS *core_settings);


typedef enum {
    TP = 'T', // My temp memory - Le dice al server que ya inicio la memoria temporal
    TC = 'M', // Temp memory connected from server - El servidor ahora esta en la memoria temporal
    WR = 'V',  // Waiting for response - Esperar para respuesta
    GC = 'G', // Give me a code - Le dice a chismeGPT que quiero un ID
    CT = 'C', // Change user type - Le dice a chismegpt que cambiare entre los 2 tipos de usuarios de la pagina
    SM = 'S' // User send msg to server - El cliente mando mensaje al encolamiento del server
} COMMUNICATION_CODE;

typedef enum {
    OK,
    NOT_OK,
} request_status;

typedef struct { // estructura para operaciones de sesion (generalmente rapidas)
    sem_t session_sem;
    sem_t client_reading;
    sem_t server_reading;
    sem_t check_response;
    int id;
    char status;
    int request_status;
    char buffer[256];
} session_packet;

int session_memory(char* name, session_packet** buf);

#endif
