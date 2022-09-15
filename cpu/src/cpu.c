#include "../include/cpu.h"

int main(int argc, char ** argv){
    if(argc > 1 && strcmp(argv[1],"-test")==0)
        return run_tests();
    else{  
        t_log* logger = log_create("./cpu.log", "CPU", true, LOG_LEVEL_INFO);
        printf("esta ok");
        log_info(logger, "Soy la Cpu! %s", mi_funcion_compartida());
        log_destroy(logger);
    } 

}