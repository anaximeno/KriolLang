%option noyywrap yylineno

%{
    #include "include/kriol/ast.hh"
    #include "include/kriol/cli.hh"

    #include <string>
    #include <cstdio>
    #include <iostream>
    #include <cstring>
    #include "parser.hh"

    #define YYERROR_VERBOSE 1

    #define SAVE_TOKEN     yylval.string  = new std::string(yytext, yyleng)
    #define SAVE_BOOLEAN   yylval.boolean = new std::string(strcmp(yytext, "sin") == 0 ? "1" : "0");
    #define TOKENIZE(sym) (yylval.token   = sym)

    int yyerror(char const*);
%}

INT [+|-]?[0-9]+

FLOAT {INT}\.[0-9]+

IDENT [a-zA-Z_][a-zA-Z0-9_]*

STRING ["]([^"\\\n]|\\.|\\\n)*["]

%%

[ \t\n]         /* ignore */;
"//"[^\n]*      /* single-line comment */;
"/*"([^*]|\*+[^*/])*\*+"/"  /* multi-line comment */;

"fn"            return TOKENIZE(FN);
"mostra"        return TOKENIZE(MOSTRA);
"inpristan"     return TOKENIZE(IMPRISTAN);
"si"            return TOKENIZE(SI);
"sinon"         return TOKENIZE(SINON);
"divolvi"       return TOKENIZE(DIVOLVI);
"pa"            return TOKENIZE(PA);
"nkuantu"       return TOKENIZE(NKUANTU);
"num"           SAVE_TOKEN;   return TYPE_NUM;
"nter"          SAVE_TOKEN;   return TYPE_NTER;
"vaziu"         SAVE_TOKEN;   return TYPE_VOID;
"bool"          SAVE_TOKEN;   return TYPE_BOOL;
"textu"         SAVE_TOKEN;   return TYPE_TEXTU;
"sin"           SAVE_BOOLEAN; return BOOL_LIT;
"nau"           SAVE_BOOLEAN; return BOOL_LIT;
"para"          return TOKENIZE(PARA);
"kontinua"      return TOKENIZE(CONTINUA);
"+"             return TOKENIZE(PLUS);
"-"             return TOKENIZE(MINUS);
"*"             return TOKENIZE(MUL);
"/"             return TOKENIZE(DIV);
"("             return TOKENIZE(LPAR);
")"             return TOKENIZE(RPAR);
"."             return TOKENIZE(DOT);
","             return TOKENIZE(COMMA);
";"             return TOKENIZE(SEMIC);
"{"             return TOKENIZE(LCURLY);
"}"             return TOKENIZE(RCURLY);
"["             return TOKENIZE(LBRAC);
"]"             return TOKENIZE(RBRAC);
"="             return TOKENIZE(ASSIGN);
"=="            return TOKENIZE(EQ);
"!="            return TOKENIZE(NE);
">"             return TOKENIZE(GT);
">="            return TOKENIZE(GE);
"<"             return TOKENIZE(LT);
"<="            return TOKENIZE(LE);
"&&"            return TOKENIZE(AND);
"||"            return TOKENIZE(OR);

{INT}           SAVE_TOKEN;   return INT_LIT;
{FLOAT}         SAVE_TOKEN;   return FLOAT_LIT;
{IDENT}         SAVE_TOKEN;   return IDENT;
{STRING}        SAVE_TOKEN;   return STR_LIT;

. {
    kriol::cli::PrintErr(
        "Unknown token " + std::string(yytext) +
        " at line " + std::to_string(yylineno)
    );
}

%%
