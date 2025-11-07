#ifndef CODIGO_MUERTO_VAR_H
#define CODIGO_MUERTO_VAR_H

#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include "../arbol-sintactico/arbol.h"

typedef struct variables {
    char *nombre;
    bool se_usa;
    struct variables *next;
} variables;

void recorrer_y_marcar(nodo *raiz);
void recorrer_y_podar(nodo *raiz);
void codigo_muerto_var(nodo * raiz);

#endif