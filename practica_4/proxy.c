#include "proxy.h"
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
char data_topics[10][100];
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

/////////////////////////////////////////

struct timespec publicador;
struct timespec suscriptor;
struct timespec broker;
struct timespec clientes_time;

// Semaforo

sem_t sem_time_broker;
sem_t sem_clientes_time;
sem_t sem_recv_thread;
// Funciones broker

void semaforo() {

    sem_init(&sem_time_broker, 0, 1);
    sem_init(&sem_clientes_time, 0, 1);
    sem_init(&sem_recv_thread, 0, 1);
}

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
        //printf("ESPERAMOS AQUI???\n\n");
        sem_post(&sem_recv_thread);
        if ((recv(connfd, &someone_to_broker, sizeof(someone_to_broker), 0)) < 0) {
            printf("Recv from the client failed...\n");
        }
        //sem_post(&sem_recv_thread);
        //printf("QUE RECIBO AQUI?%s y topic %s \n", someone_to_broker.data.data, someone_to_broker.topic);
        if (someone_to_broker.action == REGISTER_PUBLISHER){
            printf("\n\n");
            sem_wait(&sem_recv_thread);
            conexiones_publicadores();
        }else if (someone_to_broker.action == REGISTER_SUBSCRIBER ){
            sem_wait(&sem_recv_thread);
            conexiones_suscriptores();
        }else if (someone_to_broker.action == UNREGISTER_PUBLISHER){
            sem_wait(&sem_recv_thread);
            desconexion_publicador();
            break;
        }else if (someone_to_broker.action == UNREGISTER_SUBSCRIBER){
            printf("Entras en eliminar conexion con el suscriptor\n\n");
            sem_wait(&sem_recv_thread);
            desconexion_suscriptor();
            break;
        }else if (someone_to_broker.action == PUBLISH_DATA){
            sem_wait(&sem_recv_thread);
            publicar_datos();
        }

        //someone_to_broker.action = 5;  
    }
    


}

int aceptar_cliente() {

    pthread_t clientes[500];
    struct cliente recibir;
    //sem_wait(&sem_time_broker);
    clock_gettime(CLOCK_MONOTONIC, &broker);
    int nanosecons = 0;
    while(1){
        connfd = accept(sockfd, (struct sockaddr*)NULL, NULL); //Acepta un nuevo cliente
        printf("Aceptamos cliente\n");
        if (connfd < 0) {
            printf("Server accept failed...\n");
            exit(1);
        } else {
            printf("Server accepts the client...\n");
        }
        printf("RECIBIMOS EL MENSAJE DE STRUCT MENSAJE   %d\n\n", connfd);
        if ((recv(connfd, &recibir, sizeof(recibir), 0)) < 0) {
            printf("Recv from the client failed...\n");
        }
        
        printf("Recibimos el tipo de mensaje que nos mandas los clientes (SI publicador o suscriptor) %d   %d\n", recibir.cliente, connfd);

        if (recibir.cliente == 0) {
            connfd_publisher = connfd;
            //for (int i = 0; i < 500;i++ ){
            //    if(pthread_create(&clientes[i], NULL, thread_clientes, NULL ) != 0) {
            //        printf("Fallo al ejecutar pthread_create de lectores \n");
            //        exit(1);
            //    }
            //} 
        }else if (recibir.cliente == 1){
            connfd_suscriber = connfd;
            //for (int i = 0; i < 500;i++ ){
            //    if(pthread_create(&clientes[i], NULL, thread_clientes, NULL ) != 0) {
            //        printf("Fallo al ejecutar pthread_create de lectores \n");
            //        exit(1);
            //    }
            //} 
        }
        //cerramos los threads abiertos
        //for(int i = 0; i < 1; i++) {
        //    if(pthread_join(clientes[i], NULL) != 0) {
        //        printf("Fallo al ejecutar pthread_join...\n");
        //        exit(1);
        //    }
        //}
        if(pthread_create(&clientes[0], NULL, thread_clientes, NULL ) != 0) {
                    printf("Fallo al ejecutar pthread_create de clientes \n");
                    exit(1);
        }
    }
    return someone_to_broker.action;
}

