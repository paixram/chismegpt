
#include "protocol.h"
#include "../core/core_utils.h"
#include <bits/pthreadtypes.h>
#include <semaphore.h>
#include <signal.h>
#include <string.h>
#include <sys/mman.h>
#include <stdio.h>
#include <stdlib.h>
#include <fcntl.h>
#include <pthread.h>
#include <unistd.h>

#define PROCESSING_TIME_MS 500

sem_t concurrent_messages_sem;


void initialize_protocol(CORE_SETS *core_settings) {
    // Inicializar el semáforo con el valor de max_concurrent_messages
    sem_init(&concurrent_messages_sem, 0, core_settings->max_concurrent_messages);
}

void *message_processing_thread(void *arg) {
    msg_proto_ref1 *msg = (msg_proto_ref1 *)arg;

    // Esperar si se ha alcanzado el máximo de mensajes concurrentes
    sem_wait(&concurrent_messages_sem);
    
    printf("[ Procesando ] Mensaje de usuario ID %d (%s-pago): %s\n",
           msg->client_id,
           msg->priv_level == POS_PAGO ? "Pos" : "Pre",
           msg->data);

    // Simular tiempo de procesamiento
    usleep(PROCESSING_TIME_MS * 1000);

    printf("[ Procesado ] Mensaje de usuario ID %d: %s\n", msg->client_id, msg->data);

    // Liberar el semáforo al finalizar
    sem_post(&concurrent_messages_sem);

    free(msg);
    return NULL;
}


void *chismegpt_listen(void *core_settings) {
    CORE_SETS *cs = (CORE_SETS *)core_settings;
    while (1) {
        sem_wait(&cs->memory_ptr->server_messages_access);

        sem_wait(&cs->memory_ptr->mutex);
        if (cs->memory_ptr->server_messages.size > 0) {
            msg_proto_ref1 msg = deque(&cs->memory_ptr->server_messages);
            sem_post(&cs->memory_ptr->mutex);

            // Crear un nuevo hilo para procesar el mensaje
            msg_proto_ref1 *msg_copy = malloc(sizeof(msg_proto_ref1));
            memcpy(msg_copy, &msg, sizeof(msg_proto_ref1));

            pthread_t thread_id;
            pthread_create(&thread_id, NULL, message_processing_thread, msg_copy);
            pthread_detach(thread_id);
        } else {
            sem_post(&cs->memory_ptr->mutex);
        }

        sem_post(&cs->memory_ptr->server_messages_access);

        // Pequeña pausa para evitar consumo excesivo de CPU
        usleep(1000);
    }

    return NULL;
}

struct SESION_DATA_FORMAT {
    char OP;
    char payload[11];
};

typedef struct {
    char *cargautil;
    CORE_SETS *cs;
} handler_arguments;

void *synchronize_user(void *core_settings) {
    CORE_SETS *csts = (CORE_SETS *)core_settings;
    while(1) {
        // Todo: capturar la estructura y asignar id al cliente;
        sem_wait(&csts->memory_ptr->synchro_rooms.packet_ready);
        sem_wait(&csts->memory_ptr->synchro_rooms.sync_room_sem);

        // Recibir codigo de operacion y proceder con su ejecucion
        //(*csts).memory_ptr
        // <==>
        //csts->memory_ptr
        struct SESION_DATA_FORMAT *data_format = (struct SESION_DATA_FORMAT *)(csts->memory_ptr->synchro_rooms.room_name);

        printf("[ + ] Un mensaje en la sesion de espera fue recibido\n"); 
       
        if(data_format->OP != 'T') {
            printf("[ - ] No puede haber otra operacion diferente a T en la sincronizacion\n");
            sem_post(&csts->memory_ptr->synchro_rooms.sync_room_sem);
            continue;
        }

        // Usar el canal de comunicacion proporcionado por el usuario
        // Crear un subproceso que se quede escuchando las peticiones de sesion personales de cada usuario
        //printf("Type: %c, Data: %s \n", data_format->OP, (*data_format).payload);
        handler_arguments ha;
        ha.cargautil = data_format->payload;
        ha.cs = csts;
       pthread_t user_session_id;
        pthread_create(&user_session_id, NULL, handle_user_personal_session, &ha);

        // Limpiar el buffer anterior
        //memset(csts->memory_ptr->synchro_rooms.room_name, 0, 12 * sizeof(char));

        // Solo para sincronizar con la otra memoria
        // Liberar el semaforo  
        sem_post(&csts->memory_ptr->synchro_rooms.sync_room_sem);
    }
} 



