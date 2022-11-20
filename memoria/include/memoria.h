#ifndef MEMORIA_H
#define MEMORIA_H
#include<stdlib.h>
#include <stdio.h>
#include <commons/log.h>
#include <stdbool.h>
#include<string.h>
#include "client.h"
#include "server.h"
#include "comunicacion.h"

t_config* config;
	typedef struct {
	char* puertoEscuchaUno;
	char* puertoEscuchaDos;
	int tamMemoria;
	int tamPagina;
	int entradasPorTabla;
	char* retardoMemoria;
	char* algoritmoReemplazo;
	int marcosPorProceso;
    int retardoSwap;
    char* pathSwap;
    int tamanioSwap;

}t_configMemoria;


typedef struct {
	uint16_t idTablaPag;
	t_list* paginas;
	//agregar PID para hacer la busqueda y saber a que proceso pertenece 
} __attribute__((packed)) t_tabla_paginas;

typedef struct {
	int nroPagina;
	int nroMarco;
	uint8_t presencia;// 0 v 1
	uint8_t modificacion;// 0 v 1 
	uint8_t uso; // 0 v 1
	uint32_t posicionSwap;
} __attribute__((packed)) t_pagina;

void* espacioContiguiMem; // espacio que en el que voy a guardar bytes, escribir y leer como hago en el archivo (RAM)

t_configMemoria configMemoria;

t_configKernel configKernel;
int contadorIdTablaPag;

void iniciar_servidor_hacia_kernel();
void iniciar_servidor_hacia_cpu();
void agregar_tabla_paginas(t_tabla_paginas *);

t_configMemoria extraerDatosConfig(t_config* );
void crearTablasPaginas(void *pcb);
void eliminarTablasPaginas(void *pcb);
FILE *abrirArchivo(char *filename);
void crear_hilos_memoria();

int contadorIdPCB;
int socketAceptadoKernel;

int conexionMemoria;
int conexionDispatch;
int conexionConsola;
int conexionInterrupt;

//el segmento esta en utils


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
t_list *LISTA_BLOCKED_DISCO;
t_list *LISTA_BLOCKED_IMPRESORA;
t_list *LISTA_TABLA_PAGINAS;
t_list *LISTA_BLOCK_PAGE_FAULT;

// MUTEX
pthread_mutex_t mutex_creacion_ID;
pthread_mutex_t mutex_ID_Segmnento;
pthread_mutex_t mutex_lista_new;
pthread_mutex_t mutex_lista_ready;
pthread_mutex_t mutex_lista_exec;
pthread_mutex_t mutex_lista_blocked_disco;
pthread_mutex_t mutex_lista_blocked_impresora;
pthread_mutex_t mutex_lista_blocked_pantalla;
pthread_mutex_t mutex_lista_blocked_teclado;
pthread_mutex_t mutex_lista_exit;
pthread_mutex_t mutex_creacion_ID_tabla;
pthread_mutex_t mutex_lista_tabla_paginas;
pthread_mutex_t mutex_lista_block_page_fault; 

// SEMAFOROS
sem_t sem_planif_largo_plazo;
sem_t contador_multiprogramacion;
sem_t contador_pcb_running;
sem_t contador_bloqueo_teclado_running;
sem_t contador_bloqueo_pantalla_running;
sem_t contador_bloqueo_disco_running;
sem_t contador_bloqueo_impresora_running;
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


bool hayTimer;
#endif