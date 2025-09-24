%{

#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <stdarg.h>
#include "../arbol-sintactico/arbol.h"
#include "../tabla-simbolos/tabla_simbolos.h"
#include "../utils/manejo_errores.h"

extern int yylineno;
void yyerror();

nodo* raiz = NULL;
tabla_simbolos *ts = NULL;
bool es_metodo = true;

%}

%code requires {
    #include "../arbol-sintactico/arbol.h"
}

%union {
    int dig;
    char *var;
    nodo* nodo;
}

%token TOKEN_PROGRAM TOKEN_EXTERN TOKEN_RETURN
%token TOKEN_IF TOKEN_ELSE TOKEN_THEN TOKEN_WHILE
%token TOKEN_VTRUE TOKEN_VFALSE TOKEN_OP_AND TOKEN_OP_OR TOKEN_OP_NOT
%token TOKEN_INTEGER TOKEN_BOOL TOKEN_VOID
%token <var> TOKEN_ID
%token <dig> TOKEN_DIGIT
%token TOKEN_OP_MENOS TOKEN_OP_MAS TOKEN_OP_MULT TOKEN_OP_DIV TOKEN_OP_RESTO
%token TOKEN_ASIGNACION TOKEN_IGUALDAD TOKEN_MENOR TOKEN_MAYOR
%token TOKEN_PYC TOKEN_COMA TOKEN_PAR_A TOKEN_PAR_C TOKEN_LLA_A TOKEN_LLA_C 

%right TOKEN_ASIGNACION 
%left TOKEN_OP_OR
%left TOKEN_OP_AND
%left TOKEN_IGUALDAD
%left TOKEN_MENOR TOKEN_MAYOR
%left TOKEN_OP_MAS TOKEN_OP_MENOS
%left TOKEN_OP_MULT TOKEN_OP_DIV TOKEN_OP_RESTO
%right TOKEN_OP_NOT MENOS_UNARIO

%type <nodo> prog var_decls var_decl method_decls method_decl expr block_or_extern
%type <nodo> block parametros parametro type bool_literal integer_literal literal
%type <nodo> method_call exprs statements statement

%%

prog: TOKEN_PROGRAM TOKEN_LLA_A TOKEN_LLA_C
      {
        chequear_errores();
        $$ = NULL;
      }
    | TOKEN_PROGRAM TOKEN_LLA_A var_decls TOKEN_LLA_C
      {
        chequear_errores();
        info *ninfo = malloc(sizeof(info));
        ninfo->name = strdup("PROGRAM");
        ninfo->tipo_token = T_PROGRAM;

        $$ = crearArbol(ninfo, $3, NULL);
        raiz = $$;
      }
    | TOKEN_PROGRAM TOKEN_LLA_A method_decls TOKEN_LLA_C
      {
        chequear_errores();
        info *ninfo = malloc(sizeof(info));
        ninfo->name = strdup("PROGRAM");
        ninfo->tipo_token = T_PROGRAM;

        $$ = crearArbol(ninfo, NULL, $3);
        raiz = $$;
      }
    | TOKEN_PROGRAM TOKEN_LLA_A var_decls method_decls TOKEN_LLA_C
      {
        chequear_errores();
        info *ninfo = malloc(sizeof(info));
        ninfo->name = strdup("PROGRAM");
        ninfo->tipo_token = T_PROGRAM;

        $$ = crearArbol(ninfo, $3, $4);
        raiz = $$;
      }
    ;

var_decls: var_decls var_decl
          {
            info *ninfo = malloc(sizeof(info));
            ninfo->name = strdup("DECLARACIONES VARIBLES");
            ninfo->tipo_token = T_VAR_DECLS;

            $$ = crearArbol(ninfo, $1, $2);
          }
         | var_decl
          {
            $$ = $1;
          }
         ;

method_decls: method_decls method_decl
              {
                info *ninfo = malloc(sizeof(info));
                ninfo->name = strdup("DECLARACIONES METODOS");
                ninfo->tipo_token = T_METHOD_DECLS;

                $$ = crearArbol(ninfo, $1, $2);
              }
            | method_decl
              {
                $$ = $1;
              }
            ;

