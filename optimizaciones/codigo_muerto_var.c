#include <stdio.h>
#include <string.h>
#include "codigo_muerto_var.h"

void caso_condicional(nodo *hijo) {
    if (hijo == NULL) return;

    if (hijo->valor->tipo_token == T_ID) {
        hijo->valor->se_usa = true; // se usa en condición
    }

    caso_condicional(hijo->izq);
    caso_condicional(hijo->der);
}

int contar_nodos(nodo *raiz) {
    if (raiz == NULL) return 0;

    return 1 + contar_nodos(raiz->izq) + contar_nodos(raiz->med) + contar_nodos(raiz->der);
}

void recorrer_y_marcar(nodo *raiz) {
    if (raiz == NULL) return;

    switch (raiz->valor->tipo_token) {
        case T_VAR_DECL:
            if (raiz->izq && raiz->izq->valor->tipo_token == T_ID) {
                raiz->izq->valor->se_usa = false; // inicialmente no se usa
            }
            break;
        case T_STATEMENTS:{
            nodo* hijo = raiz->der;
            if (hijo->valor->tipo_token == T_ASIGNACION) {
                nodo* aux = hijo->der;
                if (aux->valor->tipo_token == T_ID) {
                    aux->valor->se_usa = true; // se usa en asignación
                }
                while (aux->izq != NULL) {
                    if (aux->der->valor->tipo_token == T_ID) {
                        aux->der->valor->se_usa = true; // se usa en asignación
                    }
                    aux = aux->izq;
                }
                if (aux->valor->tipo_token == T_ID) {
                    aux->valor->se_usa = true; // se usa en asignación
                } 
            }
            if (hijo->valor->tipo_token == T_RETURN) {
                if (hijo->izq && hijo->izq->valor->tipo_token == T_ID) {
                    hijo->izq->valor->se_usa = true; // se usa en return
                }
            }
            if (hijo->valor->tipo_token == T_IF || hijo->valor->tipo_token == T_WHILE || hijo->valor->tipo_token == T_IF_ELSE) {
                nodo* aux = hijo->der;
                caso_condicional(aux);
            }
            break;
        }
        case T_METHOD_CALL:{
            nodo* hijo = raiz->der;
            if (hijo->valor->tipo_token == T_ID) {
                hijo->valor->se_usa = true; // se usa en la llamada al método
            }
            while (hijo->izq != NULL) {
                if (hijo->der->valor->tipo_token == T_ID) {
                    hijo->der->valor->se_usa = true; // se usa en la llamada al método
                }
                hijo = hijo->izq;
            }
            if (hijo->valor->tipo_token == T_ID) {
                hijo->valor->se_usa = true; // se usa en la llamada al método
            }
            break;
        }
        default:
            recorrer_y_marcar(raiz->izq);
            recorrer_y_marcar(raiz->med);
            recorrer_y_marcar(raiz->der);
            break;
    }
}

void recorrer_y_podar(nodo *raiz) {
    if (raiz == NULL) return;

    switch (raiz->valor->tipo_token) {
        case T_VAR_DECLS:
            nodo* hijoIzq = raiz->izq;
            nodo* hijoDer = raiz->der;

            if (hijoIzq->izq && hijoIzq->izq->valor->tipo_token == T_ID) {
                if (hijoIzq->izq->valor->se_usa == false) {
                    // Podar el nodo de declaración de variable no usada
                    raiz->izq = NULL;
                    liberarArbol(hijoIzq);
                }
            }
            if (hijoDer->izq && hijoDer->izq->valor->tipo_token == T_ID) {
                if (hijoDer->izq->valor->se_usa == false) {
                    // Podar el nodo de declaración de variable no usada
                    raiz->der = NULL;
                    liberarArbol(hijoDer);
                }
            }
            break;
        case T_STATEMENTS:
            nodo* hijo = raiz->der;
            if (hijo->valor->tipo_token == T_ASIGNACION) {
                if (hijo->izq->valor->se_usa == false) {
                    // Podar el nodo de asignación a variable no usada
                    raiz->der = NULL;
                    liberarArbol(hijo);
                }
            }
            break;
        default:
            recorrer_y_marcar(raiz->izq);
            recorrer_y_marcar(raiz->der);
            break;
    }
}

void codigo_muerto_var(nodo *raiz) {
    if (raiz == NULL) return;

    int pre_poda = contar_nodos(raiz);
    int post_poda = pre_poda + 1;
    while (post_poda < pre_poda) {
        recorrer_y_marcar(raiz);
        recorrer_y_podar(raiz);
        pre_poda = post_poda;
        post_poda = contar_nodos(raiz);
    }

    return;
}