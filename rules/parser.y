%{
    #include "include/creol/ast.hh"
    #include "include/creol/cli.hh"
    #include "include/creol/codegen.hh"

    #include <cstdio>
    #include <cstdlib>
    #include <iostream>
    #include <memory>

    extern int yylex();
    void yyerror(creol::ast::BlockSttmt** Program, const char* err);

    using namespace creol;
%}

%parse-param { creol::ast::BlockSttmt** Program }

%union {
    int token;
    std::string* integer;
    std::string* floatingpoint;
    std::string* boolean;
    std::string* string;
    creol::ast::Expr* expr;
    creol::ast::Sttmt* sttmt;
    creol::ast::BlockSttmt* block;
    creol::ast::VarDeclSttmt* vardecl;
    creol::ast::FuncArgs* params;
    creol::ast::FuncCallArgs* args;
    creol::ast::LiteralExpr* litexpr;
}

%destructor { delete $$; } <string> <integer> <floatingpoint> <boolean>
%destructor { delete $$; } <expr> <sttmt> <block> <vardecl> <params> <args> <litexpr>

%token<string> IDENT STR_LIT
%token<token> MOSTRA
%token<integer> INT_LIT
%token<floatingpoint> FLOAT_LIT
%token<boolean> BOOL_LIT
%token<token>  PLUS MINUS MUL DIV
%token<token>  EQ  NE  LT LE GT GE
%token<token>  AND OR ASSIGN LCURLY RCURLY COMMA SEMIC LBRAC RBRAC
%token<string> TYPE_INT TYPE_FLOAT TYPE_BOOL TYPE_VOID
%token<token>  DIVOLVI DI STRUT
%token<token>  NKUANTU SI SINON IMPRISTAN
%token<token> PARA CONTINUA DOT RPAR LPAR

%type<expr> expression assignment_expression function_call primary_expression
            constant_expression constant logical_or_expressions logical_and_expressions
            equality_expression relational_expression additive_expression multiplicative_expression
            unary_expression initializer mostra_func_call
%type<sttmt> expression_statement selection_statement iteration_statement jump_statement
             function_declaration declaration statement import_statement
%type<block> compound_statement statements else_then
%type<string> declarator identifier type_specifier assignment_operator single_import
%type<vardecl> init_declarator parameter_declaration
%type<params> parameter_list parameter_optional_list
%type<args> argument_list

%left LT GT LE GE EQ NE ASSIGN
%left PLUS MINUS
%left MUL DIV
%left UMINUS

%start program

%define parse.error verbose

%%

program : statements { *Program = $1; }
        | error { *Program = nullptr; }
        ;

type_specifier : TYPE_INT { $$ = $1; }
               | TYPE_FLOAT { $$ = $1; }
               | TYPE_VOID { $$ = $1; }
               | TYPE_BOOL { $$ = new std::string("unsigned short"); delete $1; }
               ;

constant : INT_LIT { auto lit = new ast::LiteralExpr("int", *$1); lit->ActivateAutoCast(); $$ = lit; delete $1; }
         | FLOAT_LIT { auto lit = new ast::LiteralExpr("float", *$1); lit->ActivateAutoCast(); $$ = lit; delete $1; }
         | BOOL_LIT { auto lit = new ast::LiteralExpr("unsigned short", *$1); lit->ActivateAutoCast(); $$ = lit; delete $1; }
         | STR_LIT { auto lit = new ast::LiteralExpr("char*", *$1); lit->DeactivateAutoCast(); $$ = lit; delete $1; }
         ;

identifier : IDENT { $$ = $1; }
           ;

declarator : identifier { $$ = $1; }
           ;

declaration : type_specifier init_declarator { $2->SetType(*$1); $$ = $2; delete $1; }
            ;

init_declarator : declarator { $$ = new ast::VarDeclSttmt("void", *$1, nullptr); delete $1; }
                | declarator ASSIGN initializer { $$ = new ast::VarDeclSttmt("void", *$1, std::unique_ptr<ast::Expr>($3)); delete $1; }
                ;

initializer : expression { $$ = $1; }
            ;

expression : assignment_expression { $$ = $1; }
           | function_call { $$ = $1; }
           ;

