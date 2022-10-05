#include "comunicacion.h"

void enviarResultado(int socket, char* mensajeAEnviar){
	
		enviarMensaje(socket,mensajeAEnviar);
}

int enviarMensaje(int socket, char *msj)
{
	size_t size_stream;

	void *stream = serializarMensaje(msj, &size_stream);


	return enviarStream(socket, stream, size_stream);
}


int enviarStream(int socket, void *stream, size_t stream_size)
{

	if (send(socket, stream, stream_size, 0) == -1)
	{
		free(stream);
		return ERROR;
	}

	free(stream);
	return SUCCESS;
}

void *serializarMensaje(char *msj, size_t *size_stream)
{

	*size_stream = strlen(msj) + 1;

	void *stream = malloc(sizeof(*size_stream) + *size_stream);

	memcpy(stream, size_stream, sizeof(*size_stream));
	memcpy(stream + sizeof(*size_stream), msj, *size_stream);

	*size_stream += sizeof(*size_stream);

	return stream;
}


char *recibirMensaje(int socket)
{
	size_t *tamanio_mensaje;
	char *msj;

	tamanio_mensaje = recibirStream(socket, sizeof(*tamanio_mensaje));

	if (tamanio_mensaje)
	{

		if ((msj = recibirStream(socket, *tamanio_mensaje)))
		{
			free(tamanio_mensaje);
			return msj;
		}

		free(tamanio_mensaje);
	}

	return NULL;
}

void *recibirStream(int socket, size_t stream_size)
{
	void *stream = malloc(stream_size);

	if (recv(socket, stream, stream_size, 0) == -1)
	{
		free(stream);
		stream = NULL;
		exit(-1);
	}

	return stream;
}