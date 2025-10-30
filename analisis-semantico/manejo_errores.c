#include "manejo_errores.h"
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <stdarg.h>

extern int yylineno;

int errors = 0;

void reportar_error(int linea, const char* formato, ...) {
    errors++;
    printf(COLOR_RED "ERROR: %d (linea %d): ", errors, linea);

    va_list args;
    va_start(args, formato);
    vprintf(formato, args);
    va_end(args);
    printf("\n" COLOR_RESET);
}

void chequear_errores() {
    if (errors >= 1) {
        if (errors == 1) {
            printf(COLOR_RED "Compilacion FALLO con %d error\n" COLOR_RESET, errors);
        } else {
            printf(COLOR_RED "Compilacion FALLO con %d errores\n" COLOR_RESET, errors);
        }
        exit(1);
    } else {
        printf(COLOR_GREEN "Compilacion EXITOSA con %d errores\n" COLOR_RESET, errors);
    }
}

const char* tipo_info_to_string(tipo_info t) {
    switch (t) {
        case TIPO_INTEGER: return "integer";
        case TIPO_BOOL:    return "bool";
        case TIPO_VOID:    return "void";
        default:           return "desconocido";
    }
}
