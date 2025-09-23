#include <stdio.h>
#include "semantico.h"

extern tabla_simbolos *ts;

void recorridoSemantico(nodo *raiz, tabla_simbolos *ts){

    if (!raiz) return;

    switch (raiz->valor->tipo_token){

    case T_VAR_DECL:
        tipo_info tipo_expr = calcular_tipo_expresion(raiz->der);
        if (raiz->izq->valor->tipo_info != tipo_expr) {
            printf("Error semántico: Tipo de variable (%d) no coincide con expresión (%d)\n", raiz->izq->valor->tipo_info, tipo_expr);
        }
        break;

    case T_METHOD_DECL:
        if (strcmp(raiz->valor->name, "main") == 0) {      
            if (raiz->valor->tipo_info != TIPO_VOID) {
                printf("Error semantico: main tiene otro tipo que no es void\n");
            }
            if (raiz->izq != NULL) {
                printf("Error semantico: main tiene parametros no deberia mal ahi\n");
            }
        }

        // magico de saber que retorna
        tipo_info magia = retorno_bloque(raiz->der);
        if (raiz->valor->tipo_info != magia) {
            printf("Error semantico: el tipo de retorno no coincide con la declaracion\n");
        }
        if (raiz->valor->tipo_info == TIPO_VOID && magia != TIPO_VOID) {
            printf("Error semántico: Método void no puede retornar valor\n");
        }
        break;

    case T_ASIGNACION:

            if (!raiz->izq->valor->name) {
                printf("Error semántico: Asignación inválida\n");
                break;
            }

            tipo_info tipo_expr = calcular_tipo_expresion(raiz->der);
            info *busqueda = buscar(ts, raiz->izq->valor->name);
            
            if (!busqueda) {
                printf("Error semántico: Variable '%s' no declarada\n", 
                    raiz->izq->valor->name);
            } else if (busqueda->tipo_info != tipo_expr) {
                printf("Error semántico: Asignación (%d) no coincide con expresión (%d)\n", 
                    busqueda->tipo_info, tipo_expr);
            }
            break;

    //case T_METHOD_CALL: erich

    case T_OP_MAS:
         T_OP_MULT:
         T_OP_DIV:
         T_OP_RESTO:

            tipo_info tipo_expr1 = calcular_tipo_expresion(raiz->izq);
            tipo_info tipo_expr2 = calcular_tipo_expresion(raiz->der);
            if (tipo_expr1 != TIPO_INTEGER || tipo_expr2 != TIPO_INTEGER) {
                printf("Error semantico: el tipo de las expresiones no son ambas enteras\n");
            }
            /*if (raiz->valor->tipo_token == T_OP_DIV) { ver
                }
            */
         break;

    case T_OP_MENOS:
        {
            if (raiz->izq == NULL) {
                // Menos unario
                tipo_info tipo_expr = calcular_tipo_expresion(raiz->der);
                if (tipo_expr != TIPO_INTEGER) {
                    printf("Error semántico: Operador unario '-' requiere INTEGER\n");
                }
            } else {
                // Menos binario
                tipo_info tipo_expr1 = calcular_tipo_expresion(raiz->izq);
                tipo_info tipo_expr2 = calcular_tipo_expresion(raiz->der);
                
                if (tipo_expr1 != TIPO_INTEGER || tipo_expr2 != TIPO_INTEGER) {
                    printf("Error semántico: Resta requiere operandos INTEGER\n");
                }
            }
        }
        break;

    case T_OP_AND:
         T_OP_OR:
         T_OP_NOT:
            if (raiz->izq == NULL) {
                tipo_info tipo_operando = calcular_tipo_expresion(raiz->der);
                
                if (tipo_operando != TIPO_BOOL) {
                    printf("Error semántico: Operador '!' requiere operando BOOL\n");
                }
            }else {
                tipo_info tipo_expr1 = calcular_tipo_expresion(raiz->izq);
                tipo_info tipo_expr2 = calcular_tipo_expresion(raiz->der);
                if (tipo_expr1 != TIPO_BOOL || tipo_expr2 != TIPO_BOOL) {
                    printf("Error semantico: el tipo de las expresiones no son ambas enteras\n");
                }
            }
        break;

    case T_MENOR:
    case T_MAYOR:
            tipo_info tipo_izq = calcular_tipo_expresion(raiz->izq);
            tipo_info tipo_der = calcular_tipo_expresion(raiz->der);
            
            if (tipo_izq != TIPO_INTEGER || tipo_der != TIPO_INTEGER) {
                printf("Error semántico: Operadores < y > requieren operandos INTEGER\n");
            }
        break;
    case T_OP_NOT:
            tipo_info tipo_operando = calcular_tipo_expresion(raiz->der);
            if (tipo_operando != TIPO_BOOL) {
                printf("Error semántico: Operador '!' requiere operando BOOL\n");
            }
        break;
    case T_IGUALDAD:
        {
            tipo_info tipo_izq = calcular_tipo_expresion(raiz->izq);
            tipo_info tipo_der = calcular_tipo_expresion(raiz->der);
            
            // Igualdad requiere que ambos operandos sean del mismo tipo
            if (tipo_izq != tipo_der) {
                printf("Error semántico: Operador == requiere operandos del mismo tipo\n");
            }
            
            // No se puede comparar con VOID
            if (tipo_izq == TIPO_VOID || tipo_der == TIPO_VOID) {
                printf("Error semántico: No se puede comparar con tipo VOID\n");
            }
        }
        break;

    case T_ID:
        info *busqueda = buscar(ts, raiz->valor->name);
        if (busqueda == NULL) {
            printf("Error semantico: variable %s no declarada\n", raiz->valor->name);
        }

        break;
        
    default:
        break;
    }
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