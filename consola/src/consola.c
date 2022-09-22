#include "consola.h"

   
int main(int argc, char ** argv)
{
	 if(argc > 1 && strcmp(argv[1],"-test")==0)
        return run_tests();
    else{  
       /* ---------------- LOGGING ---------------- */


	logger = iniciar_logger("consola.log");

	log_info(logger, "\nIniciando consola...");

	/* ---------------- ARCHIVOS DE CONFIGURACION ---------------- */
	
	obtenerArgumentos(argc,argv); // Recibe 3 argumentos, ./consola, la ruta del archivoConfig y la ruta de las instrucciones de pseudocodigo
	                              //Valida la cantidad de argumentos y utiliza la funcion leerConfig() para leer el 2do parametro
	

	/* ---------------- LEER DE CONSOLA ---------------- */

//	leer_consola(logger);

	

	conexion = crear_conexion(ip, puerto);

	// Enviamos al servidor el valor de CLAVE como mensaje
	enviar_mensaje(valor, conexion);

	// Armamos y enviamos el paquete
	paquete(conexion);

	terminar_programa(conexion, logger, config);

    } 

}

void leerConfig(char* rutaConfig){
	config = iniciar_config(rutaConfig);
    
// Usando el config creado previamente, leemos los valores del config y los
	// dejamos en las variables 'ip', 'puerto' y 'valor'

	ip = config_get_string_value(config, "IP");
	valor = config_get_string_value(config, "CLAVE");
	puerto = config_get_string_value(config, "PUERTO");


	//Los colores salen del archivo globals.h :)
	printf(PRINT_COLOR_GREEN"\n===== Archivo de configuracion =====\n IP: %s \n CLAVE: %s \n PUERTO: %s"PRINT_COLOR_RESET,ip,valor,puerto);
}



void obtenerArgumentos(int argc,char** argv){
	rutaArchivoConfiguracion = "./consola.config";
	rutaInstrucciones = "./pseudocodigo/pseudocodigo";

	if(argc != 3) {
		printf(PRINT_COLOR_RED"\nError: cantidad de argumentos incorrecta:"PRINT_COLOR_RESET"\n");
		//return -1;
		}else{
			rutaArchivoConfiguracion = argv[1];
			rutaInstrucciones = argv[2];
			
			leerConfig(rutaArchivoConfiguracion);

			printf(PRINT_COLOR_GREEN"\nCantidad de argumentos de entrada Correctos!"PRINT_COLOR_RESET"\n");
		}


	printf("=== Argumentos de entrada ===\n rutaArchivoConfig: %s \n rutaArchivoDeInstrucciones: %s \n\n", rutaArchivoConfiguracion, rutaInstrucciones);
}
