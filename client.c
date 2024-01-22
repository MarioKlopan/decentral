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
#include "client.h"


void *client(void *login){
    
    char *login_str = (char *) login;
    printf("Client: Booting client\n");

    thread_args server_args;    //vytovrenie struktury v ktorej su ulozene informacie o serveroch
    
    //precistenie poli v strukture
    bzero(server_args.server_sockets, sizeof(server_args.server_sockets));
    bzero(server_args.ip_address, sizeof(server_args.ip_address));
    bzero(server_args.open_sockets, sizeof(server_args.open_sockets));

    strcat(server_args.login, login_str);   //vlozenie loginu do struktury

    //nacitanie adries zo suboru
    printf("Client: Loading addresses\n");
    FILE* addresses;
    addresses = fopen("adresy.txt", "r");      //TODO - dynamicke zadanie suboru, zle otvorenie poriesit
    char tmp;
    for (int i = 0; tmp != EOF; i++)    //kazdy cyklus je jedna adresa
    {
        for (int a = 0; (tmp = fgetc(addresses)) != '\n' && tmp != EOF; a++)
        {
            server_args.ip_address[i][a] = tmp;     //priradi nacitany char do suboru
        }
        
    }
    fclose(addresses);


    //cyklus kde sa vola funkcia server_thread ktora vytvara socket pre kazdu adresu
    for (int i = 0; server_args.ip_address[i][0] != 0; i++)
    {
        server_args.num = i;
        printf("Client: %d. Connecting with %s\n", server_args.num + 1, server_args.ip_address[i]);
	server_thread(&server_args);
    }

    int live_counter = 0;   //uklada pocet serverov s ktorymi bolo naviazane spojenie
    for(int count = 0; count < 200; count++)
    {
        if(server_args.open_sockets[count] == 1)
            live_counter++;
    }
    if(live_counter == 0)   //ak je live_counter nula tak su vsetky servre vypnute
    {
        printf("Client: No active servers, exiting\n");
        return NULL;
    }
    else    //inak bolo naviazane nejake uspesne spojenie a uzivatel moze pisat, zobrazi sa mu logo a pocet pripojenych serverov zo vsetkch
    {
        printf("Client: Active servers %d\n", live_counter);
    }
    
    char buffer[255];   //pole na ukladanie sprav ktore sa odoslu
    int a = 0;
    while (1)
    {

        bzero(buffer, sizeof(buffer));  //vycistenie pola
        printf("you: ");    //koli rozliseniu v chate
        fgets(buffer, sizeof(buffer), stdin);   

        for (int i = 0; i < 200; i++)   //cyklus sluzi na rozposlanie sprav iba aktivnym spojeniam
        {
            if(server_args.open_sockets[i] == 1)
            {
                a = write(server_args.server_sockets[i], buffer, sizeof(buffer));
                if(a < 0)
                {
                    fprintf(stderr, "Client: Failed to send message\n");
                }
                

            }
        }

        //pokial klient napise //quit tak odchadza z chatu a uknoci komunikaciu a aplikacia sa vypne
        if(strncmp("//quit", buffer, 6) == 0)
        {
            printf("closing connection\n");
            for (int j = 0; j < 200; j++)
            {
                if(server_args.open_sockets[j] == 1)
                    close(server_args.server_sockets[j]);
            }
            printf("closed\n");
            break;
        }
        
    }

    return NULL;

}

//funkcia sluzi na vytvorenie socketu a nadviazanie spojenia so serverom
int server_thread(thread_args *server_args){

    server_args->server_sockets[server_args->num] = socket(AF_INET, SOCK_STREAM, 0);    //vytvorenie socketu
    if(server_args->server_sockets[server_args->num] < 0)   //kontrola vytovrenia socketu
    {
        fprintf(stderr, "Client: %d. Failed to create socket\n", server_args->num);
        return -1;
    }
    printf("Client: %d. Socket created\n", server_args->num + 1);
    

    struct sockaddr_in server_addr; //vytorenie structuri ktora sa dava spojeniu aby vedelo s kym a ako komunikovat

    server_addr.sin_family = AF_INET;
    server_addr.sin_port = htons(8080);     //TODO - dynamicky pridelovat port

    
    if(inet_pton(AF_INET, server_args->ip_address[server_args->num], &server_addr.sin_addr) <= 0)   //funkcia na prelozenie ip adresy
    {
        
        fprintf(stderr, "Client: %d. Invalid address\n", server_args->num + 1);
        return -1;
    }
    printf("Client: %d. Connecting to address <%s>\n", server_args->num + 1, server_args->ip_address[server_args->num]);
    

    
    int connect_status = connect(server_args->server_sockets[server_args->num], (struct sockaddr*) &server_addr, sizeof(server_addr)); //funkcia na vytvorenie spojenia
    if (connect_status == -1)
    {
        fprintf(stderr, "Client: %d. Connection failed\n", server_args->num + 1);
        return -1;
    }
    printf("Client: %d. Connection established\n", server_args->num + 1);

    server_args->open_sockets[server_args->num] = 1;    //pokial sa klient pripojil na server tak to zaznaci v tabulke ako 1

    char buffer[255];
    recv(server_args->server_sockets[server_args->num], buffer, sizeof(buffer), 0); //server ako prvu spravu posle kontrolnu spravu ze uzivatel bol pripojeny
    printf("Client: Message from server - %s", buffer);

    write(server_args->server_sockets[server_args->num], server_args->login, sizeof(server_args->login));   //klient ako prvu spravu posle na server svoju indentifikaciu
    return 0;
}
