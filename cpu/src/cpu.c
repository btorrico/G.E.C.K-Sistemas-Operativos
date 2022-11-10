#include "cpu.h"

int main(char argc, char **argv)
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

		pthread_join(thrDispatchKernel,NULL);
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
	int server_fd = iniciar_servidor(IP_SERVER, configCPU.puertoEscuchaDispatch); // socket(), bind(), listen()
	log_info(logger, "Servidor listo para recibir al dispatch kernel");

	socketAceptadoDispatch = esperar_cliente(server_fd);
	printf("\nMe quedo esperando..\n");

	while (1)
	{
		t_paqueteActual *paquete = recibirPaquete(socketAceptadoDispatch);
		printf("\nMe quedo esperando6..\n");
		/*if (paquete == NULL)
		{
			continue;
		}*/
		interrupciones = false;
		retornePCB = false;
		t_pcb *pcb = deserializoPCB(paquete->buffer);
		free(paquete->buffer->stream);
		free(paquete->buffer);
		free(paquete);

		printf("se recibio pcb de running de kernel\n");

		printf("\n%d.\n", pcb->id);
		printf("\n%d.\n", pcb->program_counter);

		// imprimirInstruccionesYSegmentos(pcb->informacion);

		printf("\n%d.\n", pcb->socket);

		printf("\n%d.\n", pcb->registros.AX);

		while (!interrupciones && !retornePCB)
		{

			retornePCB = cicloInstruccion(pcb);

			checkInterrupt(pcb, retornePCB);
		}
		printf("\nSali del while infinito\n");
	}
}

void iniciar_servidor_interrupt()
{
	int server_fd = iniciar_servidor(IP_SERVER, configCPU.puertoEscuchaInterrupt);
	log_info(logger, "Servidor listo para recibir al interrupt kernel");

	int cliente_fd = esperar_cliente(server_fd);
	while (1)
	{
		// mostrar_mensajes_del_cliente(cliente_fd);
		char *mensaje = recibirMensaje(cliente_fd);

		log_info(logger, "Me llego el mensaje: %s\n", mensaje);

		interrupciones = true;
	}
}

void conectar_memoria()
{
	conexion = crear_conexion(configCPU.ipMemoria, configCPU.puertoMemoria);
	// enviar_mensaje("hola memoria, soy el cpu", conexion);
	enviarResultado(conexion, "hola memoria soy el cpu");
}

