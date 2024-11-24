# Variables
GCC = gcc 

server: chismegpt.o core.o queue.o protocol.o
	$(GCC) -o chismegpt_server chismegpt.o core.o protocol.o queue.o

chismegpt.o: ./core/chismegpt.c ./core/core_utils.h ./protocol/protocol.h
	$(GCC) -c ./core/chismegpt.c


client: client.o core.o queue.o protocol.o
	$(GCC) -o cclient client.o core.o queue.o protocol.o

client.o: ./protocol/protocol.h ./core/core_utils.h
	$(GCC) -c ./client/client.c

core.o: ./core/core.c ./core/core_utils.h
	$(GCC) -c ./core/core.c

protocol.o: ./protocol/protocol.c ./protocol/protocol.h ./core/core_utils.h
	$(GCC) -c ./protocol/protocol.c

queue.o: ./core/queue.c ./core/queue.h
	$(GCC) -c ./core/queue.c


.PHONY = clean

clean:
	rm -f server cclient *.o
