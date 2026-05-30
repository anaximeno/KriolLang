/* A Bison parser, made by GNU Bison 3.8.2.  */

/* Bison interface for Yacc-like parsers in C

   Copyright (C) 1984, 1989-1990, 2000-2015, 2018-2021 Free Software Foundation,
   Inc.

   This program is free software: you can redistribute it and/or modify
   it under the terms of the GNU General Public License as published by
   the Free Software Foundation, either version 3 of the License, or
   (at your option) any later version.

   This program is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
   GNU General Public License for more details.

   You should have received a copy of the GNU General Public License
   along with this program.  If not, see <https://www.gnu.org/licenses/>.  */

/* As a special exception, you may create a larger work that contains
   part or all of the Bison parser skeleton and distribute that work
   under terms of your choice, so long as that work isn't itself a
   parser generator using the skeleton or a modified version thereof
   as a parser skeleton.  Alternatively, if you modify or redistribute
   the parser skeleton itself, you may (at your option) remove this
   special exception, which will cause the skeleton and the resulting
   Bison output files to be licensed under the GNU General Public
   License without this special exception.

   This special exception was added by the Free Software Foundation in
   version 2.2 of Bison.  */

/* DO NOT RELY ON FEATURES THAT ARE NOT DOCUMENTED in the manual,
   especially those whose name start with YY_ or yy_.  They are
   private implementation details that can be changed or removed.  */

#ifndef YY_YY_PARSER_HH_INCLUDED
# define YY_YY_PARSER_HH_INCLUDED
/* Debug traces.  */
#ifndef YYDEBUG
# define YYDEBUG 1
#endif
#if YYDEBUG
extern int yydebug;
#endif

/* Token kinds.  */
#ifndef YYTOKENTYPE
# define YYTOKENTYPE
  enum yytokentype
  {
    YYEMPTY = -2,
    YYEOF = 0,                     /* "end of file"  */
    YYerror = 256,                 /* error  */
    YYUNDEF = 257,                 /* "invalid token"  */
    IDENT = 258,                   /* IDENT  */
    STR_LIT = 259,                 /* STR_LIT  */
    MOSTRA = 260,                  /* MOSTRA  */
    INT_LIT = 261,                 /* INT_LIT  */
    FLOAT_LIT = 262,               /* FLOAT_LIT  */
    BOOL_LIT = 263,                /* BOOL_LIT  */
    PLUS = 264,                    /* PLUS  */
    MINUS = 265,                   /* MINUS  */
    MUL = 266,                     /* MUL  */
    DIV = 267,                     /* DIV  */
    EQ = 268,                      /* EQ  */
    NE = 269,                      /* NE  */
    LT = 270,                      /* LT  */
    LE = 271,                      /* LE  */
    GT = 272,                      /* GT  */
    GE = 273,                      /* GE  */
    AND = 274,                     /* AND  */
    OR = 275,                      /* OR  */
    ASSIGN = 276,                  /* ASSIGN  */
    LCURLY = 277,                  /* LCURLY  */
    RCURLY = 278,                  /* RCURLY  */
    COMMA = 279,                   /* COMMA  */
    SEMIC = 280,                   /* SEMIC  */
    LBRAC = 281,                   /* LBRAC  */
    RBRAC = 282,                   /* RBRAC  */
    TYPE_NUM = 283,                /* TYPE_NUM  */
    TYPE_BOOL = 284,               /* TYPE_BOOL  */
    TYPE_VOID = 285,               /* TYPE_VOID  */
    TYPE_NTER = 286,               /* TYPE_NTER  */
    TYPE_TEXTU = 287,              /* TYPE_TEXTU  */
    DIVOLVI = 288,                 /* DIVOLVI  */
    PA = 289,                      /* PA  */
    STRUT = 290,                   /* STRUT  */
    NKUANTU = 291,                 /* NKUANTU  */
    SI = 292,                      /* SI  */
    SINON = 293,                   /* SINON  */
    IMPRISTAN = 294,               /* IMPRISTAN  */
    PARA = 295,                    /* PARA  */
    CONTINUA = 296,                /* CONTINUA  */
    DOT = 297,                     /* DOT  */
    RPAR = 298,                    /* RPAR  */
    LPAR = 299,                    /* LPAR  */
    FN = 300,                      /* FN  */
    UMINUS = 301                   /* UMINUS  */
  };
  typedef enum yytokentype yytoken_kind_t;
#endif

/* Value type.  */
#if ! defined YYSTYPE && ! defined YYSTYPE_IS_DECLARED
union YYSTYPE
{
#line 18 "rules/parser.y"

    int token;
    std::string* integer;
    std::string* floatingpoint;
    std::string* boolean;
    std::string* string;
    kriol::ast::Expr* expr;
    kriol::ast::Sttmt* sttmt;
    kriol::ast::BlockSttmt* block;
    kriol::ast::VarDeclSttmt* vardecl;
    kriol::ast::FuncArgs* params;
    kriol::ast::FuncCallArgs* args;
    kriol::ast::LiteralExpr* litexpr;

#line 125 "parser.hh"

};
typedef union YYSTYPE YYSTYPE;
# define YYSTYPE_IS_TRIVIAL 1
# define YYSTYPE_IS_DECLARED 1
#endif


extern YYSTYPE yylval;


int yyparse (kriol::ast::BlockSttmt** Program);


#endif /* !YY_YY_PARSER_HH_INCLUDED  */
