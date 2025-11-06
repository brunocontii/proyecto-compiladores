#include "codigo_muerto.h"
#include <string.h>
#include <stdlib.h>
#include "../arbol-sintactico/arbol.h"

static int terminaEnReturn(nodo *n);

void eliminarCodigoMuerto(nodo *n) {
    if (n == NULL || n->valor == NULL) return;

    if (n->valor->tipo_token == T_STATEMENTS) {
        eliminarCodigoMuerto(n->izq);
        if (terminaEnReturn(n->izq)) {
            liberarArbol(n->der);
            n->der = NULL;
        } else {
            eliminarCodigoMuerto(n->der);
        }
    } else { 
        eliminarCodigoMuerto(n->izq);
        eliminarCodigoMuerto(n->med);
        eliminarCodigoMuerto(n->der);
    }
}

static int terminaEnReturn(nodo *n) {
    if (n == NULL || n->valor == NULL) return 0;
    
    if (n->valor->tipo_token == T_RETURN)
        return 1;

    switch (n->valor->tipo_token) {
        
        case T_STATEMENTS:
        case T_BLOQUE:
            return terminaEnReturn(n->der);
        
        // if else garantiza return si ambos terminan en return
        case T_IF_ELSE:
            return terminaEnReturn(n->der) && terminaEnReturn(n->med);
        
        // if sin else no garantiza return lo mismo que while
        case T_IF:
        case T_WHILE:
            return 0;

        default:
            return 0;
    }
}

