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
#define NUM_HILOS 100


void *threadfunction(void *arg)
{
    char buff[MAX];
    bzero(buff, MAX);
    int n; 
    int connfd = *(int *)arg;
    //printf("MACARRONES CON TOMATICO\n");
    fd_set readmask;
    struct timeval timeout;

    FD_ZERO(&readmask); // Reset la mascara
    FD_SET(connfd, &readmask); // Asignamos el nuevo descriptor
    FD_SET(STDIN_FILENO, &readmask); // Entrada
    timeout.tv_sec = 3; timeout.tv_usec = 500000; 

    select(connfd, &readmask, NULL, NULL, &timeout);
    if ((recv(connfd, (void*) buff, sizeof(buff), MSG_DONTWAIT)) > 0)
    {
        // Escribimos el contenido de buffer en la salida estandar.
        printf("+++  ");
        if (fputs(buff, stdout) == EOF) {
            fprintf(stderr, "\n Error : Fputs error\n");
        }
    }
    
    //enviamos el mensaje al cliente
    char sendBuff[MAX];
    bzero(sendBuff, MAX); 
    printf(">  ");
    char msg[MAX] = "Hello client!  \r\n";
    //strncat(sendBuff, msg, sizeof(msg));
    send(connfd, msg, strlen(msg), 0); 
    // Cerramos el socket.
    close(connfd);
}

int main(int argc, char *argv[])
{
    int sockfd = 0, connfd = 0;
    struct sockaddr_in serv_addr;

    fd_set readmask;
    struct timeval timeout;

    char buff[MAX];
    bzero(buff, MAX);

    pthread_t thread[NUM_HILOS];
    
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

    while(1) {
        int pos;
        //printf("Hola, estamos FUERA del thread\n");
        for (int i = 0; i < NUM_HILOS; i++) {
            // El servidor quedará a la espera y aceptará a los clientes que seconecten al socket.
            connfd = accept(sockfd, (struct sockaddr*) NULL, NULL);
            if (connfd < 0) {
                printf("Server accept failed...\n");
                exit(1);
            } else {
                printf("Server accepts the client...\n");
            }

            pos = i;
            //creamos los threads para aceptar y mandar los mensajes a los clientes
            if(pthread_create(&thread[i], NULL, threadfunction, &connfd) > 0)
            {
                printf("error al cerrar el thread");
                return 1;
            }
        }

        //Una vez recibidos los 100 clientes cerramos todos los hilos
        for(int i = 0; i > NUM_HILOS; i++)
        {

            if (pthread_join(thread[i],NULL) > 0)
            {
                printf("error al cerrar el thread");
                return 1;
            }
        }

    }
    
    if (close(sockfd) < 0 ) {
        printf("No se ha cerrado el cliente correctamente\n");
        exit(1);

    }
    return 0;
}