var_decl: type TOKEN_ID TOKEN_ASIGNACION expr TOKEN_PYC
          {
            info *ninfo = malloc(sizeof(info));
            ninfo->name = strdup("DECLARACION VARIABLE");
            ninfo->op = strdup("=");
            ninfo->tipo_token = T_VAR_DECL;

            info *ninfovar = malloc(sizeof(info));
            ninfovar->name = strdup($2);
            ninfovar->tipo_token = T_ID;
            ninfovar->tipo_info = $1->valor->tipo_info; //pasando el tipo a el nodo a crear
            nodo *id_nodo = crearNodo(ninfovar);

            if (!insertar(ts, ninfovar)) {
              // redeclaraciones
              reportar_error("Variable '%s' ya declarada\n", $2);
            }

            $$ = crearArbol(ninfo, id_nodo, $4);
          }
        ;

method_decl: type TOKEN_ID TOKEN_PAR_A 
            {
              // insertar metodo en scope global
              info *ninfo = malloc(sizeof(info));
              ninfo->name = strdup($2);
              ninfo->tipo_token = T_METHOD_DECL;
              ninfo->tipo_info = $1->valor->tipo_info;

              if (!insertar(ts, ninfo)) {
                reportar_error("Metodo '%s' ya declarado\n", $2);
              }

              // abrir scope antes de procesar parametros
              abrir_scope(ts);
            }
            parametros TOKEN_PAR_C block_or_extern
            {
              printf("\n--- SCOPE DEL MÉTODO '%s' ANTES DE CERRAR ---", $2);
              imprimir_scope_actual(ts);

              // crear el ast del metodo
              info *method_info = malloc(sizeof(info));
              method_info->name = strdup($2);
              method_info->tipo_token = T_METHOD_DECL;
              method_info->tipo_info = $1->valor->tipo_info;

              $$ = crearArbol(method_info, $5, $7);
              
              // cerrar scope del metodo
              cerrar_scope(ts);
            }
           | TOKEN_VOID TOKEN_ID TOKEN_PAR_A 
            {
              // insertar metodo void en scope global
              info *ninfo = malloc(sizeof(info));
              ninfo->name = strdup($2);
              ninfo->tipo_token = T_METHOD_DECL;
              ninfo->tipo_info = TIPO_VOID;

              if (!insertar(ts, ninfo)) {
                reportar_error("Metodo '%s' ya declarado\n", $2);
              }
              
              // abrir scope antes de procesar parametros
              abrir_scope(ts);
            }
            parametros TOKEN_PAR_C block_or_extern
            {
              printf("\n--- SCOPE DEL MÉTODO '%s' ANTES DE CERRAR ---", $2);
              imprimir_scope_actual(ts);

              // crear el asd del metodo
              info *method_info = malloc(sizeof(info));
              method_info->name = strdup($2);
              method_info->tipo_token = T_METHOD_DECL;
              method_info->tipo_info = TIPO_VOID;

              $$ = crearArbol(method_info, $5, $7);
              
              // cerrar scope del metodo
              cerrar_scope(ts);
            }
           ;

block_or_extern: block
                  {
                    $$ = $1;
                  }
                | TOKEN_EXTERN TOKEN_PYC
                  {
                    info *ninfo = malloc(sizeof(info));
                    ninfo->name = strdup("EXTERN");
                    ninfo->tipo_token = T_EXTERN;

                    $$ = crearNodo(ninfo);
                  }
               ;

parametros: parametros TOKEN_COMA parametro
            {
              info *ninfo = malloc(sizeof(info));
              ninfo->name = strdup("LISTA PARAMETROS");
              ninfo->tipo_token = T_PARAMETROS;

              $$ = crearArbol(ninfo, $1, $3);
            }
          | parametro
            {
              $$ = $1;
            }
          |
            {
              $$ = NULL;
            }
          ;

