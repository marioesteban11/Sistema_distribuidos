#include "proxy.h"


//Variables para todas las funciones
int sockfd = 0, connfd = 0;
int connfd_writers = 0, connfd_readers = 0, tiempo_espera = 0;
int limite_topics = 0;
struct sockaddr_in servaddr;
struct sockaddr_in serv_addr;

////structuras del broker hacia el publicador y suscribtor
struct response broker_to_publisher;
struct response broker_to_suscriber;
struct message publisher_to_broker;
struct message suscriber_to_broker;

//// Estructuras del publicador y suscriptor al broker
struct response response_from_broker;
struct message message_to_broker;

/////////////////////////////////////////

struct timespec begin;


// Funciones broker

int server_conection(int port) {
    srand (time(NULL));
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

int aceptar_cliente() {

    clock_gettime(CLOCK_MONOTONIC, &begin);
    int nanosecons = 0;
    struct num_threads{
        int threads;
        int opcion;
    }num_clientes;

    connfd = accept(sockfd, (struct sockaddr*)NULL, NULL); //Acepta un nuevo cliente
    if (connfd < 0) {
        printf("Server accept failed...\n");
        exit(1);
    } else {
        printf("Server accepts the client...\n");
    }
    
    if ((recv(connfd, &publisher_to_broker, sizeof(publisher_to_broker), 0)) < 0) {
        printf("Recv from the client failed...\n");
    }

    if (publisher_to_broker.action == REGISTER_PUBLISHER) {
       limite_topics += 1;
    }
    return publisher_to_broker.action;

}

int conexiones_publicadores(){ 
    int nanosecons = publisher_to_broker.data.time_generated_data.tv_nsec - begin.tv_nsec;

    printf("[%d] Nuevo cliente ($%d) Publicador conectado : %s\n",nanosecons, sockfd,  publisher_to_broker.topic);
    broker_to_publisher.id = sockfd;

    if (sizeof(publisher_to_broker.data.data) == 0 )
    {
        printf("no se ha recibido ningun mensaje");
    }

    if (limite_topics >= 10){
        broker_to_publisher.response_status = LIMIT;
    }else {
        broker_to_publisher.response_status = OK;
    }
    // Comprueba si el mensaje mandado por el publicador está vacio
    if (message_to_broker.data.data[0] =='\0'){
        printf("No hay ningun mensaje\n");
    }else{
        printf("%s\n", message_to_broker.data.data);
    }

    if (send(connfd, &broker_to_publisher, sizeof(broker_to_publisher), 0) < 0) {
        printf("Send to the server failed...\n");
        exit(1);
    } 
    return 0;
}

int conexiones_suscriptores() {

    int nanosecons = publisher_to_broker.data.time_generated_data.tv_nsec - begin.tv_nsec;

    printf("[%d] Nuevo cliente ($%d) Suscriptor conectado : %s\n",nanosecons, sockfd,  publisher_to_broker.topic);
    broker_to_publisher.id = sockfd;

    //if (message_to_broker.data.data[0] =='\0'){
    //    printf("No hay ningun mensaje\n");
    //}else{
    //    printf("%s\n", message_to_broker.data.data);
    //}

    if (send(connfd, &broker_to_publisher, sizeof(broker_to_publisher), 0) < 0) {
        printf("Send to the server failed...\n");
        exit(1);
    } 
}

int close_server() {
    if(close(sockfd) == 1) {
        printf("Close failed\n");
        exit(1);
    }
  return 0;
}



////////////////////////////////
// Funciones publicador 

int client_conection(char* ip, int port) {
    // Creamos un socket TCP y comprobamos que se ha creado correctamente
    sockfd = socket(AF_INET, SOCK_STREAM, 0);
    if (sockfd == -1) {
        printf("Socket creation failed...\n" );
        exit(1);
    }else {
        printf("Socket successfully created...\n" );
    }
    printf("ip: %s\n\n", ip);
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

int topic_conection(char *topic)
{
    struct timespec end;

    clock_gettime(CLOCK_MONOTONIC, &end);
    
    // Guardamos el topic donde queremos escribir
    strcpy(message_to_broker.topic, topic);
    //Decimos que queremos registrar un suscriptor
    message_to_broker.action = REGISTER_PUBLISHER;
    //Tiempo empleado para que se conecte el publisher
    message_to_broker.data.time_generated_data = end;
    //printf("Mandamos el topic %s\n", mandar.topic);
    
    //Mandamos en que topic vamos a escribir y se lo mandamos al broker
    if (send(sockfd, &message_to_broker, sizeof(message_to_broker), 0) < 0) {
        printf("Send to the server failed...\n");
    }    

    // Recibimos el response del broker
    if ((recv(sockfd, &response_from_broker, sizeof(response_from_broker), 0)) < 0) {
        printf("Recv from the client failed...\n");
    }

    return 0;
}

// Funciones suscriptor

int topic_suscription(char *topic) {

    struct timespec end;

    clock_gettime(CLOCK_MONOTONIC, &end);
    
    // Guardamos el topic donde queremos escribir
    strcpy(message_to_broker.topic, topic);
    //Decimos que queremos registrar un suscriptor
    message_to_broker.action = REGISTER_SUBSCRIBER;
    //Tiempo empleado para que se conecte el publisher
    message_to_broker.data.time_generated_data = end;
    //printf("Mandamos el topic %s\n", mandar.topic);
    
    //Mandamos en que topic queremos recibir y se lo mandamos al broker
    if (send(sockfd, &message_to_broker, sizeof(message_to_broker), 0) < 0) {
        printf("Send to the server failed...\n");
    }    

    // Recibimos la respuesta del broker
    if ((recv(sockfd, &response_from_broker, sizeof(response_from_broker), 0)) < 0) {
        printf("Recv from the client failed...\n");
    }

    printf("%d\n", response_from_broker.response_status);
    
}

int close_client() {
    if(close(sockfd) == 1) {
        printf("Close failed\n");
        exit(1);
    }
    return 0;
}