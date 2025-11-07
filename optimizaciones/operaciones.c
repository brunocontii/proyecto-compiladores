#include "operaciones.h"
#include <string.h>
#include <stdlib.h>
#include <stdio.h>
#include "../arbol-sintactico/arbol.h"

// funciones auxiliares

// reemplazar nodo actual por su hijo
static void reemplazar_por_hijo(nodo *padre, nodo *hijo_a_mantener, nodo *hijo_a_eliminar) {
    if (!padre || !hijo_a_mantener) return;
    
    // guardar temporalmente los datos del padre
    info *temp_valor = padre->valor;
    
    // copiar el contenido del hijo al padre
    padre->valor = hijo_a_mantener->valor;
    padre->izq = hijo_a_mantener->izq;
    padre->med = hijo_a_mantener->med;
    padre->der = hijo_a_mantener->der;
    
    // liberar el valor anterior del padre (el operador)
    if (temp_valor) {
        if (temp_valor->name) free(temp_valor->name);
        if (temp_valor->bool_string) free(temp_valor->bool_string);
        if (temp_valor->op) free(temp_valor->op);
        free(temp_valor);
    }

    // liberar el nodo hijo (pero no su contenido, porque ahora es del padre)
    free(hijo_a_mantener);

    // liberar el hijo a eliminar completamente
    if (hijo_a_eliminar) {
        liberarArbol(hijo_a_eliminar);
    }
}

// reemplazar nodo por una constante
static void reemplazar_por_constante(nodo *n, int valor, tipo_info tipo) {
    // liberar hijos
    if (n->izq) liberarArbol(n->izq);
    if (n->der) liberarArbol(n->der);
    if (n->med) liberarArbol(n->med);

    // liberar valor anterior
    if (n->valor) {
        if (n->valor->name) free(n->valor->name);
        if (n->valor->bool_string) free(n->valor->bool_string);
        if (n->valor->op) free(n->valor->op);
        free(n->valor);
    }

    // crear nueva constante
    n->valor = (info*)malloc(sizeof(info));
    
    if (tipo == TIPO_INTEGER) {
        char buf[32];
        snprintf(buf, sizeof(buf), "%d", valor);
        n->valor->name = strdup(buf);
        n->valor->tipo_token = T_DIGIT;
        n->valor->tipo_info = TIPO_INTEGER;
        n->valor->nro = valor;
        n->valor->bool_string = NULL;
    } else if (tipo == TIPO_BOOL) {
        n->valor->name = strdup(valor ? "true" : "false");
        n->valor->bool_string = strdup(valor ? "true" : "false");
        n->valor->tipo_token = valor ? T_VTRUE : T_VFALSE;
        n->valor->tipo_info = TIPO_BOOL;
        n->valor->b = valor;
        n->valor->nro = valor;
    }
    
    n->valor->esTemporal = 0;
    n->valor->op = NULL;
    n->izq = NULL;
    n->med = NULL;
    n->der = NULL;
}

// optimizaciones individuales

