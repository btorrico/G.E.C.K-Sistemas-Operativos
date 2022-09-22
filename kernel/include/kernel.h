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

#define IP "127.0.0.1"
#define PUERTO "4445"

char* ip;
	char* puerto;
	char* valor;
t_config* config;
int conexion;
	
int conectar_y_mostrar_mensajes_de_cliente(char*, char*);
    void leerConfig(char*);
#endif