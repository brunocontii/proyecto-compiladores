#ifndef CODIGO_MUERTO_BLOQUE_H
#define CODIGO_MUERTO_BLOQUE_H

#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include "../arbol-sintactico/arbol.h"

// Recibe el árbol sintáctico raíz, y poda si es necesario
nodo* eliminarBloquesMuertos(nodo *raiz);

#endif