parametro: type TOKEN_ID
          {
            info *ninfo = malloc(sizeof(info));
            ninfo->name = strdup($2);
            ninfo->tipo_info = $1->valor->tipo_info;
            ninfo->tipo_token = T_PARAMETRO;

            // insertar parametro en el scope actual
            if (!insertar(ts, ninfo)) {
              reportar_error("Parametro '%s' ya declarado\n", $2);
            }

            $$ = crearNodo(ninfo);
          }
         ;

block: TOKEN_LLA_A
      {
        if(!es_metodo){
          abrir_scope(ts);
        } else {
          es_metodo = false;
        }
      }
      var_decls statements TOKEN_LLA_C
      {
        printf("\n--- SCOPE DE BLOQUE ANTES DE CERRAR ---");
        imprimir_scope_actual(ts);

        info *ninfo = malloc(sizeof(info));
        ninfo->name = strdup("BLOQUE CON DECLS DE VARS Y METS");
        ninfo->tipo_token = T_BLOQUE;

        $$ = crearArbol(ninfo, $3, $4);

        if(!es_metodo) {
          cerrar_scope(ts);
        } else {
          es_metodo = false;
        }
      }
     | TOKEN_LLA_A
      {
        if(!es_metodo){
          abrir_scope(ts);
        } else {
          es_metodo = false;
        }
      }
      statements TOKEN_LLA_C
      {
        printf("\n--- SCOPE DE BLOQUE ANTES DE CERRAR ---");
        imprimir_scope_actual(ts);
        
        info *ninfo = malloc(sizeof(info));
        ninfo->name = strdup("BLOQUE CON DECLS DE METS");
        ninfo->tipo_token = T_BLOQUE;

        $$ = crearArbol(ninfo, $3, NULL);

        if(!es_metodo) {
          cerrar_scope(ts);
        } else {
          es_metodo = false;
        }
      }
     ;

type: TOKEN_INTEGER
      {
        info *ninfo = malloc(sizeof(info));
        ninfo->name = strdup("INTEGER");
        ninfo->tipo_info = TIPO_INTEGER;
        ninfo->tipo_token = T_INTEGER;

        $$ = crearNodo(ninfo);
      }
      | TOKEN_BOOL
        {
          info *ninfo = malloc(sizeof(info));
          ninfo->name = strdup("BOOL");
          ninfo->tipo_info = TIPO_BOOL;
          ninfo->tipo_token = T_BOOL;
          
          $$ = crearNodo(ninfo);
        }
    ;

statements: statements statement
            {
              info *ninfo = malloc(sizeof(info));
              ninfo->name = strdup("STATEMENTS");
              ninfo->tipo_token = T_STATEMENTS;

              $$ = crearArbol(ninfo, $1, $2);
            }
            |
              {
                $$ = NULL;
              }
          ;

statement: TOKEN_ID TOKEN_ASIGNACION expr TOKEN_PYC
            {
              info *ninfo = malloc(sizeof(info));
              ninfo->name = strdup("ASIGNACION");
              ninfo->op = strdup("=");
              ninfo->tipo_token = T_ASIGNACION;

              info *ninfovar = malloc(sizeof(info));
              ninfovar->name = strdup($1);
              ninfovar->tipo_token = T_ID;
              nodo *id_nodo = crearNodo(ninfovar);

              $$ = crearArbol(ninfo, id_nodo, $3);
            }
           | method_call TOKEN_PYC
            {
              $$ = $1; 
            }
           | TOKEN_IF TOKEN_PAR_A expr TOKEN_PAR_C TOKEN_THEN block
            {
              info *ninfo = malloc(sizeof(info));
              ninfo->name = strdup("IF");
              ninfo->tipo_token = T_IF;

              $$ = crearArbol(ninfo, $3, $6);
            }
           | TOKEN_IF TOKEN_PAR_A expr TOKEN_PAR_C TOKEN_THEN block TOKEN_ELSE block
            {
              info *ninfo = malloc(sizeof(info));
              ninfo->name = strdup("IF_ELSE");
              ninfo->tipo_token = T_IF_ELSE;

              $$ = crearArbolTer(ninfo, $3, $6, $8);
            }
           | TOKEN_WHILE expr block
            {
              info *ninfo = malloc(sizeof(info));
              ninfo->name = strdup("WHILE");
              ninfo->tipo_token = T_WHILE;

              $$ = crearArbol(ninfo, $2, $3);
            }
           | TOKEN_RETURN expr TOKEN_PYC
            {
              info *ninfo = malloc(sizeof(info));
              ninfo->name = strdup("RETURN");
              ninfo->tipo_token = T_RETURN;

              $$ = crearArbol(ninfo, $2, NULL);
            }
           | TOKEN_RETURN TOKEN_PYC
            {
              info *ninfo = malloc(sizeof(info));
              ninfo->name = strdup("RETURN");
              ninfo->tipo_token = T_RETURN;

              $$ = crearNodo(ninfo);
            }
           | TOKEN_PYC
            {
              $$ = NULL;
            }
           | block
            {
              $$ = $1;
            }
         ;

