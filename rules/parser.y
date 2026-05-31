%{
    #include "include/kriol/ast.hh"
    #include "include/kriol/cli.hh"

    #include <cstdio>
    #include <cstdlib>
    #include <iostream>
    #include <memory>

    extern int yylex();
    extern int yylineno;
    void yyerror(kriol::ast::BlockSttmt** Program, const char* err);

    using namespace kriol;
%}

%parse-param { kriol::ast::BlockSttmt** Program }

%union {
    int token;
    std::string* integer;
    std::string* floatingpoint;
    std::string* string;
    kriol::ast::Expr* expr;
    kriol::ast::Sttmt* sttmt;
    kriol::ast::BlockSttmt* block;
    kriol::ast::VarDeclSttmt* vardecl;
    kriol::ast::FuncArgs* params;
    kriol::ast::FuncCallArgs* args;
    kriol::ast::LiteralExpr* litexpr;
}

%destructor { delete $$; } <string> <integer> <floatingpoint>
%destructor { delete $$; } <expr> <sttmt> <block> <vardecl> <params> <args> <litexpr>

%token<string> IDENT STR_LIT FSTR_TEXT
%token<token> MOSTRA "mostra" MOSTRAN "mostran"
%token<integer> INT_LIT
%token<floatingpoint> FLOAT_LIT
%token<string> BOOL_LIT
%token<token>  PLUS "+" MINUS "-" MUL "*" DIV "/"
%token<token>  EQ "=="  NE "!="  LT "<" LE "<=" GT ">" GE ">="
%token<token>  AND "&&" OR "||" ASSIGN "=" LCURLY "{" RCURLY "}" COMMA "," SEMIC ";" LBRAC "[" RBRAC "]"
%token<string> TYPE_NUM TYPE_BOOL TYPE_VOID TYPE_NTER TYPE_TEXTU
%token<token>  DIVOLVI "divolvi" PA "pa"
%token<token>  NKUANTU "nkuantu" SI "si" SINON "sinon" IMPRISTAN "inpristan"
%token<token> PARA "para" CONTINUA "kontinua" DOT "." RPAR ")" LPAR "("
%token<token> FN "fn" NOT "!" SAI "sai" KONFIRMA "konfirma"
%token<token> FSTR_START "f-string" FSTR_END "end of f-string" FSTR_LBRACE "start of interpolation" FSTR_RBRACE "end of interpolation"

%type<expr> expression assignment_expression primary_expression
            constant_expression constant logical_or_expressions logical_and_expressions
            equality_expression relational_expression additive_expression multiplicative_expression
            unary_expression initializer mostra_func_call fstring fstring_parts
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
%right NOT UMINUS

%start program

%define parse.error verbose

%%

program : statements { *Program = $1; }
        | error { *Program = nullptr; }
        ;

type_specifier : TYPE_NUM { $$ = $1; }
               | TYPE_VOID { $$ = $1; }
               | TYPE_BOOL { $$ = $1; }
               | TYPE_NTER { $$ = $1; }
               | TYPE_TEXTU { $$ = $1; }
               ;

constant : INT_LIT   { auto lit = new ast::LiteralExpr("nter",  *$1); lit->LineNum = yylineno; $$ = lit; delete $1; }
         | FLOAT_LIT { auto lit = new ast::LiteralExpr("num",   *$1); lit->LineNum = yylineno; $$ = lit; delete $1; }
         | BOOL_LIT  { auto lit = new ast::LiteralExpr("bool",  *$1); lit->LineNum = yylineno; $$ = lit; delete $1; }
         | STR_LIT   { auto lit = new ast::LiteralExpr("char*", *$1); lit->LineNum = yylineno; $$ = lit; delete $1; }
         | fstring   { $$ = $1; }
         ;

fstring : FSTR_START fstring_parts FSTR_END { $$ = $2; }
        ;

fstring_parts
    : %empty { auto fs = new ast::FStringExpr(); fs->LineNum = yylineno; $$ = fs; }
    | fstring_parts FSTR_TEXT { static_cast<ast::FStringExpr*>($1)->addText(*$2); delete $2; $$ = $1; }
    | fstring_parts FSTR_LBRACE constant_expression FSTR_RBRACE { static_cast<ast::FStringExpr*>($1)->addExpr(std::unique_ptr<ast::Expr>($3)); $$ = $1; }
    ;

identifier : IDENT { $$ = $1; }
           ;

declarator : identifier { $$ = $1; }
           ;

declaration : type_specifier init_declarator SEMIC { $2->SetType(*$1); $2->LineNum = yylineno; $$ = $2; delete $1; }
            ;

init_declarator : declarator { $$ = new ast::VarDeclSttmt("void", *$1, nullptr); delete $1; }
                | declarator ASSIGN initializer { $$ = new ast::VarDeclSttmt("void", *$1, std::unique_ptr<ast::Expr>($3)); delete $1; }
                ;

initializer : expression { $$ = $1; }
            ;

expression : assignment_expression { $$ = $1; }
           ;

constant_expression : logical_or_expressions { $$ = $1; }
                    ;

