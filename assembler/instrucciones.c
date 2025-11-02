#include <string.h>
#include <stdlib.h>
#include "instrucciones.h"

// registros de parametros convencion
static const char *PARAM_REGS[6] = {"%rdi", "%rsi", "%rdx", "%rcx", "%r8", "%r9"};

// genera un label para un metodo o para control de flujo
void generar_label(FILE *out, codigo3dir *inst) {
    if (inst->resultado->tipo_token == T_METHOD_DECL) {
        push_metodo(inst->resultado->name, inst->resultado->tipo_info);
        crear_mapeo_variables_locales(inst);
        generar_prologo_metodo(out, inst->resultado->name);
    } else {
        fprintf(out, "%s:\n", inst->resultado->name);
    }
}

// genera el epilogo de un metodo y hace pop del mismo
void generar_end(FILE *out, codigo3dir *inst) {
    metodo_info *metodo = get_metodo_actual();
    if (metodo && metodo->tipo_retorno == TIPO_VOID) {
        generar_epilogo_metodo(out);
    }
    pop_metodo();
}

// genera la carga de un parametro en su variable local correspondiente
void generar_load_param(FILE *out, codigo3dir *inst) {
    int param_idx = inst->arg1->nro;
    char *var_dest = obtener_ubicacion_operando(inst->resultado);
    
    if (param_idx < 6) {
        fprintf(out, "    movq %s, %s\n", PARAM_REGS[param_idx], var_dest);
    } else {
        int stack_offset = 16 + (param_idx - 6) * 8;
        fprintf(out, "    movq %d(%%rbp), %%rax\n", stack_offset);
        fprintf(out, "    movq %%rax, %s\n", var_dest);
    }
    free(var_dest);
}

// genera la inclusion de un parametro en la lista de parametros para una llamada
void generar_param(FILE *out, codigo3dir *inst) {
    agregar_param(inst->resultado);
}

// genera una llamada a un metodo con manejo de parametros y retorno
void generar_call(FILE *out, codigo3dir *inst) {
    int param_count = get_param_count();
    info **params_array = get_params_array();

    int stack_params = (param_count > 6) ? (param_count - 6) : 0;
    int stack_bytes = stack_params * 8;
    bool need_alignment = (stack_bytes % 16 != 0);
    
    if (need_alignment) {
        fprintf(out, "    subq $8, %%rsp\n");
    }
    
    for (int i = param_count - 1; i >= 6; i--) {
        char *src = obtener_ubicacion_operando(params_array[i]);
        fprintf(out, "    movq %s, %%rax\n", src);
        fprintf(out, "    pushq %%rax\n");
        free(src);
    }
    
    for (int i = 0; i < param_count && i < 6; i++) {
        char *src = obtener_ubicacion_operando(params_array[i]);
        fprintf(out, "    movq %s, %s\n", src, PARAM_REGS[i]);
        free(src);
    }
    
    fprintf(out, "    call %s\n", inst->arg1->name);
    
    int bytes_to_clean = stack_bytes + (need_alignment ? 8 : 0);
    if (bytes_to_clean > 0) {
        fprintf(out, "    addq $%d, %%rsp\n", bytes_to_clean);
    }

    if (inst->resultado) {
        char *dest = obtener_ubicacion_operando(inst->resultado);
        fprintf(out, "    movq %%rax, %s\n", dest);
        free(dest);
    }

    if (params_array) free(params_array);
    limpiar_params();
}

// genera una asignacion simple
void generar_assign(FILE *out, codigo3dir *inst) {
    if (!inst->resultado || !inst->arg1) return;
    
    char *src = obtener_ubicacion_operando(inst->arg1);
    char *dest = obtener_ubicacion_operando(inst->resultado);

    bool src_es_memoria = (src[0] != '$' && src[0] != '%');
    bool dest_es_memoria = (dest[0] != '$' && dest[0] != '%');

    if (src_es_memoria && dest_es_memoria) {
        fprintf(out, "    movq %s, %%rax\n", src);
        fprintf(out, "    movq %%rax, %s\n", dest);
    } else {
        fprintf(out, "    movq %s, %s\n", src, dest);
    }

    free(src);
    free(dest);
}

// genera operaciones binarias: ADD, SUB, AND, OR
void generar_operacion_binaria(FILE *out, codigo3dir *inst) {
    char *src1 = obtener_ubicacion_operando(inst->arg1);
    char *src2 = obtener_ubicacion_operando(inst->arg2);
    char *dest = obtener_ubicacion_operando(inst->resultado);

    fprintf(out, "    movq %s, %%rax\n", src1);

    char *instr = inst->instruccion;
    if (strcmp(instr, "ADD") == 0) {
        fprintf(out, "    addq %s, %%rax\n", src2);
    } else if (strcmp(instr, "SUB") == 0) {
        fprintf(out, "    subq %s, %%rax\n", src2);
    } else if (strcmp(instr, "AND") == 0) {
        fprintf(out, "    andq %s, %%rax\n", src2);
    } else if (strcmp(instr, "OR") == 0) {
        fprintf(out, "    orq %s, %%rax\n", src2);
    }

    fprintf(out, "    movq %%rax, %s\n", dest);
    
    free(src1);
    free(src2);
    free(dest);
}

