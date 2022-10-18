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
		//	paquete(conexion);

		enviar_paquete(nuevoPaquete, conexion);
		eliminar_paquete(nuevoPaquete);
		liberar_programa(informacion);
		//---
		log_info(logger, "Se enviaron todas las instrucciones y los segmentos!\n");

		char *mensaje = recibirMensaje(conexion);

		log_info(logger, "Mensaje de confirmacion del Kernel : %s\n", mensaje);

		// Falta dejar a la consola en espera de nuevos mensajes del kernel...

		terminar_programa(conexion, logger, config);
	}
}

void leerConfig(char *rutaConfig)
{
	config = iniciar_config(rutaConfig);
	extraerDatosConfig(config);

	printf(PRINT_COLOR_GREEN "\n===== Archivo de configuracion =====\n IP: %s \n PUERTO: %s \n SEGMENTOS: [", configConsola.ipKernel, configConsola.puertoKernel);

	for (int i = 0; i < size_char_array(configConsola.segmentos); i++)
	{
		printf("%s", configConsola.segmentos[i]);

		if ((i + 1) < size_char_array(configConsola.segmentos))
		{
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

t_list *listaSegmentos()
{
	t_list *listaDeSegmentos = list_create();

	for (int i = 0; i < size_char_array(configConsola.segmentos); i++)
	{
		// printf("%d", segmento);
		char *segmento = string_new();
		string_append(&segmento, configConsola.segmentos[i]);
		
		uint32_t segmentoResultado = atoi(segmento);

		list_add(listaDeSegmentos, segmentoResultado);
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

	buffer->size = sizeof(uint32_t)	* 2																  // instrucciones_size
				   + list_size(informacion->instrucciones) * sizeof(t_instruccion)  // segmentos_size
				  + list_size(informacion->segmentos) * sizeof(uint32_t);

	void *stream = malloc(buffer->size);

	int offset = 0; // Desplazamiento
	memcpy(stream + offset, &(informacion->instrucciones->elements_count), sizeof(uint32_t));
	offset += sizeof(uint32_t);

	memcpy(stream + offset, &(informacion->segmentos->elements_count), sizeof(uint32_t));
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
		uint32_t segmento = list_get(informacion->segmentos, j);
		memcpy(stream + offset, &segmento, sizeof(uint32_t));
		
		offset += sizeof(uint32_t);
		j++;
		printf(PRINT_COLOR_YELLOW "Estoy serializando el segmento: %d" PRINT_COLOR_RESET "\n", j);
	}

	buffer->stream = stream; // Payload

	// free(informacion->instrucciones);
	// free(informacion->segmentos);

	// lleno el paquete
	t_paquete *paquete = malloc(sizeof(t_paquete));
	paquete->codigo_operacion = NEW;
	paquete->buffer =  buffer;
	return paquete;
}