#ifndef CODIGO3DIR_H
#define CODIGO3DIR_H

#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include "../arbol-sintactico/arbol.h"

// estructura para representar una instruccion de 3 direcciones
typedef struct codigo3dir {
    char *instruccion;
    info *resultado;
    info *arg1;
    info *arg2;
    struct codigo3dir *siguiente;
} codigo3dir;

// punteros para la lista de instrucciones
extern codigo3dir *programa_inicio;
extern codigo3dir *programa_final;

// funciones para manipular la lista de instrucciones
void inicializar_programa(void);
void agregar_instruccion(const char *instruc, info *r, info *a1, info *a2);
void imprimir_programa(void);

#endif