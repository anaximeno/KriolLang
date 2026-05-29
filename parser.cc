/* A Bison parser, made by GNU Bison 3.8.2.  */

/* Bison implementation for Yacc-like parsers in C

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

/* C LALR(1) parser skeleton written by Richard Stallman, by
   simplifying the original so-called "semantic" parser.  */

/* DO NOT RELY ON FEATURES THAT ARE NOT DOCUMENTED in the manual,
   especially those whose name start with YY_ or yy_.  They are
   private implementation details that can be changed or removed.  */

/* All symbols defined below should begin with yy or YY, to avoid
   infringing on user name space.  This should be done even for local
   variables, as they might otherwise be expanded by user macros.
   There are some unavoidable exceptions within include files to
   define necessary library symbols; they are noted "INFRINGES ON
   USER NAME SPACE" below.  */

/* Identify Bison output, and Bison version.  */
#define YYBISON 30802

/* Bison version string.  */
#define YYBISON_VERSION "3.8.2"

/* Skeleton name.  */
#define YYSKELETON_NAME "yacc.c"

/* Pure parsers.  */
#define YYPURE 0

/* Push parsers.  */
#define YYPUSH 0

/* Pull parsers.  */
#define YYPULL 1




/* First part of user prologue.  */
#line 1 "rules/parser.y"

    #include "include/kriol/ast.hh"
    #include "include/kriol/cli.hh"

    #include <cstdio>
    #include <cstdlib>
    #include <iostream>
    #include <memory>

    extern int yylex();
    void yyerror(kriol::ast::BlockSttmt** Program, const char* err);

    using namespace kriol;

#line 86 "parser.cc"

# ifndef YY_CAST
#  ifdef __cplusplus
#   define YY_CAST(Type, Val) static_cast<Type> (Val)
#   define YY_REINTERPRET_CAST(Type, Val) reinterpret_cast<Type> (Val)
#  else
#   define YY_CAST(Type, Val) ((Type) (Val))
#   define YY_REINTERPRET_CAST(Type, Val) ((Type) (Val))
#  endif
# endif
# ifndef YY_NULLPTR
#  if defined __cplusplus
#   if 201103L <= __cplusplus
#    define YY_NULLPTR nullptr
#   else
#    define YY_NULLPTR 0
#   endif
#  else
#   define YY_NULLPTR ((void*)0)
#  endif
# endif

#include "parser.hh"
/* Symbol kind.  */
enum yysymbol_kind_t
{
  YYSYMBOL_YYEMPTY = -2,
  YYSYMBOL_YYEOF = 0,                      /* "end of file"  */
  YYSYMBOL_YYerror = 1,                    /* error  */
  YYSYMBOL_YYUNDEF = 2,                    /* "invalid token"  */
  YYSYMBOL_IDENT = 3,                      /* IDENT  */
  YYSYMBOL_STR_LIT = 4,                    /* STR_LIT  */
  YYSYMBOL_MOSTRA = 5,                     /* MOSTRA  */
  YYSYMBOL_INT_LIT = 6,                    /* INT_LIT  */
  YYSYMBOL_FLOAT_LIT = 7,                  /* FLOAT_LIT  */
  YYSYMBOL_BOOL_LIT = 8,                   /* BOOL_LIT  */
  YYSYMBOL_PLUS = 9,                       /* PLUS  */
  YYSYMBOL_MINUS = 10,                     /* MINUS  */
  YYSYMBOL_MUL = 11,                       /* MUL  */
  YYSYMBOL_DIV = 12,                       /* DIV  */
  YYSYMBOL_EQ = 13,                        /* EQ  */
  YYSYMBOL_NE = 14,                        /* NE  */
  YYSYMBOL_LT = 15,                        /* LT  */
  YYSYMBOL_LE = 16,                        /* LE  */
  YYSYMBOL_GT = 17,                        /* GT  */
  YYSYMBOL_GE = 18,                        /* GE  */
  YYSYMBOL_AND = 19,                       /* AND  */
  YYSYMBOL_OR = 20,                        /* OR  */
  YYSYMBOL_ASSIGN = 21,                    /* ASSIGN  */
  YYSYMBOL_LCURLY = 22,                    /* LCURLY  */
  YYSYMBOL_RCURLY = 23,                    /* RCURLY  */
  YYSYMBOL_COMMA = 24,                     /* COMMA  */
  YYSYMBOL_SEMIC = 25,                     /* SEMIC  */
  YYSYMBOL_LBRAC = 26,                     /* LBRAC  */
  YYSYMBOL_RBRAC = 27,                     /* RBRAC  */
  YYSYMBOL_TYPE_NUM = 28,                  /* TYPE_NUM  */
  YYSYMBOL_TYPE_BOOL = 29,                 /* TYPE_BOOL  */
  YYSYMBOL_TYPE_VOID = 30,                 /* TYPE_VOID  */
  YYSYMBOL_TYPE_NTER = 31,                 /* TYPE_NTER  */
  YYSYMBOL_TYPE_TEXTU = 32,                /* TYPE_TEXTU  */
  YYSYMBOL_DIVOLVI = 33,                   /* DIVOLVI  */
  YYSYMBOL_PA = 34,                        /* PA  */
  YYSYMBOL_STRUT = 35,                     /* STRUT  */
  YYSYMBOL_NKUANTU = 36,                   /* NKUANTU  */
  YYSYMBOL_SI = 37,                        /* SI  */
  YYSYMBOL_SINON = 38,                     /* SINON  */
  YYSYMBOL_IMPRISTAN = 39,                 /* IMPRISTAN  */
  YYSYMBOL_PARA = 40,                      /* PARA  */
  YYSYMBOL_CONTINUA = 41,                  /* CONTINUA  */
  YYSYMBOL_DOT = 42,                       /* DOT  */
  YYSYMBOL_RPAR = 43,                      /* RPAR  */
  YYSYMBOL_LPAR = 44,                      /* LPAR  */
  YYSYMBOL_UMINUS = 45,                    /* UMINUS  */
  YYSYMBOL_YYACCEPT = 46,                  /* $accept  */
  YYSYMBOL_program = 47,                   /* program  */
  YYSYMBOL_type_specifier = 48,            /* type_specifier  */
  YYSYMBOL_constant = 49,                  /* constant  */
  YYSYMBOL_identifier = 50,                /* identifier  */
  YYSYMBOL_declarator = 51,                /* declarator  */
  YYSYMBOL_declaration = 52,               /* declaration  */
  YYSYMBOL_init_declarator = 53,           /* init_declarator  */
  YYSYMBOL_initializer = 54,               /* initializer  */
  YYSYMBOL_expression = 55,                /* expression  */
  YYSYMBOL_constant_expression = 56,       /* constant_expression  */
  YYSYMBOL_logical_or_expressions = 57,    /* logical_or_expressions  */
  YYSYMBOL_logical_and_expressions = 58,   /* logical_and_expressions  */
  YYSYMBOL_equality_expression = 59,       /* equality_expression  */
  YYSYMBOL_relational_expression = 60,     /* relational_expression  */
  YYSYMBOL_additive_expression = 61,       /* additive_expression  */
  YYSYMBOL_multiplicative_expression = 62, /* multiplicative_expression  */
  YYSYMBOL_unary_expression = 63,          /* unary_expression  */
  YYSYMBOL_primary_expression = 64,        /* primary_expression  */
  YYSYMBOL_assignment_expression = 65,     /* assignment_expression  */
  YYSYMBOL_assignment_operator = 66,       /* assignment_operator  */
  YYSYMBOL_function_declaration = 67,      /* function_declaration  */
  YYSYMBOL_parameter_optional_list = 68,   /* parameter_optional_list  */
  YYSYMBOL_parameter_list = 69,            /* parameter_list  */
  YYSYMBOL_parameter_declaration = 70,     /* parameter_declaration  */
  YYSYMBOL_argument_list = 71,             /* argument_list  */
  YYSYMBOL_function_call = 72,             /* function_call  */
  YYSYMBOL_mostra_func_call = 73,          /* mostra_func_call  */
  YYSYMBOL_statements = 74,                /* statements  */
  YYSYMBOL_statement = 75,                 /* statement  */
  YYSYMBOL_import_statement = 76,          /* import_statement  */
  YYSYMBOL_single_import = 77,             /* single_import  */
  YYSYMBOL_expression_statement = 78,      /* expression_statement  */
  YYSYMBOL_compound_statement = 79,        /* compound_statement  */
  YYSYMBOL_selection_statement = 80,       /* selection_statement  */
  YYSYMBOL_else_then = 81,                 /* else_then  */
  YYSYMBOL_iteration_statement = 82,       /* iteration_statement  */
  YYSYMBOL_jump_statement = 83             /* jump_statement  */
};
typedef enum yysymbol_kind_t yysymbol_kind_t;




#ifdef short
# undef short
#endif

/* On compilers that do not define __PTRDIFF_MAX__ etc., make sure
   <limits.h> and (if available) <stdint.h> are included
   so that the code can choose integer types of a good width.  */

#ifndef __PTRDIFF_MAX__
# include <limits.h> /* INFRINGES ON USER NAME SPACE */
# if defined __STDC_VERSION__ && 199901 <= __STDC_VERSION__
#  include <stdint.h> /* INFRINGES ON USER NAME SPACE */
#  define YY_STDINT_H
# endif
#endif

/* Narrow types that promote to a signed type and that can represent a
   signed or unsigned integer of at least N bits.  In tables they can
   save space and decrease cache pressure.  Promoting to a signed type
   helps avoid bugs in integer arithmetic.  */

#ifdef __INT_LEAST8_MAX__
typedef __INT_LEAST8_TYPE__ yytype_int8;
#elif defined YY_STDINT_H
typedef int_least8_t yytype_int8;
#else
typedef signed char yytype_int8;
#endif

#ifdef __INT_LEAST16_MAX__
typedef __INT_LEAST16_TYPE__ yytype_int16;
#elif defined YY_STDINT_H
typedef int_least16_t yytype_int16;
#else
typedef short yytype_int16;
#endif

/* Work around bug in HP-UX 11.23, which defines these macros
   incorrectly for preprocessor constants.  This workaround can likely
   be removed in 2023, as HPE has promised support for HP-UX 11.23
   (aka HP-UX 11i v2) only through the end of 2022; see Table 2 of
   <https://h20195.www2.hpe.com/V2/getpdf.aspx/4AA4-7673ENW.pdf>.  */
#ifdef __hpux
# undef UINT_LEAST8_MAX
# undef UINT_LEAST16_MAX
# define UINT_LEAST8_MAX 255
# define UINT_LEAST16_MAX 65535
#endif

#if defined __UINT_LEAST8_MAX__ && __UINT_LEAST8_MAX__ <= __INT_MAX__
typedef __UINT_LEAST8_TYPE__ yytype_uint8;
#elif (!defined __UINT_LEAST8_MAX__ && defined YY_STDINT_H \
       && UINT_LEAST8_MAX <= INT_MAX)
typedef uint_least8_t yytype_uint8;
#elif !defined __UINT_LEAST8_MAX__ && UCHAR_MAX <= INT_MAX
typedef unsigned char yytype_uint8;
#else
typedef short yytype_uint8;
#endif

#if defined __UINT_LEAST16_MAX__ && __UINT_LEAST16_MAX__ <= __INT_MAX__
typedef __UINT_LEAST16_TYPE__ yytype_uint16;
#elif (!defined __UINT_LEAST16_MAX__ && defined YY_STDINT_H \
       && UINT_LEAST16_MAX <= INT_MAX)
typedef uint_least16_t yytype_uint16;
#elif !defined __UINT_LEAST16_MAX__ && USHRT_MAX <= INT_MAX
typedef unsigned short yytype_uint16;
#else
typedef int yytype_uint16;
#endif

#ifndef YYPTRDIFF_T
# if defined __PTRDIFF_TYPE__ && defined __PTRDIFF_MAX__
#  define YYPTRDIFF_T __PTRDIFF_TYPE__
#  define YYPTRDIFF_MAXIMUM __PTRDIFF_MAX__
# elif defined PTRDIFF_MAX
#  ifndef ptrdiff_t
#   include <stddef.h> /* INFRINGES ON USER NAME SPACE */
#  endif
#  define YYPTRDIFF_T ptrdiff_t
#  define YYPTRDIFF_MAXIMUM PTRDIFF_MAX
# else
#  define YYPTRDIFF_T long
#  define YYPTRDIFF_MAXIMUM LONG_MAX
# endif
#endif

#ifndef YYSIZE_T
# ifdef __SIZE_TYPE__
#  define YYSIZE_T __SIZE_TYPE__
# elif defined size_t
#  define YYSIZE_T size_t
# elif defined __STDC_VERSION__ && 199901 <= __STDC_VERSION__
#  include <stddef.h> /* INFRINGES ON USER NAME SPACE */
#  define YYSIZE_T size_t
# else
#  define YYSIZE_T unsigned
# endif
#endif

