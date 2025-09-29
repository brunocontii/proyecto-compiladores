#include <stdio.h>
#include <string.h>
#include "../analisis-semantico/semantico.h"
#include "../utils/manejo_errores.h"

void verificar_asignacion_metodo(nodo *method_call, tabla_simbolos *ts) {
    if (!method_call) return;

    info *metodo = buscar(ts, method_call->izq->valor->name);
    if (!metodo) {
        reportar_error(method_call->linea, "Error semantico: Metodo '%s' no declarado\n", method_call->izq->valor->name);
        return;
    }

    switch (metodo->tipo_info) {
        case TIPO_INTEGER:
            reportar_error(method_call->linea, "Error semantico: Metodo '%s' es de tipo integer y tiene que ser usado en una asignacion\n", method_call->izq->valor->name);
            break;
        case TIPO_BOOL:
            reportar_error(method_call->linea, "Error semantico: Metodo '%s' es de tipo bool y tiene que ser usado en una asignacion\n", method_call->izq->valor->name);
            break;
        default:
            break;
    }

}