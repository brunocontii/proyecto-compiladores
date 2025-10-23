#ifndef ASSEMBLER_H
#define ASSEMBLER_H

#include "../codigo-intermedio/generador.h"

// estructura para almacenar variables globales
typedef struct var_global {
    char *nombre;
    char *valor_inicial;
    struct var_global *siguiente;
} var_global;

typedef struct var_offset {
    char *nombre;
    int offset;
    struct var_offset *siguiente;
} var_offset;

void generar_codigo_assembler(codigo3dir *programa, FILE *out);
void agregar_variable_global(const char *nombre, const char *valor);
void recolectar_variables_globales(codigo3dir *programa);
void generar_seccion_data(FILE *out);
int contar_locales_por_metodo(codigo3dir *label);
void generar_seccion_text(FILE *out);
void generar_epilogo_archivo(FILE *out);

#endif