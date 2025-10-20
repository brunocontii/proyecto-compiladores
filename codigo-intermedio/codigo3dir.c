#include <string.h>
#include "codigo3dir.h"

codigo3dir *programa_inicio = NULL;
codigo3dir *programa_final = NULL;

void inicializar_programa(void) {
    programa_inicio = NULL;
    programa_final = NULL;
}

// agregar una instruccion al final de la lista
void agregar_instruccion(const char *instruc, info *r, info *a1, info *a2) {
    codigo3dir *nueva = (codigo3dir*)malloc(sizeof(codigo3dir));
    if (nueva == NULL) {
        fprintf(stderr, "Error: no se pudo asignar memoria para instruccion\n");
        exit(1);
    }

    nueva->instruccion = strdup(instruc);
    nueva->resultado = r;
    nueva->arg1 = a1;
    nueva->arg2 = a2;
    nueva->siguiente = NULL;

    // se agrega siempre al final
    if (programa_inicio == NULL) {
        // si es la primera instruccion
        programa_inicio = nueva;
        programa_final = nueva;
    } else {
        // sino se agrega al final
        programa_final->siguiente = nueva;
        programa_final = nueva;
    }

}

void imprimir_programa(void) {
    printf("\n--- TABLA DE INSTRUCCIONES ---\n");
    printf("%-5s %-20s %-15s %-15s %-15s\n",
            "IDX", "INSTRUCCION", "RESULTADO", "ARG1", "ARG2");
    printf("---------------------------------------------------------------\n");

    codigo3dir *actual = programa_inicio;
    int idx = 0;
    
    while (actual != NULL) {
        printf("%-5d %-20s %-15s %-15s %-15s\n",
                idx,
                actual->instruccion,
                actual->resultado ? actual->resultado->name : "-",
                actual->arg1 ? actual->arg1->name : "-",
                actual->arg2 ? actual->arg2->name : "-");
        actual = actual->siguiente;
        idx++;
    }

    printf("---------------------------------------------------------------\n");
}

