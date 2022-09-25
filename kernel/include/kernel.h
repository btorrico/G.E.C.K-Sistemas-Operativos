#ifndef KERNEL_H
#define KERNEL_H
#include<stdlib.h>
#include <stdio.h>
#include <commons/log.h>
#include <stdbool.h>
#include "client.h"
#include "server.h"
#include "tests.h"
#include<string.h>


	t_config* config;
	int conexion;

	typedef struct {
	
	char* ipMemoria;
	char* puertoMemoria;
	char* ipCPU;
	char* puertoCPUDispatch;
	char* puertoCPUInterrupt;
	char* ipKernel;
	char* puertoEscucha;
	char* algoritmo;

	int gradoMultiprogramacion;
	t_list* dispositivosIO;
	t_list* tiemposIO;
	int quantum;
}t_configKernel;

t_configKernel configKernel;


	t_configKernel extraerDatosConfig(t_config* );
    

void crear_hilo_consola();
void crear_hilo_cpu();
void conectar_memoria();

	void conectar_dispatch();
	void conectar_interrupt();
#endif