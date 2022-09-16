#include "kernel.h"

int main(void){
    
    t_log* logger = log_create("kernel.log", "KERNEL", true, LOG_LEVEL_INFO);
      
    conectar_y_mostrar_mensajes_de_cliente(logger);

}
