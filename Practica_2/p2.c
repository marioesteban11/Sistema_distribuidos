#include <stdio.h>
#include <stdlib.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <unistd.h>
#include <string.h>
#include <sys/select.h>

#include "proxy.h"

#define MAX 256
#define PORT 8080

int main(int argc, char *argv[])
{
    set_name("p2");
    set_ip_port("127.0.0.1", 8000);
    //COnectamos con los clientes
    server_connection();

    //Recibimos los dos clientes (tiene que dar igual el orden en los que se reciba)
    wait_client_shotdown();
    wait_client_shotdown();

    //Mandamos el shotdown now primero a p1 y luego p3

    server_send_shotdown("p1");
    server_send_shotdown("p3");

    printf("Clientes fueron correctamente apagados en t(lamport) = %d\n", get_clock_lamport());
    //Una vez que ya hemos terminado todo cerramos las conexiones
    sleep(2);
    close_server();
    return 0;
    
}