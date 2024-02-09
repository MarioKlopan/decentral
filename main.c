#include <time.h>
#include <pthread.h>
#include <stdio.h>
#include <string.h>
#include "server.h"
#include "client.h"



int main(int argc, char *argv[]){

    printf("password: ");
    char password[30];
    scanf("%s", password);

    FILE *login_file;
    login_file = fopen("login.txt", "r");
    char tmp[30] = {};
    fscanf(login_file, "%s", tmp);
    char origin_pass[30] = {};
    fscanf(login_file, "%s", origin_pass);
    fclose(login_file);
    int accesss = strcmp(origin_pass, password);
    if(accesss == 0)
        printf("correct password\n");    



    
    pthread_t thread_id[2];
    pthread_create(&thread_id[0], NULL, server, NULL);
    sleep(2);
    pthread_create(&thread_id[1], NULL, client, argv[1]);
    
    pthread_join(thread_id[1], NULL);   //caka sa na klienta kym skonci potom sa skonci program
    return 0;
}