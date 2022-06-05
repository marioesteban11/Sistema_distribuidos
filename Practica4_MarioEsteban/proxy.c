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
//struct message someone_to_broker;

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
pthread_mutex_t sem_bloq_pub_paralelo;
pthread_barrier_t barrier;
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
    pthread_mutex_init(&sem_bloq_pub_paralelo, NULL);
    
}

int server_conection(int port) {
    srand (time(NULL));
    // Creamos un socket TCP y comprobamos que se ha creado correctamente
    sockfd = socket(AF_INET, SOCK_STREAM, 0);
    if (sockfd == -1) {
        printf( "Socket creation failed...\n");
        exit(1);
    }
    // Creamos la IP y el puerto
    bzero(&serv_addr, sizeof(serv_addr));
    serv_addr.sin_family = AF_INET;
    serv_addr.sin_addr.s_addr = htonl(INADDR_ANY);
    serv_addr.sin_port = htons(port);
    // Asignamos la IP creada al socket y comprobamos
    if ((bind(sockfd, (struct sockaddr*)&serv_addr, sizeof(serv_addr))) != 0) {
        printf( "Socket bind failed...\n");
        exit(1);
    } 
    if ((listen(sockfd, 500)) != 0) {
        printf( "Listen failed...\n");
        exit(1);
    }
}

void *thread_clientes(void *arg) {
    int id;
    struct message someone_to_broker;
    while(1){
        id = *(int*)arg;
        if ((recv(id, &someone_to_broker, sizeof(someone_to_broker), 0)) < 0) {
            printf( "Recv from the client failed...\n");
        }
        
        //fprintf(stderr, "EL coon someone to borker. action %d e id: %d\n", someone_to_broker.action, id);
        if (someone_to_broker.action == REGISTER_PUBLISHER){
            conexiones_publicadores(someone_to_broker);
        }else if (someone_to_broker.action == REGISTER_SUBSCRIBER ){
            conexiones_suscriptores(id, someone_to_broker);
        }else if (someone_to_broker.action == UNREGISTER_PUBLISHER){
            desconexion_publicador(someone_to_broker);
            break;
        }else if (someone_to_broker.action == UNREGISTER_SUBSCRIBER){
            desconexion_suscriptor(id, someone_to_broker);
            break;
        }else if (someone_to_broker.action == PUBLISH_DATA){
            publicar_datos(someone_to_broker);
        }
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
        pthread_mutex_lock(&sem_set_connfd);
        connfd = accept(sockfd, (struct sockaddr*)NULL, NULL); //Acepta un nuevo cliente
        
        if (connfd < 0) {
            printf( "Server accept failed...\n");
            exit(1);
        }
        // AQUI LLEGA EL PRIMER mensaje que es decir si lo que recibe lo manda un publicador o un suscriptor
        if ((recv(connfd, &recibir, sizeof(recibir), 0)) < 0) {
            printf( "Recv from the client failed...\n");
        }
        
        
        if (recibir.cliente == 0) {
            connfd_publisher = connfd;
            lista_connfd_globales[clientes_acumulados] = connfd_publisher;
        }else if (recibir.cliente == 1){
            connfd_suscriber = connfd;
            lista_connfd_globales[clientes_acumulados] = connfd_suscriber;
            
        }
        
        //if(strcmp(modelo_algoritmo, "paralelo") == 0) {
        //    fprintf(stderr, "%s\n", modelo_algoritmo);
        //    
        //}else if (strcmp(modelo_algoritmo, "secuencial") == 0) {
        //    if(pthread_create(&clientes[threads_aum], NULL, thread_clientes, (void *) &lista_connfd_globales[clientes_acumulados] ) != 0) {
        //        printf( "Fallo al ejecutar pthread_create de clientes \n");
        //        exit(1);
        //    }
        //}else if (strcmp(modelo_algoritmo, "justo") == 0) {
        //    fprintf(stderr, "%s\n", modelo_algoritmo);
        //}
        
        if(pthread_create(&clientes[threads_aum], NULL, thread_clientes, (void *) &lista_connfd_globales[clientes_acumulados] ) != 0) {
            printf( "Fallo al ejecutar pthread_create de clientes \n");
            exit(1);
        }


        threads_aum++;
        clientes_acumulados++;
        pthread_mutex_unlock(&sem_set_connfd);

    }
    return 0;
}