bool cicloInstruccion(t_pcb *pcb)
{
	t_list *instrucciones = pcb->informacion->instrucciones;
	t_instruccion *insActual = list_get(instrucciones, pcb->program_counter);
	log_info(logger, "insActual->instCode: %i", insActual->instCode);

	// decode
	if (insActual->instCode == MOV_IN || insActual->instCode == MOV_OUT)
	{
		log_debug(logger, "Requiere acceso a Memoria");
		// Hacer algo en proximo Checkpoint
	}

	// fetch
	fetch(pcb);

	// execute
	char *instruccion = string_new();
	string_append(&instruccion, instruccionToString(insActual->instCode));
	char *io = string_new();
	string_append(&io, ioToString(insActual->paramIO));
	char *registro = string_new();
	string_append(&registro, registroToString(insActual->paramReg[0]));
	char *registro2 = string_new();
	string_append(&registro2, registroToString(insActual->paramReg[1]));

	log_debug(logger, "Instrucción Ejecutada: 'PID:  %i - Ejecutando: %s %s %s %s %i'",
			  pcb->id, instruccion, io, registro, registro2, insActual->paramInt); // log minimo y obligatorio
	free(instruccion);

	// interrupciones = false;
	//  bool retornePCB = false;
	switch (insActual->instCode)
	{
	case SET:
		printf(PRINT_COLOR_CYAN "\nEjecutando instruccion SET - Etapa Execute \n" PRINT_COLOR_CYAN);
		usleep(configCPU.retardoInstruccion * 1000);

		asignarValorARegistro(pcb, insActual->paramReg[0], insActual->paramInt);

		log_debug(logger, "%s = %i", registro, insActual->paramInt);
		free(registro);
		free(registro2);
		printf("estado de la interrupcion: %d", interrupciones);
		break;

	case ADD:
		printf(PRINT_COLOR_CYAN "\nEjecutando instruccion ADD - Etapa Execute \n" PRINT_COLOR_CYAN);
		usleep(configCPU.retardoInstruccion * 1000);

		uint32_t registroDestino = matchearRegistro(pcb->registros, insActual->paramReg[0]);
		uint32_t registroOrigen = matchearRegistro(pcb->registros, insActual->paramReg[1]);

		log_debug(logger, "Registro Destino -> %s = %i    &&    Registro Origen -> %s = %i \n Registro Destino = Registro Destino + Registro Origen ",
				  registro, registroDestino, registro2, registroOrigen);
		registroDestino = registroDestino + registroOrigen;
		free(registro2);

		asignarValorARegistro(pcb, insActual->paramReg[0], registroDestino);

		log_debug(logger, "%s = %i", registro, registroDestino);
		free(registro);
		break;

	case MOV_IN:
		log_debug(logger, "Leyendo valor de memoria del segmento de Datos correspondiente a la DL %i", insActual->paramInt);
		//traducir valor paramint para asignarlo despues a registroCPU
		asignarValorARegistro(pcb, insActual->paramReg[0], insActual->paramInt);

	case IO:
		printf(PRINT_COLOR_CYAN "\nEjecutando instruccion IO - Etapa Execute \n" PRINT_COLOR_CYAN);
		// pcb->program_counter += 1;
		switch (insActual->paramIO)
		{
		case TECLADO:
			serializarPCB(socketAceptadoDispatch, pcb, BLOCK_PCB_IO_TECLADO);
			log_debug(logger, "Envie BLOCK al kernel por IO_TECLADO");
			retornePCB = true;
			break;
		case PANTALLA:
			serializarPCB(socketAceptadoDispatch, pcb, BLOCK_PCB_IO_PANTALLA);
			log_debug(logger, "Envie BLOCK al kernel por IO_PANTALLA");
			retornePCB = true;
			break;
		default:
			serializarPCB(socketAceptadoDispatch, pcb, BLOCK_PCB_IO);
			log_debug(logger, "Envie BLOCK al kernel por IO");
			retornePCB = true;
			break;
		}
		free(pcb);
		break;

	case EXIT:
		printf(PRINT_COLOR_CYAN "\nEjecutando instruccion EXIT - Etapa Execute\n" PRINT_COLOR_CYAN);
		serializarPCB(socketAceptadoDispatch, pcb, EXIT_PCB);
		log_debug(logger, "Envie EXIT al kernel");
		retornePCB = true;
		free(pcb);
		printf("\nLlegue al retorno: %d\n", retornePCB);
		// limpiar_entradas_TLB();
		break;
	default:
		break;
	}

	return retornePCB;
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

	if (interrupciones && !retornePCB)
	{
		// devuelvo pcb a kernel
		log_debug(logger, "Devuelvo pcb por interrupcion");
		serializarPCB(socketAceptadoDispatch, pcb, INTERRUPT_INTERRUPCION);
		retornePCB = true;
		// interrupciones = false;
		free(pcb);
		// limpiar_entradas_TLB();
	}
	else
	{
		log_debug(logger, "No hay interrupcion, sigo el ciclo");
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

char *ioToString(t_IO io)
{
	switch (io)
	{
	case DISCO:
		return "DISCO";
		break;
	case PANTALLA:
		return "PANTALLA";
		break;
	case TECLADO:
		return "TECLADO";
		break;
	case IMPRESORA:
		return "IMPRESORA";
		break;
	default:
		return "";
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

void asignarValorARegistro(t_pcb *pcb, t_registro registro, uint32_t valor)
{
	switch (registro)
	{
	case AX:
		pcb->registros.AX = valor;
		break;
	case BX:
		pcb->registros.BX = valor;
		break;
	case CX:
		pcb->registros.CX = valor;
		break;
	case DX:
		pcb->registros.DX = valor;
	default:
		break;
	}
}
