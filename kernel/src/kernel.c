#include "kernel.h"

int main(int argc, char **argv)
{

	iniciar_kernel();

	crear_hilos_kernel();
}

t_configKernel extraerDatosConfig(t_config *archivoConfig)
{
	configKernel.ipMemoria = string_new();
	configKernel.puertoMemoria = string_new();
	configKernel.ipCPU = string_new();
	configKernel.puertoCPUDispatch = string_new();
	configKernel.puertoCPUInterrupt = string_new();
	configKernel.puertoEscucha = string_new();
	configKernel.algoritmo = string_new();

	configKernel.ipMemoria = config_get_string_value(archivoConfig, "IP_MEMORIA");
	configKernel.puertoMemoria = config_get_string_value(archivoConfig, "PUERTO_MEMORIA");
	configKernel.ipCPU = config_get_string_value(archivoConfig, "IP_CPU");
	configKernel.puertoCPUDispatch = config_get_string_value(archivoConfig, "PUERTO_CPU_DISPATCH");
	configKernel.puertoCPUInterrupt = config_get_string_value(archivoConfig, "PUERTO_CPU_INTERRUPT");
	configKernel.puertoEscucha = config_get_string_value(archivoConfig, "PUERTO_ESCUCHA");
	configKernel.algoritmo = config_get_string_value(archivoConfig, "ALGORITMO_PLANIFICACION");
	configKernel.gradoMultiprogramacion = config_get_int_value(archivoConfig, "GRADO_MAX_MULTIPROGRAMACION");
	configKernel.dispositivosIO = config_get_array_value(archivoConfig, "DISPOSITIVOS_IO");
	configKernel.tiemposIO = config_get_array_value(archivoConfig, "TIEMPOS_IO");

	return configKernel;
}

void crear_hilos_kernel()
{
	pthread_t thrCpu, thrMemoria, thrPlanificadorLargoPlazo, thrPlanificadorCortoPlazo, thrBloqueo, thrConsola;

	pthread_create(&thrConsola, NULL, (void *)crear_hilo_consola, NULL);
	pthread_create(&thrCpu, NULL, (void *)crear_hilo_cpu, NULL);
	pthread_create(&thrMemoria, NULL, (void *)conectar_memoria, NULL);
	pthread_create(&thrPlanificadorLargoPlazo, NULL, (void *)planifLargoPlazo, NULL);
	pthread_create(&thrPlanificadorCortoPlazo, NULL, (void *)planifCortoPlazo, NULL);

	pthread_detach(thrCpu);
	pthread_detach(thrPlanificadorCortoPlazo);
	pthread_detach(thrMemoria);
	pthread_detach(thrPlanificadorLargoPlazo);
	// crear_hilo_consola();
	pthread_join(thrConsola, NULL);

	log_destroy(logger);
	config_destroy(config);
}

void crear_hilo_consola()
{
	int server_fd = iniciar_servidor(IP_SERVER, configKernel.puertoEscucha); // socket(), bind() listen()
	log_info(logger, "Servidor listo para recibir al cliente");

	while (1)
	{
		pthread_t hilo_atender_consola;
		t_args_pcb argumentos;
		// esto se podria cambiar como int* cliente_fd= malloc(sizeof(int)); si lo ponemos, va antes del while
		argumentos.socketCliente = esperar_cliente(server_fd);

		int cod_op = recibir_operacion(argumentos.socketCliente);

		log_info(logger, "Llegaron las instrucciones y los segmentos");
		argumentos.informacion = recibir_informacion(argumentos.socketCliente);

		enviarResultado(argumentos.socketCliente, "Quedate tranqui Consola, llego todo lo que mandaste ;)\n");
		pthread_create(&hilo_atender_consola, NULL, (void *)crear_pcb, (void *)&argumentos);
		pthread_detach(hilo_atender_consola);
	}

	log_error(logger, "Muere hilo multiconsolas");
}

void crear_hilo_cpu()
{

	pthread_t thrDispatch, thrInterrupt;

	pthread_create(&thrDispatch, NULL, (void *)conectar_dispatch, NULL);
	pthread_create(&thrInterrupt, NULL, (void *)conectar_interrupt, NULL);

	pthread_detach(thrDispatch);
	pthread_detach(thrInterrupt);
}