#define YYSIZE_MAXIMUM                                  \
  YY_CAST (YYPTRDIFF_T,                                 \
           (YYPTRDIFF_MAXIMUM < YY_CAST (YYSIZE_T, -1)  \
            ? YYPTRDIFF_MAXIMUM                         \
            : YY_CAST (YYSIZE_T, -1)))

#define YYSIZEOF(X) YY_CAST (YYPTRDIFF_T, sizeof (X))


/* Stored state numbers (used for stacks). */
typedef yytype_uint8 yy_state_t;

/* State numbers in computations.  */
typedef int yy_state_fast_t;

#ifndef YY_
# if defined YYENABLE_NLS && YYENABLE_NLS
#  if ENABLE_NLS
#   include <libintl.h> /* INFRINGES ON USER NAME SPACE */
#   define YY_(Msgid) dgettext ("bison-runtime", Msgid)
#  endif
# endif
# ifndef YY_
#  define YY_(Msgid) Msgid
# endif
#endif


#ifndef YY_ATTRIBUTE_PURE
# if defined __GNUC__ && 2 < __GNUC__ + (96 <= __GNUC_MINOR__)
#  define YY_ATTRIBUTE_PURE __attribute__ ((__pure__))
# else
#  define YY_ATTRIBUTE_PURE
# endif
#endif

#ifndef YY_ATTRIBUTE_UNUSED
# if defined __GNUC__ && 2 < __GNUC__ + (7 <= __GNUC_MINOR__)
#  define YY_ATTRIBUTE_UNUSED __attribute__ ((__unused__))
# else
#  define YY_ATTRIBUTE_UNUSED
# endif
#endif

/* Suppress unused-variable warnings by "using" E.  */
#if ! defined lint || defined __GNUC__
# define YY_USE(E) ((void) (E))
#else
# define YY_USE(E) /* empty */
#endif

/* Suppress an incorrect diagnostic about yylval being uninitialized.  */
#if defined __GNUC__ && ! defined __ICC && 406 <= __GNUC__ * 100 + __GNUC_MINOR__
# if __GNUC__ * 100 + __GNUC_MINOR__ < 407
#  define YY_IGNORE_MAYBE_UNINITIALIZED_BEGIN                           \
    _Pragma ("GCC diagnostic push")                                     \
    _Pragma ("GCC diagnostic ignored \"-Wuninitialized\"")
# else
#  define YY_IGNORE_MAYBE_UNINITIALIZED_BEGIN                           \
    _Pragma ("GCC diagnostic push")                                     \
    _Pragma ("GCC diagnostic ignored \"-Wuninitialized\"")              \
    _Pragma ("GCC diagnostic ignored \"-Wmaybe-uninitialized\"")
# endif
# define YY_IGNORE_MAYBE_UNINITIALIZED_END      \
    _Pragma ("GCC diagnostic pop")
#else
# define YY_INITIAL_VALUE(Value) Value
#endif
#ifndef YY_IGNORE_MAYBE_UNINITIALIZED_BEGIN
# define YY_IGNORE_MAYBE_UNINITIALIZED_BEGIN
# define YY_IGNORE_MAYBE_UNINITIALIZED_END
#endif
#ifndef YY_INITIAL_VALUE
# define YY_INITIAL_VALUE(Value) /* Nothing. */
#endif

#if defined __cplusplus && defined __GNUC__ && ! defined __ICC && 6 <= __GNUC__
# define YY_IGNORE_USELESS_CAST_BEGIN                          \
    _Pragma ("GCC diagnostic push")                            \
    _Pragma ("GCC diagnostic ignored \"-Wuseless-cast\"")
# define YY_IGNORE_USELESS_CAST_END            \
    _Pragma ("GCC diagnostic pop")
#endif
#ifndef YY_IGNORE_USELESS_CAST_BEGIN
# define YY_IGNORE_USELESS_CAST_BEGIN
# define YY_IGNORE_USELESS_CAST_END
#endif


#define YY_ASSERT(E) ((void) (0 && (E)))

#if 1

/* The parser invokes alloca or malloc; define the necessary symbols.  */

# ifdef YYSTACK_USE_ALLOCA
#  if YYSTACK_USE_ALLOCA
#   ifdef __GNUC__
#    define YYSTACK_ALLOC __builtin_alloca
#   elif defined __BUILTIN_VA_ARG_INCR
#    include <alloca.h> /* INFRINGES ON USER NAME SPACE */
#   elif defined _AIX
#    define YYSTACK_ALLOC __alloca
#   elif defined _MSC_VER
#    include <malloc.h> /* INFRINGES ON USER NAME SPACE */
#    define alloca _alloca
#   else
#    define YYSTACK_ALLOC alloca
#    if ! defined _ALLOCA_H && ! defined EXIT_SUCCESS
#     include <stdlib.h> /* INFRINGES ON USER NAME SPACE */
      /* Use EXIT_SUCCESS as a witness for stdlib.h.  */
#     ifndef EXIT_SUCCESS
#      define EXIT_SUCCESS 0
#     endif
#    endif
#   endif
#  endif
# endif

# ifdef YYSTACK_ALLOC
   /* Pacify GCC's 'empty if-body' warning.  */
#  define YYSTACK_FREE(Ptr) do { /* empty */; } while (0)
#  ifndef YYSTACK_ALLOC_MAXIMUM
    /* The OS might guarantee only one guard page at the bottom of the stack,
       and a page size can be as small as 4096 bytes.  So we cannot safely
       invoke alloca (N) if N exceeds 4096.  Use a slightly smaller number
       to allow for a few compiler-allocated temporary stack slots.  */
#   define YYSTACK_ALLOC_MAXIMUM 4032 /* reasonable circa 2006 */
#  endif
# else
#  define YYSTACK_ALLOC YYMALLOC
#  define YYSTACK_FREE YYFREE
#  ifndef YYSTACK_ALLOC_MAXIMUM
#   define YYSTACK_ALLOC_MAXIMUM YYSIZE_MAXIMUM
#  endif
#  if (defined __cplusplus && ! defined EXIT_SUCCESS \
       && ! ((defined YYMALLOC || defined malloc) \
             && (defined YYFREE || defined free)))
#   include <stdlib.h> /* INFRINGES ON USER NAME SPACE */
#   ifndef EXIT_SUCCESS
#    define EXIT_SUCCESS 0
#   endif
#  endif
#  ifndef YYMALLOC
#   define YYMALLOC malloc
#   if ! defined malloc && ! defined EXIT_SUCCESS
void *malloc (YYSIZE_T); /* INFRINGES ON USER NAME SPACE */
#   endif
#  endif
#  ifndef YYFREE
#   define YYFREE free
#   if ! defined free && ! defined EXIT_SUCCESS
void free (void *); /* INFRINGES ON USER NAME SPACE */
#   endif
#  endif
# endif
#endif /* 1 */

#if (! defined yyoverflow \
     && (! defined __cplusplus \
         || (defined YYSTYPE_IS_TRIVIAL && YYSTYPE_IS_TRIVIAL)))

/* A type that is properly aligned for any stack member.  */
union yyalloc
{
  yy_state_t yyss_alloc;
  YYSTYPE yyvs_alloc;
};

/* The size of the maximum gap between one aligned stack and the next.  */
# define YYSTACK_GAP_MAXIMUM (YYSIZEOF (union yyalloc) - 1)

/* The size of an array large to enough to hold all stacks, each with
   N elements.  */
# define YYSTACK_BYTES(N) \
     ((N) * (YYSIZEOF (yy_state_t) + YYSIZEOF (YYSTYPE)) \
      + YYSTACK_GAP_MAXIMUM)

# define YYCOPY_NEEDED 1

/* Relocate STACK from its old location to the new one.  The
   local variables YYSIZE and YYSTACKSIZE give the old and new number of
   elements in the stack, and YYPTR gives the new location of the
   stack.  Advance YYPTR to a properly aligned location for the next
   stack.  */
# define YYSTACK_RELOCATE(Stack_alloc, Stack)                           \
    do                                                                  \
      {                                                                 \
        YYPTRDIFF_T yynewbytes;                                         \
        YYCOPY (&yyptr->Stack_alloc, Stack, yysize);                    \
        Stack = &yyptr->Stack_alloc;                                    \
        yynewbytes = yystacksize * YYSIZEOF (*Stack) + YYSTACK_GAP_MAXIMUM; \
        yyptr += yynewbytes / YYSIZEOF (*yyptr);                        \
      }                                                                 \
    while (0)

#endif

#if defined YYCOPY_NEEDED && YYCOPY_NEEDED
/* Copy COUNT objects from SRC to DST.  The source and destination do
   not overlap.  */
# ifndef YYCOPY
#  if defined __GNUC__ && 1 < __GNUC__
#   define YYCOPY(Dst, Src, Count) \
      __builtin_memcpy (Dst, Src, YY_CAST (YYSIZE_T, (Count)) * sizeof (*(Src)))
#  else
#   define YYCOPY(Dst, Src, Count)              \
      do                                        \
        {                                       \
          YYPTRDIFF_T yyi;                      \
          for (yyi = 0; yyi < (Count); yyi++)   \
            (Dst)[yyi] = (Src)[yyi];            \
        }                                       \
      while (0)
#  endif
# endif
#endif /* !YYCOPY_NEEDED */

/* YYFINAL -- State number of the termination state.  */
#define YYFINAL  65
/* YYLAST -- Last index in YYTABLE.  */
#define YYLAST   256

/* YYNTOKENS -- Number of terminals.  */
#define YYNTOKENS  46
/* YYNNTS -- Number of nonterminals.  */
#define YYNNTS  38
/* YYNRULES -- Number of rules.  */
#define YYNRULES  86
/* YYNSTATES -- Number of states.  */
#define YYNSTATES  138

/* YYMAXUTOK -- Last valid token kind.  */
#define YYMAXUTOK   300


/* YYTRANSLATE(TOKEN-NUM) -- Symbol number corresponding to TOKEN-NUM
   as returned by yylex, with out-of-bounds checking.  */
#define YYTRANSLATE(YYX)                                \
  (0 <= (YYX) && (YYX) <= YYMAXUTOK                     \
   ? YY_CAST (yysymbol_kind_t, yytranslate[YYX])        \
   : YYSYMBOL_YYUNDEF)

/* YYTRANSLATE[TOKEN-NUM] -- Symbol number corresponding to TOKEN-NUM
   as returned by yylex.  */
static const yytype_int8 yytranslate[] =
{
       0,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     1,     2,     3,     4,
       5,     6,     7,     8,     9,    10,    11,    12,    13,    14,
      15,    16,    17,    18,    19,    20,    21,    22,    23,    24,
      25,    26,    27,    28,    29,    30,    31,    32,    33,    34,
      35,    36,    37,    38,    39,    40,    41,    42,    43,    44,
      45
};

#if YYDEBUG
/* YYRLINE[YYN] -- Source line where rule number YYN was defined.  */
static const yytype_uint8 yyrline[] =
{
       0,    72,    72,    73,    76,    77,    78,    79,    80,    83,
      84,    85,    86,    89,    92,    95,    98,    99,   102,   105,
     106,   109,   112,   113,   116,   117,   120,   121,   122,   125,
     126,   127,   128,   129,   132,   133,   134,   137,   138,   139,
     142,   145,   146,   147,   150,   151,   154,   157,   160,   161,
     164,   165,   168,   171,   172,   175,   176,   177,   180,   181,
     184,   185,   188,   189,   190,   191,   192,   193,   194,   195,
     196,   199,   202,   205,   206,   209,   210,   213,   214,   217,
     218,   221,   222,   225,   226,   227,   228
};
#endif

/** Accessing symbol of state STATE.  */
#define YY_ACCESSING_SYMBOL(State) YY_CAST (yysymbol_kind_t, yystos[State])

#if 1
/* The user-facing name of the symbol whose (internal) number is
   YYSYMBOL.  No bounds checking.  */
static const char *yysymbol_name (yysymbol_kind_t yysymbol) YY_ATTRIBUTE_UNUSED;

/* YYTNAME[SYMBOL-NUM] -- String name of the symbol SYMBOL-NUM.
   First, the terminals, then, starting at YYNTOKENS, nonterminals.  */
