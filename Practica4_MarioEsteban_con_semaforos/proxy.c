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
int limite_topics = 0, numero_suscriptores = 0, eliminar = 0;
int MAX_NUMBER_TOPICS = 10, MAX_NUMBER_SUSCRIBERS = 1000, MAX_NUMBER_PUBLISHERS = 100;

char topic_list[10][100]; 
char data_topics[10][100];
int subscriber_list[1000];
char followed_topics[1000][100];
char modelo_algoritmo[100];
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
sem_t sem_numero_suscriptores;
sem_t sem_enviados;
sem_t sem_desuscripcion;
sem_t sem_bloq_sus;
sem_t sem_pub_bloq;
sem_t sem_set_connfd;
// Funciones broker

void semaforo() {

    sem_init(&sem_time_broker, 0, 1);
    sem_init(&sem_clientes_time, 0, 1);
    sem_init(&sem_recv_thread, 0, 1);
    sem_init(&sem_numero_suscriptores, 0, 1);
    sem_init(&sem_enviados, 0, 1);
    sem_init(&sem_desuscripcion, 0, 1);
    sem_init(&sem_bloq_sus, 0, 1);
    sem_init(&sem_pub_bloq, 0, 1);
    sem_init(&sem_set_connfd, 0, 1);
}

int server_conection(int port) {
    srand (time(NULL));
    // Creamos un socket TCP y comprobamos que se ha creado correctamente
    sockfd = socket(AF_INET, SOCK_STREAM, 0);
    if (sockfd == -1) {
        printf("Socket creation failed...\n");
        exit(1);
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
    } 
    if ((listen(sockfd, 500)) != 0) {
        printf("Listen failed...\n");
        exit(1);
    }
}
void *thread_clientes(void *arg) {
    while(1){
        
        sem_post(&sem_recv_thread);
        sem_post(&sem_desuscripcion);
        sem_post(&sem_enviados);
        if ((recv(connfd, &someone_to_broker, sizeof(someone_to_broker), 0)) < 0) {
            printf("Recv from the client failed...\n");
        }
        
        if (someone_to_broker.action == REGISTER_PUBLISHER){
            conexiones_publicadores();
        }else if (someone_to_broker.action == REGISTER_SUBSCRIBER ){
            conexiones_suscriptores();
        }else if (someone_to_broker.action == UNREGISTER_PUBLISHER){
            sem_wait(&sem_pub_bloq);
            desconexion_publicador();
            break;
        }else if (someone_to_broker.action == UNREGISTER_SUBSCRIBER){
            desconexion_suscriptor();
            sem_post(&sem_bloq_sus);
            
            break;
        }else if (someone_to_broker.action == PUBLISH_DATA){
            publicar_datos();
        } 
        sem_post(&sem_recv_thread);
    }
}

int aceptar_cliente(char *mode) {
    pthread_t clientes[500];
    struct cliente recibir;
    clock_gettime(CLOCK_MONOTONIC, &broker);
    int nanosecons = 0;
    int threads_aum = 0;
    while(1){

        connfd = accept(sockfd, (struct sockaddr*)NULL, NULL); //Acepta un nuevo cliente
        if (connfd < 0) {
            printf("Server accept failed...\n");
            exit(1);
        }
        // AQUI LLEGA EL PRIMER mensaje que es decir si lo que recibe lo manda un publicador o un suscriptor
        if ((recv(connfd, &recibir, sizeof(recibir), 0)) < 0) {
            printf("Recv from the client failed...\n");
        }
        sem_wait(&sem_set_connfd);
        if (recibir.cliente == 0) {
            connfd_publisher = connfd;
        }else if (recibir.cliente == 1){
            connfd_suscriber = connfd;
        }
        if(strcmp(mode, "paralelo") == 0) {
            strcpy(modelo_algoritmo, mode);
            
        }else if (strcmp(mode, "secuencial") == 0) {
            strcpy(modelo_algoritmo, mode);
        }else if (strcmp(mode, "justo") == 0) {
            strcpy(modelo_algoritmo, mode);
        }
        if(pthread_create(&clientes[threads_aum++], NULL, thread_clientes, NULL ) != 0) {
            printf("Fallo al ejecutar pthread_create de clientes \n");
            exit(1);
        }
        sem_post(&sem_set_connfd);

    }
    return someone_to_broker.action;
}

