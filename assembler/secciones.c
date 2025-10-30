#include <stdio.h>
#include <string.h>
#include "estructuras_metodos.h"

extern var_global* get_variables_globales(void);

void generar_seccion_data(FILE *out) {
    var_global *vars = get_variables_globales();
    if (!vars || !out) return;

    fprintf(out, "\t.data\n");
    var_global *actual = vars;

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
    if (!out) return;
    fprintf(out, "\t.text\n");
    fprintf(out, "\t.globl\tmain\n");
    fprintf(out, "\t.type\tmain, @function\n\n");
}

void generar_epilogo_archivo(FILE *out) {
    if (!out) return;
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

    int espacio_total = 8 * metodo->num_vars_locales;
    
    // Alinear a 16 bytes
    if (espacio_total % 16 != 0) {
        espacio_total += (16 - (espacio_total % 16));
    }
    
    if (espacio_total > 0) {
        fprintf(out, "    subq $%d, %%rsp\n", espacio_total);
    }
}

void crear_epilogo_metodo(FILE *out) {
    if (!out) return;
    fprintf(out, "    movq %%rbp, %%rsp\n");
    fprintf(out, "    popq %%rbp\n");
    fprintf(out, "    ret\n");
}