static const char *const yytname[] =
{
  "\"end of file\"", "error", "\"invalid token\"", "IDENT", "STR_LIT",
  "MOSTRA", "INT_LIT", "FLOAT_LIT", "BOOL_LIT", "PLUS", "MINUS", "MUL",
  "DIV", "EQ", "NE", "LT", "LE", "GT", "GE", "AND", "OR", "ASSIGN",
  "LCURLY", "RCURLY", "COMMA", "SEMIC", "LBRAC", "RBRAC", "TYPE_NUM",
  "TYPE_BOOL", "TYPE_VOID", "TYPE_NTER", "TYPE_TEXTU", "DIVOLVI", "PA",
  "STRUT", "NKUANTU", "SI", "SINON", "IMPRISTAN", "PARA", "CONTINUA",
  "DOT", "RPAR", "LPAR", "UMINUS", "$accept", "program", "type_specifier",
  "constant", "identifier", "declarator", "declaration", "init_declarator",
  "initializer", "expression", "constant_expression",
  "logical_or_expressions", "logical_and_expressions",
  "equality_expression", "relational_expression", "additive_expression",
  "multiplicative_expression", "unary_expression", "primary_expression",
  "assignment_expression", "assignment_operator", "function_declaration",
  "parameter_optional_list", "parameter_list", "parameter_declaration",
  "argument_list", "function_call", "mostra_func_call", "statements",
  "statement", "import_statement", "single_import", "expression_statement",
  "compound_statement", "selection_statement", "else_then",
  "iteration_statement", "jump_statement", YY_NULLPTR
};

static const char *
yysymbol_name (yysymbol_kind_t yysymbol)
{
  return yytname[yysymbol];
}
#endif

#define YYPACT_NINF (-63)

#define yypact_value_is_default(Yyn) \
  ((Yyn) == YYPACT_NINF)

#define YYTABLE_NINF (-3)

#define yytable_value_is_error(Yyn) \
  0

/* YYPACT[STATE-NUM] -- Index in YYTABLE of the portion describing
   STATE-NUM.  */
static const yytype_int16 yypact[] =
{
     212,    -4,   -63,   -63,    -9,   -63,   -63,   -63,   128,   -63,
     -63,   -63,   -63,   -63,   -63,     8,    23,    23,    23,    21,
      24,    28,    23,    79,    85,   -63,    69,   -63,    45,   -63,
     101,   105,     6,    57,    41,    70,   -63,   107,   -63,   -63,
     -63,   -63,    86,   -63,   -63,   -63,   -63,   -63,   -63,   -63,
     -63,    34,    -4,   -63,   170,   -63,   112,   113,   123,   123,
     -63,   -63,   -63,   -63,   103,   -63,   -63,     1,   122,   136,
     -63,    40,    40,    40,    40,    40,    40,    40,    40,    40,
      40,    40,    40,   -63,    40,   -63,   -63,   -63,   -20,   -63,
     -63,    23,   -63,   110,   -63,    23,    67,   -63,   -63,   -19,
     -63,   105,   -63,     6,    57,    57,    41,    41,    41,    41,
      70,    70,   -63,   -63,   -63,    23,   -63,   124,    -5,   -63,
     -63,    85,   109,   130,   -63,   -63,   -63,    23,   -63,   -63,
     -63,   -63,   123,    67,   123,   -63,   -63,   -63
};

/* YYDEFACT[STATE-NUM] -- Default reduction number in state STATE-NUM.
   Performed when YYTABLE does not specify something else to do.  Zero
   means the default is an error.  */
static const yytype_int8 yydefact[] =
{
       0,     3,    13,    12,     0,     9,    10,    11,     0,    74,
       4,     6,     5,     7,     8,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,    42,    41,    68,     0,    44,
      21,    22,    24,    26,    29,    34,    37,    40,    19,    67,
      20,    57,     0,    61,    69,    62,    63,    64,    65,    66,
      70,     0,     0,    76,     0,    86,     0,     0,     0,     0,
      72,    71,    83,    84,     0,     1,    14,    16,     0,     0,
      73,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,    46,     0,    60,    59,    54,     0,    75,
      85,     0,    81,    77,    43,     0,    49,    15,    56,     0,
      41,    23,    40,    25,    27,    28,    30,    32,    31,    33,
      35,    36,    38,    39,    45,     0,    58,     0,     0,    17,
      18,     0,     0,    48,    50,    55,    53,     0,    79,    80,
      78,    52,     0,     0,     0,    47,    51,    82
};

/* YYPGOTO[NTERM-NUM].  */
static const yytype_int16 yypgoto[] =
{
     -63,   -63,   -62,   -63,   -16,    42,   -63,   -63,   -63,   -15,
     -63,   -63,    84,    94,    33,    26,    30,   -63,   150,    97,
     -63,   -63,   -63,   -63,    37,   114,   -63,   -63,   174,   -36,
     -63,   -63,   -63,   -49,    66,   -63,   -63,   -63
};

/* YYDEFGOTO[NTERM-NUM].  */
static const yytype_uint8 yydefgoto[] =
{
       0,    23,    24,    25,    26,    67,    27,    68,   119,    28,
      29,    30,    31,    32,    33,    34,    35,    36,    37,    38,
      84,    39,   122,   123,   124,    88,    40,    41,    42,    43,
      44,    61,    45,    46,    47,   130,    48,    49
};

/* YYTABLE[YYPACT[STATE-NUM]] -- What to do in state STATE-NUM.  If
   positive, shift that token.  If negative, reduce the rule whose
   number is the opposite.  If YYTABLE_NINF, syntax error.  */
static const yytype_int16 yytable[] =
{
      56,    57,    58,    59,   115,   115,    85,    64,    66,    92,
      93,     2,     3,     4,     5,     6,     7,     8,    85,    73,
      74,    50,    95,   116,   125,    60,     2,     3,     4,     5,
       6,     7,    18,    55,   121,    51,    87,     2,     3,     4,
       5,     6,     7,     2,     3,    96,     5,     6,     7,    62,
      79,    80,    22,    63,    87,   100,   100,   100,   100,   100,
     100,   100,   100,   100,   100,   100,   100,    22,   100,   128,
      70,   121,    75,    76,    77,    78,   117,    86,    22,    65,
     120,    81,    82,   135,    22,   137,    -2,    52,     2,     2,
       3,     4,     5,     6,     7,    10,    11,    12,    13,    14,
     126,   106,   107,   108,   109,    66,   104,   105,     8,   110,
     111,     9,   134,    69,    10,    11,    12,    13,    14,    15,
      16,    71,    17,    18,    72,    19,    20,    21,    83,    52,
      22,     2,     3,     4,     5,     6,     7,    90,    91,     2,
       3,     4,     5,     6,     7,     8,    94,    97,   118,   127,
       8,    53,   132,     9,   133,   101,    10,    11,    12,    13,
      14,    15,    16,   131,    17,    18,   103,    19,    20,    21,
     136,    52,    22,     2,     3,     4,     5,     6,     7,    98,
      22,   114,    54,    99,   129,     0,     0,     0,     0,     0,
       0,     0,     8,    89,     0,     9,     0,     0,    10,    11,
      12,    13,    14,    15,    16,     0,    17,    18,     0,    19,
      20,    21,     0,     1,    22,     2,     3,     4,     5,     6,
       7,   102,   102,   102,   102,   102,   102,   102,   102,   102,
     102,   112,   113,     0,     8,     0,     0,     9,     0,     0,
      10,    11,    12,    13,    14,    15,    16,     0,    17,    18,
       0,    19,    20,    21,     0,     0,    22
};

static const yytype_int16 yycheck[] =
{
      15,    16,    17,    18,    24,    24,    42,    22,    24,    58,
      59,     3,     4,     5,     6,     7,     8,    22,    54,    13,
      14,    25,    21,    43,    43,     4,     3,     4,     5,     6,
       7,     8,    37,    25,    96,    44,    51,     3,     4,     5,
       6,     7,     8,     3,     4,    44,     6,     7,     8,    25,
       9,    10,    44,    25,    69,    71,    72,    73,    74,    75,
      76,    77,    78,    79,    80,    81,    82,    44,    84,   118,
      25,   133,    15,    16,    17,    18,    91,    43,    44,     0,
      95,    11,    12,   132,    44,   134,     0,     1,     3,     3,
       4,     5,     6,     7,     8,    28,    29,    30,    31,    32,
     115,    75,    76,    77,    78,   121,    73,    74,    22,    79,
      80,    25,   127,    44,    28,    29,    30,    31,    32,    33,
      34,    20,    36,    37,    19,    39,    40,    41,    21,     1,
      44,     3,     4,     5,     6,     7,     8,    25,    25,     3,
       4,     5,     6,     7,     8,    22,    43,    25,    38,    25,
      22,    23,    43,    25,    24,    71,    28,    29,    30,    31,
      32,    33,    34,   121,    36,    37,    72,    39,    40,    41,
     133,     1,    44,     3,     4,     5,     6,     7,     8,    43,
      44,    84,     8,    69,   118,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    22,    23,    -1,    25,    -1,    -1,    28,    29,
      30,    31,    32,    33,    34,    -1,    36,    37,    -1,    39,
      40,    41,    -1,     1,    44,     3,     4,     5,     6,     7,
       8,    71,    72,    73,    74,    75,    76,    77,    78,    79,
      80,    81,    82,    -1,    22,    -1,    -1,    25,    -1,    -1,
      28,    29,    30,    31,    32,    33,    34,    -1,    36,    37,
      -1,    39,    40,    41,    -1,    -1,    44
};

/* YYSTOS[STATE-NUM] -- The symbol kind of the accessing symbol of
   state STATE-NUM.  */
static const yytype_int8 yystos[] =
{
       0,     1,     3,     4,     5,     6,     7,     8,    22,    25,
      28,    29,    30,    31,    32,    33,    34,    36,    37,    39,
      40,    41,    44,    47,    48,    49,    50,    52,    55,    56,
      57,    58,    59,    60,    61,    62,    63,    64,    65,    67,
      72,    73,    74,    75,    76,    78,    79,    80,    82,    83,
      25,    44,     1,    23,    74,    25,    55,    55,    55,    55,
       4,    77,    25,    25,    55,     0,    50,    51,    53,    44,
      25,    20,    19,    13,    14,    15,    16,    17,    18,     9,
      10,    11,    12,    21,    66,    75,    43,    55,    71,    23,
      25,    25,    79,    79,    43,    21,    44,    25,    43,    71,
      50,    58,    64,    59,    60,    60,    61,    61,    61,    61,
      62,    62,    64,    64,    65,    24,    43,    55,    38,    54,
      55,    48,    68,    69,    70,    43,    55,    25,    79,    80,
      81,    51,    43,    24,    55,    79,    70,    79
};

/* YYR1[RULE-NUM] -- Symbol kind of the left-hand side of rule RULE-NUM.  */
static const yytype_int8 yyr1[] =
{
       0,    46,    47,    47,    48,    48,    48,    48,    48,    49,
      49,    49,    49,    50,    51,    52,    53,    53,    54,    55,
      55,    56,    57,    57,    58,    58,    59,    59,    59,    60,
      60,    60,    60,    60,    61,    61,    61,    62,    62,    62,
      63,    64,    64,    64,    65,    65,    66,    67,    68,    68,
      69,    69,    70,    71,    71,    72,    72,    72,    73,    73,
      74,    74,    75,    75,    75,    75,    75,    75,    75,    75,
      75,    76,    77,    78,    78,    79,    79,    80,    80,    81,
      81,    82,    82,    83,    83,    83,    83
};

/* YYR2[RULE-NUM] -- Number of symbols on the right-hand side of rule RULE-NUM.  */
static const yytype_int8 yyr2[] =
{
       0,     2,     1,     1,     1,     1,     1,     1,     1,     1,
       1,     1,     1,     1,     1,     3,     1,     3,     1,     1,
       1,     1,     1,     3,     1,     3,     1,     3,     3,     1,
       3,     3,     3,     3,     1,     3,     3,     1,     3,     3,
       1,     1,     1,     3,     1,     3,     1,     6,     1,     0,
       1,     3,     2,     3,     1,     4,     3,     1,     4,     3,
       2,     1,     1,     1,     1,     1,     1,     1,     1,     1,
       2,     2,     1,     2,     1,     3,     2,     3,     5,     1,
       1,     3,     7,     2,     2,     3,     2
};


enum { YYENOMEM = -2 };

#define yyerrok         (yyerrstatus = 0)
#define yyclearin       (yychar = YYEMPTY)

#define YYACCEPT        goto yyacceptlab
#define YYABORT         goto yyabortlab
#define YYERROR         goto yyerrorlab
#define YYNOMEM         goto yyexhaustedlab


#define YYRECOVERING()  (!!yyerrstatus)

