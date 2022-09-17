#ifndef CPU_H
#define CPU_H
#include<stdlib.h>
#include <stdio.h>
#include <commons/log.h>
#include <stdbool.h>
#include "client.h"
#include "server.h"
#include "tests.h"


#define IP "127.0.0.1"
#define PUERTO "4445"


char* ip;
	char* puerto;
	char* valor;
t_config* config;
int conexion;
	

    void leerConfig(char*);

#endif