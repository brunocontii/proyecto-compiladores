#include <stdio.h>
#include "../codigo-intermedio/generador.h"

extern int ultimo_temp;

void procesar_argumentos(nodo *raiz, FILE *file) {
    if (!raiz) return;
    
    if (raiz->valor->tipo_token == T_EXPRS) {
        procesar_argumentos(raiz->izq, file);
        procesar_argumentos(raiz->der, file);
    } else {
        codigo_intermedio(raiz, file);
        fprintf(file, "PARAM T%d\n", ultimo_temp);
    }
}