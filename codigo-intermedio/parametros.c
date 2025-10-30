#include <string.h>
#include "generador.h"
#include "auxiliares.h"

// contar parametros (usado en declaración y llamada)
int contar_parametros_metodo(nodo *params) {
    if (!params) return 0;
    
    int count = 0;
    nodo *actual = params;
    
    while (actual != NULL) {
        if (actual->valor->tipo_token == T_PARAMETROS || 
            actual->valor->tipo_token == T_EXPRS) {
            count++;
            actual = actual->izq;
        } else if (actual->valor->tipo_token) {
            count++;
            break;
        } else {
            break;
        }
    }
    return count;
}

void procesar_parametros_declaracion(nodo *params, int cant_params) {
    if (!params) return;
    
    // contamos cant de params
    if (cant_params == 0) return;
    
    // asignamos memoria para arrays temporales
    const char **param_names = (const char **)malloc(cant_params * sizeof(const char *));
    tipo_info *param_types = (tipo_info *)malloc(cant_params * sizeof(tipo_info));
    int param_count = 0;
    
    // primera pasada: recolectar nombres en orden inverso (porque la lista está al reves)
    nodo *actual = params;
    while (actual != NULL && param_count < cant_params) {
        if (actual->valor->tipo_token == T_PARAMETROS) {
            if (actual->der && actual->der->valor) {
                param_names[param_count] = actual->der->valor->name;
                param_types[param_count] = actual->der->valor->tipo_info;
                param_count++;
            }
            actual = actual->izq;
        } else {
            if (actual) {
                param_names[param_count] = actual->valor->name;
                param_types[param_count] = actual->valor->tipo_info;
                param_count++;
            }
            break;
        }
    }
    
    // segunda pasada: generar LOAD_PARAM en orden correcto
    for (int i = param_count - 1; i >= 0; i--) {
        int param_idx = param_count - 1 - i;  // Índice real del parametro
        
        // crear info para el parametro
        info *param_var = (info*)malloc(sizeof(info));
        param_var->name = strdup(param_names[i]);
        param_var->tipo_info = param_types[i];
        param_var->tipo_token = T_ID;
        param_var->esTemporal = 0;

        // crear info para el índice del parametro
        info *param_idx_info = crear_constante(param_idx);

        // generar: LOAD_PARAM <variable_parametro> <indice> -
        agregar_instruccion("LOAD_PARAM", param_var, param_idx_info, NULL);
    }
    
    // liberar memoria
    free(param_names);
    free(param_types);
}