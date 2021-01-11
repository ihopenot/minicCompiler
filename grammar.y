%{
#include <cstdio>
#include <string>
#include "ASTnode.h"
#include "lex.h"
#define SAVELINE(x, y) x->SetPos(y.first_line, y.first_column)
extern std::string errmsg;
void yyerror(const char* msg) {
    errmsg = "l: "+std::to_string(yylineno)+" : "+std::string(msg);
}
extern int yylex();
extern ASTnode* rootNode;
%}

%union
{
    std::string* string;
	ASTnode* node;
}

%locations 

%token <string> T_CONSTINT T_CONSTFLOAT T_CONSTSTR T_IDENTIFIER 
%token <string> T_ASSIGN T_PLUS T_MINUS T_MUL T_DIV T_AND T_OR
%token <string> T_NOT T_GT T_GE T_LT T_LE T_EQ T_NEQ
%token <string> TY_INT TY_FLOAT TY_VOID TY_STR TY_BOOL
%token <string> T_IF T_ELSE T_WHILE T_FOR T_SEMI T_COMMA T_RETURN T_VARG
%token <string> T_LP T_RP T_LC T_RC T_LB T_RB

%left T_COMMA
%right T_ASSIGN
%left T_OR
%left T_AND
%left T_EQ T_NEQ
%left T_GE T_GT T_LE T_LT
%left T_PLUS T_MINUS
%left T_MUL T_DIV
%right T_NOT

%type <node> program stmts stmt vardefs vardef var_decl array_dims expr exprs
%type <node> r_args func_decl func_head func_args compstmt if_stmt while_stmt
%type <node> r_params call_params call_expr gstmts gstmt for_stmt rt_arg
%type <string> typename

%start program

%%

program : gstmts { $$ = $1; rootNode = $$; }
        ;

gstmts  : gstmt { $$ = new StmtsNode("stmts"); $$->AddSon($1); SAVELINE($$, @1); }
        | gstmts gstmt { $$ = $1; $1->AddSon($2); }
        ;

gstmt   : var_decl T_SEMI { $$ = $1; }
        | func_decl { $$ = $1; }
        ; 

stmts   : stmt { $$ = new StmtsNode("stmts"); $$->AddSon($1); SAVELINE($$, @1); }
        | stmts stmt { $$ = $1; $1->AddSon($2); }
        ;

stmt    : var_decl T_SEMI { $$ = $1; }
        | if_stmt { $$ = $1; }
        | while_stmt { $$ = $1; }
        | for_stmt { $$ = $1; }
        | expr T_SEMI { $$ = $1; }
        | compstmt { $$ = $1; }
		| T_RETURN expr T_SEMI { $$ = new ReturnNode("ret_stmt"); $$->AddSon($2); SAVELINE($$, @1); }
        | T_RETURN T_SEMI { $$ = new ReturnNode("ret_stmt"); SAVELINE($$, @1); }
        ;

vardef		: T_IDENTIFIER { $$ = new VardefNode("vardef", *$1); }
			| T_IDENTIFIER T_ASSIGN expr { $$ = new VardefNode("vardef", *$1); $$->AddSon($3); SAVELINE($$, @1); }
            | T_IDENTIFIER array_dims { $$ = new ArrayDefNode("arraydef", *$1); $$->AddSon($2); SAVELINE($$, @1); }
			;

array_dims  : T_LB expr T_RB { $$ = new ArrayDimNode("arraydim"); $$->AddSon($2); SAVELINE($$, @1); }
            | array_dims T_LB expr T_RB { $$ = $1; $$->AddSon($3); }
            ;

vardefs 	: vardef { $$ = new VardeclNode("vardecl"); $$->AddSon($1); SAVELINE($$, @1); }
            | vardefs T_COMMA vardef { $$ = $1; $$->AddSon($3); }
            ;

var_decl    : typename vardefs { $$ = $2; $$->SetType(*$1); $$->MaintainType(); SAVELINE($$, @1); }
            ;

func_decl   : func_head compstmt { $$ = $1; $$->AddSon($2); }
			| func_head T_SEMI { $$ = $1; }
            ;

func_head	: typename T_IDENTIFIER T_LP func_args T_RP { $$ = new FunctionDeclNode("func_decl", *$2, *$1); $$->AddSon($4); SAVELINE($$, @1); }
			;

r_args      : rt_arg { $$ = new ASTnode("func_args"); $$->AddSon($1); SAVELINE($$, @1); }
            | r_args T_COMMA rt_arg { $$ = $1; $$->AddSon($1); }
            ;

rt_arg      : typename T_IDENTIFIER { $$ = new VardefNode("arg", *$2, *$1); SAVELINE($$, @1); }
            ;

func_args   : /* blank */ { $$ = new ASTnode("func_args"); }
			| r_args T_COMMA T_VARG { $$ = $1; $$->SetType("varg"); SAVELINE($$, @1); }
            | r_args { $$ = $1; }
            ;

r_params    : expr { $$ = new ASTnode("call_params"); $$->AddSon($1); SAVELINE($$, @1); }
            | r_params T_COMMA expr { $$ = $1; $$->AddSon($3);}
            ;

call_params : /* blank*/ { $$ = new ASTnode("call_params");}
            | r_params { $$ = $1; }
            ;

call_expr   : T_IDENTIFIER T_LP call_params T_RP { $$ = new CallNode("call_expr", *$1); $$->AddSon($3); SAVELINE($$, @1); }
            ;

