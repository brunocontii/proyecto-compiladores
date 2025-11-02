#include <stdio.h>
#include <string.h>
#include "./semantico.h"
#include "./manejo_errores.h"


tipo_info calcular_tipo_expresion(nodo *expr, tabla_simbolos *ts) {
    if (!expr) {
        return TIPO_VOID;
    }
    
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
            tipo_info tipo_izq = calcular_tipo_expresion(expr->izq, ts);
            tipo_info tipo_der = calcular_tipo_expresion(expr->der, ts);

            if (tipo_izq == TIPO_INTEGER && tipo_der == TIPO_INTEGER) {
                return TIPO_INTEGER;
            } else {
                reportar_error(expr->linea, "Error semantico: Operador '%s' requiere operandos INTEGER"
                                            " pero se recibieron tipos '%s' y '%s'\n", expr->valor->op,tipo_info_to_string(tipo_izq), tipo_info_to_string(tipo_der));
                return TIPO_VOID;
            }
        }

        case T_OP_MENOS: {
            if (expr->izq == NULL) {
                // menos unario
                tipo_info tipo_der = calcular_tipo_expresion(expr->der, ts);

                if (tipo_der == TIPO_INTEGER) {
                    return TIPO_INTEGER;
                } else {
                    reportar_error(expr->linea, "Error semántico: Operador unario '-' requiere INTEGER"
                                                " pero se recibio tipo '%s'\n", tipo_info_to_string(tipo_der));
                    return TIPO_VOID;
                }
            } else {
                // menos binario
                tipo_info tipo_izq = calcular_tipo_expresion(expr->izq, ts);
                tipo_info tipo_der = calcular_tipo_expresion(expr->der, ts);

                if (tipo_izq == TIPO_INTEGER && tipo_der == TIPO_INTEGER) {
                    return TIPO_INTEGER;
                } else {
                    reportar_error(expr->linea, "Error semántico: Operador binario '-' requiere operandos INTEGER"
                                                " pero se recibieron tipos '%s' y '%s'\n", tipo_info_to_string(tipo_izq), tipo_info_to_string(tipo_der));
                    return TIPO_VOID;
                }
            }
        }

        case T_OP_AND:
        case T_OP_OR: {
            tipo_info tipo_izq = calcular_tipo_expresion(expr->izq, ts);
            tipo_info tipo_der = calcular_tipo_expresion(expr->der, ts);

            if (tipo_izq == TIPO_BOOL && tipo_der == TIPO_BOOL) {
                return TIPO_BOOL;
            } else {
                reportar_error(expr->linea, "Error semantico: Operador booleano '%s' requiere operandos BOOL"
                                            " pero se recibieron tipos '%s' y '%s'\n", expr->valor->op, tipo_info_to_string(tipo_izq), tipo_info_to_string(tipo_der));
                return TIPO_VOID;
            }
        }

        case T_OP_NOT: {
            tipo_info tipo_der = calcular_tipo_expresion(expr->der, ts);

            if (tipo_der == TIPO_BOOL) {
                return TIPO_BOOL;
            } else {
                reportar_error(expr->linea, "Error semántico: Operador unario '!' requiere BOOL"
                                            " pero se recibio tipo '%s'\n", tipo_info_to_string(tipo_der));
                return TIPO_VOID;
            }
        }

        case T_MENOR:
        case T_MAYOR: {
            tipo_info tipo_izq = calcular_tipo_expresion(expr->izq, ts);
            tipo_info tipo_der = calcular_tipo_expresion(expr->der, ts);

            if (tipo_izq == TIPO_INTEGER && tipo_der == TIPO_INTEGER) {
                return TIPO_BOOL;
            } else {
                reportar_error(expr->linea, "Error semantico: Operador de comparacion '%s' requiere operandos INTEGER"
                                            " pero se recibieron tipos '%s' y '%s'\n", expr->valor->op, tipo_info_to_string(tipo_izq), tipo_info_to_string(tipo_der));
                return TIPO_VOID;
            }
        }

        case T_IGUALDAD: {
            tipo_info tipo_izq = calcular_tipo_expresion(expr->izq, ts);
            tipo_info tipo_der = calcular_tipo_expresion(expr->der, ts);

            if (tipo_izq == tipo_der && tipo_izq != TIPO_VOID) {
                return TIPO_BOOL;
            } else {
                reportar_error(expr->linea, "Error semantico: Operador '==' requiere operandos del mismo tipo"
                                            " pero se recibieron tipos '%s' y '%s'\n", tipo_info_to_string(tipo_izq), tipo_info_to_string(tipo_der));
                return TIPO_VOID;
            }
        }

        case T_ID: {
            info *var_info = buscar(ts, expr->valor->name);
            
            if (var_info) {
                return var_info->tipo_info;
            } else {
                reportar_error(expr->linea, "Error semántico: Variable '%s' no declarada\n", expr->valor->name);
                return TIPO_VOID;
            }
        }

        case T_METHOD_CALL: {
            info *metodo_info = buscar(ts, expr->izq->valor->name);
            
            if (metodo_info) {
                return metodo_info->tipo_info;
            } else {
                reportar_error(expr->linea, "Error semántico: Método '%s' no declarado\n", expr->izq->valor->name);
                return TIPO_VOID;
            }
        }

        default:
            return TIPO_VOID;
    }
}


// devuelve TIPO_VOID en caso de que:
//  - el bloque sea NULL
//  - no haya return en el bloque
//  - existe un "return;" (sin expresion)
// devuelve TIPO_INTEGER o TIPO_BOOL si encuentra un return con expresion
tipo_info retorno_bloque(nodo *bloque, tabla_simbolos *ts){
    if (!bloque || !bloque->valor) return TIPO_VOID;

    switch(bloque->valor->tipo_token) {
        case T_RETURN:
            if (bloque->izq != NULL) {
                return calcular_tipo_expresion(bloque->izq, ts);
            }
            return TIPO_VOID;
        case T_BLOQUE:
            return retorno_bloque(bloque->der, ts); // statements
        case T_STATEMENTS: {
            // Recorrer la lista de statements (estructura recursiva izq->der)
            nodo *curr = bloque;
            while (curr && curr->valor && curr->valor->tipo_token == T_STATEMENTS) {
                // Verificar el statement actual (izq)
                if (curr->izq) {
                    tipo_info t = retorno_bloque(curr->izq, ts);
                    if (t != TIPO_VOID) return t;
                }
                // Continuar con el siguiente statement (der)
                curr = curr->der;
            }
            // Si curr no es NULL, verificar el último nodo
            if (curr) {
                tipo_info t = retorno_bloque(curr, ts);
                if (t != TIPO_VOID) return t;
            }
            return TIPO_VOID;
        }
        default:
            if (bloque->izq) {
                tipo_info izq = retorno_bloque(bloque->izq, ts);
                if (izq != TIPO_VOID) return izq;
            }
            if (bloque->der) {
                tipo_info der = retorno_bloque(bloque->der, ts);
                if (der != TIPO_VOID) return der;
            }
            if (bloque->med) {
                tipo_info med = retorno_bloque(bloque->med, ts);
                if (med != TIPO_VOID) return med;
            }
            return TIPO_VOID;
    }
}