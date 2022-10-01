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

#define IP_SERVER "0.0.0.0"

int conexionMemoria;
t_log* loggerKernel;
void cambiaValor();


//Planificador largo plazo
t_pcb* crear_pcb();
void pasar_a_new(t_pcb* );
void pasar_a_ready(t_pcb* );
void pasar_a_exec(t_pcb* );
void pasar_a_block(t_pcb* );
void pasar_a_exit(t_pcb* );
void planifLargoPlazo(t_cod_planificador cod_planificador);
void agregar_pcb();
void eliminar_pcb(t_cod_planificador);

// LISTAS
t_list* LISTA_NEW;
t_list* LISTA_READY;
t_list* LISTA_EXEC;
t_list* LISTA_BLOCKED;
t_list* LISTA_BLOCKED_SUSPENDED;
t_list* LISTA_READY_SUSPENDED;
t_list* LISTA_EXIT;
t_list* COLA_BLOQUEO_IO;
t_list* LISTA_SOCKETS;
t_list* instrucciones_x_pcb;

// MUTEX
pthread_mutex_t mutex_creacion_ID;
pthread_mutex_t mutex_lista_new;
pthread_mutex_t mutex_lista_ready;
pthread_mutex_t mutex_lista_exec;
pthread_mutex_t mutex_lista_blocked;
pthread_mutex_t mutex_lista_blocked_suspended;
pthread_mutex_t mutex_lista_ready_suspended;
pthread_mutex_t mutex_lista_exit;
pthread_mutex_t mutex_lista_cola_io;
pthread_mutex_t mutex_lista_instrxpcb;

// SEMAFOROS
sem_t sem_planif_largo_plazo;
sem_t contador_multiprogramacion;
sem_t sem_ready;
sem_t sem_bloqueo;
sem_t sem_procesador;

#endif /* SERVER_H_ */
