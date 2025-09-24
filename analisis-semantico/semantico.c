#include <stdio.h>
#include <string.h>
#include "semantico.h"
#include "../utils/manejo_errores.h"

int dentroMetodo = 0; // variable global

void recorridoSemantico(nodo *raiz, tabla_simbolos *ts){

    if (!raiz) return;
    int linea = raiz->linea;

    switch (raiz->valor->tipo_token){
        case T_PROGRAM:
            if (raiz->izq) recorridoSemantico(raiz->izq, ts); // var_decls
            if (raiz->der) recorridoSemantico(raiz->der, ts); // method_decls
            break;

        case T_VAR_DECLS:
            recorridoSemantico(raiz->izq, ts);
            recorridoSemantico(raiz->der, ts);
            break;

        case T_METHOD_DECLS:
            recorridoSemantico(raiz->izq, ts);
            recorridoSemantico(raiz->der, ts);
            break;

        case T_PARAMETROS:
            recorridoSemantico(raiz->izq, ts);
            recorridoSemantico(raiz->der, ts);
            break;
        
        case T_STATEMENTS:
            recorridoSemantico(raiz->izq, ts);
            recorridoSemantico(raiz->der, ts);
            break;

        case T_EXPRS:
            recorridoSemantico(raiz->izq, ts);
            recorridoSemantico(raiz->der, ts);
            break;

        case T_PARAMETRO:
            // Insertar parametro en tabla de símbolos
            if (!insertar(ts, raiz->izq->valor)) {
                reportar_error(linea, "Parametro '%s' ya declarado\n", raiz->izq->valor->name);
            }

            break;
        case T_VAR_DECL: {

            // Insertar variable en tabla de símbolos
            if (!insertar(ts, raiz->izq->valor)) {
                reportar_error(linea, "Variable '%s' ya declarada\n", raiz->izq->valor->name);
            }

            tipo_info tipo_expr = calcular_tipo_expresion(raiz->der, ts);
            if (raiz->izq->valor->tipo_info != tipo_expr) {
                reportar_error(linea, "Error semántico: Incompatibilidad de tipos en la declaracion."
                                "Se declaro a '%s' de tipo '%d', pero se le quiere asignar el tipo '%d'\n",
                                raiz->izq->valor->name, raiz->izq->valor->tipo_info, tipo_expr);
            }
            break;
        }
        case T_METHOD_DECL: {
            if (!insertar(ts, raiz->valor)) {
                reportar_error(linea, "Método '%s' ya declarado\n", raiz->valor->name);
            }

            dentroMetodo = 1;   // estamos dentro de un método
            abrir_scope(ts);


            if (strcmp(raiz->valor->name, "main") == 0) {
                if (raiz->valor->tipo_info != TIPO_VOID) {
                    reportar_error(linea, "Error semantico: Main debe ser de tipo void\n");
                }
                if (raiz->izq != NULL) {
                    reportar_error(linea, "Error semantico: Main no debe tener parametros\n");
                }
            }
            
            // si es extern no hacer todo esto xq ya lo hizo donde fue declarado
            if (raiz->der->valor->tipo_token != T_EXTERN) {
                tipo_info retorno = retorno_bloque(raiz->der, ts);
                if (raiz->valor->tipo_info != retorno) {
                    reportar_error(linea, "Error semantico: Tipo de retorno no coincide con la declaracion\n");
                }
                if (raiz->valor->tipo_info == TIPO_VOID && retorno != TIPO_VOID) {
                    reportar_error(linea, "Error semántico: Metodo void no puede retornar valor\n");
                }
            }

            recorridoSemantico(raiz->izq, ts);
            recorridoSemantico(raiz->der, ts);
            printf("Nombre del metodo actual: %s\n",raiz->valor->name);
            imprimir_scope_actual(ts);
            // Cerrar scope del método
            cerrar_scope(ts);
            dentroMetodo = 0;   // salimos del método
            break;
        }

        case T_BLOQUE: {
            int abrir = 1;

            // No abrir un nuevo scope si es el bloque principal del metodo
            if (dentroMetodo && raiz->valor->tipo_token == T_METHOD_DECL) {
                abrir = 0;
            }

            if (abrir) abrir_scope(ts);

            if (raiz->izq) recorridoSemantico(raiz->izq, ts); // var_decls
            if (raiz->der) recorridoSemantico(raiz->der, ts); // statements

            if (abrir) cerrar_scope(ts);
            break;
        }


        case T_ASIGNACION: {
            info *busqueda = buscar(ts, raiz->izq->valor->name);
            tipo_info tipo_expr = calcular_tipo_expresion(raiz->der, ts);

            if (!busqueda) {
                reportar_error(linea, "Error semántico: Variable '%s' no declarada\n", raiz->izq->valor->name);
            } else if (busqueda->tipo_info != tipo_expr) {
                reportar_error(linea, "Error semántico: Incompatibilidad de tipos en la asignacion."
                                "La variable '%s' es de tipo '%d', pero la expresion es de tipo '%d'\n",
                                raiz->izq->valor->name, busqueda->tipo_info, tipo_expr);
            }
            recorridoSemantico(raiz->der, ts);
            break;
        }
        case T_OP_MAS:
        case T_OP_MULT:
        case T_OP_DIV:
        case T_OP_RESTO: {
            tipo_info tipo_expr1 = calcular_tipo_expresion(raiz->izq, ts);
            tipo_info tipo_expr2 = calcular_tipo_expresion(raiz->der, ts);

            if (tipo_expr1 != TIPO_INTEGER || tipo_expr2 != TIPO_INTEGER) {
                reportar_error(linea, "Error semantico: Operadores aritmeticos requieren operandos INTEGER"
                                " pero se recibieron tipos '%d' y '%d'\n", tipo_expr1, tipo_expr2);
            }
            recorridoSemantico(raiz->izq, ts);
            recorridoSemantico(raiz->der, ts);
            break;
        }
        case T_OP_MENOS: {
            if (raiz->izq == NULL) {
                // Menos unario
                tipo_info tipo_expr = calcular_tipo_expresion(raiz->der, ts);
                if (tipo_expr != TIPO_INTEGER) {
                    reportar_error(linea, "Error semántico: Operador unario '-' requiere INTEGER"
                                    " pero se recibio tipo '%d'\n", tipo_expr);
                }
                recorridoSemantico(raiz->der, ts);
            } else {
                // Menos binario
                tipo_info tipo_expr1 = calcular_tipo_expresion(raiz->izq, ts);
                tipo_info tipo_expr2 = calcular_tipo_expresion(raiz->der, ts);
                
                if (tipo_expr1 != TIPO_INTEGER || tipo_expr2 != TIPO_INTEGER) {
                    reportar_error(linea, "Error semántico: Resta requiere operandos INTEGER"
                                    " pero se recibieron tipos '%d' y '%d'\n", tipo_expr1, tipo_expr2);
                }
                recorridoSemantico(raiz->izq, ts);
                recorridoSemantico(raiz->der, ts);
            }
            break;
        }
        case T_OP_AND:
        case T_OP_OR: {
            tipo_info tipo_expr1 = calcular_tipo_expresion(raiz->izq, ts);
            tipo_info tipo_expr2 = calcular_tipo_expresion(raiz->der, ts);

            if (tipo_expr1 != TIPO_BOOL || tipo_expr2 != TIPO_BOOL) {
                reportar_error(linea, "Error semantico: Operacion booleana requiere operandos BOOL"
                                " pero se recibieron tipos '%d' y '%d'\n", tipo_expr1, tipo_expr2);
            }
            recorridoSemantico(raiz->izq, ts);
            recorridoSemantico(raiz->der, ts);
            break;
        }
        case T_OP_NOT: {
            tipo_info tipo_expr = calcular_tipo_expresion(raiz->der, ts);
            if (tipo_expr != TIPO_BOOL) {
                reportar_error(linea, "Error semantico: Operador '!' requiere operando BOOL"
                                " pero se recibio tipo '%d'\n", tipo_expr);
            }
            recorridoSemantico(raiz->der, ts);
            break;
        }
        case T_MENOR:
        case T_MAYOR: {
            tipo_info tipo_izq = calcular_tipo_expresion(raiz->izq, ts);
            tipo_info tipo_der = calcular_tipo_expresion(raiz->der, ts);
            
            if (tipo_izq != TIPO_INTEGER || tipo_der != TIPO_INTEGER) {
                reportar_error(linea, "Error semántico: Operadores < y > requieren operandos INTEGER"
                        " pero se recibieron tipos '%d' y '%d'\n", tipo_izq, tipo_der);
            }
            recorridoSemantico(raiz->izq, ts);
            recorridoSemantico(raiz->der, ts);
            break;
        }
        case T_IGUALDAD: {
            tipo_info tipo_izq = calcular_tipo_expresion(raiz->izq, ts);
            tipo_info tipo_der = calcular_tipo_expresion(raiz->der, ts);
            
            if (tipo_izq != tipo_der) {
                reportar_error(linea, "Error semántico: Operador == requiere operandos del mismo tipo"
                        " pero se recibieron tipos '%d' y '%d'\n", tipo_izq, tipo_der);
            }
            
            if (tipo_izq == TIPO_VOID || tipo_der == TIPO_VOID) {
                reportar_error(linea, "Error semántico: No se puede comparar con tipo VOID\n");
            }
            recorridoSemantico(raiz->izq, ts);
            recorridoSemantico(raiz->der, ts);
            break;
        }
        case T_ID: {
            info *busqueda = buscar(ts, raiz->valor->name);

            if (!busqueda) {
                reportar_error(linea, "Error semantico: Variable '%s' no declarada\n", raiz->valor->name);
            }
            break;
        }
        case T_IF:
        case T_WHILE: {
            tipo_info tipo_condicion = calcular_tipo_expresion(raiz->izq, ts);

            if (tipo_condicion != TIPO_BOOL) {
                reportar_error(linea, "Error semántico: La condicion debe ser de tipo BOOL"
                                " pero se recibio tipo '%d'\n", tipo_condicion);
            }
            recorridoSemantico(raiz->izq, ts);
            recorridoSemantico(raiz->der, ts);
            break;
        }
        case T_IF_ELSE: {
            tipo_info tipo_condicion = calcular_tipo_expresion(raiz->izq, ts);

            if (tipo_condicion != TIPO_BOOL) {
                reportar_error(linea, "Error semántico: La condicion debe ser de tipo BOOL"
                                " pero se recibio tipo '%d'\n", tipo_condicion);
            }
            recorridoSemantico(raiz->izq, ts);
            recorridoSemantico(raiz->med, ts);
            recorridoSemantico(raiz->der, ts);
            break;
        }
        case T_METHOD_CALL: {
            info *metodo = buscar(ts, raiz->izq->valor->name);
            if (!metodo) {
                reportar_error(linea, "Error: semantico: Método '%s' no declarado", raiz->izq->valor->name);
            }
            break;
        }
        default:
            if (raiz->izq) recorridoSemantico(raiz->izq, ts);
            if (raiz->der) recorridoSemantico(raiz->der, ts);
            break;
    }

}   

