
/*
 * CHISMEGPT Protocol definition
 * */

#ifndef PROTOCOL_H
#define PROTOCOL_H
#include "../core/core_utils.h"




typedef enum {
    WRITE_OK,  
    WRITE_ERROR,
    READ_OK,
    READ_ERROR,
} BUFFER_OP_STATUS;


void initialize_protocol(CORE_SETS *core_settings);

void assign_session_code(char *data, COMMUNICATION_CODE hc);
char get_char_by_code(COMMUNICATION_CODE code);
COMMUNICATION_CODE get_code_by_char(char code);
BUFFER_OP_STATUS write_in_buffer(char *buffer, char *data);

// Start server definition
// ChismeServer proto def
void open_server(CORE_SETS *core_settings);
void stablish_settings();
void handshake();


// Client proto def
typedef struct {
    int tempid;
    session_packet *buf_ref;
    char *channel;
    int user_mode;
} CLIENT_SETS;

void *handle_user_personal_session(void* session_name);
CLIENT_SETS open_conn(CORE_SETS *core_settings,char* channel);
void *chismegpt_listen(void *core_settings);

// Api funcs
int check_wait_status(CLIENT_SETS *cs);
void get_session_id(CLIENT_SETS *cs, int mode);
void power_on_resources(CORE_SETS *cs, int client_operation);
void print_user(CORE_SETS *core, int index);
void wait_for_read(session_packet *sp);
void ready_for_read(session_packet *sp);
void write_in(CORE_SETS *core_sets, CLIENT_SETS *client_sets);
#endif