#define YYBACKUP(Token, Value)                                    \
  do                                                              \
    if (yychar == YYEMPTY)                                        \
      {                                                           \
        yychar = (Token);                                         \
        yylval = (Value);                                         \
        YYPOPSTACK (yylen);                                       \
        yystate = *yyssp;                                         \
        goto yybackup;                                            \
      }                                                           \
    else                                                          \
      {                                                           \
        yyerror (Program, YY_("syntax error: cannot back up")); \
        YYERROR;                                                  \
      }                                                           \
  while (0)

/* Backward compatibility with an undocumented macro.
   Use YYerror or YYUNDEF. */
#define YYERRCODE YYUNDEF


/* Enable debugging if requested.  */
#if YYDEBUG

# ifndef YYFPRINTF
#  include <stdio.h> /* INFRINGES ON USER NAME SPACE */
#  define YYFPRINTF fprintf
# endif

# define YYDPRINTF(Args)                        \
do {                                            \
  if (yydebug)                                  \
    YYFPRINTF Args;                             \
} while (0)




# define YY_SYMBOL_PRINT(Title, Kind, Value, Location)                    \
do {                                                                      \
  if (yydebug)                                                            \
    {                                                                     \
      YYFPRINTF (stderr, "%s ", Title);                                   \
      yy_symbol_print (stderr,                                            \
                  Kind, Value, Program); \
      YYFPRINTF (stderr, "\n");                                           \
    }                                                                     \
} while (0)


/*-----------------------------------.
| Print this symbol's value on YYO.  |
`-----------------------------------*/

static void
yy_symbol_value_print (FILE *yyo,
                       yysymbol_kind_t yykind, YYSTYPE const * const yyvaluep, kriol::ast::BlockSttmt** Program)
{
  FILE *yyoutput = yyo;
  YY_USE (yyoutput);
  YY_USE (Program);
  if (!yyvaluep)
    return;
  YY_IGNORE_MAYBE_UNINITIALIZED_BEGIN
  YY_USE (yykind);
  YY_IGNORE_MAYBE_UNINITIALIZED_END
}


/*---------------------------.
| Print this symbol on YYO.  |
`---------------------------*/

static void
yy_symbol_print (FILE *yyo,
                 yysymbol_kind_t yykind, YYSTYPE const * const yyvaluep, kriol::ast::BlockSttmt** Program)
{
  YYFPRINTF (yyo, "%s %s (",
             yykind < YYNTOKENS ? "token" : "nterm", yysymbol_name (yykind));

  yy_symbol_value_print (yyo, yykind, yyvaluep, Program);
  YYFPRINTF (yyo, ")");
}

/*------------------------------------------------------------------.
| yy_stack_print -- Print the state stack from its BOTTOM up to its |
| TOP (included).                                                   |
`------------------------------------------------------------------*/

static void
yy_stack_print (yy_state_t *yybottom, yy_state_t *yytop)
{
  YYFPRINTF (stderr, "Stack now");
  for (; yybottom <= yytop; yybottom++)
    {
      int yybot = *yybottom;
      YYFPRINTF (stderr, " %d", yybot);
    }
  YYFPRINTF (stderr, "\n");
}

# define YY_STACK_PRINT(Bottom, Top)                            \
do {                                                            \
  if (yydebug)                                                  \
    yy_stack_print ((Bottom), (Top));                           \
} while (0)


/*------------------------------------------------.
| Report that the YYRULE is going to be reduced.  |
`------------------------------------------------*/

static void
yy_reduce_print (yy_state_t *yyssp, YYSTYPE *yyvsp,
                 int yyrule, kriol::ast::BlockSttmt** Program)
{
  int yylno = yyrline[yyrule];
  int yynrhs = yyr2[yyrule];
  int yyi;
  YYFPRINTF (stderr, "Reducing stack by rule %d (line %d):\n",
             yyrule - 1, yylno);
  /* The symbols being reduced.  */
  for (yyi = 0; yyi < yynrhs; yyi++)
    {
      YYFPRINTF (stderr, "   $%d = ", yyi + 1);
      yy_symbol_print (stderr,
                       YY_ACCESSING_SYMBOL (+yyssp[yyi + 1 - yynrhs]),
                       &yyvsp[(yyi + 1) - (yynrhs)], Program);
      YYFPRINTF (stderr, "\n");
    }
}

# define YY_REDUCE_PRINT(Rule)          \
do {                                    \
  if (yydebug)                          \
    yy_reduce_print (yyssp, yyvsp, Rule, Program); \
} while (0)

/* Nonzero means print parse trace.  It is left uninitialized so that
   multiple parsers can coexist.  */
int yydebug;
#else /* !YYDEBUG */
# define YYDPRINTF(Args) ((void) 0)
# define YY_SYMBOL_PRINT(Title, Kind, Value, Location)
# define YY_STACK_PRINT(Bottom, Top)
# define YY_REDUCE_PRINT(Rule)
#endif /* !YYDEBUG */


/* YYINITDEPTH -- initial size of the parser's stacks.  */
#ifndef YYINITDEPTH
# define YYINITDEPTH 200
#endif

/* YYMAXDEPTH -- maximum size the stacks can grow to (effective only
   if the built-in stack extension method is used).

   Do not make this value too large; the results are undefined if
   YYSTACK_ALLOC_MAXIMUM < YYSTACK_BYTES (YYMAXDEPTH)
   evaluated with infinite-precision integer arithmetic.  */

#ifndef YYMAXDEPTH
# define YYMAXDEPTH 10000
#endif


/* Context of a parse error.  */
typedef struct
{
  yy_state_t *yyssp;
  yysymbol_kind_t yytoken;
} yypcontext_t;

/* Put in YYARG at most YYARGN of the expected tokens given the
   current YYCTX, and return the number of tokens stored in YYARG.  If
   YYARG is null, return the number of expected tokens (guaranteed to
   be less than YYNTOKENS).  Return YYENOMEM on memory exhaustion.
   Return 0 if there are more than YYARGN expected tokens, yet fill
   YYARG up to YYARGN. */
static int
yypcontext_expected_tokens (const yypcontext_t *yyctx,
                            yysymbol_kind_t yyarg[], int yyargn)
{
  /* Actual size of YYARG. */
  int yycount = 0;
  int yyn = yypact[+*yyctx->yyssp];
  if (!yypact_value_is_default (yyn))
    {
      /* Start YYX at -YYN if negative to avoid negative indexes in
         YYCHECK.  In other words, skip the first -YYN actions for
         this state because they are default actions.  */
      int yyxbegin = yyn < 0 ? -yyn : 0;
      /* Stay within bounds of both yycheck and yytname.  */
      int yychecklim = YYLAST - yyn + 1;
      int yyxend = yychecklim < YYNTOKENS ? yychecklim : YYNTOKENS;
      int yyx;
      for (yyx = yyxbegin; yyx < yyxend; ++yyx)
        if (yycheck[yyx + yyn] == yyx && yyx != YYSYMBOL_YYerror
            && !yytable_value_is_error (yytable[yyx + yyn]))
          {
            if (!yyarg)
              ++yycount;
            else if (yycount == yyargn)
              return 0;
            else
              yyarg[yycount++] = YY_CAST (yysymbol_kind_t, yyx);
          }
    }
  if (yyarg && yycount == 0 && 0 < yyargn)
    yyarg[0] = YYSYMBOL_YYEMPTY;
  return yycount;
}




#ifndef yystrlen
# if defined __GLIBC__ && defined _STRING_H
#  define yystrlen(S) (YY_CAST (YYPTRDIFF_T, strlen (S)))
# else
/* Return the length of YYSTR.  */
static YYPTRDIFF_T
yystrlen (const char *yystr)
{
  YYPTRDIFF_T yylen;
  for (yylen = 0; yystr[yylen]; yylen++)
    continue;
  return yylen;
}
# endif
#endif

#ifndef yystpcpy
# if defined __GLIBC__ && defined _STRING_H && defined _GNU_SOURCE
#  define yystpcpy stpcpy
# else
/* Copy YYSRC to YYDEST, returning the address of the terminating '\0' in
   YYDEST.  */
static char *
yystpcpy (char *yydest, const char *yysrc)
{
  char *yyd = yydest;
  const char *yys = yysrc;

  while ((*yyd++ = *yys++) != '\0')
    continue;

  return yyd - 1;
}
# endif
#endif

#ifndef yytnamerr
/* Copy to YYRES the contents of YYSTR after stripping away unnecessary
   quotes and backslashes, so that it's suitable for yyerror.  The
   heuristic is that double-quoting is unnecessary unless the string
   contains an apostrophe, a comma, or backslash (other than
   backslash-backslash).  YYSTR is taken from yytname.  If YYRES is
   null, do not copy; instead, return the length of what the result
   would have been.  */
static YYPTRDIFF_T
yytnamerr (char *yyres, const char *yystr)
{
  if (*yystr == '"')
    {
      YYPTRDIFF_T yyn = 0;
      char const *yyp = yystr;
      for (;;)
        switch (*++yyp)
          {
          case '\'':
          case ',':
            goto do_not_strip_quotes;

          case '\\':
            if (*++yyp != '\\')
              goto do_not_strip_quotes;
            else
              goto append;

          append:
          default:
            if (yyres)
              yyres[yyn] = *yyp;
            yyn++;
            break;

          case '"':
            if (yyres)
              yyres[yyn] = '\0';
            return yyn;
          }
    do_not_strip_quotes: ;
    }

  if (yyres)
    return yystpcpy (yyres, yystr) - yyres;
  else
    return yystrlen (yystr);
}
#endif


static int
yy_syntax_error_arguments (const yypcontext_t *yyctx,
                           yysymbol_kind_t yyarg[], int yyargn)
{
  /* Actual size of YYARG. */
  int yycount = 0;
  /* There are many possibilities here to consider:
     - If this state is a consistent state with a default action, then
       the only way this function was invoked is if the default action
       is an error action.  In that case, don't check for expected
       tokens because there are none.
     - The only way there can be no lookahead present (in yychar) is if
       this state is a consistent state with a default action.  Thus,
       detecting the absence of a lookahead is sufficient to determine
       that there is no unexpected or expected token to report.  In that
       case, just report a simple "syntax error".
     - Don't assume there isn't a lookahead just because this state is a
       consistent state with a default action.  There might have been a
       previous inconsistent state, consistent state with a non-default
       action, or user semantic action that manipulated yychar.
     - Of course, the expected token list depends on states to have
       correct lookahead information, and it depends on the parser not
       to perform extra reductions after fetching a lookahead from the
       scanner and before detecting a syntax error.  Thus, state merging
       (from LALR or IELR) and default reductions corrupt the expected
       token list.  However, the list is correct for canonical LR with
       one exception: it will still contain any token that will not be
       accepted due to an error action in a later state.
  */
  if (yyctx->yytoken != YYSYMBOL_YYEMPTY)
    {
      int yyn;
      if (yyarg)
        yyarg[yycount] = yyctx->yytoken;
      ++yycount;
      yyn = yypcontext_expected_tokens (yyctx,
                                        yyarg ? yyarg + 1 : yyarg, yyargn - 1);
      if (yyn == YYENOMEM)
        return YYENOMEM;
      else
        yycount += yyn;
    }
  return yycount;
}

/* Copy into *YYMSG, which is of size *YYMSG_ALLOC, an error message
   about the unexpected token YYTOKEN for the state stack whose top is
   YYSSP.

   Return 0 if *YYMSG was successfully written.  Return -1 if *YYMSG is
   not large enough to hold the message.  In that case, also set
   *YYMSG_ALLOC to the required number of bytes.  Return YYENOMEM if the
   required number of bytes is too large to store.  */
