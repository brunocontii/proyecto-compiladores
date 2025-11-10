#include "codigo_muerto_bloque.h"
#include <string.h>
#include <stdlib.h>
#include "../arbol-sintactico/arbol.h"

nodo* eliminarBloquesMuertosBloque(nodo *n) {
    if (n == NULL || n->valor == NULL) return n;

    // Procesar hijos primero (bottom-up)
    n->izq = eliminarBloquesMuertosBloque(n->izq);
    n->med = eliminarBloquesMuertosBloque(n->med);
    n->der = eliminarBloquesMuertosBloque(n->der);

    if (n->valor->tipo_token == T_IF && n->izq && n->izq->valor) {
        // caso if, si es false, eliminar todo
        if (n->izq->valor->tipo_token == T_VFALSE) {
            liberarArbol(n);
            return NULL;
        }
    } else if (n->valor->tipo_token == T_IF_ELSE && n->izq && n->izq->valor) {
        // caso if-else, si es false solo mantener else, si es true solo mantener then
        if (n->izq->valor->tipo_token == T_VFALSE) {
            nodo *resultado = n->der;
            n->der = NULL;
            liberarArbol(n);
            return resultado;
        } else if (n->izq->valor->tipo_token == T_VTRUE) {
            nodo *resultado = n->med;
            n->med = NULL;
            liberarArbol(n);
            return resultado;
        }
    } else if (n->valor->tipo_token == T_WHILE && n->izq && n->izq->valor) {
        // caso while, si es false, eliminar todo
        if (n->izq->valor->tipo_token == T_VFALSE) {
            liberarArbol(n);
            return NULL;
        }
    }

    return n;
}

// funcion principal
nodo* eliminarBloquesMuertos(nodo *n) {
    return eliminarBloquesMuertosBloque(n);
}
