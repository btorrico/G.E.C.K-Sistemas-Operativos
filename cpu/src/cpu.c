#include "cpu.h"

int main(char argc, char **argv)
{

		logger = iniciar_logger("cpu.log", "CPU", LOG_LEVEL_DEBUG);

		config = iniciar_config("cpu.config");

		extraerDatosConfig(config);

		pthread_t thrDispatchKernel, thrInterruptKernel, thrMemoria;

		pthread_create(&thrDispatchKernel, NULL, (void *)iniciar_servidor_dispatch, NULL);
		pthread_create(&thrInterruptKernel, NULL, (void *)iniciar_servidor_interrupt, NULL);
		printf(PRINT_COLOR_BLUE "Conectando con modulo Memoria..."PRINT_COLOR_RESET "\n" );
		pthread_create(&thrMemoria, NULL, (void *)conectar_memoria, NULL);
		//pthread_create(&thrMemoria, NULL, (void *)recibir_config_memoria, NULL);
		//recibir_config_memoria();

		pthread_join(thrDispatchKernel,NULL);
		pthread_join(thrInterruptKernel, NULL);
		pthread_join(thrMemoria, NULL);

		log_destroy(logger);
		config_destroy(config);
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

	while (1)
	{
		t_paqueteActual *paquete = recibirPaquete(socketAceptadoDispatch);
		interrupciones = false;
		retornePCB = false;
		t_pcb *pcb = deserializoPCB(paquete->buffer);
		free(paquete->buffer->stream);
		free(paquete->buffer);
		free(paquete);

		printf("se recibio pcb de running de kernel\n");

		//imprimirInstruccionesYSegmentos(pcb->informacion);
		
		do
		{
			retornePCB = cicloInstruccion(pcb);

			checkInterrupt(pcb, retornePCB);
		} while (!interrupciones && !retornePCB);
		printf("\nSali del while infinito\n");
	}
}

void iniciar_servidor_interrupt()
{
	int server_fd = iniciar_servidor(NULL, configCPU.puertoEscuchaInterrupt);
	log_info(logger, "Servidor listo para recibir al interrupt kernel");

	int cliente_fd = esperar_cliente(server_fd);
	while (1)
	{
		char *mensaje = recibirMensaje(cliente_fd);
		if(mensaje == NULL){
			break;
		}
		log_info(logger, "Me llego el mensaje: %s\n", mensaje);

		interrupciones = true;
		free(mensaje);
	}
	printf("\nse desconecto interrupt\n");
}

