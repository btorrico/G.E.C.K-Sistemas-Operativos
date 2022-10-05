#include "server.h"

void conectar_y_mostrar_mensajes_de_cliente(char *IP, char *PUERTO, t_log *logger)
{

	int server_fd = iniciar_servidor(IP, PUERTO); // socket(), bind()listen()
	log_info(logger, "Servidor listo para recibir al cliente");

	crear_hilos(server_fd);
}

int crear_hilos(int server_fd)
{

	while (1)
	{

		// esto se podria cambiar como int* cliente_fd= malloc(sizeof(int)); si lo ponemos, va antes del while
		int cliente_fd = esperar_cliente(server_fd);
		// aca hay un log que dice que se conecto un cliente
		log_info(logger, "consola conectada, paso a crear el hilo");

		pthread_t thr1;

		pthread_create(&thr1, NULL, (void *)mostrar_mensajes_del_cliente, cliente_fd);

		pthread_detach(&thr1);
	}
	return EXIT_SUCCESS;
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
				// PROBAR LAS CONSOLAS 
					log_info(logger, "Llegaron las instrucciones y los segmentos"); // Ya casi!
					int size;
					void* buffer = recibir_buffer(&size, cliente_fd);
					t_informacion programa;
					int offset = 0;

					memcpy(&programa.instrucciones_size, buffer + offset, sizeof(uint32_t));
					memcpy(&programa.segmentos_size, buffer + offset, sizeof(uint32_t));

					int cantidadInstrucciones = (size - sizeof(uint32_t)) / sizeof(t_instruccion); // Calcula la cantidad de instrucciones
				    offset += sizeof(uint32_t);										               // para recorrerlas al deserializar

					int cantidadSegmentos= (size - sizeof(uint32_t)) / sizeof(t_list);
					offset += sizeof(uint32_t);

					programa.instrucciones = list_create();
					t_instruccion* instruccion;

				    programa.segmentos = list_create(); 
					char* segmento;
					
					int k = 0;
					int l =0;

					while (k < cantidadInstrucciones) {
						instruccion = malloc(sizeof(t_instruccion));
							memcpy(instruccion, buffer + offset, sizeof(t_instruccion));
							offset += sizeof(t_instruccion);
							list_add(programa.instrucciones, instruccion);
								k++;

					}

					list_iterate(programa.instrucciones,(void*) iterator); // RETORNA 8 ESPACIOS VACIOS
					
					while (l < cantidadSegmentos) {
						segmento = malloc(sizeof(char));
							memcpy(segmento, buffer + offset, sizeof(char));
							offset += sizeof(char);
							list_add(programa.segmentos, segmento); 
								l++;
								
							
					}
							list_iterate(programa.segmentos,(void*) iterator);	
					

						free(buffer);
						log_info(logger,"CUANDO FUNCIONE PODRIAMOS CREAR UNA FUNCION PARA ABSTRAER TODO EL CHOCLO(?");
				break; 
		case PROGRAMA:
			t_pcb *pcb = crear_pcb();

			pasar_a_new(pcb);

			planifLargoPlazo(AGREGAR_PCB);
		case -1:
			/*while(cod_op_servidor =! -1){
				close(socket_cliente);
			}*/
			log_error(logger, "el cliente se desconecto. Terminando servidor");
			y = 0;
			break;
		default:
			log_warning(logger, "Operacion desconocida. No quieras meter la pata");
			break;
		}
	}
}

void recibir_informacion(cliente_fd){
					int size;
					void* buffer = recibir_buffer(&size, cliente_fd);
					t_informacion programa;
					int offset = 0;

					memcpy(&(programa.instrucciones_size), buffer + offset, sizeof(uint32_t));
					offset += sizeof(uint32_t);
					memcpy(&(programa.segmentos_size), buffer + offset, sizeof(uint32_t));
					offset += sizeof(uint32_t);

					programa.instrucciones = list_create();
					t_instruccion* instruccion;

				    programa.segmentos = list_create(); 
					char* segmento;
					
					int k = 0;
					int l =0;

					while (k < (programa.instrucciones_size)) {
						instruccion = malloc(sizeof(t_instruccion));
							memcpy(instruccion, buffer + offset, sizeof(t_instruccion));
							offset += sizeof(t_instruccion);
							list_add(programa.instrucciones, instruccion);
								k++;

					}

						printf("Instrucciones:");
					for (int i = 0; i < programa.instrucciones_size; ++i) {
						t_instruccion* instruccion= list_get(programa.instrucciones, i);
							
						printf("\ninstCode: %d, Num: %d, RegCPU[0]: %d,RegCPU[1] %d, dispIO: %d",
			   instruccion->instCode, instruccion->paramInt, instruccion->paramReg[0], instruccion->paramReg[1], instruccion->paramIO);
						}

					
					while (l < (programa.segmentos_size)) {
						segmento = malloc(sizeof(char*));
							memcpy(segmento, buffer + offset, sizeof(char*));
							offset += sizeof(char*);
							list_add(programa.segmentos, segmento); 
								l++;
								
							
					}
					printf("\n\nSegmentos:");

					printf("\n[%s,%s,%s,%s]\n",list_get(programa.segmentos,0), list_get(programa.segmentos,1), list_get(programa.segmentos,2), list_get(programa.segmentos,3));
							

						free(buffer);
}

void iterator(char *value)
{

	log_info(logger, "%s", value);
}

void planifLargoPlazo(t_cod_planificador *cod_planificador)
{

	switch (*cod_planificador)
	{
	case AGREGAR_PCB:
		agregar_pcb();
		break;
	case ELIMINAR_PCB:
		eliminar_pcb(cod_planificador);
		break;
	default:
		break;
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
	LISTA_SOCKETS = list_create();
	LISTA_EXIT = list_create();

	// mutex
	pthread_mutex_init(&mutex_creacion_ID, NULL);
	pthread_mutex_init(&mutex_lista_new, NULL);
	pthread_mutex_init(&mutex_lista_ready, NULL);
	pthread_mutex_init(&mutex_lista_exec, NULL);
	pthread_mutex_init(&mutex_lista_blocked, NULL);

	// semaforos
	sem_init(&sem_ready, 0, 0);
	sem_init(&sem_bloqueo, 0, 0);
	sem_init(&sem_planif_largo_plazo, 0, 0);
	// ver como pasar el gradoDeMultiProgramacion
	// sem_init(&contador_multiprogramacion, 0, configKernel.gradoMultiprogramacion);
	sem_init(&sem_procesador, 0, 1);
}

void agregar_pcb()
{

	while (1)
	{

		sem_wait(&contador_multiprogramacion);

		pthread_mutex_lock(&mutex_lista_new);
		t_pcb *pcb = (t_pcb *)list_remove(LISTA_NEW, 0);
		pthread_mutex_unlock(&mutex_lista_new);

		pasar_a_ready(pcb);
		// enviar_mensaje("hola  memoria, inicializa las estructuras", conexionMemoria);
	}
}

void eliminar_pcb(t_cod_planificador cod_planificador)
{

	pthread_mutex_lock(&mutex_lista_exec);
	t_pcb *pcb = (t_pcb *)list_remove(LISTA_EXEC, 0);
	pthread_mutex_unlock(&mutex_lista_exec);

	pasar_a_exit(pcb);

	// enviar_mensaje("hola  memoria, libera las estructuras", conexionMemoria);
	sem_post(&contador_multiprogramacion);
}

void iteratorInt(int value) {

	log_info(logger,"Segmento = %d", value);
}
t_pcb *crear_pcb()
{
	// TODO
	
}



	