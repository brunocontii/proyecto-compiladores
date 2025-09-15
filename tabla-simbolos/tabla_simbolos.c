#include <stdio.h>
#include <string.h>
#include <stdlib.h>

#include "tabla_simbolos.h"

// inicializando la tabla de simbolos
void inicializar(tabla_simbolos *ts) {
    ts->inicio = NULL;
}

// si ya existe retorno 0, sino retorno 1 //
int insertar(tabla_simbolos *ts, info *nuevo) {
    if (!ts || !nuevo) return 0;
    
    simbolo *actual = ts->inicio; // lo arranco desde el principio

    // verificacion de si existe ya en la tabla
    while (actual != NULL) {
        if (strcmp(actual->valor->name, nuevo->name) == 0) {
            printf("identificador %s ya esta declarado.\n", nuevo->name);
            return 0;
        }
        actual = actual->sig;
    }

    // sino existe lo agrego
    simbolo *nuevoNodo = (simbolo*) malloc(sizeof(simbolo));

    nuevoNodo->valor = nuevo; 
    nuevoNodo->sig = ts->inicio;
    ts->inicio = nuevoNodo;

    return 1;
}

info* buscar(tabla_simbolos *ts, char *name) {
    
    simbolo *actual = ts->inicio;

    /* 
        busco por el id del campo info y retorno el info completo que seria la informacion que quiero,
        dentro esta el tipo que es, el token que es y que variable es.
    */
    while (actual != NULL) {
        if (strcmp(actual->valor->name, name) == 0) {
            // printf("Encontre %s ya esta declarada.\n", id);
            return actual->valor;
        }
        actual = actual->sig;
    }

    // sino encuentro es xq no esta declarada, error retorno null
    printf("variable %s no declarada.\n", name);

    return NULL;
}