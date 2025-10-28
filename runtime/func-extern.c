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
    
    if (fgets(buffer, sizeof(buffer), stdin) != NULL) {
        sscanf(buffer, "%lld", &i);
    }
    
    return i;
}