logical_or_expressions : logical_and_expressions { $$ = $1; }
                       | logical_or_expressions OR logical_and_expressions { auto n = new ast::BinExpr("||", std::unique_ptr<ast::Expr>($1), std::unique_ptr<ast::Expr>($3)); n->LineNum = yylineno; $$ = n; }
                       ;

logical_and_expressions : equality_expression { $$ = $1; }
                        | logical_and_expressions AND equality_expression { auto n = new ast::BinExpr("&&", std::unique_ptr<ast::Expr>($1), std::unique_ptr<ast::Expr>($3)); n->LineNum = yylineno; $$ = n; }
                        ;

equality_expression : relational_expression { $$ = $1; }
                    | equality_expression EQ relational_expression { auto n = new ast::BinExpr("==", std::unique_ptr<ast::Expr>($1), std::unique_ptr<ast::Expr>($3)); n->LineNum = yylineno; $$ = n; }
                    | equality_expression NE relational_expression { auto n = new ast::BinExpr("!=", std::unique_ptr<ast::Expr>($1), std::unique_ptr<ast::Expr>($3)); n->LineNum = yylineno; $$ = n; }
                    ;

relational_expression : additive_expression { $$ = $1; }
                      | relational_expression LT additive_expression { auto n = new ast::BinExpr("<", std::unique_ptr<ast::Expr>($1), std::unique_ptr<ast::Expr>($3)); n->LineNum = yylineno; $$ = n; }
                      | relational_expression GT additive_expression { auto n = new ast::BinExpr(">", std::unique_ptr<ast::Expr>($1), std::unique_ptr<ast::Expr>($3)); n->LineNum = yylineno; $$ = n; }
                      | relational_expression LE additive_expression { auto n = new ast::BinExpr("<=", std::unique_ptr<ast::Expr>($1), std::unique_ptr<ast::Expr>($3)); n->LineNum = yylineno; $$ = n; }
                      | relational_expression GE additive_expression { auto n = new ast::BinExpr(">=", std::unique_ptr<ast::Expr>($1), std::unique_ptr<ast::Expr>($3)); n->LineNum = yylineno; $$ = n; }
                      ;

additive_expression : multiplicative_expression { $$ = $1; }
                    | additive_expression PLUS multiplicative_expression { auto n = new ast::BinExpr("+", std::unique_ptr<ast::Expr>($1), std::unique_ptr<ast::Expr>($3)); n->LineNum = yylineno; $$ = n; }
                    | additive_expression MINUS multiplicative_expression { auto n = new ast::BinExpr("-", std::unique_ptr<ast::Expr>($1), std::unique_ptr<ast::Expr>($3)); n->LineNum = yylineno; $$ = n; }
                    ;

multiplicative_expression : unary_expression { $$ = $1; }
                          | multiplicative_expression MUL unary_expression { auto n = new ast::BinExpr("*", std::unique_ptr<ast::Expr>($1), std::unique_ptr<ast::Expr>($3)); n->LineNum = yylineno; $$ = n; }
                          | multiplicative_expression DIV unary_expression { auto n = new ast::BinExpr("/", std::unique_ptr<ast::Expr>($1), std::unique_ptr<ast::Expr>($3)); n->LineNum = yylineno; $$ = n; }
                          ;

unary_expression : primary_expression                          { $$ = $1; }
                 | NOT unary_expression                         { auto n = new ast::UnaryExpr("!", std::unique_ptr<ast::Expr>($2)); n->LineNum = yylineno; $$ = n; }
                 | MINUS unary_expression %prec UMINUS          { auto n = new ast::UnaryExpr("-", std::unique_ptr<ast::Expr>($2)); n->LineNum = yylineno; $$ = n; }
                 ;

primary_expression : IDENT LPAR argument_list RPAR { auto n = new ast::FunCallExpr(*$1, std::unique_ptr<ast::FuncCallArgs>($3)); n->LineNum = yylineno; $$ = n; delete $1; }
                   | IDENT LPAR RPAR               { auto n = new ast::FunCallExpr(*$1, nullptr); n->LineNum = yylineno; $$ = n; delete $1; }
                   | mostra_func_call               { $$ = $1; }
                   | IDENT                          { auto n = new ast::IdentExpr(*$1); n->LineNum = yylineno; $$ = n; delete $1; }
                   | constant                       { $$ = $1; }
                   | LPAR expression RPAR           { auto n = new ast::ParExpr(std::unique_ptr<ast::Expr>($2)); n->LineNum = yylineno; $$ = n; }
                   ;

assignment_expression : constant_expression { $$ = $1; }
                      | primary_expression assignment_operator assignment_expression { auto n = new ast::AssignExpr(*$2, std::unique_ptr<ast::Expr>($1), std::unique_ptr<ast::Expr>($3)); n->LineNum = yylineno; $$ = n; delete $2; }
                      ;

assignment_operator : ASSIGN { $$ = new std::string("=", 2); }
                    ;

