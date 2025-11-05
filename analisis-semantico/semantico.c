#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include "semantico.h"
#include "./manejo_errores.h"
#include "./verificar_parametros.c"
#include "./calcular_tipo_expresion.c"
#include "./verificar_asignacion_metodo.c"

bool es_metodo = false;
bool es_extern = false;
bool hay_main = false;
nodo *raiz_arbol = NULL;

// verifica si una expresion es un literal
bool es_literal_nodo(nodo *expr) {
    if (!expr || !expr->valor) return false;
        
    // literales validos: numeros, true, false
    return (expr->valor->tipo_token == T_DIGIT ||
            expr->valor->tipo_token == T_VTRUE ||
            expr->valor->tipo_token == T_VFALSE);
}

void recorridoSemantico(nodo *raiz, tabla_simbolos *ts){

    if (!raiz || !raiz->valor) return;
    int linea = raiz->linea;

    switch (raiz->valor->tipo_token){
        case T_PROGRAM:
            raiz_arbol = raiz;
            if (raiz->izq) recorridoSemantico(raiz->izq, ts); // var_decls
            if (raiz->der) recorridoSemantico(raiz->der, ts); // method_decls
            break;
        case T_VAR_DECLS:
            recorridoSemantico(raiz->izq, ts); // var_decls
            recorridoSemantico(raiz->der, ts); // var_decl
            break;
        case T_METHOD_DECLS:
            recorridoSemantico(raiz->izq, ts); // method_decls
            recorridoSemantico(raiz->der, ts); // method_decl
            break;
        case T_PARAMETROS:
            recorridoSemantico(raiz->izq, ts); // parametros
            recorridoSemantico(raiz->der, ts); // parametro
            break;
        case T_STATEMENTS:
            recorridoSemantico(raiz->izq, ts); // statements
            if (raiz->der->valor->tipo_token == T_METHOD_CALL) {
                verificar_asignacion_metodo(raiz->der, ts);
            }
            recorridoSemantico(raiz->der, ts); // statement
            break;
        case T_EXPRS:
            recorridoSemantico(raiz->izq, ts); // exprs
            recorridoSemantico(raiz->der, ts); // expr
            break;
        case T_PARAMETRO:
            // Insertar parametro en tabla de símbolos
            if (!insertar(ts, raiz->valor)) {
                reportar_error(linea, "Parametro '%s' ya declarado\n", raiz->valor->name);
            }
            break;
        case T_VAR_DECL: {
            bool es_global = !es_metodo;
            
            // primero verificar la compatibilidad de tipos en la inicializacion
            if (raiz->der) {
                // si es global, la inicializacion debe ser con literal
                if (es_global && !es_literal_nodo(raiz->der)) {
                    reportar_error(linea, "Error semantico: Las variables globales solo pueden "
                                            "inicializarse con valores literales (números, true, false). "
                                            "Variable '%s'\n", raiz->izq->valor->name);
                    break;
                }
                
                if (raiz->der->valor->tipo_token == T_METHOD_CALL) {
                    verificar_parametros(raiz->der, raiz_arbol, ts);
                }
                tipo_info tipo_expr = calcular_tipo_expresion(raiz->der, ts);

                if (raiz->izq->valor->tipo_info != tipo_expr) {
                    reportar_error(linea, "Error semantico: Incompatibilidad de tipos en la inicializacion."
                                            " La variable '%s' es de tipo '%s', pero la expresion es de tipo '%s'\n",
                                            raiz->izq->valor->name, tipo_info_to_string(raiz->valor->tipo_info), tipo_info_to_string(tipo_expr));
                    // si hay error de tipo, no se inserta en la tabla de simbolos
                    break;
                }
            }

            // solo si no hay error de tipo, insertamos en la tabla de simbolos
            if (!insertar(ts, raiz->izq->valor)) {
                reportar_error(linea, "Variable '%s' ya declarada\n", raiz->izq->valor->name);
            }
            break;
        }
        case T_METHOD_DECL: {
            if (!insertar(ts, raiz->valor)) {
                reportar_error(linea, "Método '%s' ya declarado\n", raiz->valor->name);
            }

            // Verificar si es extern
            es_extern = raiz->der->valor->tipo_token == T_EXTERN;

            abrir_scope(ts);
            es_metodo = true;

            if (strcmp(raiz->valor->name, "main") == 0) {
                hay_main = true;
                if (raiz->valor->tipo_info != TIPO_VOID) {
                    reportar_error(linea, "Error semantico: Main debe ser de tipo void\n");
                }
                if (raiz->izq != NULL) {
                    reportar_error(linea, "Error semantico: Main no debe tener parametros\n");
                }
            }

            raiz->der->valor->tipo_info = raiz->valor->tipo_info; // pasar tipo de metodo a bloque o extern
            recorridoSemantico(raiz->izq, ts); // var_decls
            recorridoSemantico(raiz->der, ts); // bloque o extern

            if (!es_extern) {
                tipo_info retorno = retorno_bloque(raiz->der, ts);
                if (raiz->valor->tipo_info != retorno) {
                    reportar_error(linea, "Error semantico: Tipo de retorno no coincide con la declaracion\n");
                }
                if (raiz->valor->tipo_info == TIPO_VOID && retorno != TIPO_VOID) {
                    reportar_error(linea, "Error semántico: Metodo void no puede retornar valor\n");
                }
            }

            cerrar_scope(ts);
            es_metodo = false;
            break;
        }
        case T_BLOQUE:
            // No abrimos scope aca
            recorridoSemantico(raiz->izq, ts); // var_decls
            recorridoSemantico(raiz->der, ts); // statements
            break;
        case T_ASIGNACION: {
            info *variable = buscar(ts, raiz->izq->valor->name);
            tipo_info tipo_expr = calcular_tipo_expresion(raiz->der, ts);

            if (!variable) {
                reportar_error(linea, "Error semántico: Variable '%s' no declarada\n", raiz->izq->valor->name);
            } else if (variable->tipo_info != tipo_expr) {
                reportar_error(linea, "Error semántico: Incompatibilidad de tipos en la asignacion."
                                        " La variable '%s' es de tipo '%s', pero la expresion es de tipo '%s'\n",
                                        raiz->izq->valor->name, tipo_info_to_string(variable->tipo_info), tipo_info_to_string(tipo_expr));
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
                                        " pero se recibieron tipos '%s' y '%s'\n", tipo_info_to_string(tipo_expr1), tipo_info_to_string(tipo_expr2));
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
                                            " pero se recibio tipo '%s'\n", tipo_info_to_string(tipo_expr));
                }

                recorridoSemantico(raiz->der, ts);
            } else {
                // Menos binario
                tipo_info tipo_expr1 = calcular_tipo_expresion(raiz->izq, ts);
                tipo_info tipo_expr2 = calcular_tipo_expresion(raiz->der, ts);
                
                if (tipo_expr1 != TIPO_INTEGER || tipo_expr2 != TIPO_INTEGER) {
                    reportar_error(linea, "Error semántico: Resta requiere operandos INTEGER"
                                            " pero se recibieron tipos '%s' y '%s'\n", tipo_info_to_string(tipo_expr1), tipo_info_to_string(tipo_expr2));
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
                                        " pero se recibieron tipos '%s' y '%s'\n", tipo_info_to_string(tipo_expr1), tipo_info_to_string(tipo_expr2));
            }

            recorridoSemantico(raiz->izq, ts);
            recorridoSemantico(raiz->der, ts);
            break;
        }
        case T_OP_NOT: {
            tipo_info tipo_expr = calcular_tipo_expresion(raiz->der, ts);

            if (tipo_expr != TIPO_BOOL) {
                reportar_error(linea, "Error semantico: Operador '!' requiere operando BOOL"
                                        " pero se recibio tipo '%s'\n", tipo_info_to_string(tipo_expr));
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
                                        " pero se recibieron tipos '%s' y '%s'\n", tipo_info_to_string(tipo_izq), tipo_info_to_string(tipo_der));
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
                                        " pero se recibieron tipos '%s' y '%s'\n", tipo_info_to_string(tipo_izq), tipo_info_to_string(tipo_der));
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
                reportar_error(linea, "Condición debe ser 'bool' pero es de tipo '%s'\n", tipo_info_to_string(tipo_condicion));
            }

            // Abrir scope para statements internos
            abrir_scope(ts);
            recorridoSemantico(raiz->der, ts);
            cerrar_scope(ts);
            break;
        }

        case T_IF_ELSE: {
            tipo_info tipo_condicion = calcular_tipo_expresion(raiz->izq, ts);

            if (tipo_condicion != TIPO_BOOL) {
                reportar_error(linea, "Condición debe ser 'bool' pero es de tipo '%s'\n", tipo_info_to_string(tipo_condicion));
            }

            // Abrir scope para THEN
            abrir_scope(ts);
            recorridoSemantico(raiz->med, ts);
            cerrar_scope(ts);

            // Abrir scope para ELSE
            abrir_scope(ts);
            recorridoSemantico(raiz->der, ts);
            cerrar_scope(ts);
            break;
        }

        case T_METHOD_CALL: {
            info *metodo = buscar(ts, raiz->izq->valor->name);

            if (!metodo) {
                reportar_error(linea, "Error: semantico: Método '%s' no declarado", raiz->izq->valor->name);
            } else {
                verificar_parametros(raiz, raiz_arbol, ts);
                raiz->valor->tipo_info = metodo->tipo_info;
            }
            break;
        }
        default:
            if (raiz->izq) recorridoSemantico(raiz->izq, ts);
            if (raiz->der) recorridoSemantico(raiz->der, ts);
            break;
    }

}
