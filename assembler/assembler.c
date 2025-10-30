#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include "../codigo-intermedio/codigo3dir.h"
#include "assembler.h"
#include "estructuras_metodos.h"

char* obtener_ubicacion_operando(info *operando) {
    if (!operando) return NULL;
    
    char *loc = malloc(32);
    
    // CASO 1: constante numerica
    if (operando->tipo_token == T_DIGIT) {
        sprintf(loc, "$%d", operando->nro);
        return loc;
    }
    
    // CASO 2: constante booleana
    if (operando->tipo_token == T_VTRUE || operando->tipo_token == T_VFALSE) {
        int val = (strcmp(operando->bool_string, "true") == 0) ? 1 : 0;
        sprintf(loc, "$%d", val);
        return loc;
    }
    
    // CASO 3: temporal
    if (operando->esTemporal == 1) {
        int offset = obtener_offset_variable(operando->name);
        sprintf(loc, "%d(%%rbp)", offset);
        return loc;
    }
    
    // CASO 4: variable local
    if (!es_variable_global(operando->name)) {
        int offset = obtener_offset_variable(operando->name);
        sprintf(loc, "%d(%%rbp)", offset);
        return loc;
    }
    
    // CASO 5: variable global
    sprintf(loc, "%s(%%rip)", operando->name);
    return loc;
}

