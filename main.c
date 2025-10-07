// main.c
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "arbol-sintactico/arbol.h"
#include "tabla-simbolos/tabla_simbolos.h"
#include "analisis-semantico/semantico.h"
#include "utils/manejo_errores.h"
#include "codigo-intermedio/generador.h"

#define COLOR_RED     "\033[31m"
#define COLOR_GREEN   "\033[32m"
#define COLOR_YELLOW  "\033[33m"
#define COLOR_RESET   "\033[0m"

extern FILE* yyin;              // puntero de entrada para el lexer
extern int yylex();             // lexer
extern int yyparse();           // parser
extern nodo* raiz;              // raíz del AST
extern tabla_simbolos *ts;      // tabla de símbolos
extern bool hay_main;           // indica si se encontró la función main

// Variables de configuración
char* archivo_entrada = NULL;
char* archivo_salida = NULL;
char* target = NULL;
char* optimizacion = NULL;
int debug = 0;

// Función para mostrar uso
void opciones() {
    printf("Uso: c-tds [opcion] archivo.ctds\n");
    printf("Opciones:\n");
    printf("  -o <salida>       Archivo de salida\n");
    printf("  -target <etapa>   Etapa de compilacion: lex, parse, sem, codinter, assembly\n");
    printf("  -opt [optimizacion]  Lista de optimizaciones (solo futuro)\n");
    printf("  -debug            Imprimir info de debug\n");
}

// Función auxiliar para recorrer solo el lexer
void lexer_loop() {
    int tok;
    while ((tok = yylex()) != 0) {
        printf("TOKEN: %d\n", tok);  // podrías mostrar token como string si tienes función de mapeo
    }
}

int main(int argc, char *argv[]) {
    if (argc < 2) {
        opciones();
        return 1;
    }

    // --- Parseo de opciones ---
    int i = 1;
    while (i < argc - 1) {
        if (strcmp(argv[i], "-o") == 0) {
            archivo_salida = argv[i+1];
            i += 2;
        } else if (strcmp(argv[i], "-target") == 0) {
            target = argv[i+1];
            i += 2;
        } else if (strcmp(argv[i], "-opt") == 0) {
            optimizacion = argv[i+1];
            i += 2;
        } else if (strcmp(argv[i], "-debug") == 0) {
            debug = 1;
            i += 1;
        } else {
            printf("Opcion desconocida: %s\n", argv[i]);
            opciones();
            return 1;
        }
    }

    // --- Archivo de entrada ---
    archivo_entrada = argv[argc-1];
    if (archivo_entrada[0] == '-') {
        printf("Error: archivo no puede empezar con '-'\n");
        return 1;
    }
    int len = strlen(archivo_entrada);
    if (len < 6 || strcmp(archivo_entrada + len - 5, ".ctds") != 0) {
        printf("Error: archivo debe tener extension .ctds\n");
        return 1;
    }

    yyin = fopen(archivo_entrada, "r");
    if (!yyin) {
        perror("Error al abrir archivo");
        return 1;
    }

    // --- Inicializar tabla de símbolos ---
    ts = malloc(sizeof(tabla_simbolos));
    if (!ts) {
        fprintf(stderr, "Error: no se pudo asignar memoria para la tabla\n");
        return 1;
    }
    inicializar(ts);

    // Target por defecto
    if (!target) target = "codinter";

    if (debug) {
        printf("Archivo entrada: %s\n", archivo_entrada);
        if (archivo_salida) printf("Archivo salida: %s\n", archivo_salida);
        printf("Target: %s\n", target);
        if (optimizacion) printf("Optimizacion: %s\n", optimizacion);
        printf("Debug activado\n");
    }
    // -----------------------------
    // Lógica según target
    // -----------------------------
    if (strcmp(target, "lex") == 0) {
        int token;
        while ((token = yylex()) != 0) { 
            /* Imprime los token del .l aca */ 
        }
    } 
    else if (strcmp(target, "parse") == 0) {
        int res = yyparse();
        fclose(yyin);

        if (res != 0) {
            printf(COLOR_RED "Errores de parseo, no se puede continuar.\n" COLOR_RESET);
            exit(1);
        }

        // Mostrar árbol y generar imagen
        if (raiz) {
            mostrarArbol(raiz, 0);
            if (!archivo_salida) {
                archivo_salida = "ctds_arbol";
            }
            generateASTDotFile(raiz, archivo_salida);        
        }
    }
    else if (strcmp(target, "sem") == 0) {
        int res = yyparse();
        fclose(yyin);
        if (res != 0) {
            printf(COLOR_RED "Errores de parseo, no se puede continuar.\n" COLOR_RESET);
            exit(1);
        }

        if (raiz) {
            mostrarArbol(raiz, 0);
            if (!archivo_salida) {
                archivo_salida = "ctds_arbol";
            }
            generateASTDotFile(raiz, archivo_salida);     
            recorridoSemantico(raiz, ts);
            if (!hay_main) {
                reportar_error(yylineno, "Error semántico: Falta definir la función main\n");
            }
        }
    }
    else if (strcmp(target, "codinter") == 0) {
        int res = yyparse();
        fclose(yyin);
        if (res != 0) {
            printf(COLOR_RED "Errores de parseo, no se puede continuar.\n" COLOR_RESET);
            exit(1);
        }

        if (raiz) {
            mostrarArbol(raiz, 0);
            if (!archivo_salida) {
                archivo_salida = "ctds_arbol";
            }
            generateASTDotFile(raiz, archivo_salida);     
            recorridoSemantico(raiz, ts);
            if (!hay_main) {
                reportar_error(yylineno, "Error semántico: Falta definir la función main\n");
            }
            FILE *out = fopen("codigo-intermedio.txt", "w");
            if (!out) {
                perror("No se pudo abrir el archivo de salida");
                exit(1);
            }
            codigo_intermedio(raiz, out);
            imprimir_programa();
            fclose(out);
        }
    }
    else if (strcmp(target, "assembly") == 0) {
        printf("Target assembly: pendiente implementar\n");
    }
    else {
        printf(COLOR_YELLOW "Target desconocido: %s\n" COLOR_RESET , target);
        opciones();
        return 1;
    }

    chequear_errores();
    return 0;
}
