#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <unistd.h>
#include <pthread.h>

typedef struct thread_args{
    char login[100];
    int server_sockets[200];
    char ip_address[200][16];
    int open_sockets[200];
    int num;
}thread_args;

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


int main(int argc, char const* argv[]){
    
    thread_args server_args;
    /*server_args.server_sockets[200];
    server_args.ip_address[200][15] = {};
    server_args.open_sockets[200] = {};*/
    pthread_t thread_id[200];
    bzero(server_args.server_sockets, sizeof(server_args.server_sockets));
    bzero(server_args.ip_address, sizeof(server_args.ip_address));
    bzero(server_args.open_sockets, sizeof(server_args.open_sockets));
    //bzero(server_args.login, sizeof(server_args.login));

    strcat(server_args.login, argv[2]);
    //    server_args.login = argv[2];

    FILE* adresy;
    adresy = fopen(argv[1], "r");
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