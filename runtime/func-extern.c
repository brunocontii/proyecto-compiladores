#include <stdio.h>
#include <stdbool.h>

// recibir un entero de 64 bits según la convención x86-64 (primer arg en %rdi)
void print_int(long long i) {
    printf("%lld\n", i);
    fflush(stdout);
}

void print_bool(bool a) {
    // Imprime 1 para true y 0 para false (una línea)
    printf("%d\n", a ? 1 : 0);
    fflush(stdout);
}

long long get_int() {
    char buffer[256];
    long long i = 0;
    
    fprintf(stdout, "Escriba un número: ");
    fflush(stdout); 

    // lee la linea completa de entrada
    if (fgets(buffer, sizeof(buffer), stdin) != NULL) {
        // analizar el entero de 64 bits (long long)
        if (sscanf(buffer, "%lld", &i) != 1) {
            // manejo de entrada no valida
            i = 0; 
        }
    }
    
    return i;
}

bool get_bool() {
    char buffer[256];
    
    fprintf(stdout, "Escriba 1 para True (Continuar) o 0 para False (Salir): ");
    fflush(stdout); 

    // lee la linea completa de entrada
    if (fgets(buffer, sizeof(buffer), stdin) != NULL) {
        int val = 0;
        // analizar el entero
        if (sscanf(buffer, "%d", &val) == 1) {
            return val != 0; // cualquier valor distinto de 0 es true
        }
    }
    
    return false; // valor por defecto en caso de error
}

void tds_printf(long long id) {
    switch (id) {
        case 0: puts("\n"); break;
        case 1: puts("Operación correcta"); break;
        case 2: puts("Error: fallo"); break;
        case 3: puts("Error: división por cero"); break;
        case 4: puts("Error: operación inválida"); break;
        case 5: puts("(1) para contunuar, (0) para salir: "); break;
        case 6: puts("Resultado:"); break;
        default:
            printf("Error: id no conocido\n");
            break;
    }
    fflush(stdout);
}