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
    printf("a\n");
    a = 7;
}

int main(int argc, char *argv[])
{
    char *mode = NULL; 
    int opcion;
    int option_index = 0;
    int port_number = 0;

    static struct option long_options[] = {
             {"port ", required_argument, 0, 'p'},
             {"mode ", required_argument, 0, 'm'},
             {0, 0, 0, 0}
         };

    while ((opcion =  getopt_long (argc, argv, "p:m::",long_options, &option_index)) != -1){
        
        switch (opcion)
        {
            //mode
        case 'm':
            mode = optarg;
            break;
            //port
        case 'p':
            port_number = strtol(optarg, NULL, 10);
            break;
        default:
            break;
        }
    }

    server_conection(port_number);
    semaforo();
    while (1) {
        //signal(SIGINT,manejador);
        //printf("a\n");
        //if (a != 0){
        //    break;
        //}
        int modo = aceptar_cliente(mode);
    }

    close_server();
    return 0;
}