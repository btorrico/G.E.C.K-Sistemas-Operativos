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
	tamanio = configMemoria.tamMemoria / configMemoria.tamPagina;

	inicializar_bitmap();
	// printf("\n%d",bitmap_marco[0].uso);
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
			printf("\nMi cod de op es: %d", paquete->codigo_operacion);
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
			printf("\nestoy en page fault de memoria\n");
			t_paqt paquete;
			recibirMsje(socketAceptadoKernel, &paquete);
			MSJ_CPU_KERNEL_BLOCK_PAGE_FAULT *mensaje = malloc(sizeof(MSJ_CPU_KERNEL_BLOCK_PAGE_FAULT));
			mensaje = paquete.mensaje;

			t_info_remplazo *infoRemplazo = malloc(sizeof(t_info_remplazo));
			infoRemplazo->idPagina = mensaje->nro_pagina;
			infoRemplazo->idSegmento = mensaje->nro_segmento;
			infoRemplazo->PID = pcb->id;

			printf("\nel id de pagina es: %d", infoRemplazo->idPagina);
			printf("\nel id de seg es: %d", infoRemplazo->idSegmento);

			t_marcos_por_proceso *marcoPorProceso = list_get(LISTA_MARCOS_POR_PROCESOS, pcb->id - 1);

			t_pagina *paginaUno = malloc(sizeof(t_pagina));
			paginaUno->uso = 1;
			paginaUno->nroMarco = 2;
			paginaUno->modificacion = 0;
			paginaUno->nroPagina = 1;
			paginaUno->presencia = 0;
			paginaUno->nroSegmento = 1;
			paginaUno->posicionSwap = 1;

			t_pagina *paginaDos = malloc(sizeof(t_pagina));
			paginaUno->uso = 0;
			paginaUno->nroMarco = 0;
			paginaUno->modificacion = 0;
			paginaUno->nroPagina = 3;
			paginaUno->presencia = 0;
			paginaUno->nroSegmento = 1;
			paginaUno->posicionSwap = 1;

			agregar_pagina_a_lista_de_paginas_marcos_por_proceso(marcoPorProceso, paginaUno);
			agregar_pagina_a_lista_de_paginas_marcos_por_proceso(marcoPorProceso, paginaDos);
			agregar_pagina_a_lista_de_paginas_marcos_por_proceso(marcoPorProceso, paginaUno);
			agregar_pagina_a_lista_de_paginas_marcos_por_proceso(marcoPorProceso, paginaUno);

			marcoPorProceso->marcoSiguiente=0;
			printf("\nel id de pcb es: %d\n", marcoPorProceso->idPCB);

			asignacionDeMarcos(infoRemplazo, marcoPorProceso);

			break;
		}
	}
}

