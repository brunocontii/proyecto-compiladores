%{
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
%}

%token TOKEN_PROGRAM TOKEN_EXTERN TOKEN_RETURN
%token TOKEN_IF TOKEN_ELSE TOKEN_THEN TOKEN_WHILE
%token TOKEN_VTRUE TOKEN_VFALSE TOKEN_OP_AND TOKEN_OP_OR TOKEN_OP_NOT
%token TOKEN_INTEGER TOKEN_BOOL TOKEN_VOID TOKEN_ID TOKEN_DIGIT 
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

%%

prog: TOKEN_PROGRAM TOKEN_LLA_A TOKEN_LLA_C
    | TOKEN_PROGRAM TOKEN_LLA_A var_decls TOKEN_LLA_C
    | TOKEN_PROGRAM TOKEN_LLA_A method_decls TOKEN_LLA_C
    | TOKEN_PROGRAM TOKEN_LLA_A var_decls method_decls TOKEN_LLA_C
    ;

var_decls: var_decls var_decl
         | var_decl
         ;

method_decls: method_decls method_decl
            | method_decl
            ;

var_decl: type TOKEN_ID TOKEN_ASIGNACION expr TOKEN_PYC
        ;

method_decl: type TOKEN_ID TOKEN_PAR_A parametros TOKEN_PAR_C block_or_extern
           | TOKEN_VOID TOKEN_ID TOKEN_PAR_A parametros TOKEN_PAR_C block_or_extern
           ;

block_or_extern: block
                | TOKEN_EXTERN TOKEN_PYC
               ;


parametros: parametros parametro 
            | /* lambda */  
          ;

parametro: type TOKEN_ID 
           | type TOKEN_ID TOKEN_COMA parametro
         ;

block: TOKEN_LLA_A var_decls statements TOKEN_LLA_C
     | TOKEN_LLA_A statements TOKEN_LLA_C
     ;

type: TOKEN_INTEGER
      | TOKEN_BOOL
    ;

statements: statements statement
            |
          ;

statement: TOKEN_ID TOKEN_ASIGNACION expr TOKEN_PYC
           | method_call TOKEN_PYC
           | TOKEN_IF TOKEN_PAR_A expr TOKEN_PAR_C TOKEN_THEN block
           | TOKEN_IF TOKEN_PAR_A expr TOKEN_PAR_C TOKEN_THEN block TOKEN_ELSE block
           | TOKEN_WHILE expr block
           | TOKEN_RETURN expr TOKEN_PYC
           | TOKEN_RETURN TOKEN_PYC
           | TOKEN_PYC
           | block
         ;

method_call: TOKEN_ID TOKEN_PAR_A exprs TOKEN_PAR_C
           ;

exprs: exprs TOKEN_COMA expr
       | expr
       | /* lambda */
     ;

expr: TOKEN_ID
      | method_call
      | literal
      | expr TOKEN_OP_MAS expr
      | expr TOKEN_OP_MENOS expr
      | expr TOKEN_OP_MULT expr
      | expr TOKEN_OP_DIV expr
      | expr TOKEN_OP_RESTO expr
      | expr TOKEN_OP_AND expr
      | expr TOKEN_OP_OR expr
      | expr TOKEN_OP_NOT expr
      | expr TOKEN_MENOR expr
      | expr TOKEN_MAYOR expr
      | expr TOKEN_IGUALDAD expr
      | TOKEN_OP_MENOS expr %prec MENOS_UNARIO
      | TOKEN_OP_NOT expr
      | TOKEN_PAR_A expr TOKEN_PAR_C
    ;

literal: integer_literal
        | bool_literal
       ;

integer_literal: TOKEN_DIGIT list_digit
               ;

list_digit: list_digit TOKEN_DIGIT 
            | /* lambda */

bool_literal: TOKEN_VTRUE
              | TOKEN_VFALSE
            ;

%%