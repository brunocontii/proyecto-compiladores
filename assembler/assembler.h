#ifndef ASSEMBLER_H
#define ASSEMBLER_H

#include <stdio.h>
#include "../codigo-intermedio/codigo3dir.h"
#include "instrucciones.h"

void generar_codigo_assembler(codigo3dir *programa, FILE *out);

#endif