void iniciar_servidor_hacia_cpu()
{

	int server_fd = iniciar_servidor(IP_SERVER, configMemoria.puertoEscuchaDos); // socket(), bind()listen()

	log_info(logger, "Servidor listo para recibir al cpu");

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
	int idTablaPagina;
	t_direccionFisica *direccionFisica;

	MSJ_MEMORIA_CPU_ACCESO_TABLA_DE_PAGINAS *infoMemoriaCpuTP;
	MSJ_MEMORIA_CPU_LEER *infoMemoriaCpuLeer;

	// int valorRegistro = 2;
	// int idPCB = 0;
	// int nroSegmento = 1;
	// int nroPagina = 1;

	// int y = 1;
	while (1)
	{
		direccionFisica = malloc(sizeof(t_direccionFisica));
		infoMemoriaCpuTP = malloc(sizeof(MSJ_MEMORIA_CPU_ACCESO_TABLA_DE_PAGINAS));
		infoMemoriaCpuLeer = malloc(sizeof(MSJ_MEMORIA_CPU_LEER));

		recibirMsje(socketAceptado, &paquete);

		switch (paquete.header.tipoMensaje)
		{
		case CONFIG_DIR_LOG_A_FISICA:
			configurarDireccionesCPU(socketAceptado);
			break;
		case ACCESO_MEMORIA_TABLA_DE_PAG:
			infoMemoriaCpuTP = paquete.mensaje;
			idTablaPagina = infoMemoriaCpuTP->idTablaDePaginas;
			pagina = infoMemoriaCpuTP->pagina;
			pid = infoMemoriaCpuTP->pid;
			accesoMemoriaTP(idTablaPagina, pagina, pid, socketAceptado);
			break;
		case ACCESO_MEMORIA_LEER:
			printf("llegue aqui");
			infoMemoriaCpuLeer = paquete.mensaje;
			direccionFisica->nroMarco = infoMemoriaCpuLeer->nroMarco;
			direccionFisica->desplazamientoPagina = infoMemoriaCpuLeer->desplazamiento;
			pid = infoMemoriaCpuLeer->pid;
			// accesoMemorialeer(direccionFisica, pid, socketAceptado);
			break;
		default: // TODO CHEKEAR: SI FINALIZO EL CPU ANTES QUE MEMORIA, SE PRODUCE UNA CATARATA DE LOGS. PORQUE? NO HAY PORQUE
			log_error(logger, "No se reconoce el tipo de mensaje, tas metiendo la patita");
			break;
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
	t_marcos_por_proceso *marcoPorProceso = malloc(sizeof(t_marcos_por_proceso));

	for (int i = 0; i < list_size(pcbActual->tablaSegmentos); i++)
	{
		t_tabla_paginas *tablaPagina = malloc(sizeof(t_tabla_paginas));
		t_tabla_segmentos *tablaSegmento = list_get(pcbActual->tablaSegmentos, i);

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
			pagina->nroSegmento = tablaPagina->idTablaPag;

			pagina->posicionSwap = tamanioSgtePagina; // tamanioSgtePagina = OFFSET = desplazamiento
			fseek(swap, pagina->posicionSwap, SEEK_CUR);

			// agrego pagina a lista de paginas de la tabla pagina
			agregar_pagina_a_tabla_paginas(tablaPagina, pagina);
		}

		tamanioSgtePagina += configMemoria.tamPagina;

		// agrego tabla pagina a la lista de tablas pagina
		agregar_tabla_paginas(tablaPagina);
		printf("\nla cant de elem que tiene listatablapaginas es: %d\n", list_size(LISTA_TABLA_PAGINAS));

		log_info(logger, "PID: %d - Segmento: %d - TAMAÑO: %d paginas", tablaPagina->idPCB, tablaPagina->idTablaPag, list_size(tablaPagina->paginas));
	}

	marcoPorProceso->idPCB = pcbActual->id;
	marcoPorProceso->marcoSiguiente = 0;
	marcoPorProceso->paginas = list_create();

	// agrego marco por proceso a la lista de marcos por procesos
	agregar_marco_por_proceso(marcoPorProceso);

	serializarPCB(socketAceptadoKernel, pcbActual, ASIGNAR_RECURSOS);

	// agregar_tabla_pag_en_swap();
	free(pcbActual);
}

void eliminarTablasPaginas(void *pcb)
{
	t_pcb *pcbActual = (t_pcb *)pcb;

	filtrarYEliminarMarcoPorPIDTabla(pcbActual->id);

	t_list *tablasDelPCB = filtrarPorPIDTabla(pcbActual->id);

	for (int i = 0; i < list_size(tablasDelPCB); i++)
	{
		t_tabla_paginas *tablaPagina = list_get(tablasDelPCB, i);

		for (int i = 0; i < list_size(tablaPagina->paginas); i++)
		{
			t_pagina *pagina = list_get(tablaPagina->paginas, i);

			pagina->posicionSwap = -1;
			pagina->presencia = -1;
			pagina->nroMarco = -1;
			pagina->modificacion = -1;
			pagina->uso = -1;
		}
		log_info(logger, "PID: %d - Segmento: %d - TAMAÑO: %d paginas", tablaPagina->idPCB, tablaPagina->idTablaPag, list_size(tablaPagina->paginas));
	}

	enviarResultado(socketAceptadoKernel, "se liberaron las estructuras");
}

FILE *abrirArchivo(char *filename)
{
	if (filename == NULL)
	{
		log_error(logger, "Error: debe informar un path correcto");
		exit(1);
	}

	truncate(filename, configMemoria.tamanioSwap);
	return fopen(filename, "w+");
}

void agregar_tabla_paginas(t_tabla_paginas *tablaPagina)
{
	pthread_mutex_lock(&mutex_lista_tabla_paginas);
	list_add(LISTA_TABLA_PAGINAS, tablaPagina);
	pthread_mutex_unlock(&mutex_lista_tabla_paginas);
}

void agregar_pagina_a_tabla_paginas(t_tabla_paginas *tablaPagina, t_pagina *pagina)
{
	pthread_mutex_lock(&mutex_lista_tabla_paginas_pagina);
	list_add(tablaPagina->paginas, pagina);
	pthread_mutex_unlock(&mutex_lista_tabla_paginas_pagina);
}

void accesoMemoriaLeer(t_direccionFisica *df, int pid, int socketAceptado)
{
	log_debug(logger, "[INIT - ACCESO_MEMORIA_READ] DIR_FISICA: %d%d",
			  df->nroMarco, df->desplazamientoPagina);

	int nroFrame = df->nroMarco;
	int desplazamiento = df->desplazamientoPagina;
	int tamanioFrame = configMemoria.tamPagina; // Cambiar cuando este resuelta la inicializacion de estructuras. Ambos pueden ser variables definidas en utils
	int cantidadTotalDeFrames = 512;			// 512 nro de ejemplo, cambiar cuando esten inicializadas las estructuras . Ambos pueden ser variables definidas en utils
	void *memoriaRAM;							// sacarlo cuando se definan las estructuras
	void *aLeer = malloc(tamanioFrame - desplazamiento);
	int valorLeido;
	MSJ_STRING *msjeError;

	// valido que el offset sea valido
	if (desplazamiento > tamanioFrame)
	{
		usleep(configMemoria.retardoMemoria * 1000);
		msjeError = malloc(sizeof(MSJ_STRING));
		string_append(&msjeError->cadena, "ERROR_DESPLAZAMIENTO");
		enviarMsje(socketAceptado, MEMORIA, msjeError, sizeof(MSJ_STRING), ACCESO_MEMORIA_LEER);
		free(msjeError);
		log_error(logger, "[ACCESO_MEMORIA_READ] OFFSET MAYOR AL TAMANIO DEL FRAME.  DIR_FISICA: %d%d",
				  df->nroMarco, df->desplazamientoPagina);
		return;
	}

	// valido que el nro frame sea valido
	if (nroFrame > cantidadTotalDeFrames)
	{
		usleep(configMemoria.retardoMemoria * 1000);
		msjeError = malloc(sizeof(MSJ_STRING));
		string_append(&msjeError->cadena, "ERROR_NRO_FRAME");
		enviarMsje(socketAceptado, MEMORIA, msjeError, sizeof(MSJ_STRING), ACCESO_MEMORIA_LEER);
		free(msjeError);
		log_error(logger, "[ACCESO_MEMORIA_READ] NRO DE FRAME INEXISTENTE.  DIR_FISICA: %d%d",
				  df->nroMarco, df->desplazamientoPagina);
		return;
	}

	pthread_mutex_lock(&mutex_void_memoria_ram);
	memcpy(aLeer, memoriaRAM + (nroFrame * tamanioFrame) + desplazamiento, tamanioFrame - desplazamiento);
	pthread_mutex_unlock(&mutex_void_memoria_ram);
	char **cosa2 = string_array_new();
	cosa2 = string_split((char *)aLeer, "*");
	char *leidoStringArray = string_new();
	int size = string_array_size(cosa2) - 1;
	for (int i = 0; i < size; i++)
	{
		string_append(&leidoStringArray, cosa2[i]);
	}
	valorLeido = atoi(leidoStringArray);

	log_debug(logger, "Valor Leido: %s", leidoStringArray);

	usleep(configMemoria.retardoMemoria * 1000);

	/***********************************************/
	t_tabla_paginas *tablaPaginas;
	t_pagina *pagina;
	bool update = false;
	pthread_mutex_lock(&mutex_lista_tabla_paginas);
	for (int i = 0; i < list_size(LISTA_TABLA_PAGINAS) && !update; i++)
	{
		tablaPaginas = list_get(LISTA_TABLA_PAGINAS, i);
		for (int j = 0; j < list_size(tablaPaginas->paginas) && !update; j++)
		{
			pagina = list_get(tablaPaginas->paginas, j);
			if (pagina->nroMarco == nroFrame)
			{
				pagina->uso = 1;
				update = true;
			}
		}
	}
	pthread_mutex_unlock(&mutex_lista_tabla_paginas);

	/***********************************************/
	MSJ_INT *mensajeRead = malloc(sizeof(MSJ_INT));
	mensajeRead->numero = valorLeido;
	enviarMsje(socketAceptado, MEMORIA, mensajeRead, sizeof(MSJ_INT), ACCESO_MEMORIA_LEER);
	free(leidoStringArray);
	free(cosa2);
	free(mensajeRead);

	log_debug(logger, "ACCESO_MEMORIA_READ DIR_FISICA: frame%d offset%d",
			  df->nroMarco, df->desplazamientoPagina);
}

/*Acceso a tabla de páginas
El módulo deberá responder el número de marco correspondiente, en caso de no encontrarse,
se deberá retornar Page Fault.*/

void accesoMemoriaTP(int idTablaPagina, int nroPagina, int pid, int socketAceptado)
{
	// CPU SOLICITA CUAL ES EL MARCO DONDE ESTA LA PAGINA DE ESA TABLA DE PAGINA
	log_debug(logger, "ACCEDIENDO A TABLA DE PAGINA ID: %d NRO_PAGINA: %d",
			  idTablaPagina, nroPagina);

	int marcoBuscado;
	t_pagina *pagina;
	t_tabla_paginas *tabla_de_paginas;
	int indice;
	bool corte = false;
	// busco la pagina que piden
	pthread_mutex_lock(&mutex_lista_tabla_paginas);
	for (int i = 0; i < list_size(LISTA_TABLA_PAGINAS) && !corte; i++)
	{
		tabla_de_paginas = list_get(LISTA_TABLA_PAGINAS, i);
		if (tabla_de_paginas->idTablaPag == idTablaPagina)
		{
			for (int j = 0; j < list_size(tabla_de_paginas->paginas) && !corte; j++)
			{
				pagina = list_get(tabla_de_paginas->paginas, j);
				if (pagina->nroPagina == nroPagina)
				{
					log_debug(logger, "BUSCO PAGINA %d", pagina->nroPagina);
					indice = j;
					corte = true;
				}
			}
		}
	}
	pthread_mutex_unlock(&mutex_lista_tabla_paginas);
	corte = false; // para probar page fault -- BORRAR LUEGO DE PROBAR
	if (corte == true)
	{ // REVISAR
		pthread_mutex_unlock(&mutex_lista_tabla_paginas);
		marcoBuscado = pagina->nroMarco;
		log_debug(logger, "[ACCESO_TABLA_PAGINAS] LA PAGINA ESTA EN RAM");

		MSJ_INT *mensaje = malloc(sizeof(MSJ_INT));
		mensaje->numero = marcoBuscado;

		enviarMsje(socketAceptado, MEMORIA, mensaje, sizeof(MSJ_INT), RESPUESTA_MEMORIA_MARCO_BUSCADO);
		free(mensaje);
		log_debug(logger, "[FIN - TRADUCCION_DIR] FRAME BUSCADO = %d ,DE LA PAGINA: %d DE TABLA DE PAG CON INDICE: %d ENVIADO A CPU",
				  marcoBuscado, pagina->nroPagina, tabla_de_paginas->idTablaPag); // chequear y borrar

		log_debug(logger, "Acceso a Tabla de Páginas: “PID: %d - Página: %d - Marco: %d ",
				  pid, pagina->nroPagina, marcoBuscado);
	}
	else
	{ // la pag no esta en ram. Retornar PAGE FAULT

		MSJ_INT *mensaje = malloc(sizeof(MSJ_INT));
		mensaje->numero = -1;
		enviarMsje(socketAceptado, MEMORIA, mensaje, sizeof(MSJ_INT), PAGE_FAULT);
		free(mensaje);
		log_debug(logger, "PAGE FAULT");
	}
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
	printf("\nentro a asignacion de marcos\n");
	if (chequearCantidadMarcosPorProceso(marcosPorProceso))
	{
		printf("\nLa cantidad de marcos del proceso es correcta\n");
		asignarPaginaAMarco(marcosPorProceso, infoRemplazo);
	}
	else
	{
		printf("\n entro a los algoritmos de reemplazo\n");
		implementa_algoritmo_susticion(infoRemplazo);
		enviarResultado(socketAceptadoKernel, "Algoritmos de reemoplazo realizados correctamente");
	}
}

void algoritmo_reemplazo_clock(t_info_remplazo *infoRemplazo)
{
	pthread_mutex_lock(&mutex_lista_marco_por_proceso);
	t_marcos_por_proceso *marcosPorProceso = list_get(LISTA_MARCOS_POR_PROCESOS, infoRemplazo->PID - 1);
	pthread_mutex_unlock(&mutex_lista_marco_por_proceso);
	printf("\n Estoy en algoritmo clock\n");
	primer_recorrido_paginas_clock(marcosPorProceso, infoRemplazo);
}

void primer_recorrido_paginas_clock(t_marcos_por_proceso *marcosPorProceso, t_info_remplazo *infoRemplazo)
{

	printf("\n Estoy en algoritmo clock primer recorrido \n");
	marcosPorProceso->marcoSiguiente = recorrer_marcos(marcosPorProceso->marcoSiguiente);

	for (int i = marcosPorProceso->marcoSiguiente; i < list_size(marcosPorProceso->paginas); recorrer_marcos(marcosPorProceso->marcoSiguiente))
	{
		log_debug(logger, "entre al for");
		t_pagina *pagina = list_get(marcosPorProceso->paginas, i);
		log_info(logger, "Reemplazo - PID: %d", infoRemplazo->PID);

		// nunca va a pasar por aca porque a esto lo maneja cpu como es por pf significa que lo que me manden lo va a estar cargado en memoria
		/*if (pagina->nroPagina == infoRemplazo->idPagina)
		{
			pagina->uso = 1;
			break;
		}*/
		// el puntero no tiene que moverse si la pagina ya esta cargada en memoria

		if (pagina->uso == 0)
		{
			if (pagina->modificacion == 1)
			{
				fseek(swap, pagina->posicionSwap, SEEK_SET);
				fwrite(conseguir_puntero_a_base_memoria(pagina->nroMarco, memoriaRAM), configMemoria.tamPagina, NULL, swap);
				usleep(configMemoria.retardoSwap * 1000);
				log_info(logger, "SWAP OUT -  PID: %d - Marco: %d - Page Out: %d|%d", marcosPorProceso->idPCB, pagina->nroMarco, pagina->nroSegmento, pagina->nroPagina);
			}

			t_pagina *newPagina = buscarPagina(infoRemplazo);

			newPagina->nroMarco = pagina->nroMarco;
			newPagina->uso = 1;
			newPagina->presencia = 1;
			newPagina->modificacion = 0;

			void *pagina = malloc(configMemoria.tamPagina);
			fseek(swap, newPagina->posicionSwap, SEEK_SET);
			fread(pagina, configMemoria.tamPagina, NULL, swap);

			memcpy(conseguir_puntero_a_base_memoria(newPagina->nroMarco, memoriaRAM), pagina, configMemoria.tamPagina);

			usleep(configMemoria.retardoSwap * 1000);

			t_pagina *paginaVictima = list_replace(marcosPorProceso->paginas, i, newPagina);

			paginaVictima->uso = 0;
			paginaVictima->modificacion = 0;
			paginaVictima->presencia = 0;

			log_info(logger, "Marco:%d", newPagina->nroMarco);
			log_info(logger, "Page In: %d | %d ", infoRemplazo->idSegmento, infoRemplazo->idPagina);
			log_info(logger, "Page Out: %d | %d ", paginaVictima->nroSegmento, paginaVictima->nroPagina);

			marcosPorProceso->marcoSiguiente = recorrer_marcos(i); // para que el puntero al siguiente siga abanzando
			break;
		}
		else if (pagina->uso == 1)
		{
			pagina->uso = 0;
		}
	}
}

t_pagina *buscarPagina(t_info_remplazo *infoRemplazo)
{
	t_list *tablasNewPagina = list_create();
	tablasNewPagina = filtrarPorPIDTabla(infoRemplazo->PID);
	printf("\ncant de tablas en pcb : %d\n", list_size(tablasNewPagina));
	for (int i = 0; i < list_size(tablasNewPagina); i++)
	{
		t_tabla_paginas *tablaPagina = list_get(tablasNewPagina, i);

		if (tablaPagina->idTablaPag == infoRemplazo->idSegmento)
		{
			printf("\nconcuerda el id de la tabla con el segmento, encontramos la tabla!!\n");
			for (int j = 0; j < list_size(tablaPagina->paginas); j++)
			{
				t_pagina *pagina = list_get(tablaPagina->paginas, j);

				if (infoRemplazo->idPagina == pagina->nroPagina)
				{
					printf("\nconcuerda el id de la pagina, con la pagina que queriamos encontrar, encontramos la pagina!!\n");
					return pagina;
				}
			}
		}
	}
}

int recorrer_marcos(int marcoSiguiente)
{
	if (marcoSiguiente == configMemoria.marcosPorProceso)
	{
		return 0;
	}
	else
	{
		return marcoSiguiente++;
	}
}

void algoritmo_reemplazo_clock_modificado(t_info_remplazo *infoRemplazo)
{
	pthread_mutex_lock(&mutex_lista_marco_por_proceso);
	t_marcos_por_proceso *marcosPorProceso = list_get(LISTA_MARCOS_POR_PROCESOS, infoRemplazo->PID - 1);
	pthread_mutex_unlock(&mutex_lista_marco_por_proceso);

	t_pagina *pagina = NULL;
	while (pagina == NULL)
	{
		pagina = buscarMarcoSegun(marcosPorProceso, infoRemplazo, 0, 0);
		if (pagina == NULL)
		{ // si no encontro pagina en (0-0) cambio uso
			pagina = buscarMarcoSegun(marcosPorProceso, infoRemplazo, 0, 1);
		}
	}
	t_pagina *paginaVictima = list_replace(marcosPorProceso->paginas, marcosPorProceso->marcoSiguiente, pagina);

	log_info(logger, "Marco:%d", paginaVictima->nroMarco);
	log_info(logger, "Page In: %d | %d ", infoRemplazo->idSegmento, pagina->nroPagina);
	log_info(logger, "Page Out: %d | %d ", paginaVictima->nroSegmento, paginaVictima->nroPagina);

	if (paginaVictima->modificacion == 1)
	{
		fseek(swap, paginaVictima->posicionSwap, SEEK_SET);
		fwrite(paginaVictima, sizeof(t_pagina), NULL, swap);
	}

	pagina->nroMarco = paginaVictima->nroMarco;
	marcosPorProceso->marcoSiguiente = recorrer_marcos(marcosPorProceso->marcoSiguiente); // para que el puntero al siguiente siga abanzando
	pagina->uso = 1;
	pagina->presencia = 1;
}

t_pagina *buscarMarcoSegun(t_marcos_por_proceso *marcosPorProceso, t_info_remplazo *infoRemplazo, int uso, int modificado)
{

	int i = marcosPorProceso->marcoSiguiente;

	while (1)
	{
		t_pagina *pagina = list_get(marcosPorProceso, i);
		if (pagina->uso == uso && pagina->modificacion == modificado)
		{
			marcosPorProceso->marcoSiguiente = i;
			return pagina;
		}
		// solo entra en busqueda de (0-1)
		if (modificado == 1)
		{
			pagina->uso = 0;
		}
		i = recorrer_marcos(i);
		if (i == marcosPorProceso->marcoSiguiente)
		{ // si pego la vuelta, inicio
			return NULL;
		}
	}
}

int buscar_marco_vacio() // devuelve la primera posicion del marco vacio, ver bien esto, tiene que incrementar
{
	for (int i = 0; i < list_size(LISTA_BITMAP_MARCO); i++)
	{
		t_bitmap_marcos_libres *marcoLibre = list_get(LISTA_BITMAP_MARCO, i);

		if (marcoLibre->uso == 0)
		{
			printf("\nencontre un marco libre!!\n");
			// marcoLibre->nroMarco = i;
			marcoLibre->uso = 1;
			printf("\nnroMArco: %d, bit de uso: %d\n", marcoLibre->nroMarco, marcoLibre->uso);
			return i;
		}
	}

	return -1;
}

void inicializar_bitmap()
{
	for (int i = 0; i < tamanio; i++)
	{
		t_bitmap_marcos_libres *marcoLibre = malloc(sizeof(t_bitmap_marcos_libres));
		marcoLibre->nroMarco = i;

		marcoLibre->uso = 0;

		list_add(LISTA_BITMAP_MARCO, marcoLibre);
	}
}

void asignarPaginaAMarco(t_marcos_por_proceso *marcosPorProceso, t_info_remplazo *infoReemplazo)
{
	printf("\nentro a asignar marcos por proceso");
	int posicionMarcoLibre = buscar_marco_vacio();

	printf("\nLa posicion del marco libre es: %d\n", posicionMarcoLibre);

	t_pagina *pagina = buscarPagina(infoReemplazo);

	pagina->nroMarco = posicionMarcoLibre;
	pagina->uso = 1;
	pagina->presencia = 1;

	printf("\nEl numero de marco asignado a la pagina es: %d", pagina->nroMarco);

	agregar_pagina_a_lista_de_paginas_marcos_por_proceso(marcosPorProceso, pagina);

	incrementarMarcoSiguiente(marcosPorProceso);

	enviarResultado(socketAceptadoKernel, " asignacion de marcos realizada correctamente");
	printf("\nLa cant de pag del pcb %d del marcoPorProceso es: %d\n", marcosPorProceso->idPCB, list_size(marcosPorProceso->paginas));
}

void incrementarMarcoSiguiente(t_marcos_por_proceso *marcosPorProceso)
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
		printf("\nEntrando a algoritmo Clock\n");
		algoritmo_reemplazo_clock(infoRemplazo);
		break;

	case CLOCK_MODIFICADO:
		printf("\nEntrando a algoritmo Clock Modificado\n");
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

void agregar_marco_por_proceso(t_marcos_por_proceso *marcosPorProceso)
{
	pthread_mutex_lock(&mutex_lista_marco_por_proceso);
	list_add(LISTA_MARCOS_POR_PROCESOS, marcosPorProceso);
	pthread_mutex_unlock(&mutex_lista_marco_por_proceso);

	printf("\nLa cantidad de marcos por proceso es: %d", list_size(LISTA_MARCOS_POR_PROCESOS));
}

void agregar_pagina_a_lista_de_paginas_marcos_por_proceso(t_marcos_por_proceso *marcosPorProceso, t_pagina *pagina)
{
	pthread_mutex_lock(&mutex_lista_marcos_por_proceso_pagina);
	list_add(marcosPorProceso->paginas, pagina);
	pthread_mutex_unlock(&mutex_lista_marcos_por_proceso_pagina);
}

bool chequearCantidadMarcosPorProceso(t_marcos_por_proceso *marcosPorProceso)
{
	pthread_mutex_lock(&mutex_lista_marco_por_proceso);
	t_marcos_por_proceso *marcosPorProcesoActual = list_get(LISTA_MARCOS_POR_PROCESOS, marcosPorProceso->idPCB - 1);
	pthread_mutex_unlock(&mutex_lista_marco_por_proceso);

	return list_size(marcosPorProcesoActual->paginas) < configMemoria.marcosPorProceso;
}

t_list *filtrarPorPIDTabla(int PID)
{
	t_list *listaTabla = list_create();
	int i = 0;

	while (i < list_size(LISTA_TABLA_PAGINAS))
	{
		t_tabla_paginas *tablaPagina = list_get(LISTA_TABLA_PAGINAS, i);

		if (tablaPagina->idPCB == PID)
		{
			list_add(listaTabla, tablaPagina);
		}
		i++;
	}

	return listaTabla;
}

void filtrarYEliminarMarcoPorPIDTabla(int PID)
{
	int i = 0;

	while (i < list_size(LISTA_MARCOS_POR_PROCESOS))
	{
		t_marcos_por_proceso *marcoPorProceso = list_get(LISTA_MARCOS_POR_PROCESOS, i);

		if (marcoPorProceso->idPCB == PID)
		{
			t_marcos_por_proceso *marcoAEliminar = list_remove(LISTA_MARCOS_POR_PROCESOS, i);

			eliminarEstructura(marcoAEliminar);
		}
		i++;
	}
}

void eliminarEstructura(t_marcos_por_proceso *marcoAEliminar)
{
	for (int i = 0; i < list_size(marcoAEliminar->paginas); i++)
	{
		t_pagina *paginaAEliminar = list_remove(marcoAEliminar->paginas, i);

		free(paginaAEliminar);
	}
	free(marcoAEliminar);
}