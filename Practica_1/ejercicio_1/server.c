#include <stdio.h>
#include <stdlib.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <unistd.h>
#include <string.h>
#include <sys/select.h>

#include <pthread.h>

#define MAX 256
#define PORT 8080


int main(int argc, char *argv[])
{
    int sockfd = 0, connfd = 0;
    struct sockaddr_in serv_addr;

    fd_set readmask;
    struct timeval timeout;

    char buff[MAX];
    bzero(buff, MAX);
    
    // Creamos un socket TCP y comprobamos que se ha creado correctamente
    sockfd = socket(AF_INET, SOCK_STREAM, 0);
    if (sockfd == -1) {
        printf("Socket creation failed...\n");
        exit(1);
    }else {
        printf("Socket successfully created...\n");
    }
    // Creamos la IP y el puerto
    bzero(&serv_addr, sizeof(serv_addr));
    serv_addr.sin_family = AF_INET;
    serv_addr.sin_addr.s_addr = htonl(INADDR_ANY);
    serv_addr.sin_port = htons(PORT);
    // Asignamos la IP creada al socket y comprobamos
    if ((bind(sockfd, (struct sockaddr*)&serv_addr, sizeof(serv_addr))) != 0) {
        printf("Socket bind failed...\n");
        exit(1);
    } else {
        printf("Socket successfully binded...\n");
    }
    /* Ponemos el servidor a escuchar. El segundo argumento de listen indica el número
        * de clientes que se encolarán en el socket, los siguientes se descartarán.
        */  
    if ((listen(sockfd, 100)) != 0) {
            printf("Listen failed...\n");
            exit(1);
        } else {
            printf("Server listening...\n");
        }

    
    // El servidor quedará a la espera y aceptará a los clientes que seconecten al socket.
    connfd = accept(sockfd, (struct sockaddr*) NULL, NULL);
    if (connfd < 0) {
        printf("Server accept failed...\n");
        exit(1);
    } else {
        printf("Server accepts the client...\n");
    }

    if ((recv(connfd, (void*) buff, sizeof(buff), 0)) > 0)
    {
        // Escribimos el contenido de buffer en la salida estandar.
        printf("+++  ");
        if (fputs(buff, stdout) == EOF) {
            fprintf(stderr, "\n Error : Fputs error\n");
        }
        printf("\n");
    }
    
    //enviamos el mensaje al cliente
    printf(">  ");
    char msg[MAX]; 
    fgets(msg, MAX, stdin );
    send(connfd, msg, strlen(msg), 0); 
    
    if (close(sockfd) < 0 || close(connfd) < 0) {
        printf("No se ha cerrado el cliente correctamente\n");
        exit(1);

    }
    return 0;
}