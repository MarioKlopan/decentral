/*
./decentral adresy.txt port login
*/


#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <unistd.h>
#include <pthread.h>

//structura ktora sluzi na predanie argumentov pre funkciu kotra vytvori novy thread
typedef struct thread_args{
    char login[100];            //login zada uzivatel pri spusteni programu
    int server_sockets[200];    //tu sa ukaladaju vsetky vytvorene sockety na ktore sa klient pripoji
    char ip_address[200][16];   //tu sa ukladaju naciatne ip adresy zo suboru
    int open_sockets[200];      //na zanamenavanie pripojenych serverov, poradie odpoveda ipadrese a socketu, 0 = neotvoreny, 1 = otvoreny
    int num;                    //num urcuje index pre polia, aby vo funkcii program vedel ktory parameter pouzit
}thread_args;


int server(char *port);  //hlavna funkcia serveru
int client(char *login);  //hlavna funkcia klienta
void client_thread(int client_socket);
int server_thread(thread_args *server_args);

int main(int argc, char *argv[]){

    int a = 5;
    
    pthread_t thread_id[2];
    fprintf(stderr, "server starting .....\n");
    pthread_create(&thread_id[0], NULL, server, argv[2]);

    //pthread_t client_thread_id;
    fprintf(stderr, "client starting .....\n");
    pthread_create(&thread_id[1], NULL, client, argv[3]);
    
    //pthread_join(thread_id[0], NULL);
    pthread_join(thread_id[1], NULL);
    return 0;
}

int server(char *port){
    //todo - socket -> bind -> listening -> accept(new threads)

    fprintf(stderr, "booting srever .....\n");
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
    serv_addr.sin_port = htons(atoi(port)); //pridelenie portu
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

int client(char *login){
    //todo - sockets(new threads) -> connect -> return with sockets
    fprintf(stderr, "booting client .....\n");
    thread_args server_args;
    
    pthread_t thread_id[200];
    bzero(server_args.server_sockets, sizeof(server_args.server_sockets));
    bzero(server_args.ip_address, sizeof(server_args.ip_address));
    bzero(server_args.open_sockets, sizeof(server_args.open_sockets));
    //bzero(server_args.login, sizeof(server_args.login));
    //char login[10] = "mario";
    strcat(server_args.login, login);
    //    server_args.login = argv[2];

    FILE* adresy;
    adresy = fopen("adresy.txt", "r");
    char tmp;
    for (int i = 0; tmp != EOF; i++)    //kazdy cyklus je jedna adresa
    {
        for (int a = 0; (tmp = fgetc(adresy)) != '\n' && tmp != EOF; a++)
        {
            server_args.ip_address[i][a] = tmp;     //priradi nacitany char do suboru
        }
        
    }
    fclose(adresy);

    int server_count;
    //vpise vsetky nacitane adresy
    for (int i = 0; server_args.ip_address[i][0] != 0; i++)
    {
        server_count++;
        printf("%s\n", server_args.ip_address[i]);
    }
    

    //cyklus kde sa vytvroi novy thread, for sa opakuje kym nevycerpa vsetky adresy
    for (int i = 0; server_args.ip_address[i][0] != 0; i++)
    {
        server_args.num = i;
        printf("==========================================<%d>\n", server_args.num);
        pthread_create(&thread_id[i], NULL, server_thread, &server_args); //dopisat argumenty funkcie
        pthread_join(thread_id[i], NULL);
    }

    int live_counter = 0;
    for(int count = 0; count < 200; count++)
    {
        if(server_args.open_sockets[count] == 1)
            live_counter++;
    }
    if(live_counter == 0)
    {
        printf("==========================================\n");
        printf("connected/active servers: %d/%d\n", live_counter, server_count);
        printf("==========================================\n");
        printf("decentral shutdown\n");
        return 1;
    }
    else
    {
        char decentral[6][68] = {
        "  _____  ______ _____ ______ _   _ _______ _____            _ \n",
        " |  __ \\|  ____/ ____|  ____| \\|  |__   __|  __ \\     /\\   | |\n",
        " | |  | | |__ | |    | |__  |  \\| |  | |  | |__) |   /  \\  | |\n",
        " | |  | |  __|| |    |  __| | . ` |  | |  |  _  /   / /\\ \\ | |\n",
        " | |__| | |___| |____| |____| |\\  |  | |  | | \\ \\  / ____ \\| |____\n",
        " |_____/|______\\_____|______|_| \\_|  |_|  |_|  \\_\\/_/    \\_\\______|\n"};

        printf("==========================================\n");
        printf("connected/active servers: %d/%d\n", live_counter, server_count);
        printf("===================================================================\n");
        for (int i = 0; i < 6; i++)
        {
            printf("%s", decentral[i]);
        }
        printf("===================================================================\n");
    }
    
    char buffer[255];
    while (1)
    {

        bzero(buffer, sizeof(buffer));
        printf("you: ");
        fgets(buffer, sizeof(buffer), stdin);

        for (int i = 0; i < 200; i++)
        {
            if(server_args.open_sockets[i] == 1)
            {
                int a = write(server_args.server_sockets[i], buffer, sizeof(buffer));
                if(a < 0)
                {
                    fprintf(stderr, "ERROR: faild to send message\n");
                    return -1;
                }
                

            }
        }

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

    return 0;

}

int server_thread(thread_args *server_args){

    server_args->server_sockets[server_args->num] = socket(AF_INET, SOCK_STREAM, 0);
    if(server_args->server_sockets[server_args->num] < 0)
    {
        fprintf(stderr, "%d----ERROR: faild to crete socket\n", server_args->num);
        return -1;
    }
    printf("%d----socket was created :)\n", server_args->num);
    

    struct sockaddr_in server_addr;

    server_addr.sin_family = AF_INET;
    server_addr.sin_port = htons(8080);

    
    if(inet_pton(AF_INET, server_args->ip_address[server_args->num], &server_addr.sin_addr) <= 0)
    {
        
        fprintf(stderr, "%d----ERROR: invalid address\n", server_args->num);
        return -1;
    }
    printf("%d----ip addres is valid :)\n", server_args->num);
    printf("%d----connecting to address <%s>\n", server_args->num, server_args->ip_address[server_args->num]);
    

    
    int connect_status = connect(server_args->server_sockets[server_args->num], (struct sockaddr*) &server_addr, sizeof(server_addr));
    if (connect_status == -1)
    {
        //fprintf(stderr, "adresa:%s na %d\n", server_args->ip_address[server_args->num], server_addr.sin_addr);
        fprintf(stderr, "%d----ERROR: connection faild\n", server_args->num);
        return -1;
    }
    printf("%d----connection established\n", server_args->num);

    server_args->open_sockets[server_args->num] = 1;

    char buffer[255];
    recv(server_args->server_sockets[server_args->num], buffer, sizeof(buffer), 0);
    printf("server: %s\n", buffer);

    //char user[10] = "mario";
    write(server_args->server_sockets[server_args->num], server_args->login, sizeof(server_args->login));

}