// optimizacion de valores neutros
static void valores_neutros_recursivo(nodo *n) {
    if (!n || !n->valor) return;

    // recorrer hijos primero (bottom-up)
    if (n->izq) valores_neutros_recursivo(n->izq);
    if (n->med) valores_neutros_recursivo(n->med);
    if (n->der) valores_neutros_recursivo(n->der);

    // x + 0 = x  o  0 + x = x
    if (n->valor->tipo_token == T_OP_MAS) {
        if (n->izq && n->izq->valor && n->izq->valor->tipo_token == T_DIGIT && n->izq->valor->nro == 0) {
            reemplazar_por_hijo(n, n->der, n->izq);
        } else if (n->der && n->der->valor && n->der->valor->tipo_token == T_DIGIT && n->der->valor->nro == 0) {
            reemplazar_por_hijo(n, n->izq, n->der);
        }
    }

    // x - 0 = x
    if (n->valor->tipo_token == T_OP_MENOS && n->izq && n->der) {
        if (n->der && n->der->valor && n->der->valor->tipo_token == T_DIGIT && n->der->valor->nro == 0) {
            reemplazar_por_hijo(n, n->izq, n->der);
        }
    }

    // x * 1 = x  o  1 * x = x
    if (n->valor->tipo_token == T_OP_MULT) {
        if (n->izq && n->izq->valor && n->izq->valor->tipo_token == T_DIGIT && n->izq->valor->nro == 1) {
            reemplazar_por_hijo(n, n->der, n->izq);
        } else if (n->der && n->der->valor && n->der->valor->tipo_token == T_DIGIT && n->der->valor->nro == 1) {
            reemplazar_por_hijo(n, n->izq, n->der);
        }
    }

    // x / 1 = x
    if (n->valor->tipo_token == T_OP_DIV) {
        if (n->der && n->der->valor && n->der->valor->tipo_token == T_DIGIT && n->der->valor->nro == 1) {
            reemplazar_por_hijo(n, n->izq, n->der);
        }
    }

    // x && true = x  o  true && x = x
    if (n->valor->tipo_token == T_OP_AND) {
        if (n->izq && n->izq->valor && n->izq->valor->tipo_token == T_VTRUE) {
            reemplazar_por_hijo(n, n->der, n->izq);
        } else if (n->der && n->der->valor && n->der->valor->tipo_token == T_VTRUE) {
            reemplazar_por_hijo(n, n->izq, n->der);
        }
    }

    // x || false = x  o  false || x = x
    if (n->valor->tipo_token == T_OP_OR) {
        if (n->izq && n->izq->valor && n->izq->valor->tipo_token == T_VFALSE) {
            reemplazar_por_hijo(n, n->der, n->izq);
        } else if (n->der && n->der->valor && n->der->valor->tipo_token == T_VFALSE) {
            reemplazar_por_hijo(n, n->izq, n->der);
        }
    }
}

// optimizacion de reducciones simples
static void reducciones_simples_recursivo(nodo *n) {
    if (!n || !n->valor) return;

    if (n->izq) reducciones_simples_recursivo(n->izq);
    if (n->med) reducciones_simples_recursivo(n->med);
    if (n->der) reducciones_simples_recursivo(n->der);

    // x - x = 0
    if (n->valor->tipo_token == T_OP_MENOS && n->izq && n->der) {
        if (n->izq->valor && n->der->valor && 
            n->izq->valor->tipo_token == T_ID && n->der->valor->tipo_token == T_ID &&
            strcmp(n->izq->valor->name, n->der->valor->name) == 0) {
            reemplazar_por_constante(n, 0, TIPO_INTEGER);
        }
    }

    // x * 0 = 0  o  0 * x = 0
    if (n->valor->tipo_token == T_OP_MULT) {
        if ((n->izq && n->izq->valor && n->izq->valor->tipo_token == T_DIGIT && n->izq->valor->nro == 0) ||
            (n->der && n->der->valor && n->der->valor->tipo_token == T_DIGIT && n->der->valor->nro == 0)) {
            reemplazar_por_constante(n, 0, TIPO_INTEGER);
        }
    }

    // x / x = 1
    if (n->valor->tipo_token == T_OP_DIV && n->izq && n->der) {
        if (n->izq->valor && n->der->valor &&
            n->izq->valor->tipo_token == T_ID && n->der->valor->tipo_token == T_ID &&
            strcmp(n->izq->valor->name, n->der->valor->name) == 0) {
            reemplazar_por_constante(n, 1, TIPO_INTEGER);
        }
    }

    // 0 / x = 0
    if (n->valor->tipo_token == T_OP_DIV) {
        if (n->izq && n->izq->valor && n->izq->valor->tipo_token == T_DIGIT && n->izq->valor->nro == 0) {
            reemplazar_por_constante(n, 0, TIPO_INTEGER);
        }
    }

    // x % 1 = 0, 0 % x = 0, x % x = 0
    if (n->valor->tipo_token == T_OP_RESTO) {
        if (n->der && n->der->valor && n->der->valor->tipo_token == T_DIGIT && n->der->valor->nro == 1) {
            reemplazar_por_constante(n, 0, TIPO_INTEGER);
        } else if (n->izq && n->izq->valor && n->izq->valor->tipo_token == T_DIGIT && n->izq->valor->nro == 0) {
            reemplazar_por_constante(n, 0, TIPO_INTEGER);
        } else if (n->izq && n->der && n->izq->valor && n->der->valor &&
                    n->izq->valor->tipo_token == T_ID && n->der->valor->tipo_token == T_ID &&
                    strcmp(n->izq->valor->name, n->der->valor->name) == 0) {
            reemplazar_por_constante(n, 0, TIPO_INTEGER);
        }
    }
}