void conectar_dispatch()
{
	// Enviar PCB
	conexionDispatch = crear_conexion(configKernel.ipCPU, configKernel.puertoCPUDispatch);

	while (1)
	{

		sem_wait(&sem_pasar_pcb_running);
		printf("Llego un pcb a dispatch");
		serializarPCB(conexionDispatch, list_get(LISTA_EXEC, 0), DISPATCH_PCB);
		printf("\nse envio pcb a cpu\n");
		void *pcbAEliminar = list_remove(LISTA_EXEC, 0);
		free(pcbAEliminar);
		printf("\ncantidad de elementos en lista exec: %d\n", list_size(LISTA_EXEC));

		// Recibir PCB

		t_paqueteActual *paquete = recibirPaquete(conexionDispatch);
		printf("\nRecibi de nuevo el pcb\n");
		printf("\nestoy en %d: ", paquete->codigo_operacion);
		t_pcb *pcb = deserializoPCB(paquete->buffer);
		printf("\nestoy en %d: ", paquete->codigo_operacion);
		printf("\n Id proceso nuevo que llego de cpu: %d", pcb->id);

		t_instruccion *insActual = list_get(pcb->informacion->instrucciones, pcb->program_counter - 1);
		char *dispositivoIO;

		if (hayTimer == true)
		{
			printf("\nsempostkilltrhread\n");
			sem_post(&sem_kill_trhread);
			hayTimer = false;
		}

		switch (paquete->codigo_operacion)
		{
		case EXIT_PCB:
			printf("\nestoy en %d: ", paquete->codigo_operacion);
			pasar_a_exec(pcb);
			eliminar_pcb();
			serializarValor(0, pcb->socket, TERMINAR_CONSOLA);
			break;

		case BLOCK_PCB_IO_PANTALLA:
			pthread_t thrBloqueoPantalla;
			dispositivoIO = dispositivoToString(insActual->paramIO);

			pasar_a_block_pantalla(pcb);

			pthread_create(&thrBloqueoPantalla, NULL, (void *)manejar_bloqueo_pantalla, (void *)insActual);

			pthread_detach(thrBloqueoPantalla);
			sem_post(&contador_pcb_running);
			break;

		case BLOCK_PCB_IO_TECLADO:
			pthread_t thrBloqueoTeclado;
			dispositivoIO = dispositivoToString(insActual->paramIO);
			dispositivoIO = dispositivoToString(insActual->paramIO);

			pasar_a_block_teclado(pcb);

			log_debug(logger, "Ejecutada: 'PID:  %d - Bloqueado por: %s '", pcb->id, dispositivoIO);

			pthread_create(&thrBloqueoTeclado, NULL, (void *)manejar_bloqueo_teclado, (void *)insActual);

			pthread_detach(thrBloqueoTeclado);
			sem_post(&contador_pcb_running);

			break;

		case BLOCK_PCB_IO:
			pthread_t thrBloqueoGeneralImpresora, thrBloqueoGeneralDisco;
			dispositivoIO = dispositivoToString(insActual->paramIO);

			if (!strcmp("DISCO", dispositivoIO))
			{
				printf("\nentre a ejecutar disco");
				pasar_a_block_disco(pcb);
				pthread_create(&thrBloqueoGeneralDisco, NULL, (void *)manejar_bloqueo_general_disco, (void *)insActual);
				pthread_detach(thrBloqueoGeneralDisco);
			}
			else if (!strcmp("IMPRESORA", dispositivoIO))
			{
				printf("\nentre a ejecutar impresora");
				pasar_a_block_impresora(pcb);
				pthread_create(&thrBloqueoGeneralImpresora, NULL, (void *)manejar_bloqueo_general_impresora, (void *)insActual);
				pthread_detach(thrBloqueoGeneralImpresora);
			}
			else
			{
				log_info("No exisate el dispositivo", dispositivoIO);
			}
			sem_post(&contador_pcb_running);
			// pasar_a_block(pcb);

			log_debug(logger, "Ejecutada: 'PID:  %d - Bloqueado por: %s '", pcb->id, dispositivoIO);

			break;

		case BLOCK_PCB_PAGE_FAULT:
			/*pthread_t thrBloqueoPageFault;
			dispositivoIO = dispositivoToString(insActual->paramIO);

			pasar_a_block_page_fault(pcb);

			log_debug(logger, "Ejecutada: 'PID:  %d - Bloqueado por: %s '", pcb->id, dispositivoIO);

			pthread_create(&thrBloqueoPageFault, NULL, (void *)manejar_bloqueo_page_fault, (void *)insActual);

			pthread_detach(thrBloqueoPageFault);
			sem_post(&contador_pcb_running);
			// log_debug(logger, "Ejecutada: 'PID:  %d - Bloqueado por: %s '", pcb->id, dispositivoIO);
			break;*/
		case INTERRUPT_INTERRUPCION:

			pthread_t thrInterrupt;
			log_debug(logger, "Ejecutada: 'PID:  %d - Desalojado por fin de Quantum'", pcb->id);
			// int i,j;
			printf("\nentrando a manejar interrupcion\n");
			t_tipo_algoritmo algoritmo = obtenerAlgoritmo();
			printf("\n%d\n", algoritmo);
			// t_pcb *pcb = (t_pcb *)pcbElegida;
			if (algoritmo == FEEDBACK)
			{
				printf("\npasar a ready aux");
				pasar_a_ready_auxiliar(pcb);
				sem_post(&sem_hay_pcb_lista_ready);
			}
			else if (algoritmo == RR)
			{
				printf("\nEl algoritmo obtenido es: %d\n", obtenerAlgoritmo());
				printf("\ncantidad de elementos en lista exec: %d\n", list_size(LISTA_EXEC));

				pasar_a_ready(pcb);
				printf("\ncantidad de elementos en ready: %d\n", list_size(LISTA_READY));
				sem_post(&sem_hay_pcb_lista_ready);
				printf("\ncantidad de elementos en ready: %d\n", list_size(LISTA_READY));
			}
			printf("\ntermine de manejar la interrupcion");
			// i=pthread_create(&thrInterrupt, NULL, (void *)manejar_interrupcion, (void *)pcb);
			// j=pthread_detach(thrInterrupt);
			// printf("\nse creo manejar interrupcion:%d,%d\n",i,j);
			sem_post(&contador_pcb_running);
			break;

		default:
			break;
		}
		free(paquete->buffer->stream);
		free(paquete->buffer);
		free(paquete);
		// free(dispositivoIO);
	}
}

