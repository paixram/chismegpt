

#include "core_utils.h"
#include "queue.h"

int main() {
    MessagesQueue queue;
    initQueue(&queue);

    // Crear mensajes con diferentes prioridades
    msg_proto_ref1 msg1 = {PAYLOAD, "Mensaje de pre pago 1", PRE_PAGO};
    msg_proto_ref1 msg2 = {PAYLOAD, "Mensaje de pos pago", POS_PAGO};
    msg_proto_ref1 msg3 = {SYNC, "Mensaje de pre pago 2", PRE_PAGO};
    msg_proto_ref1 msg4 = {PAYLOAD, "Otro mensaje de pos pago", POS_PAGO};

    // Insertar mensajes en la cola
    enque(&queue, msg1);
    enque(&queue, msg2);
    enque(&queue, msg3);
    enque(&queue, msg4);
    
    printf("size: %d", queue.size);
    // Extraer mensajes y mostrarlos
    printf("Extrayendo mensajes en orden de prioridad:\n");
    //MESSAGE msg = deque(&queue);
      //  printf("Prioridad: %d, Mensaje: %s\n", msg.priv_level, msg.data);

    while (queue.size != 0) {
        //printf("HOla");
        msg_proto_ref1 msg = deque(&queue);
        printf("Prioridad: %d, Mensaje: %s\n,", msg.priv_level, msg.data);
    }

    return 0;
}






