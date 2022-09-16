#include "kernel.h"

int main(void){
    
    t_log* logger = log_create("kernel.log", "KERNEL", true, LOG_LEVEL_INFO);
      
    int server_fd = iniciar_servidor();
	log_info(logger, "Servidor listo para recibir al cliente");

}
