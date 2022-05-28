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
int limite_topics = 0, numero_suscriptores = 0, eliminar = 0, num_lista_sus = 0;
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

pthread_mutex_t sem_time_broker;
pthread_mutex_t sem_clientes_time;
pthread_mutex_t sem_recv_thread;
pthread_mutex_t sem_numero_suscriptores;
pthread_mutex_t sem_enviados;
pthread_mutex_t sem_desuscripcion;
pthread_mutex_t sem_bloq_sus;
pthread_mutex_t sem_pub_bloq;
pthread_mutex_t sem_set_connfd;
// Funciones broker

void semaforo() {

    pthread_mutex_init(&sem_time_broker, NULL);
    pthread_mutex_init(&sem_clientes_time, NULL);
    pthread_mutex_init(&sem_recv_thread, NULL);
    pthread_mutex_init(&sem_numero_suscriptores, NULL);
    pthread_mutex_init(&sem_enviados, NULL);
    pthread_mutex_init(&sem_desuscripcion, NULL);
    pthread_mutex_init(&sem_bloq_sus, NULL);
    pthread_mutex_init(&sem_pub_bloq, NULL);
    pthread_mutex_init(&sem_set_connfd, NULL);
}

int server_conection(int port) {
    srand (time(NULL));
    // Creamos un socket TCP y comprobamos que se ha creado correctamente
    sockfd = socket(AF_INET, SOCK_STREAM, 0);
    if (sockfd == -1) {
        fprintf( stderr, "Socket creation failed...\n");
        exit(1);
    }
    // Creamos la IP y el puerto
    bzero(&serv_addr, sizeof(serv_addr));
    serv_addr.sin_family = AF_INET;
    serv_addr.sin_addr.s_addr = htonl(INADDR_ANY);
    serv_addr.sin_port = htons(port);
    // Asignamos la IP creada al socket y comprobamos
    if ((bind(sockfd, (struct sockaddr*)&serv_addr, sizeof(serv_addr))) != 0) {
        fprintf( stderr, "Socket bind failed...\n");
        exit(1);
    } 
    if ((listen(sockfd, 500)) != 0) {
        fprintf( stderr, "Listen failed...\n");
        exit(1);
    }
}

void *thread_clientes(void *arg) {
    int id;
    while(1){
        id = *(int*)arg;
        //pthread_mutex_lock(&sem_pub_bloq);
        
        if ((recv(id, &someone_to_broker, sizeof(someone_to_broker), 0)) < 0) {
            fprintf( stderr, "Recv from the client failed...\n");
        }
        //fprintf(stderr, "Clientes aceptados: %d\n", id);
        //fprintf(stderr, "EL DESPUES con someone to borker. action %d\n", someone_to_broker.action);
        if (someone_to_broker.action == REGISTER_PUBLISHER){
            conexiones_publicadores();
        }else if (someone_to_broker.action == REGISTER_SUBSCRIBER ){
            conexiones_suscriptores(id);
        }else if (someone_to_broker.action == UNREGISTER_PUBLISHER){
            desconexion_publicador();
            break;
        }else if (someone_to_broker.action == UNREGISTER_SUBSCRIBER){
            desconexion_suscriptor();
            if (strcmp(modelo_algoritmo, "secuencial") == 0){
                pthread_mutex_unlock(&sem_bloq_sus);
            }
            break;
        }else if (someone_to_broker.action == PUBLISH_DATA){
            publicar_datos();
        } 
        //pthread_mutex_unlock(&sem_pub_bloq);
    }
}