int conexiones_publicadores(){ 

    int aumento = 0;
    strcpy(publisher_to_broker.topic, someone_to_broker.topic);
    someone_to_broker.action = publisher_to_broker.action;
    someone_to_broker.id = publisher_to_broker.id;
    strcpy(publisher_to_broker.data.data, someone_to_broker.data.data);

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
        printf("[%ld] Nuevo cliente ($%d) Publicador conectado : %s\n ", broker.tv_nsec, connfd_publisher,  publisher_to_broker.topic);
        //sem_post(&sem_time_broker);
        broker_to_publisher.id = connfd_publisher;

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
    //sem_wait(&sem_numero_suscriptores);
    clock_gettime(CLOCK_MONOTONIC, &broker);
    strcpy(suscriber_to_broker.topic, someone_to_broker.topic);
    someone_to_broker.action = suscriber_to_broker.action;
    someone_to_broker.id = suscriber_to_broker.id;
    strcpy(suscriber_to_broker.data.data, someone_to_broker.data.data);
    sem_wait(&sem_numero_suscriptores);
    numero_suscriptores ++;
    //sem_post(&sem_numero_suscriptores);
    int nanosecons = suscriber_to_broker.data.time_generated_data.tv_nsec - broker.tv_nsec;
    for (int i = numero_suscriptores - 1; i < numero_suscriptores; i++){
        
        subscriber_list[i] = connfd_suscriber;
        strcpy(followed_topics[i], suscriber_to_broker.topic);
        //printf("JAMAUS ARMYYYY %d %d   %s  a\n\n", numero_suscriptores, subscriber_list[i], followed_topics[i]);
    }
    int mandar_connfd = subscriber_list[numero_suscriptores - 1];
    printf("[%ld] Nuevo cliente ($%d) Suscriptor conectado : %s\n", broker.tv_nsec, mandar_connfd ,  suscriber_to_broker.topic);
    for (int i = 0; i < limite_topics; i++){
        printf("$%s: 1 Publicador - %d Suscriptores\n", topic_list[i], numero_suscriptores);
    }
    broker_to_publisher.id = subscriber_list[numero_suscriptores - 1];
    
    //printf("MENSAJITO DEL BROKER PAPA, %d\n", connfd_suscriber);
    if (send(mandar_connfd, &broker_to_publisher, sizeof(broker_to_publisher), 0) < 0) {
        printf("Send to the server failed...\n");
        exit(1);
    } 
    sem_post(&sem_numero_suscriptores);    
    return 0;
}

