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