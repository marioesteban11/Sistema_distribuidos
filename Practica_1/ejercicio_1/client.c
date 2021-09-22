#include <stdio.h>
#include <stdlib.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <unistd.h>
#include <string.h>
#include <sys/select.h>


#define MAX 256
#define PORT 8080

int main(int argc, char *argv[])
{
    int sockfd = 0, connfd = 0;
    struct sockaddr_in servaddr;

    fd_set readmask;
    struct timeval timeout;

    // Creamos un socket TCP y comprobamos que se ha creado correctamente
    sockfd = socket(AF_INET, SOCK_STREAM, 0);
    if (sockfd == -1) {
        printf("Socket creation failed...\n");
        exit(1);
    }else {
        printf("Socket successfully created...\n");
    }

    // Le asignamos una IP y un puerto
    bzero(&servaddr, sizeof(servaddr));
    servaddr.sin_family = AF_INET;
    servaddr.sin_addr.s_addr = inet_addr("127.0.0.1");
    servaddr.sin_port = htons(PORT);

    // Conectamos el cliente al socket del servidor y comprobamos
    if ((connect(sockfd, (struct sockaddr*)&servaddr, sizeof(servaddr))) < 0) {
        printf("Connection  with the server failed...\n");
        exit(1);
    } else {
        printf("Socked conected to server  \n");
    }

    char sendBuff[MAX];
    bzero(sendBuff, MAX);
    //mandamos el mensaje al servidor
    char msg[MAX] = "Hello server! ";
    // strncat(msg, argv[1], sizeof(argv[1]));
    //strncat(sendBuff, msg, sizeof(msg));
    //strncat(sendBuff, "\n", sizeof("\n"));
    
    send(sockfd, msg, strlen(msg), 0); 
    printf("> ");
    printf("  %s\n", msg  );

    char buff[MAX];
    bzero(buff, MAX);
    //FD_ZERO(&readmask); // Reset la mascara
    //FD_SET(sockfd, &readmask); // Asignamos el nuevo descriptor
    //FD_SET(STDIN_FILENO, &readmask); // Entrada
    //timeout.tv_sec=3; timeout.tv_usec=500000; // Timeout de 0.1 seg.


    //select(sockfd, &readmask, NULL, NULL, &timeout);
    if ((recv(sockfd, (void*) buff, sizeof(buff), 0)) > 0)
    {
        // Escribimos el contenido de buffer en la salida estandar.
        printf("+++  ");
        if (fputs(buff, stdout) == EOF) {
            fprintf(stderr, "\n Error : Fputs error\n");
        }
        printf("\n");
    }
    if (close(sockfd) < 0) {
        printf("No se ha cerrado el cliente correctamente\n");
        exit(1);

    }
    // Cerramos el socket.
    
}