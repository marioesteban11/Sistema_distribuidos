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
    char *priority = NULL; 
    int opcion;
    int option_index = 0;
    int ratio = 0, port_number = 0;

    static struct option long_options[] = {
             {"port ", required_argument, 0, 'p'},
             {"priority ", required_argument, 0, 't'},
             {"ratio ", required_argument, 0, 'r' },
             {0, 0, 0, 0}
         };

    while ((opcion =  getopt_long (argc, argv, "p:t:r::",long_options, &option_index)) != -1){
        
        switch (opcion)
        {
            //ratio
        case 'r':
            ratio = strtol(optarg, NULL, 10);
            break;
            //priority
        case 't':
            priority = optarg;
            break;
            //port
        case 'p':
            port_number = strtol(optarg, NULL, 10);
            break;
        default:
            break;
        }
    }
    printf("las opciones elegidas para este programa son: priority: %s, port: %d y ratio: %d\n", priority, port_number, ratio);
    FILE* file;

    server_conection("127.0.0.1", port_number);

    //Creamos el fichero
    file = fopen("server_output.txt", "w"); 

    //Inicializamos todos los semaforos
    semaforo();
    while (1)
    {
        int clients = aceptar_cliente();
        //printf("Ratio%d\n", ratio);
        //printf("Numero threads%s\n\n\n", argv[2]);
        
        if(strcmp(priority, "writer") == 0)
        {
            //printf("albaricoque\n");
            //Si nos pasan clientes como prioridad
            seleccionar_prioridad(clients, ratio, priority); 
            
        }
        else if(strcmp(priority, "reader") == 0)
        {
            //Si nos pasan clientes como prioridad
            seleccionar_prioridad(clients, ratio, priority); 
        }
    }
    fclose(file);

    close_server();
    return 0;
}
