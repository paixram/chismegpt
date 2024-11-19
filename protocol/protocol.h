
/*
 * CHISMEGPT Protocol definition
 * */

#ifndef PROTOCOL_H
#define PROTOCOL_H
#include "../core/core_utils.h"


typedef enum {
    TP = 'T', // My temp memory - Le dice al server que ya inicio la memoria temporal
    WR = 'V',  // Waiting for response - Esperar para respuesta
    GC = 'G', // Give me a code - Le dice a chismeGPT que quiero un ID
    CT = 'C', // Change user type - Le dice a chismegpt que cambiare entre los 2 tipos de usuarios de la pagina
} HANDSHAKE_CODE;

typedef enum {
    WRITE_OK,  
    WRITE_ERROR,
    READ_OK,
    READ_ERROR,
} BUFFER_OP_STATUS;

void assign_session_code(char *data, HANDSHAKE_CODE hc);
char get_char_by_code(HANDSHAKE_CODE code);
BUFFER_OP_STATUS write_in_buffer(char *buffer, char *data);

// Start server definition
// ChismeServer proto def
void open_server(CORE_SETS *core_settings);
void stablish_settings();
void handshake();


// Client proto def
int open_conn(CORE_SETS *core_settings,char* channel);
void *chismegpt_listen(void *core_settings);

#endif
