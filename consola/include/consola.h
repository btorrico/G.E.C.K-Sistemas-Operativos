#ifndef CONSOLA_H
#define CONSOLA_H
#include<stdlib.h>
#include <stdio.h>
#include <commons/log.h>
#include <stdbool.h>
#include "client.h"
#include "server.h"
#include "tests.h"
#include<signal.h>
#include<sys/socket.h>
#include<string.h>
#include<unistd.h>
#include<netdb.h>
#include<commons/collections/list.h>
#include<assert.h>


	int conexion;
	char* ip;
	char* puerto;
	char* valor;
	t_config* config;

    void leerConfig();

#endif