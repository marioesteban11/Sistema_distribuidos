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
#include <math.h>
#include <signal.h>



////////////////////////////////////////////////////////////////////
// Estructuras usadas por el publicador y subscrictor
enum operations {
    REGISTER_PUBLISHER = 0,
    UNREGISTER_PUBLISHER,
    REGISTER_SUBSCRIBER,
    UNREGISTER_SUBSCRIBER,
    PUBLISH_DATA
};
struct publish {
    struct timespec time_generated_data;
    char data[100];
};
struct message {
    enum operations action;
    char topic [100];
    // Solo utilizado en mensajes de UNREGISTER
    int id;
    // Solo utilizado en mensajes PUBLISH_DATA
    struct publish data;
};
////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////
//Estructuras usadas por el broker
enum status {
    OK = 0,
    LIMIT,
    ERROR
};
struct response {
    enum status response_status;
    int id;
};
//////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////


int aceptar_cliente(char *mode);
int server_conection(int port);
int close_server();
int conexiones_publicadores(struct message someone_to_broker);
int conexiones_suscriptores(int buen_connfd, struct message someone_to_broker);
int desconexion_publicador(struct message someone_to_broker);
int desconexion_suscriptor(int buen_connfd, struct message someone_to_broker);
int publicar_datos(struct message someone_to_broker);


void semaforo();

int client_conection(char* ip, int port, int tipo);
int topic_conection(char *topic);
int remove_topic(char *topic);
int send_message(char *topic);
int close_client();

int topic_suscription(char *topic);
int get_message(char *topic);
int unfollow_topic(char *topic, int id_actual);