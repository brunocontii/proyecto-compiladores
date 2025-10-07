#ifndef GENERADOR_H
#define GENERADOR_H

#include <stdio.h>
#include "../arbol-sintactico/arbol.h"

typedef struct codigo3dir {
    char instruccion[25];
    char resultado[10];
    char argumento1[10];
    char argumento2[20];
}codigo3dir;

extern codigo3dir programa[2000];
extern int cont_instrucciones;

void codigo_intermedio(nodo *raiz, FILE *out);
void procesar_argumentos(nodo *args, FILE *file);

#endif