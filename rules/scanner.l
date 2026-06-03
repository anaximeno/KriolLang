%option noyywrap yylineno stack

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
    #define TOKENIZE(sym) (yylval.token   = sym)

    int yyerror(char const*);

    static int g_fstr_depth = 0;
%}

INT [+|-]?[0-9]+

FLOAT {INT}\.[0-9]+

IDENT [a-zA-Z_][a-zA-Z0-9_]*

STRING_DQ   ["]([^"\\\n]|\\.|\\\n)*["]
STRING_SQ   [']([^'\\\n]|\\.|\\\n)*[']

%x FSTRING_DQ FSTRING_SQ

%%

[ \t\n]         /* ignore */;
"//"[^\n]*      /* single-line comment */;
"/*"([^*]|\*+[^*/])*\*+"/"  /* multi-line comment */;

"fn"            return TOKENIZE(FN);
"mostran"       return TOKENIZE(MOSTRAN);
"mostra"        return TOKENIZE(MOSTRA);
"inpristan"     return TOKENIZE(IMPRISTAN);
"dipoz"         return TOKENIZE(DIPOZ);
"si"            return TOKENIZE(SI);
"sinon"         return TOKENIZE(SINON);
"divolvi"       return TOKENIZE(DIVOLVI);
"pa"            return TOKENIZE(PA);
"nkuantu"       return TOKENIZE(NKUANTU);
"num"           SAVE_TOKEN;   return TYPE_NUM;
"nter"          SAVE_TOKEN;   return TYPE_NTER;
"bool"          SAVE_TOKEN;   return TYPE_BOOL;
"textu"         SAVE_TOKEN;   return TYPE_TEXTU;
"sin"           { yylval.string = new std::string("1"); return BOOL_LIT; }
"nau"           { yylval.string = new std::string("0"); return BOOL_LIT; }
"sai"           return TOKENIZE(SAI);
"konfirma"      return TOKENIZE(KONFIRMA);
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
"}"             { if (g_fstr_depth > 0) { --g_fstr_depth; yy_pop_state(); return TOKENIZE(FSTR_RBRACE); } return TOKENIZE(RCURLY); }
"["             return TOKENIZE(LBRAC);
"]"             return TOKENIZE(RBRAC);
"="             return TOKENIZE(ASSIGN);
"=="            return TOKENIZE(EQ);
"!="            return TOKENIZE(NE);
"!"             return TOKENIZE(NOT);
">"             return TOKENIZE(GT);
">="            return TOKENIZE(GE);
"<"             return TOKENIZE(LT);
"<="            return TOKENIZE(LE);
"&&"            return TOKENIZE(AND);
"||"            return TOKENIZE(OR);

{INT}           SAVE_TOKEN;   return INT_LIT;
{FLOAT}         SAVE_TOKEN;   return FLOAT_LIT;
{IDENT}         SAVE_TOKEN;   return IDENT;
f["]            { yy_push_state(FSTRING_DQ); return TOKENIZE(FSTR_START); }
f[']            { yy_push_state(FSTRING_SQ); return TOKENIZE(FSTR_START); }
{STRING_DQ}     SAVE_TOKEN;   return STR_LIT;
{STRING_SQ}     SAVE_TOKEN;   return STR_LIT;

<FSTRING_DQ>{
    "\""                  { yy_pop_state(); return TOKENIZE(FSTR_END); }
    "{"                   { ++g_fstr_depth; yy_push_state(INITIAL); return TOKENIZE(FSTR_LBRACE); }
    ([^"{\\\n]|\\.)+      { SAVE_TOKEN; return FSTR_TEXT; }
    <<EOF>>               { kriol::cli::PrintErr(kriol::cli::GetSourceFile(), yylineno, "unterminated f-string", 1); }
    .|\n                  { /* skip unrecognised char inside f-string */ }
}

<FSTRING_SQ>{
    "'"                   { yy_pop_state(); return TOKENIZE(FSTR_END); }
    "{"                   { ++g_fstr_depth; yy_push_state(INITIAL); return TOKENIZE(FSTR_LBRACE); }
    ([^'{\\\n]|\\.)+      { SAVE_TOKEN; return FSTR_TEXT; }
    <<EOF>>               { kriol::cli::PrintErr(kriol::cli::GetSourceFile(), yylineno, "unterminated f-string", 1); }
    .|\n                  { /* skip unrecognised char inside f-string */ }
}

. {
    kriol::cli::PrintErr(kriol::cli::GetSourceFile(), yylineno,
        "unknown token '" + std::string(yytext) + "'", 1);
}

%%
