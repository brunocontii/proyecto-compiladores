#ifndef OPTIMIZACIONES_H
#define OPTIMIZACIONES_H

#include <stdbool.h>
#include "../arbol-sintactico/arbol.h"

// aplicar todas las optimizaciones habilitadas
void aplicar_optimizaciones(nodo *raiz);

// propagacion de constantes
void propagacion_constantes(nodo *raiz);

#endif