int desconexion_publicador() {
    int nanosecons = publisher_to_broker.data.time_generated_data.tv_nsec - broker.tv_nsec;
    clock_gettime(CLOCK_MONOTONIC, &broker);
    printf("[%ld] Eliminando cliente ($%d) Publicador : %s\n",broker.tv_nsec, connfd_publisher,  publisher_to_broker.topic);
    broker_to_publisher.id = connfd_publisher;

    if (send(connfd_publisher, &broker_to_publisher, sizeof(broker_to_publisher), 0) < 0) {
        printf("Send to the server failed...\n");
        exit(1);
    } 
    sem_wait(&sem_pub_bloq);
    // Reducimos el numero de la lista de topics
    //if (limite_topics > 0) {
    //    limite_topics -= 1;
    //}

}
int desconexion_suscriptor(){
    sem_wait(&sem_numero_suscriptores);
    int nanosecons = publisher_to_broker.data.time_generated_data.tv_nsec - broker.tv_nsec;
    int  eliminar_connfd = subscriber_list[eliminar];
    printf("[%ld] Eliminando cliente ($%d) Suscriptor :%s \n ",broker.tv_nsec, eliminar_connfd,  publisher_to_broker.topic);
    broker_to_publisher.id = connfd_suscriber;
    if (send(eliminar_connfd, &broker_to_publisher, sizeof(broker_to_publisher), 0) < 0) {
        printf("Send to the server failed...\n");
        exit(1);
    } 
    //numero_suscriptores --;
    eliminar++;
    sem_post(&sem_numero_suscriptores);
    //sem_wait(&sem_desuscripcion);
}
int publicar_datos() {

    struct message mensaje_enviado;
    char mensaje[100];
    clock_gettime(CLOCK_MONOTONIC, &broker);
    strcpy(publisher_to_broker.topic, someone_to_broker.topic);
    someone_to_broker.action = publisher_to_broker.action;
    someone_to_broker.id = publisher_to_broker.id;
    publisher_to_broker.data.time_generated_data = someone_to_broker.data.time_generated_data;
    strcpy(publisher_to_broker.data.data, someone_to_broker.data.data);
    char *mandar_mensaje;
    strcpy(mensaje, someone_to_broker.data.data);

    printf("[%ld] Recibido mensaje para publicar en topic: %s mensaje: %s - Generó: $%ld\n",broker.tv_nsec, publisher_to_broker.topic, mensaje, publisher_to_broker.data.time_generated_data.tv_sec);
    
    broker_to_publisher.response_status = OK;

    if (send(connfd_publisher, &broker_to_publisher, sizeof(broker_to_publisher), 0) < 0) {
        printf("Send to the server failed...\n");
        exit(1);
    } 

    //printf("\n\n\n");
    //for (int i = 0; i < numero_suscriptores; i++){
    //    printf("lista de connfd %d %s  \n", subscriber_list[i], followed_topics[i]);
    //}
    //printf("\n\n\n");
    sem_wait(&sem_pub_bloq);
    for (int j = 0; j < numero_suscriptores; j++){
        // Mandamos el mensaje al cliente si es necesario
            
        if (subscriber_list[j] > 0){
            for (int i = 0; i < MAX_NUMBER_TOPICS; i++){
                if (strcmp(topic_list[i], publisher_to_broker.topic) == 0 ){
                    strcpy(data_topics[i], publisher_to_broker.data.data);
                }
                
                if (strcmp(topic_list[i], followed_topics[j] ) == 0){

                    if (strcmp(modelo_algoritmo, "secuencial") == 0){
                        sem_wait(&sem_bloq_sus);
                    }
                    strcpy(mensaje_enviado.topic, topic_list[i]);
                    strcpy(mensaje_enviado.data.data, data_topics[i]);
                    mensaje_enviado.data.time_generated_data = publisher_to_broker.data.time_generated_data;
                    if (send(subscriber_list[j], &mensaje_enviado, sizeof(mensaje_enviado), 0) < 0) {
                        printf("Send to the server failed...\n");
                        exit(1);
                    } 
                    strcpy(mensaje_enviado.data.data, "");
                }
                sem_post(&sem_pub_bloq);
            }
        }
    }
    sem_post(&sem_pub_bloq);
    
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
    }
    if (send(sockfd, &mandar, sizeof(mandar), 0) < 0) {
        printf("Send to the server failed...\n");
    } 
    return 0;
}

int topic_conection(char *topic) {
    clock_gettime(CLOCK_MONOTONIC, &clientes_time);
     
    // Guardamos el topic donde queremos escribir
    strcpy(message_to_broker.topic, topic);
    //Decimos que queremos registrar un suscriptor
    message_to_broker.action = REGISTER_PUBLISHER;
    //Tiempo empleado para que se conecte el publisher
    message_to_broker.data.time_generated_data = clientes_time;
    //printf("me da tiempo a mandar el mensaje??? %ld\n\n", clientes_time.tv_sec);
    //Mandamos en que topic vamos a escribir y se lo mandamos al broker 
    if (send(sockfd, &message_to_broker, sizeof(message_to_broker), 0) < 0) {
        printf("Send to the server failed...\n");
    }    

    // Recibimos el response del broker
    if ((recv(sockfd, &response_from_broker, sizeof(response_from_broker), 0)) < 0) {
        printf("Recv from the client failed...\n");
    }

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
    //printf("ESTAMOS DENTRO PERO YA TERMINAMOS\n");
    return 0;
}

