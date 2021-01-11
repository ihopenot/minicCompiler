%{
#include<string>
#include<cstdio>
#include "ASTnode.h"
#include "grammar.hpp"
#define SAVE_TOKEN (yylval.string = new std::string(yytext))
#define TOKEN(t) (yylval.token = t)

int column = 1;

#define YY_USER_ACTION {\
    yylloc.first_column = column;\
    column += yyleng;\
    yylloc.first_line = yylineno;\
}

#ifdef DEBUG
#define DBGPUTS(s) (puts(s))
#else
#define DBGPUTS(s) 
#endif
%}

%option yylineno
%option noyywrap

%%
"//".*                  ;
[ \t\r] 				;
"\n"                    column = 1;
"if"                    SAVE_TOKEN;DBGPUTS("T_IF"); return T_IF;
"else"                  SAVE_TOKEN;DBGPUTS("T_ELSE"); return T_ELSE;
"return"                SAVE_TOKEN;DBGPUTS("T_RETURN"); return T_RETURN;
"..."                   SAVE_TOKEN;DBGPUTS("T_VARG");   return T_VARG;
"while"                 SAVE_TOKEN;DBGPUTS("T_WHILE"); return T_WHILE;
"for"                   SAVE_TOKEN;DBGPUTS("T_FOR");    return T_FOR;
"int"                   SAVE_TOKEN;DBGPUTS("TY_INT");  return TY_INT;
"float"                 SAVE_TOKEN;DBGPUTS("TY_FLOAT"); return TY_FLOAT;
"string"                SAVE_TOKEN;DBGPUTS("TY_STR");   return TY_STR;
"bool"                  SAVE_TOKEN;DBGPUTS("TY_BOOL");  return TY_BOOL;
"void"                  SAVE_TOKEN;DBGPUTS("TY_VOID");  return TY_VOID;
[a-zA-Z_][a-zA-Z0-9_]*	SAVE_TOKEN;DBGPUTS("T_IDENTIFIER"); return T_IDENTIFIER;
[0-9]+\.[0-9]*			SAVE_TOKEN;DBGPUTS("T_CONSTFLOAT"); return T_CONSTFLOAT;
[0-9]+  				SAVE_TOKEN;DBGPUTS("T_CONSTINT"); return T_CONSTINT;
\"(\\.|[^"])*\"         SAVE_TOKEN;DBGPUTS("T_CONSTSTR"); return T_CONSTSTR;
"="						SAVE_TOKEN;DBGPUTS("T_ASSIGN"); return T_ASSIGN;
"=="					SAVE_TOKEN;DBGPUTS("T_EQ"); return T_EQ;
"!="                    SAVE_TOKEN;DBGPUTS("T_NEQ"); return T_NEQ;
"<"                     SAVE_TOKEN;DBGPUTS("T_LT"); return T_LT;
"<="                    SAVE_TOKEN;DBGPUTS("T_LE"); return T_LE;
">"                     SAVE_TOKEN;DBGPUTS("T_GT"); return T_GT;
">="                    SAVE_TOKEN;DBGPUTS("T_GE"); return T_GE;
"["                     SAVE_TOKEN;DBGPUTS("T_LB"); return T_LB;
"]"                     SAVE_TOKEN;DBGPUTS("T_RB"); return T_RB;
"("                     SAVE_TOKEN;DBGPUTS("T_LP"); return T_LP;
")"                     SAVE_TOKEN;DBGPUTS("T_RP"); return T_RP;
"+"                     SAVE_TOKEN;DBGPUTS("T_PLUS"); return T_PLUS;
"-"                     SAVE_TOKEN;DBGPUTS("T_MINUS"); return T_MINUS;
"*"                     SAVE_TOKEN;DBGPUTS("T_MUL"); return T_MUL;
"/"                     SAVE_TOKEN;DBGPUTS("T_DIV"); return T_DIV;
"&"                     SAVE_TOKEN;DBGPUTS("T_AND"); return T_AND;
"|"                     SAVE_TOKEN;DBGPUTS("T_OR"); return T_OR;
"{"                     SAVE_TOKEN;DBGPUTS("T_LC"); return T_LC;
"}"                     SAVE_TOKEN;DBGPUTS("T_RC"); return T_RC;
","                     SAVE_TOKEN;DBGPUTS("T_COMMA"); return T_COMMA;
";"                     SAVE_TOKEN;DBGPUTS("T_SEMI"); return T_SEMI;
.						printf("Unknown token:%s\n", yytext); yyterminate();
%%