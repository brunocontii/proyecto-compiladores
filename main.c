// main.c
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdbool.h>

#include "arbol-sintactico/arbol.h"
#include "tabla-simbolos/tabla_simbolos.h"
#include "analisis-semantico/semantico.h"
#include "analisis-semantico/manejo_errores.h"
#include "codigo-intermedio/generador.h"
#include "assembler/assembler.h"
#include "optimizaciones/optimizaciones.h"

#define COLOR_RED     "\033[31m"
#define COLOR_GREEN   "\033[32m"
#define COLOR_YELLOW  "\033[33m"
#define COLOR_CYAN    "\033[36m"
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
int debug = 0;
bool opt_constant_folding = false;
bool opt_codigo_muerto_var = false;
bool opt_codigo_muerto_codigo_inalcanzable = false;
bool opt_operaciones = false;
bool opt_codigo_muerto_bloque = false;

// Función para mostrar uso
void opciones() {
    printf("Uso: c-tds [opcion] archivo.ctds\n");
    printf("Opciones:\n");
    printf("  -o <salida>          Archivo de salida\n");
    printf("  -target <etapa>      Etapa de compilacion: lex, parse, sem, codinter, assembly\n");
    printf("  -opt <optimizacion>  Habilitar optimización:\n");
    printf("                         prop-constantes  - Propagación de constantes\n");
    printf("                         var-muertas      - Eliminación de variables no usadas\n");
    printf("  -debug               Imprimir info de debug\n");
    printf("\nEjemplos:\n");
    printf("  ./c-tds -target assembly tests/test01.ctds\n");
    printf("  ./c-tds -target assembly -opt prop-constantes tests/test02.ctds\n");
    printf("  ./c-tds -target assembly -opt var-muertas tests/test03.ctds\n");
    printf("  ./c-tds -target assembly -opt prop-constantes -opt var-muertas tests/test04.ctds\n");
    printf("  ./c-tds -target assembly -opt cod-inalcanzable tests/test05.ctds\n");
    printf("  ./c-tds -target assembly -opt operaciones tests/test06.ctds\n");
    printf("  ./c-tds -target assembly -opt cod-bloque tests/test07.ctds\n");
}

// Función auxiliar para recorrer solo el lexer
void lexer_loop() {
    int tok;
    while ((tok = yylex()) != 0) {
        printf("TOKEN: %d\n", tok);
    }
}

int main(int argc, char *argv[]) {
    if (argc < 2) {
        opciones();
        return 1;
    }

    opt_constant_folding = false;
    opt_codigo_muerto_var = false;
    opt_codigo_muerto_codigo_inalcanzable = false;
    opt_operaciones = false;
    opt_codigo_muerto_bloque = false;

    // --- Parseo de opciones ---
    int i = 1;
    while (i < argc - 1) {
        if (strcmp(argv[i], "-o") == 0) {
            archivo_salida = argv[i+1];
            i += 2;
        } else if (strcmp(argv[i], "-target") == 0) {
            target = argv[i+1];
            i += 2;
        } 
        else if (strcmp(argv[i], "-opt") == 0) {
            if (i + 1 >= argc) {
                fprintf(stderr, COLOR_RED "Error: -opt requiere un argumento\n" COLOR_RESET);
                opciones();
                return 1;
            }
            i++;
            
            bool opt_reconocida = false;
            
            if (strcmp(argv[i], "prop-constantes") == 0) {
                opt_constant_folding = true;
                opt_reconocida = true;
                printf(COLOR_CYAN "🔧 Optimización habilitada: Propagación de Constantes\n" COLOR_RESET);
            }
            
            else if (strcmp(argv[i], "var-muertas") == 0) {
                opt_codigo_muerto_var = true;
                opt_reconocida = true;
                printf(COLOR_CYAN "🔧 Optimización habilitada: Eliminación de Variables Muertas\n" COLOR_RESET);
            }
            else if (strcmp(argv[i], "cod-inalcanzable") == 0) {
                opt_codigo_muerto_codigo_inalcanzable = true;
                opt_reconocida = true;
                printf(COLOR_CYAN "🔧 Optimización habilitada: Eliminación de Código Inalcanzable\n" COLOR_RESET);
            }
            else if (strcmp(argv[i], "operaciones") == 0) {
                opt_operaciones = true;
                opt_reconocida = true;
                printf(COLOR_CYAN "🔧 Optimización habilitada: Optimización de Operaciones\n" COLOR_RESET);
            }
            else if (strcmp(argv[i], "cod-bloque") == 0) {
                opt_codigo_muerto_bloque = true;
                opt_reconocida = true;
                printf(COLOR_CYAN "🔧 Optimización habilitada: Eliminación de Código Muerto en Bloques\n" COLOR_RESET);
            }
            
            if (!opt_reconocida) {
                fprintf(stderr, COLOR_RED "Error: optimización desconocida '%s'\n" COLOR_RESET, argv[i]);
                fprintf(stderr, "\nOptimizaciones disponibles:\n");
                fprintf(stderr, "  prop-constantes  - Propagación de constantes en tiempo de compilación\n");
                fprintf(stderr, "  var-muertas      - Eliminación de variables no usadas\n");
                fprintf(stderr, "  cod-inalcanzable - Eliminación de código inalcanzable\n");
                fprintf(stderr, "  operaciones      - Optimización de operaciones aritméticas y lógicas\n");
                fprintf(stderr, "\nEjemplo: ./c-tds -target assembly -opt prop-constantes test.ctds\n");
                return 1;
            }
            
            i++;
        }
        else if (strcmp(argv[i], "-debug") == 0) {
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
    if (!target) target = "assembly";

    if (debug) {
        printf("Archivo entrada: %s\n", archivo_entrada);
        if (archivo_salida) printf("Archivo salida: %s\n", archivo_salida);
        printf("Target: %s\n", target);
        printf("Constant Folding: %s\n", opt_constant_folding ? "SI" : "NO");
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
            aplicar_optimizaciones(raiz);
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
            aplicar_optimizaciones(raiz);
            codigo_intermedio(raiz);
            imprimir_programa();
        }
    }
    else if (strcmp(target, "assembly") == 0) {
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
            recorridoSemantico(raiz, ts);
            if (!hay_main) {
                reportar_error(yylineno, "Error semántico: Falta definir la función main\n");
            }
            aplicar_optimizaciones(raiz);
            generateASTDotFile(raiz, archivo_salida);     
            codigo_intermedio(raiz);
            imprimir_programa();

            FILE *out2 = fopen("assembler.s", "w");
            if (!out2) {
                perror("No se pudo abrir el archivo de salida");
                exit(1);
            }

            // generar assembler desde la lista global 'programa'
            generar_codigo_assembler(programa_inicio, out2);
            fclose(out2);
        }    
    }
    else {
        printf(COLOR_YELLOW "Target desconocido: %s\n" COLOR_RESET , target);
        opciones();
        return 1;
    }

    chequear_errores();
    return 0;
}
