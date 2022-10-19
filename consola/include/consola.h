#ifndef CONSOLA_H
#define CONSOLA_H
#include <stdlib.h>
#include <stdio.h>
#include <commons/log.h>
#include <stdbool.h>
#include <signal.h>
#include <sys/socket.h>
#include <string.h>
#include <unistd.h>
#include <netdb.h>
#include <commons/collections/list.h>
#include <assert.h>
#include "client.h"
#include "server.h"
#include "tests.h"
#include "instrucciones.h"

	// rutaArchivoConfiguracion = ;
	// rutaInstrucciones = "./pseudocodigo/pseudocodigo";


int conexion;


int contadorIdPCB;
int conexionMemoria;

t_config *config;



char *rutaArchivoConfiguracion;

char *rutaInstrucciones;

typedef struct
{
	char *ipKernel;
	char *puertoKernel;
	char **segmentos;
	
} t_configConsola;

t_configConsola configConsola;

t_configKernel configKernel;

/*******Funcion que permite leer la configuracion del puerrto y la ip del kernel*******/
t_configConsola extraerDatosConfig(t_config * ruta);


/*******Funcion que recibe y valida los argumentos que se ingresan cuando se inicia el modulo *******/
void obtenerArgumentos(int, char **);

FILE *abrirArchivo(char *);


/**Funcion que crea la estructura que sera enviada**/
t_informacion* crearInformacion(); 

t_paquete* crear_paquete_programa(t_informacion* informacion);

void liberar_programa(t_informacion* informacion);

t_list* listaSegmentos();

char *recibirMensaje(int socket);
void *recibirStream(int socket, size_t stream_size);

/*

██████╗ 
╚════██╗
  ▄███╔╝
  ▀▀══╝ 
  ██╗   
  ╚═╝   
        
*/


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
sem_t contador_pcb_running;
sem_t sem_ready;
sem_t sem_bloqueo;
sem_t sem_procesador;

sem_t sem_agregar_pcb;
sem_t sem_hay_pcb_lista_new;
sem_t sem_hay_pcb_lista_ready;
sem_t sem_pasar_pcb_running;
#endif