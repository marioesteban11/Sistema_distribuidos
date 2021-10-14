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
    return 0;
}