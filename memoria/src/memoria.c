#include "memoria.h"

int main(int argc, char **argv)
{

	// Parte Server
	logger = iniciar_logger("memoria.log", "MEMORIA", LOG_LEVEL_DEBUG);

	config = iniciar_config("memoria.config");

	// creo el struct
	extraerDatosConfig(config);

	memoriaRAM = malloc(sizeof(configMemoria.tamMemoria));

	swap = abrirArchivo(configMemoria.pathSwap);

	// agregar_tabla_pag_en_swap();

	iniciar_listas_y_semaforos(); // despues ver porque kernel tambien lo utiliza y por ahi lo esta pisando, despues ver si lo dejamos solo aca

	contadorIdTablaPag = 0;

	crear_hilos_memoria();

	log_destroy(logger);

	config_destroy(config);

	fclose(swap);
	// crear una lista con el tamaño de los marcos/segmanetos para ir guardado y remplazando
	// en el caso de que esten ocupados , con los algoritmos correcpondientes

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

void crear_hilos_memoria()
{
	pthread_t thrKernel, thrCpu;

	pthread_create(&thrKernel, NULL, (void *)iniciar_servidor_hacia_kernel, NULL);
	pthread_create(&thrCpu, NULL, (void *)iniciar_servidor_hacia_cpu, NULL);

	pthread_detach(thrKernel);
	pthread_join(thrCpu, NULL);
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
		printf("\nRecibi el paquete del kernel%d\n", paquete->codigo_operacion);
		t_pcb *pcb = deserializoPCB(paquete->buffer);
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

		case PASAR_A_EXIT: // solicitud de liberar las estructuras

			// liberar las estructuras y
			// enviar msj al kernel de que ya estan liberadas
			// serializarPCB(socketAceptadoKernel, pcb, PASAR_A_EXIT);
			break;

		case PAGE_FAULT:
			// recibir del kernel pagina , segmento , id pcb
			/*int idPcb = 1;
			t_marcos_por_proceso *marcosPorProceso;
			marcosPorProceso->idPCB = idPcb;
			//cargar marcos por proceso
			asignacionDeMarcos(declararInfoMarco(),marcosPorProceso);*/

			break;
		}
	}
}

t_info_remplazo *declararInfoReemplazo()
{
	int pagina = 1;
	int segmento = 1;
	int idPcb = 1;
	t_info_remplazo *infoRemplazo = malloc(sizeof(t_info_remplazo));
	infoRemplazo->idPagina = pagina;
	infoRemplazo->idSegmento = segmento;
	infoRemplazo->PID = idPcb;

	return infoRemplazo;
}

void iniciar_servidor_hacia_cpu()
{

	int server_fd = iniciar_servidor(IP_SERVER, configMemoria.puertoEscuchaDos); // socket(), bind()listen()

	log_info(logger, "Servidor listo para recibir al cpu");

	tamanio = configMemoria.tamMemoria / configMemoria.tamPagina;

	bitmap_marco[tamanio];

	int socketAceptadoCPU = esperar_cliente(server_fd);
	char *mensaje = recibirMensaje(socketAceptadoCPU);

	log_info(logger, "Mensaje de confirmacion del CPU: %s\n", mensaje);

	t_paqt paqueteCPU;

	recibirMsje(socketAceptadoCPU, &paqueteCPU);

	if (paqueteCPU.header.cliente == CPU)
	{

		log_debug(logger, "HANDSHAKE se conecto CPU");

		conexionCPU(socketAceptadoCPU);
	}

	printf("\n termine \n");
	mostrar_mensajes_del_cliente(socketAceptadoCPU);
}

void conexionCPU(int socketAceptado)
{ // void*

	t_paqt paquete;

	int pid;
	int pagina;

	int valorRegistro = 2;
	int idPCB = 0;
	int nroSegmento = 1;
	int nroPagina = 1;

	// int y = 1;
	while (1)
	{

		recibirMsje(socketAceptado, &paquete);
		printf("avanzo aca");

		switch (paquete.header.tipoMensaje)
		{
		case CONFIG_DIR_LOG_A_FISICA:
			configurarDireccionesCPU(socketAceptado);
			break;
		case ACCESO_MEMORIA_READ: // MOV_IN
			// TODO

			break;
		case ACCESO_MEMORIA_WRITE: // MOV_OUT

			// buscar_pagina_en_memoria();
			break;

			/*default: // TODO CHEKEAR: SI FINALIZO EL CPU ANTES QUE MEMORIA, SE PRODUCE UNA CATARATA DE LOGS. PORQUE? NO HAY PORQUE
				log_error(logger, "No se reconoce el tipo de mensaje, tas metiendo la patita");
				break;*/
		}
	}
}

