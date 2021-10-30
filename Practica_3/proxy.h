#define _GNU_SOURCE

#include <stdio.h>
#include <stdlib.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <string.h>
#include <netinet/in.h>
#include <pthread.h>
#include <arpa/inet.h>
#include <unistd.h>
#include <semaphore.h>
#include <time.h>



enum operations {
  WRITE = 0,
  READ
};


struct request{
  enum operations action;
};


struct response{
  enum operations action;
  unsigned int counter;
  long waiting_time;
};

//Miramos si el cliente es un lector o un escritor
int set_client(char* cliente);
//Inicializa el cliente en un ip y con un puerto determinado
int client_conection(char* ip, int port);
//Indica al servidor que lo mandado son clientes
void set_reader_or_client(char* threads, int opcion);
//Thread de lectores
void *thread_lector(void *arg);
//Thread de escritores
void *thread_escritor(void *arg);



//Funciones servidores
void semaforo();
int server_conection(char* ip,int port );

int aceptar_cliente();
void seleccionar_prioridad(int clientes, int ratio, char *prio);
void *escritores_prio_escritor(void *arg);
void *lectores_prio_escritor(void *arg);
void *escritores_prio_lector(void *arg);
void *lectores_prio_lector(void *arg);