int aceptar_cliente(char *mode) {
    pthread_t clientes[500];
    struct cliente recibir;
    clock_gettime(CLOCK_MONOTONIC, &broker);
    int nanosecons = 0;
    int threads_aum = 0;
    int lista_connfd_globales[MAX_NUMBER_PUBLISHERS + MAX_NUMBER_SUSCRIBERS];
    int clientes_acumulados = 0;
    if(strcmp(mode, "paralelo") == 0) {
            strcpy(modelo_algoritmo, mode);
    }else if (strcmp(mode, "secuencial") == 0) {
        strcpy(modelo_algoritmo, mode);
    }else if (strcmp(mode, "justo") == 0) {
        strcpy(modelo_algoritmo, mode);
    }

    while(1){
        connfd = accept(sockfd, (struct sockaddr*)NULL, NULL); //Acepta un nuevo cliente
        
        if (connfd < 0) {
            fprintf( stderr, "Server accept failed...\n");
            exit(1);
        }
        // AQUI LLEGA EL PRIMER mensaje que es decir si lo que recibe lo manda un publicador o un suscriptor
        if ((recv(connfd, &recibir, sizeof(recibir), 0)) < 0) {
            fprintf( stderr, "Recv from the client failed...\n");
        }
        
        pthread_mutex_lock(&sem_set_connfd);
        if (recibir.cliente == 0) {
            connfd_publisher = connfd;
            lista_connfd_globales[clientes_acumulados] = connfd_publisher;
        }else if (recibir.cliente == 1){
            connfd_suscriber = connfd;
            lista_connfd_globales[clientes_acumulados] = connfd_suscriber;
            
        }
        
        if(strcmp(modelo_algoritmo, "paralelo") == 0) {
            fprintf(stderr, "%s\n", modelo_algoritmo);
            
        }else if (strcmp(modelo_algoritmo, "secuencial") == 0) {
            strcpy(modelo_algoritmo, mode);
            
            if(pthread_create(&clientes[threads_aum], NULL, thread_clientes, (void *) &lista_connfd_globales[clientes_acumulados] ) != 0) {
                fprintf( stderr, "Fallo al ejecutar pthread_create de clientes \n");
                exit(1);
            }
            //if(pthread_join(clientes[threads_aum], NULL) != 0){
            //    fprintf( stderr, "Fallo al ejecutar pthread_join de clientes \n");
            //    exit(1);
            //}
        }else if (strcmp(modelo_algoritmo, "justo") == 0) {
            fprintf(stderr, "%s\n", modelo_algoritmo);
        }
        
        threads_aum++;
        clientes_acumulados++;
        pthread_mutex_unlock(&sem_set_connfd);

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
        fprintf( stderr, "[%ld] Nuevo cliente ( %d) Publicador conectado : %s\n ", broker.tv_nsec, connfd_publisher,  publisher_to_broker.topic);
        for (int i = 0; i < limite_topics; i++){
            fprintf( stderr, " %s: 1 Publicador - %d Suscriptores\n", topic_list[i], numero_suscriptores);
        }
        broker_to_publisher.id = connfd_publisher;

        if (limite_topics >= MAX_NUMBER_TOPICS){
            broker_to_publisher.response_status = LIMIT;
        }else {
            broker_to_publisher.response_status = OK;
        }
        if (send(connfd_publisher, &broker_to_publisher, sizeof(broker_to_publisher), 0) < 0) {
            fprintf( stderr, "Send to the server failed...\n");
            exit(1);
        }
    }
    return 0;
}

int conexiones_suscriptores(int buen_connfd) {
    pthread_mutex_lock(&sem_numero_suscriptores);
    clock_gettime(CLOCK_MONOTONIC, &broker);
    strcpy(suscriber_to_broker.topic, someone_to_broker.topic);
    someone_to_broker.action = suscriber_to_broker.action;
    someone_to_broker.id = suscriber_to_broker.id;
    strcpy(suscriber_to_broker.data.data, someone_to_broker.data.data);
    
    

    int nanosecons = suscriber_to_broker.data.time_generated_data.tv_nsec - broker.tv_nsec;
    //for (int i = numero_suscriptores - 1; i < numero_suscriptores; i++){
    //    subscriber_list[i] = connfd_suscriber;
    //    strcpy(followed_topics[i], suscriber_to_broker.topic);
    //    fprintf( stderr, "JAMAUS ARMYYYY %d %d   %s  \n\n", numero_suscriptores, subscriber_list[i], followed_topics[i]);
    //}

    subscriber_list[numero_suscriptores] = buen_connfd;
    strcpy(followed_topics[numero_suscriptores], suscriber_to_broker.topic);
    //fprintf( stderr, "JAMAUS ARMYYYY %d %d   %s  \n\n", numero_suscriptores, subscriber_list[numero_suscriptores], followed_topics[numero_suscriptores]);
    //int mandar_connfd = subscriber_list[numero_suscriptores - 1];
    int mandar_connfd = subscriber_list[numero_suscriptores];

    numero_suscriptores++;
    num_lista_sus = numero_suscriptores;
    fprintf( stderr, "[%ld] Nuevo cliente ( %d) Suscriptor conectado : %s\n", broker.tv_nsec, mandar_connfd ,  suscriber_to_broker.topic);
    if (limite_topics == 0){
        fprintf( stderr, " %s: 1 Publicador - %d Suscriptores\n", suscriber_to_broker.topic, numero_suscriptores);
    }
    for (int i = 0; i < limite_topics; i++){
        fprintf( stderr, " %s: 1 Publicador - %d Suscriptores\n", topic_list[i], numero_suscriptores);
    }
    broker_to_publisher.id = subscriber_list[numero_suscriptores - 1];
    
    //fprintf( stderr, "MENSAJITO DEL BROKER PAPA, %d\n", mandar_connfd);
    if (send(mandar_connfd, &broker_to_publisher, sizeof(broker_to_publisher), 0) < 0) {
        fprintf( stderr, "Send to the server failed...\n");
        exit(1);
    } 
    //fprintf(stderr, "Se termino de mandar compadre\n");
    
    pthread_mutex_unlock(&sem_numero_suscriptores);    
    return 0;
}

