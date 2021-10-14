#include "proxy.h"

//Variables para el cliente y el servidor
int sockfd = 0, connfd_p1 = 0;
int connfd_p2 = 0, connfd_p3 = 0;
struct sockaddr_in servaddr;
struct sockaddr_in serv_addr;

struct message message;

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

}
// Notifica que está listo para realizar el apagado (READY_TO_SHUTDOWN)
void notify_ready_shutdown(){
    struct message ready;

    // Copiamos el nombre de nuestra maquina para asi poder mandarlo
    strcpy(ready.origin, message.origin); 
    // Cogemos los relojes de lamport oportunos para cada momento
    ready.clock_lamport = message.clock_lamport;
    //Decimos que nuestro pc esta listo para apagarse
    ready.action = READY_TO_SHUTDOWN;

    printf("%s, %d, SEND, READY_TO_SHUTDOWN\n", ready.origin, ready.clock_lamport);

        // Enviamos la struct por el socket
    if (send(sockfd, &ready, sizeof(ready), 0) < 0)
    {
        printf("Send to the server failed...\n");
    }


}
// Notifica que va a realizar el shutdown correctamente (SHUTDOWN_ACK)
void notify_shutdown_ack(){
    struct message ready;

    // Copiamos el nombre de nuestra maquina para asi poder mandarlo
    strcpy(ready.origin, message.origin); 

    // Cogemos los relojes de lamport oportunos para cada momento

    ready.clock_lamport = message.clock_lamport;
    //Decimos que nuestro pc esta listo para apagarse
    ready.action = READY_TO_SHUTDOWN;

    printf("%s, %d, SEND, READY_TO_SHUTDOWN\n", ready.origin, ready.clock_lamport);

        // Enviamos la struct por el socket
    if (send(sockfd, &ready, sizeof(ready), 0) < 0)
    {
        printf("Send to the server failed...\n");
    }
}

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
        if(receive.action == 0) // Comprobamos que el mensaje recibido sea el esperado
            {
            receive.clock_lamport = reloj_Lamport(message); // Actualizamos el reloj de Lamport
            printf("%s, %d, RECV (%s), READY_TO_SHUTDOWN\n", message.origin, receive.clock_lamport, receive.origin);
        } else {
            printf("This is not the correct action\n");
            }   
    }
    return 0;
}

int server_send_shotdown() {

    
}



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
    // Ponemos el servidor a escuchar
    if((listen(sockfd, 2)) != 0){
        printf("Listen failed...\n");
        exit(1);
    } else {
        printf("Server listening...\n");
    }
    return 0;
}
