#include <stdio.h>
#include <string.h>
#include "generador.h"

int cont_temp = 0;
int ultimo_temp = -1;
int cont_label = 0;
codigo3dir programa[2000]; // 2000 instrucciones maximo
int cont_instrucciones = 0;

void codigo_intermedio(nodo *raiz, FILE *file) {
    if (raiz == NULL) return;

    int temp_izq;
    int temp_der;
    int temp_result;

    switch (raiz->valor->tipo_token) {
        case T_PROGRAM:
            if (raiz->izq) codigo_intermedio(raiz->izq, file);
            if (raiz->der) codigo_intermedio(raiz->der, file);
            break;
        case T_DIGIT: {
            int temp = cont_temp++; 
            fprintf(file, "LOAD T%d %d\n", temp, raiz->valor->nro);
            
            codigo3dir inst;
            strcpy(inst.instruccion, "LOAD");
            sprintf(inst.resultado, "T%d", temp);
            sprintf(inst.argumento1, "%d", raiz->valor->nro);
            inst.argumento2[0] = '\0';

            programa[cont_instrucciones] = inst;
            cont_instrucciones++;

            ultimo_temp = temp;
            break;
        }
        case T_ID: {
            int temp = cont_temp++; 
            fprintf(file, "LOAD T%d %s\n", temp, raiz->valor->name);

            codigo3dir inst;
            strcpy(inst.instruccion, "LOAD");
            sprintf(inst.resultado, "T%d", temp);
            sprintf(inst.argumento1, "%s", raiz->valor->name);
            inst.argumento2[0] = '\0';

            programa[cont_instrucciones] = inst;
            cont_instrucciones++;

            ultimo_temp = temp;
            break;
        }
        case T_PARAMETRO:{
            int temp = cont_temp++; 
            fprintf(file, "LOAD T%d %s\n", temp, raiz->valor->name);

            codigo3dir inst;
            strcpy(inst.instruccion, "LOAD");
            sprintf(inst.resultado, "T%d", temp);
            sprintf(inst.argumento1, "%s", raiz->valor->name);
            inst.argumento2[0] = '\0';

            programa[cont_instrucciones] = inst;
            cont_instrucciones++;

            ultimo_temp = temp;
            break;
        }
        case T_VTRUE: {
            int temp = cont_temp++;
            fprintf(file, "LOAD T%d true\n", temp);

            codigo3dir inst;
            strcpy(inst.instruccion, "LOAD");
            sprintf(inst.resultado, "T%d", temp);
            sprintf(inst.argumento1, "%s", "true");
            inst.argumento2[0] = '\0';

            programa[cont_instrucciones] = inst;
            cont_instrucciones++;

            ultimo_temp = temp;
            break;
        }
        case T_VFALSE: {
            int temp = cont_temp++;
            fprintf(file, "LOAD T%d false\n", temp);

            codigo3dir inst;
            strcpy(inst.instruccion, "LOAD");
            sprintf(inst.resultado, "T%d", temp);
            sprintf(inst.argumento1, "%s", "false");
            inst.argumento2[0] = '\0';

            programa[cont_instrucciones] = inst;
            cont_instrucciones++;

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

            codigo3dir inst;
            strcpy(inst.instruccion, "ADD");
            sprintf(inst.resultado, "T%d", temp_result);
            sprintf(inst.argumento1, "T%d", temp_izq);
            sprintf(inst.argumento2, "T%d", temp_der);

            programa[cont_instrucciones] = inst;
            cont_instrucciones++;

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

                codigo3dir inst;
                strcpy(inst.instruccion, "NEG");
                sprintf(inst.resultado, "T%d", temp_result);
                sprintf(inst.argumento1, "T%d", temp_der);
                inst.argumento2[0] = '\0';

                programa[cont_instrucciones] = inst;
                cont_instrucciones++;

                ultimo_temp = temp_result;
            } else {
                codigo_intermedio(raiz->izq, file);
                temp_izq = ultimo_temp;
                codigo_intermedio(raiz->der, file);
                temp_der = ultimo_temp;
                temp_result = cont_temp++;
                fprintf(file, "SUB T%d T%d T%d\n", temp_result, temp_izq, temp_der);

                codigo3dir inst;
                strcpy(inst.instruccion, "SUB");
                sprintf(inst.resultado, "T%d", temp_result);
                sprintf(inst.argumento1, "T%d", temp_izq);
                sprintf(inst.argumento2, "T%d", temp_der);

                programa[cont_instrucciones] = inst;
                cont_instrucciones++;

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

            codigo3dir inst;
            strcpy(inst.instruccion, "MUL");
            sprintf(inst.resultado, "T%d", temp_result);
            sprintf(inst.argumento1, "T%d", temp_izq);
            sprintf(inst.argumento2, "T%d", temp_der);

            programa[cont_instrucciones] = inst;
            cont_instrucciones++;

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

            codigo3dir inst;
            strcpy(inst.instruccion, "DIV");
            sprintf(inst.resultado, "T%d", temp_result);
            sprintf(inst.argumento1, "T%d", temp_izq);
            sprintf(inst.argumento2, "T%d", temp_der);

            programa[cont_instrucciones] = inst;
            cont_instrucciones++;

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

            codigo3dir inst;
            strcpy(inst.instruccion, "MOD");
            sprintf(inst.resultado, "T%d", temp_result);
            sprintf(inst.argumento1, "T%d", temp_izq);
            sprintf(inst.argumento2, "T%d", temp_der);

            programa[cont_instrucciones] = inst;
            cont_instrucciones++;

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
    
            codigo3dir inst;
            strcpy(inst.instruccion, "AND");
            sprintf(inst.resultado, "T%d", temp_result);
            sprintf(inst.argumento1, "T%d", temp_izq);
            sprintf(inst.argumento2, "T%d", temp_der);

            programa[cont_instrucciones] = inst;
            cont_instrucciones++;
        
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

            codigo3dir inst;
            strcpy(inst.instruccion, "OR");
            sprintf(inst.resultado, "T%d", temp_result);
            sprintf(inst.argumento1, "T%d", temp_izq);
            sprintf(inst.argumento2, "T%d", temp_der);

            programa[cont_instrucciones] = inst;
            cont_instrucciones++;

            ultimo_temp = temp_result;
            break;
        }
        case T_OP_NOT: {
            codigo_intermedio(raiz->der, file);
            temp_der = ultimo_temp;
            temp_result = cont_temp++;
            fprintf(file, "NOT T%d T%d\n", temp_result, temp_der);

            codigo3dir inst;
            strcpy(inst.instruccion, "NOT");
            sprintf(inst.resultado, "T%d", temp_result);
            sprintf(inst.argumento1, "T%d", temp_der);
            inst.argumento2[0] = '\0';

            programa[cont_instrucciones] = inst;
            cont_instrucciones++;

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

            codigo3dir inst;
            strcpy(inst.instruccion, "EQ");
            sprintf(inst.resultado, "T%d", temp_result);
            sprintf(inst.argumento1, "T%d", temp_izq);
            sprintf(inst.argumento2, "T%d", temp_der);

            programa[cont_instrucciones] = inst;
            cont_instrucciones++;
            
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

            codigo3dir inst;
            strcpy(inst.instruccion, "GT");
            sprintf(inst.resultado, "T%d", temp_result);
            sprintf(inst.argumento1, "T%d", temp_izq);
            sprintf(inst.argumento2, "T%d", temp_der);

            programa[cont_instrucciones] = inst;
            cont_instrucciones++;

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

            codigo3dir inst;
            strcpy(inst.instruccion, "LT");
            sprintf(inst.resultado, "T%d", temp_result);
            sprintf(inst.argumento1, "T%d", temp_izq);
            sprintf(inst.argumento2, "T%d", temp_der);

            programa[cont_instrucciones] = inst;
            cont_instrucciones++;

            ultimo_temp = temp_result;
            break;
        }
        case T_ASIGNACION: {
            codigo_intermedio(raiz->der, file);
            temp_der = ultimo_temp;
            fprintf(file, "ASSIGN %s T%d\n", raiz->izq->valor->name, temp_der);

            codigo3dir inst;
            strcpy(inst.instruccion, "ASSING");
            sprintf(inst.resultado, "%s", raiz->izq->valor->name);
            sprintf(inst.argumento1, "T%d", temp_der);
            inst.argumento2[0] = '\0';

            programa[cont_instrucciones] = inst;
            cont_instrucciones++;

            ultimo_temp = -1;
            break;
        }
        case T_VAR_DECL: {
            if (raiz->der != NULL) {
                codigo_intermedio(raiz->der, file);
                temp_der = ultimo_temp;
                fprintf(file, "ASSIGN %s T%d\n", raiz->izq->valor->name, temp_der);

                codigo3dir inst;
                strcpy(inst.instruccion, "ASSING");
                sprintf(inst.resultado, "%s", raiz->izq->valor->name);
                sprintf(inst.argumento1, "T%d", temp_der);
                inst.argumento2[0] = '\0';

                programa[cont_instrucciones] = inst;
                cont_instrucciones++;
            }
            ultimo_temp = -1;
            break;
        }
        case T_RETURN: {
            if (raiz->izq) {
                codigo_intermedio(raiz->izq, file);
                temp_der = ultimo_temp;
                fprintf(file, "RET T%d\n", temp_der);

                codigo3dir inst;
                strcpy(inst.instruccion, "RET");
                sprintf(inst.resultado, "T%d", temp_der);
                inst.argumento1[0] = '\0';
                inst.argumento2[0] = '\0';

                programa[cont_instrucciones] = inst;
                cont_instrucciones++;
            } else {
                fprintf(file, "RET\n");

                codigo3dir inst;
                strcpy(inst.instruccion, "RET");
                inst.resultado[0] = '\0';
                inst.argumento1[0] = '\0';
                inst.argumento2[0] = '\0';

                programa[cont_instrucciones] = inst;
                cont_instrucciones++;
            }
            ultimo_temp = -1;
            break;
        }
        case T_METHOD_DECL: {
            if (raiz->izq && raiz->izq->valor->tipo_token == T_EXTERN) {
                fprintf(file, "EXTERN %s\n", raiz->valor->name);
                
                codigo3dir inst;
                strcpy(inst.instruccion, "EXTERN");
                sprintf(inst.resultado, "%s", raiz->valor->name);
                inst.argumento1[0] = '\0';
                inst.argumento2[0] = '\0';

                programa[cont_instrucciones] = inst;
                cont_instrucciones++;

                break;
            }   
            
            fprintf(file, "%s:\n", raiz->valor->name);
            codigo_intermedio(raiz->der, file);
            break;
        }
        case T_METHOD_CALL: {
            if (raiz->der) {
                procesar_argumentos(raiz->der, file);
            }

            if (raiz->valor->tipo_info == TIPO_VOID) {
                fprintf(file, "CALL %s\n", raiz->izq->valor->name);

                codigo3dir inst;
                strcpy(inst.instruccion, "CALL");
                sprintf(inst.resultado, "%s", raiz->izq->valor->name);
                inst.argumento1[0] = '\0';
                inst.argumento2[0] = '\0';

                programa[cont_instrucciones] = inst;
                cont_instrucciones++;
                
                ultimo_temp = -1;
            } else {
                temp_result = cont_temp++;
                fprintf(file, "CALL T%d %s\n", temp_result, raiz->izq->valor->name);

                codigo3dir inst;
                strcpy(inst.instruccion, "CALL");
                sprintf(inst.resultado, "T%d", temp_result);
                sprintf(inst.argumento1, "%s", raiz->izq->valor->name);
                inst.argumento2[0] = '\0';

                programa[cont_instrucciones] = inst;
                cont_instrucciones++;

                ultimo_temp = temp_result;
            }
            break;
        }
        case T_EXTERN: {
            fprintf(file, "EXTERN\n");

            codigo3dir inst;
            strcpy(inst.instruccion, "EXTERN");
            sprintf(inst.resultado, "%s", raiz->valor->name);
            inst.argumento1[0] = '\0';
            inst.argumento2[0] = '\0';

            programa[cont_instrucciones] = inst;
            cont_instrucciones++;

            break;
        }
        case T_IF: {
            codigo_intermedio(raiz->izq, file);
            temp_izq = ultimo_temp;

            int label_end = cont_label++;

            fprintf(file, "Salto cond: if T%d L%d\n", temp_izq, label_end);
            
            codigo3dir inst;
            strcpy(inst.instruccion, "Salto cond: if");
            sprintf(inst.resultado, "T%d", temp_izq);
            sprintf(inst.argumento1, "L%d", label_end);
            inst.argumento2[0] = '\0';

            codigo_intermedio(raiz->der, file);

            fprintf(file, "L%d:\n", label_end);
            ultimo_temp = -1;
            break;
        }
        case T_IF_ELSE: {
            codigo_intermedio(raiz->izq, file);
            temp_izq = ultimo_temp;

            int label_else = cont_label++;
            int label_end = cont_label++;

            fprintf(file, "Salto cond: if else T%d L%d\n", temp_izq, label_else);

            codigo3dir inst;
            strcpy(inst.instruccion, "Salto cond: if else");
            sprintf(inst.resultado, "T%d", temp_izq);
            sprintf(inst.argumento1, "L%d", label_else);
            inst.argumento2[0] = '\0';

            programa[cont_instrucciones] = inst;
            cont_instrucciones++;

            codigo_intermedio(raiz->med, file);

            fprintf(file, "Salto incond: if else L%d\n", label_end);

            codigo3dir inst2;
            strcpy(inst2.instruccion, "Salto incond: if else");
            sprintf(inst2.resultado, "L%d", label_end);
            inst2.argumento1[0] = '\0';
            inst2.argumento2[0] = '\0';

            programa[cont_instrucciones] = inst2;
            cont_instrucciones++;

            fprintf(file, "L%d:\n", label_else);

            codigo_intermedio(raiz->der, file);

            fprintf(file, "L%d:\n", label_end);
            ultimo_temp = -1;
            break;
        }
        case T_WHILE: {
            int label_cond = cont_label++;
            int label_fin = cont_label++;

            fprintf(file, "L%d:\n", label_cond);

            codigo_intermedio(raiz->izq, file);
            temp_izq = ultimo_temp;

            fprintf(file, "Salto cond: while T%d L%d\n", temp_izq, label_fin);

            codigo3dir inst;
            strcpy(inst.instruccion, "Salto cond: while");
            sprintf(inst.resultado, "T%d", temp_izq);
            sprintf(inst.argumento1, "L%d", label_fin);
            inst.argumento2[0] = '\0';

            programa[cont_instrucciones] = inst;
            cont_instrucciones++;

            codigo_intermedio(raiz->der, file);

            fprintf(file, "Salto incond: while L%d\n", label_cond);

            codigo3dir inst2;
            strcpy(inst2.instruccion, "Salto incond: while");
            sprintf(inst2.resultado, "L%d", label_cond);
            inst2.argumento1[0] = '\0';
            inst2.argumento2[0] = '\0';

            programa[cont_instrucciones] = inst2;
            cont_instrucciones++;

            fprintf(file, "L%d:\n", label_fin);
            ultimo_temp = -1;
            break;
        }
        default:
            codigo_intermedio(raiz->izq, file);
            codigo_intermedio(raiz->der, file);
            break;
    }
}

// DESPUES ELIMINAR
void imprimir_programa() {
    printf("\n--- TABLA DE INSTRUCCIONES ---\n");
    printf("%-5s  %-25s  %-10s  %-10s  %-10s\n",
           "IDX", "INSTRUCCION", "RESULTADO", "ARG1", "ARG2");
    printf("---------------------------------------------------------------\n");

    for (int i = 0; i < cont_instrucciones; i++) {
        printf("%-5d  %-25s  %-10s  %-10s  %-10s\n",
               i,
               programa[i].instruccion,
               programa[i].resultado[0] ? programa[i].resultado : "-",
               programa[i].argumento1[0] ? programa[i].argumento1 : "-",
               programa[i].argumento2[0] ? programa[i].argumento2 : "-");
    }

    printf("---------------------------------------------------------------\n");
}

