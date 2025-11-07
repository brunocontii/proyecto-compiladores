#ifndef OPERACIONES_H
#define OPERACIONES_H

#include "../arbol-sintactico/arbol.h"

// funcion principal: aplica todas las optimizaciones de operaciones
void optimizaciones_operaciones(nodo *raiz);

// funciones individuales: optimizaciones de operaciones
void valores_neutros(nodo *n);
void reducciones_simples(nodo *n);
void reducciones_dominantes(nodo *n);
void comparaciones_redundantes(nodo *n);

#endif