#ifndef TABLA_SIMBOLOS_H
#define TABLA_SIMBOLOS_H

#include "../arbol-sintactico/arbol.h"

typedef struct simbolo {
    info *valor;
    struct simbolo *sig;
} simbolo;

typedef struct {
    simbolo *inicio;
} tabla_simbolos;

void inicializar(tabla_simbolos *ts);
int insertar(tabla_simbolos *ts, info *nuevo);
info* buscar(tabla_simbolos *ts, char *nombre);

#endif