int conexiones_publicadores(struct message someone_to_broker){ 

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
        printf( "[%ld] Nuevo cliente ( %d) Publicador conectado : %s\n ", broker.tv_nsec, connfd_publisher,  publisher_to_broker.topic);
        for (int i = 0; i < limite_topics; i++){
            printf( " %s: %d Publicador - %d Suscriptores\n", topic_list[i],limite_topics, numero_suscriptores);
        }
        broker_to_publisher.id = connfd_publisher;

        if (limite_topics >= MAX_NUMBER_TOPICS){
            broker_to_publisher.response_status = LIMIT;
        }else {
            broker_to_publisher.response_status = OK;
        }
        if (send(connfd_publisher, &broker_to_publisher, sizeof(broker_to_publisher), 0) < 0) {
            printf( "Send to the server publisher failed: %d...\n", connfd_publisher);
            exit(1);
        }
    }
    return 0;
}

int conexiones_suscriptores(int buen_connfd, struct message someone_to_broker) {
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
    //    printf( "JAMAUS ARMYYYY %d %d   %s  \n\n", numero_suscriptores, subscriber_list[i], followed_topics[i]);
    //}

    subscriber_list[numero_suscriptores] = buen_connfd;
    strcpy(followed_topics[numero_suscriptores], suscriber_to_broker.topic);
    //printf( "JAMAUS ARMYYYY %d %d   %s  \n\n", numero_suscriptores, subscriber_list[numero_suscriptores], followed_topics[numero_suscriptores]);
    //int mandar_connfd = subscriber_list[numero_suscriptores - 1];
    int mandar_connfd = buen_connfd;

    numero_suscriptores++;
    num_lista_sus = numero_suscriptores;
    printf( "[%ld] Nuevo cliente ( %d) Suscriptor conectado : %s\n", broker.tv_nsec, mandar_connfd ,  suscriber_to_broker.topic);
    if (limite_topics == 0){
        printf( " %s: %d Publicador - %d Suscriptores\n", suscriber_to_broker.topic, limite_topics, numero_suscriptores);
    }
    for (int i = 0; i < limite_topics; i++){
        printf( " %s: %d Publicador - %d Suscriptores\n", topic_list[i],limite_topics, numero_suscriptores);
    }
    broker_to_publisher.id = subscriber_list[numero_suscriptores - 1];
    
    //printf( "MENSAJITO DEL BROKER PAPA, %d\n", mandar_connfd);
    if (send(mandar_connfd, &broker_to_publisher, sizeof(broker_to_publisher), 0) < 0) {
        printf( "Send to the server suscriptor failed:%d ...\n", mandar_connfd);
        exit(1);
    } 
    //fprintf(stderr, "Se termino de mandar compadre\n");
    
    pthread_mutex_unlock(&sem_numero_suscriptores);    
    return 0;
}

int desconexion_publicador(struct message someone_to_broker) {
    int nanosecons = publisher_to_broker.data.time_generated_data.tv_nsec - broker.tv_nsec;
    clock_gettime(CLOCK_MONOTONIC, &broker);
    printf( "[%ld] Eliminando cliente ( %d) Publicador : %s\n",broker.tv_nsec, connfd_publisher,  publisher_to_broker.topic);
    broker_to_publisher.id = connfd_publisher;

    if (send(connfd_publisher, &broker_to_publisher, sizeof(broker_to_publisher), 0) < 0) {
        printf( "Send to the server failed...\n");
        exit(1);
    } 
    
    // Reducimos el numero de la lista de topics
    if (limite_topics > 0) {
        limite_topics -= 1;
    }
    for (int i = 0; i < limite_topics; i++){
        printf( " %s: %d Publicador - %d Suscriptores\n", topic_list[i], limite_topics, numero_suscriptores);
    }

}


