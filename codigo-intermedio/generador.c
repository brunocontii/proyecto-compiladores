#include <stdio.h>
#include <string.h>
#include "generador.h"

int cont_temp = 0;
int cont_label = 0;
int ultimo_temp = -1;

// las funciones de obtener_temp, crear_constante, crear_constante_bool y obtener_label creo que
// se pueden borrar pero para eso hay que manejar bien los temps dentro de cada nodo
// del arbol, ademas de eso habria que cambiar tambien el print de codigo3dir para que
// imprima los campos correctos del info, porque ahora siempre quiere imprimir el name (si no hay name imprime null)

int contar_parametros_decl(nodo *params) {
    if (!params) return 0;
    
    int count = 0;
    nodo *actual = params;
    
    while (actual != NULL) {
        if (actual->valor->tipo_token == T_PARAMETROS || actual->valor->tipo_token == T_EXPRS) {
            count++;
            actual = actual->izq;  // siguiente parámetro
        } else if (actual->valor->tipo_token) {
            count++;
            break;
        } else {
            break;
        }
    }
    
    return count;
}

// crea un nuevo temporal como info
info* obtener_temp(int n, tipo_info ti) {
    if (n < 0) return NULL;

    info *t = (info*)malloc(sizeof(info));
    char buf[16];
    snprintf(buf, sizeof(buf), "T%d", n);
    t->name = strdup(buf);
    t->esTemporal = 1;
    t->tipo_info = ti;            // ver de darle el tipo correcto despues
    t->tipo_token = T_ID;                   // ver de darle el token correcto despues
    return t;
}

// crea una nueva constante entera (literal) cono info
info* crear_constante(int nro) {
    info *c = (info*)malloc(sizeof(info));
    char buf[16];
    snprintf(buf, sizeof(buf), "%d", nro);
    c->name = strdup(buf);
    c->nro = nro;
    c->esTemporal = 0;
    c->tipo_info = TIPO_INTEGER;            // aca creo q esta bien poner integer porque es un numero
    c->tipo_token = T_DIGIT;                // aca creo q esta bien poner digit porque es un numero literal
    return c;
}

// crea una nueva constante booleana como info
info* crear_constante_bool(bool b) {
    info *bool_info = (info*)malloc(sizeof(info));
    char buf[6];
    snprintf(buf, sizeof(buf), "%s", b ? "true" : "false");
    bool_info->name = strdup(buf);
    bool_info->b = b;
    bool_info->bool_string = b ? "true" : "false";
    bool_info->esTemporal = 0;
    bool_info->tipo_info = TIPO_BOOL;                   // tipo bool
    bool_info->tipo_token = b ? T_VTRUE : T_VFALSE;     // token booleano segun como sea
    return bool_info;
}

// crea un nuevo label como info
// ver si necesitamos agregarle mas informacion en algun campo del info
info* obtener_label(const char *label) {
    info *lbl = (info*)malloc(sizeof(info));
    lbl->name = strdup(label);
    lbl->esTemporal = 0;
    return lbl;
}

// esto se usa unicamente en la llamada a un metodo (T_METHOD_CALL)
// ver si es necesario usarla en otro lado (por ej T_METHOD_DECL)
void procesar_argumentos(nodo *raiz, FILE *file) {
    if (!raiz) return;
    
    if (raiz->valor && raiz->valor->tipo_token == T_EXPRS) {
        // primero procesamos el hijo izq (otra lista de parametros)
        if (raiz->izq) procesar_argumentos(raiz->izq, file);

        // generamos codigo para la expresion del parametro actual
        if (raiz->der) {
            codigo_intermedio(raiz->der, file);
            
            info *param = obtener_temp(ultimo_temp, raiz->valor->tipo_info);
            
            agregar_instruccion("PARAM", param, NULL, NULL);
            fprintf(file, "PARAM T%d\n", ultimo_temp);
        }
    } else {
        // caso base de la recursion: un solo parametro
        codigo_intermedio(raiz, file);

        info *param = obtener_temp(ultimo_temp, raiz->valor->tipo_info);

        agregar_instruccion("PARAM", param, NULL, NULL);
        fprintf(file, "PARAM T%d\n", ultimo_temp);
    }
}

