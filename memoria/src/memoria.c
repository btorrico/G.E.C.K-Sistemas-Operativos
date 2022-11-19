#include "memoria.h"
#include <sys/stat.h>

int main(int argc, char **argv)
{

	// Parte Server
	logger = iniciar_logger("memoria.log", "MEMORIA", LOG_LEVEL_DEBUG);

	config = iniciar_config("memoria.config");

	// creo el struct
	extraerDatosConfig(config);

	FILE *swap = abrirArchivo(configMemoria.pathSwap);
	// como saber el tamaño del archivo ?

	fclose(swap);

	pthread_t thrKernel, thrCpu;

	pthread_create(&thrKernel, NULL, (void *)iniciar_servidor_hacia_kernel, NULL);
	pthread_create(&thrCpu, NULL, (void *)iniciar_servidor_hacia_cpu, NULL);

	pthread_join(thrKernel, NULL);
	pthread_join(thrCpu, NULL);

	log_destroy(logger);
	config_destroy(config);

	contadorIdTablaPag = 0;

	//crear una lista con el tablaño de los marcos/segmanetos para ir guardado y remplazando 
	//en el caso de que esten ocupados , con los algoritmos correcpondientes 

	// en elarchivo swap se van a guardar las tablas enteras que voy a leer segun en los bytes que esten 
	// lo voy a buscar con el fseeck y ahi agregar , reemplazar , los tatos quedan ahi es como disco 
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
	socketAceptadoKernel = esperar_cliente(server_fd);
	char *mensaje = recibirMensaje(socketAceptadoKernel);

	log_info(logger, "Mensaje de confirmacion del Kernel: %s\n", mensaje);

	while (1)
	{
		t_paqueteActual *paquete = recibirPaquete(socketAceptadoKernel);
		if (paquete == NULL)
		{
			printf("\nel paqute es nulo\n");
		}
		printf("\nRecibi el paquete del kernel%d\n", paquete->codigo_operacion);
		t_pcb *pcb = deserializoPCB(paquete->buffer);
		printf("\nRecibi el paquete del kernel\n");
		switch (paquete->codigo_operacion)
		{
		case ASIGNAR_RECURSOS:
			printf("\nMI cod de op es: %d", paquete->codigo_operacion);
			pthread_t thrTablaPaginasCrear;
			printf("\nEntro a asignar recursos\n");
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
	t_tabla_paginas *tablaPagina = malloc(sizeof(t_tabla_paginas));
	// printf("\nentro a funcion crear tablas\n");
	for (int i = 0; i < list_size(pcbActual->tablaSegmentos); i++)
	{

		t_tabla_segmantos *tablaSegmento = list_get(pcbActual->tablaSegmentos, i);
		tablaPagina->paginas = list_create();
		pthread_mutex_lock(&mutex_creacion_ID_tabla);
		tablaSegmento->indiceTablaPaginas = contadorIdTablaPag;
		tablaPagina->idTablaPag = contadorIdTablaPag;
		contadorIdTablaPag++;
		pthread_mutex_unlock(&mutex_creacion_ID_tabla);

		for (int i = 0; i < configMemoria.entradasPorTabla; i++)
		{
			printf("\nentro al for crear pagina\n");
			t_pagina *pagina = malloc(sizeof(t_pagina));

			pagina->modificacion = 0;
			pagina->presencia = 0;
			pagina->uso = 0;
			pagina->nroMarco = 0;
			pagina->nroPagina = i;

			list_add(tablaPagina->paginas, pagina);
		}
	}
	agregar_tabla_paginas(tablaPagina);
	printf("\nEnvio recursos a kernel\n");
	serializarPCB(socketAceptadoKernel, pcb, ASIGNAR_RECURSOS);
	printf("\nEnviados\n");
	free(pcbActual);
}

void eliminarTablasPaginas(void *pcb)
{
	t_pcb *pcbActual = (t_pcb *)pcb;

	// eliminar los recurso de swap
}

void agregar_tabla_paginas(t_tabla_paginas *tablaPaginas)
{
	pthread_mutex_lock(&mutex_lista_tabla_paginas);
	list_add(LISTA_TABLA_PAGINAS, tablaPaginas);
	pthread_mutex_unlock(&mutex_lista_tabla_paginas);
}

FILE *abrirArchivo(char *filename)
{
	if (filename == NULL)
	{
		log_error(logger, "Error: debe informar un path correcto.");
		exit(1);
	}

	truncate(filename, configMemoria.tamanioSwap);
	return fopen(filename, "w+");
}