void *handle_user_personal_session(void* session_name) {

    handler_arguments *ha = (handler_arguments *)session_name;
    CORE_SETS *cs = (CORE_SETS *)(ha->cs);
    // EL client crea la memoria compartida y el server la abre
   
    char *s_name = (char *)(ha->cargautil); // SI es exitoso, debe estar en espera de respuesta
    int len = strlen(s_name);
    printf("char_name:%s, strlen(%s): %d\n", s_name, s_name, len);
    if (s_name == NULL || strlen(s_name) == 0) {
        printf("[ - ] Nombre de sesión inválido\n");
        pthread_exit(NULL);
    }
        
   
    // Abrir canal
    int session_id = shm_open(s_name, O_RDWR, 0666);
    
    if(session_id == -1) {
        printf("[ - ] No se pudo obtener la session en memoria\n");
        pthread_exit(NULL);
    }
    
    session_packet *sp = mmap(NULL, sizeof(session_packet), PROT_READ | PROT_WRITE, MAP_SHARED, session_id, 0);
     
    if (sp == MAP_FAILED) {
        perror("[ - ] Error al mapear la memoria compartida\n");
        pthread_exit(NULL);
    }
    
    // stablish in waiting response for initial client status

    sem_wait(&sp->session_sem);
    sp->id = 999; // 999 is used for un-id-ed user 
    sp->status = 'V';
    sem_post(&sp->session_sem);
    sem_post(&sp->server_reading);
 
    // Bucle while para captar señales de mensaje en la session activa
    while(1) {
        
       sem_wait(&sp->client_reading);
    
       //printf("[ + ] Una nuerva respuesta ha llegado del usuario\n");
       //printf("[ + ] El usaurio desea: %c\n", sp->status);
    
       COMMUNICATION_CODE cd_received = get_code_by_char(sp->status);

       switch(cd_received) {
            case GC:
                if (sp->id != 999) {
                    printf("[ - ] Usuario ya tiene un ID asignado\n");
                } else {
                    // Crear código y mandarlo al usuario
                    sem_wait(&cs->memory_ptr->mutex);
                    sp->id = cs->memory_ptr->client_count++;
                    power_on_resources(cs, sp->id);

                    cs->memory_ptr->client_id[sp->id].id = sp->id;
                    strncpy(cs->memory_ptr->client_id[sp->id].nombre, s_name, sizeof(cs->memory_ptr->client_id[sp->id].nombre));
                    cs->memory_ptr->client_id[sp->id].priority = sp->buffer[0];
                    cs->memory_ptr->client_id[sp->id].message_count = 0; // Inicializar contador
                    sem_post(&cs->memory_ptr->mutex);

                    print_user(cs, sp->id);
                    ready_for_read(sp);
                }
                break;
             case SM:
                // Verificar límite de mensajes si es pre-pago
                //printf("MEnsajes del user: %d\n", cs->memory_ptr->client_id[sp->id].message_count);           
                // Verificar que el ID es válido
                 if (sp->id == 999) {
        printf("[ - ] Cliente con ID inválido intentando enviar mensaje. Asignando nuevo ID...\n");
        
        // Asignar un nuevo ID al cliente
        sp->id = cs->memory_ptr->client_count++;
        power_on_resources(cs, sp->id);

        // Inicializar los datos del cliente
        cs->memory_ptr->client_id[sp->id].id = sp->id;
        strncpy(cs->memory_ptr->client_id[sp->id].nombre, s_name, sizeof(cs->memory_ptr->client_id[sp->id].nombre));
        cs->memory_ptr->client_id[sp->id].priority = sp->buffer[0]; // Obtener prioridad del mensaje
        cs->memory_ptr->client_id[sp->id].message_count = 0;

        printf("[ + ] Se asignó el ID %d al cliente '%s'\n", sp->id, cs->memory_ptr->client_id[sp->id].nombre);
    }          
                if (cs->memory_ptr->client_id[sp->id].priority == PRE_PAGO) {
                    if (cs->memory_ptr->client_id[sp->id].message_count >= 10) {
                        // Enviar mensaje de error al cliente
                        //printf("SUSUIDOO");
                        printf("[ - ] Usuario pre-pago ha alcanzado el límite de mensajes.\n");
                        sp->status = 'E'; // Código de error
                        strcpy(sp->buffer, "Límite alcanzado");
                        ready_for_read(sp);
                        break;
                    } else { 
                        cs->memory_ptr->client_id[sp->id].message_count++;
                    }
                }
                // Procesar el mensaje normalmente
                sem_wait(&cs->memory_ptr->server_messages_access);

                msg_proto_ref1 mpr;
                strcpy(mpr.data, sp->buffer);
                mpr.priv_level = cs->memory_ptr->client_id[sp->id].priority;
                mpr.client_id = sp->id;

                enque(&cs->memory_ptr->server_messages, mpr);

                sem_post(&cs->memory_ptr->server_messages_access);

                ready_for_read(sp);
                break;
             case CT:
                // Iniciar sección crítica
                sem_wait(&cs->memory_ptr->mutex);
    
                // Verificar que el ID es válido
                if (sp->id == 999) {
                    printf("[ - ] Usuario con ID inválido intentando enviar mensaje\n");
                    // Enviar error al cliente
                    sp->status = 'E';
                    strcpy(sp->buffer, "ID inválido");
                    ready_for_read(sp);
                    sem_post(&cs->memory_ptr->mutex);
                    break;
                }

                // Cambiar el tipo de usuario
                cs->memory_ptr->client_id[sp->id].priority = sp->buffer[0];
                cs->memory_ptr->client_id[sp->id].message_count = 0;

                // Salir de la sección crítica
                sem_post(&cs->memory_ptr->mutex);

                printf("[ Actualizado ] Usuario ID %d cambió a %s-pago.\n",
                       sp->id,
                       cs->memory_ptr->client_id[sp->id].priority == PRE_PAGO ? "pre" : "pos");
                ready_for_read(sp);
                break;
            default:
                printf("[ - ] Código de operación no reconocido\n");
                ready_for_read(sp);
                break;
        }

       //print_user(cs, sp->id);
       //ready_for_read(sp);

       sem_post(&sp->session_sem);

       sem_post(&sp->server_reading);
        
    }

    //free(session_name);
};

