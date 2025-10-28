#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include "../codigo-intermedio/codigo3dir.h"
#include "assembler.h"

// nodo de la lista enlazada para parametros
typedef struct param_node {
    info *param;
    struct param_node *siguiente;
} param_node;

// estructura para acumular parametros de un CALL
typedef struct param_call {
    param_node *primero;     // primer parametro
    param_node *ultimo;      // ultimo parametro (para insercion O(1))
    int count;               // cantidad de parametros
} param_call;

// estructura para informacion de metodos (para preservar contexto si hay anidamiento)
typedef struct metodo_info {
    char *nombre;                    // nombre del metodo
    int num_vars_locales;            // cantidad de variables locales
    var_offset *mapeo_vars;          // mapeo de variables locales
    struct metodo_info *siguiente;   // para manejar llamadas anidadas
    tipo_info tipo_retorno;          // tipo de retorno del metodo
} metodo_info;

static var_global *variables_globales = NULL;
static param_call call_actual = {.primero = NULL, .ultimo = NULL, .count = 0};  // CALL actual
static metodo_info *metodo_stack = NULL;       // stack de metodos (para anidamiento)

bool es_variable_global(const char *nombre) {
    var_global *actual = variables_globales;
    while (actual != NULL) {
        if (strcmp(actual->nombre, nombre) == 0) {
            return true;
        }
        actual = actual->siguiente;
    }
    return false;
}

// agregar parametro al final de la lista
void agregar_param(info *param) {
    param_node *nuevo = malloc(sizeof(param_node));
    nuevo->param = param;
    nuevo->siguiente = NULL;
    
    if (call_actual.primero == NULL) {
        // lista vacia
        call_actual.primero = nuevo;
        call_actual.ultimo = nuevo;
    } else {
        // agregar al final
        call_actual.ultimo->siguiente = nuevo;
        call_actual.ultimo = nuevo;
    }
    
    call_actual.count++;
}

// limpiar lista de parámetros
void limpiar_params() {
    param_node *actual = call_actual.primero;
    while (actual != NULL) {
        param_node *temp = actual;
        actual = actual->siguiente;
        free(temp);
    }
    call_actual.primero = NULL;
    call_actual.ultimo = NULL;
    call_actual.count = 0;
}

void agregar_variable_global(const char *nombre, const char *valor) {
    var_global *var_g = (var_global *)malloc(sizeof(var_global));
    var_g->nombre = strdup(nombre);
    var_g->valor_inicial = strdup(valor);
    var_g->siguiente = NULL;
    
    if (variables_globales == NULL) {
        variables_globales = var_g;
    } else {
        var_global *actual = variables_globales;
        while (actual->siguiente != NULL) {
            actual = actual->siguiente;
        }
        actual->siguiente = var_g;
    }
}

void recolectar_variables_globales(codigo3dir *programa) {
    codigo3dir *inst = programa;

    while (inst != NULL) {
        if (strcmp(inst->instruccion, "LABEL") == 0) {
            break;
        }

        if (strcmp(inst->instruccion, "ASSIGN") == 0) {
            if (inst->resultado && inst->resultado->name && inst->arg1) {
                char valor_str[32];
                
                if (inst->arg1->tipo_token == T_DIGIT) {
                    sprintf(valor_str, "%d", inst->arg1->nro);
                }
                else if (inst->arg1->tipo_token == T_VTRUE || inst->arg1->tipo_token == T_VFALSE) {
                    strcpy(valor_str, inst->arg1->bool_string);
                }
                else if (inst->arg1->tipo_token == T_ID) {
                    strcpy(valor_str, inst->arg1->name);
                }
                else if (inst->arg1->esTemporal == 1) {
                    strcpy(valor_str, "0");
                }
                else {
                    strcpy(valor_str, "0");
                }
                
                agregar_variable_global(inst->resultado->name, valor_str);
            }
        }

        inst = inst->siguiente;
    }
}

void push_metodo(const char *nombre, tipo_info tipo_ret) {
    metodo_info *nuevo = malloc(sizeof(metodo_info));
    nuevo->nombre = strdup(nombre);
    nuevo->num_vars_locales = 0;
    nuevo->mapeo_vars = NULL;
    nuevo->siguiente = metodo_stack;
    metodo_stack = nuevo;
    nuevo->tipo_retorno = tipo_ret;
}

void pop_metodo() {
    if (metodo_stack == NULL) return;
    
    metodo_info *temp = metodo_stack;
    metodo_stack = metodo_stack->siguiente;
    
    // Liberar mapeo de variables
    while (temp->mapeo_vars != NULL) {
        var_offset *v = temp->mapeo_vars;
        temp->mapeo_vars = temp->mapeo_vars->siguiente;
        free(v->nombre);
        free(v);
    }
    
    free(temp->nombre);
    free(temp);
}

