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
    char *mode = NULL, *ip = NULL; 
    int opcion;
    int option_index = 0;
    int threads = 0, port_number = 0;

    static struct option long_options[] = {
             {"mode ", required_argument, 0, 'm'},
             {"threads ", required_argument, 0, 't'},
             {"ip ", required_argument, 0, 'i' },
             {"port ", required_argument, 0, 'p'},
             {0, 0, 0, 0}
         };

    while ((opcion =  getopt_long (argc, argv, "m:t:i:p::",long_options, &option_index)) != -1){
        
        switch (opcion)
        {
        case 'm':
            mode = optarg;
            break;
        case 't':
            threads = strtol(optarg, NULL, 10);
            break;
        case 'p':
            //port_number = optarg;
            port_number = strtol(optarg, NULL, 10);
            break;
        case 'i':
            if (optarg == "(null)"){
                printf("Select a ip_port\n");
                exit(1);
            }
            ip = optarg;
        default:
            break;
        }
    }
    printf("Esto es el modo en el que estamos:  %s  y esto es el numero de threads: %d, esto el ip; %s, y esto el puerto: %d \n", mode, threads, ip, port_number);


    // Miramos si se pasa como argumento un lector o un escritor
    // y miramos el numero de threads/clientes que van a ser ejecutados
    client_conection(ip, port_number);

    //semaforo();

    if(strcmp(mode, "writer") == 0) {
        printf("pantuflas\n");
        set_reader_or_client(threads, WRITE);
    }
    else if (strcmp(mode, "reader") == 0){
        set_reader_or_client(threads, READ);
    }

    close_client();

    return 0;
}