// genera multiplicacion
void generar_multiplicacion(FILE *out, codigo3dir *inst) {
    char *src1 = obtener_ubicacion_operando(inst->arg1);
    char *src2 = obtener_ubicacion_operando(inst->arg2);
    char *dest = obtener_ubicacion_operando(inst->resultado);
    
    fprintf(out, "    movq %s, %%rax\n", src1);
    fprintf(out, "    imulq %s, %%rax\n", src2);
    fprintf(out, "    movq %%rax, %s\n", dest);
    
    free(src1);
    free(src2);
    free(dest);
}

// genera division o modulo
void generar_division_modulo(FILE *out, codigo3dir *inst) {
    char *src1 = obtener_ubicacion_operando(inst->arg1);
    char *src2 = obtener_ubicacion_operando(inst->arg2);
    char *dest = obtener_ubicacion_operando(inst->resultado);
    
    fprintf(out, "    movq %s, %%rax\n", src1);
    fprintf(out, "    cqto\n");
    fprintf(out, "    movq %s, %%r11\n", src2);
    fprintf(out, "    idivq %%r11\n");
    
    if (strcmp(inst->instruccion, "DIV") == 0) {
        fprintf(out, "    movq %%rax, %s\n", dest);
    } else {
        fprintf(out, "    movq %%rdx, %s\n", dest);
    }
    
    free(src1);
    free(src2);
    free(dest);
}

// genera negacion aritmetica
void generar_negacion(FILE *out, codigo3dir *inst) {
    char *src = obtener_ubicacion_operando(inst->arg1);
    char *dest = obtener_ubicacion_operando(inst->resultado);
    
    fprintf(out, "    movq %s, %%rax\n", src);
    fprintf(out, "    negq %%rax\n");
    fprintf(out, "    movq %%rax, %s\n", dest);
    
    free(src);
    free(dest);
}

// genera negacion logica
void generar_not(FILE *out, codigo3dir *inst) {
    char *src = obtener_ubicacion_operando(inst->arg1);
    char *dest = obtener_ubicacion_operando(inst->resultado);
    
    fprintf(out, "    movq %s, %%rax\n", src);
    fprintf(out, "    xorq $1, %%rax\n");
    fprintf(out, "    movq %%rax, %s\n", dest);
    
    free(src);
    free(dest);
}

// genera comparacion: EQ, GT, LT
void generar_comparacion(FILE *out, codigo3dir *inst) {
    char *src1 = obtener_ubicacion_operando(inst->arg1);
    char *src2 = obtener_ubicacion_operando(inst->arg2);
    char *dest = obtener_ubicacion_operando(inst->resultado);

    fprintf(out, "    movq %s, %%rax\n", src1);
    fprintf(out, "    cmpq %s, %%rax\n", src2);

    char *instr = inst->instruccion;
    if (strcmp(instr, "EQ") == 0) {
        fprintf(out, "    sete %%al\n");
    } else if (strcmp(instr, "GT") == 0) {
        fprintf(out, "    setg %%al\n");
    } else if (strcmp(instr, "LT") == 0) {
        fprintf(out, "    setl %%al\n");
    }
    
    fprintf(out, "    movzbl %%al, %%eax\n");
    fprintf(out, "    movq %%rax, %s\n", dest);
    
    free(src1);
    free(src2);
    free(dest);
}

// genera return de un metodo, con manejo de valor de retorno
void generar_return(FILE *out, codigo3dir *inst) {
    metodo_info *metodo = get_metodo_actual();

    if (inst->resultado) {
        char *src = obtener_ubicacion_operando(inst->resultado);
        fprintf(out, "    movq %s, %%rax\n", src);
        free(src);
    }
    
    if (metodo && metodo->tipo_retorno != TIPO_VOID) {
        generar_epilogo_metodo(out);
    }
}

// genera un salto incondicional
void generar_goto(FILE *out, codigo3dir *inst) {
    fprintf(out, "    jmp %s\n", inst->resultado->name);
}

// genera un salto condicional si es false
void generar_if_false(FILE *out, codigo3dir *inst) {
    char *cond_src = obtener_ubicacion_operando(inst->arg1);
    
    fprintf(out, "    movq %s, %%rax\n", cond_src);
    fprintf(out, "    cmpq $0, %%rax\n");
    fprintf(out, "    je %s\n", inst->resultado->name);
    
    free(cond_src);
}