void generar_codigo_assembler(codigo3dir *programa, FILE *out) {
    if (!out || !programa) return;

    recolectar_variables_globales(programa);
    generar_seccion_data(out);
    generar_seccion_text(out);

    // saltar las variables globales para empezar con los metodos
    // asi no vemos 2 veces las variables globales, pq ya se ven en la seccion data
    codigo3dir *inst = programa;
    while (inst != NULL && strcmp(inst->instruccion, "LABEL") != 0) {
        inst = inst->siguiente;
    }

    while (inst != NULL) {
        char *instr = inst->instruccion;

        if (strcmp(instr, "LABEL") == 0) {
            if (inst->resultado->tipo_token == T_METHOD_DECL) {
                // es un metodo nuevo
                push_metodo(inst->resultado->name, inst->resultado->tipo_info);
                // print para debug
                printf("LABEL: metodo %s con %d parámetros\n", inst->resultado->name, inst->resultado->num_parametros);
                
                crear_mapeo_variables_locales(inst);
                crear_prologo_metodo(out, inst->resultado->name);
            } else {
                // es una etiqueta de control de flujo (L0, L1, etc.)
                fprintf(out, "%s:\n", inst->resultado->name);
            }
        }

        else if (strcmp(instr, "END") == 0) {
            metodo_info *metodo = get_metodo_actual();

            if (metodo && metodo->tipo_retorno == TIPO_VOID) {
                crear_epilogo_metodo(out);
            }
        
            pop_metodo();
        }

        else if (strcmp(instr, "LOAD_PARAM") == 0) {
            // registros que se usan para los parametros
            const char *param_regs[6] = {"%rdi", "%rsi", "%rdx", "%rcx", "%r8", "%r9"};
            
            int param_idx = inst->arg1->nro;
            char *var_dest = obtener_ubicacion_operando(inst->resultado);
            
            if (param_idx < 6) {
                // copiar desde registro porque son <6
                fprintf(out, "    movq %s, %s\n", param_regs[param_idx], var_dest);
            } else {
                // Copiar desde stack
                // Los parametros en stack estan DESPUES de %rbp
                // Offset: 16(%rbp) para el 7mo param, 24(%rbp) para el 8vo, etc.
                int stack_offset = 16 + (param_idx - 6) * 8;
                fprintf(out, "    movq %d(%%rbp), %%rax\n", stack_offset);
                fprintf(out, "    movq %%rax, %s\n", var_dest);
            }
            
            free(var_dest);
        }
        
        else if (strcmp(instr, "PARAM") == 0) {
            agregar_param(inst->resultado);
        }

        else if (strcmp(instr, "CALL") == 0) {
            const char *param_regs[6] = {"%rdi", "%rsi", "%rdx", "%rcx", "%r8", "%r9"};
            
            // obtener parametros
            int param_count = get_param_count();
            info **params_array = get_params_array();

            // paso 1: alinear stack a 16 bytes
            // ver esto, segun la convencion dice que antes de llamar a una funcion
            // el stack debe estar alineado a 16 bytes
            int stack_params = (param_count > 6) ? (param_count - 6) : 0;
            int stack_bytes = stack_params * 8;
            bool need_alignment = (stack_bytes % 16 != 0);
            
            if (need_alignment) {
                fprintf(out, "    subq $8, %%rsp\n");
            }
            
            // paso 2: parametros 7+ van al stack (en orden inverso)
            for (int i = param_count - 1; i >= 6; i--) {
                char *src = obtener_ubicacion_operando(params_array[i]);
                fprintf(out, "    movq %s, %%rax\n", src);
                fprintf(out, "    pushq %%rax\n");
                free(src);
            }
            
            // paso 3: parametros 1-6 van a registros
            for (int i = 0; i < param_count && i < 6; i++) {
                char *src = obtener_ubicacion_operando(params_array[i]);
                fprintf(out, "    movq %s, %s\n", src, param_regs[i]);
                free(src);
            }
            
            // paso 4: hacer la llamada
            fprintf(out, "    call %s\n", inst->arg1->name);
            
            // paso 5: limpiar el stack
            // ver esto tambie, segun la convencion el llamador limpia el stack
            int bytes_to_clean = stack_bytes;
            if (need_alignment) {
                bytes_to_clean += 8;
            }
            
            if (bytes_to_clean > 0) {
                fprintf(out, "    addq $%d, %%rsp\n", bytes_to_clean);
            }

            // paso 6: mover resultado
            if (inst->resultado) {
                char *dest = obtener_ubicacion_operando(inst->resultado);
                fprintf(out, "    movq %%rax, %s\n", dest);
                free(dest);
            }

            // paso 7: limpiar
            if (params_array != NULL) {
                free(params_array);
            }           
            limpiar_params();
        }
        
        else if (strcmp(instr, "ASSIGN") == 0) {
            if (inst->resultado && inst->arg1) {
                char *src = obtener_ubicacion_operando(inst->arg1);
                char *dest = obtener_ubicacion_operando(inst->resultado);

                // si ambos son memoria, usar registro intermedio porque assembler no deja instruccion de memoria a memoria
                bool src_es_memoria = (src[0] != '$' && src[0] != '%');
                bool dest_es_memoria = (dest[0] != '$' && dest[0] != '%');

                if (src_es_memoria && dest_es_memoria) {
                    fprintf(out, "    movq %s, %%rax\n", src);
                    fprintf(out, "    movq %%rax, %s\n", dest);
                } else {
                    fprintf(out, "    movq %s, %s\n", src, dest);
                }

                free(src);
                free(dest);
            }
        }

        else if (strcmp(instr, "ADD") == 0 || strcmp(instr, "SUB") == 0 || strcmp(instr, "AND") == 0 || strcmp(instr, "OR") == 0) {
            // obtener ubicaciones, son offsets de memoria
            char *src1 = obtener_ubicacion_operando(inst->arg1);
            char *src2 = obtener_ubicacion_operando(inst->arg2);
            char *dest = obtener_ubicacion_operando(inst->resultado);

            // cargar src1 en %rax
            fprintf(out, "    movq %s, %%rax\n", src1);

            // aplicar operación con src2
            if (strcmp(instr, "ADD") == 0) {
                fprintf(out, "    addq %s, %%rax\n", src2);
            } else if (strcmp(instr, "SUB") == 0) {
                fprintf(out, "    subq %s, %%rax\n", src2);
            } else if (strcmp(instr, "AND") == 0) {
                fprintf(out, "    andq %s, %%rax\n", src2);
            } else {
                fprintf(out, "    orq %s, %%rax\n", src2);
            }

            // guardar resultado en memoria (dest)
            fprintf(out, "    movq %%rax, %s\n", dest);
            
            free(src1);
            free(src2);
            free(dest);
        } 
        
        else if (strcmp(instr, "MUL") == 0) {
            char *src1 = obtener_ubicacion_operando(inst->arg1);
            char *src2 = obtener_ubicacion_operando(inst->arg2);
            char *dest = obtener_ubicacion_operando(inst->resultado);
            
            fprintf(out, "    movq %s, %%rax\n", src1);
            fprintf(out, "    imulq %s, %%rax\n", src2);
            fprintf(out, "    movq %%rax, %s\n", dest);
            
            free(src1);
            free(src2);
            free(dest);
        }

        else if (strcmp(instr, "DIV") == 0 || strcmp(instr, "MOD") == 0) {
            char *src1 = obtener_ubicacion_operando(inst->arg1);
            char *src2 = obtener_ubicacion_operando(inst->arg2);
            char *dest = obtener_ubicacion_operando(inst->resultado);
            
            fprintf(out, "    movq %s, %%rax\n", src1);
            fprintf(out, "    cqto\n");
            fprintf(out, "    movq %s, %%r11\n", src2);
            fprintf(out, "    idivq %%r11\n");
            
            if (strcmp(instr, "DIV") == 0) {
                fprintf(out, "    movq %%rax, %s\n", dest);  // cociente
            } else {
                fprintf(out, "    movq %%rdx, %s\n", dest);  // resto
            }
            
            free(src1);
            free(src2);
            free(dest);
        }

        else if (strcmp(instr, "NEG") == 0) {
            char *src = obtener_ubicacion_operando(inst->arg1);
            char *dest = obtener_ubicacion_operando(inst->resultado);
            
            fprintf(out, "    movq %s, %%rax\n", src);
            fprintf(out, "    negq %%rax\n");
            fprintf(out, "    movq %%rax, %s\n", dest);
            
            free(src);
            free(dest);
        }

        else if (strcmp(instr, "NOT") == 0) {
            char *src = obtener_ubicacion_operando(inst->arg1);
            char *dest = obtener_ubicacion_operando(inst->resultado);
            
            fprintf(out, "    movq %s, %%rax\n", src);
            fprintf(out, "    xorq $1, %%rax\n");
            fprintf(out, "    movq %%rax, %s\n", dest);
            
            free(src);
            free(dest);
        }
        
        else if (strcmp(instr, "EQ") == 0 || strcmp(instr, "GT") == 0 || strcmp(instr, "LT") == 0) {
            char *src1 = obtener_ubicacion_operando(inst->arg1);
            char *src2 = obtener_ubicacion_operando(inst->arg2);
            char *dest = obtener_ubicacion_operando(inst->resultado);

            fprintf(out, "    movq %s, %%rax\n", src1);
            fprintf(out, "    cmpq %s, %%rax\n", src2);

            if (strcmp(instr, "EQ") == 0) {
                fprintf(out, "    sete %%al\n");
            } else if (strcmp(instr, "GT") == 0) {
                fprintf(out, "    setg %%al\n");
            } else {
                fprintf(out, "    setl %%al\n");
            }
            
            fprintf(out, "    movzbl %%al, %%eax\n");
            fprintf(out, "    movq %%rax, %s\n", dest);
            
            free(src1);
            free(src2);
            free(dest);
        }

        else if (strcmp(instr, "EXTERN") == 0) {
            // no hacer nada
        }

        else if (strcmp(instr, "RET") == 0) {
            metodo_info *metodo = get_metodo_actual();

            // si el campo resultado no es NULL, entonces es porque el metodo retorna un valor
            if (inst->resultado) {
                char *src = obtener_ubicacion_operando(inst->resultado);
                fprintf(out, "    movq %s, %%rax\n", src);
                free(src);
            }
            // solo generar epilogo si NO es un metodo void
            // los metodos void generan su epilogo en END
            if (metodo && metodo->tipo_retorno != TIPO_VOID) {
                crear_epilogo_metodo(out);
            }
        }

        else if (strcmp(instr, "GOTO") == 0) {
            fprintf(out, "    jmp %s\n", inst->resultado->name);
        }
        
        else if (strcmp(instr, "IF_FALSE") == 0) {
            char *cond_src = obtener_ubicacion_operando(inst->arg1);
            
            fprintf(out, "    movq %s, %%rax\n", cond_src);
            fprintf(out, "    cmpq $0, %%rax\n");
            fprintf(out, "    je %s\n", inst->resultado->name);
            
            free(cond_src);
        }
        
        else {
            // instruccion no manejada: la imprimimos como comentario para debug
            fprintf(out, "    # instruccion no manejada: %s\n", instr);
        }

        inst = inst->siguiente; // avanzar en la lista
    }

    generar_epilogo_archivo(out);
}