int desconexion_publicador() {
    int nanosecons = publisher_to_broker.data.time_generated_data.tv_nsec - broker.tv_nsec;
    clock_gettime(CLOCK_MONOTONIC, &broker);
    fprintf( stderr, "[%ld] Eliminando cliente ( %d) Publicador : %s\n",broker.tv_nsec, connfd_publisher,  publisher_to_broker.topic);
    broker_to_publisher.id = connfd_publisher;

    if (send(connfd_publisher, &broker_to_publisher, sizeof(broker_to_publisher), 0) < 0) {
        fprintf( stderr, "Send to the server failed...\n");
        exit(1);
    } 
    
    // Reducimos el numero de la lista de topics
    //if (limite_topics > 0) {
    //    limite_topics -= 1;
    //}

}


int desconexion_suscriptor(){

    pthread_mutex_lock(&sem_numero_suscriptores);
    int existe = 0;
    int nanosecons = publisher_to_broker.data.time_generated_data.tv_nsec - broker.tv_nsec;
    int  eliminar_connfd = someone_to_broker.id;
    
    for (int j = 0; j < MAX_NUMBER_SUSCRIBERS; j++){
        if (eliminar_connfd == subscriber_list[j]){
            //fprintf(stderr, "CONNFD REAL: %d, el de la lista: %d, en la posicion %d\n", eliminar_connfd, subscriber_list[j], j);
            subscriber_list[j] = 0;
            existe = 1;
            if (existe = 1){
                fprintf( stderr, "[%ld] Eliminando cliente ( %d) Suscriptor :%s \n ",broker.tv_nsec, eliminar_connfd,  publisher_to_broker.topic);
                broker_to_publisher.id = eliminar_connfd;
                if (send(eliminar_connfd, &broker_to_publisher, sizeof(broker_to_publisher), 0) < 0) {
                    fprintf( stderr, "Send to the server failed...\n");
                    exit(1);
                } 
                numero_suscriptores--;
                for (int i = 0; i < limite_topics; i++){
                    fprintf( stderr, " %s: 1 Publicador - %d Suscriptores\n", topic_list[i], numero_suscriptores);
                }
            }
        }
    }
    connfd_suscriber = 0; 
    pthread_mutex_unlock(&sem_numero_suscriptores);
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

    fprintf( stderr, "[%ld] Recibido mensaje para publicar en topic: %s mensaje: %s - Generó:  %ld\n",broker.tv_nsec, publisher_to_broker.topic, mensaje, publisher_to_broker.data.time_generated_data.tv_sec);
    
    broker_to_publisher.response_status = OK;

    if (send(connfd_publisher, &broker_to_publisher, sizeof(broker_to_publisher), 0) < 0) {
        fprintf( stderr, "Send to the server failed...\n");
        exit(1);
    } 

    //fprintf( stderr, "\n\n\n");
    //for (int i = 0; i < numero_suscriptores; i++){
    //    fprintf( stderr, "lista de connfd %d %s  \n", subscriber_list[i], followed_topics[i]);
    //}
    //fprintf( stderr, "\n\n\n");

    //fprintf( stderr, "\n\n\n");


    pthread_mutex_lock(&sem_pub_bloq);
    for (int j = 0; j < num_lista_sus; j++){ //numero_suscriptores
        // Mandamos el mensaje al cliente si es necesario
        
        if (subscriber_list[j] > 0){
            //fprintf(stderr, "Numero suscriptor: %d\n", subscriber_list[j]);
            for (int i = 0; i < limite_topics; i++){
                if (strcmp(topic_list[i], publisher_to_broker.topic) == 0 ){
                    strcpy(data_topics[i], publisher_to_broker.data.data);
                }
                //fprintf(stderr, "TOPIC LIST %s, followed_topics: %s con el connfd %d y comparacion de ambos %d \n", topic_list[i], followed_topics[j], subscriber_list[j], strcmp(modelo_algoritmo, "secuencial"));
                if (strcmp(topic_list[i], followed_topics[j] ) == 0){

                    if (strcmp(modelo_algoritmo, "secuencial") == 0){
                        pthread_mutex_lock(&sem_bloq_sus);
                    }
                    strcpy(mensaje_enviado.topic, topic_list[i]);
                    strcpy(mensaje_enviado.data.data, data_topics[i]);
                    mensaje_enviado.data.time_generated_data = publisher_to_broker.data.time_generated_data;
                    //fprintf( stderr, "MENSAJITO DEL BROKER PAPA, %d\n", subscriber_list[j]);
                    if (send(subscriber_list[j], &mensaje_enviado, sizeof(mensaje_enviado), 0) < 0) {
                        fprintf( stderr, "Send to the server failed...\n");
                        exit(1);
                    } 
                    strcpy(mensaje_enviado.data.data, "");
                }
                //pthread_mutex_unlock(&sem_pub_bloq);
            }
        }
    }
    pthread_mutex_unlock(&sem_pub_bloq);
    
    return 0;
}

