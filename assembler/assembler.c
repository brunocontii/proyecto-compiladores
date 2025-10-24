#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include "../codigo-intermedio/codigo3dir.h"
#include "assembler.h"

static var_global *variables_globales = NULL;
static var_offset *mapeo_variables = NULL;
static int num_vars_locales = 0;
static int num_params_actual = 0;

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

// Función corregida para obtener ubicación
char* obtener_ubicacion_vars_locales(info *operando) {
    char *loc = malloc(32);
    
    // Si es temporal, usar stack temporal
    if (operando->esTemporal == 1) {
        int num_temp = extraer_numero_temporal(operando->name);
        const char *reg_temp = obtener_registro_temporal(num_temp);
        sprintf(loc, "%s", reg_temp);
    }
    // Si es variable local
    else if (!es_variable_global(operando->name)) {
        int offset = obtener_offset_variable(operando->name);
        sprintf(loc, "%d(%%rbp)", offset);
    }
    // Si es variable global
    else {
        sprintf(loc, "%s(%%rip)", operando->name);
    }
    
    return loc;
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
    codigo3dir *load_anterior = NULL;

    while (inst != NULL) {
        // si encontramos un LABEL se termina pq no hay declaraciones de variables dsp de declaraciones de metodos
        if (strcmp(inst->instruccion, "LABEL") == 0) {
            break;
        }

        // guardar referencia al LOAD anterior para el siguiente ASSIGN
        if (strcmp(inst->instruccion, "LOAD") == 0) {
            load_anterior = inst;
        }

        if (strcmp(inst->instruccion, "ASSIGN") == 0) {
            if (inst->resultado && inst->resultado->name) {
                // buscar el valor del LOAD correspondiente
                if (load_anterior && 
                    load_anterior->resultado && 
                    inst->arg1 && 
                    strcmp(load_anterior->resultado->name, inst->arg1->name) == 0) {
                    // el valor viene del LOAD anterior
                    agregar_variable_global(inst->resultado->name, load_anterior->arg1->name);
                } else {
                    // valor por defecto si no encontramos el LOAD
                    agregar_variable_global(inst->resultado->name, "0");
                }
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
    num_params_actual = 0;
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
            printf("LABEL=numero de parametros del metodo %s: %d\n", inst->resultado->name, inst->resultado->num_parametros);

            crear_mapeo_variables_locales(inst);
            crear_prologo_metodo(out, inst->resultado->name);
        }
        
        else if (strcmp(instr, "END") == 0) {
            crear_epilogo_metodo(out);
        }
        
        else if (strcmp(instr, "LOAD") == 0) {
            // resultado = temporal destino (T0, T1, etc)
            // arg1 = lo que cargo (número o variable)
            char *dest = obtener_ubicacion_vars_locales(inst->resultado);
            
            // CASO 1: Cargar número
            if (inst->arg1->tipo_token == T_DIGIT) {
                fprintf(out, "    movq $%d, %s\n", inst->arg1->nro, dest);
            }
            // CASO 2: Cargar booleano
            else if (inst->arg1->tipo_token == T_VTRUE || inst->arg1->tipo_token == T_VFALSE) {
                int val = (strcmp(inst->arg1->bool_string, "true") == 0) ? 1 : 0;
                fprintf(out, "    movq $%d, %s\n", val, dest);
            }
            // CASO 3: Cargar variable (p, r, qqw, etc)
            else if (inst->arg1->tipo_token == T_ID) {
                char *src = obtener_ubicacion_vars_locales(inst->arg1);
                
                // para no generar inst r r
                if (strcmp(src, dest) != 0) {
                    fprintf(out, "    movq %s, %s\n", src, dest);
                }
            }
        }

        else if (strcmp(instr, "ASSIGN") == 0) {
            if (inst->resultado && inst->arg1) {
                // obtener ubicacion del temporal origen (arg1)
                char *src = obtener_ubicacion_vars_locales(inst->arg1);
                
                // obtener ubicacion de la variable destino (resultado)
                char *dest;
                if (es_variable_global(inst->resultado->name)) {
                    dest = malloc(32);
                    sprintf(dest, "%s(%%rip)", inst->resultado->name);
                } else {
                    dest = obtener_ubicacion_vars_locales(inst->resultado);
                }
                
                fprintf(out, "    movq %s, %s\n", src, dest);
            }
        }

        else if ((strcmp(instr, "ADD") == 0 || strcmp(instr, "SUB") == 0 || 
                strcmp(instr, "AND") == 0 || strcmp(instr, "OR") == 0)) {

            const char *reg_dest = obtener_registro_temporal(extraer_numero_temporal(inst->resultado->name));
            char *reg_aux = (strcmp(reg_dest, "%r10") == 0) ? "%r11" : "%r10";
            char *src1 = obtener_ubicacion_vars_locales(inst->arg1);
            char *src2 = obtener_ubicacion_vars_locales(inst->arg2);

            // cargar src2 en reg_aux primero
            if (strcmp(src2, reg_aux) != 0) {
                fprintf(out, "    movq %s, %s\n", src2, reg_aux);
            }

            // cargar src1 en reg_dest segundo
            if (strcmp(src1, reg_dest) != 0) {
                fprintf(out, "    movq %s, %s\n", src1, reg_dest);
            }

            if (strcmp(instr, "ADD") == 0) {
                fprintf(out, "    addq %s, %s\n", reg_aux, reg_dest);
            } else if (strcmp(instr, "SUB") == 0) {
                fprintf(out, "    subq %s, %s\n", reg_aux, reg_dest);
            } else if (strcmp(instr, "AND") == 0) {
                fprintf(out, "    andq %s, %s\n", reg_aux, reg_dest);
            } else { // OR
                fprintf(out, "    orq %s, %s\n", reg_aux, reg_dest);
            }
        } 
        
        else if (strcmp(instr, "MUL") == 0) {
            // Se debe mover el resultado de %rax a reg_dest.
            const char* reg_dest = obtener_registro_temporal(extraer_numero_temporal(inst->resultado->name));
            // cargar el primer operando a %rax si o si convencion de assembler
            char *src1 = obtener_ubicacion_vars_locales(inst->arg1);
            
            if (strcmp(src1, "%rax") != 0) {
                fprintf(out, "    movq %s, %%rax\n", src1);
            }

            // cargar segundo operando a un auxiliar
            char *src2 = obtener_ubicacion_vars_locales(inst->arg2);
            
            if (strcmp(src2, "%r11") != 0) {
                fprintf(out, "    movq %s, %%r11\n", src2);
            }

            fprintf(out, "    imulq %%r11, %%rax\n");
            
            // guarda el resultado en r10 o r11
            if (strcmp(reg_dest, "%rax") != 0) {
                fprintf(out, "    movq %%rax, %s\n", reg_dest);
            }
        }

        else if (strcmp(instr, "DIV") == 0 || strcmp(instr, "MOD") == 0) {
            // si o si se tienen que usar %rax y %rdx
            const char* reg_dest = obtener_registro_temporal(extraer_numero_temporal(inst->resultado->name));
            char *src1 = obtener_ubicacion_vars_locales(inst->arg1);
            
            if (strcmp(src1, "%rax") != 0) {
                fprintf(out, "    movq %s, %%rax\n", src1);
            }

            // extension de signo de %rax a %rdx (Obligatorio para IDIVQ)
            fprintf(out, "    cqto\n");
            
            // aca se puede usar auxiliar usamos %r11
            char *src2 = obtener_ubicacion_vars_locales(inst->arg2);
            
            if (strcmp(src2, "%r11") != 0) {
                fprintf(out, "    movq %s, %%r11\n", src2);
            }
            
            // divide %rdx:%rax por %r11. cociente en %rax, Resto en %rdx)
            fprintf(out, "    idivq %%r11\n");
            
            // guardar el resultado en reg_dest, puede ser r10 o r11 y para div o mod 
            if (strcmp(instr, "DIV") == 0) {
                // Cociente en %rax
                fprintf(out, "    movq %%rax, %s\n", reg_dest);
            } else { // MOD
                // Resto en %rdx
                fprintf(out, "    movq %%rdx, %s\n", reg_dest);
            }
        }

        else if (strcmp(instr, "NEG") == 0) {
            const char *reg_dest = obtener_registro_temporal(extraer_numero_temporal(inst->resultado->name));

            char *src1 = obtener_ubicacion_vars_locales(inst->arg1);
            
            if (strcmp(src1, reg_dest) != 0) {
                fprintf(out, "    movq %s, %s\n", src1, reg_dest);
            }
            
            fprintf(out, "    negq %s\n", reg_dest);
        }

        else if (strcmp(instr, "NOT") == 0) {
            const char *reg_dest = obtener_registro_temporal(extraer_numero_temporal(inst->resultado->name));
            
            char *src1 = obtener_ubicacion_vars_locales(inst->arg1);
            
            if (strcmp(src1, reg_dest) != 0) {
                fprintf(out, "    movq %s, %s\n", src1, reg_dest);
            }
            // xor se usa xq invierte el bit (0->1, 1->0)
            fprintf(out, "    xorq $1, %s\n", reg_dest); 
        }
        
        else if (strcmp(instr, "EQ") == 0 || strcmp(instr, "GT") == 0 || strcmp(instr, "LT") == 0) {
            const char *reg_dest = obtener_registro_temporal(extraer_numero_temporal(inst->resultado->name));
            char *src1 = obtener_ubicacion_vars_locales(inst->arg1);
            char *src2 = obtener_ubicacion_vars_locales(inst->arg2);

            // cargar src1 a %rax
            if (strcmp(src1, "%rax") != 0) {
                fprintf(out, "    movq %s, %%rax\n", src1);
            }

            // cargar src2 a %r11
            if (strcmp(src2, "%r11") != 0) {
                fprintf(out, "    movq %s, %%r11\n", src2);
            }

            fprintf(out, "    cmpq %%r11, %%rax\n"); // Comparar arg1 (en %rax) vs arg2 (en %r11)

            // registro %al (parte baja de %rax), se usa si o si
            if (strcmp(instr, "EQ") == 0) {
                fprintf(out, "    sete %%al\n"); // setea si es igual
            } else if (strcmp(instr, "GT") == 0) {
                fprintf(out, "    setg %%al\n"); // setea si es mayor
            } else { // LT
                fprintf(out, "    setl %%al\n"); // setea si es menor
            }
            
            // mover el resultado (0 o 1) de %al al registro destino
            // limpiar %rax (usando la parte de 32 bits %eax)
            fprintf(out, "    movzbl %%al, %%eax\n"); 
            // mover el resultado final al registro temporal destino (%r10 o %r11)
            fprintf(out, "    movq %%rax, %s\n", reg_dest);
        }
        
        else {
            // instrucción no manejada: la imprimimos como comentario para debug
            fprintf(out, "    # instruccion no traducida: %s\n", instr);
        }

        inst = inst->siguiente; // avanzar en la lista
    }

    generar_epilogo_archivo(out);
}