metodo_info* get_metodo_actual() {
    return metodo_stack;
}

void crear_mapeo_variables_locales(codigo3dir *label) {
    metodo_info *metodo = get_metodo_actual();
    if (!metodo) return;
    
    // limpiar mapeo anterior del metodo actual
    while (metodo->mapeo_vars != NULL) {
        var_offset *temp = metodo->mapeo_vars;
        metodo->mapeo_vars = metodo->mapeo_vars->siguiente;
        free(temp->nombre);
        free(temp);
    }
    
    codigo3dir *p = label->siguiente;
    const char *variables[100];
    int var_count = 0;
    
    // recolectar variables unicas (solo las que NO son globales ni temporales)
    while (p != NULL) {
        if (strcmp(p->instruccion, "END") == 0) break;
        
        // ASSIGN y LOAD_PARAM crean variables locales
        // osea que se trata de la misma manera a las variables que a los parametros
        if (strcmp(p->instruccion, "ASSIGN") == 0 || strcmp(p->instruccion, "LOAD_PARAM") == 0) {
            if (p->resultado->esTemporal == 0 && !es_variable_global(p->resultado->name)) {
                bool found = false;
                for (int i = 0; i < var_count; i++) {
                    if (strcmp(variables[i], p->resultado->name) == 0) {
                        found = true;
                        break;
                    }
                }
                if (!found) {
                    variables[var_count] = p->resultado->name;
                    var_count++;
                }
            }
        }
        p = p->siguiente;
    }
    
    // crear mapeo con offsets
    for (int i = 0; i < var_count; i++) {
        var_offset *nuevo = malloc(sizeof(var_offset));
        nuevo->nombre = strdup(variables[i]);
        nuevo->offset = -8 * (i + 1);
        nuevo->siguiente = metodo->mapeo_vars;
        metodo->mapeo_vars = nuevo;
    }
    
    metodo->num_vars_locales = var_count;
}

int obtener_offset_variable(const char *nombre) {
    metodo_info *metodo = get_metodo_actual();
    if (!metodo) return -8;
    
    var_offset *actual = metodo->mapeo_vars;
    while (actual != NULL) {
        if (strcmp(actual->nombre, nombre) == 0) {
            return actual->offset;
        }
        actual = actual->siguiente;
    }
    return -8;
}

int extraer_numero_temporal(char *nombre) {
    if (nombre[0] == 'T') {
        return atoi(nombre + 1);
    }
    return -1;
}

const char *obtener_registro_temporal(int n) {
    const char *regs[] = {"%r10", "%r11", "%r12", "%r13", "%r14", "%r15"};
    return regs[n % 6];
}

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
        int num_temp = extraer_numero_temporal(operando->name);
        const char *reg_temp = obtener_registro_temporal(num_temp);
        sprintf(loc, "%s", reg_temp);
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

void generar_seccion_data(FILE *out) {
    if (variables_globales == NULL || out == NULL) return;

    fprintf(out, "\t.data\n");
    var_global *actual = variables_globales;

    while (actual != NULL) {
        fprintf(out, "%s:\n", actual->nombre);
        
        if (strcmp(actual->valor_inicial, "true") == 0) {
            fprintf(out, "    .quad 1\n");
        } else if (strcmp(actual->valor_inicial, "false") == 0) {
            fprintf(out, "    .quad 0\n");
        } else {
            fprintf(out, "    .quad %s\n", actual->valor_inicial);
        }
        
        actual = actual->siguiente;
    }
    fprintf(out, "\n");
}

void generar_seccion_text(FILE *out) {
    if (out == NULL) return;

    fprintf(out, "\t.text\n");
    fprintf(out, "\t.globl\tmain\n");
    fprintf(out, "\t.type\tmain, @function\n");
    fprintf(out, "\n");
}

void generar_epilogo_archivo(FILE *out) {
    if (out == NULL) return;

    fprintf(out, "\n.LFE0:\n");
    fprintf(out, "\t.size\tmain, .-main\n");
    fprintf(out, "\t.section\t.note.GNU-stack,\"\",@progbits\n");
}

