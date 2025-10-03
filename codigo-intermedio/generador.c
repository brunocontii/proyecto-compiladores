#include <stdio.h>
#include "generador.h"

int cont_temp = 0;
int ultimo_temp = -1;
int cont_label = 0;

void codigo_intermedio(nodo *raiz, FILE *file) {
    if (raiz == NULL) return;
    int temp_izq;
    int temp_der;
    int temp_med;
    int temp_result;

    switch (raiz->valor->tipo_token) {
        case T_PROGRAM:
            if (raiz->izq) codigo_intermedio(raiz->izq, file);
            if (raiz->der) codigo_intermedio(raiz->der, file);
            break;
        case T_DIGIT: {
            int temp = cont_temp++; 
            fprintf(file, "LOAD T%d %d\n", temp, raiz->valor->nro);
            ultimo_temp = temp;
            break;
        }
        case T_ID: {
            int temp = cont_temp++; 
            fprintf(file, "LOAD T%d %s\n", temp, raiz->valor->name);
            ultimo_temp = temp;
            break;
        }
        case T_PARAMETRO:{
            int temp = cont_temp++; 
            fprintf(file, "LOAD T%d %s\n", temp, raiz->valor->name);
            ultimo_temp = temp;
            break;
        }
        case T_VTRUE: {
            int temp = cont_temp++;
            fprintf(file, "LOAD T%d true\n", temp);
            ultimo_temp = temp;
            break;
        }
        case T_VFALSE: {
            int temp = cont_temp++;
            fprintf(file, "LOAD T%d false\n", temp);
            ultimo_temp = temp;
            break;
        }
        case T_OP_MAS: {
            codigo_intermedio(raiz->izq, file);
            temp_izq = ultimo_temp;
            codigo_intermedio(raiz->der, file);
            temp_der = ultimo_temp;
            temp_result = cont_temp++;
            fprintf(file, "ADD T%d T%d T%d\n", temp_result, temp_izq, temp_der);
            ultimo_temp = temp_result;
            break;
        }
        case T_OP_MENOS: {
            if (raiz->izq == NULL) {
                // Menos Unario
                codigo_intermedio(raiz->der, file);
                temp_der = ultimo_temp;
                temp_result = cont_temp++;
                fprintf(file, "NEG T%d T%d\n", temp_result, temp_der);
                ultimo_temp = temp_result;
            } else {
                codigo_intermedio(raiz->izq, file);
                temp_izq = ultimo_temp;
                codigo_intermedio(raiz->der, file);
                temp_der = ultimo_temp;
                temp_result = cont_temp++;
                fprintf(file, "SUB T%d T%d T%d\n", temp_result, temp_izq, temp_der);
                ultimo_temp = temp_result;
            }
            break;
        }
        case T_OP_MULT: {
            codigo_intermedio(raiz->izq, file);
            temp_izq = ultimo_temp;
            codigo_intermedio(raiz->der, file);
            temp_der = ultimo_temp;
            temp_result = cont_temp++;
            fprintf(file, "MUL T%d T%d T%d\n", temp_result, temp_izq, temp_der);
            ultimo_temp = temp_result;
            break;
        }
        case T_OP_DIV: {
            codigo_intermedio(raiz->izq, file);
            temp_izq = ultimo_temp;
            codigo_intermedio(raiz->der, file);
            temp_der = ultimo_temp;
            temp_result = cont_temp++;
            fprintf(file, "DIV T%d T%d T%d\n", temp_result, temp_izq, temp_der);
            ultimo_temp = temp_result;
            break;
        }
        case T_OP_RESTO: {
            codigo_intermedio(raiz->izq, file);
            temp_izq = ultimo_temp;
            codigo_intermedio(raiz->der, file);
            temp_der = ultimo_temp;
            temp_result = cont_temp++;
            fprintf(file, "MOD T%d T%d T%d\n", temp_result, temp_izq, temp_der);
            ultimo_temp = temp_result;
            break;
        }
        case T_OP_AND: {
            codigo_intermedio(raiz->izq, file);
            temp_izq = ultimo_temp;
            codigo_intermedio(raiz->der, file);
            temp_der = ultimo_temp;
            temp_result = cont_temp++;
            fprintf(file, "AND T%d T%d T%d\n", temp_result, temp_izq, temp_der);
            ultimo_temp = temp_result;
            break;
        }
        case T_OP_OR: {
            codigo_intermedio(raiz->izq, file);
            temp_izq = ultimo_temp;
            codigo_intermedio(raiz->der, file);
            temp_der = ultimo_temp;
            temp_result = cont_temp++;
            fprintf(file, "OR T%d T%d T%d\n", temp_result, temp_izq, temp_der);
            ultimo_temp = temp_result;
            break;
        }
        case T_OP_NOT: {
            codigo_intermedio(raiz->der, file);
            temp_der = ultimo_temp;
            temp_result = cont_temp++;
            fprintf(file, "NOT T%d T%d\n", temp_result, temp_der);
            ultimo_temp = temp_result;
            break;
        }
        case T_IGUALDAD:  {
            codigo_intermedio(raiz->izq, file);
            temp_izq = ultimo_temp;
            codigo_intermedio(raiz->der, file);
            temp_der = ultimo_temp;
            temp_result = cont_temp++;
            fprintf(file, "EQ T%d T%d T%d\n", temp_result, temp_izq, temp_der);
            ultimo_temp = temp_result;
            break;
        }
        case T_MAYOR: {
            codigo_intermedio(raiz->izq, file);
            temp_izq = ultimo_temp;
            codigo_intermedio(raiz->der, file);
            temp_der = ultimo_temp;
            temp_result = cont_temp++;
            fprintf(file, "GT T%d T%d T%d\n", temp_result, temp_izq, temp_der);
            ultimo_temp = temp_result;
            break;
        }
        case T_MENOR: {
            codigo_intermedio(raiz->izq, file);
            temp_izq = ultimo_temp;
            codigo_intermedio(raiz->der, file);
            temp_der = ultimo_temp;
            temp_result = cont_temp++;
            fprintf(file, "LT T%d T%d T%d\n", temp_result, temp_izq, temp_der);
            ultimo_temp = temp_result;
            break;
        }
        case T_ASIGNACION: {
            codigo_intermedio(raiz->der, file);
            temp_der = ultimo_temp;
            fprintf(file, "ASSIGN %s T%d\n", raiz->izq->valor->name, temp_der);
            ultimo_temp = -1;
            break;
        }
        case T_VAR_DECL: {
            if (raiz->der != NULL) {
                codigo_intermedio(raiz->der, file);
                temp_der = ultimo_temp;
                fprintf(file, "ASSIGN %s T%d\n", raiz->izq->valor->name, temp_der);
            }
            ultimo_temp = -1;
            break;
        }
        case T_RETURN: {
            if (raiz->izq) {
                codigo_intermedio(raiz->izq, file);
                temp_der = ultimo_temp;
                fprintf(file, "RET T%d\n", temp_der);
            } else {
                fprintf(file, "RET\n");
            }
            ultimo_temp = -1;
            break;
        }
        case T_METHOD_DECL: {
            fprintf(file, "%s:\n", raiz->valor->name);
            if (raiz->izq) codigo_intermedio(raiz->izq, file);
            codigo_intermedio(raiz->der, file);
            break;
        }
        case T_METHOD_CALL: {
            if (raiz->der) {
                procesar_argumentos(raiz->der, file);
            }

            if (raiz->valor->tipo_info == TIPO_VOID) {
                fprintf(file, "CALL %s\n", raiz->izq->valor->name);
                ultimo_temp = -1;
            } else {
                temp_result = cont_temp++;
                fprintf(file, "CALL T%d %s\n", temp_result, raiz->izq->valor->name);
                ultimo_temp = temp_result;
            }
            if (raiz->der) codigo_intermedio(raiz->der, file);
            break;
        }
        case T_EXTERN: {
            fprintf(file, "EXTERN\n");
            break;
        }
        default:
            codigo_intermedio(raiz->izq, file);
            codigo_intermedio(raiz->der, file);
            break;
    }
}

