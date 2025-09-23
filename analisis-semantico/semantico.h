#ifndef SEMANTICO_H
#define SEMANTICO_H

#include "../arbol-sintactico/arbol.h"
#include "../tabla-simbolos/tabla_simbolos.h"

void recorridoSemantico(nodo *raiz, tabla_simbolos *ts);

#endif