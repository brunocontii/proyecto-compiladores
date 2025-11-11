#include <stdio.h>
#include <stdbool.h>
#include "optimizaciones.h"
#include "plegado_constantes.h"
#include "operaciones.h"
#include "codigo_muerto.h"
#include "codigo_muerto_var.h"
#include "codigo_muerto_bloque.h"

// variable externa para saber si la optimizacion fue activada
extern bool opt_constant_folding;
extern bool opt_codigo_muerto_var;
extern bool opt_codigo_muerto_codigo_inalcanzable;
extern bool opt_operaciones;
extern bool opt_codigo_muerto_bloque;

// aplicar todas las optimizaciones habilitadas
void aplicar_optimizaciones(nodo *raiz) {
    if (!raiz) return;

    // primero aplicar propagacion de constantes. se ejecuta primer
    // porque crea oportunidades para otras optimizaciones
    if (opt_constant_folding) {
        propagacion_constantes(raiz);
    }

    // luego aplicar optimizaciones de operaciones. despues de 
    // propagar constantes, podemos simplificar operaciones
    if (opt_operaciones) {
        optimizaciones_operaciones(raiz);
    }

    // las optimizaciones anteriores pueden crear codigo muerto. por ej
    // if (true), while(false), etc
    if (opt_codigo_muerto_codigo_inalcanzable) {
        eliminarCodigoMuerto(raiz);
    }

    // eliminar codigo muerto en bloques
    if (opt_codigo_muerto_bloque) {
        eliminarBloquesMuertos(raiz);
    }
    
    // por ultimo, eliminar variables muertas. optimizaciones previas
    // pueden hacer que variables queden sin usar
    if (opt_codigo_muerto_var) {
        codigo_muerto_var(raiz);
    }
}