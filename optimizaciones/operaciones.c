#include "operaciones.h"
#include <string.h>
#include <stdlib.h>
#include "../arbol-sintactico/arbol.h"

nodo* valores_neutros(nodo *n) {
    if (n == NULL || n->valor == NULL) return NULL;

    if (n->izq) n->izq = valores_neutros(n->izq);
    if (n->med) n->med = valores_neutros(n->med);
    if (n->der) n->der = valores_neutros(n->der);

    // suma con 0
    if (n->valor->tipo_token == T_OP_MAS) {
        if (n->izq && n->izq->valor && n->izq->valor->tipo_token == T_DIGIT && n->izq->valor->nro == 0) {
            liberarArbol(n->izq);
            nodo *nuevoNodo = n->der;
            if (n->valor->op) free(n->valor->op);
            free(n->valor);
            free(n);
            return nuevoNodo;
        } else if (n->der && n->der->valor && n->der->valor->tipo_token == T_DIGIT && n->der->valor->nro == 0) {
            liberarArbol(n->der);
            nodo *nuevoNodo = n->izq;
            if (n->valor->op) free(n->valor->op);
            free(n->valor);
            free(n);
            return nuevoNodo;
        }
    }

    // resta con 0
    if (n->valor->tipo_token == T_OP_MENOS) { 
        if (n->izq && n->der) { // si es binario, y caso resta x - 0 = x
            if (n->der && n->der->valor && n->der->valor->tipo_token == T_DIGIT && n->der->valor->nro == 0) {
                liberarArbol(n->der);
                nodo *nuevoNodo = n->izq;
                if (n->valor->op) free(n->valor->op);
                free(n->valor);
                free(n);
                return nuevoNodo;
            }
        }
    }

    // multiplicacion por 1
    if (n->valor->tipo_token == T_OP_MULT) {
        if (n->izq && n->izq->valor && n->izq->valor->tipo_token == T_DIGIT && n->izq->valor->nro == 1) {
            liberarArbol(n->izq);
            nodo *nuevoNodo = n->der;
            if (n->valor->op) free(n->valor->op);
            free(n->valor);
            free(n);
            return nuevoNodo;
        } else if (n->der && n->der->valor && n->der->valor->tipo_token == T_DIGIT && n->der->valor->nro == 1) {
            liberarArbol(n->der);
            nodo *nuevoNodo = n->izq;
            if (n->valor->op) free(n->valor->op);
            free(n->valor);
            free(n);
            return nuevoNodo;
        }
    }

    // division por 1
    if (n->valor->tipo_token == T_OP_DIV) {
        // solo tiene sentido si el derecho es valor 1
        if (n->der && n->der->valor && n->der->valor->tipo_token == T_DIGIT && n->der->valor->nro == 1) {
            liberarArbol(n->der);
            nodo *nuevoNodo = n->izq;
            if (n->valor->op) free(n->valor->op);
            free(n->valor);
            free(n);
            return nuevoNodo;
        }
    }

    // AND con true
    if (n->valor->tipo_token == T_OP_AND) {
        if (n->izq && n->izq->valor && n->izq->valor->tipo_token == T_VTRUE) {
            liberarArbol(n->izq);
            nodo *nuevoNodo = n->der;
            if (n->valor->op) free(n->valor->op);
            free(n->valor);
            free(n);
            return nuevoNodo;
        } else if (n->der && n->der->valor && n->der->valor->tipo_token == T_VTRUE) {
            liberarArbol(n->der);
            nodo *nuevoNodo = n->izq;
            if (n->valor->op) free(n->valor->op);
            free(n->valor);
            free(n);
            return nuevoNodo;
        }
    }

    // OR con false
    if (n->valor->tipo_token == T_OP_OR) {
        if (n->izq && n->izq->valor && n->izq->valor->tipo_token == T_VFALSE) {
            liberarArbol(n->izq);
            nodo *nuevoNodo = n->der;
            if (n->valor->op) free(n->valor->op);
            free(n->valor);
            free(n);
            return nuevoNodo;
        } else if (n->der && n->der->valor && n->der->valor->tipo_token == T_VFALSE) {
            liberarArbol(n->der);
            nodo *nuevoNodo = n->izq;
            if (n->valor->op) free(n->valor->op);
            free(n->valor);
            free(n);
            return nuevoNodo;
        }
    }
    
    return n;
}


