#include <stdio.h>
#include <string.h>
#include "codigo_muerto_var.h"

variables *var_list = NULL;

void agregar_variable(variables **lista, char *nombre, bool se_usa) {
    variables *nuevo = (variables*)malloc(sizeof(variables));
    nuevo->nombre = strdup(nombre);
    nuevo->se_usa = se_usa;
    nuevo->next = *lista;
    *lista = nuevo;
}

static void caso_condicional(nodo *hijo) {
    if (hijo == NULL) return;

    caso_condicional(hijo->der);
    caso_condicional(hijo->izq);

    if (hijo->valor->tipo_token == T_ID) {
        agregar_variable(&var_list, hijo->valor->name, true);
        hijo->valor->se_usa = true; // se usa en condición
    }
}

static int contar_nodos(nodo *raiz) {
    if (raiz == NULL) return 0;

    return 1 + contar_nodos(raiz->izq) + contar_nodos(raiz->med) + contar_nodos(raiz->der);
}

bool buscar_variable(variables *lista, char *nombre) {
    variables *actual = lista;
    while (actual != NULL) {
        if (strcmp(actual->nombre, nombre) == 0) {
            return actual-> se_usa;
        }
        actual = actual->next;
    }
    return false;
}

void liberar_lista_variables(variables **lista) {
    variables *actual = *lista;
    variables *siguiente;
    while (actual != NULL) {
        siguiente = actual->next;
        free(actual->nombre);
        free(actual);
        actual = siguiente;
    }
    *lista = NULL;
}

void recorrer_y_marcar(nodo *raiz) {
    if (raiz == NULL) return;

    recorrer_y_marcar(raiz->der);
    recorrer_y_marcar(raiz->med);
    recorrer_y_marcar(raiz->izq);

    switch (raiz->valor->tipo_token) {
        case T_VAR_DECL:
            if (raiz->izq && raiz->izq->valor->tipo_token == T_ID) {
                if (buscar_variable(var_list,raiz->izq->valor->name) == true) {
                    raiz->izq->valor->se_usa = true; // inicialmente no se usa
                } else {
                    raiz->izq->valor->se_usa = false; // inicialmente no se usa
                }             
            }
            // recorrer_y_marcar(raiz->der);
            // recorrer_y_marcar(raiz->med);
            // recorrer_y_marcar(raiz->izq);
            break;
        case T_STATEMENTS:{
            nodo* hijo = raiz->der;
            if (hijo == NULL) break;
            if (hijo->valor->tipo_token == T_ASIGNACION) {
                nodo* aux = hijo->der;
                nodo* auxIzq = hijo->izq;
                if (aux->valor->tipo_token == T_ID) {
                    aux->valor->se_usa = true; // se usa en asignación
                    agregar_variable(&var_list, aux->valor->name, true);
                }
                while (aux->izq != NULL) {
                    if (aux->der->valor->tipo_token == T_ID) {
                        aux->der->valor->se_usa = true; // se usa en asignación
                        agregar_variable(&var_list, aux->der->valor->name, true);
                    }
                    aux = aux->izq;
                }
                if (aux->valor->tipo_token == T_ID) {
                    aux->valor->se_usa = true; // se usa en asignación
                    agregar_variable(&var_list, aux->valor->name, true);
                }
                if (auxIzq->valor->tipo_token == T_ID) {
                    if (buscar_variable(var_list, auxIzq->valor->name) == true) {
                        auxIzq->valor->se_usa = true; // se usa en asignación
                    } else {
                        auxIzq->valor->se_usa = false;
                    }
                }
                
            }
            if (hijo->valor->tipo_token == T_RETURN) {
                if (hijo->izq && hijo->izq->valor->tipo_token == T_ID) {
                    hijo->izq->valor->se_usa = true; // se usa en return
                    agregar_variable(&var_list, hijo->izq->valor->name, true);
                }
            }
            if (hijo->valor->tipo_token == T_IF || hijo->valor->tipo_token == T_WHILE || hijo->valor->tipo_token == T_IF_ELSE) {
                nodo* aux = hijo->izq;
                caso_condicional(aux);
            }
            // recorrer_y_marcar(raiz->der);
            // recorrer_y_marcar(raiz->med);
            // recorrer_y_marcar(raiz->izq);
            break;
        }
        case T_METHOD_CALL:{
            nodo* hijo = raiz->der;
            if (hijo == NULL) break;
            if (hijo->valor->tipo_token == T_ID) {
                hijo->valor->se_usa = true; // se usa en la llamada al método
                agregar_variable(&var_list, hijo->valor->name, true);
            }
            while (hijo && hijo->izq != NULL) {
                if (hijo->der->valor->tipo_token == T_ID) {
                    hijo->der->valor->se_usa = true; // se usa en la llamada al método
                    agregar_variable(&var_list, hijo->der->valor->name, true);
                }
                hijo = hijo->izq;
            }
            if (hijo->valor->tipo_token == T_ID) {
                hijo->valor->se_usa = true; // se usa en la llamada al método
                agregar_variable(&var_list, hijo->valor->name, true);
            }
            // recorrer_y_marcar(raiz->der);
            // recorrer_y_marcar(raiz->med);
            // recorrer_y_marcar(raiz->izq);
            break;
        }
        default:
            // recorrer_y_marcar(raiz->der);
            // recorrer_y_marcar(raiz->med);
            // recorrer_y_marcar(raiz->izq);
            break;
    }
}