void manejar_bloqueo_teclado(void *insActual)
{
	// sem_wait(&contador_bloqueo_teclado_running);
	t_instruccion *instActualConsola = (t_instruccion *)insActual;
	uint32_t valorRegistroTeclado;

	t_pcb *pcb = algoritmo_fifo(LISTA_BLOCKED_TECLADO);
	serializarValor(1, pcb->socket, BLOCK_PCB_IO_TECLADO);
	enviarResultado(pcb->socket, "solicito el ingreso de un valor por teclado");

	t_paqueteActual *paquete = recibirPaquete(pcb->socket);

	valorRegistroTeclado = deserializarValor(paquete->buffer, pcb->socket);
	printf("\n el valor de teclado es:%d\n", valorRegistroTeclado);
	switch (instActualConsola->paramReg[0])
	{
	case AX:
		pcb->registros.AX = valorRegistroTeclado;
		log_info(logger, "El valor del registro ingresado por consola es: %d", pcb->registros.AX);
		break;
	case BX:
		pcb->registros.BX = valorRegistroTeclado;
		log_info(logger, "El valor del registro ingresado por consola es: %d", pcb->registros.BX);
		break;
	case CX:
		pcb->registros.CX = valorRegistroTeclado;
		log_info(logger, "El valor del registro ingresado por consola es: %d", pcb->registros.CX);
		break;
	case DX:
		pcb->registros.DX = valorRegistroTeclado;
		log_info(logger, "El valor del registro ingresado por consola es: %d", pcb->registros.DX);
		break;
	}

	pasar_a_ready(pcb);
	// sem_post(&contador_bloqueo_teclado_running);
	sem_post(&sem_hay_pcb_lista_ready);
}

