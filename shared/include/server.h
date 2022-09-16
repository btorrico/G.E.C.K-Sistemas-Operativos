#ifndef SERVER_H_
#define SERVER_H_

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <commons/log.h>
#include "utils.h"

void conectar_y_mostrar_mensajes_de_cliente(t_log* logger);
void iterator(char* value);

#endif /* SERVER_H_ */
