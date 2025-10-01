#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include <string.h>
#include "arbol.h"

const char* getTokenString(tipo_token token) {
    switch(token) {
        case T_PROGRAM: return "PROGRAM";
        case T_EXTERN: return "EXTERN";
        case T_RETURN: return "RETURN";
        case T_IF: return "IF";
        case T_IF_ELSE: return "IF ELSE";
        case T_WHILE: return "WHILE";
        case T_VTRUE: return "TRUE";
        case T_VFALSE: return "FALSE";
        case T_OP_AND: return "&&";
        case T_OP_OR: return "||";
        case T_OP_NOT: return "!";
        case T_INTEGER: return "INTEGER";
        case T_BOOL: return "BOOL";
        case T_ID: return "ID";
        case T_DIGIT: return "DIGIT";
        case T_OP_MENOS: return "-";
        case T_OP_MAS: return "+";
        case T_OP_MULT: return "*";
        case T_OP_DIV: return "/";
        case T_OP_RESTO: return "RESTO";
        case T_ASIGNACION: return "=";
        case T_IGUALDAD: return "==";
        case T_MENOR: return "<";
        case T_MAYOR: return ">";
        case T_VAR_DECLS: return "VAR DECLS";
        case T_VAR_DECL: return "VAR DECL";
        case T_METHOD_DECLS: return "METHOD DECLS";
        case T_METHOD_DECL: return "METHOD DECL";
        case T_PARAMETROS: return "PARAMETROS";
        case T_PARAMETRO: return "PARAMETRO";
        case T_BLOQUE: return "BLOQUE";
        case T_METHOD_CALL: return "METHOD CALL";
        case T_EXPRS: return "EXPRS";
        case T_STATEMENTS: return "STATEMENTS";
        default: return "UNKNOWN";
    }
}

const char* getTypeString(tipo_info type) {
    switch(type) {
        case TIPO_INTEGER: return "INTEGER";
        case TIPO_BOOL: return "BOOLEAN";
        case TIPO_VOID: return "VOID";
        default: return "UNKNOWN";
    }
}


void getNodeValueString(nodo* node, char* buffer, size_t bufsize) {
    if (!node || !node->valor) {
        snprintf(buffer, bufsize, "?");
        return;
    }
    
    char temp[256] = "";
    bool first = true;
    
    if (node->valor->name) {
        if (!first) strcat(temp, " | ");
        strcat(temp, "name: ");
        strcat(temp, node->valor->name);
        first = false;
    }
    
    if (node->valor->op) {
        if (!first) strcat(temp, " | ");
        strcat(temp, "op: ");
        strcat(temp, node->valor->op);
        first = false;
    }
    
    if (node->valor->tipo_token == T_DIGIT) {
        if (!first) strcat(temp, " | ");
        char num_str[32];
        sprintf(num_str, "nro: %d", node->valor->nro);
        strcat(temp, num_str);
        first = false;
    }
    
    if (node->valor->tipo_token == T_BOOL) {
        if (!first) strcat(temp, " | ");
        char bool_str[64];
        
        if (node->valor->bool_string) {
            sprintf(bool_str, "bool_string: %s | b: %s", 
                    node->valor->bool_string, 
                    node->valor->b ? "true" : "false");
        } else {
            sprintf(bool_str, "b: %s", node->valor->b ? "true" : "false");
        }

        strcat(temp, bool_str);
        first = false;
    }
    
    
    if (first) {
        snprintf(temp, sizeof(temp), "%s", getTokenString(node->valor->tipo_token));
    }
    
    snprintf(buffer, bufsize, "%s", temp);
}