int desconexion_suscriptor(int buen_connfd, struct message someone_to_broker){

    pthread_mutex_lock(&sem_numero_suscriptores);
    int existe = 0;
    int nanosecons = publisher_to_broker.data.time_generated_data.tv_nsec - broker.tv_nsec;
    int  eliminar_connfd = buen_connfd; //someone_to_broker.id


    for (int j = 0; j < MAX_NUMBER_SUSCRIBERS; j++){
        //fprintf(stderr, "CONNFD REAL: %d, el de la lista: %d, en la posicion %d\n", eliminar_connfd, subscriber_list[j], j);
        if (eliminar_connfd == subscriber_list[j]){
            //fprintf(stderr, "CONNFD REAL: %d, el de la lista: %d, en la posicion %d\n", eliminar_connfd, subscriber_list[j], j);
            subscriber_list[j] = 0;
            existe = 1;
            if (existe = 1){
                printf( "[%ld] Eliminando cliente ( %d) Suscriptor :%s \n ",broker.tv_nsec, eliminar_connfd,  publisher_to_broker.topic);
                broker_to_publisher.id = eliminar_connfd;
                if (send(eliminar_connfd, &broker_to_publisher, sizeof(broker_to_publisher), 0) < 0) {
                    printf( "Send to the server failed...\n");
                    exit(1);
                } 
                numero_suscriptores--;
                for (int i = 0; i < limite_topics; i++){
                    printf( " %s: %d Publicador - %d Suscriptores\n", topic_list[i], limite_topics, numero_suscriptores);
                }
            }
        }
    }
    connfd_suscriber = 0; 
    //fprintf(stderr, "\n\n\n");
    //for (int i = 0; i < num_lista_sus; i++){
    //    fprintf(stderr, "Sub: %d desuscrito\n", subscriber_list[i]);
    //}
    //fprintf(stderr, "\n\n\n");
    pthread_mutex_unlock(&sem_numero_suscriptores);
}

void publicar_secuencial(){
    struct message mensaje_enviado;
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
                    
                    if (send(subscriber_list[j], &mensaje_enviado, sizeof(mensaje_enviado), 0) < 0) {
                        printf( "Send to the server failed...\n");
                        exit(1);
                    } 
                    //printf( "MENSAJITO DEL BROKER PAPA, %d\n", subscriber_list[j]);
                    printf("[%ld] Publicado mensaje topic: %s, - mensaje: %s - Generó: %ld\n",broker.tv_nsec, topic_list[i], mensaje_enviado.data.data, mensaje_enviado.data.time_generated_data.tv_sec );
                    strcpy(mensaje_enviado.data.data, "");
                    if ( strcmp(modelo_algoritmo, "secuencial") == 0){
                        pthread_mutex_unlock(&sem_bloq_sus);
                    }
                }
                //pthread_mutex_unlock(&sem_pub_bloq);
            }
        }
    }
    pthread_mutex_unlock(&sem_pub_bloq);
}


void *publish_paralelo(void *arg) {
    if (strcmp(modelo_algoritmo, "justo") == 0){
        printf("SOY JUSTO EN LA BARRERA\n");
        pthread_barrier_wait(&barrier);
        printf("SOMOS LIBREEEEEEEEEEEEEEES\n");
    }
    pthread_mutex_lock(&sem_bloq_pub_paralelo);
    struct message mensaje_enviado;
    int id = *(int*)arg;
    //publicar_paralelo();
    //printf( "\n\n\n");
    //for (int i = 0; i < num_lista_sus; i++){
    //    printf( "lista de connfd %d %s  \n", subscriber_list[i], followed_topics[i]);
    //}
    //printf( "\n\n\n");
    printf("ID DEL INDICADO: %d\n", id);
    for (int j = 0; j < num_lista_sus; j++){ //numero_suscriptores
        if (id == subscriber_list[j] && subscriber_list[j] > 0){
            for (int i = 0; i < limite_topics; i++){
                if (strcmp(topic_list[i], publisher_to_broker.topic) == 0 ){
                    strcpy(data_topics[i], publisher_to_broker.data.data);
                }
                //fprintf(stderr, "TOPIC LIST %s, followed_topics: %s con el connfd %d y comparacion de ambos %d \n", topic_list[i], followed_topics[j], subscriber_list[j], strcmp(modelo_algoritmo, "secuencial"));
                if (strcmp(topic_list[i], followed_topics[j] ) == 0){
                    strcpy(mensaje_enviado.topic, topic_list[i]);
                    strcpy(mensaje_enviado.data.data, data_topics[i]);
                    mensaje_enviado.data.time_generated_data = publisher_to_broker.data.time_generated_data;
                    
                    if (send(subscriber_list[j], &mensaje_enviado, sizeof(mensaje_enviado), 0) < 0) {
                        printf( "Send to the server failed...\n");
                        exit(1);
                    } 
                    //printf( "MENSAJITO DEL BROKER PAPA, %d\n", subscriber_list[j]);
                    printf("[%ld] Publicado mensaje topic: %s, - mensaje: %s - Generó: %ld\n",broker.tv_nsec, topic_list[i], mensaje_enviado.data.data, mensaje_enviado.data.time_generated_data.tv_sec );
                    strcpy(mensaje_enviado.data.data, "");
                }
            }
        }
    }
    pthread_mutex_unlock(&sem_bloq_pub_paralelo);
    //printf("ESTOY EN PARALELO MIS PANITAAAAAAAAAAAAS con id %d\n", id);
    
}

