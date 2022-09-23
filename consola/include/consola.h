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


	char* ip_kernel;
	char* puerto_kernel;
	char* buffer;
	t_list* segmentos;
	t_config* config;
	int conexion;


void leerConfig(char*);
#endif