// assembler/assembler.c
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include "../codigo-intermedio/codigo3dir.h"
#include "assembler.h"

static var_global *variables_globales = NULL;

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

    fprintf(out, ".LFE0:\n");
    fprintf(out, "\t.size\tmain, .-main\n");
    fprintf(out, "\t.section\t.note.GNU-stack,\"\",@progbits\n");
}

int contar_locales_por_metodo(codigo3dir *label) {

    codigo3dir *p = label->siguiente;
    // arreglo simple para nombres únicos (no sofisticado): max 1024 variables por función
    const char *a[100];
    int var_locales = 0;

    while (p != NULL) {
        if (strcmp(p->instruccion, "END") == 0) {
            break; // si es END listo
        }

        if (strcmp(p->instruccion, "ASSIGN") == 0) {
            if (p->resultado->esTemporal == 0) { // si es una variable real no temporal
                int i; 
                bool found = false;
                for (i = 0; i < var_locales; ++i) {
                    if (strcmp(a[i], p->resultado->name) == 0) {  // entra aca si ya estaba la variable en el arreglo
                        found = true; 
                        break; 
                    }
                }
                if (!found) {
                    a[var_locales] = p->resultado->name;
                    var_locales++;
                }
            }
        }

        if (strcmp(p->instruccion, "VAR_DECL") == 0) {
            if (p->resultado->esTemporal == 0) { // si es una variable real no temporal
                int i; 
                bool found = false;
                for (i = 0; i < var_locales; ++i) {
                    if (strcmp(a[i], p->resultado->name) == 0) {  // entra aca si ya estaba la variable en el arreglo
                        found = true; 
                        break; 
                    }
                }
                if (!found) {
                    a[var_locales] = p->resultado->name;
                    var_locales++;
                }
            }
        }

        p = p->siguiente;
    }

    return var_locales;
}

/*
POR AHORA SIN USO
static const char *reg_name_for_temp(int n) {
    // temporales mapeados a r10 y r11, se usa uno luego otro y luego el primero de nuevo
    return (n % 2 == 0) ? "%%r10d" : "%%r11d";
}
*/

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
            fprintf(out, "%s:\n", inst->resultado->name);
            fprintf(out, "    pushq %%rbp\n");
            fprintf(out, "    movq %%rsp, %%rbp\n");

            int locales = contar_locales_por_metodo(inst);
            int N = 8 * locales; // reservar espacio para variables locales (8 bytes c/u)
            if (N > 0) {
                fprintf(out, "    subq $%d, %%rsp\n", N);
            }
        }
        else if (strcmp(instr, "END") == 0) {
            fprintf(out, "    movq %%rbp, %%rsp\n");
            fprintf(out, "    popq %%rbp\n");
            fprintf(out, "    ret\n");
        }
        else if (strcmp(instr, "LOAD") == 0) {
            // Cargar un valor inmediato o variable a un registro temporal
            // Ejemplo: LOAD T1, 5  => movq $5, -8(%rbp)
            if (inst->resultado && inst->arg1) {
                fprintf(out, "    movq $%s, -8(%%rbp)   # %s = %s\n",
                        inst->arg1->name, inst->resultado->name, inst->arg1->name);
            }
            // se debe hacer mas casos
        }

        else if (strcmp(instr, "ASSIGN") == 0) {
            // Asignación simple entre variables locales (por ahora solo comentario)
            // Ejemplo: ASSIGN x, T1 => movq -8(%rbp), -16(%rbp)
            if (inst->resultado && inst->arg1) {
                fprintf(out, "    # %s = %s (asignacion local)\n",
                        inst->resultado->name, inst->arg1->name);
            }
            // se debe hacer mas casos

        }else {
            // instrucción no manejada: la imprimimos como comentario para debug
            fprintf(out, "    # instruccion no traducida: %s\n", instr);
        }

        inst = inst->siguiente; // avanzar en la lista
    }

    generar_epilogo_archivo(out);
}