void generateDotNodes(nodo* node, nodo* root, FILE* file, int* nodeCount) {
    if (node == NULL) return;

    int currentId = (*nodeCount)++;
    char valueStr[256];
    getNodeValueString(node, valueStr, sizeof(valueStr));


    if (node->valor->tipo_token == T_METHOD_CALL) {

        if (node->izq && node->izq->valor && node->izq->valor->name) {
            nodo *decl = buscarNodo(root, node->izq->valor->name);
            if (decl && decl->valor) {
                node->izq->valor->tipo_info = decl->valor->tipo_info;
            }
        }

        if (node->der && node->der->valor) {
            nodo *decl = NULL;
            if (node->izq && node->izq->valor && node->izq->valor->name) {
                decl = buscarNodo(root, node->izq->valor->name);
            }

            nodo *param_actual = node->der;
            nodo *param_formal = NULL;
            if (decl && decl->izq) param_formal = decl->izq; // puede ser T_PARAMETROS (lista) o parametro (hoja)

            // recorrer ambos: soporta lista (T_EXPRS / T_PARAMETROS) o hoja
            while ((param_actual && param_actual->valor) || (param_formal && param_formal->valor)) {
                nodo *actual_elem = NULL;
                nodo *formal_elem = NULL;

                if (param_actual && param_actual->valor && param_actual->valor->tipo_token == T_EXPRS) {
                    actual_elem = param_actual->der; // elemento actual en lista left-recursive
                } else {
                    actual_elem = param_actual;      // hoja o expresión simple
                }

                if (param_formal && param_formal->valor && param_formal->valor->tipo_token == T_PARAMETROS) {
                    formal_elem = param_formal->der; // elemento formal en lista
                } else {
                    formal_elem = param_formal;      // hoja (un parametro)
                }

                // copiar tipo si tenemos ambos elementos y el formal define tipo_info
                if (formal_elem && formal_elem->valor) {
                    tipo_info tf = formal_elem->valor->tipo_info;
                    if (actual_elem && actual_elem->valor) {
                        // asignar tipo al nodo actual para que se vea en la visualización
                        actual_elem->valor->tipo_info = tf;
                    }
                }

                // avanzar en listas: left-recursive lista almacena siguiente en izq
                if (param_actual && param_actual->valor && param_actual->valor->tipo_token == T_EXPRS) {
                    param_actual = param_actual->izq;
                } else {
                    param_actual = NULL;
                }

                if (param_formal && param_formal->valor && param_formal->valor->tipo_token == T_PARAMETROS) {
                    param_formal = param_formal->izq;
                } else {
                    param_formal = NULL;
                }
            }
        }
    }


    if (node->valor->tipo_token == T_ID || node->valor->tipo_token == T_DIGIT || node->valor->tipo_token == T_BOOL || node->valor->tipo_token == T_PARAMETRO) {
        fprintf(file, "  node%d [label=\"%s\\n(%s)\\n[%s]\", shape=box];\n",
            currentId, valueStr, getTokenString(node->valor->tipo_token), getTypeString(node->valor->tipo_info));
    } else {
        fprintf(file, "  node%d [label=\"%s\\n(%s)\", shape=box];\n",
            currentId, valueStr, getTokenString(node->valor->tipo_token));
    }

    if (node->izq != NULL) {
        int leftId = *nodeCount;
        generateDotNodes(node->izq, root, file, nodeCount);
        fprintf(file, "  node%d -> node%d [label=\"L\"];\n", currentId, leftId);
    }
    if (node->med != NULL) {
        int medId = *nodeCount;
        generateDotNodes(node->med, root, file, nodeCount);
        fprintf(file, "  node%d -> node%d [label=\"M\"];\n", currentId, medId);
    }
    if (node->der != NULL) {
        int rightId = *nodeCount;
        generateDotNodes(node->der, root, file, nodeCount);
        fprintf(file, "  node%d -> node%d [label=\"R\"];\n", currentId, rightId);
    }
}

void generateASTDotFile(nodo* root, const char* base_filename) {
    char dot_filename[256];
    char png_filename[256];
    
    // Crear nombres de archivos
    snprintf(dot_filename, sizeof(dot_filename), "%s.dot", base_filename);
    snprintf(png_filename, sizeof(png_filename), "%s.png", base_filename);
    
    FILE* file = fopen(dot_filename, "w");
    if (file == NULL) {
        printf("Error: No se pudo crear el archivo %s\n", dot_filename);
        return;
    }

    fprintf(file, "digraph AST {\n");
    fprintf(file, "  rankdir=TB;\n");
    fprintf(file, "  node [fontname=\"Arial\"];\n");
    fprintf(file, "  edge [fontname=\"Arial\"];\n");

    if (root == NULL) {
        fprintf(file, "  empty [label=\"Árbol vacío\", shape=plaintext];\n");
    } else {
        int nodeCount = 0;
        generateDotNodes(root, root, file, &nodeCount);
    }

    fprintf(file, "}\n");
    fclose(file);

    printf("Archivo DOT del AST generado: %s\n", dot_filename);

    char command[1024];
    snprintf(command, sizeof(command), "dot -Tpng %s -o %s 2>/dev/null", 
                dot_filename, png_filename);
    
    int result = system(command);
    if (result == 0) {
        printf("Imagen AST generada: %s\n", png_filename);
    } else {
        printf("Error al generar imagen. Instala: sudo apt install graphviz\n");
        printf("Ejecuta manualmente: dot -Tpng %s -o %s\n", dot_filename, png_filename);
    }
}

