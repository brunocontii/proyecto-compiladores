#ifndef ESTRUCTURAS_METODOS_H
#define ESTRUCTURAS_METODOS_H

#include <stdio.h>
#include <stdbool.h>
#include "../codigo-intermedio/codigo3dir.h"

// estructuras compartidas en varios archivos

// estructura para almacenar variables globales
typedef struct var_global {
    char *nombre;
    char *valor_inicial;
    struct var_global *siguiente;
} var_global;

// estructura para mapear variables locales, temporales y parametros a offsets en el stack
typedef struct var_offset {
    char *nombre;
    int offset;
    struct var_offset *siguiente;
} var_offset;

// estructura para almacenar informacion de metodos
typedef struct metodo_info {
    char *nombre;
    int num_vars_locales;
    var_offset *mapeo_vars;
    tipo_info tipo_retorno;
    struct metodo_info *siguiente;
} metodo_info;

// funciones y metodos relacionados a variables
void agregar_variable_global(const char *nombre, const char *valor);
void recolectar_variables_globales(codigo3dir *programa);
bool es_variable_global(const char *nombre);
int obtener_offset_variable(const char *nombre);

// funciones y metodos relacionados a metodos
void push_metodo(const char *nombre, tipo_info tipo_ret);
void pop_metodo();
metodo_info* get_metodo_actual();
void crear_mapeo_variables_locales(codigo3dir *label);

// funciones y metodos relacionados a parametros
void agregar_param(info *param);
void limpiar_params();
int get_param_count();
info** get_params_array();

// funcion para obtener la ubicacion de un operando (registro, offset, etiqueta, etc.)
char* obtener_ubicacion_operando(info *operando);

// metodos para generar secciones de distintos tipos
void generar_seccion_data(FILE *out);
void generar_seccion_text(FILE *out);
void generar_epilogo_archivo(FILE *out);
void generar_prologo_metodo(FILE *out, const char *nombre_metodo);
void generar_epilogo_metodo(FILE *out);

#endif