int publicar_datos(struct message someone_to_broker) {
    //pthread_t clientes_paralelo[numero_suscriptores];
    pthread_t *clientes_paralelo = malloc(numero_suscriptores*sizeof(pthread_t));

    char mensaje[100];
    clock_gettime(CLOCK_MONOTONIC, &broker);
    strcpy(publisher_to_broker.topic, someone_to_broker.topic);
    someone_to_broker.action = publisher_to_broker.action;
    someone_to_broker.id = publisher_to_broker.id;
    publisher_to_broker.data.time_generated_data = someone_to_broker.data.time_generated_data;
    strcpy(publisher_to_broker.data.data, someone_to_broker.data.data);
    char *mandar_mensaje;
    strcpy(mensaje, someone_to_broker.data.data);

    printf( "[%ld] Recibido mensaje para publicar en topic: %s mensaje: %s - Generó:  %ld\n",broker.tv_nsec, publisher_to_broker.topic, mensaje, publisher_to_broker.data.time_generated_data.tv_sec);
    
    broker_to_publisher.response_status = OK;

    if (send(connfd_publisher, &broker_to_publisher, sizeof(broker_to_publisher), 0) < 0) {
        printf( "Send to the server failed...\n");
        exit(1);
    } 
    
    
    if ( strcmp(modelo_algoritmo, "paralelo") == 0 || strcmp(modelo_algoritmo, "justo") == 0){
        pthread_barrier_init(&barrier, NULL, num_lista_sus);
        for (int i = 0; i < num_lista_sus; i++){
            //printf("MANDAMOS BIEN EL ID: %d\n", subscriber_list[i]);
            if(pthread_create(&clientes_paralelo[i], NULL, publish_paralelo, (void *) &subscriber_list[i] ) != 0) {
                printf( "Fallo al ejecutar pthread_create de clientes \n");
                exit(1);
            }  
        }
        for(int i = 0; i < num_lista_sus; i++){
            if(pthread_join(clientes_paralelo[i], NULL) != 0){
                printf( "Fallo al ejecutar pthread_join de clientes \n");
                exit(1);
            }
        }   
    }else if (strcmp(modelo_algoritmo, "secuencial") == 0){
        publicar_secuencial();
    }
    
    free(clientes_paralelo);
    return 0;
}

int close_server() {
    if(close(sockfd) == 1) {
        printf( "Close failed\n");
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
        printf( "[%ld] Publisher conectado con el broker correctamente.\n", clientes_time.tv_nsec);
        mandar.cliente = PUBLICADOR;
    }else if(tipo == 1){
        printf( "[%ld] Suscriber conectado con el broker correctamente.\n", clientes_time.tv_nsec);
        mandar.cliente = SUSCRIPTOR;
    }
    // Creamos un socket TCP y comprobamos que se ha creado correctamente
    sockfd = socket(AF_INET, SOCK_STREAM, 0);
    if (sockfd == -1) {
        printf( "Socket creation failed...\n" );
        exit(1);
    }
    // Le asignamos una IP y un puerto
    bzero(&servaddr, sizeof(servaddr));
    servaddr.sin_family = AF_INET;
    servaddr.sin_addr.s_addr = inet_addr(ip);
    servaddr.sin_port = htons(port);

    // Conectamos el cliente al socket del servidor y comprobamos
    if ((connect(sockfd, (struct sockaddr*)&servaddr, sizeof(servaddr))) < 0) {
        printf( "Connection with the server failed...\n");
        exit(1);
    }
    if (send(sockfd, &mandar, sizeof(mandar), 0) < 0) {
        printf( "Send to the server failed...\n");
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
    //printf( "me da tiempo a mandar el mensaje??? %ld\n\n", clientes_time.tv_sec);
    //Mandamos en que topic vamos a escribir y se lo mandamos al broker 
    if (send(sockfd, &message_to_broker, sizeof(message_to_broker), 0) < 0) {
        printf( "Send to the server failed...\n");
    }    

    // Recibimos el response del broker
    if ((recv(sockfd, &response_from_broker, sizeof(response_from_broker), 0)) < 0) {
        printf( "Recv from the client failed...\n");
    }

    if (response_from_broker.response_status == OK){
        printf( "[%ld] Registrado correctamente con ID:  %d para topic  %s\n", clientes_time.tv_nsec, response_from_broker.id, message_to_broker.topic );
    }else{
        printf( "[%ld] Error al hacer el registro: error= %d\n", clientes_time.tv_nsec, response_from_broker.response_status);
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
        printf( "Send to the server failed...\n");
    } 

    // Recibimos el response del broker
    if ((recv(sockfd, &response_from_broker, sizeof(response_from_broker), 0)) < 0) {
        printf( "Recv from the client failed...\n");
    }

    printf( "[%ld] De-Registrado ( %d) correctamente del broker.\n", clientes_time.tv_nsec, response_from_broker.id);
    //printf( "ESTAMOS DENTRO PERO YA TERMINAMOS\n");
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
        printf( "Send to the server failed...\n");
    }    
    printf( "[%ld] Publicado mensaje topic:  %s - mensaje:  %s -Generó:  %ld\n",clientes_time.tv_nsec, message_to_broker.topic, message_to_broker.data.data, clientes_time.tv_sec );

    // Recibimos el response del broker
    if ((recv(sockfd, &response_from_broker, sizeof(response_from_broker), 0)) < 0) {
        printf( "Recv from the client failed...\n");
    }
}



