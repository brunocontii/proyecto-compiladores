#ifndef ARBOL_H
#define ARBOL_H

#include <stdbool.h>

typedef enum {
    T_PROGRAM,          // 0
    T_EXTERN,           // 1
    T_RETURN,           // 2
    T_IF,               // 3
    T_IF_ELSE,          // 4
    T_WHILE,            // 5
    T_VTRUE,            // 6
    T_VFALSE,           // 7
    T_OP_AND,           // 8
    T_OP_OR,            // 9
    T_OP_NOT,           // 10
    T_INTEGER,          // 11
    T_BOOL,             // 12
    T_ID,               // 13
    T_DIGIT,            // 14
    T_OP_MENOS,         // 15
    T_OP_MAS,           // 16
    T_OP_MULT,          // 17
    T_OP_DIV,           // 18
    T_OP_RESTO,         // 19
    T_ASIGNACION,       // 20
    T_IGUALDAD,         // 21
    T_MENOR,            // 22
    T_MAYOR,            // 23
    T_VAR_DECLS,        // 24
    T_VAR_DECL,         // 25
    T_METHOD_DECLS,     // 26
    T_METHOD_DECL,      // 27
    T_PARAMETROS,       // 28
    T_PARAMETRO,        // 29
    T_BLOQUE,           // 30
    T_METHOD_CALL,      // 31
    T_EXPRS,            // 32
    T_STATEMENTS        // 33
} tipo_token;

typedef enum {
    TIPO_INTEGER,       // 0
    TIPO_BOOL,          // 1
    TIPO_VOID           // 2
} tipo_info;

typedef struct {
    int nro;
    int esTemporal;         // 1 si es temporal, 0 si es variable normal
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
void generateASTDotFile(nodo* root, const char* base_filename);

#endif