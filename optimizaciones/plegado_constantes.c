#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include <string.h>
#include "plegado_constantes.h"
#include "../analisis-semantico/semantico.h"
#include "../codigo-intermedio/auxiliares.h"

// crear constante booleana
static info* crear_constante_bool(bool b) {
    info *booleano = (info*)malloc(sizeof(info));
    
    if (b) {
        booleano->bool_string = strdup("true");
        booleano->name = strdup("true");
        booleano->b = true;
        booleano->tipo_token = T_VTRUE;
    } else {
        booleano->bool_string = strdup("false"); 
        booleano->name = strdup("false");
        booleano->b = false;
        booleano->tipo_token = T_VFALSE;
    }
    
    booleano->tipo_info = TIPO_BOOL;
    booleano->esTemporal = 0;
    
    return booleano;
}

// verificar si un nodo es literal
static bool es_literal(nodo *expr) {
    if (!expr || !expr->valor) return false;
    return (expr->valor->tipo_token == T_DIGIT ||
            expr->valor->tipo_token == T_VTRUE ||
            expr->valor->tipo_token == T_VFALSE);
}

// liberar info sin liberar sus campos
static void liberar_info(info *i) {
    if (!i) return;
    if (i->name) free(i->name);
    if (i->bool_string) free(i->bool_string);
    if (i->op) free(i->op);
    free(i);
}

// plegado de constantes en un nodo dado
static void plegar_constantes(nodo *raiz) {
    if (!raiz || !raiz->valor) return;

    long long resultado_plegado = 0;
    bool resultado_es_booleano = false;
    bool pliegue = false;
    
    nodo *izq = raiz->izq;
    nodo *der = raiz->der;

    // operandos unarios
    if (izq == NULL && der != NULL && es_literal(der)) {
        tipo_token op = raiz->valor->tipo_token;
        
        if (op == T_OP_NOT) {
            pliegue = true;
            resultado_es_booleano = true;
            bool der_val = der->valor->tipo_token == T_VTRUE;
            resultado_plegado = !der_val;
        } else if (op == T_OP_MENOS) {
            pliegue = true;
            resultado_plegado = -(der->valor->nro);
        }
    }
    // operadores binarios
    else if (izq != NULL && der != NULL && es_literal(izq) && es_literal(der)) {
        tipo_token op = raiz->valor->tipo_token;
        pliegue = true;
        
        long long izq_val = (izq->valor->tipo_token == T_DIGIT) ? 
                            izq->valor->nro : 
                            (izq->valor->tipo_token == T_VTRUE ? 1 : 0);
        long long der_val = (der->valor->tipo_token == T_DIGIT) ? 
                            der->valor->nro : 
                            (der->valor->tipo_token == T_VTRUE ? 1 : 0);

        switch (op) {
            // operaciones aritmeticas
            case T_OP_MAS:
                resultado_plegado = izq_val + der_val;
                break;
            case T_OP_MENOS:
                resultado_plegado = izq_val - der_val;
                break;
            case T_OP_MULT:
                resultado_plegado = izq_val * der_val;
                break;
            case T_OP_DIV:
                if (der_val == 0) {
                    pliegue = false;
                    return;
                }
                resultado_plegado = izq_val / der_val;
                break;
            case T_OP_RESTO:
                if (der_val == 0) {
                    pliegue = false;
                    return;
                }
                resultado_plegado = izq_val % der_val;
                break;

            // operaciones logicas
            case T_OP_AND:
                resultado_plegado = izq_val && der_val;
                resultado_es_booleano = true;
                break;
            case T_OP_OR:
                resultado_plegado = izq_val || der_val;
                resultado_es_booleano = true;
                break;

            // operaciones de comparacion
            case T_IGUALDAD:
                resultado_plegado = (izq_val == der_val);
                resultado_es_booleano = true;
                break;
            case T_MAYOR:
                resultado_plegado = (izq_val > der_val);
                resultado_es_booleano = true;
                break;
            case T_MENOR:
                resultado_plegado = (izq_val < der_val);
                resultado_es_booleano = true;
                break;
            default:
                pliegue = false;
                return;
        }
    }

    // reemplazar el nodo con el resultado plegado
    if (pliegue) {
        // liberar el valor anterior del nodo
        liberar_info(raiz->valor);
        
        // crear el nuevo valor
        if (resultado_es_booleano) {
            raiz->valor = crear_constante_bool(resultado_plegado != 0);
        } else {
            raiz->valor = crear_constante((int)resultado_plegado);
        }
        
        // liberar los hijos (ya no se necesitan)
        if (raiz->izq) {
            liberarArbol(raiz->izq);
            raiz->izq = NULL;
        }
        if (raiz->der) {
            liberarArbol(raiz->der);
            raiz->der = NULL;
        }
        if (raiz->med) {
            liberarArbol(raiz->med);
            raiz->med = NULL;
        }
    }
}

// funcion principal: aplica propagacion de constantes en todo el arbol
void propagacion_constantes(nodo *raiz) {
    if (!raiz) return;
    
    // recorrido bottom-up: primero optimizar subarboles
    propagacion_constantes(raiz->izq);
    propagacion_constantes(raiz->med);
    propagacion_constantes(raiz->der);

    // luego plegar constantes en este nodo si es posible
    plegar_constantes(raiz);
}