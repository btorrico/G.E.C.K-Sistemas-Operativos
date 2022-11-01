#include "kernel.h"

int main(int argc, char **argv)
{

	if (argc > 1 && strcmp(argv[1], "-test") == 0)
		return run_tests();
	else
	{
		iniciar_kernel();

		crear_hilos_kernel();
	}
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
	pthread_t thrCpu, thrMemoria, thrPlanificadorLargoPlazo, thrPlanificadorCortoPlazo, thrBloqueo;

	// pthread_create(&thrConsola, NULL, (void *)crear_hilo_consola, NULL);
	pthread_create(&thrCpu, NULL, (void *)crear_hilo_cpu, NULL);
	pthread_create(&thrMemoria, NULL, (void *)conectar_memoria, NULL);
	pthread_create(&thrPlanificadorLargoPlazo, NULL, (void *)planifLargoPlazo, NULL);
	pthread_create(&thrPlanificadorCortoPlazo, NULL, (void *)planifCortoPlazo, NULL);
	pthread_create(&thrBloqueo, NULL, (void *)controlBloqueo, NULL);

	pthread_detach(&thrCpu);
	pthread_detach(&thrPlanificadorCortoPlazo);
	pthread_detach(&thrMemoria);
	pthread_detach(&thrPlanificadorLargoPlazo);
	pthread_detach(&thrBloqueo);
	crear_hilo_consola();
	// pthread_join(thrConsola, NULL); // falta que consola funcione con detach

	log_destroy(logger);
	config_destroy(config);
}