method_call: TOKEN_ID TOKEN_PAR_A exprs TOKEN_PAR_C
            {
              info *ninfo = malloc(sizeof(info));
              ninfo->name = strdup("LLAMADA A METODO");
              ninfo->tipo_token = T_METHOD_CALL;

              info *ninfovar = malloc(sizeof(info));
              ninfovar->name = strdup($1);
              ninfovar->tipo_token = T_ID;
              nodo *id_nodo = crearNodo(ninfovar);

              $$ = crearArbol(ninfo, id_nodo, $3);
            }
           ;

exprs: exprs TOKEN_COMA expr
      {
        info *ninfo = malloc(sizeof(info));
        ninfo->name = strdup("LISTA DE EXPRESIONES");
        ninfo->tipo_token = T_EXPRS;

        $$ = crearArbol(ninfo, $1, $3);
      }
       | expr
        {
          $$ = $1;
        }
       |
        {
          $$ = NULL;
        }
     ;

expr: TOKEN_ID
        {
          info *ninfo = malloc(sizeof(info));
          ninfo->name = strdup($1);
          ninfo->tipo_token = T_ID;

          $$ = crearNodo(ninfo);
        }
      | method_call
        {
          $$ = $1;
        }
      | literal
        {
          $$ = $1;
        }
      | expr TOKEN_OP_MAS expr
        {
          info *op_info = malloc(sizeof(info));
          op_info->op = strdup("+");
          op_info->tipo_token = T_OP_MAS;
          op_info->tipo_info = TIPO_INTEGER;

          $$ = crearArbol(op_info, $1, $3);
        }
      | expr TOKEN_OP_MENOS expr
        {
          info *op_info = malloc(sizeof(info));
          op_info->op = strdup("-");
          op_info->tipo_token = T_OP_MENOS;
          op_info->tipo_info = TIPO_INTEGER;

          $$ = crearArbol(op_info, $1, $3);
        }
      | expr TOKEN_OP_MULT expr
        {
          info *op_info = malloc(sizeof(info));
          op_info->op = strdup("*");
          op_info->tipo_token = T_OP_MULT;
          op_info->tipo_info = TIPO_INTEGER;

          $$ = crearArbol(op_info, $1, $3);
        }
      | expr TOKEN_OP_DIV expr
        {
          info *op_info = malloc(sizeof(info));
          op_info->op = strdup("/");
          op_info->tipo_token = T_OP_DIV;
          op_info->tipo_info = TIPO_INTEGER;

          $$ = crearArbol(op_info, $1, $3);
        }
      | expr TOKEN_OP_RESTO expr
        {
          info *op_info = malloc(sizeof(info));
          op_info->op = strdup("%");
          op_info->tipo_token = T_OP_RESTO;
          op_info->tipo_info = TIPO_INTEGER;

          $$ = crearArbol(op_info, $1, $3);
        }
      | expr TOKEN_OP_AND expr
        {
          info *op_info = malloc(sizeof(info));
          op_info->op = strdup("&&");
          op_info->tipo_token = T_OP_AND;
          op_info->tipo_info = TIPO_BOOL;

          $$ = crearArbol(op_info, $1, $3);
        }
      | expr TOKEN_OP_OR expr
        {
          info *op_info = malloc(sizeof(info));
          op_info->op = strdup("||");
          op_info->tipo_token = T_OP_OR;
          op_info->tipo_info = TIPO_BOOL;

          $$ = crearArbol(op_info, $1, $3);
        }
      | expr TOKEN_MENOR expr
        {
          info *op_info = malloc(sizeof(info));
          op_info->op = strdup("<");
          op_info->tipo_token = T_MENOR;
          op_info->tipo_info = TIPO_BOOL;

          $$ = crearArbol(op_info, $1, $3);
        }
      | expr TOKEN_MAYOR expr
        {
          info *op_info = malloc(sizeof(info));
          op_info->op = strdup(">");
          op_info->tipo_token = T_MAYOR;
          op_info->tipo_info = TIPO_BOOL;

          $$ = crearArbol(op_info, $1, $3);
        }
      | expr TOKEN_IGUALDAD expr
        {
          info *op_info = malloc(sizeof(info));
          op_info->op = strdup("==");
          op_info->tipo_token = T_IGUALDAD;
          op_info->tipo_info = TIPO_BOOL;

          $$ = crearArbol(op_info, $1, $3);
        }
      | TOKEN_OP_MENOS expr %prec MENOS_UNARIO
        {
          info *op_info = malloc(sizeof(info));
          op_info->op = strdup("-");
          op_info->tipo_token = T_OP_MENOS;
          op_info->tipo_info = TIPO_INTEGER;

          $$ = crearArbol(op_info, NULL, $2);
        }
      | TOKEN_OP_NOT expr
        {
          info *op_info = malloc(sizeof(info));
          op_info->op = strdup("!");
          op_info->tipo_token = T_OP_NOT;
          op_info->tipo_info = TIPO_BOOL;

          $$ = crearArbol(op_info, NULL, $2);
        }
      | TOKEN_PAR_A expr TOKEN_PAR_C
        {
          $$ = $2;
        }
    ;

