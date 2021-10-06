#include "proxy.h"

//Variables para el cliente y el servidor
int sockfd = 0, connfd = 0;
int connfd_writers = 0, connfd_readers = 0;
struct sockaddr_in servaddr;
struct sockaddr_in serv_addr;
struct response servidor;


// Establece el nombre del proceso (para los logs y trazas)
void set_name (char name[2]){
    
}
// Establecer ip y puerto
void set_ip_port (char* ip, unsigned int port){

}
// Obtiene el valor del reloj de lamport.
// Utilízalo cada vez que necesites consultar el tiempo.
int get_clock_lamport(){

}
// Notifica que está listo para realizar el apagado (READY_TO_SHUTDOWN)
void notify_ready_shutdown(){

}
// Notifica que va a realizar el shutdown correctamente (SHUTDOWN_ACK)
void notify_shutdown_ack(){

}



int client_conection(char* ip, int port){

    // Creamos un socket TCP y comprobamos que se ha creado correctamente
    sockfd = socket(AF_INET, SOCK_STREAM, 0);
    if (sockfd == -1) {
        printf("Socket creation failed...\n" );
        exit(1);
    }else {
        printf("Socket successfully created...\n" );
    }

    // Le asignamos una IP y un puerto
    bzero(&servaddr, sizeof(servaddr));
    servaddr.sin_family = AF_INET;
    servaddr.sin_addr.s_addr = inet_addr(ip);
    servaddr.sin_port = htons(port);


    // Conectamos el cliente al socket del servidor y comprobamos
    if ((connect(sockfd, (struct sockaddr*)&servaddr, sizeof(servaddr))) < 0) {
        printf("Connection with the server failed...\n");
        exit(1);
    } else {
        printf("Socked conected to server  \n");
    }

    return 0;
}

int server_conection(char* ip,int port ){

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
    serv_addr.sin_addr.s_addr = inet_addr(ip);
    serv_addr.sin_port = htons(port);
    // Asignamos la IP creada al socket y comprobamos
    if ((bind(sockfd, (struct sockaddr*)&serv_addr, sizeof(serv_addr))) != 0) {
        printf("Socket bind failed...\n");
        exit(1);
    } else {
        printf("Socket successfully binded...\n");
    }
    
    if ((listen(sockfd, 100)) != 0) {
            printf("Listen failed...\n");
            exit(1);
    } else {
        printf("Server listening...\n");
    }
}

int aceptar_cliente()
{

    struct num_threads{
        int threads;
        int opcion;
    }num_clientes;
    connfd = accept(sockfd, (struct sockaddr*)NULL, NULL); //Acepta un nuevo cliente
    if (connfd < 0){
        printf("Server accept failed...\n");
        exit(1);
    } else {
        printf("Server accepts the client...\n");
    }

    int n;

    if ((n = recv(connfd, &num_clientes, sizeof(num_clientes), 0)) < 0) 
    {
        printf("Recv from the client failed...\n");
    }
    return 0;
}