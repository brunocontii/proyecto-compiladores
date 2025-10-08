#include "ver.h"

void generar_codigo_assembler(codigo3dir programa[], FILE *out) {
    if (!out) return;
    
    for (int i = 0; i < length(programa); i++) {
        codigo3dir inst = programa[i];
        
        if (strcmp(inst.instruccion, "LOAD") == 0) {
            fprintf(out, "MOV %s, %s\n", inst.resultado, inst.argumento1);
        }
        else if (strcmp(inst.instruccion, "ADD") == 0) {
            fprintf(out, "ADD %s, %s, %s\n", inst.resultado, inst.argumento1, inst.argumento2);
        }
        else if (strcmp(inst.instruccion, "CALL") == 0) {
            fprintf(out, "CALL %s\n", inst.argumento1);
        }
    }

}