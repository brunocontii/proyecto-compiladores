#ifndef INSTRUCTION_HANDLERS_H
#define INSTRUCTION_HANDLERS_H

#include <stdio.h>
#include "../codigo-intermedio/codigo3dir.h"
#include "estructuras_metodos.h"

// generadores de instrucciones especificas
void generar_label(FILE *out, codigo3dir *inst);
void generar_end(FILE *out, codigo3dir *inst);
void generar_load_param(FILE *out, codigo3dir *inst);
void generar_param(FILE *out, codigo3dir *inst);
void generar_call(FILE *out, codigo3dir *inst);
void generar_assign(FILE *out, codigo3dir *inst);
void generar_operacion_binaria(FILE *out, codigo3dir *inst);
void generar_multiplicacion(FILE *out, codigo3dir *inst);
void generar_division_modulo(FILE *out, codigo3dir *inst);
void generar_negacion(FILE *out, codigo3dir *inst);
void generar_not(FILE *out, codigo3dir *inst);
void generar_comparacion(FILE *out, codigo3dir *inst);
void generar_return(FILE *out, codigo3dir *inst);
void generar_goto(FILE *out, codigo3dir *inst);
void generar_if_false(FILE *out, codigo3dir *inst);
void generar_multiplicacion_opt(FILE *out, codigo3dir *inst);
void generar_division_opt(FILE *out, codigo3dir *inst);

#endif