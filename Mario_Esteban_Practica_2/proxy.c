#include "proxy.h"

//Variables para el cliente y el servidor
int sockfd = 0, connfd_p1 = 0, connfd_p2 = 0, connfd_p3 = 0;
struct sockaddr_in servaddr;
struct sockaddr_in serv_addr;

struct message message;

pthread_t thread;

// Actualizamos el reloj de lamport
unsigned int lamport_increase(struct message lamport) {
    unsigned int number_lamport;
    if (lamport.clock_lamport < message.clock_lamport) {
        number_lamport = message.clock_lamport;
    }else {
        number_lamport = lamport.clock_lamport;
    }
    message.clock_lamport = number_lamport;
    message.clock_lamport++;

    return message.clock_lamport;
}

// Establece el nombre del proceso (para los logs y trazas)
void set_name (char name[2]){
    strcpy(message.origin, name);
}
// Establecer ip y puerto
void set_ip_port (char* ip, unsigned int port){

    bzero(&serv_addr, sizeof(serv_addr));
    serv_addr.sin_family = AF_INET;
    serv_addr.sin_addr.s_addr = htonl(INADDR_ANY);
    serv_addr.sin_port = htons(port);

    bzero(&servaddr, sizeof(servaddr));
    servaddr.sin_family = AF_INET;
    servaddr.sin_addr.s_addr = inet_addr(ip);
    servaddr.sin_port = htons(port);
}
// Obtiene el valor del reloj de lamport.
// Utilízalo cada vez que necesites consultar el tiempo.
int get_clock_lamport(){
    return message.clock_lamport;
}
// Notifica que está listo para realizar el apagado (READY_TO_SHUTDOWN)
void notify_ready_shutdown(){
    struct message ready;
    // Copiamos el nombre de nuestra maquina para asi poder mandarlo
    strcpy(ready.origin, message.origin); 
    // Cogemos los relojes de lamport oportunos para cada momento
    message.clock_lamport++;
    ready.clock_lamport = message.clock_lamport;
    //Decimos que nuestro pc esta listo para apagarse
    ready.action = READY_TO_SHUTDOWN;

    printf("%s, %d, SEND, READY_TO_SHUTDOWN\n", ready.origin, ready.clock_lamport);

        // Enviamos la struct por el socket
    if (send(sockfd, &ready, sizeof(ready), 0) < 0) {
        printf("Send to the server failed...\n");
    }


}
// Notifica que va a realizar el shutdown correctamente (SHUTDOWN_ACK)
void notify_shutdown_ack(){
    struct message ready;

    // Copiamos el nombre de nuestra maquina para asi poder mandarlo
    strcpy(ready.origin, message.origin); 

    // Cogemos los relojes de lamport oportunos para cada momento
    message.clock_lamport++;
    ready.clock_lamport = message.clock_lamport;
    //Decimos que nuestro pc esta listo para apagarse
    ready.action = SHUTDOWN_ACK;

    printf("%s, %d, SEND, SHUTDOWN_ACK\n", ready.origin, ready.clock_lamport);
    // Enviamos la struct por el socket
    if (send(sockfd, &ready, sizeof(ready), 0) < 0)
    {
        printf("Send to the server failed...\n");
    }
}


//Recibe el READY_TO_SHOTDOWN de los clientes
int wait_client_shotdown() {
    struct message receive;
    // Aceptamos el cliente
    connfd_p2 = accept(sockfd,(struct sockaddr*)NULL, NULL);
    if (connfd_p2 < 0){
        printf("Server accept failed...\n");
        exit(1);
    } else {
        printf("Server accepts the client...\n");
    }
    // Esperamos a recibir un mensaje
    if ((recv(connfd_p2, &receive, sizeof(receive),0)) < 0) 
    {
        printf("Recv from the client failed...\n");
    }else {
        // Esperamos Ready_to_shutdown
        if(receive.action == READY_TO_SHUTDOWN) 
        {
            // Reajustamos lamport
            receive.clock_lamport = lamport_increase(receive); 
            printf("%s, %d, RECV (%s), READY_TO_SHUTDOWN\n", message.origin, receive.clock_lamport, receive.origin);
        } else {
            printf("This is not the correct action\n");
            }   
    }
    //De que cliente es el mensaje 
    if(strcmp(receive.origin, "p1") == 0 ) {
        connfd_p1 = connfd_p2;
    }else if(strcmp(receive.origin, "p3") == 0 ) { 
        connfd_p3 = connfd_p2;
    }
    return 0;
}