// funcion principal de generacion de codigo intermedio
void codigo_intermedio(nodo *raiz, FILE *file) {
    if (raiz == NULL) return;

    int temp_izq, temp_der, temp_result;

    switch (raiz->valor->tipo_token) {
        case T_PROGRAM:
            if (raiz->izq) codigo_intermedio(raiz->izq, file);
            if (raiz->der) codigo_intermedio(raiz->der, file);
            break;
        case T_DIGIT: {
            // literal entero
            temp_result = cont_temp++;
            fprintf(file, "LOAD T%d %d\n", temp_result, raiz->valor->nro);

            info *res = obtener_temp(temp_result, raiz->valor->tipo_info);
            info *arg1 = crear_constante(raiz->valor->nro);

            agregar_instruccion("LOAD", res, arg1, NULL);
            ultimo_temp = temp_result;
            break;
        }
        case T_ID: {
            // hay 2 casos, que sea una variable o un parametro en la declaracion de un metodo
            temp_result = cont_temp++;
            fprintf(file, "LOAD T%d %s\n", temp_result, raiz->valor->name);
            
            info *res = obtener_temp(temp_result, raiz->valor->tipo_info);
            info *arg1 = raiz->valor;

            agregar_instruccion("LOAD", res, arg1, NULL);
            ultimo_temp = temp_result;
            break;
        }
        case T_PARAMETRO: {
            // parametro en la invocacion del metodo
            // cuando la raiz es T_METHOD_DECL no vemos el hijo izq, por ende no entra nunca aca
            // ver que onda si esto esta bien asi
            temp_result = cont_temp++;
            fprintf(file, "PARAM T%d\n", temp_result);

            info *res = obtener_temp(temp_result, raiz->valor->tipo_info);
            info *arg1 = crear_constante(raiz->valor->nro);

            agregar_instruccion("PARAM", res, arg1, NULL);
            ultimo_temp = temp_result;
            break;
        }
        case T_VTRUE:
        case T_VFALSE: {
            temp_result = cont_temp++;

            info *res = obtener_temp(temp_result, raiz->valor->tipo_info);
            info *arg1 = crear_constante_bool(raiz->valor->b);

            agregar_instruccion("LOAD", res, arg1, NULL);
            fprintf(file, "LOAD T%d %s\n", temp_result, raiz->valor->b ? "true" : "false");

            ultimo_temp = temp_result;
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
            codigo_intermedio(raiz->izq, file);
            temp_izq = ultimo_temp;
            info *izq = obtener_temp(temp_izq, raiz->izq->valor->tipo_info);

            // generar codigo intermedio para el hijo derecho
            codigo_intermedio(raiz->der, file);
            temp_der = ultimo_temp;
            info *der = obtener_temp(temp_der, raiz->der->valor->tipo_info);

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
            fprintf(file, "%s T%d T%d T%d\n", op, temp_result, temp_izq, temp_der);
            
            ultimo_temp = temp_result;
            break;
        }
        case T_OP_MENOS: {
            if (raiz->izq == NULL) {
                // menos unario
                codigo_intermedio(raiz->der, file);
                temp_der = ultimo_temp;
                info *der = obtener_temp(temp_der, raiz->valor->tipo_info);

                temp_result = cont_temp++;
                info *res = obtener_temp(temp_result, raiz->valor->tipo_info);

                agregar_instruccion("NEG", res, der, NULL);
                fprintf(file, "NEG T%d T%d\n", temp_result, temp_der);

                ultimo_temp = temp_result;
            } else {
                // menos binario
                codigo_intermedio(raiz->izq, file);
                temp_izq = ultimo_temp;
                info *izq = obtener_temp(temp_izq, raiz->valor->tipo_info);

                codigo_intermedio(raiz->der, file);
                temp_der = ultimo_temp;
                info *der = obtener_temp(temp_der, raiz->valor->tipo_info);

                temp_result = cont_temp++;
                info *res = obtener_temp(temp_result, raiz->valor->tipo_info);

                agregar_instruccion("SUB", res, izq, der);
                fprintf(file, "SUB T%d T%d T%d\n", temp_result, temp_izq, temp_der);

                ultimo_temp = temp_result;
            }
            break;
        }
        case T_OP_NOT: {
            // como es unario, vemos el hijo der
            codigo_intermedio(raiz->der, file);
            int temp_der = ultimo_temp;
            info *der = obtener_temp(temp_der, raiz->valor->tipo_info);

            // crear un nuevo temporal para el resultado
            int temp_result = cont_temp++;
            info *res = obtener_temp(temp_result, raiz->valor->tipo_info);

            // guardar la instruccion
            agregar_instruccion("NOT", res, der, NULL);
            fprintf(file, "NOT T%d T%d\n", temp_result, temp_der);
            
            ultimo_temp = temp_result;
            break;
        }
        case T_ASIGNACION: {
            // primero se procesa el lado derecho (la expresion)
            codigo_intermedio(raiz->der, file);

            // el resultado queda en el ultimo temporal
            info *valor = obtener_temp(ultimo_temp, raiz->der->valor->tipo_info);

            // el lado izq es la variable
            info *var = raiz->izq->valor;

            agregar_instruccion("ASSIGN", var, valor, NULL);
            fprintf(file, "ASSIGN %s T%d\n", var->name, ultimo_temp);

            ultimo_temp = -1;
            break;
        }
        case T_VAR_DECL: {
            // primero vemos la expresion del hijo der
            codigo_intermedio(raiz->der, file);

            // aca yo se que es una nueva variable local, sumo para saber cuantas 
            // variables locales tengo en el metodo corriente
            info *valor = obtener_temp(ultimo_temp, raiz->izq->valor->tipo_info);
            info *var = raiz->izq->valor;
            agregar_instruccion("ASSIGN", var, valor, NULL);
            fprintf(file, "ASSIGN %s T%d\n", var->name, ultimo_temp);
            break;
        }
        case T_RETURN: {
            if (raiz->izq) {
                // return con expresion
                codigo_intermedio(raiz->izq, file);

                info *valor = obtener_temp(ultimo_temp, raiz->valor->tipo_info);
                agregar_instruccion("RET", valor, NULL, NULL);
                fprintf(file, "RET T%d\n", ultimo_temp);
            } else {
                // return sin expresion
                agregar_instruccion("RET", NULL, NULL, NULL);
                fprintf(file, "RET\n");
            }
            ultimo_temp = -1;
            break;
        }
        case T_METHOD_DECL: {
            if (raiz->izq && raiz->izq->valor->tipo_token == T_EXTERN) {
                // metodo externo
                fprintf(file, "EXTERN %s\n", raiz->valor->name);
                agregar_instruccion("EXTERN", raiz->valor, NULL, NULL);
                ultimo_temp = -1;
                break;
            } else {
                int num_params = contar_parametros_decl(raiz->izq);
                raiz->valor->num_parametros = num_params;
                fprintf(file, "LABEL %s\n", raiz->valor->name);
                agregar_instruccion("LABEL", raiz->valor, NULL, NULL);
                // ver si hacer algo con los parametros aca o no (antes de ver el hijo der)
                // generamos codigo intermedio para el cuerpo del metodo
                if (raiz->der) {
                    codigo_intermedio(raiz->der, file);
                }

                // remarcar el fin del metodo (lo podemos sacar si queremos)
                // capaz sirve, capaz no
                fprintf(file, "END %s\n", raiz->valor->name);
                agregar_instruccion("END", raiz->valor, NULL, NULL);
                ultimo_temp = -1;
            }
            break;
        }
        case T_METHOD_CALL: {
            if (raiz->der) {
                procesar_argumentos(raiz->der, file);
            }

            // ver que devuelve el metodo
            if (raiz->valor->tipo_info != TIPO_VOID) {
                // si devuelve integer o bool, lo guardamos en un temporal
                temp_result = cont_temp++;
                info *res = obtener_temp(temp_result, raiz->valor->tipo_info);

                fprintf(file, "CALL T%d %s\n", temp_result, raiz->izq->valor->name);
                agregar_instruccion("CALL", res, raiz->izq->valor, NULL);

                ultimo_temp = temp_result;
            } else {
                // si es void, no necesitamos un temporal
                fprintf(file, "CALL %s\n", raiz->izq->valor->name);
                agregar_instruccion("CALL", raiz->izq->valor, NULL, NULL);
                ultimo_temp = -1;
            }
            break;
        }
        case T_EXTERN: {
            // ver si queremos agregar algo mas aca
            fprintf(file, "EXTERN\n");
            agregar_instruccion("EXTERN", NULL, NULL, NULL);
            ultimo_temp = -1;
            break;
        }
        case T_IF: {
            // generamos codigo intermedio para la condicion
            codigo_intermedio(raiz->izq, file);
            temp_izq = ultimo_temp;
            info *cond = obtener_temp(temp_izq, raiz->valor->tipo_info);

            // aca en vez de usar un arreglo de char, se podria
            // usar directamente char*. ver si cambiarlo o no
            // lo mismo para los case T_IF_ELSE y T_WHILE
            char label_end[16];
            snprintf(label_end, sizeof(label_end), "L%d", cont_label++);

            fprintf(file, "IF_FALSE T%d %s\n", temp_izq, label_end);
            info *endif = obtener_label(label_end);
            agregar_instruccion("IF_FALSE", cond, endif, NULL);

            if (raiz->der) codigo_intermedio(raiz->der, file);

            fprintf(file, "LABEL %s\n", label_end);
            agregar_instruccion("LABEL", endif, NULL, NULL);

            ultimo_temp = -1;
            break;
        }
        case T_IF_ELSE: {
            // generamos codigo intermedio para la condicion
            codigo_intermedio(raiz->izq, file);
            temp_izq = ultimo_temp;
            info *cond = obtener_temp(temp_izq, raiz->valor->tipo_info);

            char label_else[16], label_end[16];
            snprintf(label_else, sizeof(label_else), "L%d", cont_label++);
            snprintf(label_end, sizeof(label_end), "L%d", cont_label++);

            fprintf(file, "IF_FALSE T%d %s\n", temp_izq, label_else);
            // ver si se cambia IF_FALSE por otra cosa
            agregar_instruccion("IF_FALSE", cond, obtener_label(label_else), NULL);

            // codigo intermedio para el THEN
            if (raiz->med) codigo_intermedio(raiz->med, file);

            // salto incondicional al final, una vez terminado el THEN
            fprintf(file, "GOTO %s\n", label_end);
            agregar_instruccion("GOTO", obtener_label(label_end), NULL, NULL);

            // aca inicia el ELSE
            fprintf(file, "LABEL %s\n", label_else);
            agregar_instruccion("LABEL", obtener_label(label_else), NULL, NULL);

            // codigo intermedio para el ELSE
            if (raiz->der) codigo_intermedio(raiz->der, file);

            fprintf(file, "LABEL %s\n", label_end);
            agregar_instruccion("LABEL", obtener_label(label_end), NULL, NULL);

            ultimo_temp = -1;
            break;
        }
        case T_WHILE: {
            char label_inicio[16], label_end[16];
            snprintf(label_inicio, sizeof(label_inicio), "L%d", cont_label++);
            snprintf(label_end, sizeof(label_end), "L%d", cont_label++);

            // label para el inicio del while, antes de la condicion
            fprintf(file, "LABEL %s\n", label_inicio);
            agregar_instruccion("LABEL", obtener_label(label_inicio), NULL, NULL);

            // generamos codigo intermedio para la condicion
            codigo_intermedio(raiz->izq, file);
            temp_izq = ultimo_temp;
            info *cond = obtener_temp(temp_izq, raiz->valor->tipo_info);

            fprintf(file, "IF_FALSE T%d %s\n", temp_izq, label_end);
            agregar_instruccion("IF_FALSE", cond, obtener_label(label_end), NULL);

            // cuerpo del while
            if (raiz->der) codigo_intermedio(raiz->der, file);

            // salto incondicional al inicio, para reevaluar la condicion
            fprintf(file, "GOTO %s\n", label_inicio);
            agregar_instruccion("GOTO", obtener_label(label_inicio), NULL, NULL);

            // label para el fin del while
            fprintf(file, "LABEL %s\n", label_end);
            agregar_instruccion("LABEL", obtener_label(label_end), NULL, NULL);

            ultimo_temp = -1;
            break;
        }
        default:
            codigo_intermedio(raiz->izq, file);
            codigo_intermedio(raiz->der, file);
            break;
    }
}