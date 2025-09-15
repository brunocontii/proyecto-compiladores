%{

#include <stdlib.h>
#include <string.h>
#include <stdio.h>

#include "parser.tab.h"

void init();

%}

%option noyywrap
%option yylineno

alpha [a-zA-Z]
digit [0-9]
alpha_num {alpha}|{digit}|"_"

%%

[ \t\r\n]+ ; 					// Ignora tab espacios y salto de línea

"//".*  						// Comentario de línea

"/*"([^*]|\*+[^*/])*\*+"/" 		/* Comentario de bloque de línea */

"program"   { printf("program : %s\n",yytext); return TOKEN_PROGRAM; }

"integer"	{ printf("integer : %s\n",yytext); return TOKEN_INTEGER; }

"bool" 	    { printf("bool : %s\n",yytext); return TOKEN_BOOL; }

"void" 	    { printf("void : %s\n",yytext); return TOKEN_VOID; }

"extern"    { printf("extern : %s\n",yytext); return TOKEN_EXTERN; }

"return"	{ printf("return : %s\n",yytext); return TOKEN_RETURN; }

"if"	    { printf("if : %s\n",yytext); return TOKEN_IF; }

"else"	    { printf("else : %s\n",yytext); return TOKEN_ELSE; }

"then"	    { printf("then : %s\n",yytext); return TOKEN_THEN; }

"while"	    { printf("while : %s\n",yytext); return TOKEN_WHILE; }

"true"	    { printf("true : %s\n",yytext); return TOKEN_VTRUE; }

"false"	    { printf("false : %s\n",yytext); return TOKEN_VFALSE; }

"&&"	    { printf("&& : %s\n",yytext); return TOKEN_OP_AND; }

"||"	    { printf("|| : %s\n",yytext); return TOKEN_OP_OR; }

"!"	        { printf("! : %s\n",yytext); return TOKEN_OP_NOT; }

{alpha}({alpha_num})*		{ 
								printf("ID : %s\n",yytext);
								yylval.var = strdup(yytext); // para copiar el valor de yytext
								return TOKEN_ID;
							}

{digit}+ 	{ 
				printf("DIGIT : %s\n",yytext);
				yylval.dig = atoi(yytext); // para convertir yytext en un int
				return TOKEN_DIGIT;
			}	

"-"	    { printf("- : %s\n",yytext); return TOKEN_OP_MENOS; }

"+"	    { printf("+ : %s\n",yytext); return TOKEN_OP_MAS; }

"*"	    { printf("* : %s\n",yytext); return TOKEN_OP_MULT; }

"/"	    { printf("/ : %s\n",yytext); return TOKEN_OP_DIV; }

"%"	    { printf("mod : %s\n",yytext); return TOKEN_OP_RESTO; }

","	    { printf("; : %s\n",yytext); return TOKEN_COMA; }

";"	    { printf("; : %s\n",yytext); return TOKEN_PYC; }

"("	    { printf("( : %s\n",yytext); return TOKEN_PAR_A; }

")"	    { printf(") : %s\n",yytext); return TOKEN_PAR_C; }

"{"	    { printf("{ : %s\n",yytext); return TOKEN_LLA_A; }

"}"	    { printf("} : %s\n",yytext); return TOKEN_LLA_C; }

"="	    { printf("= : %s\n",yytext); return TOKEN_ASIGNACION; }

"=="	{ printf("== : %s\n",yytext); return TOKEN_IGUALDAD; }

"<"	    { printf("< : %s\n",yytext); return TOKEN_MENOR; }

">"	    { printf("> : %s\n",yytext); return TOKEN_MAYOR; }

.   	{
			printf("Error léxico: caracter no válido '%C'\n", yytext[0]);
		  	exit(EXIT_FAILURE);
		} /* Ignora todo el resto */

%%

int main(int argc,char *argv[]){
	++argv,--argc;
	if (argc > 0)
		yyin = fopen(argv[0],"r");
	else
		yyin = stdin;

	init();
	yyparse();

}
