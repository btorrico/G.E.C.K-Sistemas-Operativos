#include "server.h"


void conectar_y_mostrar_mensajes_de_cliente(char* IP, char* PUERTO, t_log* logger){

	

	int server_fd = iniciar_servidor(IP, PUERTO); //socket(), bind()listen()
	log_info(logger, "Servidor listo para recibir al cliente");
	
	

	crear_hilos(server_fd);

	
	
}

int crear_hilos(int server_fd){


	while(1){
		
			//esto se podria cambiar como int* cliente_fd= malloc(sizeof(int)); si lo ponemos, va antes del while
			int cliente_fd = esperar_cliente(server_fd);
		//aca hay un log que dice que se conecto un cliente
			log_info(logger,"consola conectada, paso a crear el hilo");
		
			pthread_t thr1;

			pthread_create(&thr1, NULL, (void *)mostrar_mensajes_del_cliente, cliente_fd);
	 		
			pthread_detach(&thr1);
			
		
		}
return EXIT_SUCCESS;
	
}

void mostrar_mensajes_del_cliente(int cliente_fd){

	t_list* lista;
	int y = 1;
		while (y) {
			int cod_op = recibir_operacion(cliente_fd);
		
			switch (cod_op) {
				case MENSAJE:
					recibir_mensaje(cliente_fd);
					break;
				case PAQUETE:
					lista = recibir_paquete(cliente_fd);
					log_info(logger, "Me llegaron los siguientes valores:");
					list_iterate(lista, (void*) iterator);
					break;
				case PROGRAMA:
					t_pcb* pcb = crear_pcb();

					pasar_a_new(pcb);
					
					planifLargoPlazo(AGREGAR_PCB);

				case -1:
					/*while(cod_op_servidor =! -1){
						close(socket_cliente);
					}*/
					log_error(logger, "el cliente se desconecto. Terminando servidor");
					y=0;
					break;
				default:
					log_warning(logger,"Operacion desconocida. No quieras meter la pata");
					break;
			}
		}	
}


void iterator(char* value) {

	log_info(logger,"%s", value);
}


//Funciones planificador largo plazo

void pasar_a_new(t_pcb* pcb){
	pthread_mutex_lock(&mutex_lista_new);
	list_add(LISTA_NEW, pcb);
	pthread_mutex_unlock(&mutex_lista_new);

	log_debug(loggerKernel, "Paso a NEW el proceso %d", pcb->id);
}

void pasar_a_ready(t_pcb* pcb) {
	pthread_mutex_lock(&mutex_lista_ready);
	list_add(LISTA_READY, pcb);
	pthread_mutex_unlock(&mutex_lista_ready);

	log_debug(loggerKernel, "Paso a READY el proceso %d", pcb->id);
}

void pasar_a_exec(t_pcb* pcb){
	pthread_mutex_lock(&mutex_lista_exec);
	list_add(LISTA_EXEC, pcb);
	pthread_mutex_unlock(&mutex_lista_exec);

	log_debug(loggerKernel, "Paso a EXEC el proceso %d", pcb->id);
}

void pasar_a_block(t_pcb* pcb) {
	pthread_mutex_lock(&mutex_lista_blocked);
	list_add(LISTA_BLOCKED,pcb);
	pthread_mutex_unlock(&mutex_lista_blocked);

	log_debug(loggerKernel, "Paso a BLOCK el proceso %d", pcb->id);
}

void pasar_a_exit(t_pcb* pcb) {
	pthread_mutex_lock(&mutex_lista_exit);
	list_add(LISTA_EXIT, pcb);
	pthread_mutex_unlock(&mutex_lista_exit);

	log_debug(loggerKernel, "Paso a EXIT el proceso %d", pcb->id);
}
t_pcb* crear_pcb(){
	//TODO
}


void planifLargoPlazo(t_cod_planificador cod_planificador){

	switch (cod_planificador)
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

void agregar_pcb(){

while(1){

	sem_wait(&contador_multiprogramacion);

	pthread_mutex_lock(&mutex_lista_new);
	t_pcb* pcb = (t_pcb*)list_remove(LISTA_NEW, 0);
	pthread_mutex_unlock(&mutex_lista_new);

	pasar_a_ready(pcb);
	enviar_mensaje("hola  memoria, inicializa las estructuras", conexionMemoria);


}
}

void eliminar_pcb(t_cod_planificador cod_planificador){
	
	pthread_mutex_lock(&mutex_lista_exec);
	t_pcb* pcb = (t_pcb*)list_remove(LISTA_EXEC, 0);
	pthread_mutex_unlock(&mutex_lista_exec);

	pasar_a_exit(pcb);

	enviar_mensaje("hola  memoria, libera las estructuras", conexionMemoria);
	sem_post(&contador_multiprogramacion);
}

