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

	socketAceptadoDispatch = esperar_cliente(server_fd);

	while (1)
	{
		t_paqueteActual *paquete = recibirPaquete(socketAceptadoDispatch);
		if (paquete == NULL)
		{
			continue;
			
		}
		t_pcb *pcb = deserializoPCB(paquete->buffer);
		// free(paquete->buffer->stream);
		// free(paquete->buffer);
		// free(paquete);

		printf("se recibio pcb de running de kernel\n");

		printf("\n%d.\n", pcb->id);
		printf("\n%d.\n", pcb->program_counter);

		imprimirInstruccionesYSegmentos(pcb->informacion);

		printf("\n%d.\n", pcb->socket);

		printf("\n%d.\n", pcb->registros.AX);

		//cicloInstruccion(pcb);

		// hacer cosas
		/*hacer_cosas_con_pcb(

			sem_post(&sem_pasar_pcb_kernel);
		)*/

		serializarPCB(socketAceptadoDispatch, pcb, BLOCK_PCB_IO);

		// printf("\nenvie pcb por bloqueado\n");
		//  sem_wait(&sem_pasar_pcb_kernel);
		//  serializarPCB(conexion, pcb, EXIT_PCB);
	}
}
void iniciar_servidor_interrupt()
{
	int server_fd = iniciar_servidor(IP_SERVER, configCPU.puertoEscuchaInterrupt);
	log_info(logger, "Servidor listo para recibir al interrupt kernel");

	int cliente_fd = esperar_cliente(server_fd);
	mostrar_mensajes_del_cliente(cliente_fd);
	interrupciones = true;
}

void conectar_memoria()
{
	conexion = crear_conexion(configCPU.ipMemoria, configCPU.puertoMemoria);
	enviar_mensaje("hola memoria, soy el cpu", conexion);
}

void cicloInstruccion(t_pcb *pcb)
{
	t_list *instrucciones = pcb->informacion->instrucciones;
	t_instruccion *insActual = list_get(instrucciones, pcb->program_counter);
	log_info(logger, "insActual->instCode: %i", insActual->instCode);

	// fetch
	fetch(pcb);

	// decode
	if (insActual->instCode == MOV_IN || insActual->instCode == MOV_OUT)
	{
		log_debug(logger, "Requiere acceso a Memoria");
		// Hacer algo en proximo Checkpoint
	}

	// execute
	char *instruccion = string_new();
	string_append(&instruccion, instruccionToString(insActual->instCode));
	char *registro = string_new();
	string_append(&registro, registroToString(insActual->paramReg[0]));
	char *registro2 = string_new();
	string_append(&registro2, registroToString(insActual->paramReg[1]));

	log_debug(logger, "Instrucción Ejecutada: 'PID:  %i - Ejecutando: %s %s %s %i'",
			  pcb->id, instruccion, registro, registro2, insActual->paramInt); // log minimo y obligatorio
	free(instruccion);

	bool retornePCB = false;
	switch (insActual->instCode)
	{
	case SET:
		// log_debug(logger,"SET");
		printf(PRINT_COLOR_RED "\nEjecutando instruccion SET - Etapa Execute \n" PRINT_COLOR_RESET);
		usleep(configCPU.retardoInstruccion);
		uint32_t registroCPU = matchearRegistro(pcb->registros, insActual->paramReg[0]);
		registroCPU = insActual->paramInt;
		log_debug(logger, "%s = %i", registro, registroCPU);
		free(registro);
		break;

	case ADD:
		// log_debug(logger,"ADD");
		printf(PRINT_COLOR_RED "\nEjecutando instruccion ADD - Etapa Execute \n" PRINT_COLOR_RESET);
		usleep(configCPU.retardoInstruccion);
		uint32_t registroDestino = matchearRegistro(pcb->registros, insActual->paramReg[0]);
		uint32_t registroOrigen = matchearRegistro(pcb->registros, insActual->paramReg[1]);

		log_debug(logger, "Registro Destino -> %s = %i \n Registro Origen -> %s = %i \n Registro Destino = Registro Destino + Resgitro Origen ",
				  registro, registroDestino, registro2, registroOrigen);
		registroDestino = registroDestino + registroOrigen;

		log_debug(logger, "Registro Destino %s =  %i", registro, registroDestino);
		// free(registro);
		// free(registro2);
		break;

	case EXIT:
		printf(PRINT_COLOR_RED "\nEjecutando instruccion EXIT - Etapa Execute\n" PRINT_COLOR_RESET);
		serializarPCB(socketAceptadoDispatch, pcb, EXIT_PCB);
		log_debug(logger, "Envie EXIT al kernel");
		retornePCB = true;
		// limpiar_entradas_TLB();
		break;
	}

	// check interrupt
	checkInterrupt(pcb, retornePCB);
}

void fetch(t_pcb *pcb)
{

	uint32_t index = pcb->program_counter;
	pcb->program_counter += 1;

	log_info(logger, "insActual->pc: %i", index);
	log_info(logger, " Valor nuevo Program counter: %i", pcb->program_counter);
}

void checkInterrupt(t_pcb *pcb, bool retornePCB)
{
	if (!interrupciones && !retornePCB)
	{
		// ejecuto ciclo de instruccion en caso de no haber interrupciones
		cicloInstruccion(pcb);
	}
	else if (interrupciones && !retornePCB)
	{
		// devuelvo pcb a kernel
		log_debug(logger, "Devuelvo pcb por interrupcion");
		serializarPCB(socketAceptadoDispatch, pcb, INTERRUPT_INTERRUPCION);
		interrupciones = false;
		// limpiar_entradas_TLB();
	}
}

char *registroToString(t_registro registroCPU)
{
	switch (registroCPU)
	{
	case AX:
		return "AX";
		break;
	case BX:
		return "BX";
		break;
	case CX:
		return "CX";
		break;
	case DX:
		return "DX";
		break;
	default:
		return "";
		break;
	}
}

char *instruccionToString(t_instCode codigoInstruccion)
{
	char *string = string_new();
	switch (codigoInstruccion)
	{
	case SET:
		string_append(&string, "SET");
		return string;
		break;
	case ADD:
		string_append(&string, "ADD");
		return string;
		break;
	case MOV_IN:
		string_append(&string, "MOV_IN");
		return string;
		break;
	case MOV_OUT:
		string_append(&string, "MOV_OUT");
		return string;
		break;
	case IO:
		string_append(&string, "IO");
		return string;
		break;
	case EXIT:
		string_append(&string, "EXIT");
		return string;
		break;

	default:
		break;
	}
}

uint32_t matchearRegistro(t_registros registros, t_registro registro)
{
	switch (registro)
	{
	case AX:
		return registros.AX;
		break;
	case BX:
		return registros.BX;
		break;
	case CX:
		return registros.CX;
		break;
	case DX:
		return registros.DX;
		break;

	default:
		break;
	}
}