void manejar_bloqueo_pantalla(void *insActual)
{
	// sem_wait(&contador_bloqueo_pantalla_running);
	t_instruccion *instActualPantalla = (t_instruccion *)insActual;

	uint32_t valorRegistro;

	t_pcb *pcb = algoritmo_fifo(LISTA_BLOCKED_PANTALLA);

	printf("%d", instActualPantalla->paramReg[0]);

	switch (instActualPantalla->paramReg[0])
	{
	case AX:
		valorRegistro = pcb->registros.AX;
		printf("valor registro %d", valorRegistro);

		break;
	case BX:
		valorRegistro = pcb->registros.BX;
		printf("valor registro %d", valorRegistro);
		break;
	case CX:
		valorRegistro = pcb->registros.CX;
		printf("valor registro %d", valorRegistro);
		break;
	case DX:
		valorRegistro = pcb->registros.DX;
		printf("valor registro %d", valorRegistro);
		break;
	}

	//  Serializamos valor registro y se envia a la consola
	log_info(logger, "El valor del registro que se muestra por pantalla es: %d", valorRegistro);
	;
	serializarValor(valorRegistro, pcb->socket, BLOCK_PCB_IO_PANTALLA);
	char *mensaje = recibirMensaje(pcb->socket);
	log_info(logger, "Me llego el mensaje: %s\n", mensaje);

	pasar_a_ready(pcb);
	// sem_post(&contador_bloqueo_pantalla_running);
	sem_post(&sem_hay_pcb_lista_ready);
}

void manejar_bloqueo_general_impresora(void *insActual)
{
	sem_wait(&contador_bloqueo_impresora_running);
	t_instruccion *instActualBloqueoGeneral = (t_instruccion *)insActual;

	char *dispositivoCpu = dispositivoToString(instActualBloqueoGeneral->paramIO);

	int tamanio = size_char_array(configKernel.dispositivosIO);
	uint32_t tiempoIO;
	uint32_t duracionUnidadDeTrabajo;
	for (int i = 0; i < tamanio; i++)
	{
		if (!strcmp(configKernel.dispositivosIO[i], dispositivoCpu))
		{
			tiempoIO = atoi(configKernel.tiemposIO[i]);

			duracionUnidadDeTrabajo = tiempoIO * instActualBloqueoGeneral->paramInt;

			log_info(logger, "Ejecutando el dispositivo %s", dispositivoCpu);
			log_info(logger, "Por un tiempo de: %d", duracionUnidadDeTrabajo);

			t_pcb *pcb = algoritmo_fifo(LISTA_BLOCKED_IMPRESORA);
			usleep(duracionUnidadDeTrabajo * 1000);
			pasar_a_ready(pcb);
			sem_post(&sem_hay_pcb_lista_ready);
			break;
		}
	}

	free(dispositivoCpu);
	sem_post(&contador_bloqueo_impresora_running);
}
void manejar_bloqueo_general_disco(void *insActual)
{
	sem_wait(&contador_bloqueo_disco_running);
	t_instruccion *instActualBloqueoGeneral = (t_instruccion *)insActual;

	char *dispositivoCpu = dispositivoToString(instActualBloqueoGeneral->paramIO);

	int tamanio = size_char_array(configKernel.dispositivosIO);
	uint32_t tiempoIO;
	uint32_t duracionUnidadDeTrabajo;
	for (int i = 0; i < tamanio; i++)
	{
		if (!strcmp(configKernel.dispositivosIO[i], dispositivoCpu))
		{

			tiempoIO = atoi(configKernel.tiemposIO[i]);

			duracionUnidadDeTrabajo = tiempoIO * instActualBloqueoGeneral->paramInt;

			log_info(logger, "Ejecutando el dispositivo %s", dispositivoCpu);
			log_info(logger, "Por un tiempo de: %d", duracionUnidadDeTrabajo);

			t_pcb *pcb = algoritmo_fifo(LISTA_BLOCKED_DISCO);
			usleep(duracionUnidadDeTrabajo * 1000);
			pasar_a_ready(pcb);
			sem_post(&sem_hay_pcb_lista_ready);
			break;
		}
	}

	free(dispositivoCpu);
	sem_post(&contador_bloqueo_disco_running);
}
void manejar_bloqueo_page_fault(void *insActual)
{
	// sem_wait(&contador_bloqueo_teclado_running);
	/*t_instruccion *instActualConsola = (t_instruccion *)insActual;

	t_pcb *pcb = algoritmo_fifo(LISTA_BLOCK_PAGE_FAULT);


	t_paqueteActual *paquete = recibirPaquete(pcb->socket);


	pasar_a_ready(pcb);
	// sem_post(&contador_bloqueo_teclado_running);
	sem_post(&sem_hay_pcb_lista_ready);*/
}