int close_server() {
    if(close(sockfd) == 1) {
        fprintf( stderr, "Close failed\n");
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
        fprintf( stderr, "[%ld] Publisher conectado con el broker correctamente.\n", clientes_time.tv_nsec);
        mandar.cliente = PUBLICADOR;
    }else if(tipo == 1){
        fprintf( stderr, "[%ld] Suscriber conectado con el broker correctamente.\n", clientes_time.tv_nsec);
        mandar.cliente = SUSCRIPTOR;
    }
    // Creamos un socket TCP y comprobamos que se ha creado correctamente
    sockfd = socket(AF_INET, SOCK_STREAM, 0);
    if (sockfd == -1) {
        fprintf( stderr, "Socket creation failed...\n" );
        exit(1);
    }
    // Le asignamos una IP y un puerto
    bzero(&servaddr, sizeof(servaddr));
    servaddr.sin_family = AF_INET;
    servaddr.sin_addr.s_addr = inet_addr(ip);
    servaddr.sin_port = htons(port);

    // Conectamos el cliente al socket del servidor y comprobamos
    if ((connect(sockfd, (struct sockaddr*)&servaddr, sizeof(servaddr))) < 0) {
        fprintf( stderr, "Connection with the server failed...\n");
        exit(1);
    }
    if (send(sockfd, &mandar, sizeof(mandar), 0) < 0) {
        fprintf( stderr, "Send to the server failed...\n");
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
    //fprintf( stderr, "me da tiempo a mandar el mensaje??? %ld\n\n", clientes_time.tv_sec);
    //Mandamos en que topic vamos a escribir y se lo mandamos al broker 
    if (send(sockfd, &message_to_broker, sizeof(message_to_broker), 0) < 0) {
        fprintf( stderr, "Send to the server failed...\n");
    }    

    // Recibimos el response del broker
    if ((recv(sockfd, &response_from_broker, sizeof(response_from_broker), 0)) < 0) {
        fprintf( stderr, "Recv from the client failed...\n");
    }

    if (response_from_broker.response_status == OK){
        fprintf( stderr, "[%ld] Registrado correctamente con ID:  %d para topic  %s\n", clientes_time.tv_nsec, response_from_broker.id, message_to_broker.topic );
    }else{
        fprintf( stderr, "[%ld] Error al hacer el registro: error= %d\n", clientes_time.tv_nsec, response_from_broker.response_status);
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
        fprintf( stderr, "Send to the server failed...\n");
    } 

    // Recibimos el response del broker
    if ((recv(sockfd, &response_from_broker, sizeof(response_from_broker), 0)) < 0) {
        fprintf( stderr, "Recv from the client failed...\n");
    }

    fprintf( stderr, "[%ld] De-Registrado ( %d) correctamente del broker.\n", clientes_time.tv_nsec, response_from_broker.id);
    //fprintf( stderr, "ESTAMOS DENTRO PERO YA TERMINAMOS\n");
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
        fprintf( stderr, "Send to the server failed...\n");
    }    
    fprintf( stderr, "[%ld] Publicado mensaje topic:  %s - mensaje:  %s -Generó:  %ld\n",clientes_time.tv_nsec, message_to_broker.topic, message_to_broker.data.data, clientes_time.tv_sec );

    // Recibimos el response del broker
    if ((recv(sockfd, &response_from_broker, sizeof(response_from_broker), 0)) < 0) {
        fprintf( stderr, "Recv from the client failed...\n");
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
        fprintf( stderr, "Send to the server failed...\n");
    }    
    
    // Recibimos la respuesta del broker
    if ((recv(sockfd, &response_from_broker, sizeof(response_from_broker), 0)) < 0) {
        fprintf( stderr, "Recv from the client failed...\n");
    }
    if (response_from_broker.response_status == OK){
        fprintf( stderr, "[%ld] Registrado correctamente con ID:  %d para topic  %s\n", clientes_time.tv_nsec, response_from_broker.id, message_to_broker.topic);

    }else {
        fprintf( stderr, "[%ld] Error al hacer el registro:  %d\n", clientes_time.tv_nsec, response_from_broker.response_status);

    }

    // Recibimos la posible publicacion del suscriptor
    if ((recv(sockfd, &publicacion, sizeof(publicacion), 0)) < 0) {
        fprintf( stderr, "Recv from the client failed...\n");
    }
    clock_gettime(CLOCK_MONOTONIC, &clientes_time);
        
    if (publicacion.data.data[0] != '\0'){
        fprintf( stderr, "[%ld] Recibido mensaje topic:  %s - mensaje:  %s - Generó:  %ld - Recibido:  %ld - Latencia:  %f.\n", clientes_time.tv_nsec, publicacion.topic, publicacion.data.data, clientes_time.tv_sec, publicacion.data.time_generated_data.tv_sec, (-publicacion.data.time_generated_data.tv_nsec + clientes_time.tv_nsec)*pow(10, -9));
    }
    return response_from_broker.id;
}


