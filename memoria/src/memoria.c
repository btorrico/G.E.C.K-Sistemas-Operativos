#include "memoria.h"

int main(int argc, char **argv)
{

	// Parte Server
	logger = iniciar_logger("memoria.log", "MEMORIA", LOG_LEVEL_DEBUG);

	config = iniciar_config("memoria.config");

	// creo el struct
	extraerDatosConfig(config);

	pthread_t thrKernel, thrCpu;

	pthread_create(&thrKernel, NULL, (void *)iniciar_servidor_hacia_kernel, NULL);
	pthread_create(&thrCpu, NULL, (void *)iniciar_servidor_hacia_cpu, NULL);

	pthread_join(thrKernel, NULL);
	pthread_join(thrCpu, NULL);

	log_destroy(logger);
	config_destroy(config);

	contadorIdTablaPag = 0;
}

t_configMemoria extraerDatosConfig(t_config *archivoConfig)
{
	configMemoria.puertoEscuchaUno = string_new();
	configMemoria.puertoEscuchaDos = string_new();
	configMemoria.algoritmoReemplazo = string_new();
	configMemoria.pathSwap = string_new();

	configMemoria.puertoEscuchaUno = config_get_string_value(archivoConfig, "PUERTO_ESCUCHA_UNO");
	configMemoria.puertoEscuchaDos = config_get_string_value(archivoConfig, "PUERTO_ESCUCHA_DOS");
	configMemoria.retardoMemoria = config_get_int_value(archivoConfig, "RETARDO_MEMORIA");
	configMemoria.algoritmoReemplazo = config_get_string_value(archivoConfig, "ALGORITMO_REEMPLAZO");
	configMemoria.pathSwap = config_get_string_value(archivoConfig, "PATH_SWAP");

	configMemoria.tamMemoria = config_get_int_value(archivoConfig, "TAM_MEMORIA");
	configMemoria.tamPagina = config_get_int_value(archivoConfig, "TAM_PAGINA");
	configMemoria.entradasPorTabla = config_get_int_value(archivoConfig, "ENTRADAS_POR_TABLA");
	configMemoria.marcosPorProceso = config_get_int_value(archivoConfig, "MARCOS_POR_PROCESO");
	configMemoria.retardoSwap = config_get_int_value(archivoConfig, "RETARDO_SWAP");
	configMemoria.tamanioSwap = config_get_int_value(archivoConfig, "TAMANIO_SWAP");

	return configMemoria;
}

void iniciar_servidor_hacia_kernel()
{
	int server_fd = iniciar_servidor(IP_SERVER, configMemoria.puertoEscuchaUno);
	log_info(logger, "Servidor listo para recibir al kernel");
	int cliente_fd = esperar_cliente(server_fd);
	char *mensaje = recibirMensaje(cliente_fd);

	log_info(logger, "Mensaje de confirmacion del Kernel: %s\n", mensaje);

	while (1)
	{
		t_paqueteActual *paquete = recibirPaquete(socketAceptadoKernel);

		t_pcb *pcb = deserializoPCB(paquete->buffer);

		switch (paquete->codigo_operacion)
		{
		case ASIGNAR_RECURSOS: 
			pthread_t thrTablaPaginasCrear;

			pthread_create(&thrTablaPaginasCrear, NULL, (void *)crearTablasPaginas, (void *)pcb);
			pthread_detach(thrTablaPaginasCrear);
			break;

		case LIBERAR_RECURSOS: 
			pthread_t thrTablaPaginasEliminar;

			pthread_create(&thrTablaPaginasEliminar, NULL, (void *)eliminarTablasPaginas, (void *)pcb);
			pthread_detach(thrTablaPaginasEliminar);
			break;
		}
	}
}

void iniciar_servidor_hacia_cpu()
{
	int server_fd = iniciar_servidor(IP_SERVER, configMemoria.puertoEscuchaDos); 
	log_info(logger, "Servidor listo para recibir al cpu");

	int cliente_fd = esperar_cliente(server_fd);

	char *mensaje = recibirMensaje(cliente_fd);

	log_info(logger, "Mensaje de confirmacion del CPU: %s\n", mensaje);
	mostrar_mensajes_del_cliente(cliente_fd);
}

void crearTablasPaginas(void *pcb)
{
	t_pcb *pcbActual = (t_pcb *)pcb;

	for (int i = 0; i < list_size(pcbActual->tablaSegmentos); i++)
	{
		t_tabla_paginas *tablaPagina = malloc(sizeof(t_tabla_paginas));
		tablaPagina->modificacion = 0;
		tablaPagina->presencia = 0;
		tablaPagina->uso = 0;
		tablaPagina->nroMarco = 0;
		tablaPagina->nroPagina = 0;

		t_tabla_paginas *tablaPag = list_get(pcbActual->tablaSegmentos, i);

		pthread_mutex_lock(&mutex_creacion_ID_tabla);
		tablaPagina->idTablaPag = contadorIdTablaPag;
		contadorIdTablaPag++;
		pthread_mutex_unlock(&mutex_creacion_ID_tabla);

		list_add(pcbActual->tablaSegmentos, tablaPag);

		free(tablaPag);
	}

	serializarPCB(socketAceptadoKernel, pcb, ASIGNAR_RECURSOS);
	free(pcbActual);

}

void eliminarTablasPaginas(void *pcb)
{
	t_pcb *pcbActual = (t_pcb *)pcb;

	//eliminar los recurso de swap


}