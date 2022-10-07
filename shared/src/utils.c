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
// t_log* loggerKernel;

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

int size_char_array(char **array)
{
	int i = 0;

	while (array[i] != NULL)
	{
		i++;
	}
	return i;
}

/*
t_buffer *cargar_buffer_a_t_pcb(t_pcb t_pcb)
{

	t_buffer *buffer = malloc(sizeof(t_buffer));

	buffer->size = sizeof(uint8_t) * 3;
	//+ (sizeof(t_link_element) + sizeof(int))* list_size(&t_pcb->instrucciones);
	//  + (sizeof(t_link_element) + sizeof(int))* list_size(&t_pcb->segmentos);

	void *stream = malloc(buffer->size);
	int offset = 0; // Desplazamiento

	memcpy(stream + offset, &t_pcb.id, sizeof(uint8_t));
	offset += sizeof(uint8_t);
	memcpy(stream + offset, &t_pcb.program_counter, sizeof(uint8_t));
	offset += sizeof(uint8_t);
	memcpy(stream + offset, &t_pcb.registro_CPU, sizeof(uint8_t));
	// offset += sizeof(uint8_t);
	// agregar listras

	buffer->stream = stream;

	return buffer;
}

void cargar_buffer_a_paquete(t_buffer *buffer, int conexion)
{
	t_paquete *paquete = malloc(sizeof(t_paquete));

	paquete->codigo_operacion = PCB; // Podemos usar una constante por operación
	paquete->buffer = buffer;			 // Nuestro buffer de antes.

	// Armamos el stream a enviar
	void *a_enviar = malloc(buffer->size + sizeof(uint8_t));
	int offset = 0;

	memcpy(a_enviar + offset, &(paquete->codigo_operacion), sizeof(uint8_t));
	offset += sizeof(uint8_t);
	memcpy(a_enviar + offset, &(paquete->buffer->size), sizeof(uint32_t));
	offset += sizeof(uint32_t);
	memcpy(a_enviar + offset, paquete->buffer->stream, paquete->buffer->size);

	// Por último enviamos
	send(conexion, a_enviar, buffer->size + sizeof(uint8_t), 0);

	// No nos olvidamos de liberar la memoria que ya no usaremos
	free(a_enviar);
	free(paquete->buffer->stream);
	free(paquete->buffer);
	free(paquete);
}

void deserializar_paquete (int conexion)
{
	t_paquete *paquete = malloc(sizeof(t_paquete));
	paquete->buffer = malloc(sizeof(t_buffer));

	// Primero recibimos el codigo de operacion
	recv(conexion, &(paquete->codigo_operacion), sizeof(uint8_t), 0);

	// Después ya podemos recibir el buffer. Primero su tamaño seguido del contenido
	recv(conexion, &(paquete->buffer->size), sizeof(uint32_t), 0);
	paquete->buffer->stream = malloc(paquete->buffer->size);
	recv(conexion, paquete->buffer->stream, paquete->buffer->size, 0);


printf("estoy en seserializar_paquete");
	// Ahora en función del código recibido procedemos a deserializar el resto
	switch (paquete->codigo_operacion)
	{
	case PCB:
		t_pcb* pcb = deserializar_pcb(paquete->buffer);
		printf("tengo el paquete ");
			// Hacemos lo que necesitemos con esta info
			// Y eventualmente liberamos memoria
			free(pcb);
		break;
		// Evaluamos los demás casos según corresponda
	}

	// Liberamos memoria
	free(paquete->buffer->stream);
	free(paquete->buffer);
	free(paquete);
}

t_pcb* deserializar_pcb(t_buffer* buffer) {
	t_pcb* pcb = malloc(sizeof(t_pcb));

	void* stream = buffer->stream;
	// Deserializamos los campos que tenemos en el buffer
	memcpy(&(pcb->id), stream, sizeof(uint8_t));
	stream += sizeof(uint8_t);
	memcpy(&(pcb->program_counter), stream, sizeof(uint8_t));
	stream += sizeof(uint8_t);
	memcpy(&(pcb->registro_CPU), stream, sizeof(uint8_t));

	return pcb;
}
*/

