#ifndef KERNEL_H
#define KERNEL_H
#include <stdlib.h>
#include <stdio.h>
#include <commons/log.h>
#include <stdbool.h>
#include "client.h"
#include "server.h"
#include "tests.h"
#include <string.h>

t_config *config;
int conexion;

typedef struct
{

	char *ipMemoria;
	char *puertoMemoria;
	char *ipCPU;
	char *puertoCPUDispatch;
	char *puertoCPUInterrupt;
	char *ipKernel;
	char *puertoEscucha;
	char *algoritmo;

	int gradoMultiprogramacion;
	t_list *dispositivosIO;
	t_list *tiemposIO;
	int quantum;
} t_configKernel;

t_configKernel configKernel;

t_configKernel extraerDatosConfig(t_config *);

void crear_hilo_consola();
void crear_hilo_cpu();
void conectar_memoria();

void conectar_dispatch();
void conectar_interrupt();

t_cod_planificador* cod_planificador ;

// LISTAS
t_list *LISTA_NEW;
t_list *LISTA_READY;
t_list *LISTA_EXEC;
t_list *LISTA_BLOCKED;
t_list *LISTA_EXIT;
t_list *LISTA_SOCKETS;

// MUTEX
pthread_mutex_t mutex_creacion_ID;
pthread_mutex_t mutex_lista_new;
pthread_mutex_t mutex_lista_ready;
pthread_mutex_t mutex_lista_exec;
pthread_mutex_t mutex_lista_blocked;
pthread_mutex_t mutex_lista_exit;

// SEMAFOROS
sem_t sem_planif_largo_plazo;
sem_t contador_multiprogramacion;
sem_t sem_ready;
sem_t sem_bloqueo;
sem_t sem_procesador;
#endif