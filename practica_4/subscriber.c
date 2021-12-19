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


int main(int argc, char *argv[])
{
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
    printf("las opciones elegidas para este programa son: topic: %s, port: %d y ip: %s\n", topic, port_number, ip_port);
    
    client_conection(ip_port, port_number);

    topic_suscription(topic);

    close_client();
    return 0;
}