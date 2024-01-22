objects = server.o client.o main.o

decentral : $(objects)
	gcc $(objects) -o decentral

server.o : server.c server.h
client.o : client.c client.h
main.o : main.c

