#ifndef UTILS_H_
#define UTILS_H_

#include<stdio.h>
#include<stdlib.h>
#include<signal.h>
#include<unistd.h>
#include<sys/socket.h>
#include<string.h>
#include<commons/log.h>
#include<commons/config.h>
#include<unistd.h>
#include<netdb.h>
#include<commons/collections/list.h>
#include<assert.h>
#include "../globals.h"


typedef enum
{
	MENSAJE,
	PAQUETE
}op_code;

typedef enum
{
	SET, 
	ADD, 
	MOV_IN, 
	MOV_OUT, 
	IO,
    EXIT
} t_instCode;

typedef enum
{
  AX,
  BX,
  CX,
  DX
} t_registro;

//Utils del cliente
typedef struct
{
	int size;
	void* stream;
} t_buffer;

typedef struct
{
	op_code codigo_operacion;
	t_buffer* buffer;
} t_paquete;
/*
typedef struct 
{
    t_instCode instCode;
    uint32_t param[2];
	t_registro registro;
	char* dispositivo;
} t_instruccion;
*/

typedef struct 
{
    t_instCode instCode;
    uint32_t paramInt;
	char* paramChar[1];
} t_instruccion;

typedef struct
{
    uint8_t id;
    //t_list instrucciones;
    uint8_t program_counter;
    uint8_t registro_CPU;
    //t_list segmentos;

} t_pcb;

enum tipo_mensaje{
    PCB
};


int crear_conexion(char* ip, char* puerto);
void enviar_mensaje(char* mensaje, int socket_cliente);
t_paquete* crear_paquete(void);
t_paquete* crear_super_paquete(void);
void agregar_a_paquete(t_paquete* paquete, void* valor, int tamanio);
void enviar_paquete(t_paquete* paquete, int socket_cliente);
void liberar_conexion(int socket_cliente);
void eliminar_paquete(t_paquete* paquete);
t_buffer* cargar_buffer_a_t_pcb(t_pcb t_pcb);
void cargar_buffer_a_paquete(t_buffer* buffer, int conexion);
t_pcb* deserializar_pcb(t_buffer* buffer); 
void deserializar_paquete (int conexion);
//Utils del servidor


extern t_log* logger;

int iniciar_servidor(char* , char*);
int esperar_cliente(int);
t_list* recibir_paquete(int);
void recibir_mensaje(int);
int recibir_operacion(int);
void* recibir_buffer(int*, int);

#endif /* UTILS_H_ */