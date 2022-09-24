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