void crear_hilo_consola()
{
	// conectar_y_mostrar_mensajes_de_cliente(IP_SERVER, configKernel.puertoEscucha, logger);
	int server_fd = iniciar_servidor(IP_SERVER, configKernel.puertoEscucha); // socket(), bind() listen()
	log_info(logger, "Servidor listo para recibir al cliente");

	while (1)
	{
		pthread_t hilo_atender_consola;
		t_args_pcb argumentos;
		// esto se podria cambiar como int* cliente_fd= malloc(sizeof(int)); si lo ponemos, va antes del while
		argumentos.socketCliente = esperar_cliente(server_fd);
		// aca hay un log que dice que se conecto un cliente
		// log_info(logger, "Consola conectada, paso a crear el hilo");

		int cod_op = recibir_operacion(argumentos.socketCliente);

		log_info(logger, "Llegaron las instrucciones y los segmentos");
		argumentos.informacion = recibir_informacion(argumentos.socketCliente);
		printf("\ncreo el hilo consola\n");

		enviarResultado(argumentos.socketCliente, "Quedate tranqui Consola, llego todo lo que mandaste ;)\n");

		pthread_create(&hilo_atender_consola, NULL, (void *)crear_pcb2, (void *)&argumentos);
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
	conexion = crear_conexion(configKernel.ipCPU, configKernel.puertoCPUDispatch);

	while (1)
	{
		sem_wait(&sem_pasar_pcb_running);
		printf("Llego un pcb a dispatch");
		serializarPCB(conexion, list_get(LISTA_EXEC, 0), DISPATCH_PCB);
		printf("\nse envio pcb a cpu\n");
		void *pcbAEliminar = list_remove(LISTA_EXEC, 0);
		free(pcbAEliminar);

		// void list_clean(t_list *); ver si se puede usar este
		// o este void list_replace_and_destroy_element(t_list*, int index, void* element, void(*element_destroyer)(void*));

		// Recibir PCB
		printf("\nRecibi de nuevo el pcb\n");
		t_paqueteActual *paquete = recibirPaquete(conexion);
		printf("\nestoy en %d: ", paquete->codigo_operacion);
		t_pcb *pcb = deserializoPCB(paquete->buffer);
		printf("\n Id proceso nuevo que llego de cpu: %d", pcb->id);
		printf("\nestoy en %d: ", paquete->codigo_operacion);

		// t_instruccion *insActual = list_get(pcb->informacion->instrucciones, pcb->program_counter);
		uint32_t valorRegistro;
		t_instruccion *instruccion = malloc(sizeof(t_instruccion));
		switch (paquete->codigo_operacion)
		{
		case EXIT_PCB:
			printf("\nestoy en %d: ", paquete->codigo_operacion);
			pasar_a_exec(pcb);
			eliminar_pcb();
			serializarValor(valorRegistro, pcb->socket, TERMINAR_CONSOLA);
			break;

		case BLOCK_PCB_IO_PANTALLA:
			do
			{
				sem_post(&contador_pcb_running);
				pasar_a_block_pantalla(pcb);
				pcb = algoritmo_fifo(LISTA_BLOCKED_PANTALLA);
				instruccion->instCode = 4;
				instruccion->paramInt = -1;
				instruccion->paramIO = PANTALLA;
				instruccion->paramReg[0] = 1;
				// instruccion->paramReg[1] = -1;
				//  switch (insActual->paramReg[0])
				switch (instruccion->paramReg[0])
				{
				case AX:
					valorRegistro = pcb->registros.AX;
					break;
				case BX:
					valorRegistro = pcb->registros.BX;
					break;
				case CX:
					valorRegistro = pcb->registros.CX;
					break;
				case DX:
					valorRegistro = pcb->registros.DX;
					break;
				}

				valorRegistro = 82;
				// Serializamos valor registro y se envia a la consola
				serializarValor(valorRegistro, pcb->socket, BLOCK_PCB_IO_PANTALLA);
				char *mensaje = recibirMensaje(pcb->socket);
				log_info(logger, "Me llego el mensaje: %s\n", mensaje);

				pasar_a_ready(pcb);
				sem_post(&sem_hay_pcb_lista_ready);
			} while (!list_is_empty(LISTA_BLOCKED_PANTALLA));

			/// esto va para cuando discriminemos que tipo de dispositivo es, y si se encuentra en el configKernel, si si no esta ver si lo mandamos a error
			// sem_post(&sem_kill_trhread); //no se si funca, probar

			break;

		case BLOCK_PCB_IO_TECLADO:
			
		do{
				sem_post(&contador_pcb_running);
				pasar_a_block_teclado(pcb);
				pcb = algoritmo_fifo(BLOCK_PCB_IO_TECLADO);
				serializarValor(1, pcb->socket, BLOCK_PCB_IO_TECLADO);
				enviarResultado(pcb->socket, "solicito el ingreso de un valor por teclado");

				paquete = recibirPaquete(pcb->socket);

				uint32_t valorRegistro = deserializarValor(paquete->buffer, pcb->socket);

				instruccion->instCode = 4;
				instruccion->paramInt = -1;
				instruccion->paramIO = TECLADO;
				instruccion->paramReg[0] = 0;

				switch (instruccion->paramReg[0])
				{
				case AX:
					pcb->registros.AX = valorRegistro;
					break;
				case BX:
					pcb->registros.BX = valorRegistro;
					break;
				case CX:
					pcb->registros.CX = valorRegistro;
					break;
				case DX:
					pcb->registros.DX = valorRegistro;
					break;
				}

				printf("\nEl valor del registro es: %d", pcb->registros.AX);

				pasar_a_ready(pcb);

				sem_post(&sem_hay_pcb_lista_ready);
			} while (!list_is_empty(LISTA_BLOCKED_TECLADO));

			break;

		case BLOCK_PCB_IO:
			
		do{
			sem_post(&contador_pcb_running);
			instruccion->paramIO = 4;
			char *dispositivoCpu = dispositivoToString(instruccion->paramIO);
			

			int tamanio = string_length(configKernel.dispositivosIO);
			int tiempoIO;
			int duracionUnidadDeTrabajo;
			for (int i = 0; i < tamanio; i++)
			{
				if (!strcmp(configKernel.dispositivosIO[i], dispositivoCpu))
				{

					tiempoIO = atoi(configKernel.tiemposIO[i]) ;
					duracionUnidadDeTrabajo= (tiempoIO/1000) * instruccion->paramInt;

					pasar_a_block(pcb);
					
					sleep(duracionUnidadDeTrabajo);

					pcb = algoritmo_fifo(LISTA_BLOCKED);

					pasar_a_ready(pcb);
					sem_post(&sem_hay_pcb_lista_ready);
					break;
				}
				else
				{
					log_error(logger, "No existe este dispositivo de IO: %s", dispositivoCpu);
				}
				
			}

			free(dispositivoCpu);
		} while (!list_is_empty(LISTA_BLOCKED_TECLADO));
		

			break;

		case BLOCK_PCB_PAGE_FAULT:
			// TODO
			break;
		case FIN_QUANTUM:
			sem_post(&contador_pcb_running);
			if (obtenerAlgoritmo() == FEEDBACK)
			{
				pasar_a_ready_auxiliar(pcb);
				sem_post(&sem_llamar_feedback);
			}
			else if (obtenerAlgoritmo() == RR)
			{
				pasar_a_ready(pcb);
				sem_post(&sem_hay_pcb_lista_ready);
			}

			break;

		default:
			break;
		}
	}
}

void conectar_interrupt()
{
	sem_wait(&sem_desalojar_pcb);
	conexion = crear_conexion(configKernel.ipCPU, configKernel.puertoCPUInterrupt);

	printf("\n desalojo pcb\n");
	enviar_mensaje("Se envio interrupcion", conexion);
}

void conectar_memoria()
{
	conexionMemoria = crear_conexion(configKernel.ipMemoria, configKernel.puertoMemoria);
	// enviar_mensaje("hola memoria, soy el kernel", conexionMemoria);
	enviarResultado(conexionMemoria, "Hola memoria soy el kernel");
}

void iniciar_kernel()
{

	// Parte Server
	logger = iniciar_logger("kernel.log", "KERNEL", LOG_LEVEL_DEBUG);

	config = iniciar_config("kernel.config");

	// creo el struct
	extraerDatosConfig(config);

	iniciar_listas_y_semaforos();

	contadorIdPCB = 0;
}

void crear_pcb2(void *argumentos)
{
	log_info(logger, "Consola conectada, paso a crear el hilo");
	t_args_pcb *args = (t_args_pcb *)argumentos;
	// t_args_pcb args;
	t_pcb *pcb = malloc(sizeof(t_pcb));

	pcb->socket = args->socketCliente;
	pcb->program_counter = 0;
	pcb->informacion = &(args->informacion);
	pcb->registros.AX = 0;
	pcb->registros.BX = 0;
	pcb->registros.CX = 0;
	pcb->registros.DX = 0;

	printf("\nsocket del pcb: %d", pcb->socket);

	/*
		printf("Instrucciones:");
		for (int i = 0; i < pcb->informacion->instrucciones_size; ++i)
		{
			t_instruccion *instruccion = list_get(pcb->informacion->instrucciones, i);

			printf("\ninstCode: %d, Num: %d, RegCPU[0]: %d,RegCPU[1] %d, dispIO: %d",
				   instruccion->instCode, instruccion->paramInt, instruccion->paramReg[0], instruccion->paramReg[1], instruccion->paramIO);
		}

		// enviarResultado(pcb->socket, "Quedate tranqui Consola, llego todo lo que mandaste ;)\n");
		printf("\n\nSegmentos:");

		printf("\n[%d,%d,%d,%d]\n", list_get(pcb->informacion->segmentos, 0), list_get(pcb->informacion->segmentos, 1), list_get(pcb->informacion->segmentos, 2), list_get(pcb->informacion->segmentos, 3));
	*/
	pthread_mutex_lock(&mutex_creacion_ID);
	pcb->id = contadorIdPCB;
	contadorIdPCB++;
	pthread_mutex_unlock(&mutex_creacion_ID);

	pasar_a_new(pcb);
	log_debug(logger, "Estado Actual: NEW , proceso id: %d", pcb->id);

	printf("Cant de elementos de new: %d\n", list_size(LISTA_NEW));

	sem_post(&sem_agregar_pcb);
	// sem_p//ost(&sem_hay_pcb_lista_new);
	// sem_t semaforo[1];
	// sem_post(&sem_planif_largo_plazo);//podemos sacarlo
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