// Serializar
void serializarPCB(int socket, t_pcb *pcb, t_tipoMensaje tipoMensaje)
{
	t_buffer *buffer = malloc(sizeof(t_buffer));
	buffer->size = sizeof(uint32_t) * 4
				   + list_size(pcb->informacion.instrucciones) * sizeof(t_instruccion) 
				   + list_size(pcb->informacion.segmentos) * sizeof(char *)
				   + sizeof(int)
				   + sizeof(t_registros);
	

	void *stream = malloc(buffer->size);
	int offset = 0;

	memcpy(stream + offset, &pcb->id, sizeof(uint32_t));
	offset += sizeof(uint32_t);

	memcpy(stream + offset, &pcb->program_counter, sizeof(uint32_t));
	offset += sizeof(uint32_t);

	memcpy(stream + offset, &pcb->socket , sizeof(int));
	offset += sizeof(int);

	memcpy(stream + offset, &pcb->registros , sizeof(t_registros));
	offset += sizeof(t_registros);

	memcpy(stream + offset, &(pcb->informacion.instrucciones->elements_count), sizeof(uint32_t));
	offset += sizeof(uint32_t);

	memcpy(stream + offset, &(pcb->informacion.segmentos->elements_count), sizeof(uint32_t));
	offset += sizeof(uint32_t);


	int i = 0, j = 0;
	while (i < list_size(pcb->informacion.instrucciones))
	{
		memcpy(stream + offset, list_get(pcb->informacion.instrucciones, i), sizeof(t_instruccion));
		offset += sizeof(t_instruccion);
		i++;
		printf(PRINT_COLOR_MAGENTA "Estoy serializando las instruccion %d" PRINT_COLOR_RESET "\n", i);
	}
	

	while (j < list_size(pcb->informacion.segmentos))
	{

		memcpy(stream + offset, list_get(pcb->informacion.segmentos, j), sizeof(char *));
		offset += sizeof(char *);
		j++;
		printf(PRINT_COLOR_YELLOW "Estoy serializando el segmento: %d" PRINT_COLOR_RESET "\n", j);
	}

	buffer->stream = stream;

	crearPaquete(buffer, tipoMensaje, socket);
}

void crearPaquete(t_buffer *buffer, t_tipoMensaje op, int unSocket)
{
	t_paquete *paquete = malloc(sizeof(t_paquete));
	paquete->codigo_operacion = (uint8_t)op;
	paquete->buffer = buffer;

	void *a_enviar = malloc(buffer->size + sizeof(uint8_t) + sizeof(uint32_t));
	int offset = 0;

	memcpy(a_enviar + offset, &(paquete->codigo_operacion), sizeof(uint8_t));
	offset += sizeof(uint8_t);
	memcpy(a_enviar + offset, &(paquete->buffer->size), sizeof(uint32_t));
	offset += sizeof(uint32_t);
	memcpy(a_enviar + offset, paquete->buffer->stream, paquete->buffer->size);

	send(unSocket, a_enviar, buffer->size + sizeof(uint8_t) + sizeof(uint32_t), 0);

	free(a_enviar);
	free(paquete->buffer->stream);
	free(paquete->buffer);
	free(paquete);
}

// Deserializar
t_paquete *recibirPaquete(int socket)
{
	t_paquete *paquete = malloc(sizeof(t_paquete));
	paquete->buffer = malloc(sizeof(t_buffer));

	// Primero recibimos el codigo de operacion
	int rec = recv(socket, &(paquete->codigo_operacion), sizeof(uint8_t), MSG_WAITALL);
	if (rec <= 0)
	{
		return NULL;
	}

	// Después ya podemos recibir el buffer. Primero su tamaño seguido del contenido
	recv(socket, &(paquete->buffer->size), sizeof(uint32_t), MSG_WAITALL);
	paquete->buffer->stream = malloc(paquete->buffer->size);
	recv(socket, paquete->buffer->stream, paquete->buffer->size, MSG_WAITALL);

	return paquete;
}

t_pcb *deserializoPCB(t_buffer *buffer)
{
	t_pcb *pcb = malloc(sizeof(t_pcb));

	void *stream = buffer->stream;

	// Deserializamos los campos que tenemos en el buffer
	memcpy(&(pcb->id), stream, sizeof(uint32_t));
	stream += sizeof(uint32_t);

	memcpy(&(pcb->program_counter), stream, sizeof(uint32_t));
    stream += sizeof(uint32_t);

	memcpy(&(pcb->socket), stream, sizeof(int));
    stream += sizeof(int);

    memcpy(&(pcb->registros), stream, sizeof(t_registros));
    stream += sizeof(t_registros);

	memcpy(&(pcb->informacion.instrucciones_size), stream, sizeof(uint32_t));
	stream += sizeof(uint32_t);
	
    memcpy(&(pcb->informacion.segmentos_size), stream, sizeof(uint32_t));
	stream += sizeof(uint32_t);

	pcb->informacion.instrucciones = list_create();
	t_instruccion *instruccion;

	pcb->informacion.segmentos = list_create();
	char *segmento;

	int k = 0 , l = 0;

	while (k < (pcb->informacion.instrucciones_size))
	{
		instruccion = malloc(sizeof(t_instruccion));
		memcpy(instruccion, stream, sizeof(t_instruccion));
		stream += sizeof(t_instruccion);
		list_add(pcb->informacion.instrucciones, instruccion);
		k++;
	}

	while (l < (pcb->informacion.segmentos_size))
	{
		segmento = malloc(sizeof(char *));
		memcpy(segmento, stream, sizeof(char *));
		stream+= sizeof(char *);
		list_add(pcb->informacion.segmentos, segmento);
		l++;
	}

	return pcb;
}