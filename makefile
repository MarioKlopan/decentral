CC=gcc
CFLAGS= -std=c11

objects = server.o client.o main.o

decentral : $(objects)
	$(CC) $(CFLAGS) $(objects) -o decentral

server.o : server.c ./headers/server.h
client.o : client.c ./headers/client.h
main.o : main.c

clear: 
	rm *.o decentral