static int
yysyntax_error (YYPTRDIFF_T *yymsg_alloc, char **yymsg,
                const yypcontext_t *yyctx)
{
  enum { YYARGS_MAX = 5 };
  /* Internationalized format string. */
  const char *yyformat = YY_NULLPTR;
  /* Arguments of yyformat: reported tokens (one for the "unexpected",
     one per "expected"). */
  yysymbol_kind_t yyarg[YYARGS_MAX];
  /* Cumulated lengths of YYARG.  */
  YYPTRDIFF_T yysize = 0;

  /* Actual size of YYARG. */
  int yycount = yy_syntax_error_arguments (yyctx, yyarg, YYARGS_MAX);
  if (yycount == YYENOMEM)
    return YYENOMEM;

  switch (yycount)
    {
#define YYCASE_(N, S)                       \
      case N:                               \
        yyformat = S;                       \
        break
    default: /* Avoid compiler warnings. */
      YYCASE_(0, YY_("syntax error"));
      YYCASE_(1, YY_("syntax error, unexpected %s"));
      YYCASE_(2, YY_("syntax error, unexpected %s, expecting %s"));
      YYCASE_(3, YY_("syntax error, unexpected %s, expecting %s or %s"));
      YYCASE_(4, YY_("syntax error, unexpected %s, expecting %s or %s or %s"));
      YYCASE_(5, YY_("syntax error, unexpected %s, expecting %s or %s or %s or %s"));
#undef YYCASE_
    }

  /* Compute error message size.  Don't count the "%s"s, but reserve
     room for the terminator.  */
  yysize = yystrlen (yyformat) - 2 * yycount + 1;
  {
    int yyi;
    for (yyi = 0; yyi < yycount; ++yyi)
      {
        YYPTRDIFF_T yysize1
          = yysize + yytnamerr (YY_NULLPTR, yytname[yyarg[yyi]]);
        if (yysize <= yysize1 && yysize1 <= YYSTACK_ALLOC_MAXIMUM)
          yysize = yysize1;
        else
          return YYENOMEM;
      }
  }

  if (*yymsg_alloc < yysize)
    {
      *yymsg_alloc = 2 * yysize;
      if (! (yysize <= *yymsg_alloc
             && *yymsg_alloc <= YYSTACK_ALLOC_MAXIMUM))
        *yymsg_alloc = YYSTACK_ALLOC_MAXIMUM;
      return -1;
    }

  /* Avoid sprintf, as that infringes on the user's name space.
     Don't have undefined behavior even if the translation
     produced a string with the wrong number of "%s"s.  */
  {
    char *yyp = *yymsg;
    int yyi = 0;
    while ((*yyp = *yyformat) != '\0')
      if (*yyp == '%' && yyformat[1] == 's' && yyi < yycount)
        {
          yyp += yytnamerr (yyp, yytname[yyarg[yyi++]]);
          yyformat += 2;
        }
      else
        {
          ++yyp;
          ++yyformat;
        }
  }
  return 0;
}


/*-----------------------------------------------.
| Release the memory associated to this symbol.  |
`-----------------------------------------------*/

static void
yydestruct (const char *yymsg,
            yysymbol_kind_t yykind, YYSTYPE *yyvaluep, kriol::ast::BlockSttmt** Program)
{
  YY_USE (yyvaluep);
  YY_USE (Program);
  if (!yymsg)
    yymsg = "Deleting";
  YY_SYMBOL_PRINT (yymsg, yykind, yyvaluep, yylocationp);

  YY_IGNORE_MAYBE_UNINITIALIZED_BEGIN
  switch (yykind)
    {
    case YYSYMBOL_IDENT: /* IDENT  */
#line 33 "rules/parser.y"
            { delete ((*yyvaluep).string); }
#line 1291 "parser.cc"
        break;

    case YYSYMBOL_STR_LIT: /* STR_LIT  */
#line 33 "rules/parser.y"
            { delete ((*yyvaluep).string); }
#line 1297 "parser.cc"
        break;

    case YYSYMBOL_INT_LIT: /* INT_LIT  */
#line 33 "rules/parser.y"
            { delete ((*yyvaluep).integer); }
#line 1303 "parser.cc"
        break;

    case YYSYMBOL_FLOAT_LIT: /* FLOAT_LIT  */
#line 33 "rules/parser.y"
            { delete ((*yyvaluep).floatingpoint); }
#line 1309 "parser.cc"
        break;

    case YYSYMBOL_BOOL_LIT: /* BOOL_LIT  */
#line 33 "rules/parser.y"
            { delete ((*yyvaluep).boolean); }
#line 1315 "parser.cc"
        break;

    case YYSYMBOL_TYPE_NUM: /* TYPE_NUM  */
#line 33 "rules/parser.y"
            { delete ((*yyvaluep).string); }
#line 1321 "parser.cc"
        break;

    case YYSYMBOL_TYPE_BOOL: /* TYPE_BOOL  */
#line 33 "rules/parser.y"
            { delete ((*yyvaluep).string); }
#line 1327 "parser.cc"
        break;

    case YYSYMBOL_TYPE_VOID: /* TYPE_VOID  */
#line 33 "rules/parser.y"
            { delete ((*yyvaluep).string); }
#line 1333 "parser.cc"
        break;

    case YYSYMBOL_TYPE_NTER: /* TYPE_NTER  */
#line 33 "rules/parser.y"
            { delete ((*yyvaluep).string); }
#line 1339 "parser.cc"
        break;

    case YYSYMBOL_TYPE_TEXTU: /* TYPE_TEXTU  */
#line 33 "rules/parser.y"
            { delete ((*yyvaluep).string); }
#line 1345 "parser.cc"
        break;

    case YYSYMBOL_type_specifier: /* type_specifier  */
#line 33 "rules/parser.y"
            { delete ((*yyvaluep).string); }
#line 1351 "parser.cc"
        break;

    case YYSYMBOL_constant: /* constant  */
#line 34 "rules/parser.y"
            { delete ((*yyvaluep).expr); }
#line 1357 "parser.cc"
        break;

    case YYSYMBOL_identifier: /* identifier  */
#line 33 "rules/parser.y"
            { delete ((*yyvaluep).string); }
#line 1363 "parser.cc"
        break;

    case YYSYMBOL_declarator: /* declarator  */
#line 33 "rules/parser.y"
            { delete ((*yyvaluep).string); }
#line 1369 "parser.cc"
        break;

    case YYSYMBOL_declaration: /* declaration  */
#line 34 "rules/parser.y"
            { delete ((*yyvaluep).sttmt); }
#line 1375 "parser.cc"
        break;

    case YYSYMBOL_init_declarator: /* init_declarator  */
#line 34 "rules/parser.y"
            { delete ((*yyvaluep).vardecl); }
#line 1381 "parser.cc"
        break;

    case YYSYMBOL_initializer: /* initializer  */
#line 34 "rules/parser.y"
            { delete ((*yyvaluep).expr); }
#line 1387 "parser.cc"
        break;

    case YYSYMBOL_expression: /* expression  */
#line 34 "rules/parser.y"
            { delete ((*yyvaluep).expr); }
#line 1393 "parser.cc"
        break;

    case YYSYMBOL_constant_expression: /* constant_expression  */
#line 34 "rules/parser.y"
            { delete ((*yyvaluep).expr); }
#line 1399 "parser.cc"
        break;

    case YYSYMBOL_logical_or_expressions: /* logical_or_expressions  */
#line 34 "rules/parser.y"
            { delete ((*yyvaluep).expr); }
#line 1405 "parser.cc"
        break;

    case YYSYMBOL_logical_and_expressions: /* logical_and_expressions  */
#line 34 "rules/parser.y"
            { delete ((*yyvaluep).expr); }
#line 1411 "parser.cc"
        break;

    case YYSYMBOL_equality_expression: /* equality_expression  */
#line 34 "rules/parser.y"
            { delete ((*yyvaluep).expr); }
#line 1417 "parser.cc"
        break;

    case YYSYMBOL_relational_expression: /* relational_expression  */
#line 34 "rules/parser.y"
            { delete ((*yyvaluep).expr); }
#line 1423 "parser.cc"
        break;

    case YYSYMBOL_additive_expression: /* additive_expression  */
#line 34 "rules/parser.y"
            { delete ((*yyvaluep).expr); }
#line 1429 "parser.cc"
        break;

    case YYSYMBOL_multiplicative_expression: /* multiplicative_expression  */
#line 34 "rules/parser.y"
            { delete ((*yyvaluep).expr); }
#line 1435 "parser.cc"
        break;

    case YYSYMBOL_unary_expression: /* unary_expression  */
#line 34 "rules/parser.y"
            { delete ((*yyvaluep).expr); }
#line 1441 "parser.cc"
        break;

    case YYSYMBOL_primary_expression: /* primary_expression  */
#line 34 "rules/parser.y"
            { delete ((*yyvaluep).expr); }
#line 1447 "parser.cc"
        break;

    case YYSYMBOL_assignment_expression: /* assignment_expression  */
#line 34 "rules/parser.y"
            { delete ((*yyvaluep).expr); }
#line 1453 "parser.cc"
        break;

    case YYSYMBOL_assignment_operator: /* assignment_operator  */
#line 33 "rules/parser.y"
            { delete ((*yyvaluep).string); }
#line 1459 "parser.cc"
        break;

    case YYSYMBOL_function_declaration: /* function_declaration  */
#line 34 "rules/parser.y"
            { delete ((*yyvaluep).sttmt); }
#line 1465 "parser.cc"
        break;

    case YYSYMBOL_parameter_optional_list: /* parameter_optional_list  */
#line 34 "rules/parser.y"
            { delete ((*yyvaluep).params); }
#line 1471 "parser.cc"
        break;

    case YYSYMBOL_parameter_list: /* parameter_list  */
#line 34 "rules/parser.y"
            { delete ((*yyvaluep).params); }
#line 1477 "parser.cc"
        break;

    case YYSYMBOL_parameter_declaration: /* parameter_declaration  */
#line 34 "rules/parser.y"
            { delete ((*yyvaluep).vardecl); }
#line 1483 "parser.cc"
        break;

    case YYSYMBOL_argument_list: /* argument_list  */
#line 34 "rules/parser.y"
            { delete ((*yyvaluep).args); }
#line 1489 "parser.cc"
        break;

    case YYSYMBOL_function_call: /* function_call  */
#line 34 "rules/parser.y"
            { delete ((*yyvaluep).expr); }
#line 1495 "parser.cc"
        break;

    case YYSYMBOL_mostra_func_call: /* mostra_func_call  */
#line 34 "rules/parser.y"
            { delete ((*yyvaluep).expr); }
#line 1501 "parser.cc"
        break;

    case YYSYMBOL_statements: /* statements  */
#line 34 "rules/parser.y"
            { delete ((*yyvaluep).block); }
#line 1507 "parser.cc"
        break;

    case YYSYMBOL_statement: /* statement  */
#line 34 "rules/parser.y"
            { delete ((*yyvaluep).sttmt); }
#line 1513 "parser.cc"
        break;

    case YYSYMBOL_import_statement: /* import_statement  */
#line 34 "rules/parser.y"
            { delete ((*yyvaluep).sttmt); }
#line 1519 "parser.cc"
        break;

    case YYSYMBOL_single_import: /* single_import  */
#line 33 "rules/parser.y"
            { delete ((*yyvaluep).string); }
#line 1525 "parser.cc"
        break;

    case YYSYMBOL_expression_statement: /* expression_statement  */
#line 34 "rules/parser.y"
            { delete ((*yyvaluep).sttmt); }
#line 1531 "parser.cc"
        break;

    case YYSYMBOL_compound_statement: /* compound_statement  */
#line 34 "rules/parser.y"
            { delete ((*yyvaluep).block); }
#line 1537 "parser.cc"
        break;

    case YYSYMBOL_selection_statement: /* selection_statement  */
#line 34 "rules/parser.y"
            { delete ((*yyvaluep).sttmt); }
#line 1543 "parser.cc"
        break;

    case YYSYMBOL_else_then: /* else_then  */
#line 34 "rules/parser.y"
            { delete ((*yyvaluep).block); }
#line 1549 "parser.cc"
        break;

    case YYSYMBOL_iteration_statement: /* iteration_statement  */
#line 34 "rules/parser.y"
            { delete ((*yyvaluep).sttmt); }
#line 1555 "parser.cc"
        break;

    case YYSYMBOL_jump_statement: /* jump_statement  */
#line 34 "rules/parser.y"
            { delete ((*yyvaluep).sttmt); }
#line 1561 "parser.cc"
        break;

      default:
        break;
    }
  YY_IGNORE_MAYBE_UNINITIALIZED_END
}


/* Lookahead token kind.  */
int yychar;

/* The semantic value of the lookahead symbol.  */
YYSTYPE yylval;
/* Number of syntax errors so far.  */
int yynerrs;




/*----------.
| yyparse.  |
`----------*/

