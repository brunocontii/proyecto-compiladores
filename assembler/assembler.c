#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include "../codigo-intermedio/codigo3dir.h"
#include "assembler.h"

static var_global *variables_globales = NULL;
static var_offset *mapeo_variables = NULL;
static int locales = 0;
static int num_params_actual = 0;

bool es_variable_global(const char *nombre) {
    var_global *actual = variables_globales;
    while (actual != NULL) {
        if (strcmp(actual->nombre, nombre) == 0) {
            return true;
        }
        actual = actual->siguiente;
    }
    return false;
}

int extraer_numero_temporal(char *nombre) {
    // si es t0 queda 0, si es tn queda n
    if (nombre[0] == 'T') {
        return atoi(nombre + 1);
    }
    return -1;
}

// Crear mapeo de variables locales para un método
void crear_mapeo_variables_locales(codigo3dir *label) {
    // Limpiar mapeo anterior
    while (mapeo_variables != NULL) {
        var_offset *temp = mapeo_variables;
        mapeo_variables = mapeo_variables->siguiente;
        free(temp->nombre);
        free(temp);
    }
    
    codigo3dir *p = label->siguiente;
    const char *variables[100];
    int var_count = 0;
    
    // Primera pasada: recolectar variables únicas
    while (p != NULL) {
        if (strcmp(p->instruccion, "END") == 0) break;
        
        if (strcmp(p->instruccion, "ASSIGN") == 0) {
            if (p->resultado->esTemporal == 0 && !es_variable_global(p->resultado->name)) {
                // Es variable local
                bool found = false;
                for (int i = 0; i < var_count; i++) {
                    if (strcmp(variables[i], p->resultado->name) == 0) {
                        found = true;
                        break;
                    }
                }
                if (!found) {
                    variables[var_count] = p->resultado->name;
                    var_count++;
                }
            }
        }
        p = p->siguiente;
    }
    
    // Segunda pasada: crear mapeo con offsets
    for (int i = 0; i < var_count; i++) {
        var_offset *nuevo = malloc(sizeof(var_offset));
        nuevo->nombre = strdup(variables[i]);
        nuevo->offset = -8 * (i + 1);  // -8, -16, -24, etc.
        nuevo->siguiente = mapeo_variables;
        mapeo_variables = nuevo;
    }
}

// Obtener offset de una variable específica
int obtener_offset_variable(const char *nombre) {
    var_offset *actual = mapeo_variables;
    while (actual != NULL) {
        if (strcmp(actual->nombre, nombre) == 0) {
            return actual->offset;
        }
        actual = actual->siguiente;
    }
    return -8; // fallback
}

const char *obtener_registro_temporal(int n) {
    // temporales mapeados a r10 y r11, se usa uno luego otro y luego el primero de nuevo
    return (n % 2 == 0) ? "%r10" : "%r11";
}

// Función corregida para obtener ubicación
char* obtener_ubicacion_vars_locales(info *operando) {
    char *loc = malloc(32);
    
    // Si es temporal, usar stack temporal
    if (operando->esTemporal == 1) {
        int num_temp = extraer_numero_temporal(operando->name);
        const char *reg_temp = obtener_registro_temporal(num_temp);
        sprintf(loc, "%s", reg_temp);
    }
    // Si es variable local
    else if (!es_variable_global(operando->name)) {
        int offset = obtener_offset_variable(operando->name);
        sprintf(loc, "%d(%%rbp)", offset);
    }
    // Si es variable global
    else {
        sprintf(loc, "%s(%%rip)", operando->name);
    }
    
    return loc;
}

void agregar_variable_global(const char *nombre, const char *valor) {
    var_global *var_g = (var_global *)malloc(sizeof(var_global));
    var_g->nombre = strdup(nombre);
    var_g->valor_inicial = strdup(valor);  // guardar el valor inicial
    var_g->siguiente = NULL;
    
    // insertar al final asi se preserva el orden en como se definen
    if (variables_globales == NULL) {
        variables_globales = var_g;
    } else {
        var_global *actual = variables_globales;
        while (actual->siguiente != NULL) {
            actual = actual->siguiente;
        }
        actual->siguiente = var_g;
    }
}

void recolectar_variables_globales(codigo3dir *programa) {
    codigo3dir *inst = programa;
    codigo3dir *load_anterior = NULL;

    while (inst != NULL) {
        // si encontramos un LABEL se termina pq no hay declaraciones de variables dsp de declaraciones de metodos
        if (strcmp(inst->instruccion, "LABEL") == 0) {
            break;
        }

        // guardar referencia al LOAD anterior para el siguiente ASSIGN
        if (strcmp(inst->instruccion, "LOAD") == 0) {
            load_anterior = inst;
        }

        if (strcmp(inst->instruccion, "ASSIGN") == 0) {
            if (inst->resultado && inst->resultado->name) {
                // buscar el valor del LOAD correspondiente
                if (load_anterior && 
                    load_anterior->resultado && 
                    inst->arg1 && 
                    strcmp(load_anterior->resultado->name, inst->arg1->name) == 0) {
                    // el valor viene del LOAD anterior
                    agregar_variable_global(inst->resultado->name, load_anterior->arg1->name);
                } else {
                    // valor por defecto si no encontramos el LOAD
                    agregar_variable_global(inst->resultado->name, "0");
                }
            }
        }

        inst = inst->siguiente;
    }
}

