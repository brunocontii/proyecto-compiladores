#include <stdio.h>
#include <string.h>
#include "estructuras_metodos.h"

extern var_global* get_variables_globales(void);

// genera la seccion .data con las variables globales
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

// genera la seccion .text con el prologo inicial, indicando que la ejecucion comienza en main
void generar_seccion_text(FILE *out) {
    if (!out) return;
    fprintf(out, "\t.text\n");
    fprintf(out, "\t.globl\tmain\n");
    fprintf(out, "\t.type\tmain, @function\n\n");
}

// genera el epilogo del archivo ensamblador, cerrando secciones y definiendo tamaño de main
void generar_epilogo_archivo(FILE *out) {
    if (!out) return;
    fprintf(out, "\n.LFE0:\n");
    fprintf(out, "\t.size\tmain, .-main\n");
    fprintf(out, "\t.section\t.note.GNU-stack,\"\",@progbits\n");
}

// genera el prologo de un metodo, reservando espacio en el stack para variables locales, temporales y parametros
void generar_prologo_metodo(FILE *out, const char *nombre_metodo) {
    if (!out || !nombre_metodo) return;
    
    metodo_info *metodo = get_metodo_actual();
    if (!metodo) return;

    fprintf(out, "%s:\n", nombre_metodo);
    fprintf(out, "    pushq %%rbp\n");
    fprintf(out, "    movq %%rsp, %%rbp\n");

    int espacio_total = 8 * metodo->num_vars_locales;
    
    // alinear a 16 bytes, requerido por la convencion de llamadas
    if (espacio_total % 16 != 0) {
        espacio_total += (16 - (espacio_total % 16));
    }
    
    if (espacio_total > 0) {
        fprintf(out, "    subq $%d, %%rsp\n", espacio_total);
    }
}

// genera el epilogo de un metodo, restaurando el stack y retornando
void generar_epilogo_metodo(FILE *out) {
    if (!out) return;
    fprintf(out, "    movq %%rbp, %%rsp\n");
    fprintf(out, "    popq %%rbp\n");
    fprintf(out, "    ret\n");
}

