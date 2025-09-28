#ifndef ARBOL_H
#define ARBOL_H

#include <stdbool.h>

typedef enum {
    T_PROGRAM,          // 0
    T_EXTERN,           // 0
    T_RETURN,           // 0
    T_IF,               // 0
    T_IF_ELSE,          // 0
    T_WHILE,            // 0
    T_VTRUE,            // 0
    T_VFALSE,           // 0
    T_OP_AND,           // 0
    T_OP_OR,            // 0
    T_OP_NOT,           // 0
    T_INTEGER,          // 0
    T_BOOL,             // 0
    T_ID,               // 0
    T_DIGIT,            // 0
    T_OP_MENOS,         // 0
    T_OP_MAS,           // 0
    T_OP_MULT,          // 0
    T_OP_DIV,           // 0
    T_OP_RESTO,         // 0
    T_ASIGNACION,       // 0
    T_IGUALDAD,         // 0
    T_MENOR,            // 0
    T_MAYOR,            // 0
    T_VAR_DECLS,        // 0
    T_VAR_DECL,         // 0
    T_METHOD_DECLS,     // 0
    T_METHOD_DECL,      // 0
    T_PARAMETROS,       // 0
    T_PARAMETRO,        // 0
    T_BLOQUE,           // 0
    T_METHOD_CALL,      // 0
    T_EXPRS,            // 0
    T_STATEMENTS        // 0
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
    int linea;
    struct nodo *izq;
    struct nodo *med;
    struct nodo *der;
} nodo;

nodo* crearNodo(info *valorNodo);
nodo* crearArbol(info *valorNodo, nodo* hijoIzq, nodo* hijoDer);
nodo* crearArbolTer(info *valorNodo, nodo* hijoIzq, nodo* hijoMed, nodo* hijoDer);
nodo* buscarNodo(nodo* raiz, const char* nombre);
void mostrarArbol(nodo *raiz, int nivel);
void liberarArbol(nodo *raiz);

#endif