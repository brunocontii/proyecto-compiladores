#include <stdlib.h>
#include "estructuras_metodos.h"

// nodo de la lista enlazada para parametros
typedef struct param_node {
    info *param;
    struct param_node *siguiente;
} param_node;

// estructura para acumular parametros de un CALL
typedef struct param_call {
    param_node *primero;     // primer parametro
    param_node *ultimo;      // ultimo parametro (para insercion O(1))
    int count;               // cantidad de parametros
} param_call;

static param_call call_actual = {.primero = NULL, .ultimo = NULL, .count = 0};  // CALL actual

// agregar parametro al final de la lista
void agregar_param(info *param) {
    param_node *nuevo = malloc(sizeof(param_node));
    nuevo->param = param;
    nuevo->siguiente = NULL;
    
    if (call_actual.primero == NULL) {
        call_actual.primero = nuevo;
        call_actual.ultimo = nuevo;
    } else {
        call_actual.ultimo->siguiente = nuevo;
        call_actual.ultimo = nuevo;
    }
    
    call_actual.count++;
}

// limpiar lista de parámetros
void limpiar_params(void) {
    param_node *actual = call_actual.primero;
    while (actual != NULL) {
        param_node *temp = actual;
        actual = actual->siguiente;
        free(temp);
    }
    call_actual.primero = NULL;
    call_actual.ultimo = NULL;
    call_actual.count = 0;
}

// obtener cantidad de parametros actuales
int get_param_count() {
    return call_actual.count;
}

// obtener array de punteros a parametros
info** get_params_array(void) {
    if (call_actual.count == 0) return NULL;
    
    info **array = malloc(call_actual.count * sizeof(info*));
    if (!array) {
        fprintf(stderr, "Error: No se pudo asignar memoria para array de parámetros\n");
        exit(1);
    }
    
    param_node *actual = call_actual.primero;
    int idx = 0;
    
    while (actual != NULL) {
        array[idx++] = actual->param;
        actual = actual->siguiente;
    }
    
    return array;
}