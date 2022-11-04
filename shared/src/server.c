#include "server.h"

void conectar_y_mostrar_mensajes_de_cliente(char *IP, char *PUERTO, t_log *logger)
{

	int server_fd = iniciar_servidor(IP, PUERTO); // socket(), bind() listen()
	log_info(logger, "Servidor listo para recibir al cliente");

	crear_hilos(server_fd);
}

void crear_hilos(int server_fd)
{
	while (1)
	{

		// esto se podria cambiar como int* cliente_fd= malloc(sizeof(int)); si lo ponemos, va antes del while
		int cliente_fd = esperar_cliente(server_fd);
		// aca hay un log que dice que se conecto un cliente
		log_info(logger, "Consola conectada, paso a crear el hilo");

		pthread_t thr1;

		pthread_create(&thr1, NULL, (void *)mostrar_mensajes_del_cliente, cliente_fd);

		pthread_detach(&thr1);
	}
	// return EXIT_SUCCESS;
}

void mostrar_mensajes_del_cliente(int cliente_fd)
{
	t_list *lista;
	int y = 1;
	while (y)
	{
		int cod_op = recibir_operacion(cliente_fd);

		switch (cod_op)
		{
		case MENSAJE:
			recibir_mensaje(cliente_fd);
			break;
		case PAQUETE:
			lista = recibir_paquete(cliente_fd);
			log_info(logger, "Me llegaron los siguientes valores:");
			list_iterate(lista, (void *)iterator);
			break;
		case NEW:
			log_info(logger, "Llegaron las instrucciones y los segmentos");

			t_informacion info = recibir_informacion(cliente_fd);

			enviarResultado(cliente_fd, "Quedate tranqui Consola, llego todo lo que mandaste ;)\n");

			// aca deberia hacer que la consola se quede esperando

			//t_pcb *pcb = crear_pcb(&info, cliente_fd);

			//pasar_a_new(pcb);
			//log_debug(logger, "Estado Actual: NEW , proceso id: %d", pcb->id);

			printf("Cant de elementos de new: %d\n", list_size(LISTA_NEW));

			sem_post(&sem_hay_pcb_lista_new);
			sem_post(&sem_planif_largo_plazo);
			sem_post(&sem_agregar_pcb);

			break;

		case -1:
			// liberar_conexion(cliente_fd); //esto lo va a mandar kernel cuando lo necesite
			// log_error(logger, "el cliente se desconecto. Terminando servidor");
			y = 1;
			break;
		default:
			log_warning(logger, "Operacion desconocida. No quieras meter la pata");
			break;
		}
	}
}

t_informacion recibir_informacion(cliente_fd)
{
	int size;
	void *buffer = recibir_buffer(&size, cliente_fd);
	t_informacion programa;
	int offset = 0;

	memcpy(&(programa.instrucciones_size), buffer + offset, sizeof(uint32_t));
	offset += sizeof(uint32_t);
	memcpy(&(programa.segmentos_size), buffer + offset, sizeof(uint32_t));
	offset += sizeof(uint32_t);

	programa.instrucciones = list_create();
	t_instruccion *instruccion;

	programa.segmentos = list_create();
	uint32_t segmento;

	int k = 0;
	int l = 0;

	while (k < (programa.instrucciones_size))
	{
		instruccion = malloc(sizeof(t_instruccion));
		memcpy(instruccion, buffer + offset, sizeof(t_instruccion));
		offset += sizeof(t_instruccion);
		list_add(programa.instrucciones, instruccion);
		k++;
	}

	printf("Instrucciones:");
	for (int i = 0; i < programa.instrucciones_size; ++i)
	{
		t_instruccion *instruccion = list_get(programa.instrucciones, i);

		printf("\ninstCode: %d, Num: %d, RegCPU[0]: %d,RegCPU[1] %d, dispIO: %d",
			   instruccion->instCode, instruccion->paramInt, instruccion->paramReg[0], instruccion->paramReg[1], instruccion->paramIO);
	}

	while (l < (programa.segmentos_size))
	{
		// segmento = malloc(sizeof(uint32_t));
		memcpy(&segmento, buffer + offset, sizeof(uint32_t));
		offset += sizeof(uint32_t);
		list_add(programa.segmentos, segmento);
		l++;
	}

	printf("\n\nSegmentos:");

	printf("\n[%d,%d,%d,%d]\n", list_get(programa.segmentos, 0), list_get(programa.segmentos, 1), list_get(programa.segmentos, 2), list_get(programa.segmentos, 3));

	free(buffer);

	return programa;
}

