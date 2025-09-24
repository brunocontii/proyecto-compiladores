#ifndef MANEJO_ERRORES_H
#define MANEJO_ERRORES_H

#define COLOR_RED     "\033[31m"
#define COLOR_GREEN   "\033[32m"
#define COLOR_YELLOW  "\033[33m"
#define COLOR_RESET   "\033[0m"

extern int yylineno;
extern int errors;

void reportar_error(const char* formato, ...);
void chequear_errores();

#endif