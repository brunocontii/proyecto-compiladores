#include <stdio.h>
#include <string.h>
#include "assembler.h"
#include "estructuras_metodos.h"

// obtiene la ubicacion del operando en assembler, ya sea inmediato, variable local o global
char* obtener_ubicacion_operando(info *operando) {
    if (!operando) return NULL;
    
    char *loc = malloc(32);
    
    if (operando->tipo_token == T_DIGIT) {
        sprintf(loc, "$%d", operando->nro);
        return loc;
    }
    
    if (operando->tipo_token == T_VTRUE || operando->tipo_token == T_VFALSE) {
        int val = (strcmp(operando->bool_string, "true") == 0) ? 1 : 0;
        sprintf(loc, "$%d", val);
        return loc;
    }
    
    if (operando->esTemporal == 1) {
        int offset = obtener_offset_variable(operando->name);
        sprintf(loc, "%d(%%rbp)", offset);
        return loc;
    }
    
    if (!es_variable_global(operando->name)) {
        int offset = obtener_offset_variable(operando->name);
        sprintf(loc, "%d(%%rbp)", offset);
        return loc;
    }
    
    sprintf(loc, "%s(%%rip)", operando->name);
    return loc;
}

// tabla de instrucciones
typedef struct {
    const char *nombre;                         // nombre de la instruccion
    void (*manejador)(FILE*, codigo3dir*);      // puntero a la funcion que maneja la instruccion
                                                // la funcion toma 2 parametros (FILE*, codigo3dir*) y no retorna nada (void)
} entrada_instruccion;

static entrada_instruccion tabla_instrucciones[] = {
    // cuando encuentra la instruccion, llama a la funcion correspondiente
    {"LABEL", generar_label},
    {"END", generar_end},
    {"LOAD_PARAM", generar_load_param},
    {"PARAM", generar_param},
    {"CALL", generar_call},
    {"ASSIGN", generar_assign},
    {"ADD", generar_operacion_binaria},
    {"SUB", generar_operacion_binaria},
    {"AND", generar_operacion_binaria},
    {"OR", generar_operacion_binaria},
    {"MUL", generar_multiplicacion},
    {"DIV", generar_division_modulo},
    {"MOD", generar_division_modulo},
    {"NEG", generar_negacion},
    {"NOT", generar_not},
    {"EQ", generar_comparacion},
    {"GT", generar_comparacion},
    {"LT", generar_comparacion},
    {"RET", generar_return},
    {"GOTO", generar_goto},
    {"IF_FALSE", generar_if_false},
    {"EXTERN", NULL},  // no hacer nada, se asume que la funcion externa esta declarada en otro lado
    {NULL, NULL}
};

// procesa cada instruccion buscando en la tabla y llamando a la funcion correspondiente
static void procesar_instruccion(FILE *out, codigo3dir *inst) {
    for (int i = 0; tabla_instrucciones[i].nombre != NULL; i++) {
        if (strcmp(inst->instruccion, tabla_instrucciones[i].nombre) == 0) {
            if (tabla_instrucciones[i].manejador) {
                tabla_instrucciones[i].manejador(out, inst);
            }
            return;
        }
    }
    fprintf(out, "    # instruccion no manejada: %s\n", inst->instruccion);
}

// funcion principal para generar el codigo assembler
void generar_codigo_assembler(codigo3dir *programa, FILE *out) {
    if (!out || !programa) return;

    recolectar_variables_globales(programa);
    generar_seccion_data(out);
    generar_seccion_text(out);

    codigo3dir *inst = programa;
    while (inst != NULL && strcmp(inst->instruccion, "LABEL") != 0) {
        inst = inst->siguiente;
    }

    while (inst != NULL) {
        procesar_instruccion(out, inst);
        inst = inst->siguiente;
    }

    generar_epilogo_archivo(out);
}