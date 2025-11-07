#ifndef OPTIMIZACIONES_H
#define OPTIMIZACIONES_H

#include <stdbool.h>
#include "../arbol-sintactico/arbol.h"

// aplicar todas las optimizaciones habilitadas
void aplicar_optimizaciones(nodo *raiz);

// propagacion de constantes
void propagacion_constantes(nodo *raiz);

// eliminacion de codigo muerto
void codigo_muerto_var(nodo * raiz);

// eliminacion de codigo inalcanzable
void eliminarCodigoMuerto(nodo * raiz);

#endif