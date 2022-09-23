#include "kernel.h"

int main(char** argc, char ** argv)
{
	 if(argc > 1 && strcmp(argv[1],"-test")==0)
        return run_tests();
    else{  

    //Parte Server
    logger = iniciar_logger("kernel.log","KERNEL", LOG_LEVEL_DEBUG);
	
	config = iniciar_config("kernel.config");
    //leerConfig("./kernel.config");


    //Se conecta el Kernel a la consola
	ip_consola = config_get_string_value(config, "IP_CONSOLA");
	printf("%s", ip_consola);
	puerto_escucha = config_get_string_value(config, "PUERTO_ESCUCHA");
    conectar_y_mostrar_mensajes_de_cliente(ip_consola, puerto_escucha, logger);
    
	
	}	
}


/*

void leerConfig(char* archivoConfig){
	config = iniciar_config(archivoConfig);
// Usando el config creado previamente, leemos los valores del config y los
	// dejamos en las variables 'ip', 'puerto' y 'valor'

	ip = config_get_string_value(config, "IP");
	valor = config_get_string_value(config, "CLAVE");
	puerto = config_get_string_value(config, "PUERTO");

	// Loggeamos el valor de config
	//Los colores salen del archivo globals.h :)
	printf(PRINT_COLOR_GREEN"\n===== Archivo de configuracion =====\n IP: %s \n CLAVE: %s \n PUERTO: %s"PRINT_COLOR_RESET,ip,valor,puerto);
}*/

    

