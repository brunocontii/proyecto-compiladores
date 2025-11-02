#include <string.h>
#include <stdlib.h>
#include "estructuras_metodos.h"

static var_global *variables_globales = NULL;
static var_global *ultima_variable = NULL; // puntero al final de la lista

// agrega variable global a la lista
void agregar_variable_global(const char *nombre, const char *valor) {
    var_global *var_g = (var_global *)malloc(sizeof(var_global));
    var_g->nombre = strdup(nombre);
    var_g->valor_inicial = strdup(valor);
    var_g->siguiente = NULL;
    
    if (variables_globales == NULL) {
        variables_globales = var_g;
        ultima_variable = var_g;
    } else {
        ultima_variable->siguiente = var_g;  // O(1) en vez de O(n)
        ultima_variable = var_g;
    }
}

// recolecta variables globales del codigo intermedio, desde el inicio del codigo intermedio hasta la primera LABEL
void recolectar_variables_globales(codigo3dir *programa) {
    codigo3dir *inst = programa;

    while (inst != NULL) {
        if (strcmp(inst->instruccion, "LABEL") == 0) {
            break;
        }

        if (strcmp(inst->instruccion, "ASSIGN") == 0) {
            if (inst->resultado && inst->resultado->name && inst->arg1) {
                char valor_str[32];
                
                if (inst->arg1->tipo_token == T_DIGIT) {
                    snprintf(valor_str, sizeof(valor_str), "%d", inst->arg1->nro);
                }
                else if (inst->arg1->tipo_token == T_VTRUE || inst->arg1->tipo_token == T_VFALSE) {
                    snprintf(valor_str, sizeof(valor_str), "%s", inst->arg1->bool_string);
                }
                else if (inst->arg1->tipo_token == T_ID) {
                    snprintf(valor_str, sizeof(valor_str), "%s", inst->arg1->name);  // trunca si es muy largo
                }
                else {
                    snprintf(valor_str, sizeof(valor_str), "0");
                }
                
                agregar_variable_global(inst->resultado->name, valor_str);
            }
        }

        inst = inst->siguiente;
    }
}

// verifica si una variable esta en la lista de variables globales
bool es_variable_global(const char *nombre) {
    var_global *actual = variables_globales;
    while (actual != NULL) {
        if (strcmp(actual->nombre, nombre) == 0) {
            return true;
        }
        actual = actual->siguiente;
    }
    return false;
}

// obtiene el offset de una variable
int obtener_offset_variable(const char *nombre) {
    metodo_info *metodo = get_metodo_actual();
    if (!metodo) return -8;
    
    var_offset *actual = metodo->mapeo_vars;
    while (actual != NULL) {
        if (strcmp(actual->nombre, nombre) == 0) {
            return actual->offset;
        }
        actual = actual->siguiente;
    }
    return -8;
}

// obtiene la lista de variables globales
var_global* get_variables_globales(void) {
    return variables_globales;
}