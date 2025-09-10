#ifndef ARBOL_H
#define ARBOL_H

#include <stdbool.h>

typedef enum {
    T_PROGRAM,
    T_EXTERN,
    T_RETURN,
    T_IF,
    T_ELSE,
    T_THEN,
    T_WHILE,
    T_VTRUE,
    T_VFALSE,
    T_OP_AND,
    T_OP_OR,
    T_OP_NOT,
    T_INTEGER,
    T_BOOL,
    T_VOID,
    T_ID,
    T_DIGIT,
    T_OP_MENOS,
    T_OP_MAS,
    T_OP_MULT,
    T_OP_DIV,
    T_OP_RESTO,
    T_ASIGNACION,
    T_IGUALDAD,
    T_MENOR,
    T_MAYOR,
    T_PYC,
    T_COMA,
    T_PAR_A,
    T_PAR_C,
    T_LLA_A,
    T_LLA_C,
    T_VAR_DECLS,
    T_VAR_DECL,
    T_METHOD_DECLS,
    T_METHOD_DECL,
    T_PARAMETROS,
    T_BLOQUE,
    T_METHOD_CALL,
    T_EXPRS,
    T_EXPR,
    T_STATEMENTS
} tipo_token;

typedef enum {
    TIPO_INTEGER,
    TIPO_BOOL, 
    TIPO_VOID
} tipo_info;

typedef struct {
    int nro;
    bool b;
    char *name;
    char *op;
    char *bool_string;
    tipo_info tipo_info;
    tipo_token tipo_token; 
} info;

typedef struct nodo {
    info *valor;
    struct nodo *izq;
    struct nodo *der;
} nodo;

nodo* crearNodo(info *valorNodo);
nodo* crearArbol(info *valorNodo, nodo* hijoIzq, nodo* hijoDer);
void mostrarArbol(nodo *raiz, int nivel);
void liberarArbol(nodo *raiz);

#endif