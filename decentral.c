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
void client_thread(int client_socket);      //funkcia ktoru vola server
int server_thread(thread_args *server_args);    //funkciu ktoru vola client

int main(int argc, char *argv[]){
    
    pthread_t thread_id[2];
    fprintf(stderr, "server starting .....\n");
    pthread_create(&thread_id[0], NULL, server, argv[2]);

    fprintf(stderr, "client starting .....\n");
    pthread_create(&thread_id[1], NULL, client, argv[3]);
    
    pthread_join(thread_id[1], NULL);   //caka sa na klienta kym skonci potom sa skonci program
    return 0;
}

int server(char *port){

    printf("booting srever .....\n");

    int serv_sock = socket(AF_INET, SOCK_STREAM, 0);    //socket serveru
    if(serv_sock < 0)       //kontrola navratovej hodnoty funkcie socket
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

    int clients[200]; //pole kde sa vytvoria sockety ked sa pripoji klient
    bzero(clients, sizeof(clients));    //vycistenie pola cielnts
    int num = 0;    //sluzi na indexovanie v poli
    pthread_t thread_id[200];   //pole pre id threadov

    while(1)
    {
        
        printf("listening .......\n");
        if(listen(serv_sock, 1) < 0)    //funkcia listen caka na spojenie akonahle sa pripoji niekto tak sa pripoji na socket ktory bol vytvoreny
            perror("error with listening");

        clients[num] = accept(serv_sock, NULL, NULL);   //funkcia accept vytvory novy socket s klientom a hlavy socket sa uvolni pre dalsich klientov ktory sa chcu pripojit
        send(clients[num], msg, sizeof(msg), 0);    //send posle kontrolnu spravu klientovy aby vedel ze je uspesne pripojeny
        pthread_create(&thread_id[num], NULL, client_thread, clients[num]); //vytvori sa novy thread s pripojenym klientom
        num++;  //posunie sa poradovnik
        
    }

    return 0;
}

//funkcia ktora sluzi na prijmanie sparv, funkcia je spustena v novom threade
void client_thread(int client_socket){

    char user[100] = {};     //pole na uzivatelske meno
    recv(client_socket, user, sizeof(user), 0);     //prva sprava od klienta je pasivne poslana a je to uzivatelske meno

    
    printf("accepted, new user: %s\n", user);  

    

    char buffer[255];   //pole na zapisanie prichadzajucich sprav
    int a = 0;  //uklada navratovu hodnotu funkcie recv()
    while (1)
    {
        bzero(buffer, sizeof(buffer));  //vycisti pole kde sa zapisuju prijate spravy

        a = recv(client_socket, buffer, sizeof(buffer), 0);     //funkcia prijma spravy
        
        if(a == 0)
        {
            fprintf(stderr, "ERROR: client was shutdowned, user: %s\n", user);
            break;
        } 
        
        printf("%s: %s", user, buffer);

        if(strncmp("//quit", buffer, 6) == 0)   //pokial client napise //quit tak ukoncuje spojenie, thread sa zavrie
        {
            printf("closing connection with user: %s\n", user);
            break;
            

        }
    }
    printf("closed connection with user: %s\n", user);
    close(client_socket);

}

int client(char *login){
    
    printf("booting client .....\n");

    thread_args server_args;    //vytovrenie struktury v ktorej su ulozene informacie o serveroch
    
    pthread_t thread_id[200];   //pole na vytvorenie id pre thready

    //precistenie poli v strukture
    bzero(server_args.server_sockets, sizeof(server_args.server_sockets));
    bzero(server_args.ip_address, sizeof(server_args.ip_address));
    bzero(server_args.open_sockets, sizeof(server_args.open_sockets));

    strcat(server_args.login, login);   //vlozenie loginu do struktury

    //nacitanie adries zo suboru
    FILE* adresy;
    adresy = fopen("adresy.txt", "r");      //TODO - dynamicke zadanie suboru
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
        pthread_create(&thread_id[i], NULL, server_thread, &server_args); //funkcia na vytvorenie noveho threadu
        pthread_join(thread_id[i], NULL);   //caka sa kym funkcia v novom threade skonci
    }

    int live_counter = 0;   //uklada pocet serverov s kotrymi bolo naviazane spojenie
    for(int count = 0; count < 200; count++)
    {
        if(server_args.open_sockets[count] == 1)
            live_counter++;
    }
    if(live_counter == 0)   //ak je live_counter nula tak su vsetky servre vypnute
    {
        printf("==========================================\n");
        printf("connected/active servers: %d/%d\n", live_counter, server_count);
        printf("==========================================\n");
        printf("decentral shutdown\n");
        return 1;
    }
    else    //inak bolo naviazane nejake uspesne spojenie a uzivatel moze pisat, zobrazi sa mu logo a pocet pripojenych serverov zo vsetkch
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
    
    char buffer[255];   //pole na ukladanie sprav ktore sa odoslu
    while (1)
    {

        bzero(buffer, sizeof(buffer));  //vycistenie pola
        printf("you: ");    //koli rozliseniu v chate
        fgets(buffer, sizeof(buffer), stdin);   

        for (int i = 0; i < 200; i++)   //cyklus sluzi na rozposlanie sprav iba aktivnym spojeniam
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

    return 0;

}

//funkcia sluzi na vytvorenie socketu a nadviazanie spojenia so serverom
int server_thread(thread_args *server_args){

    server_args->server_sockets[server_args->num] = socket(AF_INET, SOCK_STREAM, 0);    //vytvorenie socketu
    if(server_args->server_sockets[server_args->num] < 0)   //kontrola vytovrenia socketu
    {
        fprintf(stderr, "%d----ERROR: faild to crete socket\n", server_args->num);
        return -1;
    }
    printf("%d----socket was created :)\n", server_args->num);
    

    struct sockaddr_in server_addr; //vytorenie structuri ktora sa dava spojeniu aby vedelo s kym a ako komunikovat

    server_addr.sin_family = AF_INET;
    server_addr.sin_port = htons(8080);     //TODO - dynamicky pridelovat port

    
    if(inet_pton(AF_INET, server_args->ip_address[server_args->num], &server_addr.sin_addr) <= 0)   //funkcia na prelozenie ip adresy
    {
        
        fprintf(stderr, "%d----ERROR: invalid address\n", server_args->num);
        return -1;
    }
    printf("%d----ip addres is valid :)\n", server_args->num);
    printf("%d----connecting to address <%s>\n", server_args->num, server_args->ip_address[server_args->num]);
    

    
    int connect_status = connect(server_args->server_sockets[server_args->num], (struct sockaddr*) &server_addr, sizeof(server_addr)); //funkcia na vytvorenie spojenia
    if (connect_status == -1)
    {
        fprintf(stderr, "%d----ERROR: connection faild\n", server_args->num);
        return -1;
    }
    printf("%d----connection established\n", server_args->num);

    server_args->open_sockets[server_args->num] = 1;    //pokial sa klient pripojil na server tak to zaznaci v tabulke ako 1

    char buffer[255];
    recv(server_args->server_sockets[server_args->num], buffer, sizeof(buffer), 0); //server ako prvu spravu posle kontrolnu spravu ze uzivatel bol pripojeny
    printf("server: %s\n", buffer);

    write(server_args->server_sockets[server_args->num], server_args->login, sizeof(server_args->login));   //klient ako prvu spravu posle na server svoju indentifikaciu

}