int server_wait_shotdown_ack(struct message ack) {
    if (message.clock_lamport == 6) {
        connfd_p2 = connfd_p1;
    }
    else if (message.clock_lamport == 10) {
        connfd_p2 = connfd_p3;
    }
    // Esperamos al ack del cliente
    if ((recv(connfd_p2, &ack, sizeof(ack),0)) < 0) 
    {
        printf("Recv from the client failed...\n");
    } else {
        //Esperamos el shotdown ack
        if(ack.action == SHUTDOWN_ACK) {
            //  Reajustamos lamport
            ack.clock_lamport = lamport_increase(ack); 
            printf("%s, %d, RECV (%s), SHUTDOWN_ACK\n", message.origin, ack.clock_lamport, ack.origin);
        }else {
            printf("Wrong operation\n");
        }
    }
    return 0;
}

//Mandar SHOTDOWN_NOW del servidor a los clientes
int server_send_shotdown(char name[2]) {
    struct message ack;

    //QUe cliente es el que manda el mensaje
    strcpy(ack.origin, message.origin); 
    // Actualizamos lamport
    message.clock_lamport++; 
    ack.clock_lamport = message.clock_lamport;
    // Enviamos el SHOTDOWN_NOW
    ack.action = SHUTDOWN_NOW; 

    printf("%s, %d, SEND, SHUTDOWN_NOW (%s)\n",ack.origin, ack.clock_lamport, name);

    if (ack.clock_lamport == 4 ) {
        connfd_p2 = connfd_p1;
    } 
    else if (ack.clock_lamport == 8) {
        connfd_p2 = connfd_p3;
    }
    if (send(connfd_p2, &ack, sizeof(ack), 0) < 0)
    {
        printf("Send to the client failed...\n");
    }
    server_wait_shotdown_ack(ack);
}


//Conexiones para unir el servidor con el cliente
int client_connection() {
    sockfd = socket(AF_INET, SOCK_STREAM, 0);
    if(sockfd == -1) {
        printf("Socket creation failed...\n");
        exit(1);
    } else {
        printf("Socket successfully created...\n");
    }


    if((connect(sockfd,(struct sockaddr*)&servaddr,sizeof(servaddr))) < 0){
        printf("Connection with the server failed...\n");
        exit(1);
    } else{
        printf("Connected to the server...\n");
    }
    return 0;
}

int server_connection(){

    sockfd = socket(AF_INET, SOCK_STREAM, 0);
    if(sockfd == -1) {
        printf("Socket creation failed...\n");
        exit(1);
    } else {
        printf("Socket successfully created...\n");
    }
    if((bind(sockfd,(struct sockaddr*)&serv_addr,sizeof(serv_addr))) != 0){
        printf("Socket bind failed...\n");
        exit(1);
    } else {
        printf("Socket successfully binded...\n");
    }
    if((listen(sockfd, 2)) != 0){
        printf("Listen failed...\n");
        exit(1);
    } else {
        printf("Server listening...\n");
    }
    return 0;
}

void *client_wait_message() {
    while(1)
    {
        struct message shutdown;
        usleep(5000);
        if ((recv(sockfd, &shutdown, sizeof(shutdown),MSG_DONTWAIT)) > 0)
        {
            if(shutdown.action == SHUTDOWN_NOW) 
            {
                shutdown.clock_lamport = lamport_increase(shutdown);
                printf("%s, %d, RECV (%s), SHUTDOWN_NOW\n", message.origin, shutdown.clock_lamport, shutdown.origin);
                break;
            } else {
                printf("Wrong operation\n");
            }
        }

    }

}
//Iniciamos el thread para escuchar
void init_recv_thread () {
    pthread_create(&thread, NULL, client_wait_message, NULL);
}


//Cerramos conexiones 

int close_server() {
    if(close(connfd_p2) == -1)
    {
        printf("Close failed\n");
        exit(1);
    }
    if(close(sockfd) == 1)
    {
        printf("Close failed\n");
        exit(1);
    }
  return 0;
}

int close_clients(){

    if(pthread_join(thread, NULL) != 0)
    {
      printf("join failed\n");
      exit(1);
    }
    if(close(connfd_p2) == -1)
    {
        printf("Close failed\n");
        exit(1);
    }
    if(close(sockfd) == 1)
    {
        printf("Close failed\n");
        exit(1);
    }
    printf("llegas");
    return 0;
}