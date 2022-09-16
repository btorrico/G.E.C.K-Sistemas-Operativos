#include "consola.h"

int main(void)
{

	/* ---------------- LOGGING ---------------- */

	logger = iniciar_logger("consola.log");

	log_info(logger, "Hola! Soy la consola");

	/* ---------------- ARCHIVOS DE CONFIGURACION ---------------- */
//Se tiene que leer la configuracion del archivo consola.config (Todavia no existe)
    leerConfig(); //podria agregarse un parametro que sea archivoConfig cosa de hacerlo mas global,al igual que los parametros ip, puerto y valor, despues ver
	
	/* ---------------- LEER DE CONSOLA ---------------- */

//	leer_consola(logger);

	/*---------------------------------------------------PARTE 3-------------------------------------------------------------*/


	conexion = crear_conexion(ip, puerto);

	// Enviamos al servidor el valor de CLAVE como mensaje
	enviar_mensaje(valor, conexion);

	// Armamos y enviamos el paquete
	paquete(conexion);

	terminar_programa(conexion, logger, config);

	
}


void leerConfig(){
	config = iniciar_config("./consola.config");
// Usando el config creado previamente, leemos los valores del config y los
	// dejamos en las variables 'ip', 'puerto' y 'valor'

	ip = config_get_string_value(config, "IP");
	valor = config_get_string_value(config, "CLAVE");
	puerto = config_get_string_value(config, "PUERTO");

	// Loggeamos el valor de config
	//Los colores salen del archivo globals.h :)
	printf(PRINT_COLOR_GREEN"\n===== Archivo de configuracion =====\n IP: %s \n CLAVE: %s \n PUERTO: %s"PRINT_COLOR_RESET,ip,valor,puerto);
}