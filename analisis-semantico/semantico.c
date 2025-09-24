#include <stdio.h>
#include <string.h>
#include "semantico.h"
#include "../utils/manejo_errores.h"

extern tabla_simbolos *ts;

void recorridoSemantico(nodo *raiz, tabla_simbolos *ts){

    if (!raiz) return;

    switch (raiz->valor->tipo_token){
        case T_VAR_DECL: {
            tipo_info tipo_expr = calcular_tipo_expresion(raiz->der);
            if (raiz->izq->valor->tipo_info != tipo_expr) {
                reportar_error("Error semántico: Incompatibilidad de tipos en la declaracion."
                                "Se declaro a '%s' de tipo '%d', pero se le quiere asignar el tipo '%d'\n",
                                raiz->izq->valor->name, raiz->izq->valor->tipo_info, tipo_expr);
            }
            break;
        }
        case T_METHOD_DECL: {
            if (strcmp(raiz->valor->name, "main") == 0) {
                if (raiz->valor->tipo_info != TIPO_VOID) {
                    reportar_error("Error semantico: Main debe ser de tipo void\n");
                }
                if (raiz->izq != NULL) {
                    reportar_error("Error semantico: Main no debe tener parametros\n");
                }
            }

            tipo_info retorno = retorno_bloque(raiz->der);
            if (raiz->valor->tipo_info != retorno) {
                reportar_error("Error semantico: Tipo de retorno no coincide con la declaracion\n");
            }
            if (raiz->valor->tipo_info == TIPO_VOID && retorno != TIPO_VOID) {
                reportar_error("Error semántico: Metodo void no puede retornar valor\n");
            }
            break;
        }
        case T_ASIGNACION: {
            info *busqueda = buscar(ts, raiz->izq->valor->name);
            tipo_info tipo_expr = calcular_tipo_expresion(raiz->der);

            if (!busqueda) {
                reportar_error("Error semántico: Variable '%s' no declarada\n", raiz->izq->valor->name);
            } else if (busqueda->tipo_info != tipo_expr) {
                reportar_error("Error semántico: Incompatibilidad de tipos en la asignacion."
                                "La variable '%s' es de tipo '%d', pero la expresion es de tipo '%d'\n",
                                raiz->izq->valor->name, busqueda->tipo_info, tipo_expr);
            }
            break;
        }
        case T_OP_MAS:
        case T_OP_MULT:
        case T_OP_DIV:
        case T_OP_RESTO: {
            tipo_info tipo_expr1 = calcular_tipo_expresion(raiz->izq);
            tipo_info tipo_expr2 = calcular_tipo_expresion(raiz->der);

            if (tipo_expr1 != TIPO_INTEGER || tipo_expr2 != TIPO_INTEGER) {
                reportar_error("Error semantico: Operadores aritmeticos requieren operandos INTEGER"
                                " pero se recibieron tipos '%d' y '%d'\n", tipo_expr1, tipo_expr2);
            }
            break;
        }
        case T_OP_MENOS: {
            if (raiz->izq == NULL) {
                // Menos unario
                tipo_info tipo_expr = calcular_tipo_expresion(raiz->der);
                if (tipo_expr != TIPO_INTEGER) {
                    reportar_error("Error semántico: Operador unario '-' requiere INTEGER"
                                    " pero se recibio tipo '%d'\n", tipo_expr);
                }
            } else {
                // Menos binario
                tipo_info tipo_expr1 = calcular_tipo_expresion(raiz->izq);
                tipo_info tipo_expr2 = calcular_tipo_expresion(raiz->der);
                
                if (tipo_expr1 != TIPO_INTEGER || tipo_expr2 != TIPO_INTEGER) {
                    reportar_error("Error semántico: Resta requiere operandos INTEGER"
                                    " pero se recibieron tipos '%d' y '%d'\n", tipo_expr1, tipo_expr2);
                }
            }
            break;
        }
        case T_OP_AND:
        case T_OP_OR: {
            tipo_info tipo_expr1 = calcular_tipo_expresion(raiz->izq);
            tipo_info tipo_expr2 = calcular_tipo_expresion(raiz->der);

            if (tipo_expr1 != TIPO_BOOL || tipo_expr2 != TIPO_BOOL) {
                reportar_error("Error semantico: Operacion booleana requiere operandos BOOL"
                                " pero se recibieron tipos '%d' y '%d'\n", tipo_expr1, tipo_expr2);
            }
            break;
        }
        case T_OP_NOT: {
            tipo_info tipo_expr = calcular_tipo_expresion(raiz->der);
            if (tipo_expr != TIPO_BOOL) {
                reportar_error("Error semantico: Operador '!' requiere operando BOOL"
                                " pero se recibio tipo '%d'\n", tipo_expr);
            }
            break;
        }
        case T_MENOR:
        case T_MAYOR: {
            tipo_info tipo_izq = calcular_tipo_expresion(raiz->izq);
            tipo_info tipo_der = calcular_tipo_expresion(raiz->der);
            
            if (tipo_izq != TIPO_INTEGER || tipo_der != TIPO_INTEGER) {
                reportar_error("Error semántico: Operadores < y > requieren operandos INTEGER"
                        " pero se recibieron tipos '%d' y '%d'\n", tipo_izq, tipo_der);
            }
            break;
        }
        case T_IGUALDAD: {
            tipo_info tipo_izq = calcular_tipo_expresion(raiz->izq);
            tipo_info tipo_der = calcular_tipo_expresion(raiz->der);
            
            if (tipo_izq != tipo_der) {
                reportar_error("Error semántico: Operador == requiere operandos del mismo tipo"
                        " pero se recibieron tipos '%d' y '%d'\n", tipo_izq, tipo_der);
            }
            
            if (tipo_izq == TIPO_VOID || tipo_der == TIPO_VOID) {
                reportar_error("Error semántico: No se puede comparar con tipo VOID\n");
            }
            break;
        }
        case T_ID: {
            info *busqueda = buscar(ts, raiz->valor->name);

            if (!busqueda) {
                reportar_error("Error semantico: Variable '%s' no declarada\n", raiz->valor->name);
            }
            break;
        }
        case T_IF:
        case T_WHILE: {
            tipo_info tipo_condicion = calcular_tipo_expresion(raiz->izq);

            if (tipo_condicion != TIPO_BOOL) {
                reportar_error("Error semántico: La condicion debe ser de tipo BOOL"
                                " pero se recibio tipo '%d'\n", tipo_condicion);
            }
            break;
        }
        case T_IF_ELSE: {
            tipo_info tipo_condicion = calcular_tipo_expresion(raiz->izq);

            if (tipo_condicion != TIPO_BOOL) {
                reportar_error("Error semántico: La condicion debe ser de tipo BOOL"
                                " pero se recibio tipo '%d'\n", tipo_condicion);
            }
            break;
        }
        default:
            // ver que se hace aca
            break;
    }

    if (raiz->izq) recorridoSemantico(raiz->izq, ts);
    if (raiz->der) recorridoSemantico(raiz->der, ts);
    if (raiz->med) recorridoSemantico(raiz->med, ts);
}   