// Funciones suscriptor
int topic_suscription(char *topic) {
       
    struct timespec end;
    
    
    // Guardamos el topic donde queremos escribir
    strcpy(message_to_broker.topic, topic);
    //Decimos que queremos registrar un suscriptor
    message_to_broker.action = REGISTER_SUBSCRIBER;
    //Tiempo empleado para que se conecte el publisher
    message_to_broker.data.time_generated_data = end;
    //Mandamos en que topic queremos recibir y se lo mandamos al broker
    if (send(sockfd, &message_to_broker, sizeof(message_to_broker), 0) < 0) {
        printf( "Send to the server failed...\n");
    }    
    
    // Recibimos la respuesta del broker
    if ((recv(sockfd, &response_from_broker, sizeof(response_from_broker), 0)) < 0) {
        printf( "Recv from the client failed...\n");
    }
    if (response_from_broker.response_status == OK){
        printf( "[%ld] Registrado correctamente con ID:  %d para topic  %s\n", clientes_time.tv_nsec, response_from_broker.id, message_to_broker.topic);

    }else {
        printf( "[%ld] Error al hacer el registro:  %d\n", clientes_time.tv_nsec, response_from_broker.response_status);

    }
    
     
    return response_from_broker.id;
}

int get_message(char *topic){

    
    struct message publicacion;
    int salida = 0;
    if ((recv(sockfd, &publicacion, sizeof(publicacion), 0)) < 0) {
        printf( "Recv from the client failed...\n");
    }
    clock_gettime(CLOCK_MONOTONIC, &clientes_time);
        
    if (publicacion.data.data[0] != '\0' && strcmp(publicacion.data.data, "FUENLABRADA") == 0){
        printf( "[%ld] Recibido mensaje topic:  %s - mensaje:  %s - Generó:  %ld - Recibido:  %ld - Latencia:  %f.\n", clientes_time.tv_nsec, publicacion.topic, publicacion.data.data, clientes_time.tv_sec, publicacion.data.time_generated_data.tv_sec, (-publicacion.data.time_generated_data.tv_nsec + clientes_time.tv_nsec)*pow(10, -9));
    }else {
        salida = 1;
    }
    // Recibimos la posible publicacion del suscriptor
    return salida;
}


int unfollow_topic(char *topic, int id_actual ) {
    clock_gettime(CLOCK_MONOTONIC, &clientes_time);
    // Guardamos el topic donde queremos escribir
    strcpy(message_to_broker.topic, topic);
    //Decimos que queremos registrar un suscriptor
    message_to_broker.action = UNREGISTER_SUBSCRIBER;
    message_to_broker.id = id_actual;
    //fprintf(stderr, "Desconectamos nuestro suscriptor\n");
    //Mandamos en que topic vamos a desuscribirnos y se lo mandamos al broker
    if (send(sockfd, &message_to_broker, sizeof(message_to_broker), 0) < 0) {
        printf( "Send to the server failed...\n");
    } 
    // Recibimos el response del broker
    if ((recv(sockfd, &response_from_broker, sizeof(response_from_broker), 0)) < 0) {
        printf( "Recv from the client failed...\n");
    }
    printf( "[%ld] De-Registrado ( %d) correctamente del broker.\n", clientes_time.tv_nsec, response_from_broker.id);
    return 0;
}


int close_client() {
    //printf( "CERRAMOS CLIENTE");
    if(close(sockfd) == 1) {
        printf( "Close failed\n");
        exit(1);
    }
    return 0;
}