#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include "queue.h"



void initQueue(MessagesQueue *queue) {

    queue->size = 0;
}

void enque(MessagesQueue *queue, msg_proto_ref1 message) {
    if(queue->size >= DEFAULT_QUEUESIZE) {
        printf("[ - ] La cola de mensajes esta llena\n");
        return;
    }

    // Añadir el mensajes
    int i = queue->size;
    queue->messages[i] = message;
    queue->size++;
   // printf("HOLA: %s", queue->messages[i].data);
    // Amontonar arriba
    while(i != 0) {
        int parent = (i - 1) / 2;
        if(queue->messages[parent].priv_level < queue->messages[i].priv_level) {

            msg_proto_ref1 temp = queue->messages[i];
            queue->messages[i] = queue->messages[parent];
            queue->messages[parent] = temp;
            i = parent;
        }else{
            break;
        }
    }

}

msg_proto_ref1 deque(MessagesQueue *queue) {
    if(queue->size <= 0) {
        // Heap vacio
        printf("[ - ] La cola esta vacia\n");
        msg_proto_ref1 emptyMessage;
        return emptyMessage;
    }

    msg_proto_ref1 max_msg = queue->messages[0];
    queue->messages[0] = queue->messages[queue->size - 1];
    queue->size--;
    
    // Heapify down (Ajustar hacia abajo)
    int i = 0;
    while(1) {
        int left = 2 * i +1;
        int right = 2 * i + 2;
        int largest = i;
        if(left < queue->size && queue->messages[left].priv_level > queue->messages[largest].priv_level) {
            largest = left;
        }

        if(right < queue->size && queue->messages[right].priv_level > queue->messages[largest].priv_level) {
            largest = right;
        }

        if(largest != i) {
            msg_proto_ref1 temp = queue->messages[i];
            queue->messages[i] = queue->messages[largest];
            queue->messages[largest] = temp;
            i = largest;
        }else{
            break;
        }
    }
    
    return max_msg;
}



