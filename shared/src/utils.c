#include "utils.h"

// Esto es del cliente

void *serializar_paquete(t_paquete *paquete, int bytes)
{
	void *magic = malloc(bytes);
	int desplazamiento = 0; // esto es el offset que esta en la guia de serializacion

	memcpy(magic + desplazamiento, &(paquete->codigo_operacion), sizeof(int));
	desplazamiento += sizeof(int);
	memcpy(magic + desplazamiento, &(paquete->buffer->size), sizeof(int));
	desplazamiento += sizeof(int);
	memcpy(magic + desplazamiento, paquete->buffer->stream, paquete->buffer->size);
	desplazamiento += paquete->buffer->size;

	return magic;
}

int crear_conexion(char *ip, char *puerto)
{
	struct addrinfo hints;
	struct addrinfo *server_info;

	memset(&hints, 0, sizeof(hints));
	hints.ai_family = AF_INET;
	hints.ai_socktype = SOCK_STREAM;
	hints.ai_flags = AI_PASSIVE;

	// getaddrinfo(): es una llamada al sistema que devuelve la informacion de red sobre la IP y puerto que le pasemos, en este caso el servidor

	getaddrinfo(ip, puerto, &hints, &server_info);

	// Ahora vamos a crear el socket.
	int socket_cliente = socket(server_info->ai_family,
								server_info->ai_socktype,
								server_info->ai_protocol);

	// Ahora que tenemos el socket, vamos a conectarlo
	// connect():
	if (connect(socket_cliente, server_info->ai_addr, server_info->ai_addrlen) == -1)
	{
		printf("error");
	}

	freeaddrinfo(server_info);

	return socket_cliente;
}

void enviar_mensaje(char *mensaje, int socket_cliente)
{

	// esto es lo de crear_paquete()
	t_paquete *paquete = malloc(sizeof(t_paquete));

	paquete->codigo_operacion = MENSAJE;
	paquete->buffer = malloc(sizeof(t_buffer));

	paquete->buffer->size = strlen(mensaje) + 1;
	paquete->buffer->stream = malloc(paquete->buffer->size);
	memcpy(paquete->buffer->stream, mensaje, paquete->buffer->size);

	// todo esto es lo mismo que enviar_paquete()
	int bytes = paquete->buffer->size + 2 * sizeof(int);

	void *a_enviar = serializar_paquete(paquete, bytes);

	send(socket_cliente, a_enviar, bytes, 0);

	free(a_enviar);
	eliminar_paquete(paquete);
}

void crear_buffer(t_paquete *paquete)
{
	paquete->buffer = malloc(sizeof(t_buffer));
	paquete->buffer->size = 0;
	paquete->buffer->stream = NULL;
}

t_paquete *crear_paquete(void)
{
	t_paquete *paquete = malloc(sizeof(t_paquete));
	paquete->codigo_operacion = PAQUETE;
	crear_buffer(paquete);
	return paquete;
}

void agregar_a_paquete(t_paquete *paquete, void *valor, int tamanio)
{
	paquete->buffer->stream = realloc(paquete->buffer->stream, paquete->buffer->size + tamanio + sizeof(int));

	memcpy(paquete->buffer->stream + paquete->buffer->size, &tamanio, sizeof(int));
	memcpy(paquete->buffer->stream + paquete->buffer->size + sizeof(int), valor, tamanio);

	paquete->buffer->size += tamanio + sizeof(int);
}

void enviar_paquete(t_paquete *paquete, int socket_cliente)
{
	int bytes = paquete->buffer->size + 2 * sizeof(int);
	void *a_enviar = serializar_paquete(paquete, bytes);

	send(socket_cliente, a_enviar, bytes, 0);

	free(a_enviar);
}

void eliminar_paquete(t_paquete *paquete)
{
	free(paquete->buffer->stream);
	free(paquete->buffer);
	free(paquete);
}

void liberar_conexion(int socket_cliente)
{
	close(socket_cliente);
}

// Esto es del servidor

t_log *logger;

