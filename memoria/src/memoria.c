#include "memoria.h"

int main(int argc, char ** argv){
    if(argc > 1 && strcmp(argv[1],"-test")==0)
        return run_tests();
    else{  
        t_log* logger = log_create("./memoria.log", "MEMORIA", true, LOG_LEVEL_INFO);
        printf("esta ok");
        log_info(logger, "Quien soy? ah si, la memoria! %s", mi_funcion_compartida());
        log_destroy(logger);
    } 

}