void crear_prologo_metodo(FILE *out, const char *nombre_metodo) {
    if (!out || !nombre_metodo) return;
    
    metodo_info *metodo = get_metodo_actual();
    if (!metodo) return;

    fprintf(out, "%s:\n", nombre_metodo);
    fprintf(out, "    pushq %%rbp\n");
    fprintf(out, "    movq %%rsp, %%rbp\n");

    // calcular espacio total necesario:
    // - 4 registros callee-saved (r12-r15) = 32 bytes
    // - variables locales = num_vars_locales * 8 bytes
    int espacio_registros = 32;  // 4 registros * 8 bytes
    int espacio_variables = 8 * metodo->num_vars_locales;
    int espacio_total = espacio_registros + espacio_variables;

    int ajuste = (8 + espacio_total) % 16;
    if (ajuste != 0) {
        espacio_total += (16 - ajuste);
    }
    
    if (espacio_total > 0) {
        fprintf(out, "    subq $%d, %%rsp\n", espacio_total);
    }
    
    // guardar registros callee-saved en las primeras posiciones
    fprintf(out, "    movq %%r12, 0(%%rsp)\n");
    fprintf(out, "    movq %%r13, 8(%%rsp)\n");
    fprintf(out, "    movq %%r14, 16(%%rsp)\n");
    fprintf(out, "    movq %%r15, 24(%%rsp)\n");
}

