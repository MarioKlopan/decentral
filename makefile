decentral : server.o client.o main.o
	gcc server.o client.o main.o -o decentral

server.o : server.c server.h
	gcc -c -o server.o server.c
client.o : client.c client.h
	gcc -c -o client.o client.c
main.o : main.c
	gcc -c -o main.o main.c

