#ifndef SERVER_H_
#define SERVER_H_

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <commons/log.h>
#include <pthread.h>
#include "utils.h"
#include "../globals.h"


void conectar_y_mostrar_mensajes_de_cliente(char*, char*, t_log*);
void mostrar_mensajes_del_cliente(int);
int crear_hilos(int );
void iterator(char* value);
void iteratorInt(int value);

#define IP_SERVER "0.0.0.0"

void cambiaValor();
void planifLargoPlazo(t_cod_planificador*);

void recibir_informacion(int cliente_fd);

// //t_cod_planificador* cod_planificador ;

// // LISTAS
// t_list *LISTA_NEW;
// t_list *LISTA_READY;
// t_list *LISTA_EXEC;
// t_list *LISTA_BLOCKED;
// t_list *LISTA_EXIT;
// t_list *LISTA_SOCKETS;

// // MUTEX
// pthread_mutex_t mutex_creacion_ID;
// pthread_mutex_t mutex_lista_new;
// pthread_mutex_t mutex_lista_ready;
// pthread_mutex_t mutex_lista_exec;
// pthread_mutex_t mutex_lista_blocked;
// pthread_mutex_t mutex_lista_exit;

// // SEMAFOROS
// sem_t sem_planif_largo_plazo;
// sem_t contador_multiprogramacion;
// sem_t sem_ready;
// sem_t sem_bloqueo;
// sem_t sem_procesador;

#endif /* SERVER_H_ */
