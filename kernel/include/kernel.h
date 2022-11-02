#ifndef KERNEL_H
#define KERNEL_H
#include <stdlib.h>
#include <stdio.h>
#include <commons/log.h>
#include <stdbool.h>
#include "client.h"
#include "server.h"
#include "comunicacion.h"
#include "tests.h"
#include <string.h>

t_config *config;
int conexion;
int contadorIdPCB;
int conexionMemoria;

t_configKernel configKernel;

t_configKernel extraerDatosConfig(t_config *);

void crear_hilo_consola();
void crear_hilo_cpu();
void conectar_memoria();

void conectar_dispatch();
void conectar_interrupt();
void crear_hilos_kernel();
void crear_pcb2(void* );
char *dispositivoToString(t_IO );
typedef struct
{
    int socketCliente;
    t_informacion informacion;
}t_args_pcb;

int conexionDispatch;
int conexionConsola;

// LISTAS
t_list *LISTA_NEW;
t_list *LISTA_READY;
t_list *LISTA_EXEC;
t_list *LISTA_BLOCKED;
t_list *LISTA_BLOCKED_PANTALLA;
t_list *LISTA_BLOCKED_TECLADO;
t_list *LISTA_EXIT;
t_list *LISTA_SOCKETS;
t_list *LISTA_READY_AUXILIAR;

// MUTEX
pthread_mutex_t mutex_creacion_ID;
pthread_mutex_t mutex_lista_new;
pthread_mutex_t mutex_lista_ready;
pthread_mutex_t mutex_lista_exec;
pthread_mutex_t mutex_lista_blocked;
pthread_mutex_t mutex_lista_blocked_pantalla;
pthread_mutex_t mutex_lista_blocked_teclado;
pthread_mutex_t mutex_lista_exit;


// SEMAFOROS
sem_t sem_planif_largo_plazo;
sem_t contador_multiprogramacion;
sem_t contador_pcb_running;
sem_t sem_ready;
sem_t sem_bloqueo;
sem_t sem_procesador;

sem_t sem_agregar_pcb;
sem_t sem_eliminar_pcb;
sem_t sem_hay_pcb_lista_new;
sem_t sem_hay_pcb_lista_ready;
sem_t sem_pasar_pcb_running;
sem_t sem_timer;
sem_t sem_desalojar_pcb;
sem_t sem_kill_trhread;


pthread_mutex_t mutex_lista_ready_auxiliar;
sem_t sem_llamar_feedback;
#endif