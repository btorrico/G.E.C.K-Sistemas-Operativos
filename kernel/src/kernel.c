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
	pthread_t thrConsola, thrCpu, thrMemoria, thrPlanificadorLargoPlazo, thrPlanificadorCortoPlazo;

	pthread_create(&thrConsola, NULL, (void *)crear_hilo_consola, NULL);
	pthread_create(&thrCpu, NULL, (void *)crear_hilo_cpu, NULL);
	pthread_create(&thrMemoria, NULL, (void *)conectar_memoria, NULL);
	pthread_create(&thrPlanificadorLargoPlazo, NULL, (void *)planifLargoPlazo, NULL);
	pthread_create(&thrPlanificadorCortoPlazo, NULL, (void *)planifCortoPlazo, NULL);

	pthread_detach(&thrCpu);
	pthread_detach(&thrPlanificadorCortoPlazo);
	pthread_detach(&thrMemoria);
	pthread_detach(&thrPlanificadorLargoPlazo);

	pthread_join(thrConsola, NULL); // falta que consola funcione con detach

	log_destroy(logger);
	config_destroy(config);
}

void crear_hilo_consola()
{

	conectar_y_mostrar_mensajes_de_cliente(IP_SERVER, configKernel.puertoEscucha, logger);
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
		printf("Llego UN pcb a dispatch");
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

		t_instruccion *insActual = list_get(pcb->informacion->instrucciones, pcb->program_counter);
		uint32_t valorRegistro;
		switch (paquete->codigo_operacion)
		{
		case EXIT_PCB:
			printf("\nestoy en %d: ", paquete->codigo_operacion);
			pasar_a_exec(pcb);

			sem_post(&sem_eliminar_pcb);

			break;

		case BLOCK_PCB_IO_PANTALLA:
			t_instruccion *instruccion = malloc(sizeof(t_instruccion));
			instruccion->instCode = 4;
			instruccion->paramInt = -1;
			instruccion->paramIO = PANTALLA;
			instruccion->paramReg[0] = 1;
			instruccion->paramReg[1] = -1;
			// switch (insActual->paramReg[0])
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

			// Serializamos valor registro y se envia a la consola
			printf("\nserializo valor para consola \n%d", valorRegistro);
			serializarValor(valorRegistro, pcb->socket);

			/// esto va para cuando discriminemos que tipo de dispositivo es, y si se encuentra en el configKernel, si si no esta ver si lo mandamos a error
			// sem_post(&sem_kill_trhread); //no se si funca, probar

			break;

		case BLOCK_PCB_IO_TECLADO:

			// deserializar lo que me manda consola
			// valorRegistro = deserializarValor(paquete->buffer,pcb->socket);
			valorRegistro = 0;

			switch (insActual->paramReg[1])
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
			pasar_a_ready(pcb);
			printf("\nse envio el pcb a redy\n");
			sem_post(&sem_hay_pcb_lista_ready);

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
	enviar_mensaje("hola memoria, soy el kernel", conexionMemoria);
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
