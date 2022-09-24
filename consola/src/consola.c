#include "consola.h"

int main(char **argc, char **argv)
{
	if (argc > 1 && strcmp(argv[1], "-test") == 0)
		return run_tests();
	else
	{
		/* ---------------- LOGGING ---------------- */

		logger = iniciar_logger("consola.log", "CONSOLA", LOG_LEVEL_DEBUG);

		log_info(logger, "Hola! Soy la consola");

		/*---------------------------------------------------PARTE 3-------------------------------------------------------------*/
		config = iniciar_config("consola.config");

		extraerDatosConfig(config);
		conexion = crear_conexion(configConsola.ipKernel, configConsola.puertoKernel);

		paquete(conexion);

		terminar_programa(conexion, logger, config);
	}
}

t_configConsola extraerDatosConfig(t_config *archivoConfig)
{
	configConsola.ipKernel = string_new();
	configConsola.puertoKernel = string_new();


	configConsola.ipKernel = config_get_string_value(archivoConfig, "IP_KERNEL");
	configConsola.puertoKernel = config_get_string_value(archivoConfig, "PUERTO_KERNEL");
	

	return configConsola;
}