int conexiones_publicadores(){ 

    int aumento = 0;
    strcpy(publisher_to_broker.topic, someone_to_broker.topic);
    someone_to_broker.action = publisher_to_broker.action;
    someone_to_broker.id = publisher_to_broker.id;
    strcpy(publisher_to_broker.data.data, someone_to_broker.data.data);

    int nanosecons = publisher_to_broker.data.time_generated_data.tv_nsec - broker.tv_nsec;

    if (topic_list[0][0] ==  '\0') {
        strcpy(topic_list[limite_topics], publisher_to_broker.topic);
        if (publisher_to_broker.action == REGISTER_PUBLISHER) {
            limite_topics += 1;
        } 
        aumento = 1;
    }else {
        for (int i = 0; i < limite_topics; i++){
            if (strcmp(publisher_to_broker.topic, topic_list[i]) != 0) {
                aumento = 1;
            }
        }
    }
    if (aumento == 1) {
        clock_gettime(CLOCK_MONOTONIC, &broker);
        printf("[%ld] Nuevo cliente ($%d) Publicador conectado : %s\n", broker.tv_nsec, sockfd,  publisher_to_broker.topic);
        //sem_post(&sem_time_broker);
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
    }
    return 0;
}

int conexiones_suscriptores() {
    clock_gettime(CLOCK_MONOTONIC, &broker);
    strcpy(suscriber_to_broker.topic, someone_to_broker.topic);
    someone_to_broker.action = suscriber_to_broker.action;
    someone_to_broker.id = suscriber_to_broker.id;
    strcpy(suscriber_to_broker.data.data, someone_to_broker.data.data);

    int nanosecons = suscriber_to_broker.data.time_generated_data.tv_nsec - broker.tv_nsec;

    printf("[%ld] Nuevo cliente ($%d) Suscriptor conectado : %s\n",broker.tv_nsec, sockfd,  suscriber_to_broker.topic);
    broker_to_publisher.id = sockfd;
    printf("MENSAJITO DEL BROKER PAPA, %d\n", connfd_suscriber);
    if (send(connfd_suscriber, &broker_to_publisher, sizeof(broker_to_publisher), 0) < 0) {
        printf("Send to the server failed...\n");
        exit(1);
    } 
        
    return 0;
}

int desconexion_publicador() {
    int nanosecons = publisher_to_broker.data.time_generated_data.tv_nsec - broker.tv_nsec;
    clock_gettime(CLOCK_MONOTONIC, &broker);
    printf("[%ld] Eliminando cliente ($%d) Publicador : %s\n",broker.tv_nsec, sockfd,  publisher_to_broker.topic);
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

     int nanosecons = publisher_to_broker.data.time_generated_data.tv_nsec - broker.tv_nsec;

    printf("[%ld] Eliminando cliente ($%d) Suscriptor :%s\n",broker.tv_nsec, sockfd,  publisher_to_broker.topic);
    broker_to_publisher.id = sockfd;

    if (send(connfd_suscriber, &broker_to_publisher, sizeof(broker_to_publisher), 0) < 0) {
        printf("Send to the server failed...\n");
        exit(1);
    } 
}