literal: integer_literal
          {
            $$ = $1;
          }
        | bool_literal
          {
            $$ = $1;
          }
       ;

integer_literal: TOKEN_DIGIT
                {
                  info *digito = malloc(sizeof(info));
                  digito->nro = $1;
                  digito->name = strdup("digito");
                  digito->tipo_info = TIPO_INTEGER;
                  digito->tipo_token = T_DIGIT;

                  $$ = crearNodo(digito);
                }
              | integer_literal TOKEN_DIGIT
                {
                  info *literal = malloc(sizeof(info));
                  literal->name = strdup("literal");
                  literal->tipo_token = T_INTEGER;
                  literal->tipo_info = TIPO_INTEGER;
                  
                  info *digito = malloc(sizeof(info));
                  digito->nro = $2;
                  digito->name = strdup("digito");
                  digito->tipo_info = TIPO_INTEGER;
                  digito->tipo_token = T_DIGIT;
                  nodo *dig = crearNodo(digito);

                  $$ = crearArbol(literal, $1, dig);
                }
              ;

bool_literal: TOKEN_VTRUE
                {
                  info *booleano = malloc(sizeof(info));
                  booleano->bool_string = strdup("true");
                  booleano->b = true;
                  booleano->tipo_info = TIPO_BOOL;
                  booleano->tipo_token = T_VTRUE;

                  $$ = crearNodo(booleano);
                }
              | TOKEN_VFALSE
                {
                  info *booleano = malloc(sizeof(info));
                  booleano->bool_string = strdup("false");
                  booleano->b = false;
                  booleano->tipo_info = TIPO_BOOL;
                  booleano->tipo_token = T_VFALSE;

                  $$ = crearNodo(booleano);
                }
            ;

%%

void yyerror() {
  errors++;
  printf(COLOR_RED "-> ERROR Sintactico en la linea: %d \n" COLOR_RESET, yylineno);
}