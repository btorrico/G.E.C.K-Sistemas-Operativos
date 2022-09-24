#include "kernel.h"

int main(char** argc, char ** argv)
{
	 if(argc > 1 && strcmp(argv[1],"-test")==0)
        return run_tests();
    else{  

    //Parte Server
    logger = iniciar_logger("kernel.log","KERNEL", LOG_LEVEL_DEBUG);
	
	config = iniciar_config("kernel.config");
    
	//creo el struct
	extraerDatosConfig(config);

		pthread_t thrConsola, thrCpu;

		
		pthread_create(&thrConsola, NULL, (void *)crear_hilo_consola, NULL);
		pthread_create(&thrCpu, NULL, (void *)crear_hilo_cpu, NULL);

		pthread_join(thrConsola, NULL);
		pthread_join(thrCpu, NULL);


	//crear_hilos_consola_y_cpu(ip_consola, ip_cpu, puerto_escucha, logger);

    
	

	}	
}

t_configKernel extraerDatosConfig(t_config* archivoConfig) {
configKernel.ipMemoria = string_new();
configKernel.puertoMemoria = string_new();
	configKernel.ipCPU = string_new();
	configKernel.puertoCPUDispatch = string_new();
	configKernel.puertoCPUInterrupt = string_new();
	configKernel.puertoEscucha = string_new();
configKernel.algoritmo = string_new();


configKernel.ipKernel = string_new();

	configKernel.ipMemoria = config_get_string_value(archivoConfig, "IP_MEMORIA");
	configKernel.puertoMemoria= config_get_string_value(archivoConfig, "PUERTO_MEMORIA");
	configKernel.ipCPU=config_get_string_value(archivoConfig, "IP_CPU");
	configKernel.puertoCPUDispatch=config_get_string_value(archivoConfig, "PUERTO_CPU_DISPATCH");
	configKernel.puertoCPUInterrupt=config_get_string_value(archivoConfig, "PUERTO_CPU_INTERRUPT");
	configKernel.puertoEscucha=config_get_string_value(archivoConfig, "PUERTO_ESCUCHA");
	configKernel.algoritmo= config_get_string_value(archivoConfig, "ALGORITMO_PLANIFICACION");
	configKernel.gradoMultiprogramacion = config_get_int_value(archivoConfig, "GRADO_MAX_MULTIPROGRAMACION");
	

configKernel.ipKernel=config_get_string_value(archivoConfig, "IP_KERNEL");

	return configKernel;
}

void crear_hilo_consola(){
	
conectar_y_mostrar_mensajes_de_cliente(configKernel.ipKernel, configKernel.puertoEscucha, logger);
log_info(logger, "soy la consola");
	
}

void crear_hilo_cpu(){

	pthread_t thrDispatch, thrInterrupt;

		pthread_create(&thrDispatch, NULL, (void *)conectar_dispatch, NULL);
		pthread_create(&thrInterrupt, NULL, (void *)conectar_interrupt, NULL);

		pthread_join(thrDispatch, NULL);
		pthread_join(thrInterrupt, NULL);

	log_info(logger, "soy el cpu");
}
    
void conectar_dispatch(){
	conexion = crear_conexion(configKernel.ipCPU, configKernel.puertoCPUDispatch);
	enviar_mensaje("soy el dispatch", conexion);
}


void conectar_interrupt(){
	
	conexion = crear_conexion(configKernel.ipCPU, configKernel.puertoCPUInterrupt);
	enviar_mensaje("soy el interrupt", conexion);
}