int publicar_datos() {

    struct message mensaje_enviado;
    char mensaje[100];
    clock_gettime(CLOCK_MONOTONIC, &broker);
    //strcpy(mensaje, publisher_to_broker.data.data);
    //printf("Esperando el mensaje del publicador\n");
    strcpy(publisher_to_broker.topic, someone_to_broker.topic);
    someone_to_broker.action = publisher_to_broker.action;
    someone_to_broker.id = publisher_to_broker.id;
    strcpy(publisher_to_broker.data.data, someone_to_broker.data.data);
    char *mandar_mensaje;
    
    //strcmp(data_topics[limite_topics], publisher_to_broker.data.data);

    printf("%s,  %s, ananaaaaaaaaaaaas\n\n\n", mandar_mensaje, publisher_to_broker.data.data);
    strcpy(mensaje, someone_to_broker.data.data);

    printf("[%ld] Recibido mensaje para publicar en topic: %s mensaje: %s - Generó: $time_generated_data\n",broker.tv_nsec, publisher_to_broker.topic, data_topics[limite_topics]);
    
    broker_to_publisher.response_status = OK;

    if (send(connfd_publisher, &broker_to_publisher, sizeof(broker_to_publisher), 0) < 0) {
        printf("Send to the server failed...\n");
        exit(1);
    } 
    // Mandamos el mensaje al cliente si es necesario
    if (connfd_suscriber > 0){
        //strcpy(publisher_to_broker.topic, someone_to_broker.topic);
        //printf("%s,  %s,\n\n", publisher_to_broker.data.data, publisher_to_broker.topic );

        for (int i = 0; i < MAX_NUMBER_TOPICS; i++){
            //printf("yfadsihfiuasdf    %s,\n", topic_list[i]);
            if (strcmp(topic_list[i], publisher_to_broker.topic) == 0 ){
                strcpy(data_topics[i], publisher_to_broker.data.data);
            }
            printf("%s, %s, patatilals fritas con gonnorea, %s\n\n\n", topic_list[i], publisher_to_broker.topic, data_topics[i]);
            if (strcmp(topic_list[i], publisher_to_broker.topic ) == 0){
                //printf("MANDAMOS MENSAJEEEEE, %s, %s,  %s\n", publisher_to_broker.topic, topic_list[i], data_topics[i]);

                strcpy(mensaje_enviado.topic, topic_list[i]);
                strcpy(mensaje_enviado.data.data, data_topics[i]);
                if (send(connfd_suscriber, &mensaje_enviado, sizeof(mensaje_enviado), 0) < 0) {
                    printf("Send to the server failed...\n");
                    exit(1);
                } 
                //sleep(10);
            }
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



////////////////////////////////
// Funciones publicador 

int client_conection(char* ip, int port, int tipo) {

    clock_gettime(CLOCK_MONOTONIC, &clientes_time);
    struct cliente mandar;

    if (tipo == 0){
        printf("[%ld] Publisher conectado con el broker correctamente.\n", clientes_time.tv_nsec);
        mandar.cliente = PUBLICADOR;
    }else if(tipo == 1){
        printf("[%ld] Suscriber conectado con el broker correctamente.\n", clientes_time.tv_nsec);
        mandar.cliente = SUSCRIPTOR;
    }
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
    printf("Mandando tipo de cosa que es lo mandado?    %d\n\n", sockfd);
    if (send(sockfd, &mandar, sizeof(mandar), 0) < 0) {
        printf("Send to the server failed...\n");
    } 
    printf("MANDADO!!!!!!!!!!!\n");
    return 0;
}

int topic_conection(char *topic) {
    //struct timespec end;
    clock_gettime(CLOCK_MONOTONIC, &clientes_time);
    //clock_gettime(CLOCK_MONOTONIC, &publicador);
     
    // Guardamos el topic donde queremos escribir
    strcpy(message_to_broker.topic, topic);
    //Decimos que queremos registrar un suscriptor
    message_to_broker.action = REGISTER_PUBLISHER;
    //Tiempo empleado para que se conecte el publisher
    message_to_broker.data.time_generated_data = publicador;
    //printf("Mandamos el topic %s\n", mandar.topic);
    printf("me da tiempo a mandar el mensaje???\n\n");
    //Mandamos en que topic vamos a escribir y se lo mandamos al broker
    printf("TOPIC SUSCRIPTION\n");   
    if (send(sockfd, &message_to_broker, sizeof(message_to_broker), 0) < 0) {
        printf("Send to the server failed...\n");
    }    

    // Recibimos el response del broker
    if ((recv(sockfd, &response_from_broker, sizeof(response_from_broker), 0)) < 0) {
        printf("Recv from the client failed...\n");
    }
    printf("Pero no me da tiempo a RECIBIR el mensaje???\n\n");

    if (response_from_broker.response_status == OK){
        printf("[%ld] Registrado correctamente con ID: $%d para topic $%s\n", clientes_time.tv_nsec, response_from_broker.id, message_to_broker.topic );
    }else{
        printf("[%ld] Error al hacer el registro: error=$%d\n", clientes_time.tv_nsec, response_from_broker.response_status);
    }

    return 0;
}

int remove_topic(char *topic) {
    // Guardamos el topic donde queremos escribir
    strcpy(message_to_broker.topic, topic);
    //Decimos que queremos registrar un suscriptor
    message_to_broker.action = UNREGISTER_PUBLISHER;
    message_to_broker.id = sockfd;
    clock_gettime(CLOCK_MONOTONIC, &clientes_time);
    //Mandamos en que topic vamos a desuscribirnos y se lo mandamos al broker
    if (send(sockfd, &message_to_broker, sizeof(message_to_broker), 0) < 0) {
        printf("Send to the server failed...\n");
    } 

    // Recibimos el response del broker
    if ((recv(sockfd, &response_from_broker, sizeof(response_from_broker), 0)) < 0) {
        printf("Recv from the client failed...\n");
    }

    printf("[%ld] De-Registrado ($%d) correctamente del broker.\n", clientes_time.tv_nsec, response_from_broker.id);

    return 0;
}

int send_message(char *topic) {
    struct timespec end;

    //clock_gettime(CLOCK_MONOTONIC, &end);
    
    // Guardamos el topic donde queremos escribir
    strcpy(message_to_broker.topic, topic);
    //Decimos que queremos registrar un suscriptor
    message_to_broker.action = PUBLISH_DATA;
    //Tiempo empleado para que se conecte el publisher
    message_to_broker.data.time_generated_data = end;
    char mensaje[100] = "       FUENLABRADA";
    strcpy(message_to_broker.data.data, mensaje);
    //printf("Mandamos el topic %s\n", mandar.topic);
    //printf("HOLA VAMOS A MANDAR UN MENSAJE,  %s\n", message_to_broker.data.data);
    //Mandamos en que topic vamos a escribir y se lo mandamos al broker
    if (send(sockfd, &message_to_broker, sizeof(message_to_broker), 0) < 0) {
        printf("Send to the server failed...\n");
    }    

    //printf("Mensaje mandado\n");
    // Recibimos el response del broker
    if ((recv(sockfd, &response_from_broker, sizeof(response_from_broker), 0)) < 0) {
        printf("Recv from the client failed...\n");
    }
    //printf("Mensaje del broker recibido\n");

}
// Funciones suscriptor

int topic_suscription(char *topic) {
       
    struct timespec end;
    struct message publicacion;
    clock_gettime(CLOCK_MONOTONIC, &clientes_time);
    
    // Guardamos el topic donde queremos escribir
    strcpy(message_to_broker.topic, topic);
    //Decimos que queremos registrar un suscriptor
    message_to_broker.action = REGISTER_SUBSCRIBER;
    //Tiempo empleado para que se conecte el publisher
    message_to_broker.data.time_generated_data = end;
    //printf("Mandamos el topic %s\n", mandar.topic);
    printf("TOPIC SUSCRIPTION %d\n", sockfd); 
    //Mandamos en que topic queremos recibir y se lo mandamos al broker
    if (send(sockfd, &message_to_broker, sizeof(message_to_broker), 0) < 0) {
        printf("Send to the server failed...\n");
    }    
    
    // Recibimos la respuesta del broker
    if ((recv(sockfd, &response_from_broker, sizeof(response_from_broker), 0)) < 0) {
        printf("Recv from the client failed...\n");
    }
    printf("AGAGAGAGGAGAGGAGAG\n\n\n\n");
    if (response_from_broker.response_status == OK){
        printf("[%ld] Registrado correctamente con ID: $%d para topic $%s\n", clientes_time.tv_nsec, response_from_broker.id, message_to_broker.topic);

    }else {
        printf("[%ld] Error al hacer el registro: $%d\n", clientes_time.tv_nsec, response_from_broker.response_status);

    }
    // Recibimos la posible publicacion del suscriptor
    if ((recv(sockfd, &publicacion, sizeof(publicacion), 0)) < 0) {
        printf("Recv from the client failed...\n");
    }
    printf("%s , %s \n", publicacion.topic, topic);
    //if(strcmp(publicacion.topic, topic) == 0){
        
        if (publicacion.data.data[0] != '\0'){
            printf("[%ld] Recibido mensaje topic: $%s - mensaje: $%s - Generó: $time_generated_data - Recibido: $time_received_data - Latencia: $latency.\n", clientes_time.tv_nsec, publicacion.topic, publicacion.data.data);
        }
    //}

}


int unfollow_topic(char *topic) {
    clock_gettime(CLOCK_MONOTONIC, &clientes_time);
    // Guardamos el topic donde queremos escribir
    strcpy(message_to_broker.topic, topic);
    //Decimos que queremos registrar un suscriptor
    message_to_broker.action = UNREGISTER_SUBSCRIBER;
    message_to_broker.id = sockfd;

    //Mandamos en que topic vamos a desuscribirnos y se lo mandamos al broker
    if (send(sockfd, &message_to_broker, sizeof(message_to_broker), 0) < 0) {
        printf("Send to the server failed...\n");
    } 
    printf("HOLAAAAAAAAAA\n");
    // Recibimos el response del broker
    if ((recv(sockfd, &response_from_broker, sizeof(response_from_broker), 0)) < 0) {
        printf("Recv from the client failed...\n");
    }
    printf("adioooooooooooos\n");
    printf("[%ld] De-Registrado ($%d) correctamente del broker.\n", clientes_time.tv_nsec, response_from_broker.id);


    return 0;
}
int close_client() {
    if(close(sockfd) == 1) {
        printf("Close failed\n");
        exit(1);
    }
    return 0;
}