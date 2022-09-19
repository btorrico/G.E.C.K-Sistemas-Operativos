#include "kernel.h"

int main(char** argc, char ** argv)
{
	 if(argc > 1 && strcmp(argv[1],"-test")==0)
        return run_tests();
    else{  
    //Parte Server
    t_log* logger = log_create("kernel.log", "KERNEL", true, LOG_LEVEL_INFO);
    leerConfig("./kernel.config");

    //Se conecta el Kernel
    conectar_y_mostrar_mensajes_de_cliente(IP, PUERTO);
    logger = log_create("log.log", "Servidor", 1, LOG_LEVEL_DEBUG);
	
	}	
}



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
}

    

