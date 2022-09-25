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

	// rutaArchivoConfiguracion = ;
	// rutaInstrucciones = "./pseudocodigo/pseudocodigo";

t_config *config;
int conexion;
char *rutaArchivoConfiguracion ;
char *rutaInstrucciones;

typedef struct
{
	char *ipKernel;
	char *puertoKernel;
	t_list *segmentos;
	
} t_configConsola;

t_configConsola configConsola;

/*******Funcion que permite leer la configuracion del puerrto y la ip del kernel*******/
t_configConsola extraerDatosConfig(t_config * ruta);

/*******Funcion que recibe y valida los argumentos que se ingresan cuando se inicia el modulo *******/
void obtenerArgumentos(int, char **);

FILE *abrirArchivo(char *);

void agregarInstruccionesDesdeArchivo(FILE *instructionsFile, t_list *instrucciones);

#endif