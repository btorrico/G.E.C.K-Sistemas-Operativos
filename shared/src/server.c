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
			    case NEW:
				// PROBAR LAS CONSOLAS Y VER LOS BELLOS ERRORES POR FAVOR(?)
					log_info(logger, "Llegaron las instrucciones y los segmentos"); // Mentiraaaa
					int size;
					void* buffer = recibir_buffer(&size, cliente_fd);
					t_informacion programa;
					int offset = 0;

					memcpy(&programa.instrucciones_size, buffer + offset, sizeof(uint32_t));
					memcpy(&programa.segmentos_size, buffer + offset, sizeof(uint32_t));

					int cantidadInstrucciones = (size - sizeof(uint32_t)) / sizeof(t_instruccion); // Calcula la cantidad de instrucciones
				    offset += sizeof(uint32_t);										               // para recorrerlas al deserializar

					int cantidadSegmentos= (size - sizeof(uint32_t)) / sizeof(char**);
					offset += sizeof(uint32_t);

					programa.instrucciones = list_create();
					t_instruccion* instruccion;

				    programa.segmentos = ""; //?????? char** /////!!!!!!!!!!1
					char** segmento;
					
					int k = 0;
					int l =0;

					while (k < cantidadInstrucciones) {
						instruccion = malloc(sizeof(t_instruccion));
							memcpy(instruccion, buffer + offset, sizeof(t_instruccion));
							offset += sizeof(t_instruccion);
							list_add(programa.instrucciones, instruccion);
								k++;

								//Retornamos la lista?????
					}

					while (l < cantidadSegmentos) {
						segmento = malloc(sizeof(char**));
							memcpy(segmento, buffer + offset, sizeof(char**));
							offset += sizeof(char**);
							list_add(programa.segmentos, segmento); // LOS SEGMENTOS NO SON UNA LISTA :()
								l++;

								//Retornamos la lista????? o los char
					}
						free(buffer);
						log_info(logger, "YA TIENE INCLUIDA LA LISTA DE SEGMENTO, FALTA REVISAR BIEN");
						log_info(logger,"CUANDO FUNCIONE PODRIAMOS CREAR UNA FUNCION PARA ABSTRAER TODO EL CHOCLO(?");
				
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
