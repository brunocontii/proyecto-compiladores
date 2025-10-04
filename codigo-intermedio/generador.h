#ifndef GENERADOR_H
#define GENERADOR_H

#include <stdio.h>
#include "../arbol-sintactico/arbol.h"

typedef struct codigo3dir {
    char instruccion[25];
    char resultado[5];
    char argumento1[5];
    char argumento2[20];
}codigo3dir;

codigo3dir programa[2000]; // 2000 instrucciones maximo
int cont_instrucciones = 0;

void codigo_intermedio(nodo *raiz, FILE *out);
void procesar_argumentos(nodo *args, FILE *file);

#endif