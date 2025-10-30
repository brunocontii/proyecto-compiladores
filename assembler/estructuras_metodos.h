#ifndef ESTRUCTURAS_METODOS_H
#define ESTRUCTURAS_METODOS_H

#include "../codigo-intermedio/codigo3dir.h"


// estructuras compartidas en varios archivos

// estructura para almacenar variables globales
typedef struct var_global {
    char *nombre;
    char *valor_inicial;
    struct var_global *siguiente;
} var_global;

typedef struct var_offset {
    char *nombre;
    int offset;
    struct var_offset *siguiente;
} var_offset;

typedef struct metodo_info {
    char *nombre;
    int num_vars_locales;
    var_offset *mapeo_vars;
    struct metodo_info *siguiente;
    tipo_info tipo_retorno;
} metodo_info;

// todo variables
void agregar_variable_global(const char *nombre, const char *valor);
void recolectar_variables_globales(codigo3dir *programa);
bool es_variable_global(const char *nombre);
int obtener_offset_variable(const char *nombre);

// todo metodos
void push_metodo(const char *nombre, tipo_info tipo_ret);
void pop_metodo();
metodo_info* get_metodo_actual();
void crear_mapeo_variables_locales(codigo3dir *label);

// todo parametros
void agregar_param(info *param);
void limpiar_params();
int get_param_count();
info** get_params_array();

// operandos
char* obtener_ubicacion_operando(info *operando);

// secciones de distinto tipo
void generar_seccion_data(FILE *out);
void generar_seccion_text(FILE *out);
void generar_epilogo_archivo(FILE *out);
void crear_prologo_metodo(FILE *out, const char *nombre_metodo);
void crear_epilogo_metodo(FILE *out);

#endif