constant_expression : logical_or_expressions { $$ = $1; }
                    ;

logical_or_expressions : logical_and_expressions { $$ = $1; }
                       | logical_or_expressions OR logical_and_expressions { $$ = new ast::BinExpr("||", std::unique_ptr<ast::Expr>($1), std::unique_ptr<ast::Expr>($3)); }
                       ;

logical_and_expressions : equality_expression { $$ = $1; }
                        | logical_and_expressions AND equality_expression { $$ = new ast::BinExpr("&&", std::unique_ptr<ast::Expr>($1), std::unique_ptr<ast::Expr>($3)); }
                        ;

equality_expression : relational_expression { $$ = $1; }
                    | equality_expression EQ relational_expression { $$ = new ast::BinExpr("==", std::unique_ptr<ast::Expr>($1), std::unique_ptr<ast::Expr>($3)); }
                    | equality_expression NE relational_expression { $$ = new ast::BinExpr("!=", std::unique_ptr<ast::Expr>($1), std::unique_ptr<ast::Expr>($3)); }
                    ;

relational_expression : additive_expression { $$ = $1; }
                      | relational_expression LT additive_expression { $$ = new ast::BinExpr("<", std::unique_ptr<ast::Expr>($1), std::unique_ptr<ast::Expr>($3)); }
                      | relational_expression GT additive_expression { $$ = new ast::BinExpr(">", std::unique_ptr<ast::Expr>($1), std::unique_ptr<ast::Expr>($3)); }
                      | relational_expression LE additive_expression { $$ = new ast::BinExpr("<=", std::unique_ptr<ast::Expr>($1), std::unique_ptr<ast::Expr>($3)); }
                      | relational_expression GE additive_expression { $$ = new ast::BinExpr(">=", std::unique_ptr<ast::Expr>($1), std::unique_ptr<ast::Expr>($3)); }
                      ;

additive_expression : multiplicative_expression { $$ = $1; }
                    | additive_expression PLUS multiplicative_expression { $$ = new ast::BinExpr("+", std::unique_ptr<ast::Expr>($1), std::unique_ptr<ast::Expr>($3)); }
                    | additive_expression MINUS multiplicative_expression { $$ = new ast::BinExpr("-", std::unique_ptr<ast::Expr>($1), std::unique_ptr<ast::Expr>($3)); }
                    ;

multiplicative_expression : unary_expression { $$ = $1; }
                          | multiplicative_expression MUL primary_expression { $$ = new ast::BinExpr("*", std::unique_ptr<ast::Expr>($1), std::unique_ptr<ast::Expr>($3)); }
                          | multiplicative_expression DIV primary_expression { $$ = new ast::BinExpr("/", std::unique_ptr<ast::Expr>($1), std::unique_ptr<ast::Expr>($3)); }
                          ;

unary_expression : primary_expression { $$ = $1; }
                 ;

primary_expression : identifier { $$ = new ast::IdentExpr(*$1); delete $1; }
                   | constant { $$ = $1; }
                   | LPAR expression RPAR { $$ = new ast::ParExpr(std::unique_ptr<ast::Expr>($2)); }
                   ;

assignment_expression : constant_expression { $$ = $1; }
                      | primary_expression assignment_operator assignment_expression { $$ = new ast::AssignExpr(*$2, std::unique_ptr<ast::Expr>($1), std::unique_ptr<ast::Expr>($3)); delete $2; }
                      ;

assignment_operator : ASSIGN { $$ = new std::string("=", 2); }
                    ;

function_declaration : type_specifier declarator LPAR parameter_optional_list RPAR compound_statement { $$ = new ast::FuncDeclSttmt(*$1, *$2, std::unique_ptr<ast::FuncArgs>($4), std::unique_ptr<ast::BlockSttmt>($6)); delete $1; delete $2; }
                     ;

parameter_optional_list : parameter_list { $$ = $1; }
                        | %empty { $$ = new ast::FuncArgs(); }
                        ;

parameter_list : parameter_declaration { $$ = new ast::FuncArgs(); $$->AddArg(std::unique_ptr<ast::VarDeclSttmt>($1)); }
               | parameter_list COMMA parameter_declaration { $1->AddArg(std::unique_ptr<ast::VarDeclSttmt>($3)); $$ = $1; }
               ;