void manejar_interrupcion(void *pcbElegida)
{
	printf("\nentrando a manejar interrupcion\n");
	t_tipo_algoritmo algoritmo = obtenerAlgoritmo();
	printf("\n%d\n", algoritmo);
	t_pcb *pcb = (t_pcb *)pcbElegida;
	if (algoritmo == FEEDBACK)
	{
		printf("\npasar a ready aux");
		pasar_a_ready_auxiliar(pcb);
		sem_post(&sem_hay_pcb_lista_ready);
	}
	else if (algoritmo == RR)
	{
		printf("\nEl algoritmo obtenido es: %d\n", obtenerAlgoritmo());
		printf("\ncantidad de elementos en lista exec: %d\n", list_size(LISTA_EXEC));

		pasar_a_ready(pcb);
		printf("\ncantidad de elementos en ready: %d\n", list_size(LISTA_READY));
		sem_post(&sem_hay_pcb_lista_ready);
		printf("\ncantidad de elementos en ready: %d\n", list_size(LISTA_READY));
	}
	printf("\ntermine de manejar la interrupcion");
}

void conectar_interrupt()
{
	conexionInterrupt = crear_conexion(configKernel.ipCPU, configKernel.puertoCPUInterrupt);

	while (1)
	{
		sem_wait(&sem_desalojar_pcb);
		printf("\n desalojo pcb\n");
		enviarResultado(conexionInterrupt, "interrupcion de la instruccion");
	}
}

void conectar_memoria()
{
	conexionMemoria = crear_conexion(configKernel.ipMemoria, configKernel.puertoMemoria);
	enviarResultado(conexionMemoria, "hola memoria soy el kernel");
}

void iniciar_kernel()
{

	// Parte Server
	logger = iniciar_logger("kernel.log", "KERNEL", LOG_LEVEL_DEBUG);

	config = iniciar_config("kernel.config");

	// creo el struct
	extraerDatosConfig(config);

	iniciar_listas_y_semaforos();

	contadorIdPCB = 1;
	contadorIdSegmento = 0;
	hayTimer = false;
}

void crear_pcb(void *argumentos)
{
	log_info(logger, "Consola conectada, paso a crear el hilo");
	t_args_pcb *args = (t_args_pcb *)argumentos;
	t_pcb *pcb = malloc(sizeof(t_pcb));

	pcb->socket = args->socketCliente;
	pcb->program_counter = 0;
	pcb->informacion = &(args->informacion);
	pcb->registros.AX = 0;
	pcb->registros.BX = 0;
	pcb->registros.CX = 0;
	pcb->registros.DX = 0;
	pcb->tablaSegmentos = list_create();

	for (int i = 0; i < list_size(pcb->informacion->segmentos); i++)
	{
		t_tabla_segmantos *tablaSegmento = malloc(sizeof(t_tabla_segmantos));
		uint32_t segmento = list_get(pcb->informacion->segmentos, i);

		tablaSegmento->tamanio = segmento;

		pthread_mutex_lock(&mutex_ID_Segmnento);
		tablaSegmento->id = contadorIdSegmento;
		contadorIdSegmento++;
		pthread_mutex_unlock(&mutex_ID_Segmnento);

		list_add(pcb->tablaSegmentos, tablaSegmento);
	}

	// printf("\nsocket del pcb: %d", pcb->socket);

	pthread_mutex_lock(&mutex_creacion_ID);
	pcb->id = contadorIdPCB;
	contadorIdPCB++;
	pthread_mutex_unlock(&mutex_creacion_ID);

	pasar_a_new(pcb);
	log_debug(logger, "Estado Actual: NEW , proceso id: %d", pcb->id);
	log_info(logger, "Cant de elementos de new: %d", list_size(LISTA_NEW));

	sem_post(&sem_agregar_pcb);
}

char *dispositivoToString(t_IO dispositivo)
{
	char *string = string_new();
	switch (dispositivo)
	{
	case DISCO:
		string_append(&string, "DISCO");
		return string;
		break;
	case TECLADO:
		string_append(&string, "TECLADO");
		return string;
		break;
	case PANTALLA:
		string_append(&string, "PANTALLA");
		return string;
		break;
	case IMPRESORA:
		string_append(&string, "IMPRESORA");
		return string;
		break;
	default:
		log_error(logger, "No existe el dispositivo");
		break;
	}
}

