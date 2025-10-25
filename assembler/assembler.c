#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include "../codigo-intermedio/codigo3dir.h"
#include "assembler.h"

static var_global *variables_globales = NULL;
static var_offset *mapeo_variables = NULL;
static int num_vars_locales = 0;
static char *nombre_metodo_actual = NULL;

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

int extraer_numero_temporal(char *nombre) {
    // si es t0 queda 0, si es tn queda n
    if (nombre[0] == 'T') {
        return atoi(nombre + 1);
    }
    return -1;
}

// Crear mapeo de variables locales para un método
void crear_mapeo_variables_locales(codigo3dir *label) {
    // Limpiar mapeo anterior
    while (mapeo_variables != NULL) {
        var_offset *temp = mapeo_variables;
        mapeo_variables = mapeo_variables->siguiente;
        free(temp->nombre);
        free(temp);
    }
    
    codigo3dir *p = label->siguiente;
    const char *variables[100];
    int var_count = 0;
    
    // Primera pasada: recolectar variables únicas
    while (p != NULL) {
        if (strcmp(p->instruccion, "END") == 0) break;
        
        if (strcmp(p->instruccion, "ASSIGN") == 0) {
            if (p->resultado->esTemporal == 0 && !es_variable_global(p->resultado->name)) {
                // Es variable local
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
    
    // Segunda pasada: crear mapeo con offsets
    for (int i = 0; i < var_count; i++) {
        var_offset *nuevo = malloc(sizeof(var_offset));
        nuevo->nombre = strdup(variables[i]);
        nuevo->offset = -8 * (i + 1);  // -8, -16, -24, etc.
        nuevo->siguiente = mapeo_variables;
        mapeo_variables = nuevo;
    }
    
    // actualizar el contador global
    num_vars_locales = var_count;
}

// Obtener offset de una variable específica
int obtener_offset_variable(const char *nombre) {
    var_offset *actual = mapeo_variables;
    while (actual != NULL) {
        if (strcmp(actual->nombre, nombre) == 0) {
            return actual->offset;
        }
        actual = actual->siguiente;
    }
    return -8; // fallback
}

const char *obtener_registro_temporal(int n) {
    // temporales mapeados a r10 y r11, se usa uno luego otro y luego el primero de nuevo
    return (n % 2 == 0) ? "%r10" : "%r11";
}

void agregar_variable_global(const char *nombre, const char *valor) {
    var_global *var_g = (var_global *)malloc(sizeof(var_global));
    var_g->nombre = strdup(nombre);
    var_g->valor_inicial = strdup(valor);  // guardar el valor inicial
    var_g->siguiente = NULL;
    
    // insertar al final asi se preserva el orden en como se definen
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
        // si encontramos un LABEL se termina pq no hay declaraciones de variables dsp de declaraciones de metodos
        if (strcmp(inst->instruccion, "LABEL") == 0) {
            break;
        }

        if (strcmp(inst->instruccion, "ASSIGN") == 0) {
            if (inst->resultado && inst->resultado->name && inst->arg1) {
                char valor_str[32];
                
                // Obtener el valor según el tipo del arg1
                if (inst->arg1->tipo_token == T_DIGIT) {
                    // Es un número
                    sprintf(valor_str, "%d", inst->arg1->nro);
                }
                else if (inst->arg1->tipo_token == T_VTRUE || inst->arg1->tipo_token == T_VFALSE) {
                    // Es un booleano
                    strcpy(valor_str, inst->arg1->bool_string);
                }
                else if (inst->arg1->tipo_token == T_ID) {
                    // Es otra variable (caso: GLOBAL_A = GLOBAL_B)
                    strcpy(valor_str, inst->arg1->name);
                }
                else if (inst->arg1->esTemporal == 1) {
                    // Es un temporal (expresiones complejas)
                    // Por ahora, valor por defecto
                    strcpy(valor_str, "0");
                }
                else {
                    // Valor por defecto
                    strcpy(valor_str, "0");
                }
                
                agregar_variable_global(inst->resultado->name, valor_str);
            }
        }

        inst = inst->siguiente;
    }
}

void generar_seccion_data(FILE *out) {
    if (variables_globales == NULL || out == NULL) return;

    fprintf(out, "\t.data\n");
    var_global *actual = variables_globales;

    while (actual != NULL) {
        fprintf(out, "%s:\n", actual->nombre);
        
        // convertir valores booleanos a enteros
        if (strcmp(actual->valor_inicial, "true") == 0) {
            fprintf(out, "    .quad 1\n");
        } else if (strcmp(actual->valor_inicial, "false") == 0) {
            fprintf(out, "    .quad 0\n");
        } else {
            // sino se usa tal cual
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

    fprintf(out, "%s:\n", nombre_metodo);
    fprintf(out, "    pushq %%rbp\n");
    fprintf(out, "    movq %%rsp, %%rbp\n");
    
    // reservar espacio para variables locales
    int n = 8 * num_vars_locales;
    if (n > 0) {
        fprintf(out, "    subq $%d, %%rsp\n", n);
    }
}

void crear_epilogo_metodo(FILE *out) {
    if (!out) return;

    fprintf(out, "    movq %%rbp, %%rsp\n");
    fprintf(out, "    popq %%rbp\n");
    fprintf(out, "    ret\n");

    // resetear contadores para el siguiente metodo
    num_vars_locales = 0;
}

char* obtener_ubicacion_operando(info *operando) {
    if (!operando) return NULL;
    
    char *loc = malloc(32);
    
    // CASO 1: Es una constante numérica
    if (operando->tipo_token == T_DIGIT) {
        sprintf(loc, "$%d", operando->nro);
        return loc;
    }
    
    // CASO 2: Es una constante booleana
    if (operando->tipo_token == T_VTRUE || operando->tipo_token == T_VFALSE) {
        int val = (strcmp(operando->bool_string, "true") == 0) ? 1 : 0;
        sprintf(loc, "$%d", val);
        return loc;
    }
    
    // CASO 3: Es un temporal
    if (operando->esTemporal == 1) {
        int num_temp = extraer_numero_temporal(operando->name);
        const char *reg_temp = obtener_registro_temporal(num_temp);
        sprintf(loc, "%s", reg_temp);
        return loc;
    }
    
    // CASO 4: Es variable local
    if (!es_variable_global(operando->name)) {
        int offset = obtener_offset_variable(operando->name);
        sprintf(loc, "%d(%%rbp)", offset);
        return loc;
    }
    
    // CASO 5: Es variable global
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

            if (nombre_metodo_actual != NULL) {
                free(nombre_metodo_actual);
            }

            nombre_metodo_actual = strdup(inst->resultado->name);

            // esto se hace solo si es un metodo, puede ser un label con una etiqueta solamente
            if (inst->resultado->tipo_token == T_METHOD_DECL) {
                printf("LABEL=numero de parametros del metodo %s: %d\n", inst->resultado->name, inst->resultado->num_parametros);
                crear_mapeo_variables_locales(inst);
                crear_prologo_metodo(out, inst->resultado->name);
            } else { //sino es metodo entonces es label tipo L0
                fprintf(out, "%s:\n", inst->resultado->name);
            }
        }
        
        else if (strcmp(instr, "END") == 0) {
            crear_epilogo_metodo(out);
        }

        else if (strcmp(instr, "ASSIGN") == 0) {
            if (inst->resultado && inst->arg1) {
                char *src = obtener_ubicacion_operando(inst->arg1);
                
                // obtener ubicacion de la variable destino (resultado)
                char *dest = obtener_ubicacion_operando(inst->resultado);

                // si ambos son memoria, usar registro intermedio porque assembler no deja instruccion de memoria a memoria
                bool src_es_memoria = (src[0] != '$' && src[0] != '%');
                bool dest_es_memoria = (dest[0] != '$' && dest[0] != '%');

                if (src_es_memoria && dest_es_memoria) {
                    // Usar %rax como registro intermedio
                    fprintf(out, "    movq %s, %%rax\n", src);
                    fprintf(out, "    movq %%rax, %s\n", dest);
                } else {
                    // Movimiento directo está bien
                    fprintf(out, "    movq %s, %s\n", src, dest);
                }

                free(src);
                free(dest);
            }
        }

        else if (strcmp(instr, "ADD") == 0 || strcmp(instr, "SUB") == 0 || 
                strcmp(instr, "AND") == 0 || strcmp(instr, "OR") == 0) {

            const char *reg_dest = obtener_registro_temporal(extraer_numero_temporal(inst->resultado->name));
            
            // CAMBIAR: usar obtener_ubicacion_operando
            char *src1 = obtener_ubicacion_operando(inst->arg1);
            char *src2 = obtener_ubicacion_operando(inst->arg2);

            // Cargar src1 en reg_dest
            fprintf(out, "    movq %s, %s\n", src1, reg_dest);
            
            // Aplicar operación con src2
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
            // Se debe mover el resultado de %rax a reg_dest.
            const char* reg_dest = obtener_registro_temporal(extraer_numero_temporal(inst->resultado->name));
            
            // CAMBIAR: usar obtener_ubicacion_operando
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
            
            // CAMBIAR: usar obtener_ubicacion_operando
            char *src1 = obtener_ubicacion_operando(inst->arg1);
            char *src2 = obtener_ubicacion_operando(inst->arg2);
            
            fprintf(out, "    movq %s, %%rax\n", src1);
            fprintf(out, "    cqto\n");
            fprintf(out, "    movq %s, %%r11\n", src2);
            fprintf(out, "    idivq %%r11\n");
            
            // guardar el resultado en reg_dest, puede ser r10 o r11 y para div o mod 
            if (strcmp(instr, "DIV") == 0) {
                // Cociente en %rax
                fprintf(out, "    movq %%rax, %s\n", reg_dest);
            } else {
                // Resto en %rdx
                fprintf(out, "    movq %%rdx, %s\n", reg_dest);
            }
            
            free(src1);
            free(src2);
        }

        else if (strcmp(instr, "NEG") == 0) {
            const char *reg_dest = obtener_registro_temporal(extraer_numero_temporal(inst->resultado->name));
            
            // CAMBIAR: usar obtener_ubicacion_operando
            char *src1 = obtener_ubicacion_operando(inst->arg1);
            fprintf(out, "    movq %s, %s\n", src1, reg_dest);
            fprintf(out, "    negq %s\n", reg_dest);
            
            free(src1);
        }

        else if (strcmp(instr, "NOT") == 0) {
            const char *reg_dest = obtener_registro_temporal(extraer_numero_temporal(inst->resultado->name));
            
            // CAMBIAR: usar obtener_ubicacion_operando
            char *src1 = obtener_ubicacion_operando(inst->arg1);
            fprintf(out, "    movq %s, %s\n", src1, reg_dest);
            fprintf(out, "    xorq $1, %s\n", reg_dest);
            
            free(src1);
        }
        
        else if (strcmp(instr, "EQ") == 0 || strcmp(instr, "GT") == 0 || strcmp(instr, "LT") == 0) {
            const char *reg_dest = obtener_registro_temporal(extraer_numero_temporal(inst->resultado->name));
            
            // CAMBIAR: usar obtener_ubicacion_operando
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

        
        else if (strcmp(instr, "RET") == 0) {
            // si hay retorno con valor, ese valor va a rax(registro para retorno)
            if (inst->arg1) {
                    
                    char *src = obtener_ubicacion_operando(inst->arg1);
                    
                    fprintf(out, "    movq %s, %%rax\n", src);
                }

            // tanto si es return con valor o no se va derecho al epilogo, con el guardado del nombre del metodo actual/corriente
            fprintf(out, "    jmp END_%s\n", nombre_metodo_actual);
        }

        else if (strcmp(instr, "GOTO") == 0) {
            // salta si o si
            fprintf(out, "    jmp %s\n", inst->resultado->name);
        }
        
        else if (strcmp(instr, "IF_FALSE") == 0) {
            
            // 1. OBTENER LA UBICACIÓN DE LA CONDICIÓN
            // Esto devuelve: OFFSET de 'probando' (Memoria) O %r10/r11 (Temporal)
            char *cond_src = obtener_ubicacion_operando(inst->arg1);
            char *label_name = inst->resultado->name; // L0, L2, L4, L
            // Paso 2: Mover el valor booleano (0 o 1) a RAX para CMP

            // si cond_src es -algo(%rbp), esta instrucción mueve memoria-registro.
            // si cond_src es algun registro, esta instrucción mueve de registro-registro (redundante pero funcional, ver).
            fprintf(out, "    movq %s, %%rax\n", cond_src);
            // Paso 3: Comparar el valor con 0 (FALSO)
            // El C3D está diseñado para saltar si la condición es FALSO (valor = 0).
            fprintf(out, "    cmpq $0, %%rax\n");
            // Paso 4: Saltar si es IGUAL (JE) a 0 (FALSO)
            fprintf(out, "    je %s\n", label_name);
        }
        
        else {
            // instrucción no manejada: la imprimimos como comentario para debug
            fprintf(out, "    # instruccion no traducida: %s\n", instr);
        }

        inst = inst->siguiente; // avanzar en la lista
    }

    generar_epilogo_archivo(out);
}
