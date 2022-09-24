#ifndef CPU_H
#define CPU_H
#include<stdlib.h>
#include <stdio.h>
#include <commons/log.h>
#include <stdbool.h>
#include "client.h"
#include "server.h"
#include "tests.h"

t_config* config;
int conexion;
	
typedef struct {
	int entradasTLB;
	char* reemplazoTLB;
	int retardoInstruccion;
	char* ipMemoria;
	char* puertoMemoria;
	char* puertoEscuchaDispatch;
	char* puertoEscuchaInterrupt;

	char* ipCPU;

}t_configCPU;

t_configCPU configCPU;


	t_configCPU extraerDatosConfig(t_config* );

void iniciar_servidor_dispatch();
void iniciar_servidor_interrupt();


#endif