compstmt    : T_LC stmts T_RC { $$ = new CompstmtNode("compstmt"); $$->AddSon($2); SAVELINE($$, @1); }
            | T_LC T_RC { $$ = new CompstmtNode("compstmt"); SAVELINE($$, @1); }
            ;

if_stmt     : T_IF T_LP expr T_RP stmt { $$ = new IfStmtNode("if_stmt"); $$->AddSon($3); $$->AddSon($5); SAVELINE($$, @1); }
            | T_IF T_LP expr T_RP stmt T_ELSE stmt { $$ = new IfStmtNode("if_else_stmt"); $$->AddSon($3); $$->AddSon($5); $$->AddSon($7); SAVELINE($$, @1); }
            ;

while_stmt  : T_WHILE T_LP expr T_RP stmt { $$ = new WhileNode("while_stmt"); $$->AddSon($3); $$->AddSon($5); SAVELINE($$, @1); }
            ;

for_stmt    : T_FOR T_LP expr T_SEMI expr T_SEMI exprs T_RP stmt { 
                $$ = new ForNode("for_stmt"); $$->AddSon($3); $$->AddSon($5); $$->AddSon($7); $$->AddSon($9); SAVELINE($$, @1); 
              }
            | T_FOR T_LP var_decl T_SEMI expr T_SEMI exprs T_RP stmt { 
                $$ = new ForNode("for_stmt"); $$->AddSon($3); $$->AddSon($5); $$->AddSon($7); $$->AddSon($9); SAVELINE($$, @1); 
              }
            ;

typename    : TY_INT { $$ = $1; }
            | TY_FLOAT { $$ = $1; }
            | TY_VOID { $$ = $1; }
            | TY_STR { $$ = $1; }
			| TY_BOOL { $$ = $1; }
            ;

exprs   : expr T_COMMA expr { $$ = new BinaryNode("binary", *$2); $$->AddSon($1); $$->AddSon($3); SAVELINE($$, @1); }
        | expr { $$ = $1; }
        ;

expr	: expr T_PLUS expr { $$ = new BinaryNode("binary", *$2); $$->AddSon($1); $$->AddSon($3); SAVELINE($$, @1); }
		| expr T_MINUS expr { $$ = new BinaryNode("binary", *$2); $$->AddSon($1); $$->AddSon($3); SAVELINE($$, @1); }
		| expr T_MUL expr { $$ = new BinaryNode("binary", *$2); $$->AddSon($1); $$->AddSon($3); SAVELINE($$, @1); }
		| expr T_DIV expr { $$ = new BinaryNode("binary", *$2); $$->AddSon($1); $$->AddSon($3); SAVELINE($$, @1); }
		| expr T_EQ expr { $$ = new BinaryNode("binary", *$2); $$->AddSon($1); $$->AddSon($3); SAVELINE($$, @1); }
        | expr T_NEQ expr { $$ = new BinaryNode("binary", *$2); $$->AddSon($1); $$->AddSon($3); SAVELINE($$, @1); }
        | expr T_LT expr { $$ = new BinaryNode("binary", *$2); $$->AddSon($1); $$->AddSon($3); SAVELINE($$, @1); }
        | expr T_LE expr { $$ = new BinaryNode("binary", *$2); $$->AddSon($1); $$->AddSon($3); SAVELINE($$, @1); }
        | expr T_GT expr { $$ = new BinaryNode("binary", *$2); $$->AddSon($1); $$->AddSon($3); SAVELINE($$, @1); }
        | expr T_GE expr { $$ = new BinaryNode("binary", *$2); $$->AddSon($1); $$->AddSon($3); SAVELINE($$, @1); }
        | expr T_ASSIGN expr { $$ = new BinaryNode("binary", *$2); $$->AddSon($1); $$->AddSon($3); SAVELINE($$, @1); }
        | expr T_AND expr { $$ = new BinaryNode("binary", *$2); $$->AddSon($1); $$->AddSon($3); SAVELINE($$, @1); }
        | expr T_OR expr { $$ = new BinaryNode("binary", *$2); $$->AddSon($1); $$->AddSon($3); SAVELINE($$, @1); }
        | T_NOT expr { $$ = new UnaryNode("unary", *$1); $$->AddSon($2); SAVELINE($$, @1); }
        | T_LP expr T_RP { $$ = new UnaryNode("unary", *$1); $$->AddSon($2); SAVELINE($$, @1); }
        | T_AND expr { $$ = new UnaryNode("unary", *$1); $$->AddSon($2); SAVELINE($$, @1); }
		| T_IDENTIFIER { $$ = new DeclrefNode("decl_ref", *$1); SAVELINE($$, @1); }
        | T_IDENTIFIER array_dims { $$ = new DeclrefNode("array_ref", *$1); $$->AddSon($2); SAVELINE($$, @1); }
		| T_CONSTFLOAT { $$ = new LiteralNode("float_literal", *$1); $$->SetType("float"); SAVELINE($$, @1); }
		| T_CONSTINT { $$ = new LiteralNode("int_literal", *$1); $$->SetType("int"); SAVELINE($$, @1); }
		| T_CONSTSTR { $$ = new LiteralNode("str_literal", *$1); $$->SetType("string"); SAVELINE($$, @1); }
        | call_expr { $$ = $1; }
		;

%%