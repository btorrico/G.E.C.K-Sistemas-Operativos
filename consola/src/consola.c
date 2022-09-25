#include "consola.h"

int main(int argc, char **argv)
{
	if (argc > 1 && strcmp(argv[1], "-test") == 0)
		return run_tests();
	else
	{
		/* ---------------- LOGGING ---------------- */

		logger = iniciar_logger("consola.log", "CONSOLA", LOG_LEVEL_DEBUG);

		log_info(logger, "\nIniciando consola...");

		/* ---------------- ARCHIVOS DE CONFIGURACION ---------------- */
printf("hola");
		//obtenerArgumentos(argc, argv); // Recibe 3 argumentos, ./consola, la ruta del archivoConfig y la ruta de las instrucciones de pseudocodigo
									   // Valida la cantidad de argumentos y utiliza la funcion leerConfig() para leer el 2do parametro

		/* ---------------- LEER DE CONSOLA ---------------- */
	//config = iniciar_config("consola.config");
	
		FILE *instructionsFile = abrirArchivo(argv[2]);

		t_list *instrucciones = list_create();
		
		agregarInstruccionesDesdeArchivo(instructionsFile, instrucciones);
		list_get(instrucciones, 1);

		conexion = crear_conexion(configConsola.ipKernel, configConsola.puertoKernel);

		// Enviamos al servidor el valor de CLAVE como mensaje
		//enviar_mensaje(valor, conexion);

		// Armamos y enviamos el paquete
		paquete(conexion);

		terminar_programa(conexion, logger, config);
	}
}

void leerConfig(char *rutaConfig)
{
	config = iniciar_config(rutaConfig);
	extraerDatosConfig(rutaConfig);
	// Usando el config creado previamente, leemos los valores del config y los
	// dejamos en las variables 'ip', 'puerto' y 'valor'

	// ip = config_get_string_value(config, "IP");
	// valor = config_get_string_value(config, "CLAVE");
	// puerto = config_get_string_value(config, "PUERTO");

	printf(PRINT_COLOR_GREEN "\n===== Archivo de configuracion =====\n IP: %s \n PUERTO: %s" PRINT_COLOR_RESET, configConsola.ipKernel, configConsola.puertoKernel);
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


void agregarInstruccionesDesdeArchivo(FILE *instructionsFile, t_list *instrucciones)
{
	if (instructionsFile == NULL)
	{
		log_error(logger,
				  "Error: no se pudo abrir el archivo de instrucciones.");
		exit(1);
	}
	const unsigned MAX_LENGTH = 256;
	char buffer[MAX_LENGTH];
	while (fgets(buffer, 256, instructionsFile) != NULL)
	{
		int i = 0;
		char **palabra = string_split(buffer, " ");
		t_instruccion *instr = malloc(sizeof(t_instruccion));

		if (strcmp(palabra[i], "SET") == 0)
		{
			instr->instCode = SET;
			instr->paramChar[0] = palabra[1];
			instr->paramInt = atoi(palabra[2]);
			free(palabra[0]);
			free(palabra[1]);
			free(palabra[2]);
		}
		else if (strcmp(palabra[0], "ADD") == 0)
		{
			instr->instCode = ADD;
			instr->paramChar[0] = palabra[1];
			instr->paramChar[1]= palabra[2];
			free(palabra[0]);
			free(palabra[1]);
			free(palabra[2]);
		}
		else if (strcmp(palabra[0], "MOV_IN") == 0)
		{
			instr->instCode = MOV_IN;
			instr->paramChar[0] = palabra[1];
			instr->paramInt = atoi(palabra[2]);
			free(palabra[0]);
			free(palabra[1]);
			free(palabra[2]);
		}
		else if (strcmp(palabra[0], "MOV_OUT") == 0)
		{
			instr->instCode = MOV_OUT;
			instr->paramInt = atoi(palabra[1]);
			instr->paramChar[0] = palabra[2];
			free(palabra[0]);
			free(palabra[1]);
			free(palabra[2]);
		}
		else if (strcmp(palabra[0], "I/O") == 0)
		{
			instr->instCode = IO;
			instr->paramChar[0] = palabra[1];
			instr->paramInt = atoi(palabra[2]);
			free(palabra[0]);
			free(palabra[1]);
			free(palabra[2]);
		}
		else if (strcmp(palabra[0], "EXIT") == 0)
		{
			instr->instCode = EXIT;
			instr->paramChar[0] = NULL; // TODO : checkear esto!!! tira un warning por el tipo de dato!!!
			instr->paramChar[1] = NULL; // Tengo alergia a los warnings(?
			free(palabra[0]);
		}
		list_add(instrucciones, instr);

		free(palabra);
	}
	fclose(instructionsFile);
	log_info(logger, "Se parsearon #Instrucciones: %d", list_size(instrucciones));

}

t_configConsola extraerDatosConfig(t_config *archivoConfig)
{
	configConsola.ipKernel = string_new();
	configConsola.puertoKernel = string_new();


	configConsola.ipKernel = config_get_string_value(archivoConfig, "IP_KERNEL");
	configConsola.puertoKernel = config_get_string_value(archivoConfig, "PUERTO_KERNEL");
	

	return configConsola;
}