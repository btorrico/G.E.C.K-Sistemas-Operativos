#include "client.h"




t_log* iniciar_logger(char* archivoDeLog, char* nombreLogger, t_log_level nivel)
{
	t_log* nuevo_logger;
	if((nuevo_logger = log_create(archivoDeLog, nombreLogger, true, nivel)) == NULL){
		printf("¡No se pudo crear el logger!");
	}
	return nuevo_logger;
}

t_config* iniciar_config(char* archivoDeConfig)
{
	t_config* nuevo_config;
	if((nuevo_config = config_create(archivoDeConfig)) == NULL){
		printf("¡No se pudo crear el config!");
	}
	return nuevo_config;
}

void leer_consola(t_log* logger)
{
	char* leido;

	// La primera te la dejo de yapa
	leido = readline("> ");

	// El resto, las vamos leyendo y logueando hasta recibir un string vacío
	while(strcmp(leido, "")){ //si es igual me devuelve 0, caso contrario me duelve 1 y sigue el bucle
		log_info(logger, leido);

	// ¡No te olvides de liberar las lineas antes de regresar!
		free(leido);
		leer_consola(logger);
	}
}

void paquete(int conexion)
{
	// Ahora toca lo divertido!
	char* leido;
	t_paquete* paquete;

	
	//creo el paquete
	paquete = crear_paquete();

	// Leemos y esta vez agregamos las lineas al paquete

	//aca podia haber usado leer_consola(logger), pero tenia que definir un logger y se hubiera mandado un logger, y me parecio mejor esta solucion
	leido = readline("> ");

	while(strcmp(leido, "")){
		int tamanio = strlen(leido) + 1;
		agregar_a_paquete(paquete, leido , tamanio);
		free(leido);
		leido = readline("> ");
	}

	enviar_paquete(paquete, conexion);

	// ¡No te olvides de liberar las líneas y el paquete antes de regresar!
	eliminar_paquete(paquete);
	
}

void paqueteKernelACPU(int conexion)
{
	t_paquete* paquete;

	nombre* personaUno = malloc(sizeof(nombre));
	
	memcpy(personaUno->primerNombre, "Daniela", 8);
	memcpy(personaUno -> segundoNombre, "Andrea", 7);

	paquete = crear_paquete();	

	paquete = empaquetarEstructura(personaUno);
	printf("%d %d %s", paquete->codigo_operacion, paquete->buffer->size, paquete->buffer->stream);

	enviar_paquete(paquete, conexion);

	// ¡No te olvides de liberar las líneas y el paquete antes de regresar!
	eliminar_paquete(paquete);
	
}

t_paquete* empaquetarEstructura(nombre* estructura){

	t_paquete* paquete;
	char* auxiliar = malloc(sizeof(estructura));

	char* separador = '|';

	string_append(&auxiliar, estructura->primerNombre);
	string_append(&auxiliar, separador);
	string_append(&auxiliar, estructura->segundoNombre);
	
	paquete->buffer->stream = auxiliar;
	paquete->buffer->size = sizeof(estructura) + 1 + 2;
	paquete->codigo_operacion = PAQUETE;

	return paquete;
}


nombre* desempaquetarEstructura(char* stream){

	char* separador = '|';

	nombre* estructura = malloc(sizeof(nombre));

	char* buffer = strtok(stream, separador);
	
	for (int i = 0; i < 2; i++)
	{
		if (i==0)
		{
			memcpy(estructura->primerNombre, buffer, string_length(buffer));
			buffer =strtok(stream, NULL);
		}else if(i == 1){
			memcpy(estructura->segundoNombre, buffer, string_length(buffer));
		}
		
	}


	return estructura;
}



void terminar_programa(int conexion, t_log* logger, t_config* config)
{
	/* Y por ultimo, hay que liberar lo que utilizamos (conexion, log y config) 
	  con las funciones de las commons y del TP mencionadas en el enunciado */
	log_destroy(logger);
	config_destroy(config);
	liberar_conexion(conexion);
}