// retorna el tipo de la expresion o TIPO_VOID si hay un error
tipo_info calcular_tipo_expresion(nodo *expr) {
    if (!expr) return TIPO_VOID;
    
    switch(expr->valor->tipo_token) {
        case T_DIGIT:
            return TIPO_INTEGER;
        case T_VTRUE:
        case T_VFALSE:
            return TIPO_BOOL;
        case T_OP_MAS:
        case T_OP_MULT:
        case T_OP_DIV:
        case T_OP_RESTO: {
            tipo_info tipo_izq = calcular_tipo_expresion(expr->izq);
            tipo_info tipo_der = calcular_tipo_expresion(expr->der);

            if (tipo_izq == TIPO_INTEGER && tipo_der == TIPO_INTEGER) {
                return TIPO_INTEGER;
            }
            return TIPO_VOID;
        }
        case T_OP_MENOS: {
            if (expr->izq == NULL) {
                // caso de menos unario
                tipo_info tipo_der = calcular_tipo_expresion(expr->der);
                return (tipo_der == TIPO_INTEGER) ? TIPO_INTEGER : TIPO_VOID;
            } else {
                // caso de menos binario
                tipo_info tipo_izq = calcular_tipo_expresion(expr->izq);
                tipo_info tipo_der = calcular_tipo_expresion(expr->der);

                if (tipo_izq == TIPO_INTEGER && tipo_der == TIPO_INTEGER) {
                    return TIPO_INTEGER;
                }
                return TIPO_VOID;
            }
        }
        case T_OP_AND:
        case T_OP_OR: {
            tipo_info tipo_izq = calcular_tipo_expresion(expr->izq);
            tipo_info tipo_der = calcular_tipo_expresion(expr->der);

            if (tipo_izq == TIPO_BOOL && tipo_der == TIPO_BOOL) {
                return TIPO_BOOL;
            }
            return TIPO_VOID;
        }
        case T_OP_NOT: {
            // caso de operador not (siempre es unario)
            tipo_info tipo_der = calcular_tipo_expresion(expr->der);
            return (tipo_der == TIPO_BOOL) ? TIPO_BOOL : TIPO_VOID;
        }
        case T_MENOR:
        case T_MAYOR: {
            tipo_info tipo_izq = calcular_tipo_expresion(expr->izq);
            tipo_info tipo_der = calcular_tipo_expresion(expr->der);
            
            if (tipo_izq == TIPO_INTEGER && tipo_der == TIPO_INTEGER) {
                return TIPO_BOOL;
            }
            return TIPO_VOID;
        }
        case T_IGUALDAD: {
            tipo_info tipo_izq = calcular_tipo_expresion(expr->izq);
            tipo_info tipo_der = calcular_tipo_expresion(expr->der);

            if (tipo_izq == tipo_der && tipo_izq != TIPO_VOID) {
                return TIPO_BOOL;
            }
            return TIPO_VOID;
        }
        case T_ID: {
            info *var_info = buscar(ts, expr->valor->name);
            return var_info ? var_info->tipo_info : TIPO_VOID;
        }
        case T_METHOD_CALL: {
            info *metodo_info = buscar(ts, expr->valor->name);
            return metodo_info ? metodo_info->tipo_info : TIPO_VOID;
        }
        default:
            // si se llama con algo que no es una expresion
            return TIPO_VOID;
    }
}

// devuelve TIPO_VOID en caso de que:
//  - el bloque sea NULL
//  - no haya return en el bloque
//  - existe un "return;" (sin expresion)
// devuelve TIPO_INTEGER o TIPO_BOOL si encuentra un return con expresion
tipo_info retorno_bloque(nodo *bloque){
    if (!bloque) return TIPO_VOID;

    switch(bloque->valor->tipo_token) {
        case T_RETURN:
            if (bloque->izq != NULL) {
                return calcular_tipo_expresion(bloque->izq);
            }
            return TIPO_VOID;
        case T_VAR_DECLS:
            // saltear las declaraciones de variables
            return TIPO_VOID;
        default: {
            tipo_info tipo_izq = retorno_bloque(bloque->izq);
            tipo_info tipo_der = retorno_bloque(bloque->der);
            
            if (tipo_izq != TIPO_VOID) return tipo_izq;
            if (tipo_der != TIPO_VOID) return tipo_der;
            
            return TIPO_VOID;
        }
    }
}