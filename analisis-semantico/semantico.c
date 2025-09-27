#include <stdio.h>
#include <string.h>
#include "semantico.h"
#include "../utils/manejo_errores.h"

bool es_metodo = true;
bool es_extern = false;
bool hay_main = false;
nodo *raiz_arbol = NULL;

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

    while (parametros_actuales && parametros_formales) {
        bool formal_es_lista = (parametros_formales->valor->tipo_token == T_PARAMETROS);
        bool actual_es_lista = (parametros_actuales->valor->tipo_token == T_EXPRS);
        
        printf("DEBUG: formal_es_lista=%d, actual_es_lista=%d\n", formal_es_lista, actual_es_lista);

        if (formal_es_lista && actual_es_lista) {
            // Comparar los parámetros actuales (der)
            if (parametros_actuales->der && parametros_formales->der) {
                tipo_info tipo_par_actual = calcular_tipo_expresion(parametros_actuales->der, ts);
                tipo_info tipo_par_formal = parametros_formales->der->valor->tipo_info;

                printf("DEBUG: Comparando tipos: formal=%d, actual=%d\n", tipo_par_formal, tipo_par_actual);

                if (tipo_par_actual != tipo_par_formal) {
                    reportar_error(method_call->linea, "Error semantico: En la llamada al metodo '%s', "
                                    "el tipo del parametro no coincide con la declaracion. Se esperaba '%d' pero se recibio '%d'\n",
                                    nombre_metodo_call, tipo_par_formal, tipo_par_actual);
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

    // Verificar casos de cantidad incorrecta de parámetros
    if (parametros_formales && !parametros_actuales) {
        reportar_error(method_call->linea, "Error semantico: En la llamada al metodo '%s', "
                        "faltan parametros. Se esperaba mas parametros.\n", nombre_metodo_call);
        return;
    } else if (parametros_actuales && !parametros_formales) {
        reportar_error(method_call->linea, "Error semantico: En la llamada al metodo '%s', "
                        "sobran parametros. Se esperaba menos parametros.\n", nombre_metodo_call);
        return;
    }

    // Verificar el último parámetro (las hojas)
    if (parametros_actuales && parametros_formales) {
        tipo_info tipo_actual_hoja = calcular_tipo_expresion(parametros_actuales->der, ts);

        if (parametros_formales->valor) {
            tipo_info tipo_formal_hoja = parametros_formales->valor->tipo_info;

            printf("DEBUG: Comparando hojas: formal=%d, actual=%d\n", tipo_formal_hoja, tipo_actual_hoja);

            if (tipo_actual_hoja != tipo_formal_hoja) {
                reportar_error(method_call->linea, "Error semantico: En la llamada al metodo '%s', "
                                "el tipo del parametro no coincide con la declaracion. Se esperaba '%s' pero se recibio '%s'\n",
                                nombre_metodo_call, tipo_info_to_string(tipo_formal_hoja), tipo_info_to_string(tipo_actual_hoja));
            }
        }
    }
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

            printf("Nombre del metodo actual: %s\n",raiz->valor->name);
            imprimir_scope_actual(ts);

            // Verificar si es extern
            es_extern = raiz->der->valor->tipo_token == T_EXTERN;

            // Abrir scope solo si NO es extern
            if (!es_extern) {
                abrir_scope(ts);
                es_metodo = true;
            }


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

            if (!es_extern && raiz->der->valor->tipo_token != T_EXTERN) {
                tipo_info retorno = retorno_bloque(raiz->der, ts);
                if (raiz->valor->tipo_info != retorno) {
                    printf("lado izq %d\n",raiz->valor->tipo_info);
                    printf("retorno %d\n",retorno);
                    reportar_error(linea, "Error semantico: Tipo de retorno no coincide con la declaracion\n");
                }
                if (raiz->valor->tipo_info == TIPO_VOID && retorno != TIPO_VOID) {
                    reportar_error(linea, "Error semántico: Metodo void no puede retornar valor\n");
                }
            }

            if (!es_extern) {
                cerrar_scope(ts); // cerramos solo los scopes internos
            }
            //es_metodo = false;
            break;
        }
        case T_BLOQUE:
            // No abrimos scope aca
            recorridoSemantico(raiz->izq, ts); // var_decls
            recorridoSemantico(raiz->der, ts); // statements
            break;

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
            if (tipo_condicion != TIPO_BOOL)
                reportar_error(linea, "Condición debe ser BOOL\n");

            // Abrir scope para statements internos
            abrir_scope(ts);
            recorridoSemantico(raiz->der, ts);
            cerrar_scope(ts);
            break;
        }

        case T_IF_ELSE: {
            tipo_info tipo_condicion = calcular_tipo_expresion(raiz->izq, ts);
            if (tipo_condicion != TIPO_BOOL)
                reportar_error(linea, "Condición debe ser BOOL\n");

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
            }
            
            verificar_parametros(raiz, raiz_arbol, ts);
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
            }
            return TIPO_VOID;
        }

        case T_OP_MENOS: {
            if (expr->izq == NULL) {
                // menos unario
                tipo_info tipo_der = calcular_tipo_expresion(expr->der, ts);
                return (tipo_der == TIPO_INTEGER) ? TIPO_INTEGER : TIPO_VOID;
            } else {
                // menos binario
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
            info *metodo_info = buscar(ts, expr->izq->valor->name);
            return metodo_info ? metodo_info->tipo_info : TIPO_VOID;
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


