// assembler/assembler.c
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include "../codigo-intermedio/codigo3dir.h"
#include "assembler.h"

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

    codigo3dir *inst = programa;

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
                        inst->arg1, inst->resultado->name, inst->arg1);
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
}