int send_message(char *topic) {
    struct timespec end;
    clock_gettime(CLOCK_MONOTONIC, &clientes_time);
    //clock_gettime(CLOCK_MONOTONIC, &end);
    
    // Guardamos el topic donde queremos escribir
    strcpy(message_to_broker.topic, topic);
    //Decimos que queremos registrar un suscriptor
    message_to_broker.action = PUBLISH_DATA;
    //Tiempo empleado para que se conecte el publisher
    message_to_broker.data.time_generated_data = clientes_time;
    char mensaje[100] = "FUENLABRADA";
    strcpy(message_to_broker.data.data, mensaje);
    //Mandamos en que topic vamos a escribir y se lo mandamos al broker
    if (send(sockfd, &message_to_broker, sizeof(message_to_broker), 0) < 0) {
        printf("Send to the server failed...\n");
    }    
    printf("[%ld] Publicado mensaje topic: $%s - mensaje: $%s -Generó: $%ld\n",clientes_time.tv_nsec, message_to_broker.topic, message_to_broker.data.data, clientes_time.tv_sec );

    // Recibimos el response del broker
    if ((recv(sockfd, &response_from_broker, sizeof(response_from_broker), 0)) < 0) {
        printf("Recv from the client failed...\n");
    }
}



// Funciones suscriptor

int topic_suscription(char *topic) {
       
    struct timespec end;
    struct message publicacion;
    
    
    // Guardamos el topic donde queremos escribir
    strcpy(message_to_broker.topic, topic);
    //Decimos que queremos registrar un suscriptor
    message_to_broker.action = REGISTER_SUBSCRIBER;
    //Tiempo empleado para que se conecte el publisher
    message_to_broker.data.time_generated_data = end;
    //Mandamos en que topic queremos recibir y se lo mandamos al broker
    if (send(sockfd, &message_to_broker, sizeof(message_to_broker), 0) < 0) {
        printf("Send to the server failed...\n");
    }    
    
    // Recibimos la respuesta del broker
    if ((recv(sockfd, &response_from_broker, sizeof(response_from_broker), 0)) < 0) {
        printf("Recv from the client failed...\n");
    }
    if (response_from_broker.response_status == OK){
        printf("[%ld] Registrado correctamente con ID: $%d para topic $%s\n", clientes_time.tv_nsec, response_from_broker.id, message_to_broker.topic);

    }else {
        printf("[%ld] Error al hacer el registro: $%d\n", clientes_time.tv_nsec, response_from_broker.response_status);

    }
    // Recibimos la posible publicacion del suscriptor
    if ((recv(sockfd, &publicacion, sizeof(publicacion), 0)) < 0) {
        printf("Recv from the client failed...\n");
    }
    clock_gettime(CLOCK_MONOTONIC, &clientes_time);
        
    if (publicacion.data.data[0] != '\0'){
        printf("[%ld] Recibido mensaje topic: $%s - mensaje: $%s - Generó: $%ld - Recibido: $%ld - Latencia: $%f.\n", clientes_time.tv_nsec, publicacion.topic, publicacion.data.data, clientes_time.tv_sec, publicacion.data.time_generated_data.tv_sec, (-publicacion.data.time_generated_data.tv_nsec + clientes_time.tv_nsec)*pow(10, -9));
    }
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
    // Recibimos el response del broker
    if ((recv(sockfd, &response_from_broker, sizeof(response_from_broker), 0)) < 0) {
        printf("Recv from the client failed...\n");
    }
    printf("[%ld] De-Registrado ($%d) correctamente del broker.\n", clientes_time.tv_nsec, response_from_broker.id);
    return 0;
}


int close_client() {
    //printf("CERRAMOS CLIENTE");
    if(close(sockfd) == 1) {
        printf("Close failed\n");
        exit(1);
    }
    return 0;
}