void recorrer_y_podar(nodo *raiz) {
    if (raiz == NULL) return;

    recorrer_y_podar(raiz->izq);
    recorrer_y_podar(raiz->med);
    recorrer_y_podar(raiz->der);

    switch (raiz->valor->tipo_token) {
        case T_VAR_DECLS:
            if (raiz->izq && raiz->izq->valor->tipo_token == T_VAR_DECL) {
                if (raiz->izq->izq && raiz->izq->izq->valor->tipo_token == T_ID) {
                    if (raiz->izq->izq->valor->se_usa == false) {
                        // Variable no usada → podar
                        nodo* temp = raiz->izq;
                        raiz->izq = NULL;
                        liberarArbol(temp);
                    }
                }
            }
            
            if (raiz->der && raiz->der->valor->tipo_token == T_VAR_DECL) {
                if (raiz->der->izq && raiz->der->izq->valor->tipo_token == T_ID) {
                    if (raiz->der->izq->valor->se_usa == false) {
                        // Variable no usada → podar
                        nodo* temp = raiz->der;
                        raiz->der = NULL;
                        liberarArbol(temp);
                    }
                }
            }
            break;
        case T_STATEMENTS:
            if (raiz->der && raiz->der->valor->tipo_token == T_ASIGNACION) {
                nodo* asignacion = raiz->der;
                if (asignacion->izq && asignacion->izq->valor->tipo_token == T_ID) {
                    if (asignacion->izq->valor->se_usa == false) {
                        // Asignación a variable no usada → podar
                        nodo* temp = raiz->der;
                        raiz->der = NULL;
                        liberarArbol(temp);
                    }
                }
            }
            break;
        default:
            break;
    }
    // recorrer_y_podar(raiz->izq);
    // recorrer_y_podar(raiz->med);
    // recorrer_y_podar(raiz->der);
}

void codigo_muerto_var(nodo *raiz) {
    if (raiz == NULL) return;

    int iteracion = 0;
    int pre_poda = contar_nodos(raiz);
    int post_poda = 0;
    
    // Crear imagen del árbol original
    generateASTDotFileWithLiveness(raiz, "ast_original", false);
    
    // Iterar mientras se puedan eliminar nodos
    while (post_poda < pre_poda) {
        iteracion++;
        
        // Marcar variables usadas
        recorrer_y_marcar(raiz);
        recorrer_y_marcar(raiz);

        // Crear imagen del árbol marcado
        char nombre_marcado[256];
        snprintf(nombre_marcado, sizeof(nombre_marcado), "ast_marcado_%d", iteracion);
        generateASTDotFileWithLiveness(raiz, nombre_marcado, true);
        
        // Podar variables no usadas
        recorrer_y_podar(raiz);
        
        // Crear imagen del árbol después de podar
        char nombre_podado[256];
        snprintf(nombre_podado, sizeof(nombre_podado), "ast_podado_%d", iteracion);
        generateASTDotFileWithLiveness(raiz, nombre_podado, false);

        // Liberar lista de variables y reiniciar
        liberar_lista_variables(&var_list);
        
        // Actualizar contadores
        post_poda = contar_nodos(raiz);
        if (post_poda < pre_poda) {
            pre_poda = post_poda;
            post_poda = 0;
        }
    }
}