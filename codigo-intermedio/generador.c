#include <stdio.h>
#include <string.h>
#include "generador.h"
#include "auxiliares.h"

static int cont_temp = 0;
static int cont_label = 0;
static int ultimo_temp = -1;

// funcion principal de generacion de codigo intermedio
void codigo_intermedio(nodo *raiz) {
    if (raiz == NULL) return;

    int temp_result;

    switch (raiz->valor->tipo_token) {
        case T_PROGRAM:
            if (raiz->izq) codigo_intermedio(raiz->izq);
            if (raiz->der) codigo_intermedio(raiz->der);
            break;
        case T_DIGIT:
        case T_ID: 
        case T_VTRUE:
        case T_VFALSE: {
            // no hacemos nada, ya que el valor esta en el nodo
            // el padre va a usar raiz->der/izq->valor directo
            ultimo_temp = -1;
            break;
        }
        case T_OP_MAS:
        case T_OP_MULT:
        case T_OP_DIV:
        case T_OP_RESTO:
        case T_OP_AND:
        case T_OP_OR:
        case T_IGUALDAD:
        case T_MAYOR:
        case T_MENOR: {
            // generar codigo intermedio para el hijo izquierdo
            codigo_intermedio(raiz->izq);
            
            // si izq no genero temporal, usar el valor directo
            info *izq;
            if (ultimo_temp == -1) {
                izq = raiz->izq->valor;
            } else {
                izq = obtener_temp(ultimo_temp, raiz->izq->valor->tipo_info);
            }

            // generar codigo intermedio para el hijo derecho
            codigo_intermedio(raiz->der);
            
            // si der no genero temporal, usar el valor directo
            info *der;
            if (ultimo_temp == -1) {
                der = raiz->der->valor;
            } else {
                der = obtener_temp(ultimo_temp, raiz->der->valor->tipo_info);
            }

            // crear nuevo temporal para el resultado
            temp_result = cont_temp++;
            info *res = obtener_temp(temp_result, raiz->valor->tipo_info);

            // determinar la instrucción segun el token
            const char *op;
            switch (raiz->valor->tipo_token) {
                case T_OP_MAS:   op = "ADD"; break;
                case T_OP_MULT:  op = "MUL"; break;
                case T_OP_DIV:   op = "DIV"; break;
                case T_OP_RESTO: op = "MOD"; break;
                case T_OP_AND:   op = "AND"; break;
                case T_OP_OR:    op = "OR"; break;
                case T_IGUALDAD: op = "EQ"; break;
                case T_MAYOR:    op = "GT"; break;
                case T_MENOR:    op = "LT"; break;
                default: op = "???"; break;
            }

            // guardar la instruccion
            agregar_instruccion(op, res, izq, der);
            ultimo_temp = temp_result;
            break;
        }
        case T_OP_MENOS: {
            if (raiz->izq == NULL) {
                // menos unario
                codigo_intermedio(raiz->der);
                
                info *der;
                if (ultimo_temp == -1) {
                    der = raiz->der->valor;
                } else {
                    der = obtener_temp(ultimo_temp, raiz->der->valor->tipo_info);
                }

                temp_result = cont_temp++;
                info *res = obtener_temp(temp_result, raiz->valor->tipo_info);

                agregar_instruccion("NEG", res, der, NULL);
                ultimo_temp = temp_result;
            } else {
                // menos binario
                codigo_intermedio(raiz->izq);
                
                info *izq;
                if (ultimo_temp == -1) {
                    izq = raiz->izq->valor;
                } else {
                    izq = obtener_temp(ultimo_temp, raiz->izq->valor->tipo_info);
                }

                codigo_intermedio(raiz->der);
                
                info *der;
                if (ultimo_temp == -1) {
                    der = raiz->der->valor;
                } else {
                    der = obtener_temp(ultimo_temp, raiz->der->valor->tipo_info);
                }

                temp_result = cont_temp++;
                info *res = obtener_temp(temp_result, raiz->valor->tipo_info);

                agregar_instruccion("SUB", res, izq, der);
                ultimo_temp = temp_result;
            }
            break;
        }
        case T_OP_NOT: {
            // como es unario, vemos el hijo der
            codigo_intermedio(raiz->der);
            
            info *der;
            if (ultimo_temp == -1) {
                der = raiz->der->valor;
            } else {
                der = obtener_temp(ultimo_temp, raiz->der->valor->tipo_info);
            }

            temp_result = cont_temp++;
            info *res = obtener_temp(temp_result, raiz->valor->tipo_info);

            // guardar la instruccion
            agregar_instruccion("NOT", res, der, NULL);
            ultimo_temp = temp_result;
            break;
        }
        case T_ASIGNACION: {
            // primero se procesa el lado derecho (la expresion)
            codigo_intermedio(raiz->der);

            info *valor;
            if (ultimo_temp == -1) {
                // Si es constante/variable directa, no necesitamos un temporal intermedio
                valor = raiz->der->valor;
            } else {
                valor = obtener_temp(ultimo_temp, raiz->der->valor->tipo_info);
            }

            info *var = raiz->izq->valor;

            agregar_instruccion("ASSIGN", var, valor, NULL);
            ultimo_temp = -1;
            break;
        }
        case T_VAR_DECL: {
            // primero vemos la expresion del hijo der
            codigo_intermedio(raiz->der);

            info *valor;
            if (ultimo_temp == -1) {
                valor = raiz->der->valor;
            } else {
                valor = obtener_temp(ultimo_temp, raiz->izq->valor->tipo_info);
            }
            
            info *var = raiz->izq->valor;
            agregar_instruccion("ASSIGN", var, valor, NULL);
            break;
        }
        case T_RETURN: {
            if (raiz->izq) {
                // return con expresion
                codigo_intermedio(raiz->izq);
        
                info *valor;
                if (ultimo_temp == -1) {
                    // Es una variable o constante directa (no una expresión)
                    valor = raiz->izq->valor;
                } else {
                    // Es resultado de una expresión (temporal)
                    valor = obtener_temp(ultimo_temp, raiz->izq->valor->tipo_info);
                }
                agregar_instruccion("RET", valor, NULL, NULL);
            } else {
                // return sin expresion
                agregar_instruccion("RET", NULL, NULL, NULL);
            }
            ultimo_temp = -1;
            break;
        }
        case T_METHOD_DECL: {
            if (raiz->der && raiz->der->valor->tipo_token == T_EXTERN) {
                // metodo externo
                agregar_instruccion("EXTERN", raiz->valor, NULL, NULL);
                ultimo_temp = -1;
                break;
            } else {
                raiz->valor->num_parametros = contar_parametros_metodo(raiz->izq);
                agregar_instruccion("LABEL", raiz->valor, NULL, NULL);
                
                if (raiz->izq && raiz->valor->num_parametros > 0) {
                    procesar_parametros_declaracion(raiz->izq, raiz->valor->num_parametros);
                }
                
                if (raiz->der) {
                    codigo_intermedio(raiz->der);
                }

                agregar_instruccion("END", raiz->valor, NULL, NULL);
                ultimo_temp = -1;
            }
            break;
        }
        case T_METHOD_CALL: {
            // la idea es primero evaluar los parametros para despues pasarlos
            // porque si no son evaluados puede que un parametros sea otra funcion, entonces
            // se pasan mal los parametros
            raiz->valor->num_parametros = contar_parametros_metodo(raiz->der);
            info **params_evaluados = NULL;

            if (raiz->der) {
                // reservar espacio para la cantidad de parametros
                params_evaluados = malloc(raiz->valor->num_parametros * sizeof(info*));

                // ahora se evalua cada expresion de parametro
                nodo *arg = raiz->der;
                int idx = raiz->valor->num_parametros - 1;  // llenamos de atras hacia adelante

                while (arg != NULL && idx >= 0) {
                    nodo *expr = (arg->valor->tipo_token == T_EXPRS) ? arg->der : arg;

                    // generar codigo para la expresion del parametro
                    codigo_intermedio(expr);

                    if (ultimo_temp == -1) {
                        params_evaluados[idx] = expr->valor;
                    } else {
                        params_evaluados[idx] = obtener_temp(ultimo_temp, expr->valor->tipo_info);
                    }

                    if (arg->valor->tipo_token == T_EXPRS && arg->izq) {
                        arg = arg->izq;
                    } else {
                        break;
                    }
                    idx--;
                }

                // ahora generar los PARAM en orden correcto
                for (int i = 0; i < raiz->valor->num_parametros; i++) {
                    agregar_instruccion("PARAM", params_evaluados[i], NULL, NULL);
                }

                free(params_evaluados);
            }
        
            // ver que devuelve el metodo
            if (raiz->valor->tipo_info != TIPO_VOID) {
                // si devuelve integer o bool, lo guardamos en un temporal
                temp_result = cont_temp++;
                info *res = obtener_temp(temp_result, raiz->valor->tipo_info);
            
                agregar_instruccion("CALL", res, raiz->izq->valor, NULL);
            
                ultimo_temp = temp_result;
            } else {
                // si es void, no necesitamos un temporal
                agregar_instruccion("CALL", NULL, raiz->izq->valor, NULL);
                ultimo_temp = -1;
            }
            break;
        }
        case T_IF: {
            // generamos codigo intermedio para la condicion
            codigo_intermedio(raiz->izq);
            
            info* cond;
            if (ultimo_temp == -1) {
                cond = raiz->izq->valor;
            } else {
                cond = obtener_temp(ultimo_temp, raiz->valor->tipo_info);
            }

            // aca en vez de usar un arreglo de char, se podria
            // usar directamente char*. ver si cambiarlo o no
            // lo mismo para los case T_IF_ELSE y T_WHILE
            char *label_end = malloc(16);
            snprintf(label_end, 16, "L%d", cont_label++);

            info *endif = obtener_label(label_end);
            free(label_end);
            agregar_instruccion("IF_FALSE", endif, cond, NULL);

            if (raiz->der) codigo_intermedio(raiz->der);

            agregar_instruccion("LABEL", endif, NULL, NULL);

            ultimo_temp = -1;
            break;
        }
        case T_IF_ELSE: {
            // generamos codigo intermedio para la condicion
            codigo_intermedio(raiz->izq);

            // si existe el temporal entonces hay que agarrar el temporal
            info* cond;
            if (ultimo_temp == -1) {
                cond = raiz->izq->valor;
            } else {
                cond = obtener_temp(ultimo_temp, raiz->valor->tipo_info);
            }

            char *label_else = malloc(16);
            char *label_end = malloc(16);

            snprintf(label_else, 16, "L%d", cont_label++);
            snprintf(label_end, 16, "L%d", cont_label++);

            // ver si se cambia IF_FALSE por otra cosa
            agregar_instruccion("IF_FALSE", obtener_label(label_else), cond, NULL);
            // codigo intermedio para el THEN
            if (raiz->med) codigo_intermedio(raiz->med);

            // salto incondicional al final, una vez terminado el THEN
            agregar_instruccion("GOTO", obtener_label(label_end), NULL, NULL);

            // aca inicia el ELSE
            agregar_instruccion("LABEL", obtener_label(label_else), NULL, NULL);
            free(label_else);

            // codigo intermedio para el ELSE
            if (raiz->der) codigo_intermedio(raiz->der);

            agregar_instruccion("LABEL", obtener_label(label_end), NULL, NULL);
            free(label_end);

            ultimo_temp = -1;
            break;
        }
        case T_WHILE: {
            char *label_inicio = malloc(16);
            char *label_end = malloc(16);
            snprintf(label_inicio, 16, "L%d", cont_label++);
            snprintf(label_end, 16, "L%d", cont_label++);

            // label para el inicio del while, antes de la condicion
            agregar_instruccion("LABEL", obtener_label(label_inicio), NULL, NULL);

            // generamos codigo intermedio para la condicion
            codigo_intermedio(raiz->izq);
            info* cond = NULL;
            if (ultimo_temp == -1) {
                cond = raiz->izq->valor;
            } else {
                cond = obtener_temp(ultimo_temp, raiz->valor->tipo_info);
            }

            agregar_instruccion("IF_FALSE", obtener_label(label_end), cond, NULL);

            // cuerpo del while
            if (raiz->der) codigo_intermedio(raiz->der);

            // salto incondicional al inicio, para reevaluar la condicion
            agregar_instruccion("GOTO", obtener_label(label_inicio), NULL, NULL);
            free(label_inicio);

            // label para el fin del while
            agregar_instruccion("LABEL", obtener_label(label_end), NULL, NULL);
            free(label_end);

            ultimo_temp = -1;
            break;
        }
        case T_STATEMENTS:
            // procesar lista de statements
            if (raiz->izq) codigo_intermedio(raiz->izq);
            if (raiz->der) codigo_intermedio(raiz->der);
            break;
        default:
            // casos como el extern y los parametros ya se manejan en sus nodos padres
            codigo_intermedio(raiz->izq);
            codigo_intermedio(raiz->der);
            break;
    }
}