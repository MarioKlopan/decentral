#include <time.h>
#include <pthread.h>
#include "server.h"
#include "client.h"


int main(int argc, char *argv[]){
    
    pthread_t thread_id[2];
    pthread_create(&thread_id[0], NULL, server, argv[2]);
    sleep(2);
    pthread_create(&thread_id[1], NULL, client, argv[3]);
    
    pthread_join(thread_id[1], NULL);   //caka sa na klienta kym skonci potom sa skonci program
    return 0;
}