int unfollow_topic(char *topic, int id_actual ) {
    clock_gettime(CLOCK_MONOTONIC, &clientes_time);
    // Guardamos el topic donde queremos escribir
    strcpy(message_to_broker.topic, topic);
    //Decimos que queremos registrar un suscriptor
    message_to_broker.action = UNREGISTER_SUBSCRIBER;
    message_to_broker.id = id_actual;
    fprintf(stderr, "Desconectamos nuestro suscriptor\n");
    //Mandamos en que topic vamos a desuscribirnos y se lo mandamos al broker
    if (send(sockfd, &message_to_broker, sizeof(message_to_broker), 0) < 0) {
        fprintf( stderr, "Send to the server failed...\n");
    } 
    // Recibimos el response del broker
    if ((recv(sockfd, &response_from_broker, sizeof(response_from_broker), 0)) < 0) {
        fprintf( stderr, "Recv from the client failed...\n");
    }
    fprintf( stderr, "[%ld] De-Registrado ( %d) correctamente del broker.\n", clientes_time.tv_nsec, response_from_broker.id);
    return 0;
}


int close_client() {
    //fprintf( stderr, "CERRAMOS CLIENTE");
    if(close(sockfd) == 1) {
        fprintf( stderr, "Close failed\n");
        exit(1);
    }
    return 0;
}