nodo* reducciones_simples(nodo *n) {
    if (n == NULL || n->valor == NULL) return NULL;

    if (n->izq) n->izq = reducciones_simples(n->izq);
    if (n->med) n->med = reducciones_simples(n->med);
    if (n->der) n->der = reducciones_simples(n->der);

    // resta x - x = 0
    if (n->valor->tipo_token == T_OP_MENOS) {
        if (n->izq && n->der && n->izq->valor && n->der->valor && n->izq->valor->tipo_token == T_ID && n->der->valor->tipo_token == T_ID && strcmp(n->izq->valor->name, n->der->valor->name) == 0) {
            liberarArbol(n);
            info *cero = (info*)malloc(sizeof(info));
            cero->name = strdup("digito");
            cero->tipo_token = T_DIGIT;
            cero->tipo_info = TIPO_INTEGER;
            cero->nro = 0;
            nodo *nuevoNodo = crearNodo(cero);
            return nuevoNodo;
        }
    }

    // multiplicacion 0 * x = 0 o x * 0 = 0
    if (n->valor->tipo_token == T_OP_MULT) {
        if ((n->izq && n->izq->valor && n->izq->valor->tipo_token == T_DIGIT && n->izq->valor->nro == 0) ||
            (n->der && n->der->valor && n->der->valor->tipo_token == T_DIGIT && n->der->valor->nro == 0)) {
            liberarArbol(n);
            info *cero = (info*)malloc(sizeof(info));
            cero->name = strdup("digito");
            cero->tipo_token = T_DIGIT;
            cero->tipo_info = TIPO_INTEGER;
            cero->nro = 0;
            nodo *nuevoNodo = crearNodo(cero);
            return nuevoNodo;
        }
    }

    // division x / x = 1
    if (n->valor->tipo_token == T_OP_DIV) {
        if (n->izq && n->der && n->izq->valor && n->der->valor && n->izq->valor->tipo_token == T_ID && n->der->valor->tipo_token == T_ID &&
            strcmp(n->izq->valor->name, n->der->valor->name) == 0) {
            liberarArbol(n);
            info *uno = (info*)malloc(sizeof(info));
            uno->name = strdup("digito");
            uno->tipo_token = T_DIGIT;
            uno->tipo_info = TIPO_INTEGER;
            uno->nro = 1;
            nodo *nuevoNodo = crearNodo(uno);
            return nuevoNodo;
        }
    }

    // division 0 / x = 0
    if (n->valor->tipo_token == T_OP_DIV) {
        if (n->izq && n->izq->valor && n->izq->valor->tipo_token == T_DIGIT && n->izq->valor->nro == 0) {
            liberarArbol(n);
            info *cero = (info*)malloc(sizeof(info));
            cero->name = strdup("digito");
            cero->tipo_token = T_DIGIT;
            cero->tipo_info = TIPO_INTEGER;
            cero->nro = 0;
            nodo *nuevoNodo = crearNodo(cero);
            return nuevoNodo;
        }
    }

    // x % 1 = 0
    if (n->valor->tipo_token == T_OP_RESTO) {
        if (n->der && n->der->valor && n->der->valor->tipo_token == T_DIGIT && n->der->valor->nro == 1) {   
            liberarArbol(n);
            info *cero = (info*)malloc(sizeof(info));
            cero->name = strdup("digito");
            cero->tipo_token = T_DIGIT;
            cero->tipo_info = TIPO_INTEGER;
            cero->nro = 0;
            nodo *nuevoNodo = crearNodo(cero);
            return nuevoNodo;
        } else if (n->izq && n->izq->valor && n->izq->valor->tipo_token == T_DIGIT && n->izq->valor->nro == 0) { // 0 % x = 0
            liberarArbol(n);
            info *cero = (info*)malloc(sizeof(info));
            cero->name = strdup("digito");
            cero->tipo_token = T_DIGIT;
            cero->tipo_info = TIPO_INTEGER;
            cero->nro = 0;
            nodo *nuevoNodo = crearNodo(cero);
            return nuevoNodo;
        } else {
            // x % x = 0
            if (n->izq && n->der && n->izq->valor && n->der->valor && n->izq->valor->tipo_token == T_ID && n->der->valor->tipo_token == T_ID &&
                strcmp(n->izq->valor->name, n->der->valor->name) == 0) {
                liberarArbol(n);
                info *cero = (info*)malloc(sizeof(info));
                cero->name = strdup("digito");
                cero->tipo_token = T_DIGIT;
                cero->tipo_info = TIPO_INTEGER;
                cero->nro = 0;
                nodo *nuevoNodo = crearNodo(cero);
                return nuevoNodo;
            }
        }
    }

    return n;
}

