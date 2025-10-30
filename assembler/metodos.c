#include <string.h>
#include <stdlib.h>
#include "estructuras_metodos.h"

static metodo_info *metodo_stack = NULL; // stack de metodos (para anidamiento)

// nodo temporal para recoleccion de variables
typedef struct var_temp_node {
    const char *nombre;
    struct var_temp_node *siguiente;
} var_temp_node;

// estructura para almacenar variables locales, temporales y parametros temporalmente
typedef struct {
    var_temp_node *primero;
    int count;
} var_temp_list;

// verifica si la variable ya esta en la lista
static bool variable_en_lista(var_temp_node *lista, const char *nombre) {
    while (lista != NULL) {
        if (strcmp(lista->nombre, nombre) == 0) {
            return true;
        }
        lista = lista->siguiente;
    }
    return false;
}

// agrega la variable a la lista si no existe
static void agregar_variable_unica(var_temp_list *lista, const char *nombre) {
    if (!variable_en_lista(lista->primero, nombre)) {
        var_temp_node *nuevo = malloc(sizeof(var_temp_node));
        nuevo->nombre = nombre;
        nuevo->siguiente = lista->primero;
        lista->primero = nuevo;
        lista->count++;
    }
}

// libera la lista temporal
static void liberar_lista_temporal(var_temp_list *lista) {
    while (lista->primero != NULL) {
        var_temp_node *temp = lista->primero;
        lista->primero = lista->primero->siguiente;
        free(temp);
    }
    lista->count = 0;
}

// push de un nuevo metodo al stack
void push_metodo(const char *nombre, tipo_info tipo_ret) {
    metodo_info *nuevo = malloc(sizeof(metodo_info));
    nuevo->nombre = strdup(nombre);
    nuevo->num_vars_locales = 0;
    nuevo->mapeo_vars = NULL;
    nuevo->siguiente = metodo_stack;
    nuevo->tipo_retorno = tipo_ret;
    metodo_stack = nuevo;
}

// pop del metodo actual del stack
void pop_metodo(void) {
    if (metodo_stack == NULL) return;
    
    metodo_info *temp = metodo_stack;
    metodo_stack = metodo_stack->siguiente;
    
    while (temp->mapeo_vars != NULL) {
        var_offset *v = temp->mapeo_vars;
        temp->mapeo_vars = temp->mapeo_vars->siguiente;
        free(v->nombre);
        free(v);
    }
    
    free(temp->nombre);
    free(temp);
}

// obtener el metodo actual (tope del stack)
metodo_info* get_metodo_actual(void) {
    return metodo_stack;
}

// crear mapeo de variables locales, temporales y parametros a offsets en el stack
void crear_mapeo_variables_locales(codigo3dir *label) {
    metodo_info *metodo = get_metodo_actual();
    if (!metodo) return;
    
    // limpiar mapeo anterior del metodo actual
    while (metodo->mapeo_vars != NULL) {
        var_offset *temp = metodo->mapeo_vars;
        metodo->mapeo_vars = metodo->mapeo_vars->siguiente;
        free(temp->nombre);
        free(temp);
    }
    
    // inicializar lista temporal
    var_temp_list variables = {.primero = NULL, .count = 0};
    codigo3dir *p = label->siguiente;
    
    // recolectar variables locales, temporales y parametros
    while (p != NULL && strcmp(p->instruccion, "END") != 0) {
        // ASSIGN y LOAD_PARAM crean variables locales
        if (strcmp(p->instruccion, "ASSIGN") == 0 || 
            strcmp(p->instruccion, "LOAD_PARAM") == 0) {
            if (p->resultado && p->resultado->name) {
                agregar_variable_unica(&variables, p->resultado->name);
            }
        }

        // buscar temporales en resultados de operaciones
        if (p->resultado && p->resultado->esTemporal == 1 && p->resultado->name) {
            agregar_variable_unica(&variables, p->resultado->name);
        }

        p = p->siguiente;
    }
    
    // crear mapeo con offsets negativos desde %rbp
    var_temp_node *actual = variables.primero;
    int offset_idx = 0;
    
    while (actual != NULL) {
        var_offset *nuevo = malloc(sizeof(var_offset));
        nuevo->nombre = strdup(actual->nombre);
        nuevo->offset = -8 * (offset_idx + 1);
        nuevo->siguiente = metodo->mapeo_vars;
        metodo->mapeo_vars = nuevo;
        
        offset_idx++;
        actual = actual->siguiente;
    }
    
    metodo->num_vars_locales = variables.count;
    
    liberar_lista_temporal(&variables);
}