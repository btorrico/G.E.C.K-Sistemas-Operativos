#include "consola.h"

   
int main(char** argc, char ** argv)
{
	 if(argc > 1 && strcmp(argv[1],"-test")==0)
        return run_tests();
    else{  
       /* ---------------- LOGGING ---------------- */

	logger = iniciar_logger("consola.log", "CONSOLA",LOG_LEVEL_INFO);

	log_info(logger, "Hola! Soy la consola");

	/* ---------------- ARCHIVOS DE CONFIGURACION ---------------- */
//Se tiene que leer la configuracion del archivo consola.config (Todavia no existe)
   // leerConfig("./consola.config"); //podria agregarse un parametro que sea archivoConfig cosa de hacerlo mas global,al igual que los parametros ip, puerto y valor, despues ver
	
	/* ---------------- LEER DE CONSOLA ---------------- */

//	leer_consola(logger);

	/*---------------------------------------------------PARTE 3-------------------------------------------------------------*/
	config = iniciar_config("consola.config");
	ip_kernel = config_get_string_value(config, "IP_KERNEL");
	puerto_kernel = config_get_string_value(config, "PUERTO_KERNEL");
	conexion = crear_conexion(ip_kernel, puerto_kernel);

	// Enviamos al servidor el valor de CLAVE como mensaje
	//enviar_mensaje(segmentos, conexion);

	// Armamos y enviamos el paquete
	paquete(conexion);

	terminar_programa(conexion, logger, config);

    } 

}

void leerConfig(char* archivoConfig){
	config = iniciar_config(archivoConfig);
// Usando el config creado previamente, leemos los valores del config y los
	// dejamos en las variables 'ip', 'puerto' y 'valor'

	ip_kernel = config_get_string_value(config, "IP_KERNEL");
	puerto_kernel = config_get_string_value(config, "PUERTO_KERNEL");

/*	buffer = config_get_array_value(config, "SEGMENTOS");
	for(int i=0; i<string_length(buffer);i++){
		list_add(segmentos, buffer[i]);
	}

	segmentos->elements_count=list_size(segmentos);*/
	// Loggeamos el valor de config
	//Los colores salen del archivo globals.h :)
	printf(PRINT_COLOR_GREEN"\n===== Archivo de configuracion =====\n IP_KERNEL: %s \n PUERTO_KERNEL: %s"PRINT_COLOR_RESET,ip_kernel,puerto_kernel);
}
