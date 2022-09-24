#ifndef MEMORIA_H
#define MEMORIA_H
#include<stdlib.h>
#include <stdio.h>
#include <commons/log.h>
#include <stdbool.h>
#include<string.h>
#include "client.h"
#include "server.h"
#include "tests.h"

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

t_configMemoria configMemoria;


void iniciar_servidor_hacia_kernel();
void iniciar_servidor_hacia_cpu();

t_configMemoria extraerDatosConfig(t_config* );

#endif