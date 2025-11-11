#include <string.h>
#include "generador.h"
#include "auxiliares.h"

// crea un nuevo temporal como info
info* obtener_temp(int n, tipo_info ti) {
    if (n < 0) return NULL;

    info *t = (info*)malloc(sizeof(info));
    if (!t) return NULL;
    memset(t, 0, sizeof(info));
    char buf[16];
    snprintf(buf, sizeof(buf), "T%d", n);
    t->name = strdup(buf);
    t->esTemporal = 1;
    t->tipo_info = ti;           
    t->tipo_token = T_ID;
    return t;
}

// crea una nueva constante entera (literal) cono info
info* crear_constante(int nro) { 
    info *c = (info*)malloc(sizeof(info));
    if (!c) return NULL;
    memset(c, 0, sizeof(info)); 
    char buf[16];
    snprintf(buf, sizeof(buf), "%d", nro);
    c->name = strdup(buf);
    c->nro = nro;
    c->esTemporal = 0;
    c->tipo_info = TIPO_INTEGER;            
    c->tipo_token = T_DIGIT;                
    return c;
}

// crea un nuevo label como info
info* obtener_label(const char *label) { 
    info *lbl = (info*)malloc(sizeof(info));
    if (!lbl) return NULL;
    memset(lbl, 0, sizeof(info));  
    lbl->name = strdup(label);
    lbl->esTemporal = 0;
    return lbl;
}