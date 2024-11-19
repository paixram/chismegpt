#ifndef QUEUE_H
#define QUEUE_H

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <semaphore.h>


#define DEFAULT_QUEUESIZE 60
#define MESSAGE_SIZE 256

typedef enum {
        POS_PAGO = 0,
        PRE_PAGO = 1,
} USER_PRIV;

typedef struct {
    char data[MESSAGE_SIZE];
    USER_PRIV priv_level; 
} MESSAGE;

typedef enum {
    SYNC = 0,
    PAYLOAD = 1,
    BYE = 2,
} MSG_TYPE;

// Buffer structure
#define MAX_BUFFER 1024

typedef char buffer[MAX_BUFFER];

typedef struct {
    MSG_TYPE type;
    buffer data;
    USER_PRIV priv_level;
} msg_proto_ref1;

typedef struct {
    sem_t access;    
    msg_proto_ref1 messages[DEFAULT_QUEUESIZE];
    int size;  
} MessagesQueue;

void initQueue(MessagesQueue *queue);
void enque(MessagesQueue *queue, msg_proto_ref1 message);
msg_proto_ref1 deque(MessagesQueue *queue);


#endif
