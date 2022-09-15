
#include "../include/consola.h"

int main(int argc, char ** argv){
    if(argc > 1 && strcmp(argv[1],"-test")==0)
        return run_tests();
    else{  
        t_log* logger = log_create("./consola.log", "CONSOLA", true, LOG_LEVEL_INFO);
        printf("esta ok");
        log_info(logger, "Soy la consola! %s", mi_funcion_compartida());
        log_destroy(logger);
    } 

    int a=5;
    int b=10;

    int* c = suma_alloc(a,b);
    printf("%d",*c);
}




int* suma_alloc(int a ,int b){
   int* ret = malloc(sizeof(int));
   *ret = a+b;
   return ret;
}
