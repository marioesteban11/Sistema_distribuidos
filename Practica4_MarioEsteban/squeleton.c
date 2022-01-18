#include "squeleton.h" 

enum tipo_cliente {
    PUBLICADOR = 0,
    SUSCRIPTOR
};

struct cliente {
    enum tipo_cliente cliente;
};


//Variables para todas las funciones
int sockfd = 0, connfd = 0;
int connfd_publisher = 0, connfd_suscriber = 0, tiempo_espera = 0;
int limite_topics = 0;
int MAX_NUMBER_TOPICS = 10, MAX_NUMBER_SUSCRIBERS = 1000, MAX_NUMBER_PUBLISHERS = 100;

char topic_list[10][100]; 
struct sockaddr_in servaddr;
struct sockaddr_in serv_addr;

////structuras del broker hacia el publicador y suscribtor
struct response broker_to_publisher;
struct response broker_to_suscriber;
struct message publisher_to_broker;
struct message suscriber_to_broker;
struct message someone_to_broker;

//// Estructuras del publicador y suscriptor al broker
struct response response_from_broker;
struct message message_to_broker;


/////////////////////////////////////////////////////////////////////

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


void *thread_clientes(void *arg) {

    while(1){
        printf("ESPERAMOS AQUI???\n\n");
        if ((recv(connfd, &someone_to_broker, sizeof(someone_to_broker), 0)) < 0) {
        printf("Recv from the client failed...\n");
        }
        printf("QUE RECIBO AQUI?%s\n", someone_to_broker.data.data);
        if (someone_to_broker.action == REGISTER_PUBLISHER){
            conexiones_publicadores();
        }else if (someone_to_broker.action == REGISTER_SUBSCRIBER ){
            conexiones_suscriptores();
        }else if (someone_to_broker.action == UNREGISTER_PUBLISHER){
            desconexion_publicador();
        }else if (someone_to_broker.action == UNREGISTER_SUBSCRIBER){
            desconexion_suscriptor();
        }else if (someone_to_broker.action == PUBLISH_DATA){
            publicar_datos();
        }
    }
    


}

int aceptar_cliente() {

    pthread_t clientes;
    struct cliente recibir;
    clock_gettime(CLOCK_MONOTONIC, &begin);
    int nanosecons = 0;

    connfd = accept(sockfd, (struct sockaddr*)NULL, NULL); //Acepta un nuevo cliente
    printf("RECIBIMOS ALGO?\n");
    if (connfd < 0) {
        printf("Server accept failed...\n");
        exit(1);
    } else {
        printf("Server accepts the client...\n");
    }

    if ((recv(connfd, &recibir, sizeof(recibir), 0)) < 0) {
        printf("Recv from the client failed...\n");
    }

    printf("Recibimos el tipo de mensaje que nos mandas los clientes (SI publicador o suscriptor) %d\n", recibir.cliente);

    if (recibir.cliente == 0) {
        connfd_publisher = connfd;
    }else if (recibir.cliente == 1){
        printf("suscriner");
        connfd_suscriber = connfd;
    }

    if(pthread_create(&clientes, NULL, thread_clientes, NULL ) != 0) {
        printf("Fallo al ejecutar pthread_create de lectores \n");
        exit(1);
    }

    return someone_to_broker.action;
}

int conexiones_publicadores(){ 

    strcpy(publisher_to_broker.topic, someone_to_broker.topic);
    someone_to_broker.action = publisher_to_broker.action;
    someone_to_broker.id = publisher_to_broker.id;
    strcpy(publisher_to_broker.data.data, someone_to_broker.data.data);

    int nanosecons = publisher_to_broker.data.time_generated_data.tv_nsec - begin.tv_nsec;

    if (publisher_to_broker.action == REGISTER_PUBLISHER) {
       limite_topics += 1;
    }


    strcpy(topic_list[limite_topics - 1], publisher_to_broker.topic);

    printf("[%d] Nuevo cliente ($%d) Publicador conectado : %s\n",nanosecons, sockfd,  publisher_to_broker.topic);
    broker_to_publisher.id = sockfd;

    if (limite_topics >= MAX_NUMBER_TOPICS){
        broker_to_publisher.response_status = LIMIT;
    }else {
        broker_to_publisher.response_status = OK;
    }

    if (send(connfd_publisher, &broker_to_publisher, sizeof(broker_to_publisher), 0) < 0) {
        printf("Send to the server failed...\n");
        exit(1);
    } 
    return 0;
}

int conexiones_suscriptores() {

    int nanosecons = publisher_to_broker.data.time_generated_data.tv_nsec - begin.tv_nsec;

    printf("[%d] Nuevo cliente ($%d) Suscriptor conectado : %s\n",nanosecons, sockfd,  publisher_to_broker.topic);
    broker_to_publisher.id = sockfd;

    if (send(connfd_suscriber, &broker_to_publisher, sizeof(broker_to_publisher), 0) < 0) {
        printf("Send to the server failed...\n");
        exit(1);
    } 


}



int desconexion_publicador() {
    int nanosecons = publisher_to_broker.data.time_generated_data.tv_nsec - begin.tv_nsec;

    printf("[%d] Eliminando cliente ($%d) Publicador : %s\n",nanosecons, sockfd,  publisher_to_broker.topic);
    broker_to_publisher.id = sockfd;

    if (send(connfd_publisher, &broker_to_publisher, sizeof(broker_to_publisher), 0) < 0) {
        printf("Send to the server failed...\n");
        exit(1);
    } 

    // Reducimos el numero de la lista de topics
    if (limite_topics > 0) {
        limite_topics -= 1;
    }

}
int desconexion_suscriptor(){

     int nanosecons = publisher_to_broker.data.time_generated_data.tv_nsec - begin.tv_nsec;

    printf("[%d] Eliminando cliente ($%d) Suscriptor :%s\n",nanosecons, sockfd,  publisher_to_broker.topic);
    broker_to_publisher.id = sockfd;

    if (send(connfd_suscriber, &broker_to_publisher, sizeof(broker_to_publisher), 0) < 0) {
        printf("Send to the server failed...\n");
        exit(1);
    } 
}

int publicar_datos() {

    char mensaje[100];
    //strcpy(mensaje, publisher_to_broker.data.data);
    printf("Esperando el mensaje del publicador\n");
    strcpy(publisher_to_broker.topic, someone_to_broker.topic);
    someone_to_broker.action = publisher_to_broker.action;
    someone_to_broker.id = publisher_to_broker.id;
    strcpy(publisher_to_broker.data.data, someone_to_broker.data.data);
    
    strcpy(mensaje, someone_to_broker.data.data);
    printf("[SECONDS.NANOSECONDS] Recibido mensaje para publicar en topic: %s mensaje: %s - Generó: $time_generated_data\n", publisher_to_broker.topic, mensaje);
    
    broker_to_publisher.response_status = OK;

    if (send(connfd_publisher, &broker_to_publisher, sizeof(broker_to_publisher), 0) < 0) {
        printf("Send to the server failed...\n");
        exit(1);
    } 

    // Mandamos el mensaje al cliente si es necesario
    if (connfd_suscriber > 0){
        if (send(connfd_suscriber, &publisher_to_broker, sizeof(publisher_to_broker), 0) < 0) {
            printf("Send to the server failed...\n");
            exit(1);
        } 
    }


    
    
    return 0;
}

int close_server() {
    if(close(sockfd) == 1) {
        printf("Close failed\n");
        exit(1);
    }
  return 0;
}
