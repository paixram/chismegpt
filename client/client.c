


#include "../protocol/protocol.h"
#include "../core/core_utils.h"

int main() {
   // Abrir conexion mapeando la memoria
   //
    CORE_SETS cs = get_default_core_sets();
    int errno = open_conn(&cs);
    if(errno == -1) {
        printf("Error al abrir la memoria compatrtida");
        return -1;
    }
    
    // Si se obtuvo la memoria compartida por lo que es hora de iniciar la comunicacion
    // lo primero que se hace en la conexion es registrarse en el servidor (haz ping)
    //synchronize(&cs);
        
    return 1;
}

// El cliente para poder conectarse con chismeGPT primero va a tener que conectar a un cuarto de espera que es una memoria compartida pequeña donde podra mandar mensajes de operaciones atomicas,
// tales como de sincronizacion, etc