parameter_declaration : type_specifier declarator { $$ = new ast::VarDeclSttmt(*$1, *$2, nullptr); delete $1; delete $2; }
                      ;

argument_list : argument_list COMMA expression { $1->AddArg(std::unique_ptr<ast::Expr>($3)); $$ = $1; }
              | expression { $$ = new ast::FuncCallArgs(); $$->AddArg(std::unique_ptr<ast::Expr>($1)); }
              ;

function_call : identifier LPAR argument_list RPAR { $$ = new ast::FunCallExpr(*$1, std::unique_ptr<ast::FuncCallArgs>($3)); delete $1; }
              | identifier LPAR RPAR { $$ = new ast::FunCallExpr(*$1, nullptr); delete $1; }
              | mostra_func_call { $$ = $1; }
              ;

mostra_func_call : MOSTRA LPAR argument_list RPAR { $$ = new ast::MostraFunCallExpr(std::unique_ptr<ast::FuncCallArgs>($3)); }
                 | MOSTRA LPAR RPAR { $$ = new ast::MostraFunCallExpr(nullptr); }
                 ;

statements : statements statement { $1->AddSttmt(std::unique_ptr<ast::Sttmt>($2)); $$ = $1; }
           | statement { $$ = new ast::BlockSttmt(); $$->AddSttmt(std::unique_ptr<ast::Sttmt>($1)); }
           ;

statement : expression_statement { $$ = $1; }
          | compound_statement { $$ = $1; }
          | selection_statement { $$ = $1; }
          | iteration_statement { $$ = $1; }
          | jump_statement { $$ = $1; }
          | function_declaration { $$ = $1; }
          | declaration { $$ = $1; }
          | import_statement { $$ = $1; }
          | error SEMIC { $$ = nullptr; }
          ;

import_statement : IMPRISTAN single_import { $$ = new ast::ImportSttmt(*$2); delete $2; }
                 ;

single_import : STR_LIT { $$ = $1; }
              ;

expression_statement : expression SEMIC { $$ = new ast::ExprSttmt(std::unique_ptr<ast::Expr>($1)); }
                     | SEMIC { $$ = new ast::ExprSttmt(nullptr); }
                     ;

compound_statement : LCURLY statements RCURLY { $$ = $2; $$->UseBrackets(); }
                   | LCURLY RCURLY { $$ = new ast::BlockSttmt(); $$->UseBrackets(); }
                   ;

selection_statement : SI expression compound_statement { $$ = new ast::IfSttmt(std::unique_ptr<ast::Expr>($2), std::unique_ptr<ast::BlockSttmt>($3), nullptr); }
                    | SI expression compound_statement SINON else_then { $$ = new ast::IfSttmt(std::unique_ptr<ast::Expr>($2), std::unique_ptr<ast::BlockSttmt>($3), std::unique_ptr<ast::BlockSttmt>($5)); }
                    ;

else_then : compound_statement { $$ = $1; }
          | selection_statement { $$ = new ast::BlockSttmt(); $$->AddSttmt(std::unique_ptr<ast::Sttmt>($1)); }
          ;

iteration_statement : NKUANTU expression compound_statement { $$ = new ast::WhileSttmt(std::unique_ptr<ast::Expr>($2), std::unique_ptr<ast::BlockSttmt>($3)); }
                    | DI expression SEMIC expression SEMIC expression compound_statement { $$ = new ast::ForSttmt(std::unique_ptr<ast::Expr>($2), std::unique_ptr<ast::Expr>($4), std::unique_ptr<ast::Expr>($6), std::unique_ptr<ast::BlockSttmt>($7)); }
                    ;

jump_statement : PARA SEMIC { $$ = new ast::JumpSttmt("break"); }
               | CONTINUA SEMIC { $$ = new ast::JumpSttmt("continue"); }
               | DIVOLVI expression SEMIC { $$ = new ast::ReturnSttmt(std::unique_ptr<ast::Expr>($2)); }
               | DIVOLVI SEMIC { $$ = new ast::ReturnSttmt(nullptr); }
               ;
%%

void yyerror(creol::ast::BlockSttmt** Program, const char* err) {
    creol::cli::PrintErr(err, 1);
}
