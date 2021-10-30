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
    // Miramos si se pasa como argumento un lector o un escritor
    // y miramos el numero de threads/clientes que van a ser ejecutados
    //if (argc < 4)
    //{
    //    printf("Introduzca el numero correcto de argumentos");
    //    return -1;
    //}
    //client_conection("127.0.0.1", 8000);
//
    //semaforo();
    //if (strcmp(argv[1], "--mode") == 0 && strcmp(argv[3], "--threads") == 0)
    //{
    //    if(set_client(argv[2]) == WRITE){
    //        printf("writer yes papa\n");
    //        
    //        set_reader_or_client(argv[4], WRITE);
    //    }
    //    else if (set_client(argv[2]) == READ){
//
    //        printf("reader yes papa\n");
    //        set_reader_or_client(argv[4], READ);
    //    }
    //}

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

    return 0;
}