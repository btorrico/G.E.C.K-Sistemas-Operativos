#ifndef CONSOLA_H
#define CONSOLA_H
#include<stdlib.h>
#include<stdio.h>
#include<commons/log.h>
#include<stdbool.h>
#include<signal.h>
#include<sys/socket.h>
#include<string.h>
#include<unistd.h>
#include<netdb.h>
#include<commons/collections/list.h>
#include<assert.h>
#include "client.h"
#include "server.h"
#include "tests.h"

	t_config* config;
	int conexion;

	typedef struct {
	
	char* ipKernel;
	char* puertoKernel;

	t_list* segmentos;
}t_configConsola;

t_configConsola configConsola;
t_configConsola extraerDatosConfig(t_config*);


#endif