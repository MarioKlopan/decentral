#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <time.h>
#include <unistd.h>
#include <pthread.h>
#include "server.h"


void *server(void *port){

    char *port_str = (char *) port;
    printf("Server: Booting server\n");

    int serv_sock = socket(AF_INET, SOCK_STREAM, 0);    //socket serveru
    if(serv_sock < 0){       				//kontrola navratovej hodnoty funkcie socket
    	fprintf(stderr,"Server: Failed to create socket\n");
	return NULL;
    }

    printf("Server: Socket created\n");

    //pole na uvodnu spravu
    char msg[255] = "stable connection, you can send messages\n";

    //structura kde sa definuje adresa serveru
    struct sockaddr_in serv_addr;

    serv_addr.sin_family = AF_INET; //definovanie ipv4 adries
    serv_addr.sin_port = htons(atoi(port_str)); //pridelenie portu
    serv_addr.sin_addr.s_addr = INADDR_ANY; //server bude pocuvat na vsetkych sietovych interfaceoch

    //socket sa zviaze so specifickou ip adresou
    printf("Server: Binding\n");

    int bind_status;
    bind_status = bind(serv_sock, (struct sockaddr*) &serv_addr, sizeof(serv_addr));
    if (bind_status == -1){
	    fprintf(stderr, "Server: Failed to bind\n");
	    return NULL;
    }

    int clients[200]; //pole kde sa vytvoria sockety ked sa pripoji klient
    pthread_t thread_id[200];   //pole pre id threadov
    bzero(clients, sizeof(clients));    //vycistenie pola clients

    if(listen(serv_sock, 1) < 0){    //zapne pasivne cakanie na pripojenia
    	fprintf(stderr,"Sever: Listening failed\n");
	return NULL;
    }

    printf("Server: Ready to accept connections\n");

    for(int i = 0; i < 200; i++)
    {        
        clients[i] = accept(serv_sock, NULL, NULL);   //akceptuje pripojenie klienta a uvolni hlavny socket pre dalsie pripojenia
        send(clients[i], msg, sizeof(msg), 0);    //send posle kontrolnu spravu klientovy aby vedel ze je uspesne pripojeny
        pthread_create(&thread_id[i], NULL, client_thread, &clients[i]); //vytvori sa novy thread s pripojenym klientom
    }

    return NULL;
}

//funkcia ktora sluzi na prijmanie sparv, funkcia je spustena v novom threade
void *client_thread(void *client_socket){
	
   int *client_socket_int = (int *) client_socket;

    char user[100] = {};     //pole na uzivatelske meno
    recv(*client_socket_int, user, sizeof(user), 0);     //prva sprava od klienta je pasivne poslana ako identifikacne uzivatelske meno 
    printf("Server: User joined the conversation: %s\n", user);  

   
    char buffer[255];   //pole na zapisanie prichadzajucich sprav
    int disconnect_check = 0;  //uklada navratovu hodnotu funkcie recv()
    while (1)
    {
        bzero(buffer, sizeof(buffer));  //vycisti pole kde sa zapisuju prijate spravy

        disconnect_check = recv(*client_socket_int, buffer, sizeof(buffer), 0);     //funkcia prijma spravy
        
        if(disconnect_check == 0)
        {
            printf("Server: User %s disconnected\n", user);
            break;
        } 
        
        printf("%s: %s", user, buffer);

        if(strncmp("//quit", buffer, 6) == 0)   //pokial client napise //quit tak ukoncuje spojenie, thread sa zavrie
        {
            printf("Server: User %s disconnected\n", user);
            break;
            

        }
    }
    printf("Server: Closed connection with user: %s\n", user);
    close(*client_socket_int);
    return NULL;
}