void conectar_memoria()
{
	conexion = crear_conexion(configCPU.ipMemoria, configCPU.puertoMemoria);
	// enviar_mensaje("hola memoria, soy el cpu", conexion);
	enviarResultado(conexion, "hola memoria soy el cpu");

	log_debug(logger,"Buscando configuracion inicial de memoria");
	//socketMemoria = crear_conexion(configCPU.ipMemoria, configCPU.puertoMemoria);

	enviarMsje(conexion, CPU, NULL, 0, HANDSHAKE_INICIAL);
	log_debug(logger,"Se envio Handshake a MEMORIA");


	MSJ_INT* mensaje = malloc(sizeof(MSJ_INT));
	//mensaje->numero = 1;

	enviarMsje(conexion, CPU, mensaje, sizeof(MSJ_INT), CONFIG_DIR_LOG_A_FISICA);
	free(mensaje);
	log_debug(logger,"Esperando mensaje de memoria para config inicial");

	t_paqt paquete;

	//recibe el mensaje inicial de memoria
	recibirMsje(conexion, &paquete);

	//Reservo espacio para recibir las ENTRADAS_POR_TABLA y TAM_PAGINA
	MSJ_MEMORIA_CPU_INIT* infoDeMemoria = malloc(sizeof(MSJ_MEMORIA_CPU_INIT));

	infoDeMemoria = paquete.mensaje;


	log_debug(logger,"Se recibio la informacion de memoria: tamanio pagina= %i cantidad de Entradas por Tabla= %i", infoDeMemoria->tamanioPagina, infoDeMemoria->cantEntradasPorTabla);

	configCPU.cantidadEntradasPorTabla = infoDeMemoria->cantEntradasPorTabla;
	configCPU.tamanioPagina = infoDeMemoria->tamanioPagina;


	free(infoDeMemoria);
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
	
		t_direccionFisica* dirFisicaMoveIn = malloc(sizeof(t_direccionFisica));
			
			dirFisicaMoveIn = calcular_direccion_fisica(insActual->paramInt,configCPU.cantidadEntradasPorTabla,configCPU.tamanioPagina,pcb); // Para el calculo de la DF no necesitariamos tambien incluir el indice de la tabla de paginas como parametro?????

			MSJ_MEMORIA_CPU_LEER* mensajeAMemoriaLeer = malloc(sizeof(MSJ_MEMORIA_CPU_LEER));

			mensajeAMemoriaLeer->desplazamiento = dirFisicaMoveIn->desplazamientoPagina;
			mensajeAMemoriaLeer->nroMarco = dirFisicaMoveIn->nroMarco;
			mensajeAMemoriaLeer->pid = pcb->id;
			enviarMsje(conexionMemoria, CPU, mensajeAMemoriaLeer, sizeof(MSJ_MEMORIA_CPU_LEER), ACCESO_MEMORIA_LEER);
			log_debug(logger, "Envie direccion fisica a memoria swap: MARCO: %d, OFFSET: %d\n", mensajeAMemoriaLeer->nroMarco, mensajeAMemoriaLeer->desplazamiento);


		/* 	t_paqt paqueteMemoriaSwap;
			recibirMsje(conexionMemoria, &paqueteMemoriaSwap);
			MSJ_INT* mensajeValorLeido = malloc(sizeof(MSJ_INT));
			mensajeValorLeido = paqueteMemoriaSwap.mensaje;
			
			asignarValorARegistro(pcb, insActual->paramReg[0], mensajeValorLeido->numero);
			
			log_debug(logger, "Mensaje leido: %d", mensajeValorLeido->numero);
			log_debug(logger, "Registro %s = %i", registro, insActual->paramReg[0]); */

		//  free(dirFisicaMoveIn);
		//  free(mensajeAMemoriaLeer);
		//	free(mensajeValorLeido);
			break;

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


/************** Traduccion */
// tam_max_segmento = cant_entradas_por_tabla * tam_pagina
// num_segmento = floor(dir_logica / tam_max_segmento)
// desplazamiento_segmento = dir_logica % tam_max_segmento
// num_pagina = floor(desplazamiento_segmento  / tam_pagina)
// desplazamiento_pagina = desplazamiento_segmento % tam_pagina

t_direccionFisica* calcular_direccion_fisica(int direccionLogica,int cant_entradas_por_tabla, int tam_pagina, t_pcb *pcb){
	

	printf(PRINT_COLOR_GREEN "\n---------------------------------------------------" PRINT_COLOR_RESET);
	log_info(logger, "MMU entrando en acción...");
	log_info(logger, "Traduccion de la dirección logica");
	log_info(logger, "direccionLogica: %d", direccionLogica);
	t_direccionFisica *dir_fisica = malloc(sizeof(t_direccionFisica));

	int tamanio_maximo_segmento = tamanioMaximoPorSegmento(cant_entradas_por_tabla, tam_pagina); 
	log_info(logger, "Tamanio Maximo Por Segmento = %d * %d = %d", cant_entradas_por_tabla, tam_pagina, tamanio_maximo_segmento);

	int numero_segmento = numeroDeSegmento(direccionLogica, tamanioMaximoPorSegmento);
	log_info(logger, "Número de Segmento = %d / %d = %d", direccionLogica,tamanio_maximo_segmento, numero_segmento);

	int desplazamiento_Segmento = desplazamientoSegmento( direccionLogica, tamanio_maximo_segmento);
	log_info(logger, "Desplazamiento Segmento = %d ·/. %d = %d", direccionLogica,tamanio_maximo_segmento, desplazamiento_Segmento);

	int numero_pagina= numeroPagina(desplazamiento_Segmento, tam_pagina);  // numero_pagina CAMBIAR A UINT_32 ??? o esta bien en int?? la misma pregunta por todos los demas int
	log_info(logger, "Número Pagina = %d / %d = %d", desplazamiento_Segmento, tam_pagina, numero_pagina);

	int desplazamiento_pagina = desplazamientoPagina(desplazamiento_Segmento, tam_pagina);
	log_info(logger, "Desplazamiento Pagina = %d ·/. %d = %d", desplazamiento_Segmento, tam_pagina,desplazamiento_pagina);

	
	printf(PRINT_COLOR_GREEN "---------------------------------------------------\n" PRINT_COLOR_RESET);

    


	t_tabla_segmentos *segmento = malloc(sizeof(t_tabla_segmentos)); //hacer el free(segmento);
	segmento = list_get(pcb->tablaSegmentos, numero_segmento);
	printf("\nel id del segmento es: %d\n", segmento->id);

	printf("\nel id de la tabla es: %d\n", segmento->indiceTablaPaginas);
 
	int nroMarco = -1;//buscar_en_TLB(numero_pagina);

	//1ero Chequear SEGMENTATION FAULT
	printf(PRINT_COLOR_MAGENTA "Chequeando que no haya SEGMENTATION FAULT \n"PRINT_COLOR_MAGENTA);
	printf( "desplazamiento_Segmento:%d > segmento->tamanio: %d ???\n", desplazamiento_Segmento, segmento->tamanio);

	if(desplazamiento_Segmento > segmento->tamanio){ // Uso el tamanio real
		//Devolvemos el pcb a nuestro bello kernel
		serializarPCB(socketAceptadoDispatch, pcb, SEGMENTATION_FAULT);
		log_debug(logger, "Envie de Nuevo el proceso para ser finalizado...");

	} else if(nroMarco != -1){ // significa que La PAGINA ESTA EN LA TLB
		// Direccion fisica = Numero de marco * tamaño de marco + offset
		//dirFisica = malloc(sizeof(t_direccionFisica));
		dir_fisica->nroMarco = nroMarco;
		dir_fisica->desplazamientoPagina = desplazamiento_pagina;
	} else if(nroMarco == -1){ // COMO LA PAG NO ESTA EN LA TLB, TRADUCIR DIR CON MMU -> TLB MISS

		int respuestaMemoriaPrimerAcceso = primer_acceso(numero_pagina, segmento->indiceTablaPaginas);
		if(respuestaMemoriaPrimerAcceso==-1){ //respuesta PAGE FAULT
			//Devolvemos el pcb a nuestro bello kernel
			MSJ_CPU_KERNEL_BLOCK_PAGE_FAULT *mensajeAKernelPageFault = malloc(sizeof(MSJ_CPU_KERNEL_BLOCK_PAGE_FAULT));
			mensajeAKernelPageFault->nro_pagina = numero_pagina;
			mensajeAKernelPageFault->nro_segmento = numero_segmento;
			pcb->program_counter--;
			//mensajeAKernelPageFault->pcb =pcb;
			enviarMsje(socketAceptadoDispatch, CPU, mensajeAKernelPageFault, sizeof(MSJ_CPU_KERNEL_BLOCK_PAGE_FAULT), BLOCK_PCB_PAGE_FAULT);
			serializarPCB(socketAceptadoDispatch, pcb, BLOCK_PCB_PAGE_FAULT);
			log_debug(logger, "Envie de Nuevo el proceso a Kernel sin actualizar Program Counter (para bloquear por PAGE FAULT)");

		} else {
			dir_fisica->nroMarco = respuestaMemoriaPrimerAcceso;
			dir_fisica->desplazamientoPagina = desplazamiento_pagina; //Checkear
	
			log_debug(logger, "El valor del marco es: %d", dir_fisica->nroMarco);
			log_debug(logger, "El valor del offset es: %d", dir_fisica->desplazamientoPagina);
			actualizar_TLB(numero_pagina, dir_fisica->nroMarco, numero_segmento, pcb->id);
		}
	}
	
	return dir_fisica;

}

int tamanioMaximoPorSegmento(int cant_entradas_por_tabla, int tam_pagina){
	int tam_max_segmento = cant_entradas_por_tabla * tam_pagina;
	return tam_max_segmento;
}

int numeroDeSegmento(int dir_logica, int tam_max_segmento){
	int num_segmento = floor(dir_logica / tam_max_segmento);
	return num_segmento;
}
int desplazamientoSegmento(int dir_logica, int tam_max_segmento){
	int desplazamiento_segmento = dir_logica % tam_max_segmento;
	return desplazamiento_segmento;
}

int numeroPagina(int desplazamiento_segmento, int tam_pagina){
	int num_pagina = floor(desplazamiento_segmento / tam_pagina);
	return num_pagina;
}

int desplazamientoPagina(int desplazamiento_segmento, int tam_pagina){
	int desplazamiento_pagina = desplazamiento_segmento % tam_pagina;
	return desplazamiento_pagina;
}

int primer_acceso(int numero_pagina, uint32_t indiceTablaPaginas){ 
	MSJ_MEMORIA_CPU_ACCESO_TABLA_DE_PAGINAS *mensajeAMemoriaAccesoTP = malloc(sizeof(MSJ_MEMORIA_CPU_ACCESO_TABLA_DE_PAGINAS));
	mensajeAMemoriaAccesoTP->idTablaDePaginas = indiceTablaPaginas;
	mensajeAMemoriaAccesoTP->pagina = numero_pagina;
	enviarMsje(socketMemoria, CPU, mensajeAMemoriaAccesoTP, sizeof(MSJ_MEMORIA_CPU_ACCESO_TABLA_DE_PAGINAS), ACCESO_MEMORIA_TABLA_DE_PAG);
	free(mensajeAMemoriaAccesoTP);
	
	log_debug(logger, "Envie mensaje a memoria para acceder a Tabla de Paginas con ID %d",indiceTablaPaginas);

	t_paqt paqueteMemoria;
	recibirMsje(socketMemoria, &paqueteMemoria);
	MSJ_INT* mensajePrimerAcceso = malloc(sizeof(MSJ_INT));
	mensajePrimerAcceso = paqueteMemoria.mensaje;
	switch (mensajePrimerAcceso->numero)
	{
	case PAGE_FAULT:
		return -1;
		break;
	case RESPUESTA_MEMORIA_MARCO_BUSCADO:
		int nroFrame = mensajePrimerAcceso->numero;
		log_info(logger, "(primer acceso)EL MARCO BUSCADO ES: %d", nroFrame);
		return nroFrame;
		break;
	
	default:
		break;
	}
	
}

/*----------------------TLB------------------------------*/


void iniciar_TLB(){

	int cantidadEntradasTLB = configCPU.entradasTLB;

	if(cantidadEntradasTLB==0){
		habilitarTLB = 0;
		return;
	}
	habilitarTLB = 1;

	TLB = malloc(sizeof(tlb));
	TLB->cant_entradas = cantidadEntradasTLB;
	TLB->entradas = list_create();
	TLB->algoritmo = configCPU.reemplazoTLB; // FIFO o LRU
}

int tlbTieneEntradasLibres(){ // Podria ser un Boole 
	return TLB->cant_entradas > TLB->entradas->elements_count;
}

//En este caso, la TLB tiene una o mas entradas libres // Revisar
void llenar_TLB(int nroPagina,int nroFrame, int nroSegmento, int pid){
	entrada_tlb* entrada = malloc(sizeof(entrada_tlb));
	entrada->nroPagina = nroPagina;
	entrada->nroFrame = nroFrame;
	entrada->nroSegmento = nroSegmento;
	entrada->pid = pid;
	list_add_in_index(TLB->entradas, 0, entrada);

	/**
	* @NAME: list_add_in_index
	* @DESC: Agrega un elemento en una posicion determinada de la lista
	*/
	//void list_add_in_index(t_list *, int index, void *element);

}

//Deveria buscar por nro pagina y nroSegmento?
int buscar_en_TLB(int nroPagina){ //int nroSegmento, int pid //devuelve numero de frame, si esta en la tlb, devuelve -1 si no esta en la tlb
	entrada_tlb* entradaActual;
	for(int i=0; i< TLB->entradas->elements_count; i++){
		entradaActual = list_get(TLB->entradas, i);
		if(entradaActual->nroPagina == nroPagina){
			if(strcmp(TLB->algoritmo , "LRU")== 0){
				entradaActual = list_remove(TLB->entradas,i);
				list_add_in_index(TLB->entradas,0,entradaActual);
			}

			//	TLB Hit: “PID: <PID> - TLB HIT - Segmento: <NUMERO_SEGMENTO> - Pagina: <NUMERO_PAGINA>”


			log_debug(logger, "TLB HIT: Pagina: %i, Frame: %i.\n", entradaActual->nroPagina, entradaActual->nroFrame);

			return entradaActual->nroFrame;
		}
	}

	//TLB Miss: “PID: <PID> - TLB MISS - Segmento: <NUMERO_SEGMENTO> - Pagina: <NUMERO_PAGINA>”
	
	log_debug(logger, "TLB MISS - pagina no encontrada en TLB\n");
	return -1;
}


void actualizar_TLB(int nroPagina,int nroFrame, int nroSegmento, int pid){

	if(tlbTieneEntradasLibres()){
		llenar_TLB(nroPagina, nroFrame,nroSegmento,pid);
		return;
	}

	//REEMPLAZO DE PAGINA
	if(strcmp(TLB->algoritmo , "LRU")== 0){
		reemplazo_algoritmo_lru(nroPagina, nroFrame);
	} else {
		reemplazo_algoritmo_fifo(nroPagina, nroFrame);
	}

}




void reemplazo_algoritmo_lru(int nroPagina, int nroFrame){ // int nroSegmento, int pid
	int i = list_size(TLB->entradas);
	i--;
	entrada_tlb* nuevaEntrada = malloc(sizeof(entrada_tlb));

	nuevaEntrada->nroPagina = nroPagina;
	nuevaEntrada->nroFrame = nroFrame;

	entrada_tlb* entradaRemplazada = list_remove(TLB->entradas, i);
	log_warning(logger, "Reemplaza pagina: %d por nueva pagina %d", entradaRemplazada->nroPagina, nuevaEntrada->nroPagina);
	printf("Reemplaza pagina: %d por nueva pagina %d", entradaRemplazada->nroPagina, nuevaEntrada->nroPagina);
	free(entradaRemplazada);
	list_add_in_index(TLB->entradas, 0, nuevaEntrada);

}


void reemplazo_algoritmo_fifo(int nroPagina, int nroFrame){ //int nroSegmento, int pid
	entrada_tlb* entradaNueva = malloc(sizeof(entrada_tlb));

	entradaNueva->nroPagina = nroPagina;
	entradaNueva->nroFrame = nroFrame;

	entrada_tlb* entradaAnterior = list_remove(TLB->entradas, TLB->cant_entradas-1);
	log_warning(logger, "Reemplazo de pagina: %d por nueva pagina %d", entradaAnterior->nroPagina, entradaNueva->nroPagina);
	printf(PRINT_COLOR_YELLOW "Reemplazo de pagina: %d por nueva pagina %d"PRINT_COLOR_RESET, entradaAnterior->nroPagina, entradaNueva->nroPagina);
	list_add_in_index(TLB->entradas, 0, entradaNueva);
	free(entradaAnterior);
}

// Freedoooom(?

void destruir_entrada(void* entrada){
	entrada_tlb* entradaTlb = (entrada_tlb*) entrada;
	free(entradaTlb);
}

void limpiar_entradas_TLB(){
	list_clean_and_destroy_elements(TLB->entradas, destruir_entrada);
}

void cerrar_TLB(){

	entrada_tlb* entradaActual;
	//Va destruyendo todas las entradas
	for(int i=0; i< TLB->entradas->elements_count; i++){
		entradaActual = list_get(TLB->entradas, i);
		list_remove_and_destroy_element(TLB->entradas, i, destruir_entrada);
	}
	free(TLB->algoritmo);
	free(TLB);
}