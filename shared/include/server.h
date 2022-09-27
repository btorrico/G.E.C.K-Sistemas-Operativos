#ifndef SERVER_H_
#define SERVER_H_

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <commons/log.h>
#include <pthread.h>
#include "utils.h"
#include "../globals.h"


void conectar_y_mostrar_mensajes_de_cliente(char*, char*, t_log*);
void mostrar_mensajes_del_cliente(int);
void mostrar_mensajes_del_cliente_dos(int );
int crear_hilos(int );
void iterator(char* value);

#define IP_SERVER "0.0.0.0"


void cambiaValor();

#endif /* SERVER_H_ */