// optimizacion de reducciones dominantes
static void reducciones_dominantes_recursivo(nodo *n) {
    if (!n || !n->valor) return;

    if (n->izq) reducciones_dominantes_recursivo(n->izq);
    if (n->med) reducciones_dominantes_recursivo(n->med);
    if (n->der) reducciones_dominantes_recursivo(n->der);

    // x && false = false  o  false && x = false
    if (n->valor->tipo_token == T_OP_AND) {
        if ((n->izq && n->izq->valor && n->izq->valor->tipo_token == T_VFALSE) ||
            (n->der && n->der->valor && n->der->valor->tipo_token == T_VFALSE)) {
            reemplazar_por_constante(n, 0, TIPO_BOOL);
        }
    }

    // x || true = true  o  true || x = true
    if (n->valor->tipo_token == T_OP_OR) {
        if ((n->izq && n->izq->valor && n->izq->valor->tipo_token == T_VTRUE) ||
            (n->der && n->der->valor && n->der->valor->tipo_token == T_VTRUE)) {
            reemplazar_por_constante(n, 1, TIPO_BOOL);
        }
    }
}

static void comparaciones_redundantes_recursivo(nodo *n) {
    if (!n || !n->valor) return;

    if (n->izq) comparaciones_redundantes_recursivo(n->izq);
    if (n->med) comparaciones_redundantes_recursivo(n->med);
    if (n->der) comparaciones_redundantes_recursivo(n->der);

    // x < x = false
    if (n->valor->tipo_token == T_MENOR && n->izq && n->der) {
        if (n->izq->valor && n->der->valor &&
            n->izq->valor->tipo_token == T_ID && n->der->valor->tipo_token == T_ID &&
            strcmp(n->izq->valor->name, n->der->valor->name) == 0) {
            reemplazar_por_constante(n, 0, TIPO_BOOL);
        }
    }

    // x > x = false
    if (n->valor->tipo_token == T_MAYOR && n->izq && n->der) {
        if (n->izq->valor && n->der->valor &&
            n->izq->valor->tipo_token == T_ID && n->der->valor->tipo_token == T_ID &&
            strcmp(n->izq->valor->name, n->der->valor->name) == 0) {
            reemplazar_por_constante(n, 0, TIPO_BOOL);
        }
    }

    // x == x = true
    if (n->valor->tipo_token == T_IGUALDAD && n->izq && n->der) {
        if (n->izq->valor && n->der->valor &&
            n->izq->valor->tipo_token == T_ID && n->der->valor->tipo_token == T_ID &&
            strcmp(n->izq->valor->name, n->der->valor->name) == 0) {
            reemplazar_por_constante(n, 1, TIPO_BOOL);
        }
    }
}

// funciones encargadas de aplicar su respectiva optimizacion
void valores_neutros(nodo *raiz) {
    if (!raiz) return;
    valores_neutros_recursivo(raiz);
}

void reducciones_simples(nodo *raiz) {
    if (!raiz) return;
    reducciones_simples_recursivo(raiz);
}

void reducciones_dominantes(nodo *raiz) {
    if (!raiz) return;
    reducciones_dominantes_recursivo(raiz);
}

void comparaciones_redundantes(nodo *raiz) {
    if (!raiz) return;
    comparaciones_redundantes_recursivo(raiz);
}

void optimizaciones_operaciones(nodo *raiz) {
    if (!raiz) return;
    
    valores_neutros(raiz);
    reducciones_simples(raiz);
    reducciones_dominantes(raiz);
    comparaciones_redundantes(raiz);
}