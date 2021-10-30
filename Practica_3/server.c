#include <stdio.h>
#include <stdlib.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <unistd.h>
#include <string.h>
#include <sys/select.h>

#include <pthread.h>

#include "proxy.h"

int main(int argc, char *argv[])
{
    FILE* file;


    if (argc != 3)
    {
        printf("Introduzca el numero correcto de argumentos\n");
        return -1;
    }

    if (strcmp(argv[1], "--priority") == 0)
    {
        server_conection("127.0.0.1", 8000);
    }

    //Creamos el fichero
    file = fopen("server_output.txt", "w"); 

    // SI existe el parametro ratio

    int ratio = 0;
    if(argc == 5)
    {
        ratio = atoi(argv[4]);
    }
    //Inicializamos todos los semaforos
    semaforo();
    while (1)
    {
        int clients = aceptar_cliente();
        printf("Ratio%d\n", ratio);
        //printf("Numero threads%s\n\n\n", argv[2]);
        
        if(strcmp(argv[2], "writer") == 0)
        {
            //Si nos pasan clientes como prioridad
            seleccionar_prioridad(clients, ratio, argv[2]); 
            
        }
        else if(strcmp(argv[2], "reader") == 0)
        {
            //Si nos pasan clientes como prioridad
            seleccionar_prioridad(clients, ratio, argv[2]); 
        }
    }
    fclose(file);
    
    return 0;
}
