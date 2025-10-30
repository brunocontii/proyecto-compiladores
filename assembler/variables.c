#include <string.h>
#include <stdlib.h>
#include "estructuras_metodos.h"

static var_global *variables_globales = NULL;

void agregar_variable_global(const char *nombre, const char *valor) {
    var_global *var_g = (var_global *)malloc(sizeof(var_global));
    var_g->nombre = strdup(nombre);
    var_g->valor_inicial = strdup(valor);
    var_g->siguiente = NULL;
    
    if (variables_globales == NULL) {
        variables_globales = var_g;
    } else {
        var_global *actual = variables_globales;
        while (actual->siguiente != NULL) {
            actual = actual->siguiente;
        }
        actual->siguiente = var_g;
    }
}

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
                    sprintf(valor_str, "%d", inst->arg1->nro);
                }
                else if (inst->arg1->tipo_token == T_VTRUE || inst->arg1->tipo_token == T_VFALSE) {
                    strcpy(valor_str, inst->arg1->bool_string);
                }
                else if (inst->arg1->tipo_token == T_ID) {
                    strcpy(valor_str, inst->arg1->name);
                }
                else if (inst->arg1->esTemporal == 1) {
                    strcpy(valor_str, "0");
                }
                else {
                    strcpy(valor_str, "0");
                }
                
                agregar_variable_global(inst->resultado->name, valor_str);
            }
        }

        inst = inst->siguiente;
    }
}

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

var_global* get_variables_globales(void) {
    return variables_globales;
}