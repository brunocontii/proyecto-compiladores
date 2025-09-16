#include <stdio.h>
#include <string.h>
#include <stdlib.h>

#include "tabla_simbolos.h"

// inicializando la tabla de simbolos con scope global
void inicializar(tabla_simbolos *ts) {
    if (!ts) return;

    // creo el scope global, nivel 0
    scope *scope_global = (scope*)malloc(sizeof(scope));
    scope_global->tabla = NULL;
    scope_global->padre = NULL;
    scope_global->nivel = 0;

    ts->scope_actual = scope_global;
    ts->nivel_actual = 0;
}

// abro un nuevo scope (hijo del actual)
void abrir_scope(tabla_simbolos *ts) {
    if (!ts) return;

    // creo un nuevo scope
    scope *nuevo_scope = (scope*)malloc(sizeof(scope));
    nuevo_scope->tabla = NULL;
    nuevo_scope->padre = ts->scope_actual; // el padre del nuevo scope es el scope actual
    nuevo_scope->nivel = ts->nivel_actual + 1;

    ts->scope_actual = nuevo_scope; // actualizo el scope actual
    ts->nivel_actual++;

    printf("Abierto nuevo scope de nivel %d\n", ts->nivel_actual);
}

// cierro el scope actual y vuelvo al padre
void cerrar_scope(tabla_simbolos *ts) {
    if (!ts || !ts->scope_actual || ts->nivel_actual == 0) return;

    scope *scope_a_cerrar = ts->scope_actual;
    ts->scope_actual = scope_a_cerrar->padre; // vuelvo al scope padre, es decir el anterior
    ts->nivel_actual--;
    free(scope_a_cerrar);   // libero memoria del scope cerrado,
                            // la idea es eliminar unicamente lo que apunta a la tabla (no todos los simbolos dentro de ella),
                            // ya que los simbolos pueden estar siendo apuntados por nodos del AST
    printf("Cerrado scope, volviendo a nivel %d\n", ts->nivel_actual);
}

// si ya existe retorno 0, sino retorno 1
int insertar(tabla_simbolos *ts, info *nuevo) {
    if (!ts || !nuevo || !ts->scope_actual) return 0;
    
    simbolo *actual = ts->scope_actual->tabla; // lo arranco desde el principio

    // verificacion de si existe ya en el scope actual
    while (actual != NULL) {
        if (strcmp(actual->valor->name, nuevo->name) == 0) {
            printf("identificador %s ya esta declarado en este scope (nivel %d).\n", nuevo->name, ts->nivel_actual);
            return 0;
        }
        actual = actual->sig;
    }

    // sino existe en el scope actual lo agrego
    simbolo *nuevoNodo = (simbolo*) malloc(sizeof(simbolo));

    nuevoNodo->valor = nuevo; 
    nuevoNodo->sig = ts->scope_actual->tabla;   // el siguiente del nuevo nodo es el inicio actual
    ts->scope_actual->tabla = nuevoNodo;        // el inicio ahora es el nuevo nodo

    printf("Insertado %s en scope nivel %d.\n", nuevo->name, ts->nivel_actual);
    return 1;
}

info* buscar(tabla_simbolos *ts, char *name) {
    if (!ts || !name || !ts->scope_actual) return NULL;
    
    scope *scope_actual = ts->scope_actual;
    
    // busco desde el scope actual hacia arriba
    while (scope_actual != NULL) {
        simbolo *simbolo_actual = scope_actual->tabla;
        
        // busco en la tabla de simbolos del scope actual
        while (simbolo_actual != NULL) {
            if (strcmp(simbolo_actual->valor->name, name) == 0) {
                printf("Encontrado %s en scope nivel %d.\n", name, scope_actual->nivel);
                return simbolo_actual->valor;
            }
            simbolo_actual = simbolo_actual->sig;
        }
        
        scope_actual = scope_actual->padre; // si no lo encuentro en este scope, subo al padre
    }
    
    // no se encontro en ningun scope
    printf("%s no declarada.\n", name);
    return NULL;
}