void configurarDireccionesCPU(int socketAceptado)
{
	// SE ENVIAN LAS ENTRADAS_POR_TABLA y TAM_PAGINA AL CPU PARA PODER HACER LA TRADUCCION EN EL MMU
	log_debug(logger, "Se envian las ENTRADAS_POR_TABLA y TAM_PAGINA al CPU ");

	MSJ_MEMORIA_CPU_INIT *infoAcpu = malloc(sizeof(MSJ_MEMORIA_CPU_INIT));

	infoAcpu->cantEntradasPorTabla = configMemoria.entradasPorTabla;

	infoAcpu->tamanioPagina = configMemoria.tamPagina;

	// usleep(configMemoria.retardoMemoria * 1000); // CHEQUEAR, SI LO DESCOMENTAS NO PASA POR LAS OTRAS LINEAS

	enviarMsje(socketAceptado, MEMORIA, infoAcpu, sizeof(MSJ_MEMORIA_CPU_INIT), CONFIG_DIR_LOG_A_FISICA);

	free(infoAcpu);

	log_debug(logger, "Informacion de la cantidad de entradas por tabla y tamaño pagina enviada al CPU");
}

void crearTablasPaginas(void *pcb) // directamente asignar el la posswap aca para no recorrer 2 veces
{
	t_pcb *pcbActual = (t_pcb *)pcb;
	for (int i = 0; i < list_size(pcbActual->tablaSegmentos); i++)
	{
		t_tabla_paginas *tablaPagina = malloc(sizeof(t_tabla_paginas));
		t_tabla_segmentos *tablaSegmento = list_get(pcbActual->tablaSegmentos, i);
		// aca tengo que crear el malloc de t_listainiciocosas
		size_t tamanioSgtePagina = 0;
		tablaPagina->paginas = list_create();
		pthread_mutex_lock(&mutex_creacion_ID_tabla);
		tablaSegmento->indiceTablaPaginas = contadorIdTablaPag;
		tablaPagina->idTablaPag = contadorIdTablaPag;
		contadorIdTablaPag++;
		pthread_mutex_unlock(&mutex_creacion_ID_tabla);

		tablaPagina->idPCB = pcbActual->id;

		for (int i = 0; i < configMemoria.entradasPorTabla; i++)
		{
			t_pagina *pagina = malloc(sizeof(t_pagina));

			pagina->modificacion = 0;
			pagina->presencia = 0;
			pagina->uso = 0;
			pagina->nroMarco = 0;
			pagina->nroPagina = i;

			pagina->posicionSwap = tamanioSgtePagina; // tamanioSgtePagina = OFFSET = desplazamiento
			fseek(swap, pagina->posicionSwap, SEEK_CUR);

			pthread_mutex_lock(&mutex_lista_tabla_paginas_pagina);
			list_add(tablaPagina->paginas, pagina);
			pthread_mutex_unlock(&mutex_lista_tabla_paginas_pagina);
		}

		tamanioSgtePagina += configMemoria.tamPagina;
		printf("\nestoy agregando tabla a la lista");
		agregar_tabla_paginas(tablaPagina);
	}

	printf("\nEnvio recursos a kernel\n");
	serializarPCB(socketAceptadoKernel, pcbActual, ASIGNAR_RECURSOS);
	printf("\nEnviados\n");
	// agregar_tabla_pag_en_swap();
	free(pcbActual);
}

