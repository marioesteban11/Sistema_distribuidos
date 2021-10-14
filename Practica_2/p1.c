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
    set_name("p1");
    set_ip_port("127.0.0.1", 8000);

    //Conectamos con el server
    client_connection();

    //Inicializamos el thread para recibir mensajes
    init_recv_thread();

    notify_ready_shutdown();

    while(get_clock_lamport() < 5) {
        usleep(100);
    }
    notify_shutdown_ack();
    printf("SHOTDOWN\n");

    sleep(1);

    close_clients();

    return 0;
}