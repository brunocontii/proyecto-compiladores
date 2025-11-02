#include <stdio.h>
#include <string.h>
#include "./semantico.h"
#include "./manejo_errores.h"


int contar_parametros(nodo *params, int tipo_lista) {
    if (!params) return 0;
    if ((int)params->valor->tipo_token == (int)tipo_lista)
        return contar_parametros(params->izq, tipo_lista) + 1;
    return 1;
}

void verificar_parametros(nodo *method_call, nodo* raiz, tabla_simbolos *ts) {
    if (!method_call || method_call->valor->tipo_token != T_METHOD_CALL) return;

    char *nombre_metodo_call = method_call->izq->valor->name;
    nodo *parametros_actuales = method_call->der;

    if (!buscar(ts, nombre_metodo_call)) {
        reportar_error(method_call->linea, "Error semantico: Metodo '%s' no declarado\n", nombre_metodo_call);
        return;
    }

    nodo *method_decl = buscarNodo(raiz, nombre_metodo_call);
    if (!method_decl) {
        reportar_error(method_call->linea, "Error interno: No se encontró el método en el AST\n");
        return;
    }

    nodo *parametros_formales = method_decl->izq;

    int cant_par_formales = contar_parametros(parametros_formales, T_PARAMETROS);
    int cant_par_actuales = contar_parametros(parametros_actuales, T_EXPRS);
    
    if (cant_par_formales != cant_par_actuales) {
        reportar_error(method_call->linea, "Error semantico: En la llamada al metodo '%s', "
                                            "la cantidad de parametros no coincide con la declaracion. Se esperaban %d pero se recibieron %d\n",
                                            nombre_metodo_call, cant_par_formales, cant_par_actuales);
        return;
    }

    while (parametros_actuales && parametros_formales) {
        bool formal_es_lista = (parametros_formales->valor->tipo_token == T_PARAMETROS);
        bool actual_es_lista = (parametros_actuales->valor->tipo_token == T_EXPRS);
        
        if (formal_es_lista && actual_es_lista) {
            // Comparar los parámetros actuales (der)
            if (parametros_actuales->der && parametros_formales->der) {
                tipo_info tipo_par_actual = calcular_tipo_expresion(parametros_actuales->der, ts);
                tipo_info tipo_par_formal = parametros_formales->der->valor->tipo_info;

                if (tipo_par_actual != tipo_par_formal) {
                    reportar_error(method_call->linea, "Error semantico: En la llamada al metodo '%s', "
                                    "el tipo del parametro no coincide con la declaracion. Se esperaba '%s' pero se recibio '%s'\n",
                                    nombre_metodo_call, tipo_info_to_string(tipo_par_formal), tipo_info_to_string(tipo_par_actual));
                }
            }
            
            // Avanzar en ambas listas
            parametros_actuales = parametros_actuales->izq;
            parametros_formales = parametros_formales->izq;
        } else {
            // Llegamos a las hojas o a un caso no manejado por el while
            break;
        }
    }

    // Verificar el último parámetro (las hojas)
    if (parametros_actuales && parametros_formales) {
        
        tipo_info tipo_actual_hoja = calcular_tipo_expresion(parametros_actuales, ts);

        if (parametros_formales->valor) {
            tipo_info tipo_formal_hoja = parametros_formales->valor->tipo_info;

            if (tipo_actual_hoja != tipo_formal_hoja) {
                reportar_error(method_call->linea, "Error semantico: En la llamada al metodo '%s', "
                                "el tipo del parametro no coincide con la declaracion. Se esperaba '%s' pero se recibio '%s'\n",
                                nombre_metodo_call, tipo_info_to_string(tipo_formal_hoja), tipo_info_to_string(tipo_actual_hoja));
            }
        }
    }
}