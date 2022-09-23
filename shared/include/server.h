#ifndef SERVER_H_
#define SERVER_H_

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <commons/log.h>
#include <pthread.h>
#include "utils.h"
#include "../globals.h"

int conectar_y_mostrar_mensajes_de_cliente(char*, char*, t_log*);
void mostrar_mensajes_del_cliente();
void crear_hilos(int );
void iterator(char* value);


void cambiaValor();

#endif /* SERVER_H_ */
