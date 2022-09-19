#include "server.h"

//int cliente_fd;(1)
int conectar_y_mostrar_mensajes_de_cliente(char* IP, char* PUERTO){

	logger = log_create("log.log", "Servidor", 1, LOG_LEVEL_DEBUG);

	int server_fd = iniciar_servidor(IP, PUERTO);
	log_info(logger, "Servidor listo para recibir al cliente");
	
	

	crear_hilos(server_fd);

	
	return EXIT_SUCCESS;
}

void crear_hilos(int server_fd){

	while(1){
		int cliente_fd = esperar_cliente(server_fd);
		//aca hay un log que dice que se conecto un cliente
		log_info(logger,"consola conectada, paso a crear el hilo");
		
		pthread_t thr1;
   
   //(1)esto funca pero sin pasarle argumentos, utiliza una variable global (cliente_fd)
   /*pthread_create(&thr1, NULL, (void *)mostrar_mensajes_del_cliente, NULL);
	log_info(logger,"se creo el hilo");*/

	//(2)esto esta funcionando, con pasarle una variable por parametro
	pthread_create(&thr1, NULL, (void *)mostrar_mensajes_del_cliente, cliente_fd);
	
	pthread_join(&thr1, NULL);
	
	}		
	
}

//(1)
/*
void mostrar_mensajes_del_cliente(){

	//int cliente_fd1 = (int)cliente_fd;
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
				case -1:
					log_error(logger, "el cliente se desconecto. Terminando servidor");
					y=0;
					break;
				default:
					log_warning(logger,"Operacion desconocida. No quieras meter la pata");
					break;
			}
		}	
}*/

//(2)
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
				case -1:
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
