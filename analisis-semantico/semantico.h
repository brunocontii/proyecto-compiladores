#ifndef SEMANTICO_H
#define SEMANTICO_H

#include "../arbol-sintactico/arbol.h"
#include "../tabla-simbolos/tabla_simbolos.h"

void recorridoSemantico(nodo *raiz, tabla_simbolos *ts);
tipo_info calcular_tipo_expresion(nodo *expr, tabla_simbolos *ts);
tipo_info retorno_bloque(nodo *bloque, tabla_simbolos *ts);
info* crear_constante2(int nro);
info* crear_constante_bool2(bool b);
void plegar_constantes(nodo *raiz);

#endif