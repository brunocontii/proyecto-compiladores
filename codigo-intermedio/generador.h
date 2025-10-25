#ifndef GENERADOR_H
#define GENERADOR_H

#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include "../arbol-sintactico/arbol.h"
#include "codigo3dir.h"

// contadores para temporales y labels
extern int cont_temp;
extern int cont_label;
extern int ultimo_temp;

// funciones auxiliares para crear *info y procesar parametros en la llamada a un metodo
info* obtener_temp(int n, tipo_info ti);
info* crear_constante(int nro);
info* crear_constante_bool(bool b);
info* obtener_label(const char *label);
void procesar_argumentos(nodo *args);

// funcion principal de generacion de codigo intermedio
void codigo_intermedio(nodo *raiz);

#endif