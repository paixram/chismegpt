


#include <stdio.h>
#include <stdlib.h>
#include <fcntl.h>
#include <semaphore.h>
#include <sys/shm.h>
#include <sys/mman.h>
#include <time.h>
#include <unistd.h>
#include "core_utils.h"
#include <memory.h>

SEMCHECK init_program_semaphores(memory_data *mem) {
    // TODO: Inicializar los semaforos
    if(sem_init(&(mem->server_messages.access), 1, 1) == -1) {
        printf("[ - ] Error al inicializar el semaforo de acceso a la cola de mensajes\n");
        return errors;
    }

    if(sem_init(&(mem->synchro_rooms.sync_room_sem), 1, 1) == -1) {
        printf("[ - ] Error al inicializar el semaforo del cuarto de sincronizacion\n");
        return errors;
    }



    if(sem_init(&(mem->mutex), 1, 1) == -1) {
        printf("[ - ] Error al inicializar el semaforo de acceso a la estructura de memoria compartida\n");
        return errors;
    }
    
    return success;
};

void print_memory_info(CORE_SETS *core_settings) {
    printf("******************************\n");
    printf("*     Memory Information     *\n");
    printf("*          Schema            *\n");
    printf("******************************\n");
    printf("[ Info ] Memory Name: %s\n", core_settings->name);
    printf("[ Info ] Memory Address Space: %p\n", core_settings->memory_ptr);
    printf("[ Info ] Memory Decriptor: %d\n", core_settings->shimd);
}

CORE_SETS get_default_core_sets() {
    CORE_SETS cs;
    cs.name = MEMORY_KEY;
    cs.memory_ptr = NULL;
    cs.shimd = 0;

    return cs;

}

CORE_SETS init_memory() {
    CORE_SETS cs = get_default_core_sets();
    
    printf("[ + ] Inicializando la memoria compartida del servidor\n");

    // Crear la memoria compartida
    create_shared_memory(&cs);
    printf("[ + ] La memoria compartida ha sido creada\n");
    print_memory_info(&cs);
    
    return cs; // retorno la memoria creada
}

void create_shared_memory(CORE_SETS *core_settings) {
    // Variables declare
    int shimd;

    // create memory logic
    shimd = shm_open(core_settings->name, O_CREAT | O_RDWR, 0666);

    core_settings->shimd = shimd; // Establecer el id del decriptor para poder cerrarlo despues

    ftruncate(shimd, sizeof(memory_data));
    
    memory_data *memory_ptr = mmap(NULL, sizeof(memory_data), PROT_READ | PROT_WRITE, MAP_SHARED, shimd, 0);
    core_settings->memory_ptr = memory_ptr;
    
    // Inicializar semaforos
    if(init_program_semaphores(memory_ptr) != success) {
        printf("[ - ] Error al inicializar los semaforos\n");
        destroy_all_resources(core_settings);
        return;
    };


    return;
}

int temp_memory(char* name) {
    int shimd;
    // create memory logic
    shimd = shm_open(name, O_CREAT | O_RDWR, 0666);

    ftruncate(shimd, (12 * sizeof(char))); // define size of memory
    
    memory_data *buffer = mmap(NULL, 12 * sizeof(char), PROT_READ | PROT_WRITE, MAP_SHARED, shimd, 0);

    memset(buffer, 0, 12 * sizeof(char));
    
    
    return shimd;
}

int destroy_all_resources(CORE_SETS *core_settings) {
    // Destruir todos mis semaforos y la memoria compartida
    sem_destroy(&core_settings->memory_ptr->mutex);
    sem_destroy(&core_settings->memory_ptr->server_messages.access);
    sem_destroy(&core_settings->memory_ptr->synchro_rooms.sync_room_sem);

    munmap(core_settings->memory_ptr, sizeof(memory_data));
    close(core_settings->shimd);
    shm_unlink(core_settings->name);
    
    printf("[ - ] Los recursos han sido destruidos\n");
    core_settings->shimd = 0;
    core_settings->memory_ptr = NULL;
    core_settings->name = "";

    return 1;
}




/*
 * bin
 * int shmid = shmget(MEMORY_KEY, memory_size, int shmflg);
    // Se usa shm_open en ves de shmget ya que shm_open permite que otros procesos usen la memoria compartida ya que conocen un nombre en este caso /chismegpt_key, en cambio shmget es mas para procesos con fork() ya que a shmget no se le puede pasar un nombre, se le pasa un numero
    //
 * */