nodo* reducciones_dominantes(nodo *n) {
    if (n == NULL || n->valor == NULL) return NULL;

    if (n->izq) n->izq = reducciones_dominantes(n->izq);
    if (n->med) n->med = reducciones_dominantes(n->med);
    if (n->der) n->der = reducciones_dominantes(n->der);

    // x && false = false o false && x = false
    if (n->valor->tipo_token == T_OP_AND) {
        if ((n->izq && n->izq->valor && n->izq->valor->tipo_token == T_VFALSE) ||
            (n->der && n->der->valor && n->der->valor->tipo_token == T_VFALSE)) {
            liberarArbol(n);
            info *falso = (info*)malloc(sizeof(info));
            falso->bool_string = strdup("false");
            falso->b = false;
            falso->tipo_token = T_VFALSE;
            falso->tipo_info = TIPO_BOOL;
            nodo *nuevoNodo = crearNodo(falso);
            return nuevoNodo;
        }
    }

    // x || true = true o true || x = true
    if (n->valor->tipo_token == T_OP_OR) {
        if ((n->izq && n->izq->valor && n->izq->valor->tipo_token == T_VTRUE) ||
            (n->der && n->der->valor && n->der->valor->tipo_token == T_VTRUE)) {
            liberarArbol(n);
            info *verdadero = (info*)malloc(sizeof(info));
            verdadero->bool_string = strdup("true");
            verdadero->b = true;
            verdadero->tipo_token = T_VTRUE;
            verdadero->tipo_info = TIPO_BOOL;
            nodo *nuevoNodo = crearNodo(verdadero);
            return nuevoNodo;
        }
    }

    return n;
}

nodo* comparaciones_redundantes(nodo *n) {
    if (n == NULL || n->valor == NULL) return NULL;

    if (n->izq) n->izq = comparaciones_redundantes(n->izq);
    if (n->med) n->med = comparaciones_redundantes(n->med);
    if (n->der) n->der = comparaciones_redundantes(n->der);

    // x < x = false
    if (n->valor->tipo_token == T_MENOR) {
        if (n->izq && n->der && n->izq->valor && n->der->valor && n->izq->valor->tipo_token == T_ID && n->der->valor->tipo_token == T_ID &&
            strcmp(n->izq->valor->name, n->der->valor->name) == 0) {
            liberarArbol(n);
            info *falso = (info*)malloc(sizeof(info));
            falso->bool_string = strdup("false");
            falso->b = false;
            falso->tipo_token = T_VFALSE;
            falso->tipo_info = TIPO_BOOL;
            nodo *nuevoNodo = crearNodo(falso);
            return nuevoNodo;
        }
    }

    // x > x = false
    if (n->valor->tipo_token == T_MAYOR) {
        if (n->izq && n->der && n->izq->valor && n->der->valor && n->izq->valor->tipo_token == T_ID && n->der->valor->tipo_token == T_ID &&
            strcmp(n->izq->valor->name, n->der->valor->name) == 0) {
            liberarArbol(n);
            info *falso = (info*)malloc(sizeof(info));
            falso->bool_string = strdup("false");
            falso->b = false;
            falso->tipo_token = T_VFALSE;
            falso->tipo_info = TIPO_BOOL;
            nodo *nuevoNodo = crearNodo(falso);
            return nuevoNodo;
        }
    }

    // x == x = true
    if (n->valor->tipo_token == T_IGUALDAD) {
        if (n->izq && n->der && n->izq->valor && n->der->valor && n->izq->valor->tipo_token == T_ID && n->der->valor->tipo_token == T_ID &&
            strcmp(n->izq->valor->name, n->der->valor->name) == 0) {
            liberarArbol(n);
            info *verdadero = (info*)malloc(sizeof(info));
            verdadero->bool_string = strdup("true");
            verdadero->b = true;
            verdadero->tipo_token = T_VTRUE;
            verdadero->tipo_info = TIPO_BOOL;
            nodo *nuevoNodo = crearNodo(verdadero);
            return nuevoNodo;
        }
    }

    return n;
}