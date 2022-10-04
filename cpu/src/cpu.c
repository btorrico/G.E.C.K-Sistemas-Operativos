#include "cpu.h"

int main(char **argc, char **argv)
{
	if (argc > 1 && strcmp(argv[1], "-test") == 0)
		return run_tests();
	else
	{

		logger = iniciar_logger("cpu.log", "CPU", LOG_LEVEL_DEBUG);

		config = iniciar_config("cpu.config");

		extraerDatosConfig(config);


		pthread_t thrDispatchKernel, thrInterruptKernel, thrMemoria;

		pthread_create(&thrDispatchKernel, NULL, (void *)iniciar_servidor_dispatch, NULL);
		pthread_create(&thrInterruptKernel, NULL, (void *)iniciar_servidor_interrupt, NULL);
		pthread_create(&thrMemoria, NULL, (void *)conectar_memoria, NULL);

		pthread_join(thrDispatchKernel, NULL);
		pthread_join(thrInterruptKernel, NULL);
		pthread_join(thrMemoria, NULL);

		log_destroy(logger);
		config_destroy(config);
	}
}
t_configCPU extraerDatosConfig(t_config *archivoConfig)
{

	configCPU.reemplazoTLB = string_new();
	configCPU.ipMemoria = string_new();
	configCPU.puertoMemoria = string_new();
	configCPU.puertoEscuchaDispatch = string_new();
	configCPU.puertoEscuchaInterrupt = string_new();

	configCPU.ipCPU = config_get_string_value(archivoConfig, "IP_CPU");

	configCPU.ipMemoria = config_get_string_value(archivoConfig, "IP_MEMORIA");
	configCPU.puertoMemoria = config_get_string_value(archivoConfig, "PUERTO_MEMORIA");
	configCPU.reemplazoTLB = config_get_string_value(archivoConfig, "REEMPLAZO_TLB");
	configCPU.puertoEscuchaDispatch = config_get_string_value(archivoConfig, "PUERTO_ESCUCHA_DISPATCH");
	configCPU.puertoEscuchaInterrupt = config_get_string_value(archivoConfig, "PUERTO_ESCUCHA_INTERRUPT");
	configCPU.retardoInstruccion = config_get_int_value(archivoConfig, "RETARDO_INSTRUCCION");
	configCPU.entradasTLB = config_get_int_value(archivoConfig, "ENTRADAS_TLB");

	return configCPU;
}

void iniciar_servidor_dispatch()
{
	int server_fd = iniciar_servidor(IP_SERVER, configCPU.puertoEscuchaDispatch); // socket(), bind()listen()
	log_info(logger, "Servidor listo para recibir al dispatch kernel");

	int cliente_fd = esperar_cliente(server_fd);
	//mostrar_mensajes_del_cliente(cliente_fd);

	t_paquete* paquete = recibirPaquete(cliente_fd);


	t_pcb* pcb=deserializoPCB(paquete->buffer);
	printf("\n%d.\n",pcb->id);
	printf("\n%s.\n",pcb->instrucciones);
    printf("se recibio pcb\n");
	
}

void iniciar_servidor_interrupt()
{
	int server_fd = iniciar_servidor(IP_SERVER, configCPU.puertoEscuchaInterrupt);
	log_info(logger, "Servidor listo para recibir al interrupt kernel");

	int cliente_fd = esperar_cliente(server_fd);
	mostrar_mensajes_del_cliente(cliente_fd);
}

void conectar_memoria(){
	conexion = crear_conexion(configCPU.ipMemoria, configCPU.puertoMemoria);
	enviar_mensaje("hola memoria, soy el cpu", conexion);

	
}