void generar_seccion_data(FILE *out) {
    if (variables_globales == NULL || out == NULL) return;

    fprintf(out, "\t.data\n");
    var_global *actual = variables_globales;

    while (actual != NULL) {
        fprintf(out, "%s:\n", actual->nombre);
        
        // convertir valores booleanos a enteros
        if (strcmp(actual->valor_inicial, "true") == 0) {
            fprintf(out, "    .quad 1\n");
        } else if (strcmp(actual->valor_inicial, "false") == 0) {
            fprintf(out, "    .quad 0\n");
        } else {
            // sino se usa tal cual
            fprintf(out, "    .quad %s\n", actual->valor_inicial);
        }
        
        actual = actual->siguiente;
    }
    fprintf(out, "\n");
}

void generar_seccion_text(FILE *out) {
    if (out == NULL) return;

    fprintf(out, "\t.text\n");
    fprintf(out, "\t.globl\tmain\n");
    fprintf(out, "\t.type\tmain, @function\n");
    fprintf(out, "\n");
}

void generar_epilogo_archivo(FILE *out) {
    if (out == NULL) return;

    fprintf(out, ".LFE0:\n");
    fprintf(out, "\t.size\tmain, .-main\n");
    fprintf(out, "\t.section\t.note.GNU-stack,\"\",@progbits\n");
}

int contar_locales_por_metodo(codigo3dir *label) {

    codigo3dir *p = label->siguiente;
    // arreglo simple para nombres únicos (no sofisticado): max 1024 variables por función
    const char *a[100];
    int var_locales = 0;

    while (p != NULL) {
        if (strcmp(p->instruccion, "END") == 0) {
            break; // si es END listo
        }

        if (strcmp(p->instruccion, "ASSIGN") == 0) {
            if (p->resultado->esTemporal == 0) { // si es una variable real no temporal
                int i; 
                bool found = false;
                for (i = 0; i < var_locales; ++i) {
                    if (strcmp(a[i], p->resultado->name) == 0) {  // entra aca si ya estaba la variable en el arreglo
                        found = true; 
                        break; 
                    }
                }
                if (!found) {
                    a[var_locales] = p->resultado->name;
                    var_locales++;
                }
            }
        }

        p = p->siguiente;
    }

    return var_locales;
}

void generar_codigo_assembler(codigo3dir *programa, FILE *out) {
    if (!out || !programa) return;

    recolectar_variables_globales(programa);
    generar_seccion_data(out);
    generar_seccion_text(out);

    // saltar las variables globales para empezar con los metodos
    // asi no vemos 2 veces las variables globales, pq ya se ven en la seccion data
    codigo3dir *inst = programa;
    while (inst != NULL && strcmp(inst->instruccion, "LABEL") != 0) {
        inst = inst->siguiente;
    }

    while (inst != NULL) {
        char *instr = inst->instruccion;

        if (strcmp(instr, "LABEL") == 0) {

            printf("LABEL=numero de parametros del metodo %s: %d\n", inst->resultado->name, inst->resultado->num_parametros);

            crear_mapeo_variables_locales(inst);

            fprintf(out, "%s:\n", inst->resultado->name);
            fprintf(out, "    pushq %%rbp\n");
            fprintf(out, "    movq %%rsp, %%rbp\n");

            locales = contar_locales_por_metodo(inst);
            int N = 8 * locales; // reservar espacio para variables locales (8 bytes c/u)
            if (N > 0) {
                fprintf(out, "    subq $%d, %%rsp\n", N);
            }
        }
        else if (strcmp(instr, "END") == 0) {
            fprintf(out, "    movq %%rbp, %%rsp\n");
            fprintf(out, "    popq %%rbp\n");
            fprintf(out, "    ret\n");

            locales = 0;
            num_params_actual = 0;
        }
        else if (strcmp(instr, "LOAD") == 0) {
            // resultado = temporal destino (T0, T1, etc)
            // arg1 = lo que cargo (número o variable)
            
            char *dest = obtener_ubicacion_vars_locales(inst->resultado);
            
            // CASO 1: Cargar número
            if (inst->arg1->tipo_token == T_DIGIT) {
                fprintf(out, "    movq $%d, %s\n", inst->arg1->nro, dest);
            }
            // CASO 2: Cargar booleano
            else if (inst->arg1->tipo_token == T_VTRUE || inst->arg1->tipo_token == T_VFALSE) {
                int val = (strcmp(inst->arg1->bool_string, "true") == 0) ? 1 : 0;
                fprintf(out, "    movq $%d, %s\n", val, dest);
            }
            // CASO 3: Cargar variable (p, r, qqw, etc)
            else if (inst->arg1->tipo_token == T_ID) {
                char *src = obtener_ubicacion_vars_locales(inst->arg1);
                fprintf(out, "    movq %s, %rax\n", src);
                fprintf(out, "    movq %rax, %s\n", dest);
            }
        }

        else if (strcmp(instr, "ASSIGN") == 0) {

            if (inst->resultado && inst->arg1) {
                // Obtener ubicación del temporal origen (arg1)
                char *src = obtener_ubicacion_vars_locales(inst->arg1);
                
                // Obtener ubicación de la variable destino (resultado)
                char *dest;
                if (es_variable_global(inst->resultado->name)) {
                    dest = malloc(32);
                    sprintf(dest, "%s(%%rip)", inst->resultado->name);
                } else {
                    dest = obtener_ubicacion_vars_locales(inst->resultado);
                }
                
                if (src[0] == '%' && dest[0] != '%') {
                    fprintf(out, "    movq %s, %s\n", src, dest);  // Directo reg->mem
                } else {
                    fprintf(out, "    movq %s, %rax\n", src);
                    fprintf(out, "    movq %rax, %s\n", dest);
                }

            }

        } else {
            // instrucción no manejada: la imprimimos como comentario para debug
            fprintf(out, "    # instruccion no traducida: %s\n", instr);
        }

        inst = inst->siguiente; // avanzar en la lista
    }

    generar_epilogo_archivo(out);
}
