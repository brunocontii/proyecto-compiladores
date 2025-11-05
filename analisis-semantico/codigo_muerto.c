#include "codigo_muerto.h"
#include <string.h>
#include <stdlib.h>
#include "semantico.h"
#include "../arbol-sintactico/arbol.h"

static int terminaEnReturn(nodo *n);

void eliminarCodigoMuerto(nodo *n) {
    if (n == NULL || n->valor == NULL) return;

    if (n->valor->tipo_token == T_STATEMENTS) {
        nodo *izq = n->izq;

        if (terminaEnReturn(izq)) {
            liberarArbol(n->der);
            n->der = NULL;
        } else {
            eliminarCodigoMuerto(izq);
        }
        eliminarCodigoMuerto(n->der);
    } else { 
        eliminarCodigoMuerto(n->izq);
        eliminarCodigoMuerto(n->der);
    }
}

int terminaEnReturn(nodo *n) {
    if (n == NULL) return 0;
    
    if (n->valor && n->valor->tipo_token == T_RETURN)
        return 1;

    if (n->valor && n->valor->tipo_token == T_IF)
        return terminaEnReturn(n->der);

    if (n->valor && n->valor->tipo_token == T_IF_ELSE)
        return terminaEnReturn(n->der);

    if (n->valor && n->valor->tipo_token == T_WHILE)
        return terminaEnReturn(n->der);

    if (n->valor && n->valor->tipo_token == T_STATEMENTS)
        return terminaEnReturn(n->der);

    if (n->valor && n->valor->tipo_token == T_BLOQUE)
        return terminaEnReturn(n->der);

    return 0;
}