function_declaration : FN declarator LPAR parameter_optional_list RPAR type_specifier compound_statement { auto n = new ast::FuncDeclSttmt(*$6, *$2, std::unique_ptr<ast::FuncArgs>($4), std::unique_ptr<ast::BlockSttmt>($7)); n->LineNum = yylineno; $$ = n; delete $6; delete $2; }
                     | FN declarator LPAR parameter_optional_list RPAR compound_statement { auto n = new ast::FuncDeclSttmt("vaziu", *$2, std::unique_ptr<ast::FuncArgs>($4), std::unique_ptr<ast::BlockSttmt>($6)); n->LineNum = yylineno; $$ = n; delete $2; }
                     ;

parameter_optional_list : parameter_list { $$ = $1; }
                        | %empty { $$ = new ast::FuncArgs(); }
                        ;

parameter_list : parameter_declaration { $$ = new ast::FuncArgs(); $$->AddArg(std::unique_ptr<ast::VarDeclSttmt>($1)); }
               | parameter_list COMMA parameter_declaration { $1->AddArg(std::unique_ptr<ast::VarDeclSttmt>($3)); $$ = $1; }
               ;

parameter_declaration : type_specifier declarator { $$ = new ast::VarDeclSttmt(*$1, *$2, nullptr); $$->IsParam = true; delete $1; delete $2; }
                      ;

argument_list : argument_list COMMA expression { $1->AddArg(std::unique_ptr<ast::Expr>($3)); $$ = $1; }
              | expression { $$ = new ast::FuncCallArgs(); $$->AddArg(std::unique_ptr<ast::Expr>($1)); }
              ;

mostra_func_call : MOSTRA LPAR argument_list RPAR { auto n = new ast::MostraFunCallExpr(std::unique_ptr<ast::FuncCallArgs>($3)); n->LineNum = yylineno; $$ = n; }
                 | MOSTRA LPAR RPAR { auto n = new ast::MostraFunCallExpr(nullptr); n->LineNum = yylineno; $$ = n; }
                 | MOSTRAN LPAR argument_list RPAR { auto n = new ast::MostraFunCallExpr(std::unique_ptr<ast::FuncCallArgs>($3), true); n->LineNum = yylineno; $$ = n; }
                 | MOSTRAN LPAR RPAR { auto n = new ast::MostraFunCallExpr(nullptr, true); n->LineNum = yylineno; $$ = n; }
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

selection_statement : SI expression compound_statement { auto n = new ast::IfSttmt(std::unique_ptr<ast::Expr>($2), std::unique_ptr<ast::BlockSttmt>($3), nullptr); n->LineNum = yylineno; $$ = n; }
                    | SI expression compound_statement SINON else_then { auto n = new ast::IfSttmt(std::unique_ptr<ast::Expr>($2), std::unique_ptr<ast::BlockSttmt>($3), std::unique_ptr<ast::BlockSttmt>($5)); n->LineNum = yylineno; $$ = n; }
                    ;

else_then : compound_statement { $$ = $1; }
          | selection_statement { $$ = new ast::BlockSttmt(); $$->AddSttmt(std::unique_ptr<ast::Sttmt>($1)); }
          ;

iteration_statement : NKUANTU expression compound_statement { auto n = new ast::WhileSttmt(std::unique_ptr<ast::Expr>($2), std::unique_ptr<ast::BlockSttmt>($3)); n->LineNum = yylineno; $$ = n; }
                    | PA expression SEMIC expression SEMIC expression compound_statement { auto n = new ast::ForSttmt(std::unique_ptr<ast::Expr>($2), std::unique_ptr<ast::Expr>($4), std::unique_ptr<ast::Expr>($6), std::unique_ptr<ast::BlockSttmt>($7)); n->LineNum = yylineno; $$ = n; }
                    ;

jump_statement : PARA SEMIC { auto n = new ast::JumpSttmt("break"); n->LineNum = yylineno; $$ = n; }
               | CONTINUA SEMIC { auto n = new ast::JumpSttmt("continue"); n->LineNum = yylineno; $$ = n; }
               | DIVOLVI expression SEMIC { auto n = new ast::ReturnSttmt(std::unique_ptr<ast::Expr>($2)); n->LineNum = yylineno; $$ = n; }
               | DIVOLVI SEMIC { auto n = new ast::ReturnSttmt(nullptr); n->LineNum = yylineno; $$ = n; }
               | SAI LPAR expression RPAR SEMIC { auto n = new ast::SaiSttmt(std::unique_ptr<ast::Expr>($3)); n->LineNum = yylineno; $$ = n; }
               | SAI LPAR RPAR SEMIC { auto n = new ast::SaiSttmt(nullptr); n->LineNum = yylineno; $$ = n; }
               | KONFIRMA LPAR expression RPAR SEMIC { auto n = new ast::KonfirmaSttmt(std::unique_ptr<ast::Expr>($3)); n->LineNum = yylineno; $$ = n; }
               | KONFIRMA LPAR RPAR SEMIC { auto n = new ast::KonfirmaSttmt(nullptr); n->LineNum = yylineno; $$ = n; }
               ;
%%

void yyerror(kriol::ast::BlockSttmt** Program, const char* err) {
    kriol::cli::PrintErr(kriol::cli::GetSourceFile(), yylineno, err, 1);
}
