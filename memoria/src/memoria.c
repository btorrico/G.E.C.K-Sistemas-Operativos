#include "memoria.h"

int main(int argc, char ** argv){
	  if(argc > 1 && strcmp(argv[1],"-test")==0)
        return run_tests();
    else{  
    t_log* logger = log_create("kernel.log", "KERNEL", true, LOG_LEVEL_INFO);
      
    //Se conecta el Kernel
    conectar_y_mostrar_mensajes_de_cliente(IP, PUERTO);
   
    } 

}