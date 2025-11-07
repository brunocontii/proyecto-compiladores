#ifndef OPERACIONES_H
#define OPERACIONES_H

#include "../arbol-sintactico/arbol.h"

nodo* valores_neutros(nodo *n);

nodo* reducciones_simples(nodo *n);

nodo* reducciones_dominantes(nodo *n);

nodo* comparaciones_redundantes(nodo *n);

#endif