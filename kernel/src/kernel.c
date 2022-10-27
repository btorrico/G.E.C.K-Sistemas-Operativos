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

	sem_wait(&sem_pasar_pcb_running);
	printf("Llego UN pcb a dispatch");
	serializarPCB(conexion, list_get(LISTA_EXEC, 0), DISPATCH_PCB);
	printf("\nse envio pcb a cpu\n");

	/*
	// Recibir PCB
	printf("\nRecibi de nuevo el pcb\n");
	t_paqueteActual *paquete = recibirPaquete(conexion);

	t_pcb *pcb = deserializoPCB(paquete->buffer);

	switch (paquete->codigo_operacion)
	{
	case EXIT_PCB:
		sem_post(&sem_planif_largo_plazo);
		sem_post(&sem_eliminar_pcb);
		break;

	case BLOCK_PCB:
		sem_post(&sem_kill_trhread);
		break;
	
	case FIN_QUANTUM:
		if(obtenerAlgoritmo() == 2){
		pasar_a_ready_auxiliar(pcb);
		sem_post(&sem_llamar_feedback);

	default:
		break;
		//
		// sem_post(&contador_pcb_running);??
	}*/
}
void conectar_interrupt()
{
	sem_wait(&sem_desalojar_pcb);
	conexion = crear_conexion(configKernel.ipCPU, configKernel.puertoCPUInterrupt);

	printf("\n desalojo pcb\n");
	enviar_mensaje("Se envio interrupcion", conexionInterrupt);

	
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
