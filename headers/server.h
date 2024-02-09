#ifndef __CLIENT
#define __CLIENT

void *server();  //hlavna funkcia serveru

void *client_thread(void *client_socket);      //funkcia ktoru vola server
#endif