void eliminarTablasPaginas(void *pcb)
{
	t_pcb *pcbActual = (t_pcb *)pcb;

	// eliminar los recurso de swap
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

void agregar_tabla_paginas(t_tabla_paginas *tablaPaginas)
{
	pthread_mutex_lock(&mutex_lista_tabla_paginas);
	list_add(LISTA_TABLA_PAGINAS, tablaPaginas);
	pthread_mutex_unlock(&mutex_lista_tabla_paginas);
}

bool esta_vacio_el_archivo(FILE *nombreFile)
{

	if (NULL != nombreFile)
	{
		fseek(nombreFile, 0, SEEK_END);
		size_t size = ftell(nombreFile);

		if (0 == size)
		{
			return true;
		}
		return false;
	}
}

bool buscar_pagina_en_memoria()
{

	t_tabla_paginas *tablaPagina;
	tablaPagina->idTablaPag;

	// list_find(t_list *, bool(*closure)(void*));
}

void *conseguir_puntero_a_base_memoria(int nro_marco, void *memoriaRAM)
{ // aca conseguimos el puntero que apunta a la posicion donde comienza el marco

	void *aux = memoriaRAM;

	return (aux + nro_marco * configMemoria.tamPagina);
}

void *conseguir_puntero_al_desplazamiento_memoria(int nro_marco, void *memoriaRAM, int desplazamiento)
{ // aca conseguimos el puntero al desplazamiento respecto al marco

	return (conseguir_puntero_a_base_memoria(nro_marco, memoriaRAM) + desplazamiento);
}

void asignacionDeMarcos(t_info_remplazo *infoRemplazo, t_marcos_por_proceso *marcosPorProceso)
{
	if (chequearCantidadMarcosPorProceso(marcosPorProceso))
	{
		asignarPaginaAMarco(marcosPorProceso, infoRemplazo->idPagina);
	}
	else
	{
		implementa_algoritmo_susticion(infoRemplazo);
	}
}

void algoritmo_reemplazo_clock(t_info_remplazo *infoRemplazo)
{
}

void algoritmo_reemplazo_clock_modificado(t_info_remplazo *infoRemplazo)
{
}

int buscar_marco_vacio() // devuelve la primera posicion del marco vacio
{
	for (int i = 0; i < strlen(bitmap_marco); i++)
	{
		if (bitmap_marco[i].uso == 0)
		{
			return i;
		}
		else
		{
			return -1;
		}
	}
}

void asignarPaginaAMarco(t_marcos_por_proceso *marcosPorProceso, int nroPagina)
{
	int posicionMarcoLibre = buscar_marco_vacio();

	t_list *tablasDelPCB = list_create();

	tablasDelPCB = filtrarPorPIDTabla(marcosPorProceso->idPCB);

	for (int i = 0; i < list_size(tablasDelPCB); i++)
	{
		t_tabla_paginas *tablaPagina = list_get(tablasDelPCB, i);

		for (int j = 0; j < list_size(tablaPagina->paginas); j++)
		{
			t_pagina *pagina = list_get(tablaPagina->paginas, j);

			if (nroPagina == pagina->nroPagina)
			{
				pagina->nroMarco = posicionMarcoLibre;

				pthread_mutex_lock(&mutex_lista_pagina_marco_por_proceso);
				list_add(marcosPorProceso->paginas, pagina);
				pthread_mutex_unlock(&mutex_lista_pagina_marco_por_proceso);

				incrementarMarcoSiquiente(marcosPorProceso);
			}
		}

		pasar_a_lista_marcos_por_procesos(marcosPorProceso);
	}
}

void incrementarMarcoSiquiente(t_marcos_por_proceso *marcosPorProceso)
{

	if (marcosPorProceso->marcoSiguiente < configMemoria.marcosPorProceso)
	{
		marcosPorProceso->marcoSiguiente = list_size(marcosPorProceso->paginas);
	}
	else
	{
		marcosPorProceso->marcoSiguiente = 0;
	}
}

void implementa_algoritmo_susticion(t_info_remplazo *infoRemplazo)
{

	switch (obtenerAlgoritmoSustitucion())
	{
	case CLOCK:
		algoritmo_reemplazo_clock(infoRemplazo);
		break;

	case CLOCK_MODIFICADO:
		algoritmo_reemplazo_clock_modificado(infoRemplazo);
		break;

	default:
		break;
	}
}

t_tipo_algoritmo_sustitucion obtenerAlgoritmoSustitucion()
{

	char *algoritmo = configMemoria.algoritmoReemplazo;

	t_tipo_algoritmo_sustitucion algoritmoResultado;

	if (!strcmp(algoritmo, "CLOCK"))
	{
		algoritmoResultado = CLOCK;
	}
	else if (!strcmp(algoritmo, "CLOCK_MODIFICADO"))
	{
		algoritmoResultado = CLOCK_MODIFICADO;
	}

	return algoritmoResultado;
}

void pasar_a_lista_marcos_por_procesos(t_marcos_por_proceso *marcosPorProceso)
{
	pthread_mutex_lock(&mutex_lista_marco_por_proceso);
	list_add(LISTA_MARCOS_POR_PROCESOS, marcosPorProceso);
	pthread_mutex_unlock(&mutex_lista_marco_por_proceso);

	// log_debug(logger, "Paso a BLOCK el proceso %d", pcb->id);
}

bool chequearCantidadMarcosPorProceso(t_marcos_por_proceso *marcosPorProceso)
{
	t_marcos_por_proceso *marcosPorProcesoActual = list_get(LISTA_MARCOS_POR_PROCESOS, marcosPorProceso->idPCB);

	return list_size(marcosPorProcesoActual->paginas) <= configMemoria.marcosPorProceso;
}

t_list *filtrarPorPIDTabla(int PID)
{
	t_list *listaTabla = list_create();

	for (int i = 0; i < list_size(LISTA_TABLA_PAGINAS); i++)
	{
		t_tabla_paginas *tablaPagina = list_get(LISTA_TABLA_PAGINAS, i);

		if (tablaPagina->idPCB == PID)
		{
			list_add(listaTabla, tablaPagina);
		}
	}
	// no hacer free de la lista

	return listaTabla;
}
