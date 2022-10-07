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
#include<semaphore.h>
#include<unistd.h>
#include<netdb.h>
#include<commons/collections/list.h>
#include<assert.h>
#include "../globals.h"


typedef enum
{
	MENSAJE,
	PAQUETE,
	NEW,
	PROGRAMA
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
  DX,
  DESCONOCIDO
} t_registro;

typedef enum
{
  DISCO,
  TECLADO,
  PANTALLA
} t_IO;


//Utils del cliente
typedef struct
{
	uint32_t size; // Tamaño del payload
	void* stream;  // Payload
} t_buffer;

typedef struct
{
	op_code codigo_operacion;
	t_buffer* buffer;
} t_paquete;



typedef struct 
{
    t_instCode instCode;
    uint32_t paramInt;
	t_IO paramIO;
	t_registro paramReg[2]; 
} __attribute__((packed)) t_instruccion; 


typedef struct 
{	
	t_list* instrucciones;
	uint32_t instrucciones_size;
	t_list* segmentos;
	uint32_t segmentos_size;
} __attribute__((packed)) t_informacion;

typedef struct 

{	uint32_t AX;
    uint32_t BX;
    uint32_t CX;
    uint32_t DX;

}  __attribute__((packed)) t_registros;


int size_char_array(char**) ;


extern int conexionMemoria;


typedef struct
{
    uint32_t id;
	//uint32_t tamanio;
    uint32_t program_counter;
	//uint32_t tablaPag; // definir con memoria
	//double estimacion_actual;
	//double real_anterior;
	//double ejecutados_total;
    t_informacion informacion;
	t_registros registros;
	int socket;

} t_pcb;

enum tipo_mensaje{
    PCB
};

typedef enum
{
	AGREGAR_PCB,
	ELIMINAR_PCB
}t_cod_planificador;

extern int contadorIdPCB;
typedef enum {
	INSTRUCCIONES,    				//entre consola-kernel
	DISPATCH_PCB,     				//entre kernel-cpu
	BLOCK_PCB,						//entre kernel-cpu
	INTERRUPT_INTERRUPCION,			//entre kernel-cpu
	EXIT_PCB,						//entre kernel-cpu
	PASAR_A_READY,					//entre kernel-memoria
	SUSPENDER,						//entre kernel-memoria
	PASAR_A_EXIT,					//entre kernel-memoria
	CONFIG_DIR_LOG_A_FISICA,   	 	//entre cpu-memoria: ESTO ES PARA PASARLE LA CONFIGURACION DE LAS DIRECCIONES, ES EN EL INIT DE LA CPU
	TRADUCCION_DIR_PRIMER_PASO,		//entre cpu-memoria
	TRADUCCION_DIR_SEGUNDO_PASO,	//entre cpu-memoria
	ACCESO_MEMORIA_READ,			//entre cpu-memoria
	ACCESO_MEMORIA_WRITE,			//entre cpu-memoria
	ACCESO_MEMORIA_COPY,			//entre cpu-memoria
	HANDSHAKE_INICIAL,
}t_tipoMensaje;

/*

 ██████╗██╗     ██╗███████╗███╗   ██╗████████╗███████╗
██╔════╝██║     ██║██╔════╝████╗  ██║╚══██╔══╝██╔════╝
██║     ██║     ██║█████╗  ██╔██╗ ██║   ██║   █████╗  
██║     ██║     ██║██╔══╝  ██║╚██╗██║   ██║   ██╔══╝  
╚██████╗███████╗██║███████╗██║ ╚████║   ██║   ███████╗
 ╚═════╝╚══════╝╚═╝╚══════╝╚═╝  ╚═══╝   ╚═╝   ╚══════╝
                                                      
*/

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

void serializarPCB(int socket, t_pcb* pcb, t_tipoMensaje tipoMensaje);
void crearPaquete(t_buffer* buffer, t_tipoMensaje op, int unSocket);
t_paquete* recibirPaquete(int socket);
t_pcb* deserializoPCB(t_buffer* buffer);
/*
███████╗███████╗██████╗ ██╗   ██╗██╗██████╗  ██████╗ ██████╗ 
██╔════╝██╔════╝██╔══██╗██║   ██║██║██╔══██╗██╔═══██╗██╔══██╗
███████╗█████╗  ██████╔╝██║   ██║██║██║  ██║██║   ██║██████╔╝
╚════██║██╔══╝  ██╔══██╗╚██╗ ██╔╝██║██║  ██║██║   ██║██╔══██╗
███████║███████╗██║  ██║ ╚████╔╝ ██║██████╔╝╚██████╔╝██║  ██║
╚══════╝╚══════╝╚═╝  ╚═╝  ╚═══╝  ╚═╝╚═════╝  ╚═════╝ ╚═╝  ╚═╝
                                                             
*/

int iniciar_servidor(char* , char*);

int esperar_cliente(int);

t_list* recibir_paquete(int);

void recibir_mensaje(int);

int recibir_operacion(int);

void* recibir_buffer(int*, int);



/*

██████╗ ██╗      █████╗ ███╗   ██╗██╗███████╗██╗ ██████╗ █████╗  ██████╗██╗ ██████╗ ███╗   ██╗
██╔══██╗██║     ██╔══██╗████╗  ██║██║██╔════╝██║██╔════╝██╔══██╗██╔════╝██║██╔═══██╗████╗  ██║
██████╔╝██║     ███████║██╔██╗ ██║██║█████╗  ██║██║     ███████║██║     ██║██║   ██║██╔██╗ ██║
██╔═══╝ ██║     ██╔══██║██║╚██╗██║██║██╔══╝  ██║██║     ██╔══██║██║     ██║██║   ██║██║╚██╗██║
██║     ███████╗██║  ██║██║ ╚████║██║██║     ██║╚██████╗██║  ██║╚██████╗██║╚██████╔╝██║ ╚████║
╚═╝     ╚══════╝╚═╝  ╚═╝╚═╝  ╚═══╝╚═╝╚═╝     ╚═╝ ╚═════╝╚═╝  ╚═╝ ╚═════╝╚═╝ ╚═════╝ ╚═╝  ╚═══╝
                                                                                              

*/

extern t_log* logger;
extern t_cod_planificador* cod_planificador;
//extern t_log* loggerKernel;

t_pcb* crear_pcb();

// LISTAS
extern t_list* LISTA_NEW;
extern t_list* LISTA_READY;
extern t_list* LISTA_EXEC;
extern t_list* LISTA_BLOCKED;
extern t_list* LISTA_EXIT;
extern t_list* LISTA_SOCKETS;


// MUTEX
extern pthread_mutex_t mutex_creacion_ID;
extern pthread_mutex_t mutex_lista_new;
extern pthread_mutex_t mutex_lista_ready;
extern pthread_mutex_t mutex_lista_exec;
extern pthread_mutex_t mutex_lista_blocked;
extern pthread_mutex_t mutex_lista_exit;


// SEMAFOROS
extern sem_t sem_planif_largo_plazo;
extern sem_t contador_multiprogramacion;
extern sem_t sem_ready;
extern sem_t sem_bloqueo;
extern sem_t sem_procesador;

#endif /* UTILS_H_ */