void print_user(CORE_SETS *core, int index) {
    printf("----------------- USER CONNECTED ----------------- \n");
    printf("--- User ID: %d                            ------- \n", core->memory_ptr->client_id[index].id);
    printf("--- User name: %s                          ------- \n", core->memory_ptr->client_id[index].nombre);
    printf("--- User Priority: %d                      ------- \n", core->memory_ptr->client_id[index].priority);
    printf("-------------------------------------------------- \n");

}


void power_on_resources(CORE_SETS *cs, int client_operation) {
    if(sem_init(&cs->memory_ptr->client_id[client_operation].client_sync, 1, 0) == -1) {
        printf("[ - ] No se pudo abrir el semaforo de sincronizacion de cliente\n");
    };

    if(sem_init(&cs->memory_ptr->client_id[client_operation].server_sync, 1, 0) == -1) {
        printf("[ - ] No se pudo abrir el semaforo de sincronizacion de servidor\n");
    };
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


CLIENT_SETS open_conn(CORE_SETS *core_settings, char* channel) {
    int shimd = shm_open(core_settings->name, O_RDWR, 0666);
    if(shimd == -1) {
        printf("[ - ] Error al abrir el obtener el decriptor de memoria compartida\n");
        //return;
        raise(SIGINT);
    }
    core_settings->shimd =  shimd;

    core_settings->memory_ptr = mmap(NULL, sizeof(memory_data), PROT_READ | PROT_WRITE, MAP_SHARED, shimd, 0);
    
    printf("[ + ] El cliente ha obtenido la memoria compartida\n");

    // Start handshaking process
    // Create temp share memory
    session_packet *buf_ref;
    int temp_id = session_memory(channel, &buf_ref);
    if(temp_id == -1) {
        printf("[ - ] Error al crear la memoria compartida\n");
        raise(SIGINT);
    }; 
    CLIENT_SETS cs;
    cs.channel = channel;
    cs.buf_ref = buf_ref;
    cs.tempid = temp_id;

    // Comunicar al servidor que ya esta creada la memoria temporal
    sem_wait(&(core_settings->memory_ptr->synchro_rooms.sync_room_sem));
    
    // Crear mi buffer de sincronizacion
    char buffer[12];

    // Asignar codigo de sesion
    assign_session_code(&(buffer[0]), TP);

    // Escribir en el buffer
    if(write_in_buffer(&(buffer[0]), &(channel[0])) != WRITE_OK) {
        printf("[ - ] Error al escribir en el buffer de datos durante la sincronizacion");

    };

    // Escribir en la memoria
    
    strcpy(core_settings->memory_ptr->synchro_rooms.room_name, buffer);

    // Abrir el lock de la memoria
    sem_post(&(core_settings->memory_ptr->synchro_rooms.sync_room_sem));
    sem_post(&(core_settings->memory_ptr->synchro_rooms.packet_ready)); 
     
    //destroy_all_resources(core_settings);
    return cs;
} 

int get_profile(USER_PRIV user_type) {
    // TODO: Mandar a crear un user con este tipo, al server (LO ULTIMO QUE HIZE)
    // get_session_id();
}

char get_char_by_code(COMMUNICATION_CODE code) {
    switch (code) {
        case TP: return 'T';
        case WR: return 'V';
        case GC: return 'G';
        case CT: return 'C';
        case SM: return 'S';
        default: return '?'; // Valor por defecto
    }
}

COMMUNICATION_CODE get_code_by_char(char code) {
    switch (code) {
        case 'T': return TP;
        case 'V': return WR;
        case 'G': return GC;
        case 'C': return CT;
        case 'S': return SM;
        //default: return NULL; // Valor por defecto
    }
}

void assign_session_code(char *data, COMMUNICATION_CODE hc) {
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


// Funcion para comunicarse predefinida
// int exec_send(CLIENT_SETS *cs, void* (fn*)())

//// Client API memory communication link frontend

void wait_for_read(session_packet *sp) {
    sem_wait(&sp->check_response);
}

void ready_for_read(session_packet *sp) {
    sem_post(&sp->check_response);
}

void get_session_id(CLIENT_SETS *cs, int mode) {
    // TODO: implementar la obtencion del ID para poder comunicarme con la cola
    //cs->buf_ref
    // Primero debemos chequear que el servidor este en espera de respuesta 
    //check_wait_status(cs);

    sem_wait(&(((*cs).buf_ref))->server_reading);
    sem_wait(&(((*cs).buf_ref))->session_sem);

    // Escribir en la cola el status para obtener un id
    
    (*cs).buf_ref->status = get_char_by_code(GC);
    (*cs).buf_ref->buffer[0] = mode;

    sem_post(&(((*cs).buf_ref))->session_sem);

    sem_post(&(((*cs).buf_ref))->client_reading);
     
    wait_for_read(cs->buf_ref);

}


void write_in(CORE_SETS *core_sets, CLIENT_SETS *client_sets) {
    char msg_buffer[1024];
    while(1) {
        printf("Ingrese su mensaje: \n");
        fgets(msg_buffer, sizeof(msg_buffer), stdin);
        msg_buffer[strcspn(msg_buffer, "\n")] = '\0';
        
        // Escribir el mensaje en la cola
        sem_wait(&(((*client_sets).buf_ref))->server_reading);
        sem_wait(&(((*client_sets).buf_ref))->session_sem);

        client_sets->buf_ref->status = get_char_by_code(SM);
        sem_wait(&core_sets->memory_ptr->mutex);
        
        msg_proto_ref1 mpr;
        //mpr.data = msg_buffer;
        strcpy(mpr.data, msg_buffer);
        mpr.priv_level = core_sets->memory_ptr->client_id[client_sets->buf_ref->id].priority;
        
        enque(&core_sets->memory_ptr->server_messages, mpr);
        sem_post(&core_sets->memory_ptr->mutex);
        
        sem_post(&(((*client_sets).buf_ref))->session_sem);
        sem_post(&(((*client_sets).buf_ref))->client_reading);
       
        // Liberacion para contactar con memory_ptr
        sem_post(&core_sets->memory_ptr->client_id[client_sets->buf_ref->id].client_sync);

        memset(msg_buffer, 0, 1024);
    }    

}

int check_wait_status(CLIENT_SETS *cs) {
    
    
    

    return 1;
}

