#include "proxy.h"

//Variables para el cliente y el servidor
int sockfd = 0, connfd = 0;
int connfd_writers = 0, connfd_readers = 0;
struct sockaddr_in servaddr;
struct sockaddr_in serv_addr;
struct response servidor;

FILE* file;


//Semillas de los semáforos
sem_t sem_mutex;
sem_t sem_numero_clientes;
sem_t sem_escritores;
sem_t sem_lectores;
sem_t sem_escritores_max;
sem_t sem_maximos_lectores;
sem_t sem_cliente_escritor;
sem_t sem_numero_escritores;
sem_t sem_cliente_lector;
sem_t sem_numero_lectores;
sem_t sem_ratio;
sem_t sem_ratio_counter;

//Variables para la lógica de semáforos
int numero_lectores = 0;
int numero_escritores = 0;
int writing = 0;
int num_lectores_bloqueados = 0;
int num_clientes_escritores = 0;
int num_clientes_lectores = 0;

int ratio_exist = 0;
int block_ratio = 0;
int ratio_counter = 0;

//Fichero a escribir
FILE* file;

//Funciones cliente

int client_conection(char* ip, int port)
{
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


//Mirar si el cliente es escritor o lector
int set_client(char *cliente)
{
    int opcion = -1;
    if(strcmp(cliente, "reader") == 0) //Si el argumento es writer será un escritor
    {
        opcion = READ;
    }else if (strcmp(cliente, "writer") == 0)
    {
        opcion = WRITE;
    }else{
        printf("La opción elegida no es la correcta\n");
    }
    return opcion;
}


void *thread_lector(void *arg)
{
    //printf("estamos en thread_lector\n");
    struct request request;
    request.action = READ;

    struct response response;
    int n;
    //Enviamos enviamos al servidor la estructura del mensaje
    if (send(sockfd, &request, sizeof(request), 0) < 0) 
    {
        printf("Send to the server failed...\n");
    }
    //printf("antes del recv \n");
    //Recibimos los datos que nos devulve el servidor
    if ((n = recv(sockfd, &response, sizeof(response), 0)) > 0) 
    {   
        if(response.action == READ)
        {
            
            //print de numero del hilo, contador del .txt y tiempo de respuesta
            printf("Cliente #%d Lector, contador = %d, tiempo = %ld ns.\n", *(int *)arg, response.counter, response.waiting_time);
        }
    } else {
        printf("Recv from the server failed...\n");
    }
}

void *thread_escritor(void *arg)
{
    struct request request;
    request.action = WRITE;
    //Enviamos enviamos al servidor la estructura del mensaje
    
    if (send(sockfd, &request, sizeof(request), 0) < 0)
    {
        printf("Send to the server failed...\n");
    }

    struct response message;
    int n;
    //Recibimos los datos que nos devulve el servidor
    
    if ((n = recv(sockfd, &message, sizeof(message), 0)) > 0)
    {   
        if(message.action == WRITE)
        {
            //print de numero del hilo, contador del .txt y tiempo de respuesta
            printf("Cliente #%d Escritor, contador = %d, tiempo = %ld ns.\n", *(int *)arg, message.counter, message.waiting_time);
        }
    } else {
        printf("Recv from the server failed...\n");
    }
}

void set_reader_or_client(char* threads, int opcion)
{
    struct num_threads{
        int threads;
        int opcion;
    }num_clientes;


    int hilos = atoi(threads);
    pthread_t lectores[hilos];

    //Seleccionamos si el parametro es escritor o lector para luego poner crear el thread en correspondencia
    if (opcion == READ)
    {
        num_clientes.opcion = READ;
    }else if (opcion == WRITE)
    {
        num_clientes.opcion = WRITE;
    }

    num_clientes.threads = hilos;
    
    //printf("numero de clientes %d \n", num_clientes.threads);
    //Lanzo el numero de clientes a procesar
    if (send(sockfd, &num_clientes, sizeof(num_clientes), 0) < 0) 
    {
        printf("Send to the server failed...\n");
    }

    // Generamos un hilo por cada cliente
    int array_thread[hilos];

    for (int i = 0; i < hilos ; i++)
    {
        //Hay que opner en un array las posiciones porque si no el thread no lo coge bn
        array_thread[i] = i; 
    }
    
    for(int i = 0; i < hilos; i++)
    {   
        if (num_clientes.opcion == READ)
        {
            //printf("seguimos en read\n");
            if(pthread_create(&lectores[i], NULL, thread_lector, &array_thread[i] ) != 0)
            {

                printf("Fallo al ejecutar pthread_create de lectores \n");
                exit(1);
            }
        }else if (num_clientes.opcion == WRITE)
        {
            if(pthread_create(&lectores[i], NULL, thread_escritor, &array_thread[i] ) != 0)
            {
                printf("Fallo al ejecutar pthread_create de escritores \n");
                exit(1);
            }
        }
        
    }
    for(int i = 0; i < hilos; i++)
    {
        if(pthread_join(lectores[i], NULL) != 0)
        {
            printf("Fallo al ejecutar pthread_join...\n");
            exit(1);
        }
    }
}


//funciones servidor

// Iniciamos todos los semaforos que vamos a utilizar a continuación
void semaforo()
{
    sem_init(&sem_mutex, 0, 1);
    sem_init(&sem_numero_clientes, 0, 1);

    sem_init(&sem_maximos_lectores, 0, 100);
    sem_init(&sem_escritores_max, 0, 150);
    
    sem_init(&sem_escritores, 0, 0);
    sem_init(&sem_lectores, 0, 0);
    sem_init(&sem_numero_escritores, 0, 1);
    
    sem_init(&sem_numero_lectores, 0, 1);

    sem_init(&sem_cliente_lector, 0, 1);
    sem_init(&sem_cliente_escritor, 0, 1);
    sem_init(&sem_ratio,0,1);
    sem_init(&sem_ratio_counter,0,1);
}


int server_conection(char* ip, int port)
{
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
    

    //Recibe WRITE o READ y el numero de clientes que hay
    if ((n = recv(connfd, &num_clientes, sizeof(num_clientes), 0)) < 0) 
    {
        printf("Recv from the client failed...\n");
    }
    
    //Tenemos si los clientes a tratar son lectores o escritores
    if(num_clientes.opcion == WRITE) 
    {
        sem_wait(&sem_cliente_escritor);
        connfd_writers = connfd;
    }
    else if(num_clientes.opcion == READ) 
    {
        sem_wait(&sem_cliente_lector);
        connfd_readers = connfd;
    }

    return num_clientes.threads;
}

void *escritores_prio_escritor(void *arg)
{
    
    struct response response;
    struct timespec begin, end;

    sem_wait(&sem_numero_escritores);
    num_clientes_escritores++;
    sem_post(&sem_numero_escritores);


    clock_gettime(CLOCK_MONOTONIC, &begin);
    sem_wait(&sem_mutex);
    clock_gettime(CLOCK_MONOTONIC, &end);

    if(numero_lectores > 0 || writing)
    {
        numero_escritores++;
        sem_post(&sem_mutex);
        sem_wait(&sem_escritores);
        numero_escritores--;
    }
    writing = 1;
    sem_post(&sem_mutex);

    //INICIO SECCIÓN CRÍTICA
    servidor.counter++;
    printf("Escritor modifica contador con valor %d\n", servidor.counter);
    file = fopen("server_output.txt", "a");
    char contador[256];
    sprintf(contador,"%d", servidor.counter);
    fputs(contador, file);
    fputs("\n", file);
    fclose(file);

    response.action = WRITE;
    response.counter = servidor.counter;
    response.waiting_time = end.tv_nsec - begin.tv_nsec;

    usleep(50000);

    if (send(connfd_writers, &response, sizeof(response), 0) < 0)
    {
        printf("Send to the server failed...\n");
    }
    //FIN SECCIÓN CRÍTICA

    sem_wait(&sem_mutex);
    writing = 0;
    if(numero_escritores > 0)
    {
        sem_wait(&sem_ratio_counter);
        ratio_counter++;
        if(ratio_exist && ratio_counter % *(int*)arg == 0 && num_clientes_lectores != 0)
        {
            block_ratio = 1;
            printf("holaaaaaaaaaaaaaaa\n\n\n");
            sem_post(&sem_lectores);
            sem_wait(&sem_ratio);
        }

        sem_post(&sem_ratio_counter);
        sem_post(&sem_escritores);
    } 
    else if(num_lectores_bloqueados > 0)
    {
        
        sem_post(&sem_lectores);
    } 
    else 
    {
        sem_post(&sem_mutex);
    } 

    sem_wait(&sem_numero_escritores);
    num_clientes_escritores--;
    if(num_clientes_escritores == 0){
        sem_post(&sem_cliente_escritor);
    }
    sem_post(&sem_numero_escritores);

}

void *lectores_prio_escritor(void *arg)
{
    struct response response;
    struct timespec begin, end;
    
    sem_wait(&sem_numero_lectores);
    num_clientes_lectores++;
    sem_post(&sem_numero_lectores);
    
    sem_wait(&sem_maximos_lectores);
    clock_gettime(CLOCK_MONOTONIC, &begin);
    sem_wait(&sem_mutex);
    clock_gettime(CLOCK_MONOTONIC, &end);
    
    if(numero_escritores || writing)
    {
        num_lectores_bloqueados++;
        sem_post(&sem_mutex);
        
        sem_wait(&sem_lectores);
        
    }
    
    numero_lectores++;
    if(num_lectores_bloqueados > 0)
    {
        sem_post(&sem_lectores);
    }else
    {
        sem_post(&sem_mutex);
    }
    
    //INICIO SECCIÓN CRÍTICA
    response.action = READ;
    response.counter = servidor.counter;
    response.waiting_time = end.tv_nsec - begin.tv_nsec;

    usleep(50000);
    
    if (send(connfd_readers, &response, sizeof(response), 0) < 0)
    {
        printf("Send to the server failed...\n");
    }

    //FIN SECCIÓN CRÍTICA

    if (block_ratio)
    {
        block_ratio = 0;
        sem_post(&sem_ratio);
    }

    sem_wait(&sem_mutex);
    numero_lectores--;
    if(numero_lectores == 0 && numero_escritores == 0)
    {
        sem_post(&sem_escritores);
    }else{
        sem_post(&sem_mutex);
    }


    if(numero_lectores < 150)
    {
        sem_post(&sem_maximos_lectores);
    }

    sem_wait(&sem_numero_lectores);
    num_clientes_lectores--;
    if(num_clientes_lectores == 0){
        sem_post(&sem_cliente_lector);
    }
    sem_post(&sem_numero_lectores);

    
}




void seleccionar_prioridad(int clientes, int ratio, char *prio)
{
    pthread_t escritor;
    pthread_t lector;

    struct request request;
    int n;

    if(ratio != 0)
    {
        ratio_exist = 1;
    }
    //Se ejecuta hasta que se hayan tratado todos los lectores/escritores
    //printf("Seleccionas klk %s?\n\n", prio);
    while(clientes != 0) 
    {

        if (strcmp(prio, "writer") == 0)
        {
            if ((n = recv(connfd, &request, sizeof(request),0)) < 0) //Recibimos la operación a realizar y decidimos que función de thread ejecutamos
            {
                printf("Recv from the client failed...\n");
            } else {
                
                if(request.action == WRITE)
                {
                    pthread_create(&escritor, NULL, escritores_prio_escritor, &ratio);
                } 
                else if(request.action == READ)
                {
                    pthread_create(&lector, NULL, lectores_prio_escritor, NULL); 
                }
            }
        }else if (strcmp(prio, "reader") == 0)
        {
            printf("Entramos en modo mandar mensajes a lectores\n\n\n");
            if ((n = recv(connfd, &request, sizeof(request),0)) < 0) //Recibimos la operación a realizar y decidimos que función de thread ejecutamos
            {
                printf("Recv from the client failed...\n");
            } else {
                if(request.action == READ)
                {
                    
                    pthread_create(&lector, NULL, lectores_prio_lector, &ratio); 
                }else if(request.action == WRITE)
                {
                    pthread_create(&escritor, NULL, escritores_prio_lector, NULL);
                } 
                
            }
        }
        
        clientes--;
    }

    n = 0;
}



void *lectores_prio_lector(void *arg)
{
    
    sem_wait(&sem_numero_lectores);
    num_clientes_lectores++;
    sem_post(&sem_numero_lectores);
    
    struct response response;
    struct timespec begin, end;
    
    sem_wait(&sem_maximos_lectores);
    //printf("Lectores por aqui\n");

    sem_wait(&sem_numero_clientes);
    numero_lectores++;
    ratio_counter++;

    clock_gettime(CLOCK_MONOTONIC, &begin);

    if(numero_lectores == 1)
    {
        sem_wait(&sem_mutex);
    }
    if (ratio_exist && ratio_counter %*(int*) arg == 0 && num_clientes_escritores != 0)
    {
        block_ratio = 1;
        sem_post(&sem_mutex);
        sem_wait(&sem_ratio);
    }
    sem_post(&sem_numero_clientes);
    clock_gettime(CLOCK_MONOTONIC, &end);
    
    //INICIO SECCIÓN CRÍTICA
    response.action = READ;
    response.counter = servidor.counter;
    response.waiting_time = end.tv_nsec - begin.tv_nsec;

    if (send(connfd_readers, &response, sizeof(response), 0) < 0)
    {
        printf("Send to the server failed...\n");
    }

    usleep(50000);
    //FIN SECCIÓN CRÍTICA

    sem_wait(&sem_numero_clientes);
    numero_lectores--;
    if(numero_lectores == 0)
    {
        sem_post(&sem_mutex);
    }
    sem_post(&sem_numero_clientes);
    if(numero_lectores < 100)
    {
        sem_post(&sem_maximos_lectores);
    }

    sem_wait(&sem_numero_lectores);
    num_clientes_lectores--;
    if(num_clientes_lectores == 0){
        sem_post(&sem_cliente_lector);
    }
    sem_post(&sem_numero_lectores);

}


void *escritores_prio_lector(void *arg)
{
    struct response response;
    struct timespec begin, end;

    sem_wait(&sem_numero_escritores);
    num_clientes_escritores++;
    sem_post(&sem_numero_escritores);

    sem_wait(&sem_escritores_max);
    clock_gettime(CLOCK_MONOTONIC, &begin);
    sem_wait(&sem_mutex);
    clock_gettime(CLOCK_MONOTONIC, &end);

    //INICIO SECCIÓN CRÍTICA
    servidor.counter++;
    printf("Escritor modifica contador con valor %d\n", servidor.counter);
    file = fopen("server_output.txt", "a");
    char contador[256];
    sprintf(contador,"%d", servidor.counter);
    fputs(contador, file);
    fputs("\n", file);
    fclose(file);

    response.action = WRITE;
    response.counter = servidor.counter;
    response.waiting_time = end.tv_nsec - begin.tv_nsec;

    usleep(50000);

    if (send(connfd_writers, &response, sizeof(response), 0) < 0)
    {
        printf("Send to the server failed...\n");
    }
    //FIN SECCIÓN CRÍTICA


    if(block_ratio)
    {
        block_ratio = 0;
        sem_post(&sem_ratio);
    }
    sem_post(&sem_mutex);
    sem_post(&sem_escritores_max);

    sem_wait(&sem_numero_escritores);
    num_clientes_escritores--;

    if(num_clientes_escritores == 0){
        sem_post(&sem_cliente_escritor);
    }
    sem_post(&sem_numero_escritores);

}

