#include <stdio.h>
#include "../codigo-intermedio/generador.h"
extern int ultimo_temp;

void procesar_argumentos(nodo *raiz, FILE *file) {
    if (!raiz) return;
    
    if (raiz->valor->tipo_token == T_EXPRS) {
        procesar_argumentos(raiz->izq, file);
        codigo_intermedio(raiz->der, file);

        codigo3dir inst;
        strcpy(inst.instruccion, "PARAM");
        sprintf(inst.resultado, "T%d", ultimo_temp);
        inst.argumento1[0] = '\0';
        inst.argumento2[0] = '\0';

        programa[cont_instrucciones] = inst;
        cont_instrucciones++;

        fprintf(file, "PARAM T%d\n", ultimo_temp);
    } else {
        codigo_intermedio(raiz, file);

        codigo3dir inst;
        strcpy(inst.instruccion, "PARAM");
        sprintf(inst.resultado, "T%d", ultimo_temp);
        inst.argumento1[0] = '\0';
        inst.argumento2[0] = '\0';

        programa[cont_instrucciones] = inst;
        cont_instrucciones++;

        fprintf(file, "PARAM T%d\n", ultimo_temp);
    }
}