void planifLargoPlazo()
{
	while (1)
	{
		sem_wait(&sem_agregar_pcb);
		agregar_pcb();
	}
}

void planifCortoPlazo()
{
	while (1)
	{
		sem_wait(&sem_hay_pcb_lista_ready);
		log_info(logger, "Llego pcb a plani corto plazo");
		t_tipo_algoritmo algoritmo = obtenerAlgoritmo();

		sem_wait(&contador_pcb_running);

		switch (algoritmo)
		{
		case FIFO:
			log_debug(logger, "Implementando algoritmo FIFO");
			log_debug(logger, " Cola Ready FIFO:");
			cargarListaReadyIdPCB(LISTA_READY);
			implementar_fifo();

			break;
		case RR:
			log_debug(logger, "Implementando algoritmo RR");
			log_debug(logger, " Cola Ready RR:");
			cargarListaReadyIdPCB(LISTA_READY);
			implementar_rr();

			break;
		case FEEDBACK:
			log_debug(logger, "Implementando algoritmo FEEDBACK");
			implementar_feedback();

			break;

		default:
			break;
		}
	}
}

void cargarListaReadyIdPCB(t_list *listaReady)
{

	for (int i = 0; i < list_size(listaReady); i++)
	{
		t_pcb *pcb = list_get(listaReady, i);
		log_debug(logger, " '[ %d ] '", pcb->id);
	}
}

void agregar_pcb()
{
	sem_wait(&contador_multiprogramacion);

	log_info(logger, "Agregando un pcb a lista ready");

	pthread_mutex_lock(&mutex_lista_new);
	t_pcb *pcb = algoritmo_fifo(LISTA_NEW);
	printf("Cant de elementos de new: %d\n", list_size(LISTA_NEW));
	pthread_mutex_unlock(&mutex_lista_new);

	// solicito que memoria inicialice sus estructuras
	serializarPCB(conexionMemoria, pcb, ASIGNAR_RECURSOS);
	printf("\nEnvio recursos a memoria\n");
	// memoria me devuelve el pcb modificado
	t_paqueteActual *paquete = recibirPaquete(conexionMemoria);
	printf("\nRecibo recursos de memoria\n");

	
	//aca esta el problema 
	pcb = deserializoPCB(paquete->buffer);

	for (int i = 0; i < list_size(pcb->tablaSegmentos); i++)
	{
		t_tabla_segmantos *tablaSegmento = malloc(sizeof(t_tabla_segmantos));

		t_tabla_segmantos *segmento = list_get(pcb->tablaSegmentos, i);
		printf("\nel id del segmento es: %d\n", segmento->id);

		printf("\nel id de la tabla es: %d\n", segmento->indiceTablaPaginas);
	}

	pasar_a_ready(pcb);

	log_debug(logger, "Estado Anterior: NEW , proceso id: %d", pcb->id);
	log_debug(logger, "Estado Actual: READY , proceso id: %d", pcb->id);

	printf("Cant de elementos de ready: %d\n", list_size(LISTA_READY));

	sem_post(&sem_hay_pcb_lista_ready);

	log_info(logger, "Envie a memoria los recursos para asignar");
}

void eliminar_pcb()
{
	pthread_mutex_lock(&mutex_lista_exec);
	t_pcb *pcb = algoritmo_fifo(LISTA_EXEC);
	pthread_mutex_unlock(&mutex_lista_exec);

	// solicito que memoria libere sus estructuras
	// serializarPCB(conexionMemoria, pcb, LIBERAR_RECURSOS);

	// memoria me devuelve el pcb modificado
	// t_paqueteActual *paquete = recibirPaquete(conexionMemoria);

	// pcb = deserializoPCB(paquete->buffer);

	pasar_a_exit(pcb);

	sem_post(&contador_pcb_running);
	log_debug(logger, "Estado Anterior: EXEC , proceso id: %d", pcb->id);
	log_debug(logger, "Estado, proceso Actual: EXIT  id: %d", pcb->id);

	for (int i = 0; i < list_size(LISTA_EXIT); i++)
	{
		t_pcb *pcb = list_get(LISTA_EXIT, i);
		log_debug(logger, "Procesos finalizados: %d", pcb->id);
	}

	sem_post(&contador_multiprogramacion);
}