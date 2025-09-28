#include <stdio.h>
#include <string.h>
#include "../analisis-semantico/semantico.h"
#include "../utils/manejo_errores.h"

void verificar_asignacion_metodo(nodo *method_call, nodo* raiz, tabla_simbolos *ts) {
    if (!method_call || method_call->valor->tipo_token != T_METHOD_CALL) return;

    if (!buscar(ts, method_call->izq->valor->name)) {
        reportar_error(method_call->linea, "Error semantico: Metodo '%s' no declarado\n", method_call->izq->valor->name);
        return;
    }

    switch (method_call->izq->valor->tipo_info)
    {
    case TIPO_INTEGER:
        reportar_error(method_call->linea, "Error semantico: Metodo '%s' es de tipo integer y tiene que ser usado en una asignacion\n", method_call->izq->valor->name);
        break;
    case TIPO_BOOL:
        break;
    case TIPO_VOID:
        break;
    default:
        break;
    }

}