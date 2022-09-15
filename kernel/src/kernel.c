#include "../include/kernel.h"

int main(int argc, char ** argv){
    if(argc > 1 && strcmp(argv[1],"-test")==0)
        return run_tests();
    else{  
        t_log* logger = log_create("./kernel.log", "KERNEL", true, LOG_LEVEL_INFO);
        printf("esta ok");
        log_info(logger, "Ser o no ser, esa es la cuestion, en fin, soy el Kernel %s", mi_funcion_compartida());
        log_destroy(logger);
    } 

}