#ifndef MANEJO_ERRORES_H
#define MANEJO_ERRORES_H

#define COLOR_RED     "\033[31m"
#define COLOR_GREEN   "\033[32m"
#define COLOR_YELLOW  "\033[33m"
#define COLOR_RESET   "\033[0m"

#include "../arbol-sintactico/arbol.h"

extern int yylineno;
extern int errors;

void reportar_error(int linea, const char* formato, ...);
void chequear_errores();
const char* tipo_info_to_string(tipo_info t);

#endif