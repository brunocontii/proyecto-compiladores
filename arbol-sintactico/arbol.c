#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "arbol.h"

nodo* crearNodo(info *valorNodo) {
    nodo* n = (nodo*)malloc(sizeof(nodo));

    n->valor = valorNodo;
    n->izq = NULL;
    n->der = NULL;

    return n;
}

nodo* crearArbol(info *valorNodo, nodo *hijoIzq, nodo *hijoDer) {
    nodo* n = (nodo*)malloc(sizeof(nodo));

    n->valor = valorNodo;
    n->izq = hijoIzq;
    n->der = hijoDer;

    return n;
}

void mostrarArbol(nodo *raiz, int nivel) {
    if (!raiz) return;

    for (int i = 0; i < nivel; i++) {
        printf("   ");
    }

    switch (raiz->valor->tipo_token) {
        case T_PROGRAM: printf("PROGRAM\n"); break;
        case T_EXTERN: printf("EXTERN\n"); break;
        case T_RETURN: printf("RETURN\n"); break;
        case T_IF: printf("IF\n"); break;
        case T_ELSE: printf("ELSE\n"); break;
        case T_THEN: printf("THEN\n"); break;
        case T_WHILE: printf("WHILE\n"); break;
        case T_VTRUE: printf("TRUE\n"); break;
        case T_VFALSE: printf("FALSE\n"); break;
        case T_OP_AND: printf("AND: %s\n", raiz->valor->op); break;
        case T_OP_OR: printf("OR: %s\n", raiz->valor->op); break;
        case T_OP_NOT: printf("NOT: %s\n", raiz->valor->op); break;
        case T_INTEGER: printf("INTEGER\n"); break;
        case T_BOOL: printf("BOOL\n"); break;
        case T_VOID: printf("VOID\n"); break;
        case T_ID: printf("ID: %s\n", raiz->valor->name); break;
        case T_DIGIT: printf("DIGIT: %d\n", raiz->valor->nro); break;
        case T_OP_MENOS: printf("MENOS: %s\n", raiz->valor->op); break;
        case T_OP_MAS: printf("MAS: %s\n", raiz->valor->op); break;
        case T_OP_MULT: printf("MULT: %s\n", raiz->valor->op); break;
        case T_OP_DIV: printf("DIV: %s\n", raiz->valor->op); break;
        case T_OP_RESTO: printf("RESTO: %s\n", raiz->valor->op); break;
        case T_ASIGNACION: printf("ASIG: %s\n", raiz->valor->op); break;
        case T_IGUALDAD: printf("IGUAL: %s\n", raiz->valor->op); break;
        case T_MENOR: printf("MENOR: %s\n", raiz->valor->op); break;
        case T_MAYOR: printf("MAYOR: %s\n", raiz->valor->op); break;
        case T_PYC: printf("PUNTO_Y_COMA\n"); break;
        case T_COMA: printf("COMA\n"); break;
        case T_PAR_A: printf("PAR_A\n"); break;
        case T_PAR_C: printf("PAR_C\n"); break;
        case T_LLA_A: printf("LLAVE_A\n"); break;
        case T_LLA_C: printf("LLAVE_C\n"); break;
        default: printf("Nodo desconocido\n"); break;
    }

    mostrarArbol(raiz->izq, nivel + 1);
    mostrarArbol(raiz->der, nivel + 1);
}

void liberarArbol(nodo *raiz) {
    if (!raiz) return;

    liberarArbol(raiz->izq);
    liberarArbol(raiz->der);

    switch (raiz->valor->tipo_token) {
        // Tokens que usan el campo name
        case T_ID:
            if (raiz->valor->name) 
                free(raiz->valor->name);
            break;

        // Tokens que usan el campo op
        case T_OP_AND:
        case T_OP_OR:
        case T_OP_NOT:
        case T_OP_MENOS:
        case T_OP_MAS:
        case T_OP_MULT:
        case T_OP_DIV:
        case T_OP_RESTO:
        case T_ASIGNACION:
        case T_IGUALDAD:
        case T_MENOR:
        case T_MAYOR:
            if (raiz->valor->op)
                free(raiz->valor->op);
            break;

        // Tokens que usan el campo op (palabras reservadas)
        case T_INTEGER:
        case T_BOOL:
        case T_VOID:
        case T_PROGRAM:
        case T_EXTERN:
        case T_RETURN:
        case T_IF:
        case T_ELSE:
        case T_THEN:
        case T_WHILE:
            if (raiz->valor->op)
                free(raiz->valor->op);
            break;

        // Tokens que NO requieren liberar memoria adicional
        case T_VTRUE:
        case T_VFALSE:
        case T_DIGIT:
        case T_PYC:
        case T_COMA:
        case T_PAR_A:
        case T_PAR_C:
        case T_LLA_A:
        case T_LLA_C:
            // No hacer nada - son valores primitivos
            break;
    }

    free(raiz->valor);
    free(raiz);
}
