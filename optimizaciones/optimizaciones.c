#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include <string.h>
#include "optimizaciones.h"
#include "codigo_muerto_var.h"
#include "codigo_muerto.h"
#include "../analisis-semantico/semantico.h"
#include "../codigo-intermedio/auxiliares.h"

// variable externa para saber si la optimizacion fue activada
extern bool opt_constant_folding;
extern bool opt_codigo_muerto_var;
extern bool opt_codigo_muerto_codigo_inalcanzable;

// crear constante booleana. el crear constante de enteros ya existe en auxiliares.c
static info* crear_constante_bool(bool b) {
    info *c = (info*)malloc(sizeof(info));
    
    if (b) {
        c->name = strdup("true");
        c->b = true;
        c->tipo_token = T_VTRUE;
    } else {
        c->name = strdup("false");
        c->b = false;
        c->tipo_token = T_VFALSE;
    }
    
    c->nro = b ? 1 : 0;
    c->esTemporal = 0;
    c->tipo_info = TIPO_BOOL;
    
    return c;
}

// verificar si un nodo es literal
static bool es_literal(nodo *expr) {
    if (!expr || !expr->valor) return false;
    return (expr->valor->tipo_token == T_DIGIT ||
            expr->valor->tipo_token == T_VTRUE ||
            expr->valor->tipo_token == T_VFALSE);
}

void plegar_constantes(nodo *raiz) {

    if (!raiz || !raiz->valor) return;

    long long resultado_plegado = 0;
    bool resultado_es_booleano = false;
    bool pliegue = false;
    
    nodo *izq = raiz->izq;
    nodo *der = raiz->der;

    switch (raiz->valor->tipo_token) {

        // casos unarios, el menos unario y not
        case T_OP_NOT:
        case T_OP_MENOS: {
            if (raiz->izq == NULL && es_literal(der)) {
                pliegue = true;
                if (raiz->valor->tipo_token == T_OP_NOT) { // caso not
                    resultado_es_booleano = true;
                    bool der_val = der->valor->tipo_token == T_VTRUE;
                    resultado_plegado = !der_val;
                } else { // caso menos unario
                    resultado_plegado = -(der->valor->nro);
                }
            }
            break;
        }

        case T_OP_MAS:
        case T_OP_MULT:
        case T_OP_DIV:
        case T_OP_RESTO:
        case T_OP_AND:
        case T_OP_OR:
        case T_IGUALDAD:
        case T_MAYOR:
        case T_MENOR: {
            if (!es_literal(izq) || !es_literal(der)) {
                return;
            }
            
            pliegue = true;
            
            long long izq_val = (izq->valor->tipo_token == T_DIGIT) ? izq->valor->nro : (izq->valor->tipo_token == T_VTRUE ? 1 : 0);
            long long der_val = (der->valor->tipo_token == T_DIGIT) ? der->valor->nro : (der->valor->tipo_token == T_VTRUE ? 1 : 0);

            switch (raiz->valor->tipo_token) {
                // aritmeticos
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
                    resultado_plegado = izq_val / der_val;
                    break;
                case T_OP_RESTO:
                    resultado_plegado = izq_val % der_val;
                    break;

                // logico y comparativo(mayor,menor,igualdad)
                case T_OP_AND:
                    resultado_plegado = izq_val && der_val;
                    resultado_es_booleano = true;
                    break;
                case T_OP_OR:
                    resultado_plegado = izq_val || der_val;
                    resultado_es_booleano = true;
                    break;
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

                default: pliegue = false;
                return;
            }
            break;
        }
        default: return;
    }

    // si se puede plegar xq son dos literales, se crea un nuevo info entero o booleano
    if (pliegue) {
        info *nuevo_valor;
        if (resultado_es_booleano) {
            nuevo_valor = crear_constante_bool(resultado_plegado == 1);
        } else {
            nuevo_valor = crear_constante((int)resultado_plegado);
        }
        
        raiz->valor = nuevo_valor;
    }
}

// propagacion de constantes (recorre todo el arbol)
void propagacion_constantes(nodo *raiz) {
    if (!raiz) return;
    
    // primero optimizar subarboles
    propagacion_constantes(raiz->izq);
    propagacion_constantes(raiz->med);
    propagacion_constantes(raiz->der);
    
    // luego plegar constantes en este nodo si es posible
    plegar_constantes(raiz);
}

// aplicar todas las optimizaciones habilitadas
void aplicar_optimizaciones(nodo *raiz) {
    if (!raiz) return;

    if (opt_constant_folding) {
        propagacion_constantes(raiz);
    }

    if (opt_codigo_muerto_var) {
        codigo_muerto_var(raiz);
    }

    if (opt_codigo_muerto_codigo_inalcanzable) {
        eliminarCodigoMuerto(raiz);
    }
    // aca irian mas optimizaciones en el futuro, ej
    // if (opt_dead_code) { ... }
}
