#ifndef TABLA_SIMBOLOS_H
#define TABLA_SIMBOLOS_H

#include "../arbol-sintactico/arbol.h"

typedef struct simbolo {
    info *valor;
    struct simbolo *sig;
} simbolo;

typedef struct scope {
    simbolo *tabla;               // lista de símbolos de este scope
    struct scope *padre;          // puntero al scope padre
    int nivel;                    // nivel de anidamiento (0=global, 1=función, 2=bloque, etc.)
} scope;

typedef struct tabla_simbolos {
    scope *scope_actual;          // "tope de la pila" de scopes
    int nivel_actual;             // nivel actual
} tabla_simbolos;

void inicializar(tabla_simbolos *ts);
int insertar(tabla_simbolos *ts, info *nuevo);
info* buscar(tabla_simbolos *ts, char *nombre);
void abrir_scope(tabla_simbolos *ts);
void cerrar_scope(tabla_simbolos *ts);
void imprimir_scope_actual(tabla_simbolos *ts);

#endif