int iniciar_servidor(char *IP, char *PUERTO)
{

	int socket_servidor;

	struct addrinfo hints, *servinfo, *p;

	memset(&hints, 0, sizeof(hints));
	hints.ai_family = AF_INET;		 /* ai_family = Protocol family for socket.  */
	hints.ai_socktype = SOCK_STREAM; /* ai_socktype =	Socket type.  */
	hints.ai_flags = AI_PASSIVE;	 /*ai_flags= Input flags.  */

	// char* ip = config_get_string_value(archivoConfig, "IP");

	// char* puerto = config_get_string_value(archivoConfig, "PUERTO");

	getaddrinfo(IP, PUERTO, &hints, &servinfo);

	// Asociamos el socket a un puerto
	// bind() lo que hace es tomar el socket que creamos con anterioridad y pegarlo con pegamento industrial al puerto que le digamos

	/* ai_addr: Socket address for socket.  */
	/* ai_addrlen: Length of socket address.  */

	for (p = servinfo; p != NULL; p = p->ai_next)
	{
		if ((socket_servidor = socket(p->ai_family, p->ai_socktype, p->ai_protocol)) == -1)
		{
			printf("connection_error_crear_socket");
			continue;
		}
		int yes = 1;
		setsockopt(socket_servidor, SOL_SOCKET, SO_REUSEADDR, &yes, sizeof yes);
		if (bind(socket_servidor, p->ai_addr, p->ai_addrlen) != 0)
		{
			printf("Error en el bind");
			close(socket_servidor);
			continue;
		}
		break;
	}

	// Escuchamos las conexiones entrantes

	// listen() : toma el socket del servidor creado y lo marca en el sistema como un socket cuya unica responsabilidad es notificar cuando
	// un nuevo cliente esta intentando conectarse

	listen(socket_servidor, SOMAXCONN); // SOMAXCONN: es la cantidad maxima de conexiones vivas que admite el sistema operativo

	freeaddrinfo(servinfo);
	log_trace(logger, "Listo para escuchar a mi cliente");

	return socket_servidor;
}

int esperar_cliente(int socket_servidor)
{
	// Quitar esta línea cuando hayamos terminado de implementar la funcion
	// assert(!"no implementado!");

	// Aceptamos un nuevo cliente
	int socket_cliente;

	// accept() es bloqueante, significa que el proceso servidor se quedara bloqueado en accept hasta que llegue un cliente

	struct sockaddr_in direccion_Cliente;
	socklen_t tam_Direccion = sizeof(struct sockaddr_in);

	socket_cliente = accept(socket_servidor, (void *)&direccion_Cliente, &tam_Direccion);

	// Una vez que el cliente aceptado, accept retorna un nuevo socket(file descriptor) que representa la conexion BIDIRECCIONAL entre el servidor y el cliente
	// La comunicacion entre el cliente y el servidor se hace entre el socket que realizo connect(lado del cliente) hacia el servidor, y el socket que fue devuelto por el accept

	log_info(logger, "Se conecto un cliente!");

	return socket_cliente;
}

int recibir_operacion(int socket_cliente)
{
	int cod_op;
	if (recv(socket_cliente, &cod_op, sizeof(int), MSG_WAITALL) > 0)
		return cod_op;
	else
	{
		close(socket_cliente);
		return -1;
	}
}

void *recibir_buffer(int *size, int socket_cliente)
{
	void *buffer;

	recv(socket_cliente, size, sizeof(int), MSG_WAITALL);
	buffer = malloc(*size);
	recv(socket_cliente, buffer, *size, MSG_WAITALL);

	return buffer;
}

void recibir_mensaje(int socket_cliente)
{
	int size;
	char *buffer = recibir_buffer(&size, socket_cliente);
	log_info(logger, "Me llego el mensaje %s", buffer);
	free(buffer);
}

t_list *recibir_paquete(int socket_cliente)
{
	int size;
	int desplazamiento = 0;
	void *buffer;
	t_list *valores = list_create();
	int tamanio;

	buffer = recibir_buffer(&size, socket_cliente);
	while (desplazamiento < size)
	{
		memcpy(&tamanio, buffer + desplazamiento, sizeof(int));
		desplazamiento += sizeof(int);
		char *valor = malloc(tamanio);
		memcpy(valor, buffer + desplazamiento, tamanio);
		desplazamiento += tamanio;
		list_add(valores, valor);
	}
	free(buffer);
	return valores;
}

t_buffer* cargar_buffer_a_PCB(PCB pcb)
{

	t_buffer *buffer = malloc(sizeof(t_buffer));

	buffer->size = sizeof(uint8_t) * 3		;	
				   //+ (sizeof(t_link_element) + sizeof(int))* list_size(&pcb->instrucciones);
				 //  + (sizeof(t_link_element) + sizeof(int))* list_size(&pcb->segmentos);

	void *stream = malloc(buffer->size);
	int offset = 0; // Desplazamiento

	memcpy(stream + offset, &pcb.id, sizeof(uint8_t));
	offset += sizeof(uint8_t);
	memcpy(stream + offset, &pcb.program_counter, sizeof(uint8_t));
	offset += sizeof(uint8_t);
	memcpy(stream + offset, &pcb.registro_CPU, sizeof(uint8_t));
	//offset += sizeof(uint8_t);


/*	memcpy(stream + offset, &persona.nombre_length, sizeof(uint32_t));
	offset += sizeof(uint32_t);
	memcpy(stream + offset, persona.nombre, strlen(persona.nombre) + 1);
	// No tiene sentido seguir calculando el desplazamiento, ya ocupamos el buffer completo*/
	buffer->stream = stream;
	

	// Si usamos memoria dinámica para el nombre, y no la precisamos más, ya podemos liberarla:
	//free(&pcb);

	printf("termine el buffer");


	return buffer;
}