void crear_epilogo_metodo(FILE *out) {
    if (!out) return;
    
    metodo_info *metodo = get_metodo_actual();
    if (!metodo) return;

    // restaurar registros callee-saved
    fprintf(out, "    movq 0(%%rsp), %%r12\n");
    fprintf(out, "    movq 8(%%rsp), %%r13\n");
    fprintf(out, "    movq 16(%%rsp), %%r14\n");
    fprintf(out, "    movq 24(%%rsp), %%r15\n");
    
    // restaurar stack pointer
    fprintf(out, "    movq %%rbp, %%rsp\n");
    fprintf(out, "    popq %%rbp\n");
    fprintf(out, "    ret\n");
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
            
            // convertir lista a array temporal para facilitar acceso por indice
            info **params_array = NULL;
            if (call_actual.count > 0) {
                params_array = malloc(call_actual.count * sizeof(info*));
                param_node *actual = call_actual.primero;
                int idx = 0;
                while (actual != NULL) {
                    params_array[idx++] = actual->param;
                    actual = actual->siguiente;
                }
            }
            
            // paso 1: parametros 7+ van al stack (en orden inverso)
            for (int i = call_actual.count - 1; i >= 6; i--) {
                char *src = obtener_ubicacion_operando(params_array[i]);
                
                if (src[0] == '%' && src[1] != 'r' && strncmp(src, "%rbp", 4) != 0) {
                    fprintf(out, "    pushq %s\n", src);
                } else {
                    fprintf(out, "    movq %s, %%rax\n", src);
                    fprintf(out, "    pushq %%rax\n");
                }
                
                free(src);
            }
            
            // paso 2: parametros 1-6 van a registros
            for (int i = 0; i < call_actual.count && i < 6; i++) {
                char *src = obtener_ubicacion_operando(params_array[i]);
                fprintf(out, "    movq %s, %s\n", src, param_regs[i]);
                free(src);
            }
            
            // paso 3: alinear stack a 16 bytes
            // ver esto, segun la convencion dice que antes de llamar a una funcion
            // el stack debe estar alineado a 16 bytes
            int stack_params = (call_actual.count > 6) ? (call_actual.count - 6) : 0;
            int stack_bytes = stack_params * 8;
            bool need_alignment = (stack_bytes % 16 != 0);
            
            if (need_alignment) {
                fprintf(out, "    subq $8, %%rsp\n");
            }
            
            // paso 4: hacer la llamada
            char *nombre_funcion = inst->arg1->name;
            fprintf(out, "    call %s\n", nombre_funcion);
            
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
            if (inst->resultado && inst->resultado->esTemporal == 1) {
                const char *reg_dest = obtener_registro_temporal(
                    extraer_numero_temporal(inst->resultado->name)
                );
                
                if (strcmp(reg_dest, "%rax") != 0) {
                    fprintf(out, "    movq %%rax, %s\n", reg_dest);
                }
            }

            // paso 7: limpiar
            free(params_array);
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

        else if (strcmp(instr, "ADD") == 0 || strcmp(instr, "SUB") == 0 || 
                strcmp(instr, "AND") == 0 || strcmp(instr, "OR") == 0) {

            const char *reg_dest = obtener_registro_temporal(extraer_numero_temporal(inst->resultado->name));
            
            char *src1 = obtener_ubicacion_operando(inst->arg1);
            char *src2 = obtener_ubicacion_operando(inst->arg2);

            // cargar src1 en reg_dest
            fprintf(out, "    movq %s, %s\n", src1, reg_dest);
            
            // aplicar operación con src2
            if (strcmp(instr, "ADD") == 0) {
                fprintf(out, "    addq %s, %s\n", src2, reg_dest);
            } else if (strcmp(instr, "SUB") == 0) {
                fprintf(out, "    subq %s, %s\n", src2, reg_dest);
            } else if (strcmp(instr, "AND") == 0) {
                fprintf(out, "    andq %s, %s\n", src2, reg_dest);
            } else {
                fprintf(out, "    orq %s, %s\n", src2, reg_dest);
            }
            
            free(src1);
            free(src2);
        } 
        
        else if (strcmp(instr, "MUL") == 0) {
            // se debe mover el resultado de %rax a reg_dest.
            const char* reg_dest = obtener_registro_temporal(extraer_numero_temporal(inst->resultado->name));
            
            char *src1 = obtener_ubicacion_operando(inst->arg1);
            char *src2 = obtener_ubicacion_operando(inst->arg2);
            
            fprintf(out, "    movq %s, %%rax\n", src1);
            fprintf(out, "    imulq %s, %%rax\n", src2);
            
            // guarda el resultado en r10 o r11
            if (strcmp(reg_dest, "%rax") != 0) {
                fprintf(out, "    movq %%rax, %s\n", reg_dest);
            }
            
            free(src1);
            free(src2);
        }

        else if (strcmp(instr, "DIV") == 0 || strcmp(instr, "MOD") == 0) {
            // si o si se tienen que usar %rax y %rdx
            const char* reg_dest = obtener_registro_temporal(extraer_numero_temporal(inst->resultado->name));
            
            char *src1 = obtener_ubicacion_operando(inst->arg1);
            char *src2 = obtener_ubicacion_operando(inst->arg2);
            
            fprintf(out, "    movq %s, %%rax\n", src1);
            fprintf(out, "    cqto\n");
            fprintf(out, "    movq %s, %%r11\n", src2);
            fprintf(out, "    idivq %%r11\n");
            
            // guardar el resultado en reg_dest, puede ser r10 o r11 y para div o mod 
            if (strcmp(instr, "DIV") == 0) {
                // cociente en %rax
                fprintf(out, "    movq %%rax, %s\n", reg_dest);
            } else {
                // resto en %rdx
                fprintf(out, "    movq %%rdx, %s\n", reg_dest);
            }
            
            free(src1);
            free(src2);
        }

        else if (strcmp(instr, "NEG") == 0) {
            const char *reg_dest = obtener_registro_temporal(extraer_numero_temporal(inst->resultado->name));
            
            char *src1 = obtener_ubicacion_operando(inst->arg1);
            fprintf(out, "    movq %s, %s\n", src1, reg_dest);
            fprintf(out, "    negq %s\n", reg_dest);
            
            free(src1);
        }

        else if (strcmp(instr, "NOT") == 0) {
            const char *reg_dest = obtener_registro_temporal(extraer_numero_temporal(inst->resultado->name));
            
            char *src1 = obtener_ubicacion_operando(inst->arg1);
            fprintf(out, "    movq %s, %s\n", src1, reg_dest);
            fprintf(out, "    xorq $1, %s\n", reg_dest);
            
            free(src1);
        }
        
        else if (strcmp(instr, "EQ") == 0 || strcmp(instr, "GT") == 0 || strcmp(instr, "LT") == 0) {
            const char *reg_dest = obtener_registro_temporal(extraer_numero_temporal(inst->resultado->name));
            
            char *src1 = obtener_ubicacion_operando(inst->arg1);
            char *src2 = obtener_ubicacion_operando(inst->arg2);

            fprintf(out, "    movq %s, %%rax\n", src1);
            fprintf(out, "    cmpq %s, %%rax\n", src2);

            if (strcmp(instr, "EQ") == 0) {
                fprintf(out, "    sete %%al\n"); // setea si es igual
            } else if (strcmp(instr, "GT") == 0) {
                fprintf(out, "    setg %%al\n"); // setea si es mayor
            } else {
                fprintf(out, "    setl %%al\n"); // setea si es menor
            }
            
            fprintf(out, "    movzbl %%al, %%eax\n");
            fprintf(out, "    movq %%rax, %s\n", reg_dest);
            
            free(src1);
            free(src2);
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
            char *label_name = inst->resultado->name;
            
            fprintf(out, "    movq %s, %%rax\n", cond_src);
            fprintf(out, "    cmpq $0, %%rax\n");
            fprintf(out, "    je %s\n", label_name);
            
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
