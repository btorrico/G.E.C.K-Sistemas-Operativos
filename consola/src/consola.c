#include "consola.h"

int main(int argc, char **argv)
{
	if (argc > 1 && strcmp(argv[1], "-test") == 0)
		return run_tests();
	else
	{
		/* ---------------- LOGGING ---------------- */

		logger = iniciar_logger("consola.log", "CONSOLA", LOG_LEVEL_DEBUG);

		log_info(logger, "\n Iniciando consola...");

		/* ---------------- ARCHIVOS DE CONFIGURACION ---------------- */

		obtenerArgumentos(argc, argv); // Recibe 3 argumentos, ./consola, la ruta del archivoConfig y la ruta de las instrucciones de pseudocodigo
		


		/* ---------------- LEER DE CONSOLA ---------------- */

		FILE *instructionsFile = abrirArchivo(argv[2]);

		t_informacion *informacion = crearInformacion();

		agregarInstruccionesDesdeArchivo(instructionsFile, informacion->instrucciones);

		t_paquete *nuevoPaquete = crear_paquete_programa(informacion);

		conexion = crear_conexion(configConsola.ipKernel, configConsola.puertoKernel);

		enviar_mensaje("Hola", conexion);

		// Armamos y enviamos el paquete
		paquete(conexion);

		//--------
		enviar_paquete(nuevoPaquete, conexion);
		eliminar_paquete(nuevoPaquete);
		liberar_programa(informacion);
		//---
		terminar_programa(conexion, logger, config);
	}
}


void leerConfig(char *rutaConfig)
{
	config = iniciar_config(rutaConfig);
	extraerDatosConfig(config);

	printf(PRINT_COLOR_GREEN "\n===== Archivo de configuracion =====\n IP: %s \n PUERTO: %s \n SEGMENTOS: [", configConsola.ipKernel, configConsola.puertoKernel);
	
	for(int i=0; i<size_char_array(configConsola.segmentos); i++) {
		printf("%s",configConsola.segmentos[i]);

		if((i+1)<size_char_array(configConsola.segmentos)){
			printf(", ");
		}
	}
	printf("]" PRINT_COLOR_RESET);


}


void obtenerArgumentos(int argc, char **argv)
{

	if (argc != 3)
	{
		printf(PRINT_COLOR_RED "\nError: cantidad de argumentos incorrecta:" PRINT_COLOR_RESET "\n");
		// return -1;
	}
	else
	{
		rutaArchivoConfiguracion = argv[1];
		rutaInstrucciones = argv[2];

		leerConfig(rutaArchivoConfiguracion);

		printf(PRINT_COLOR_GREEN "\nCantidad de argumentos de entrada Correctos!" PRINT_COLOR_RESET "\n");
	}

	printf("=== Argumentos de entrada ===\n rutaArchivoConfig: %s \n rutaArchivoDeInstrucciones: %s \n\n", rutaArchivoConfiguracion, rutaInstrucciones);
}

FILE *abrirArchivo(char *filename)
{
	if (filename == NULL)
	{
		log_error(logger, "Error: debe informar un archivo de instrucciones.");
		exit(1);
	}
	return fopen(filename, "r");
}

t_configConsola extraerDatosConfig(t_config *archivoConfig)
{
	configConsola.ipKernel = string_new();
	configConsola.puertoKernel = string_new();
	configConsola.segmentos = string_array_new(); 

	configConsola.ipKernel = config_get_string_value(archivoConfig, "IP_KERNEL");
	configConsola.puertoKernel = config_get_string_value(archivoConfig, "PUERTO_KERNEL");
	configConsola.segmentos = config_get_array_value(archivoConfig, "SEGMENTOS"); 																		

	return configConsola;
}


t_list* listaSegmentos(){

	t_list* listaDeSegmentos = list_create();
	for(int i=0; i<size_char_array(configConsola.segmentos); i++) {

		//uint32_t segmento = atoi(configConsola.segmentos[i]);	
		char* segmento = (char*)configConsola.segmentos[i];	 //De esta forma se crea una lista de segmentos de tipo char*
		//del lado del kernel podriamos despues de serializar podriamos obtener cada elemento y convertirlo a int

		list_add(listaDeSegmentos,segmento);
	

	}
		return listaDeSegmentos;


}



t_informacion *crearInformacion()
{
	t_informacion *informacion = malloc(sizeof(t_informacion));
	informacion->instrucciones = list_create();
	informacion->segmentos = listaSegmentos();
	return informacion; 
}


void liberar_programa(t_informacion *informacion)
{
	list_destroy_and_destroy_elements(informacion->instrucciones, free);
	free(informacion);
}



t_paquete *crear_paquete_programa(t_informacion *informacion)
{
	t_buffer *buffer = malloc(sizeof(t_buffer));

	buffer->size = sizeof(uint32_t)  // instrucciones_size
				 + list_size(informacion->instrucciones) * sizeof(t_instruccion)
				 + sizeof(uint32_t)  // segmentos_size
				 + list_size(informacion->segmentos) * sizeof(char*);

	void *stream = malloc(buffer->size);

	int offset = 0; // Desplazamiento
	memcpy(stream + offset, &(informacion->instrucciones_size), sizeof(uint32_t));
	offset += sizeof(uint32_t);
	
	memcpy(stream + offset, &(informacion->segmentos_size), sizeof(uint32_t));
	offset += sizeof(uint32_t);
	// Serializa las instrucciones
	int i = 0;
	int j = 0;
	while (i < list_size(informacion->instrucciones))
	{
		memcpy(stream + offset, list_get(informacion->instrucciones, i), sizeof(t_instruccion));
		offset += sizeof(t_instruccion);
		i++;
		printf(PRINT_COLOR_MAGENTA "Estoy serializando las instruccion %d" PRINT_COLOR_RESET "\n", i);
	}

	
	while (j < list_size(informacion->segmentos))
	{

		memcpy(stream + offset, list_get(informacion->segmentos, j), sizeof(char*));
		offset += sizeof(char*);
		j++;
		printf(PRINT_COLOR_YELLOW "Estoy serializando el segmento: %d" PRINT_COLOR_RESET "\n", j);
		
		
	}
	

	buffer->stream = stream; //Payload

	//free(informacion->instrucciones);
	//free(informacion->segmentos);

	// lleno el paquete
	t_paquete *paquete = malloc(sizeof(t_paquete));
	paquete->codigo_operacion = NEW;
	paquete->buffer = buffer;
	return paquete;
}