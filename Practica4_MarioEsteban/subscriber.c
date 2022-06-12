#include <stdio.h>
#include <stdlib.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <unistd.h>
#include <string.h>
#include <sys/select.h>
#include <pthread.h>
#include "proxy.h"
#include <getopt.h>
#include <signal.h>


int a = 0;
void manejador (int signum){
    a = 7;
}
int main(int argc, char *argv[])
{
    srand (time(NULL));
    char *topic = NULL, *ip_port = "127.0.0.1" ; 
    int opcion;
    int option_index = 0;
    int port_number = 0;

    static struct option long_options[] = {
             {"port ", required_argument, 0, 'p'},
             {"topic ", required_argument, 0, 't'},
             {"ip ", required_argument, 0, 'i'},
             {0, 0, 0, 0}
         };

    while ((opcion =  getopt_long (argc, argv, "p:t:i::",long_options, &option_index)) != -1){
        
        switch (opcion)
        {
            //mode
        case 't':
            topic = optarg;
            break;
            //port
        case 'p':
            port_number = strtol(optarg, NULL, 10);
            break;
        case 'i':
            ip_port = optarg;
        default:
            break;
        }
    }
    int tipo = 1;
    client_conection(ip_port, port_number, tipo);
    // suscripcion de un topic al broker
    int id_actual = 0, salida = 0;
    
    id_actual = topic_suscription(topic);
    int i = 0;
    while (i < 50){
        signal(SIGINT,manejador);
        if (a != 0){
            break;
        }
        salida = get_message(topic);
        if (salida != 0){
            break;
        }
        i++;
    }

    // Dexconexion de un topic del browser
    unfollow_topic(topic, id_actual);

    //printf("LOLOLOLOLOLOLOLOLO");
    close_client();
    return 0;
}