int
yyparse (kriol::ast::BlockSttmt** Program)
{
    yy_state_fast_t yystate = 0;
    /* Number of tokens to shift before error messages enabled.  */
    int yyerrstatus = 0;

    /* Refer to the stacks through separate pointers, to allow yyoverflow
       to reallocate them elsewhere.  */

    /* Their size.  */
    YYPTRDIFF_T yystacksize = YYINITDEPTH;

    /* The state stack: array, bottom, top.  */
    yy_state_t yyssa[YYINITDEPTH];
    yy_state_t *yyss = yyssa;
    yy_state_t *yyssp = yyss;

    /* The semantic value stack: array, bottom, top.  */
    YYSTYPE yyvsa[YYINITDEPTH];
    YYSTYPE *yyvs = yyvsa;
    YYSTYPE *yyvsp = yyvs;

  int yyn;
  /* The return value of yyparse.  */
  int yyresult;
  /* Lookahead symbol kind.  */
  yysymbol_kind_t yytoken = YYSYMBOL_YYEMPTY;
  /* The variables used to return semantic value and location from the
     action routines.  */
  YYSTYPE yyval;

  /* Buffer for error messages, and its allocated size.  */
  char yymsgbuf[128];
  char *yymsg = yymsgbuf;
  YYPTRDIFF_T yymsg_alloc = sizeof yymsgbuf;

#define YYPOPSTACK(N)   (yyvsp -= (N), yyssp -= (N))

  /* The number of symbols on the RHS of the reduced rule.
     Keep to zero when no symbol should be popped.  */
  int yylen = 0;

  YYDPRINTF ((stderr, "Starting parse\n"));

  yychar = YYEMPTY; /* Cause a token to be read.  */

  goto yysetstate;


/*------------------------------------------------------------.
| yynewstate -- push a new state, which is found in yystate.  |
`------------------------------------------------------------*/
yynewstate:
  /* In all cases, when you get here, the value and location stacks
     have just been pushed.  So pushing a state here evens the stacks.  */
  yyssp++;


/*--------------------------------------------------------------------.
| yysetstate -- set current state (the top of the stack) to yystate.  |
`--------------------------------------------------------------------*/
yysetstate:
  YYDPRINTF ((stderr, "Entering state %d\n", yystate));
  YY_ASSERT (0 <= yystate && yystate < YYNSTATES);
  YY_IGNORE_USELESS_CAST_BEGIN
  *yyssp = YY_CAST (yy_state_t, yystate);
  YY_IGNORE_USELESS_CAST_END
  YY_STACK_PRINT (yyss, yyssp);

  if (yyss + yystacksize - 1 <= yyssp)
#if !defined yyoverflow && !defined YYSTACK_RELOCATE
    YYNOMEM;
#else
    {
      /* Get the current used size of the three stacks, in elements.  */
      YYPTRDIFF_T yysize = yyssp - yyss + 1;

# if defined yyoverflow
      {
        /* Give user a chance to reallocate the stack.  Use copies of
           these so that the &'s don't force the real ones into
           memory.  */
        yy_state_t *yyss1 = yyss;
        YYSTYPE *yyvs1 = yyvs;

        /* Each stack pointer address is followed by the size of the
           data in use in that stack, in bytes.  This used to be a
           conditional around just the two extra args, but that might
           be undefined if yyoverflow is a macro.  */
        yyoverflow (YY_("memory exhausted"),
                    &yyss1, yysize * YYSIZEOF (*yyssp),
                    &yyvs1, yysize * YYSIZEOF (*yyvsp),
                    &yystacksize);
        yyss = yyss1;
        yyvs = yyvs1;
      }
# else /* defined YYSTACK_RELOCATE */
      /* Extend the stack our own way.  */
      if (YYMAXDEPTH <= yystacksize)
        YYNOMEM;
      yystacksize *= 2;
      if (YYMAXDEPTH < yystacksize)
        yystacksize = YYMAXDEPTH;

      {
        yy_state_t *yyss1 = yyss;
        union yyalloc *yyptr =
          YY_CAST (union yyalloc *,
                   YYSTACK_ALLOC (YY_CAST (YYSIZE_T, YYSTACK_BYTES (yystacksize))));
        if (! yyptr)
          YYNOMEM;
        YYSTACK_RELOCATE (yyss_alloc, yyss);
        YYSTACK_RELOCATE (yyvs_alloc, yyvs);
#  undef YYSTACK_RELOCATE
        if (yyss1 != yyssa)
          YYSTACK_FREE (yyss1);
      }
# endif

      yyssp = yyss + yysize - 1;
      yyvsp = yyvs + yysize - 1;

      YY_IGNORE_USELESS_CAST_BEGIN
      YYDPRINTF ((stderr, "Stack size increased to %ld\n",
                  YY_CAST (long, yystacksize)));
      YY_IGNORE_USELESS_CAST_END

      if (yyss + yystacksize - 1 <= yyssp)
        YYABORT;
    }
#endif /* !defined yyoverflow && !defined YYSTACK_RELOCATE */


  if (yystate == YYFINAL)
    YYACCEPT;

  goto yybackup;


/*-----------.
| yybackup.  |
`-----------*/
yybackup:
  /* Do appropriate processing given the current state.  Read a
     lookahead token if we need one and don't already have one.  */

  /* First try to decide what to do without reference to lookahead token.  */
  yyn = yypact[yystate];
  if (yypact_value_is_default (yyn))
    goto yydefault;

  /* Not known => get a lookahead token if don't already have one.  */

  /* YYCHAR is either empty, or end-of-input, or a valid lookahead.  */
  if (yychar == YYEMPTY)
    {
      YYDPRINTF ((stderr, "Reading a token\n"));
      yychar = yylex ();
    }

  if (yychar <= YYEOF)
    {
      yychar = YYEOF;
      yytoken = YYSYMBOL_YYEOF;
      YYDPRINTF ((stderr, "Now at end of input.\n"));
    }
  else if (yychar == YYerror)
    {
      /* The scanner already issued an error message, process directly
         to error recovery.  But do not keep the error token as
         lookahead, it is too special and may lead us to an endless
         loop in error recovery. */
      yychar = YYUNDEF;
      yytoken = YYSYMBOL_YYerror;
      goto yyerrlab1;
    }
  else
    {
      yytoken = YYTRANSLATE (yychar);
      YY_SYMBOL_PRINT ("Next token is", yytoken, &yylval, &yylloc);
    }

  /* If the proper action on seeing token YYTOKEN is to reduce or to
     detect an error, take that action.  */
  yyn += yytoken;
  if (yyn < 0 || YYLAST < yyn || yycheck[yyn] != yytoken)
    goto yydefault;
  yyn = yytable[yyn];
  if (yyn <= 0)
    {
      if (yytable_value_is_error (yyn))
        goto yyerrlab;
      yyn = -yyn;
      goto yyreduce;
    }

  /* Count tokens shifted since error; after three, turn off error
     status.  */
  if (yyerrstatus)
    yyerrstatus--;

  /* Shift the lookahead token.  */
  YY_SYMBOL_PRINT ("Shifting", yytoken, &yylval, &yylloc);
  yystate = yyn;
  YY_IGNORE_MAYBE_UNINITIALIZED_BEGIN
  *++yyvsp = yylval;
  YY_IGNORE_MAYBE_UNINITIALIZED_END

  /* Discard the shifted token.  */
  yychar = YYEMPTY;
  goto yynewstate;


/*-----------------------------------------------------------.
| yydefault -- do the default action for the current state.  |
`-----------------------------------------------------------*/
yydefault:
  yyn = yydefact[yystate];
  if (yyn == 0)
    goto yyerrlab;
  goto yyreduce;


/*-----------------------------.
| yyreduce -- do a reduction.  |
`-----------------------------*/
yyreduce:
  /* yyn is the number of a rule to reduce with.  */
  yylen = yyr2[yyn];

  /* If YYLEN is nonzero, implement the default value of the action:
     '$$ = $1'.

     Otherwise, the following line sets YYVAL to garbage.
     This behavior is undocumented and Bison
     users should not rely upon it.  Assigning to YYVAL
     unconditionally makes the parser a bit smaller, and it avoids a
     GCC warning that YYVAL may be used uninitialized.  */
  yyval = yyvsp[1-yylen];


  YY_REDUCE_PRINT (yyn);
  switch (yyn)
    {
  case 2: /* program: statements  */
#line 72 "rules/parser.y"
                     { *Program = (yyvsp[0].block); }
#line 1834 "parser.cc"
    break;

  case 3: /* program: error  */
#line 73 "rules/parser.y"
                { *Program = nullptr; }
#line 1840 "parser.cc"
    break;

  case 4: /* type_specifier: TYPE_NUM  */
#line 76 "rules/parser.y"
                          { (yyval.string) = (yyvsp[0].string); }
#line 1846 "parser.cc"
    break;

  case 5: /* type_specifier: TYPE_VOID  */
#line 77 "rules/parser.y"
                           { (yyval.string) = (yyvsp[0].string); }
#line 1852 "parser.cc"
    break;

  case 6: /* type_specifier: TYPE_BOOL  */
#line 78 "rules/parser.y"
                           { (yyval.string) = (yyvsp[0].string); }
#line 1858 "parser.cc"
    break;

  case 7: /* type_specifier: TYPE_NTER  */
#line 79 "rules/parser.y"
                           { (yyval.string) = (yyvsp[0].string); }
#line 1864 "parser.cc"
    break;

  case 8: /* type_specifier: TYPE_TEXTU  */
#line 80 "rules/parser.y"
                            { (yyval.string) = (yyvsp[0].string); }
#line 1870 "parser.cc"
    break;

  case 9: /* constant: INT_LIT  */
#line 83 "rules/parser.y"
                   { auto lit = new ast::LiteralExpr("int", *(yyvsp[0].integer)); lit->ActivateAutoCast(); (yyval.expr) = lit; delete (yyvsp[0].integer); }
#line 1876 "parser.cc"
    break;

  case 10: /* constant: FLOAT_LIT  */
#line 84 "rules/parser.y"
                     { auto lit = new ast::LiteralExpr("float", *(yyvsp[0].floatingpoint)); lit->ActivateAutoCast(); (yyval.expr) = lit; delete (yyvsp[0].floatingpoint); }
#line 1882 "parser.cc"
    break;

  case 11: /* constant: BOOL_LIT  */
#line 85 "rules/parser.y"
                    { auto lit = new ast::LiteralExpr("unsigned short", *(yyvsp[0].boolean)); lit->ActivateAutoCast(); (yyval.expr) = lit; delete (yyvsp[0].boolean); }
#line 1888 "parser.cc"
    break;

  case 12: /* constant: STR_LIT  */
#line 86 "rules/parser.y"
                   { auto lit = new ast::LiteralExpr("char*", *(yyvsp[0].string)); lit->DeactivateAutoCast(); (yyval.expr) = lit; delete (yyvsp[0].string); }
#line 1894 "parser.cc"
    break;

  case 13: /* identifier: IDENT  */
#line 89 "rules/parser.y"
                   { (yyval.string) = (yyvsp[0].string); }
#line 1900 "parser.cc"
    break;

  case 14: /* declarator: identifier  */
#line 92 "rules/parser.y"
                        { (yyval.string) = (yyvsp[0].string); }
#line 1906 "parser.cc"
    break;

  case 15: /* declaration: type_specifier init_declarator SEMIC  */
#line 95 "rules/parser.y"
                                                   { (yyvsp[-1].vardecl)->SetType(*(yyvsp[-2].string)); (yyval.sttmt) = (yyvsp[-1].vardecl); delete (yyvsp[-2].string); }
#line 1912 "parser.cc"
    break;

  case 16: /* init_declarator: declarator  */
#line 98 "rules/parser.y"
                             { (yyval.vardecl) = new ast::VarDeclSttmt("void", *(yyvsp[0].string), nullptr); delete (yyvsp[0].string); }
#line 1918 "parser.cc"
    break;

  case 17: /* init_declarator: declarator ASSIGN initializer  */
#line 99 "rules/parser.y"
                                                { (yyval.vardecl) = new ast::VarDeclSttmt("void", *(yyvsp[-2].string), std::unique_ptr<ast::Expr>((yyvsp[0].expr))); delete (yyvsp[-2].string); }
#line 1924 "parser.cc"
    break;

  case 18: /* initializer: expression  */
#line 102 "rules/parser.y"
                         { (yyval.expr) = (yyvsp[0].expr); }
#line 1930 "parser.cc"
    break;

  case 19: /* expression: assignment_expression  */
#line 105 "rules/parser.y"
                                   { (yyval.expr) = (yyvsp[0].expr); }
#line 1936 "parser.cc"
    break;

  case 20: /* expression: function_call  */
#line 106 "rules/parser.y"
                           { (yyval.expr) = (yyvsp[0].expr); }
#line 1942 "parser.cc"
    break;

  case 21: /* constant_expression: logical_or_expressions  */
#line 109 "rules/parser.y"
                                             { (yyval.expr) = (yyvsp[0].expr); }
#line 1948 "parser.cc"
    break;

  case 22: /* logical_or_expressions: logical_and_expressions  */
#line 112 "rules/parser.y"
                                                 { (yyval.expr) = (yyvsp[0].expr); }
#line 1954 "parser.cc"
    break;

  case 23: /* logical_or_expressions: logical_or_expressions OR logical_and_expressions  */
#line 113 "rules/parser.y"
                                                                           { (yyval.expr) = new ast::BinExpr("||", std::unique_ptr<ast::Expr>((yyvsp[-2].expr)), std::unique_ptr<ast::Expr>((yyvsp[0].expr))); }
#line 1960 "parser.cc"
    break;

  case 24: /* logical_and_expressions: equality_expression  */
#line 116 "rules/parser.y"
                                              { (yyval.expr) = (yyvsp[0].expr); }
#line 1966 "parser.cc"
    break;

  case 25: /* logical_and_expressions: logical_and_expressions AND equality_expression  */
#line 117 "rules/parser.y"
                                                                          { (yyval.expr) = new ast::BinExpr("&&", std::unique_ptr<ast::Expr>((yyvsp[-2].expr)), std::unique_ptr<ast::Expr>((yyvsp[0].expr))); }
#line 1972 "parser.cc"
    break;

  case 26: /* equality_expression: relational_expression  */
#line 120 "rules/parser.y"
                                            { (yyval.expr) = (yyvsp[0].expr); }
#line 1978 "parser.cc"
    break;

  case 27: /* equality_expression: equality_expression EQ relational_expression  */
#line 121 "rules/parser.y"
                                                                   { (yyval.expr) = new ast::BinExpr("==", std::unique_ptr<ast::Expr>((yyvsp[-2].expr)), std::unique_ptr<ast::Expr>((yyvsp[0].expr))); }
#line 1984 "parser.cc"
    break;

  case 28: /* equality_expression: equality_expression NE relational_expression  */
#line 122 "rules/parser.y"
                                                                   { (yyval.expr) = new ast::BinExpr("!=", std::unique_ptr<ast::Expr>((yyvsp[-2].expr)), std::unique_ptr<ast::Expr>((yyvsp[0].expr))); }
#line 1990 "parser.cc"
    break;

  case 29: /* relational_expression: additive_expression  */
#line 125 "rules/parser.y"
                                            { (yyval.expr) = (yyvsp[0].expr); }
#line 1996 "parser.cc"
    break;

  case 30: /* relational_expression: relational_expression LT additive_expression  */
#line 126 "rules/parser.y"
                                                                     { (yyval.expr) = new ast::BinExpr("<", std::unique_ptr<ast::Expr>((yyvsp[-2].expr)), std::unique_ptr<ast::Expr>((yyvsp[0].expr))); }
#line 2002 "parser.cc"
    break;

  case 31: /* relational_expression: relational_expression GT additive_expression  */
#line 127 "rules/parser.y"
                                                                     { (yyval.expr) = new ast::BinExpr(">", std::unique_ptr<ast::Expr>((yyvsp[-2].expr)), std::unique_ptr<ast::Expr>((yyvsp[0].expr))); }
#line 2008 "parser.cc"
    break;

  case 32: /* relational_expression: relational_expression LE additive_expression  */
#line 128 "rules/parser.y"
                                                                     { (yyval.expr) = new ast::BinExpr("<=", std::unique_ptr<ast::Expr>((yyvsp[-2].expr)), std::unique_ptr<ast::Expr>((yyvsp[0].expr))); }
#line 2014 "parser.cc"
    break;

  case 33: /* relational_expression: relational_expression GE additive_expression  */
#line 129 "rules/parser.y"
                                                                     { (yyval.expr) = new ast::BinExpr(">=", std::unique_ptr<ast::Expr>((yyvsp[-2].expr)), std::unique_ptr<ast::Expr>((yyvsp[0].expr))); }
#line 2020 "parser.cc"
    break;

  case 34: /* additive_expression: multiplicative_expression  */
#line 132 "rules/parser.y"
                                                { (yyval.expr) = (yyvsp[0].expr); }
#line 2026 "parser.cc"
    break;

  case 35: /* additive_expression: additive_expression PLUS multiplicative_expression  */
#line 133 "rules/parser.y"
                                                                         { (yyval.expr) = new ast::BinExpr("+", std::unique_ptr<ast::Expr>((yyvsp[-2].expr)), std::unique_ptr<ast::Expr>((yyvsp[0].expr))); }
#line 2032 "parser.cc"
    break;

  case 36: /* additive_expression: additive_expression MINUS multiplicative_expression  */
#line 134 "rules/parser.y"
                                                                          { (yyval.expr) = new ast::BinExpr("-", std::unique_ptr<ast::Expr>((yyvsp[-2].expr)), std::unique_ptr<ast::Expr>((yyvsp[0].expr))); }
#line 2038 "parser.cc"
    break;

  case 37: /* multiplicative_expression: unary_expression  */
#line 137 "rules/parser.y"
                                             { (yyval.expr) = (yyvsp[0].expr); }
#line 2044 "parser.cc"
    break;

  case 38: /* multiplicative_expression: multiplicative_expression MUL primary_expression  */
#line 138 "rules/parser.y"
                                                                             { (yyval.expr) = new ast::BinExpr("*", std::unique_ptr<ast::Expr>((yyvsp[-2].expr)), std::unique_ptr<ast::Expr>((yyvsp[0].expr))); }
#line 2050 "parser.cc"
    break;

  case 39: /* multiplicative_expression: multiplicative_expression DIV primary_expression  */
#line 139 "rules/parser.y"
                                                                             { (yyval.expr) = new ast::BinExpr("/", std::unique_ptr<ast::Expr>((yyvsp[-2].expr)), std::unique_ptr<ast::Expr>((yyvsp[0].expr))); }
#line 2056 "parser.cc"
    break;

  case 40: /* unary_expression: primary_expression  */
#line 142 "rules/parser.y"
                                      { (yyval.expr) = (yyvsp[0].expr); }
#line 2062 "parser.cc"
    break;

  case 41: /* primary_expression: identifier  */
#line 145 "rules/parser.y"
                                { (yyval.expr) = new ast::IdentExpr(*(yyvsp[0].string)); delete (yyvsp[0].string); }
#line 2068 "parser.cc"
    break;

  case 42: /* primary_expression: constant  */
#line 146 "rules/parser.y"
                              { (yyval.expr) = (yyvsp[0].expr); }
#line 2074 "parser.cc"
    break;

  case 43: /* primary_expression: LPAR expression RPAR  */
#line 147 "rules/parser.y"
                                          { (yyval.expr) = new ast::ParExpr(std::unique_ptr<ast::Expr>((yyvsp[-1].expr))); }
#line 2080 "parser.cc"
    break;

  case 44: /* assignment_expression: constant_expression  */
#line 150 "rules/parser.y"
                                            { (yyval.expr) = (yyvsp[0].expr); }
#line 2086 "parser.cc"
    break;

  case 45: /* assignment_expression: primary_expression assignment_operator assignment_expression  */
#line 151 "rules/parser.y"
                                                                                     { (yyval.expr) = new ast::AssignExpr(*(yyvsp[-1].string), std::unique_ptr<ast::Expr>((yyvsp[-2].expr)), std::unique_ptr<ast::Expr>((yyvsp[0].expr))); delete (yyvsp[-1].string); }
#line 2092 "parser.cc"
    break;

  case 46: /* assignment_operator: ASSIGN  */
#line 154 "rules/parser.y"
                             { (yyval.string) = new std::string("=", 2); }
#line 2098 "parser.cc"
    break;

  case 47: /* function_declaration: type_specifier declarator LPAR parameter_optional_list RPAR compound_statement  */
#line 157 "rules/parser.y"
                                                                                                      { (yyval.sttmt) = new ast::FuncDeclSttmt(*(yyvsp[-5].string), *(yyvsp[-4].string), std::unique_ptr<ast::FuncArgs>((yyvsp[-2].params)), std::unique_ptr<ast::BlockSttmt>((yyvsp[0].block))); delete (yyvsp[-5].string); delete (yyvsp[-4].string); }
#line 2104 "parser.cc"
    break;

  case 48: /* parameter_optional_list: parameter_list  */
#line 160 "rules/parser.y"
                                         { (yyval.params) = (yyvsp[0].params); }
#line 2110 "parser.cc"
    break;

  case 49: /* parameter_optional_list: %empty  */
#line 161 "rules/parser.y"
                                 { (yyval.params) = new ast::FuncArgs(); }
#line 2116 "parser.cc"
    break;

  case 50: /* parameter_list: parameter_declaration  */
#line 164 "rules/parser.y"
                                       { (yyval.params) = new ast::FuncArgs(); (yyval.params)->AddArg(std::unique_ptr<ast::VarDeclSttmt>((yyvsp[0].vardecl))); }
#line 2122 "parser.cc"
    break;

  case 51: /* parameter_list: parameter_list COMMA parameter_declaration  */
#line 165 "rules/parser.y"
                                                            { (yyvsp[-2].params)->AddArg(std::unique_ptr<ast::VarDeclSttmt>((yyvsp[0].vardecl))); (yyval.params) = (yyvsp[-2].params); }
#line 2128 "parser.cc"
    break;

  case 52: /* parameter_declaration: type_specifier declarator  */
#line 168 "rules/parser.y"
                                                  { (yyval.vardecl) = new ast::VarDeclSttmt(*(yyvsp[-1].string), *(yyvsp[0].string), nullptr); (yyval.vardecl)->IsParam = true; delete (yyvsp[-1].string); delete (yyvsp[0].string); }
#line 2134 "parser.cc"
    break;

  case 53: /* argument_list: argument_list COMMA expression  */
#line 171 "rules/parser.y"
                                               { (yyvsp[-2].args)->AddArg(std::unique_ptr<ast::Expr>((yyvsp[0].expr))); (yyval.args) = (yyvsp[-2].args); }
#line 2140 "parser.cc"
    break;

  case 54: /* argument_list: expression  */
#line 172 "rules/parser.y"
                           { (yyval.args) = new ast::FuncCallArgs(); (yyval.args)->AddArg(std::unique_ptr<ast::Expr>((yyvsp[0].expr))); }
#line 2146 "parser.cc"
    break;

  case 55: /* function_call: identifier LPAR argument_list RPAR  */
#line 175 "rules/parser.y"
                                                   { (yyval.expr) = new ast::FunCallExpr(*(yyvsp[-3].string), std::unique_ptr<ast::FuncCallArgs>((yyvsp[-1].args))); delete (yyvsp[-3].string); }
#line 2152 "parser.cc"
    break;

  case 56: /* function_call: identifier LPAR RPAR  */
#line 176 "rules/parser.y"
                                     { (yyval.expr) = new ast::FunCallExpr(*(yyvsp[-2].string), nullptr); delete (yyvsp[-2].string); }
#line 2158 "parser.cc"
    break;

  case 57: /* function_call: mostra_func_call  */
#line 177 "rules/parser.y"
                                 { (yyval.expr) = (yyvsp[0].expr); }
#line 2164 "parser.cc"
    break;

  case 58: /* mostra_func_call: MOSTRA LPAR argument_list RPAR  */
#line 180 "rules/parser.y"
                                                  { (yyval.expr) = new ast::MostraFunCallExpr(std::unique_ptr<ast::FuncCallArgs>((yyvsp[-1].args))); }
#line 2170 "parser.cc"
    break;

  case 59: /* mostra_func_call: MOSTRA LPAR RPAR  */
#line 181 "rules/parser.y"
                                    { (yyval.expr) = new ast::MostraFunCallExpr(nullptr); }
#line 2176 "parser.cc"
    break;

  case 60: /* statements: statements statement  */
#line 184 "rules/parser.y"
                                  { (yyvsp[-1].block)->AddSttmt(std::unique_ptr<ast::Sttmt>((yyvsp[0].sttmt))); (yyval.block) = (yyvsp[-1].block); }
#line 2182 "parser.cc"
    break;

  case 61: /* statements: statement  */
#line 185 "rules/parser.y"
                       { (yyval.block) = new ast::BlockSttmt(); (yyval.block)->AddSttmt(std::unique_ptr<ast::Sttmt>((yyvsp[0].sttmt))); }
#line 2188 "parser.cc"
    break;

  case 62: /* statement: expression_statement  */
#line 188 "rules/parser.y"
                                 { (yyval.sttmt) = (yyvsp[0].sttmt); }
#line 2194 "parser.cc"
    break;

  case 63: /* statement: compound_statement  */
#line 189 "rules/parser.y"
                               { (yyval.sttmt) = (yyvsp[0].block); }
#line 2200 "parser.cc"
    break;

  case 64: /* statement: selection_statement  */
#line 190 "rules/parser.y"
                                { (yyval.sttmt) = (yyvsp[0].sttmt); }
#line 2206 "parser.cc"
    break;

  case 65: /* statement: iteration_statement  */
#line 191 "rules/parser.y"
                                { (yyval.sttmt) = (yyvsp[0].sttmt); }
#line 2212 "parser.cc"
    break;

  case 66: /* statement: jump_statement  */
#line 192 "rules/parser.y"
                           { (yyval.sttmt) = (yyvsp[0].sttmt); }
#line 2218 "parser.cc"
    break;

  case 67: /* statement: function_declaration  */
#line 193 "rules/parser.y"
                                 { (yyval.sttmt) = (yyvsp[0].sttmt); }
#line 2224 "parser.cc"
    break;

  case 68: /* statement: declaration  */
#line 194 "rules/parser.y"
                        { (yyval.sttmt) = (yyvsp[0].sttmt); }
#line 2230 "parser.cc"
    break;

  case 69: /* statement: import_statement  */
#line 195 "rules/parser.y"
                             { (yyval.sttmt) = (yyvsp[0].sttmt); }
#line 2236 "parser.cc"
    break;

  case 70: /* statement: error SEMIC  */
#line 196 "rules/parser.y"
                        { (yyval.sttmt) = nullptr; }
#line 2242 "parser.cc"
    break;

  case 71: /* import_statement: IMPRISTAN single_import  */
#line 199 "rules/parser.y"
                                           { (yyval.sttmt) = new ast::ImportSttmt(*(yyvsp[0].string)); delete (yyvsp[0].string); }
#line 2248 "parser.cc"
    break;

  case 72: /* single_import: STR_LIT  */
#line 202 "rules/parser.y"
                        { (yyval.string) = (yyvsp[0].string); }
#line 2254 "parser.cc"
    break;

  case 73: /* expression_statement: expression SEMIC  */
#line 205 "rules/parser.y"
                                        { (yyval.sttmt) = new ast::ExprSttmt(std::unique_ptr<ast::Expr>((yyvsp[-1].expr))); }
#line 2260 "parser.cc"
    break;

  case 74: /* expression_statement: SEMIC  */
#line 206 "rules/parser.y"
                             { (yyval.sttmt) = new ast::ExprSttmt(nullptr); }
#line 2266 "parser.cc"
    break;

  case 75: /* compound_statement: LCURLY statements RCURLY  */
#line 209 "rules/parser.y"
                                              { (yyval.block) = (yyvsp[-1].block); (yyval.block)->UseBrackets(); }
#line 2272 "parser.cc"
    break;

  case 76: /* compound_statement: LCURLY RCURLY  */
#line 210 "rules/parser.y"
                                   { (yyval.block) = new ast::BlockSttmt(); (yyval.block)->UseBrackets(); }
#line 2278 "parser.cc"
    break;

  case 77: /* selection_statement: SI expression compound_statement  */
#line 213 "rules/parser.y"
                                                       { (yyval.sttmt) = new ast::IfSttmt(std::unique_ptr<ast::Expr>((yyvsp[-1].expr)), std::unique_ptr<ast::BlockSttmt>((yyvsp[0].block)), nullptr); }
#line 2284 "parser.cc"
    break;

  case 78: /* selection_statement: SI expression compound_statement SINON else_then  */
#line 214 "rules/parser.y"
                                                                       { (yyval.sttmt) = new ast::IfSttmt(std::unique_ptr<ast::Expr>((yyvsp[-3].expr)), std::unique_ptr<ast::BlockSttmt>((yyvsp[-2].block)), std::unique_ptr<ast::BlockSttmt>((yyvsp[0].block))); }
#line 2290 "parser.cc"
    break;

  case 79: /* else_then: compound_statement  */
#line 217 "rules/parser.y"
                               { (yyval.block) = (yyvsp[0].block); }
#line 2296 "parser.cc"
    break;

  case 80: /* else_then: selection_statement  */
#line 218 "rules/parser.y"
                                { (yyval.block) = new ast::BlockSttmt(); (yyval.block)->AddSttmt(std::unique_ptr<ast::Sttmt>((yyvsp[0].sttmt))); }
#line 2302 "parser.cc"
    break;

  case 81: /* iteration_statement: NKUANTU expression compound_statement  */
#line 221 "rules/parser.y"
                                                            { (yyval.sttmt) = new ast::WhileSttmt(std::unique_ptr<ast::Expr>((yyvsp[-1].expr)), std::unique_ptr<ast::BlockSttmt>((yyvsp[0].block))); }
#line 2308 "parser.cc"
    break;

  case 82: /* iteration_statement: PA expression SEMIC expression SEMIC expression compound_statement  */
#line 222 "rules/parser.y"
                                                                                         { (yyval.sttmt) = new ast::ForSttmt(std::unique_ptr<ast::Expr>((yyvsp[-5].expr)), std::unique_ptr<ast::Expr>((yyvsp[-3].expr)), std::unique_ptr<ast::Expr>((yyvsp[-1].expr)), std::unique_ptr<ast::BlockSttmt>((yyvsp[0].block))); }
#line 2314 "parser.cc"
    break;

  case 83: /* jump_statement: PARA SEMIC  */
#line 225 "rules/parser.y"
                            { (yyval.sttmt) = new ast::JumpSttmt("break"); }
#line 2320 "parser.cc"
    break;

  case 84: /* jump_statement: CONTINUA SEMIC  */
#line 226 "rules/parser.y"
                                { (yyval.sttmt) = new ast::JumpSttmt("continue"); }
#line 2326 "parser.cc"
    break;

  case 85: /* jump_statement: DIVOLVI expression SEMIC  */
#line 227 "rules/parser.y"
                                          { (yyval.sttmt) = new ast::ReturnSttmt(std::unique_ptr<ast::Expr>((yyvsp[-1].expr))); }
#line 2332 "parser.cc"
    break;

  case 86: /* jump_statement: DIVOLVI SEMIC  */
#line 228 "rules/parser.y"
                               { (yyval.sttmt) = new ast::ReturnSttmt(nullptr); }
#line 2338 "parser.cc"
    break;


#line 2342 "parser.cc"

      default: break;
    }
  /* User semantic actions sometimes alter yychar, and that requires
     that yytoken be updated with the new translation.  We take the
     approach of translating immediately before every use of yytoken.
     One alternative is translating here after every semantic action,
     but that translation would be missed if the semantic action invokes
     YYABORT, YYACCEPT, or YYERROR immediately after altering yychar or
     if it invokes YYBACKUP.  In the case of YYABORT or YYACCEPT, an
     incorrect destructor might then be invoked immediately.  In the
     case of YYERROR or YYBACKUP, subsequent parser actions might lead
     to an incorrect destructor call or verbose syntax error message
     before the lookahead is translated.  */
  YY_SYMBOL_PRINT ("-> $$ =", YY_CAST (yysymbol_kind_t, yyr1[yyn]), &yyval, &yyloc);

  YYPOPSTACK (yylen);
  yylen = 0;

  *++yyvsp = yyval;

  /* Now 'shift' the result of the reduction.  Determine what state
     that goes to, based on the state we popped back to and the rule
     number reduced by.  */
  {
    const int yylhs = yyr1[yyn] - YYNTOKENS;
    const int yyi = yypgoto[yylhs] + *yyssp;
    yystate = (0 <= yyi && yyi <= YYLAST && yycheck[yyi] == *yyssp
               ? yytable[yyi]
               : yydefgoto[yylhs]);
  }

  goto yynewstate;


/*--------------------------------------.
| yyerrlab -- here on detecting error.  |
`--------------------------------------*/
yyerrlab:
  /* Make sure we have latest lookahead translation.  See comments at
     user semantic actions for why this is necessary.  */
  yytoken = yychar == YYEMPTY ? YYSYMBOL_YYEMPTY : YYTRANSLATE (yychar);
  /* If not already recovering from an error, report this error.  */
  if (!yyerrstatus)
    {
      ++yynerrs;
      {
        yypcontext_t yyctx
          = {yyssp, yytoken};
        char const *yymsgp = YY_("syntax error");
        int yysyntax_error_status;
        yysyntax_error_status = yysyntax_error (&yymsg_alloc, &yymsg, &yyctx);
        if (yysyntax_error_status == 0)
          yymsgp = yymsg;
        else if (yysyntax_error_status == -1)
          {
            if (yymsg != yymsgbuf)
              YYSTACK_FREE (yymsg);
            yymsg = YY_CAST (char *,
                             YYSTACK_ALLOC (YY_CAST (YYSIZE_T, yymsg_alloc)));
            if (yymsg)
              {
                yysyntax_error_status
                  = yysyntax_error (&yymsg_alloc, &yymsg, &yyctx);
                yymsgp = yymsg;
              }
            else
              {
                yymsg = yymsgbuf;
                yymsg_alloc = sizeof yymsgbuf;
                yysyntax_error_status = YYENOMEM;
              }
          }
        yyerror (Program, yymsgp);
        if (yysyntax_error_status == YYENOMEM)
          YYNOMEM;
      }
    }

  if (yyerrstatus == 3)
    {
      /* If just tried and failed to reuse lookahead token after an
         error, discard it.  */

      if (yychar <= YYEOF)
        {
          /* Return failure if at end of input.  */
          if (yychar == YYEOF)
            YYABORT;
        }
      else
        {
          yydestruct ("Error: discarding",
                      yytoken, &yylval, Program);
          yychar = YYEMPTY;
        }
    }

  /* Else will try to reuse lookahead token after shifting the error
     token.  */
  goto yyerrlab1;


/*---------------------------------------------------.
| yyerrorlab -- error raised explicitly by YYERROR.  |
`---------------------------------------------------*/
yyerrorlab:
  /* Pacify compilers when the user code never invokes YYERROR and the
     label yyerrorlab therefore never appears in user code.  */
  if (0)
    YYERROR;
  ++yynerrs;

  /* Do not reclaim the symbols of the rule whose action triggered
     this YYERROR.  */
  YYPOPSTACK (yylen);
  yylen = 0;
  YY_STACK_PRINT (yyss, yyssp);
  yystate = *yyssp;
  goto yyerrlab1;


/*-------------------------------------------------------------.
| yyerrlab1 -- common code for both syntax error and YYERROR.  |
`-------------------------------------------------------------*/
yyerrlab1:
  yyerrstatus = 3;      /* Each real token shifted decrements this.  */

  /* Pop stack until we find a state that shifts the error token.  */
  for (;;)
    {
      yyn = yypact[yystate];
      if (!yypact_value_is_default (yyn))
        {
          yyn += YYSYMBOL_YYerror;
          if (0 <= yyn && yyn <= YYLAST && yycheck[yyn] == YYSYMBOL_YYerror)
            {
              yyn = yytable[yyn];
              if (0 < yyn)
                break;
            }
        }

      /* Pop the current state because it cannot handle the error token.  */
      if (yyssp == yyss)
        YYABORT;


      yydestruct ("Error: popping",
                  YY_ACCESSING_SYMBOL (yystate), yyvsp, Program);
      YYPOPSTACK (1);
      yystate = *yyssp;
      YY_STACK_PRINT (yyss, yyssp);
    }

  YY_IGNORE_MAYBE_UNINITIALIZED_BEGIN
  *++yyvsp = yylval;
  YY_IGNORE_MAYBE_UNINITIALIZED_END


  /* Shift the error token.  */
  YY_SYMBOL_PRINT ("Shifting", YY_ACCESSING_SYMBOL (yyn), yyvsp, yylsp);

  yystate = yyn;
  goto yynewstate;


/*-------------------------------------.
| yyacceptlab -- YYACCEPT comes here.  |
`-------------------------------------*/
yyacceptlab:
  yyresult = 0;
  goto yyreturnlab;


/*-----------------------------------.
| yyabortlab -- YYABORT comes here.  |
`-----------------------------------*/
yyabortlab:
  yyresult = 1;
  goto yyreturnlab;


/*-----------------------------------------------------------.
| yyexhaustedlab -- YYNOMEM (memory exhaustion) comes here.  |
`-----------------------------------------------------------*/
yyexhaustedlab:
  yyerror (Program, YY_("memory exhausted"));
  yyresult = 2;
  goto yyreturnlab;


/*----------------------------------------------------------.
| yyreturnlab -- parsing is finished, clean up and return.  |
`----------------------------------------------------------*/
yyreturnlab:
  if (yychar != YYEMPTY)
    {
      /* Make sure we have latest lookahead translation.  See comments at
         user semantic actions for why this is necessary.  */
      yytoken = YYTRANSLATE (yychar);
      yydestruct ("Cleanup: discarding lookahead",
                  yytoken, &yylval, Program);
    }
  /* Do not reclaim the symbols of the rule whose action triggered
     this YYABORT or YYACCEPT.  */
  YYPOPSTACK (yylen);
  YY_STACK_PRINT (yyss, yyssp);
  while (yyssp != yyss)
    {
      yydestruct ("Cleanup: popping",
                  YY_ACCESSING_SYMBOL (+*yyssp), yyvsp, Program);
      YYPOPSTACK (1);
    }
#ifndef yyoverflow
  if (yyss != yyssa)
    YYSTACK_FREE (yyss);
#endif
  if (yymsg != yymsgbuf)
    YYSTACK_FREE (yymsg);
  return yyresult;
}

#line 230 "rules/parser.y"


void yyerror(kriol::ast::BlockSttmt** Program, const char* err) {
    kriol::cli::PrintErr(err, 1);
}
