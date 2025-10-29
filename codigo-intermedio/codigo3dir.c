#include <string.h>
#include "codigo3dir.h"

codigo3dir *programa_inicio = NULL;
codigo3dir *programa_final = NULL;

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

// función auxiliar para imprimir un operando según su tipo
void imprimir_operando(info *operando, char *buffer, size_t size) {
    if (!operando) {
        snprintf(buffer, size, "-");
        return;
    }
    
    if (operando->tipo_token == T_DIGIT) {
        snprintf(buffer, size, "%d", operando->nro);
    }
    else if (operando->tipo_token == T_VTRUE || operando->tipo_token == T_VFALSE) {
        snprintf(buffer, size, "%s", operando->bool_string);
    }
    else if (operando->tipo_token == T_ID || operando->esTemporal == 1) {
        snprintf(buffer, size, "%s", operando->name);
    }
    else {
        snprintf(buffer, size, "%s", operando->name);
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
        char resultado_str[16];
        char arg1_str[16];
        char arg2_str[16];
        
        // obtener strings formateados segun el tipo
        imprimir_operando(actual->resultado, resultado_str, sizeof(resultado_str));
        imprimir_operando(actual->arg1, arg1_str, sizeof(arg1_str));
        imprimir_operando(actual->arg2, arg2_str, sizeof(arg2_str));
        
        printf("%-5d %-20s %-15s %-15s %-15s\n",
                idx,
                actual->instruccion,
                resultado_str,
                arg1_str,
                arg2_str);
        
        actual = actual->siguiente;
        idx++;
    }

    printf("---------------------------------------------------------------\n");
}

