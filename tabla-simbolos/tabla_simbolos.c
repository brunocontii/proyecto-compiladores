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

void imprimir_scope_actual(tabla_simbolos *ts) {
    if (!ts || !ts->scope_actual) {
        printf("No hay tabla de símbolos o scope actual.\n");
        return;
    }

    printf("\n=== SCOPE NIVEL %d ===\n", ts->nivel_actual);
    
    simbolo *actual = ts->scope_actual->tabla;
    int contador = 0;
    
    if (actual == NULL) {
        printf("(Scope vacío)\n");
        printf("========================\n\n");
        return;
    }
    
    while (actual != NULL) {
        contador++;
        printf("Símbolo %d:\n", contador);
        
        // Campo name (si no es NULL)
        if (actual->valor->name) {
            printf("  Campo name: %s\n", actual->valor->name);
        }
        
        // Campo op (si no es NULL)
        if (actual->valor->op) {
            printf("  Campo op: %s\n", actual->valor->op);
        }
        
        // Campo bool_string (si no es NULL)
        if (actual->valor->bool_string) {
            printf("  Campo bool_string: %s\n", actual->valor->bool_string);
        }
        
        // Campo nro (solo si tipo_token es T_DIGIT)
        if (actual->valor->tipo_token == T_DIGIT) {
            printf("  Campo nro: %d\n", actual->valor->nro);
        }
        
        // Campo b (solo si tipo_token es T_BOOL o relacionados)
        if (actual->valor->tipo_token == T_BOOL || 
            actual->valor->tipo_token == T_VTRUE || 
            actual->valor->tipo_token == T_VFALSE) {
            printf("  Campo b: %s\n", actual->valor->b ? "true" : "false");
        }

        // Campo tipo_token
        printf("  Campo tipo_token: ");
        switch (actual->valor->tipo_token) {
            case T_PROGRAM: printf("PROGRAM"); break;
            case T_EXTERN: printf("EXTERN"); break;
            case T_RETURN: printf("RETURN"); break;
            case T_IF: printf("IF"); break;
            case T_IF_ELSE: printf("IF_ELSE"); break;
            case T_WHILE: printf("WHILE"); break;
            case T_VTRUE: printf("VTRUE"); break;
            case T_VFALSE: printf("VFALSE"); break;
            case T_OP_AND: printf("OP_AND"); break;
            case T_OP_OR: printf("OP_OR"); break;
            case T_OP_NOT: printf("OP_NOT"); break;
            case T_INTEGER: printf("INTEGER"); break;
            case T_BOOL: printf("BOOL"); break;
            case T_ID: printf("ID"); break;
            case T_DIGIT: printf("DIGIT"); break;
            case T_OP_MENOS: printf("OP_MENOS"); break;
            case T_OP_MAS: printf("OP_MAS"); break;
            case T_OP_MULT: printf("OP_MULT"); break;
            case T_OP_DIV: printf("OP_DIV"); break;
            case T_OP_RESTO: printf("OP_RESTO"); break;
            case T_ASIGNACION: printf("ASIGNACION"); break;
            case T_IGUALDAD: printf("IGUALDAD"); break;
            case T_MENOR: printf("MENOR"); break;
            case T_MAYOR: printf("MAYOR"); break;
            case T_VAR_DECLS: printf("VAR_DECLS"); break;
            case T_VAR_DECL: printf("VAR_DECL"); break;
            case T_METHOD_DECLS: printf("METHOD_DECLS"); break;
            case T_METHOD_DECL: printf("METHOD_DECL"); break;
            case T_PARAMETROS: printf("PARAMETROS"); break;
            case T_BLOQUE: printf("BLOQUE"); break;
            case T_METHOD_CALL: printf("METHOD_CALL"); break;
            case T_EXPRS: printf("EXPRS"); break;
            case T_STATEMENTS: printf("STATEMENTS"); break;
            default: printf("DESCONOCIDO (%d)", actual->valor->tipo_token);
        }
        printf("\n");
        
        // Campo tipo_info
        printf("  Campo tipo_info: ");
        switch (actual->valor->tipo_info) {
            case TIPO_INTEGER: printf("INTEGER"); break;
            case TIPO_BOOL: printf("BOOL"); break;
            case TIPO_VOID: printf("VOID"); break;
            default: printf("DESCONOCIDO (%d)", actual->valor->tipo_info);
        }
        printf("\n");
        
        printf("  ---\n");
        actual = actual->sig;
    }
    
    printf("Total símbolos en scope: %d\n", contador);
    printf("========================\n\n");
}