void iterator(char *value)
{

	log_info(logger, "%s", value);
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
		printf("\nllego pcb a plani corto plazo\n");
		t_tipo_algoritmo algoritmo = obtenerAlgoritmo();

		sem_wait(&contador_pcb_running);

		switch (algoritmo)
		{
		case FIFO:
			log_debug(logger, "Implementando algoritmo FIFO");
			implementar_fifo();
			break;
		case RR:
			log_debug(logger, "Implementando algoritmo RR");
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

void pasar_a_new(t_pcb *pcb)
{
	pthread_mutex_lock(&mutex_lista_new);
	list_add(LISTA_NEW, pcb);
	pthread_mutex_unlock(&mutex_lista_new);

	log_debug(logger, "Paso a NEW el proceso %d", pcb->id);
}

void pasar_a_ready(t_pcb *pcb)
{
	pthread_mutex_lock(&mutex_lista_ready);
	list_add(LISTA_READY, pcb);
	pthread_mutex_unlock(&mutex_lista_ready);

	log_debug(logger, "Paso a READY el proceso %d", pcb->id);
}

void pasar_a_ready_auxiliar(t_pcb *pcb)
{
	pthread_mutex_lock(&mutex_lista_ready_auxiliar);
	list_add(LISTA_READY_AUXILIAR, pcb);
	pthread_mutex_unlock(&mutex_lista_ready_auxiliar);

	log_debug(logger, "Paso a READY aux el proceso %d", pcb->id);
}

void pasar_a_exec(t_pcb *pcb)
{
	pthread_mutex_lock(&mutex_lista_exec);
	list_add(LISTA_EXEC, pcb);
	pthread_mutex_unlock(&mutex_lista_exec);

	log_debug(logger, "Paso a EXEC el proceso %d", pcb->id);
}

void pasar_a_block(t_pcb *pcb)
{
	pthread_mutex_lock(&mutex_lista_blocked);
	list_add(LISTA_BLOCKED, pcb);
	pthread_mutex_unlock(&mutex_lista_blocked);

	log_debug(logger, "Paso a BLOCK el proceso %d", pcb->id);
}
void pasar_a_block_pantalla(t_pcb *pcb)
{
	pthread_mutex_lock(&mutex_lista_blocked_pantalla);
	list_add(LISTA_BLOCKED_PANTALLA, pcb);
	pthread_mutex_unlock(&mutex_lista_blocked_pantalla);

	log_debug(logger, "Paso a BLOCK el proceso %d", pcb->id);
}
void pasar_a_block_teclado(t_pcb *pcb)
{
	pthread_mutex_lock(&mutex_lista_blocked_teclado);
	list_add(LISTA_BLOCKED_TECLADO, pcb);
	pthread_mutex_unlock(&mutex_lista_blocked_teclado);

	log_debug(logger, "Paso a BLOCK el proceso %d", pcb->id);
}

void pasar_a_exit(t_pcb *pcb)
{
	pthread_mutex_lock(&mutex_lista_exit);
	list_add(LISTA_EXIT, pcb);
	pthread_mutex_unlock(&mutex_lista_exit);

	log_debug(logger, "Paso a EXIT el proceso %d", pcb->id);
}

void iniciar_listas_y_semaforos()
{
	// listas
	LISTA_NEW = list_create();
	LISTA_READY = list_create();
	LISTA_EXEC = list_create();
	LISTA_BLOCKED = list_create();
	LISTA_BLOCKED_PANTALLA = list_create();
	LISTA_BLOCKED_TECLADO = list_create();
	LISTA_SOCKETS = list_create();
	LISTA_EXIT = list_create();
	LISTA_READY_AUXILIAR = list_create();

	// mutex
	pthread_mutex_init(&mutex_creacion_ID, NULL);
	pthread_mutex_init(&mutex_lista_new, NULL);
	pthread_mutex_init(&mutex_lista_ready, NULL);
	pthread_mutex_init(&mutex_lista_exec, NULL);
	pthread_mutex_init(&mutex_lista_blocked, NULL);
	pthread_mutex_init(&mutex_lista_blocked_pantalla, NULL);
	pthread_mutex_init(&mutex_lista_blocked_teclado, NULL);
	pthread_mutex_init(&mutex_lista_ready_auxiliar, NULL);

	// semaforos
	sem_init(&sem_ready, 0, 0);
	sem_init(&sem_bloqueo, 0, 0);
	sem_init(&sem_planif_largo_plazo, 0, 0);
	sem_init(&sem_hay_pcb_lista_new, 0, 0);
	sem_init(&sem_hay_pcb_lista_ready, 0, 0);
	sem_init(&sem_agregar_pcb, 0, 0);
	sem_init(&sem_eliminar_pcb, 0, 0);
	sem_init(&sem_pasar_pcb_running, 0, 0);
	sem_init(&sem_timer, 0, 0);
	sem_init(&sem_desalojar_pcb, 0, 0);
	sem_init(&sem_kill_trhread, 0, 0);
	sem_init(&contador_multiprogramacion, 0, configKernel.gradoMultiprogramacion);
	sem_init(&contador_pcb_running, 0, 1);
	sem_init(&sem_llamar_feedback, 0, 0);
}

void agregar_pcb()
{
	sem_wait(&contador_multiprogramacion);

	printf("Agregando un pcb a lista ready");

	pthread_mutex_lock(&mutex_lista_new);
	t_pcb *pcb = algoritmo_fifo(LISTA_NEW);
	printf("Cant de elementos de new: %d\n", list_size(LISTA_NEW));
	pthread_mutex_unlock(&mutex_lista_new);

	pasar_a_ready(pcb);

	log_debug(logger, "Estado Anterior: NEW , proceso id: %d", pcb->id);
	log_debug(logger, "Estado Actual: READY , proceso id: %d", pcb->id);

	printf("Cant de elementos de ready: %d\n", list_size(LISTA_READY));


	//sem_post(&sem_enviar_mensaje_memoria);
	sem_post(&sem_hay_pcb_lista_ready);

	
	
}

void eliminar_pcb()
{
	pthread_mutex_lock(&mutex_lista_exec);
	t_pcb *pcb = algoritmo_fifo(LISTA_EXEC);
	pthread_mutex_unlock(&mutex_lista_exec);

	pasar_a_exit(pcb);
	sem_post(&contador_pcb_running);
	log_debug(logger, "Estado Anterior: EXEC , proceso id: %d", pcb->id);
	log_debug(logger, "Estado, proceso Actual: EXIT  id: %d", pcb->id);

	 //enviar_mensaje("hola  memoria, libera las estructuras", conexionMemoria);
	// podriamos poner un semaforo que envie memoria, que diga que ya libero las estructuras para seguir
	sem_post(&contador_multiprogramacion);
}

void iteratorInt(int value)
{

	log_info(logger, "Segmento = %d", value);
}


t_tipo_algoritmo obtenerAlgoritmo()
{
	
	char *algoritmo = configKernel.algoritmo;

	t_tipo_algoritmo algoritmoResultado;

	if (!strcmp(algoritmo, "FIFO"))
	{
		algoritmoResultado = FIFO;
	}
	else if (!strcmp(algoritmo, "RR"))
	{
		algoritmoResultado = RR;
	}
	else
	{
		algoritmoResultado = FEEDBACK;
	}

	return algoritmoResultado;
}



t_pcb *algoritmo_fifo(t_list *lista)
{
	t_pcb *pcb = (t_pcb *)list_remove(lista, 0);
	return pcb;
}

void implementar_feedback()
{
	//implementar_rr();

	
	if (list_is_empty(LISTA_READY))
	{
		implementar_fifo_auxiliar();
	}
	else
	{
		implementar_rr();
	}

}

void implementar_fifo()
{

	t_pcb *pcb = algoritmo_fifo(LISTA_READY);
	printf("\nAgregando UN pcb a lista exec");
	pasar_a_exec(pcb);
	printf("\nCant de elementos de exec: %d\n", list_size(LISTA_EXEC));

	log_debug(logger, "Estado Anterior: READY , proceso id: %d", pcb->id);
	log_debug(logger, "Estado Actual: EXEC , proceso id: %d", pcb->id);

	sem_post(&sem_pasar_pcb_running);
}

void implementar_fifo_auxiliar()
{

	t_pcb *pcb = algoritmo_fifo(LISTA_READY_AUXILIAR);
	printf("\nAgregando UN pcb a lista exec");
	pasar_a_exec(pcb);
	printf("\nCant de elementos de exec: %d\n", list_size(LISTA_EXEC));

	log_debug(logger, "Estado Anterior: READY , proceso id: %d", pcb->id);
	log_debug(logger, "Estado Actual: EXEC , proceso id: %d", pcb->id);

	sem_post(&sem_pasar_pcb_running);
}

void implementar_rr()
{
	t_pcb *pcb = algoritmo_fifo(LISTA_READY);
	pthread_t thrTimer;

	pthread_create(&thrTimer, NULL, (void *)hilo_timer, NULL);
	pthread_detach(&thrTimer);
	printf("\nAgregando UN pcb a lista exec rr");
	pasar_a_exec(pcb);
	printf("\nCant de elementos de exec: %d\n", list_size(LISTA_EXEC));

	log_debug(logger, "Estado Anterior: READY , proceso id: %d", pcb->id);
	log_debug(logger, "Estado Actual: EXEC , proceso id: %d", pcb->id);

	sem_post(&sem_pasar_pcb_running);
	sem_post(&sem_timer);

	sem_wait(&sem_kill_trhread);
	
	pthread_cancel(thrTimer);
	
	if (pthread_cancel(thrTimer)==0)
	{
		printf("Hilo cancelado con exito");
	}else{
		printf("No mate el hilo");
	}
	
	
	
	
}

void hilo_timer()
{
	sem_wait(&sem_timer);
	printf("\nvoy a dormir, soy el timer\n");
	usleep(configKernel.quantum);

	pthread_setcancelstate(PTHREAD_CANCEL_ENABLE,NULL);

	printf("\nme desperte!\n");
	sem_post(&sem_desalojar_pcb);

	printf("\nenvie post desalojar pcb\n");
}

void serializarValor(uint32_t valorRegistro, int socket, t_tipoMensaje tipoMensaje)
{

	t_buffer *buffer = malloc(sizeof(t_buffer));

	buffer->size = sizeof(uint32_t) * 2;
	void *stream = malloc(buffer->size);
	int offset = 0;

	memcpy(stream + offset, &(valorRegistro), sizeof(uint32_t));
	offset += sizeof(uint32_t);

	buffer->stream = stream;
	crearPaquete(buffer, tipoMensaje, socket);
}

uint32_t *deserializarValor(t_buffer *buffer, int socket)
{
	uint32_t *valorRegistro = malloc(sizeof(uint32_t));
	void *stream = buffer->stream;

	memcpy(&(valorRegistro), stream, sizeof(uint32_t));
	stream += sizeof(uint32_t);

	return valorRegistro;
}

void controlBloqueo()
{

	//TODO
	sem_wait(&sem_bloqueo);
	t_pcb *pcb = algoritmo_fifo(LISTA_BLOCKED);

	t_instruccion *insActual = list_get(pcb->informacion->instrucciones, pcb->program_counter);

	
	int tamanio = string_length(configKernel.dispositivosIO);
	//int tamanio = string_length(configKernel.dispositivosIO);
	for (int i = 0; i < tamanio; i++)
	{
		
	}
}