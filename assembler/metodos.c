#include <string.h>
#include <stdlib.h>
#include "estructuras_metodos.h"

static metodo_info *metodo_stack = NULL; // stack de metodos (para anidamiento)

void push_metodo(const char *nombre, tipo_info tipo_ret) {
    metodo_info *nuevo = malloc(sizeof(metodo_info));
    nuevo->nombre = strdup(nombre);
    nuevo->num_vars_locales = 0;
    nuevo->mapeo_vars = NULL;
    nuevo->siguiente = metodo_stack;
    nuevo->tipo_retorno = tipo_ret;
    metodo_stack = nuevo;
}

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

metodo_info* get_metodo_actual(void) {
    return metodo_stack;
}

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
    
    codigo3dir *p = label->siguiente;
    const char *variables[200];
    int var_count = 0;
    
    // recolectar variables locales, temporales y parametros
    while (p != NULL && strcmp(p->instruccion, "END") != 0) {
        // ASSIGN y LOAD_PARAM crean variables locales
        // osea que se trata de la misma manera a las variables que a los parametros
        if (strcmp(p->instruccion, "ASSIGN") == 0 || strcmp(p->instruccion, "LOAD_PARAM") == 0) {
            if (p->resultado && p->resultado->name) {
                bool found = false;
                for (int i = 0; i < var_count; i++) {
                    if (strcmp(variables[i], p->resultado->name) == 0) {
                        found = true;
                        break;
                    }
                }
                if (!found) {
                    variables[var_count++] = p->resultado->name;
                }
            }
        }

        // tambien buscar temporales en otras instrucciones
        // porque algunos temporales solo aparecen como resultado de operaciones
        if (p->resultado && p->resultado->esTemporal == 1 && p->resultado->name) {
            bool found = false;
            for (int i = 0; i < var_count; i++) {
                if (strcmp(variables[i], p->resultado->name) == 0) {
                    found = true;
                    break;
                }
            }
            if (!found) {
                variables[var_count++] = p->resultado->name;
            }
        }

        p = p->siguiente;
    }
    
    // crear mapeo con offsets
    for (int i = 0; i < var_count; i++) {
        var_offset *nuevo = malloc(sizeof(var_offset));
        nuevo->nombre = strdup(variables[i]);
        nuevo->offset = -8 * (i + 1);
        nuevo->siguiente = metodo->mapeo_vars;
        metodo->mapeo_vars = nuevo;
    }
    
    metodo->num_vars_locales = var_count;
}