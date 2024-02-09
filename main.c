#include <time.h>
#include <pthread.h>
#include <stdio.h>
#include <string.h>
#include "headers/server.h"
#include "headers/client.h"



int main(int argc, char *argv[]){    
    pthread_t thread_id[2];
    pthread_create(&thread_id[0], NULL, server, NULL);
    sleep(2);
    pthread_create(&thread_id[1], NULL, client, argv[1]);
    
    pthread_join(thread_id[1], NULL);   //caka sa na klienta kym skonci potom sa skonci program
    return 0;
}