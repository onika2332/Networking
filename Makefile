CFLAGS = -c -Wall
CC = gcc
LIBS =  -lm 

all: client server

server: server.o
	${CC} server.o -o server -pthread

client: client.o
	${CC} client.o -o client -pthread -lncurses

server.o: server.c
	${CC} ${CFLAGS} server.c

client.o: client.c
	${CC} ${CFLAGS} client.c

clean:
	rm -f *.o *~ client server