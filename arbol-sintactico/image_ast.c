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

void generateDotNodes(nodo* node, nodo* root, FILE* file, int* nodeCount, bool mostrar_se_usa) {
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
            if (decl && decl->izq) param_formal = decl->izq;

            while ((param_actual && param_actual->valor) || (param_formal && param_formal->valor)) {
                nodo *actual_elem = NULL;
                nodo *formal_elem = NULL;

                if (param_actual && param_actual->valor && param_actual->valor->tipo_token == T_EXPRS) {
                    actual_elem = param_actual->der;
                } else {
                    actual_elem = param_actual;
                }

                if (param_formal && param_formal->valor && param_formal->valor->tipo_token == T_PARAMETROS) {
                    formal_elem = param_formal->der;
                } else {
                    formal_elem = param_formal;
                }

                if (formal_elem && formal_elem->valor) {
                    tipo_info tf = formal_elem->valor->tipo_info;
                    if (actual_elem && actual_elem->valor) {
                        actual_elem->valor->tipo_info = tf;
                    }
                }

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

    // Etiqueta del nodo con se_usa
    char label_final[512];
    if (node->valor->tipo_token == T_ID || node->valor->tipo_token == T_DIGIT || 
        node->valor->tipo_token == T_BOOL || node->valor->tipo_token == T_PARAMETRO) {
        
        if (mostrar_se_usa && node->valor->tipo_token == T_ID) {
            snprintf(label_final, sizeof(label_final), "%s\\n(%s)\\n[%s]\\nUSADA: %s",
                    valueStr, 
                    getTokenString(node->valor->tipo_token),
                    getTypeString(node->valor->tipo_info),
                    node->valor->se_usa ? "True" : "False");
        } else {
            snprintf(label_final, sizeof(label_final), "%s\\n(%s)\\n[%s]",
                    valueStr, 
                    getTokenString(node->valor->tipo_token),
                    getTypeString(node->valor->tipo_info));
        }
    } else {
        snprintf(label_final, sizeof(label_final), "%s\\n(%s)",
                valueStr, 
                getTokenString(node->valor->tipo_token));
    }

    fprintf(file, "  node%d [label=\"%s\", shape=box];\n", currentId, label_final);

    if (node->izq != NULL) {
        int leftId = *nodeCount;
        generateDotNodes(node->izq, root, file, nodeCount, mostrar_se_usa);
        fprintf(file, "  node%d -> node%d [label=\"L\"];\n", currentId, leftId);
    }
    if (node->med != NULL) {
        int medId = *nodeCount;
        generateDotNodes(node->med, root, file, nodeCount, mostrar_se_usa);
        fprintf(file, "  node%d -> node%d [label=\"M\"];\n", currentId, medId);
    }
    if (node->der != NULL) {
        int rightId = *nodeCount;
        generateDotNodes(node->der, root, file, nodeCount, mostrar_se_usa);
        fprintf(file, "  node%d -> node%d [label=\"R\"];\n", currentId, rightId);
    }
}

//  Wrapper para generar con flag se_usa
void generateASTDotFileWithLiveness(nodo* root, const char* base_filename, bool mostrar_se_usa) {
    char dot_filename[256];
    char png_filename[256];
    
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
        generateDotNodes(root, root, file, &nodeCount, mostrar_se_usa);
    }

    fprintf(file, "}\n");
    fclose(file);

    char command[1024];
    snprintf(command, sizeof(command), "dot -Tpng %s -o %s 2>/dev/null", 
                dot_filename, png_filename);
    system(command);
}


void generateASTDotFile(nodo* root, const char* base_filename) {
    generateASTDotFileWithLiveness(root, base_filename, false);
}