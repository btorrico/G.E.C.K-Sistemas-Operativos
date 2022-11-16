#include "memoria.h"

int main(int argc, char **argv)
{
	if (argc > 1 && strcmp(argv[1], "-test") == 0)
		return run_tests();
	else
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
	}

	log_destroy(logger);
	config_destroy(config);
}

t_configMemoria extraerDatosConfig(t_config *archivoConfig)
{
	configMemoria.puertoEscuchaUno = string_new();
	configMemoria.puertoEscuchaDos = string_new();
	configMemoria.retardoMemoria = string_new();
	configMemoria.algoritmoReemplazo = string_new();
	configMemoria.pathSwap = string_new();

	configMemoria.puertoEscuchaUno = config_get_string_value(archivoConfig, "PUERTO_ESCUCHA_UNO");
	configMemoria.puertoEscuchaDos = config_get_string_value(archivoConfig, "PUERTO_ESCUCHA_DOS");
	configMemoria.retardoMemoria = config_get_string_value(archivoConfig, "RETARDO_MEMORIA");
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
	int server_fd = iniciar_servidor(IP_SERVER, configMemoria.puertoEscuchaUno); // socket(), bind()listen()
	log_info(logger, "Servidor listo para recibir al kernel");
	//socketAceptadoKernel = esperar_cliente(server_fd);
 int cliente_fd= esperar_cliente(server_fd);
 char *mensaje = recibirMensaje(cliente_fd);
	//char *mensaje = recibirMensaje(socketAceptadoKernel);
	log_info(logger, "Mensaje de confirmacion del Kernel: %s\n", mensaje);

	/*while (1)
	{
		t_paqueteActual *paquete = recibirPaquete(socketAceptadoKernel);
		
		t_pcb *pcb = deserializoPCB(paquete->buffer);

		switch (paquete->codigo_operacion)
		{
		case PASAR_A_READY: //solicitud de inicializar las estructuras

		// enviar el índice/identificador de la tabla de páginas de cada segmento 
		//que deberán estar almacenados en la tabla de segmentos del PCB
		//serializarPCB(socketAceptadoKernel, pcb, PASAR_A_READY);

			break;
		case PASAR_A_EXIT: //solicitud de liberar las estructuras

			//liberar las estructuras y
			//enviar msj al kernel de que ya estan liberadas
			//serializarPCB(socketAceptadoKernel, pcb, PASAR_A_EXIT);
			break;
		}
	}*/
}

void iniciar_servidor_hacia_cpu()
{


	int server_fd = iniciar_servidor(IP_SERVER, configMemoria.puertoEscuchaDos); // socket(), bind()listen()
	struct sockaddr_in dir_cliente;
	socklen_t tam_direccion = sizeof(struct sockaddr_in);
	log_info(logger, "Servidor listo para recibir al cpu");

	int cliente_fd = esperar_cliente(server_fd);

	char *mensaje = recibirMensaje(cliente_fd);
	//char *mensaje = recibirMensaje(socketAceptadoKernel);
	log_info(logger, "Mensaje de confirmacion del CPU: %s\n", mensaje);
	


	int socketAceptadoCPU = 0;
	socketAceptadoCPU = accept(server_fd, (void*)&dir_cliente, &tam_direccion);

	t_paqt paqueteCPU;
	recibirMsje(socketAceptadoCPU, &paqueteCPU);


	if(paqueteCPU.header.cliente == CPU){
		log_debug(logger,"HANSHAKE se conecto CPU");

			conexionCPU(socketAceptadoCPU);
	}
    mostrar_mensajes_del_cliente(cliente_fd);

}

void conexionCPU(int socketAceptadoVoid){ // void*
	int socketAceptado = socketAceptadoVoid;
	t_paqt paquete;

	int pid;
	int pagina;

	int y = 1;
	while(y){


		recibirMsje(socketAceptado, &paquete);

		switch(paquete.header.tipoMensaje) {
			case CONFIG_DIR_LOG_A_FISICA:
				configurarDireccionesCPU(socketAceptado);
				break;
		case -1:
			
			y = 1;
			break;
			default: // TODO CHEKEAR: SI FINALIZO EL CPU ANTES QUE MEMORIA, SE PRODUCE UNA CATARATA DE LOGS. PORQUE? NO HAY PORQUE
				log_error(logger, "No se reconoce el tipo de mensaje, tas metiendo la patita");
				break;
		}


		}
}

void configurarDireccionesCPU(int socketAceptado){
	//SE ENVIAN LAS ENTRADAS_POR_TABLA y TAM_PAGINA AL CPU PARA PODER HACER LA TRADUCCION EN EL MMU
	log_debug(logger,"Se envian las ENTRADAS_POR_TABLA y TAM_PAGINA al CPU ");

	MSJ_MEMORIA_CPU_INIT* infoAcpu = malloc(sizeof(MSJ_MEMORIA_CPU_INIT));

	infoAcpu->cantEntradasPorTabla = configMemoria.entradasPorTabla;

	infoAcpu->tamanioPagina = configMemoria.tamPagina;


	//usleep(configMemoria.retardoMemoria * 1000); // CHEQUEAR, SI LO DESCOMENTAS NO PASA POR LAS OTRAS LINEAS


	enviarMsje(socketAceptado, MEMORIA, infoAcpu, sizeof(MSJ_MEMORIA_CPU_INIT), CONFIG_DIR_LOG_A_FISICA);

	free(infoAcpu);

	log_debug(logger,"Informacion de la cantidad de entradas por tabla y tamaño pagina enviada al CPU");
}
