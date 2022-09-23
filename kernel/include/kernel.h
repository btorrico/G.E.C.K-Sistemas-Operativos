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


	char* ip_consola;
	char* puerto_escucha;
	t_config* config;
	int conexion;
	
    void leerConfig(char*);
#endif