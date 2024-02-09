//structura ktora sluzi na predanie argumentov pre funkciu kotra vytvori novy thread
typedef struct thread_args{
    char login[100];            //login zada uzivatel pri spusteni programu
    int server_sockets[200];    //tu sa ukaladaju vsetky vytvorene sockety na ktore sa klient pripoji
    char ip_address[200][16];   //tu sa ukladaju naciatne ip adresy zo suboru
    int open_sockets[200];      //na zanamenavanie pripojenych serverov, poradie odpoveda ipadrese a socketu, 0 = neotvoreny, 1 = otvoreny
    int num;                    //num urcuje index pre polia, aby vo funkcii program vedel ktory parameter pouzit
}thread_args;

void *client(void *login);  //hlavna funkcia klienta
int server_thread(thread_args *server_args);    //funkciu ktoru vola client