// retorna el tipo de la expresion o TIPO_VOID si hay un error
tipo_info calcular_tipo_expresion(nodo *expr, tabla_simbolos *ts) {
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
            tipo_info tipo_izq = calcular_tipo_expresion(expr->izq, ts);
            tipo_info tipo_der = calcular_tipo_expresion(expr->der, ts);

            if (tipo_izq == TIPO_INTEGER && tipo_der == TIPO_INTEGER) {
                return TIPO_INTEGER;
            }
            return TIPO_VOID;
        }
        case T_OP_MENOS: {
            if (expr->izq == NULL) {
                // caso de menos unario
                tipo_info tipo_der = calcular_tipo_expresion(expr->der, ts);
                return (tipo_der == TIPO_INTEGER) ? TIPO_INTEGER : TIPO_VOID;
            } else {
                // caso de menos binario
                tipo_info tipo_izq = calcular_tipo_expresion(expr->izq, ts);
                tipo_info tipo_der = calcular_tipo_expresion(expr->der, ts);

                if (tipo_izq == TIPO_INTEGER && tipo_der == TIPO_INTEGER) {
                    return TIPO_INTEGER;
                }
                return TIPO_VOID;
            }
        }
        case T_OP_AND:
        case T_OP_OR: {
            tipo_info tipo_izq = calcular_tipo_expresion(expr->izq, ts);
            tipo_info tipo_der = calcular_tipo_expresion(expr->der, ts);

            if (tipo_izq == TIPO_BOOL && tipo_der == TIPO_BOOL) {
                return TIPO_BOOL;
            }
            return TIPO_VOID;
        }
        case T_OP_NOT: {
            // caso de operador not (siempre es unario)
            tipo_info tipo_der = calcular_tipo_expresion(expr->der, ts);
            return (tipo_der == TIPO_BOOL) ? TIPO_BOOL : TIPO_VOID;
        }
        case T_MENOR:
        case T_MAYOR: {
            tipo_info tipo_izq = calcular_tipo_expresion(expr->izq, ts);
            tipo_info tipo_der = calcular_tipo_expresion(expr->der, ts);
            
            if (tipo_izq == TIPO_INTEGER && tipo_der == TIPO_INTEGER) {
                return TIPO_BOOL;
            }
            return TIPO_VOID;
        }
        case T_IGUALDAD: {
            tipo_info tipo_izq = calcular_tipo_expresion(expr->izq, ts);
            tipo_info tipo_der = calcular_tipo_expresion(expr->der, ts);

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
tipo_info retorno_bloque(nodo *bloque, tabla_simbolos *ts){
    if (!bloque) return TIPO_VOID;

    switch(bloque->valor->tipo_token) {
        case T_RETURN:
            if (bloque->izq != NULL) {
                return calcular_tipo_expresion(bloque->izq, ts);
            }
            return TIPO_VOID;
        case T_VAR_DECLS:
            // saltear las declaraciones de variables
            return TIPO_VOID;
        default: {
            tipo_info tipo_izq = retorno_bloque(bloque->izq, ts);
            tipo_info tipo_der = retorno_bloque(bloque->der, ts);
            
            if (tipo_izq != TIPO_VOID) return tipo_izq;
            if (tipo_der != TIPO_VOID) return tipo_der;
            
            return TIPO_VOID;
        }
    }
}