#include <stdio.h>
#include <stdbool.h>

// recibir un entero de 64 bits según la convención x86-64 (primer arg en %rdi)
void print_int(long long i) {
    printf("%lld\n", (long long)i);
    fflush(stdout);
}

void print_bool(bool a) {
    // Imprime "true" / "false" (una línea)
    printf("%s\n", a ? "true" : "false");
    fflush(stdout);
}