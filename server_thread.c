#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <unistd.h>
#include <pthread.h>


void client_thread(int client_socket){

        char user[10] = {};
        recv(client_socket, user, sizeof(user), 0);

        //user[strcspn(user, "\n")] = "\0";

        printf("accepted, new user: %s\n", user);

        

        char buffer[255];
        int a = 0;
        while (1)
        {
            bzero(buffer, sizeof(buffer));

            a = recv(client_socket, buffer, sizeof(buffer), 0);
            
            if(a == 0)
            {
                fprintf(stderr, "ERROR: client was shutdowned, user: %s\n", user);
                break;
            } 
            
            printf("%s: %s", user, buffer);

            if(strncmp("//quit", buffer, 6) == 0)
            {
                printf("closing connection with user: %s\n", user);
                break;
                

            }
        }
        printf("closed connection with user: %s\n", user);
        close(client_socket);

}

int main(int argc, char const* argv[]){
    printf("booting..........\n");
    //server socket
    int serv_sock = socket(AF_INET, SOCK_STREAM, 0);
    if(serv_sock < 0)
        perror("error socket");

    printf("socket spraveny :)\n");
    printf("socket int: %d\n", serv_sock);
    //pole na spravy
    char msg[255] = "stable connection, you can send messages";

    //structura kde sa definuje adresa serveru
    struct sockaddr_in serv_addr;

    serv_addr.sin_family = AF_INET; //definovanie ipv4 adries
    serv_addr.sin_port = htons(atoi(argv[1])); //pridelenie portu
    serv_addr.sin_addr.s_addr = INADDR_ANY; //server bude pocuvat na vsetkych sietovych interfaceoch

    //socket sa zviaze so specifickou ip adresou
    printf("binding ........\n");
    bind(serv_sock, (struct sockaddr*) &serv_addr, sizeof(serv_addr));

    int clients[200];
    bzero(clients, sizeof(clients));
    int num = 0;
    pthread_t thread_id[200];
    while(1)
    {
        
        printf("listening .......\n");
        if(listen(serv_sock, 1) < 0)
            perror("error with listening");


        clients[num] = accept(serv_sock, NULL, NULL);
        send(clients[num], msg, sizeof(msg), 0);
        pthread_create(&thread_id[num], NULL, client_thread, clients[num]); 
        num++;
        
    }

    return 0;
}