#ifndef GENERADOR_H
#define GENERADOR_H

#include <stdio.h>
#include "../arbol-sintactico/arbol.h"

void codigo_intermedio(nodo *raiz, FILE *out);
void procesar_argumentos(nodo *args, FILE *file);

#endif