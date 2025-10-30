#ifndef AUXILIARES_H
#define AUXILIARES_H

#include "../arbol-sintactico/arbol.h"
#include "codigo3dir.h"

// metodos de auxiliares.c
info* obtener_temp(int n, tipo_info ti);
info* crear_constante(int nro);
info* obtener_label(const char *label);

// metodos de parametros.c
int contar_parametros_metodo(nodo *params);
void procesar_parametros_declaracion(nodo *params, int cant_params);

#endif