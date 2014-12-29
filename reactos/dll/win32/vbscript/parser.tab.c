/* A Bison parser, made by GNU Bison 3.0.2.  */

/* Bison implementation for Yacc-like parsers in C

   Copyright (C) 1984, 1989-1990, 2000-2013 Free Software Foundation, Inc.

   This program is free software: you can redistribute it and/or modify
   it under the terms of the GNU General Public License as published by
   the Free Software Foundation, either version 3 of the License, or
   (at your option) any later version.

   This program is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
   GNU General Public License for more details.

   You should have received a copy of the GNU General Public License
   along with this program.  If not, see <http://www.gnu.org/licenses/>.  */

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

/* All symbols defined below should begin with yy or YY, to avoid
   infringing on user name space.  This should be done even for local
   variables, as they might otherwise be expanded by user macros.
   There are some unavoidable exceptions within include files to
   define necessary library symbols; they are noted "INFRINGES ON
   USER NAME SPACE" below.  */

/* Identify Bison output.  */
#define YYBISON 1

/* Bison version.  */
#define YYBISON_VERSION "3.0.2"

/* Skeleton name.  */
#define YYSKELETON_NAME "yacc.c"

/* Pure parsers.  */
#define YYPURE 1

/* Push parsers.  */
#define YYPUSH 0

/* Pull parsers.  */
#define YYPULL 1


/* Substitute the variable and function names.  */
#define yyparse         parser_parse
#define yylex           parser_lex
#define yyerror         parser_error
#define yydebug         parser_debug
#define yynerrs         parser_nerrs


/* Copy the first part of user declarations.  */
#line 19 "parser.y" /* yacc.c:339  */


#include "vbscript.h"

static int parser_error(parser_ctx_t *,const char*);

static void parse_complete(parser_ctx_t*,BOOL);

static void source_add_statement(parser_ctx_t*,statement_t*);
static void source_add_class(parser_ctx_t*,class_decl_t*);

static void *new_expression(parser_ctx_t*,expression_type_t,size_t);
static expression_t *new_bool_expression(parser_ctx_t*,VARIANT_BOOL);
static expression_t *new_string_expression(parser_ctx_t*,const WCHAR*);
static expression_t *new_long_expression(parser_ctx_t*,expression_type_t,LONG);
static expression_t *new_double_expression(parser_ctx_t*,double);
static expression_t *new_unary_expression(parser_ctx_t*,expression_type_t,expression_t*);
static expression_t *new_binary_expression(parser_ctx_t*,expression_type_t,expression_t*,expression_t*);
static expression_t *new_new_expression(parser_ctx_t*,const WCHAR*);

static member_expression_t *new_member_expression(parser_ctx_t*,expression_t*,const WCHAR*);

static void *new_statement(parser_ctx_t*,statement_type_t,size_t);
static statement_t *new_call_statement(parser_ctx_t*,BOOL,member_expression_t*);
static statement_t *new_assign_statement(parser_ctx_t*,member_expression_t*,expression_t*);
static statement_t *new_set_statement(parser_ctx_t*,member_expression_t*,expression_t*);
static statement_t *new_dim_statement(parser_ctx_t*,dim_decl_t*);
static statement_t *new_while_statement(parser_ctx_t*,statement_type_t,expression_t*,statement_t*);
static statement_t *new_forto_statement(parser_ctx_t*,const WCHAR*,expression_t*,expression_t*,expression_t*,statement_t*);
static statement_t *new_foreach_statement(parser_ctx_t*,const WCHAR*,expression_t*,statement_t*);
static statement_t *new_if_statement(parser_ctx_t*,expression_t*,statement_t*,elseif_decl_t*,statement_t*);
static statement_t *new_function_statement(parser_ctx_t*,function_decl_t*);
static statement_t *new_onerror_statement(parser_ctx_t*,BOOL);
static statement_t *new_const_statement(parser_ctx_t*,const_decl_t*);
static statement_t *new_select_statement(parser_ctx_t*,expression_t*,case_clausule_t*);

static dim_decl_t *new_dim_decl(parser_ctx_t*,const WCHAR*,BOOL,dim_list_t*);
static dim_list_t *new_dim(parser_ctx_t*,unsigned,dim_list_t*);
static elseif_decl_t *new_elseif_decl(parser_ctx_t*,expression_t*,statement_t*);
static function_decl_t *new_function_decl(parser_ctx_t*,const WCHAR*,function_type_t,unsigned,arg_decl_t*,statement_t*);
static arg_decl_t *new_argument_decl(parser_ctx_t*,const WCHAR*,BOOL);
static const_decl_t *new_const_decl(parser_ctx_t*,const WCHAR*,expression_t*);
static case_clausule_t *new_case_clausule(parser_ctx_t*,expression_t*,statement_t*,case_clausule_t*);

static class_decl_t *new_class_decl(parser_ctx_t*);
static class_decl_t *add_class_function(parser_ctx_t*,class_decl_t*,function_decl_t*);
static class_decl_t *add_dim_prop(parser_ctx_t*,class_decl_t*,dim_decl_t*,unsigned);

static statement_t *link_statements(statement_t*,statement_t*);

static const WCHAR propertyW[] = {'p','r','o','p','e','r','t','y',0};

#define STORAGE_IS_PRIVATE    1
#define STORAGE_IS_DEFAULT    2

#define CHECK_ERROR if(((parser_ctx_t*)ctx)->hres != S_OK) YYABORT


#line 136 "parser.tab.c" /* yacc.c:339  */

# ifndef YY_NULLPTR
#  if defined __cplusplus && 201103L <= __cplusplus
#   define YY_NULLPTR nullptr
#  else
#   define YY_NULLPTR 0
#  endif
# endif

/* Enabling verbose error messages.  */
#ifdef YYERROR_VERBOSE
# undef YYERROR_VERBOSE
# define YYERROR_VERBOSE 1
#else
# define YYERROR_VERBOSE 0
#endif


/* Debug traces.  */
#ifndef YYDEBUG
# define YYDEBUG 0
#endif
#if YYDEBUG
extern int parser_debug;
#endif

/* Token type.  */
#ifndef YYTOKENTYPE
# define YYTOKENTYPE
  enum yytokentype
  {
    tEOF = 258,
    tNL = 259,
    tREM = 260,
    tEMPTYBRACKETS = 261,
    tTRUE = 262,
    tFALSE = 263,
    tNOT = 264,
    tAND = 265,
    tOR = 266,
    tXOR = 267,
    tEQV = 268,
    tIMP = 269,
    tNEQ = 270,
    tIS = 271,
    tLTEQ = 272,
    tGTEQ = 273,
    tMOD = 274,
    tCALL = 275,
    tDIM = 276,
    tSUB = 277,
    tFUNCTION = 278,
    tPROPERTY = 279,
    tGET = 280,
    tLET = 281,
    tCONST = 282,
    tIF = 283,
    tELSE = 284,
    tELSEIF = 285,
    tEND = 286,
    tTHEN = 287,
    tEXIT = 288,
    tWHILE = 289,
    tWEND = 290,
    tDO = 291,
    tLOOP = 292,
    tUNTIL = 293,
    tFOR = 294,
    tTO = 295,
    tSTEP = 296,
    tEACH = 297,
    tIN = 298,
    tSELECT = 299,
    tCASE = 300,
    tBYREF = 301,
    tBYVAL = 302,
    tOPTION = 303,
    tEXPLICIT = 304,
    tSTOP = 305,
    tNOTHING = 306,
    tEMPTY = 307,
    tNULL = 308,
    tCLASS = 309,
    tSET = 310,
    tNEW = 311,
    tPUBLIC = 312,
    tPRIVATE = 313,
    tDEFAULT = 314,
    tME = 315,
    tERROR = 316,
    tNEXT = 317,
    tON = 318,
    tRESUME = 319,
    tGOTO = 320,
    tIdentifier = 321,
    tString = 322,
    tLong = 323,
    tShort = 324,
    tDouble = 325
  };
#endif

/* Value type.  */
#if ! defined YYSTYPE && ! defined YYSTYPE_IS_DECLARED
typedef union YYSTYPE YYSTYPE;
union YYSTYPE
{
#line 88 "parser.y" /* yacc.c:355  */

    const WCHAR *string;
    statement_t *statement;
    expression_t *expression;
    member_expression_t *member;
    elseif_decl_t *elseif;
    dim_decl_t *dim_decl;
    dim_list_t *dim_list;
    function_decl_t *func_decl;
    arg_decl_t *arg_decl;
    class_decl_t *class_decl;
    const_decl_t *const_decl;
    case_clausule_t *case_clausule;
    unsigned uint;
    LONG lng;
    BOOL boolean;
    double dbl;

#line 263 "parser.tab.c" /* yacc.c:355  */
};
# define YYSTYPE_IS_TRIVIAL 1
# define YYSTYPE_IS_DECLARED 1
#endif



int parser_parse (parser_ctx_t *ctx);



/* Copy the second part of user declarations.  */

#line 277 "parser.tab.c" /* yacc.c:358  */

#ifdef short
# undef short
#endif

#ifdef YYTYPE_UINT8
typedef YYTYPE_UINT8 yytype_uint8;
#else
typedef unsigned char yytype_uint8;
#endif

#ifdef YYTYPE_INT8
typedef YYTYPE_INT8 yytype_int8;
#else
typedef signed char yytype_int8;
#endif

#ifdef YYTYPE_UINT16
typedef YYTYPE_UINT16 yytype_uint16;
#else
typedef unsigned short int yytype_uint16;
#endif

#ifdef YYTYPE_INT16
typedef YYTYPE_INT16 yytype_int16;
#else
typedef short int yytype_int16;
#endif

#ifndef YYSIZE_T
# ifdef __SIZE_TYPE__
#  define YYSIZE_T __SIZE_TYPE__
# elif defined size_t
#  define YYSIZE_T size_t
# elif ! defined YYSIZE_T
#  include <stddef.h> /* INFRINGES ON USER NAME SPACE */
#  define YYSIZE_T size_t
# else
#  define YYSIZE_T unsigned int
# endif
#endif

#define YYSIZE_MAXIMUM ((YYSIZE_T) -1)

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

#ifndef YY_ATTRIBUTE
# if (defined __GNUC__                                               \
      && (2 < __GNUC__ || (__GNUC__ == 2 && 96 <= __GNUC_MINOR__)))  \
     || defined __SUNPRO_C && 0x5110 <= __SUNPRO_C
#  define YY_ATTRIBUTE(Spec) __attribute__(Spec)
# else
#  define YY_ATTRIBUTE(Spec) /* empty */
# endif
#endif

#ifndef YY_ATTRIBUTE_PURE
# define YY_ATTRIBUTE_PURE   YY_ATTRIBUTE ((__pure__))
#endif

#ifndef YY_ATTRIBUTE_UNUSED
# define YY_ATTRIBUTE_UNUSED YY_ATTRIBUTE ((__unused__))
#endif

#if !defined _Noreturn \
     && (!defined __STDC_VERSION__ || __STDC_VERSION__ < 201112)
# if defined _MSC_VER && 1200 <= _MSC_VER
#  define _Noreturn __declspec (noreturn)
# else
#  define _Noreturn YY_ATTRIBUTE ((__noreturn__))
# endif
#endif

/* Suppress unused-variable warnings by "using" E.  */
#if ! defined lint || defined __GNUC__
# define YYUSE(E) ((void) (E))
#else
# define YYUSE(E) /* empty */
#endif

#if defined __GNUC__ && 407 <= __GNUC__ * 100 + __GNUC_MINOR__
/* Suppress an incorrect diagnostic about yylval being uninitialized.  */
# define YY_IGNORE_MAYBE_UNINITIALIZED_BEGIN \
    _Pragma ("GCC diagnostic push") \
    _Pragma ("GCC diagnostic ignored \"-Wuninitialized\"")\
    _Pragma ("GCC diagnostic ignored \"-Wmaybe-uninitialized\"")
# define YY_IGNORE_MAYBE_UNINITIALIZED_END \
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


#if ! defined yyoverflow || YYERROR_VERBOSE

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
#endif /* ! defined yyoverflow || YYERROR_VERBOSE */


#if (! defined yyoverflow \
     && (! defined __cplusplus \
         || (defined YYSTYPE_IS_TRIVIAL && YYSTYPE_IS_TRIVIAL)))

/* A type that is properly aligned for any stack member.  */
union yyalloc
{
  yytype_int16 yyss_alloc;
  YYSTYPE yyvs_alloc;
};

/* The size of the maximum gap between one aligned stack and the next.  */
# define YYSTACK_GAP_MAXIMUM (sizeof (union yyalloc) - 1)

/* The size of an array large to enough to hold all stacks, each with
   N elements.  */
# define YYSTACK_BYTES(N) \
     ((N) * (sizeof (yytype_int16) + sizeof (YYSTYPE)) \
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
        YYSIZE_T yynewbytes;                                            \
        YYCOPY (&yyptr->Stack_alloc, Stack, yysize);                    \
        Stack = &yyptr->Stack_alloc;                                    \
        yynewbytes = yystacksize * sizeof (*Stack) + YYSTACK_GAP_MAXIMUM; \
        yyptr += yynewbytes / sizeof (*yyptr);                          \
      }                                                                 \
    while (0)

#endif

#if defined YYCOPY_NEEDED && YYCOPY_NEEDED
/* Copy COUNT objects from SRC to DST.  The source and destination do
   not overlap.  */
# ifndef YYCOPY
#  if defined __GNUC__ && 1 < __GNUC__
#   define YYCOPY(Dst, Src, Count) \
      __builtin_memcpy (Dst, Src, (Count) * sizeof (*(Src)))
#  else
#   define YYCOPY(Dst, Src, Count)              \
      do                                        \
        {                                       \
          YYSIZE_T yyi;                         \
          for (yyi = 0; yyi < (Count); yyi++)   \
            (Dst)[yyi] = (Src)[yyi];            \
        }                                       \
      while (0)
#  endif
# endif
#endif /* !YYCOPY_NEEDED */

/* YYFINAL -- State number of the termination state.  */
#define YYFINAL  5
/* YYLAST -- Last index in YYTABLE.  */
#define YYLAST   862

/* YYNTOKENS -- Number of terminals.  */
#define YYNTOKENS  87
/* YYNNTS -- Number of nonterminals.  */
#define YYNNTS  59
/* YYNRULES -- Number of rules.  */
#define YYNRULES  164
/* YYNSTATES -- Number of states.  */
#define YYNSTATES  335

/* YYTRANSLATE[YYX] -- Symbol number corresponding to YYX as returned
   by yylex, with out-of-bounds checking.  */
#define YYUNDEFTOK  2
#define YYMAXUTOK   325

#define YYTRANSLATE(YYX)                                                \
  ((unsigned int) (YYX) <= YYMAXUTOK ? yytranslate[YYX] : YYUNDEFTOK)

/* YYTRANSLATE[TOKEN-NUM] -- Symbol number corresponding to TOKEN-NUM
   as returned by yylex, without out-of-bounds checking.  */
static const yytype_uint8 yytranslate[] =
{
       0,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,    81,     2,
      76,    77,    84,    82,    75,    78,    74,    85,    73,     2,
       2,     2,     2,     2,     2,     2,     2,     2,    71,     2,
      80,    72,    79,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,    83,     2,    86,     2,     2,     2,     2,     2,
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
      45,    46,    47,    48,    49,    50,    51,    52,    53,    54,
      55,    56,    57,    58,    59,    60,    61,    62,    63,    64,
      65,    66,    67,    68,    69,    70
};

#if YYDEBUG
  /* YYRLINE[YYN] -- Source line where rule number YYN was defined.  */
static const yytype_uint16 yyrline[] =
{
       0,   147,   147,   150,   151,   153,   155,   156,   159,   160,
     163,   164,   167,   170,   171,   172,   173,   174,   177,   178,
     179,   181,   182,   183,   185,   188,   191,   192,   193,   194,
     195,   196,   197,   198,   200,   201,   202,   203,   204,   206,
     208,   212,   213,   216,   217,   220,   221,   222,   225,   226,
     229,   230,   233,   236,   237,   240,   241,   244,   245,   248,
     250,   251,   254,   256,   259,   260,   263,   264,   267,   271,
     272,   275,   276,   277,   281,   282,   285,   286,   288,   290,
     293,   294,   297,   298,   301,   302,   305,   306,   309,   310,
     313,   314,   317,   318,   321,   322,   323,   324,   325,   326,
     327,   328,   331,   332,   335,   336,   337,   340,   341,   344,
     345,   349,   350,   352,   356,   357,   360,   361,   362,   363,
     366,   367,   370,   371,   372,   373,   374,   375,   376,   379,
     380,   381,   382,   385,   386,   387,   390,   391,   394,   397,
     398,   400,   402,   403,   406,   408,   410,   414,   416,   420,
     421,   424,   425,   426,   429,   430,   433,   434,   437,   438,
     439,   443,   444,   448,   449
};
#endif

#if YYDEBUG || YYERROR_VERBOSE || 0
/* YYTNAME[SYMBOL-NUM] -- String name of the symbol SYMBOL-NUM.
   First, the terminals, then, starting at YYNTOKENS, nonterminals.  */
static const char *const yytname[] =
{
  "$end", "error", "$undefined", "tEOF", "tNL", "tREM", "tEMPTYBRACKETS",
  "tTRUE", "tFALSE", "tNOT", "tAND", "tOR", "tXOR", "tEQV", "tIMP", "tNEQ",
  "tIS", "tLTEQ", "tGTEQ", "tMOD", "tCALL", "tDIM", "tSUB", "tFUNCTION",
  "tPROPERTY", "tGET", "tLET", "tCONST", "tIF", "tELSE", "tELSEIF", "tEND",
  "tTHEN", "tEXIT", "tWHILE", "tWEND", "tDO", "tLOOP", "tUNTIL", "tFOR",
  "tTO", "tSTEP", "tEACH", "tIN", "tSELECT", "tCASE", "tBYREF", "tBYVAL",
  "tOPTION", "tEXPLICIT", "tSTOP", "tNOTHING", "tEMPTY", "tNULL", "tCLASS",
  "tSET", "tNEW", "tPUBLIC", "tPRIVATE", "tDEFAULT", "tME", "tERROR",
  "tNEXT", "tON", "tRESUME", "tGOTO", "tIdentifier", "tString", "tLong",
  "tShort", "tDouble", "':'", "'='", "'0'", "'.'", "','", "'('", "')'",
  "'-'", "'>'", "'<'", "'&'", "'+'", "'\\\\'", "'*'", "'/'", "'^'",
  "$accept", "Program", "OptionExplicit_opt", "SourceElements",
  "StatementsNl_opt", "StatementsNl", "StatementNl", "Statement",
  "SimpleStatement", "MemberExpression", "DimDeclList", "DimDecl",
  "DimList", "ConstDeclList", "ConstDecl", "ConstExpression", "DoType",
  "Step_opt", "IfStatement", "EndIf_opt", "ElseIfs_opt", "ElseIfs",
  "ElseIf", "Else_opt", "CaseClausules", "Arguments_opt",
  "ArgumentList_opt", "EmptyBrackets_opt", "ExpressionList", "Expression",
  "EqvExpression", "XorExpression", "OrExpression", "AndExpression",
  "NotExpression", "EqualityExpression", "ConcatExpression",
  "AdditiveExpression", "ModExpression", "IntdivExpression",
  "MultiplicativeExpression", "ExpExpression", "UnaryExpression",
  "CallExpression", "LiteralExpression", "NumericLiteralExpression",
  "IntegerValue", "PrimaryExpression", "ClassDeclaration", "ClassBody",
  "PropertyDecl", "FunctionDecl", "Storage_opt", "Storage",
  "ArgumentsDecl_opt", "ArgumentDeclList", "ArgumentDecl", "Identifier",
  "StSep", YY_NULLPTR
};
#endif

# ifdef YYPRINT
/* YYTOKNUM[NUM] -- (External) token number corresponding to the
   (internal) symbol number NUM (which must be that of a token).  */
static const yytype_uint16 yytoknum[] =
{
       0,   256,   257,   258,   259,   260,   261,   262,   263,   264,
     265,   266,   267,   268,   269,   270,   271,   272,   273,   274,
     275,   276,   277,   278,   279,   280,   281,   282,   283,   284,
     285,   286,   287,   288,   289,   290,   291,   292,   293,   294,
     295,   296,   297,   298,   299,   300,   301,   302,   303,   304,
     305,   306,   307,   308,   309,   310,   311,   312,   313,   314,
     315,   316,   317,   318,   319,   320,   321,   322,   323,   324,
     325,    58,    61,    48,    46,    44,    40,    41,    45,    62,
      60,    38,    43,    92,    42,    47,    94
};
# endif

#define YYPACT_NINF -158

#define yypact_value_is_default(Yystate) \
  (!!((Yystate) == (-158)))

#define YYTABLE_NINF -150

#define yytable_value_is_error(Yytable_value) \
  0

  /* YYPACT[STATE-NUM] -- Index in YYTABLE of the portion describing
     STATE-NUM.  */
static const yytype_int16 yypact[] =
{
     -28,   -18,    43,  -158,    56,  -158,   307,  -158,  -158,     6,
      -2,  -158,    -2,   468,   184,   468,    20,    33,    32,  -158,
      -2,     6,    26,  -158,  -158,    50,  -158,   557,   468,  -158,
     116,    51,   388,  -158,    62,  -158,  -158,  -158,   130,  -158,
    -158,     8,  -158,    63,    10,  -158,    93,   104,  -158,  -158,
     468,  -158,  -158,  -158,    -2,  -158,  -158,  -158,  -158,  -158,
     498,     8,    24,   166,   171,   177,   189,  -158,    29,   113,
      53,   182,   122,   102,   129,  -158,    62,  -158,  -158,  -158,
    -158,  -158,  -158,  -158,    47,   598,  -158,  -158,   468,    -2,
     145,   468,   214,     8,  -158,   127,  -158,    11,  -158,   557,
    -158,   468,   152,  -158,    -5,  -158,    19,    -2,    -2,    -2,
     468,   163,  -158,    -2,  -158,   112,    -2,    97,  -158,  -158,
    -158,  -158,   468,   348,   468,   468,   468,   468,   498,   498,
     498,   498,   498,   498,   498,   498,   498,   498,   498,   498,
     498,   498,   498,   651,   201,  -158,   557,    48,   200,   468,
      36,   120,   173,   185,   175,  -158,  -158,   169,    35,   468,
     468,  -158,    13,    13,  -158,  -158,  -158,  -158,   176,   179,
    -158,    45,  -158,  -158,   166,   557,   221,   171,   177,   189,
    -158,   113,   113,   113,   113,   113,   113,   113,    53,   182,
     182,   122,   102,   129,   129,  -158,   217,   121,  -158,   598,
     468,    25,  -158,  -158,   211,    -2,   228,   256,   257,   150,
     196,   468,  -158,  -158,  -158,   249,  -158,   -11,  -158,   260,
     261,  -158,   112,  -158,   237,   786,  -158,   468,   233,    59,
     468,   418,   240,   268,   219,   120,   120,   135,   270,   249,
      -2,    -2,   198,   202,   277,   692,   692,  -158,   468,   247,
    -158,   237,   253,   249,  -158,   733,    12,     7,     7,   242,
     120,   283,  -158,  -158,   224,   225,   226,   120,   277,   277,
    -158,   -11,  -158,   263,   265,    27,   293,   271,  -158,   273,
    -158,   243,   468,   295,   786,   557,  -158,  -158,  -158,    13,
     231,   236,  -158,  -158,  -158,  -158,   282,   286,   310,   692,
     287,  -158,  -158,   249,   733,  -158,   211,   312,   -11,   -11,
    -158,  -158,   557,  -158,  -158,   255,  -158,   692,   241,   252,
    -158,  -158,   288,   320,   326,   308,   692,   692,  -158,   302,
     305,   314,   315,  -158,  -158
};

  /* YYDEFACT[STATE-NUM] -- Default reduction number in state STATE-NUM.
     Performed when YYTABLE does not specify something else to do.  Zero
     means the default is an error.  */
static const yytype_uint8 yydefact[] =
{
       3,     0,     0,     5,     0,     1,   149,     4,     2,     0,
       0,   162,     0,     0,     0,     0,     0,     0,     0,    34,
       0,     0,   152,   153,   137,     0,   161,    13,     0,     6,
       0,    15,    78,    22,     0,   120,     7,    27,     0,   150,
      41,    78,    21,    43,    45,    37,    50,     0,   122,   123,
       0,   128,   126,   127,     0,   124,   131,   129,   132,   130,
       0,    78,     0,    82,    84,    86,    88,    90,    92,    94,
     102,   104,   107,   109,   111,   114,   117,   116,   125,    32,
      30,    31,    28,    29,     0,   149,    55,    56,     0,     0,
       0,     0,     0,    78,   151,     0,    14,     0,    12,    17,
      79,     0,   121,    18,    76,    77,    80,     0,     0,     0,
       0,    19,    74,     0,    47,     0,     0,     0,    93,   118,
     119,   121,     0,   149,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,   149,     0,     9,    10,     0,     0,     0,
       0,   149,   121,     0,     0,   136,    16,     0,     0,     0,
       0,    42,    78,    78,    44,   135,   133,   134,     0,    48,
      51,     0,    52,    53,    83,     8,    60,    85,    87,    89,
      91,    96,   101,   100,    99,    95,    97,    98,   103,   106,
     105,   108,   110,   112,   113,   115,     0,    26,    11,   149,
       0,     0,   163,   164,    71,     0,     0,     0,     0,     0,
     150,     0,    35,    36,    75,    20,    81,     0,   154,     0,
       0,    46,     0,    54,    64,   149,    23,     0,     0,     0,
       0,     0,     0,     0,     0,   149,   149,     0,     0,    33,
       0,     0,     0,   156,    78,   149,   149,    49,     0,    69,
      65,    66,    62,    25,    24,   149,    57,     0,     0,     0,
     149,     0,   143,   140,     0,     0,     0,   149,    78,    78,
     155,     0,   158,     0,     0,     0,     0,     0,    67,     0,
      61,     0,     0,     0,   149,     8,    40,   142,   138,    78,
       0,     0,   141,   159,   160,   157,     0,     0,     0,   149,
       0,    63,    39,    58,   149,    72,    71,     0,     0,     0,
     147,   148,     8,    70,    59,     0,    73,   149,     0,     0,
      68,    38,     0,     0,     0,     0,   149,   149,   144,     0,
       0,     0,     0,   145,   146
};

  /* YYPGOTO[NTERM-NUM].  */
static const yytype_int16 yypgoto[] =
{
    -158,  -158,  -158,  -158,  -101,  -137,   336,   -23,  -158,    -6,
     232,   139,   125,   234,  -158,  -158,   151,  -158,  -158,  -158,
    -158,    98,  -158,  -158,    49,   -13,  -158,     0,   -31,    55,
     238,   229,   246,   230,   -40,  -158,    99,   223,    60,   216,
     220,    69,   -52,    -4,   262,   192,  -158,  -158,  -158,   -78,
    -158,  -144,  -139,  -133,  -157,    95,   -96,    17,   -22
};

  /* YYDEFGOTO[NTERM-NUM].  */
static const yytype_int16 yydefgoto[] =
{
      -1,     2,     3,     6,   144,   145,   146,    30,    31,    61,
      42,    43,   168,    45,    46,   172,    88,   283,    33,   280,
     249,   250,   251,   277,   232,   102,   103,   112,   157,   106,
      63,    64,    65,    66,    67,    68,    69,    70,    71,    72,
      73,    74,    75,    76,    77,    78,   169,    35,    36,   206,
     207,    37,    38,    39,   219,   242,   243,    40,   204
};

  /* YYTABLE[YYPACT[STATE-NUM]] -- What to do in state STATE-NUM.  If
     positive, shift that token.  If negative, reduce the rule whose
     number is the opposite.  If YYTABLE_NINF, syntax error.  */
static const yytype_int16 yytable[] =
{
      32,   105,    34,    41,    96,    34,   220,   208,   120,   198,
     118,   202,   209,    11,   100,    93,   114,    34,   210,   100,
       1,    32,    11,    34,    85,   122,   122,    44,   111,    47,
      11,     4,   104,   122,    90,   240,   241,    92,   122,   122,
     202,   122,   196,     5,   128,   129,   130,   131,   121,   122,
     122,   143,   199,   282,    86,    26,   123,    11,    87,   298,
       7,   122,   122,   255,    26,   230,    24,   -74,    62,   -74,
      84,   119,    26,   122,   224,    89,   156,    91,   203,    32,
     152,    34,    28,    97,   110,    94,   115,   180,   155,   217,
     195,   208,   208,    32,   160,    34,   209,   209,   228,    26,
     176,   132,   210,   210,    48,    49,   148,   203,   133,   134,
     160,    95,   155,    56,    57,    58,   208,    32,    59,    34,
      98,   209,    99,   208,   161,   162,   163,   210,   209,   216,
      44,   136,   307,    47,   210,   137,   107,    32,   113,    34,
      32,   205,    34,   147,   273,   274,   150,   305,    51,    52,
      53,  -139,   108,   109,   281,    86,   158,   262,   263,    87,
     264,   265,   218,   218,    55,    56,    57,    58,   116,    32,
      59,    34,   108,   109,   237,   171,   117,    22,    23,   124,
     165,   166,   287,   125,   306,   167,   140,   141,   126,   292,
     266,   153,   154,    32,   135,    34,   189,   190,   313,   127,
     258,   138,   252,   315,   201,   139,    79,    80,    81,   193,
     194,   320,   318,   319,   215,   142,   322,   149,   151,    32,
      82,    34,    44,    83,   159,   329,   330,   181,   182,   183,
     184,   185,   186,   187,   244,   284,   285,  -121,   197,    32,
      32,    34,    34,   200,   272,   211,   214,   212,   213,    32,
     225,    34,   226,   221,   222,   229,   231,   268,   269,   234,
     235,   236,   238,   122,   245,   246,   239,   248,   293,   294,
     254,   259,   260,   261,   267,   270,   276,   271,    32,    32,
      34,    34,   253,   100,   279,   256,   286,   288,   244,   218,
     289,   290,   291,    32,   296,    34,   297,   299,    32,   304,
      34,   301,   300,   275,   310,   302,    32,   308,    34,   311,
       8,    32,   309,    34,   312,   314,   317,   321,   323,   325,
      32,    32,    34,    34,   326,   244,   244,     9,    10,   324,
     327,    11,   328,   331,    12,    13,   332,   303,   333,   334,
      14,    15,    29,    16,   233,   164,    17,   247,   227,   278,
     170,    18,   175,   177,   191,   316,   179,    19,   188,   192,
     174,    20,    21,   223,    22,    23,   295,    24,     9,    10,
      25,   178,    11,    26,     0,    12,    13,     0,    27,   173,
       0,    14,    15,    28,    16,     0,     0,    17,     0,     0,
       0,     0,    18,     0,   100,    48,    49,    50,    19,     0,
       0,     0,     0,    21,     0,    22,    23,     0,    24,     0,
       0,    25,    11,     0,    26,     0,     0,     0,     0,    27,
       0,     0,     0,     0,    28,    48,    49,    50,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,    51,
      52,    53,    11,     0,    54,     0,     0,   257,    24,     0,
       0,     0,     0,     0,    26,    55,    56,    57,    58,     0,
       0,    59,     0,     0,   101,     0,    60,     0,     0,    51,
      52,    53,     0,     0,    54,    48,    49,    50,    24,     0,
       0,     0,     0,     0,    26,    55,    56,    57,    58,     0,
       0,    59,    11,     0,    28,     0,    60,     0,     0,     0,
       0,     0,     0,     0,     0,    48,    49,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,    51,
      52,    53,    11,     0,    54,     0,     0,     0,    24,     0,
       0,     0,     0,     0,    26,    55,    56,    57,    58,     0,
       0,    59,     0,     0,    28,     0,    60,     0,     0,    51,
      52,    53,     0,     0,    54,     0,     0,     0,    24,     0,
       0,     0,     0,     0,    26,    55,    56,    57,    58,     0,
       0,    59,     0,     0,    28,     0,    60,     9,    10,  -149,
    -149,    11,     0,     0,    12,    13,     0,     0,     0,     0,
      14,    15,     0,    16,     0,     0,    17,     0,     0,     0,
       0,    18,     0,     0,     0,     0,     0,    19,     0,     0,
       0,     0,    21,     0,    22,    23,     0,    24,     9,    10,
      25,     0,    11,    26,     0,    12,    13,     0,    27,     0,
       0,    14,    15,    28,    16,    -8,     0,    17,     0,     0,
       0,     0,    18,     0,     0,     0,     0,     0,    19,     0,
       0,     0,     0,    21,     0,    22,    23,     0,    24,     0,
       0,    25,     0,     0,    26,     0,     0,     0,     0,    27,
       0,     9,    10,     0,    28,    11,     0,     0,    12,    13,
       0,     0,     0,     0,    14,    15,    -8,    16,     0,     0,
      17,     0,     0,     0,     0,    18,     0,     0,     0,     0,
       0,    19,     0,     0,     0,     0,    21,     0,    22,    23,
       0,    24,     9,    10,    25,     0,    11,    26,     0,    12,
      13,     0,    27,    -8,     0,    14,    15,    28,    16,     0,
       0,    17,     0,     0,     0,     0,    18,     0,     0,     0,
       0,     0,    19,     0,     0,     0,     0,    21,     0,    22,
      23,     0,    24,     9,    10,    25,     0,    11,    26,     0,
      12,    13,     0,    27,     0,     0,    14,    15,    28,    16,
       0,     0,    17,     0,     0,     0,     0,    18,     0,     0,
       0,     0,     0,    19,     0,     0,     0,     0,    21,     0,
      22,    23,     0,    24,     0,    -8,    25,     0,     0,    26,
       0,     0,     0,     0,    27,     0,     9,    10,     0,    28,
      11,     0,     0,    12,    13,     0,     0,     0,     0,    14,
      15,     0,    16,     0,     0,    17,     0,     0,     0,     0,
      18,     0,     0,     0,     0,     0,    19,     0,     0,     0,
       0,    21,     0,    22,    23,     0,    24,     0,     0,    25,
       0,     0,    26,     0,     0,     0,     0,    27,     0,     0,
       0,     0,    28
};

static const yytype_int16 yycheck[] =
{
       6,    32,     6,     9,    27,     9,   163,   151,    60,   146,
      50,     4,   151,    24,     6,    21,     6,    21,   151,     6,
      48,    27,    24,    27,     4,    14,    14,    10,    41,    12,
      24,    49,    32,    14,    17,    46,    47,    20,    14,    14,
       4,    14,   143,     0,    15,    16,    17,    18,    61,    14,
      14,     4,     4,    41,    34,    66,    32,    24,    38,    32,
       4,    14,    14,     4,    66,    40,    60,    72,    13,    74,
      15,    54,    66,    14,   175,    42,    99,    45,    71,    85,
      93,    85,    76,    28,    76,    59,    76,   127,    77,    76,
     142,   235,   236,    99,    75,    99,   235,   236,   199,    66,
     123,    72,   235,   236,     7,     8,    89,    71,    79,    80,
      75,    61,    77,    68,    69,    70,   260,   123,    73,   123,
       4,   260,    71,   267,   107,   108,   109,   260,   267,   160,
     113,    78,   289,   116,   267,    82,    74,   143,    75,   143,
     146,    21,   146,    88,   245,   246,    91,   284,    51,    52,
      53,    31,    22,    23,   255,    34,   101,   235,   236,    38,
      25,    26,   162,   163,    67,    68,    69,    70,    75,   175,
      73,   175,    22,    23,    24,    78,    72,    57,    58,    13,
      68,    69,   260,    12,   285,    73,    84,    85,    11,   267,
      55,    64,    65,   199,    81,   199,   136,   137,   299,    10,
     231,    19,   225,   304,   149,    83,    22,    23,    24,   140,
     141,   312,   308,   309,   159,    86,   317,    72,     4,   225,
      36,   225,   205,    39,    72,   326,   327,   128,   129,   130,
     131,   132,   133,   134,   217,   257,   258,    74,    37,   245,
     246,   245,   246,    43,   244,    72,    77,    62,    73,   255,
      29,   255,    35,    77,    75,   200,    45,   240,   241,    31,
       4,     4,    66,    14,     4,     4,   211,    30,   268,   269,
      37,    31,     4,    54,     4,    77,    29,    75,   284,   285,
     284,   285,   227,     6,    31,   230,    44,     4,   271,   289,
      66,    66,    66,   299,    31,   299,    31,     4,   304,     4,
     304,    28,    31,   248,    22,    62,   312,    76,   312,    23,
       3,   317,    76,   317,     4,    28,     4,    62,    77,    31,
     326,   327,   326,   327,     4,   308,   309,    20,    21,    77,
       4,    24,    24,    31,    27,    28,    31,   282,    24,    24,
      33,    34,     6,    36,   205,   113,    39,   222,   197,   251,
     116,    44,     4,   124,   138,   306,   126,    50,   135,   139,
     122,    54,    55,   171,    57,    58,   271,    60,    20,    21,
      63,   125,    24,    66,    -1,    27,    28,    -1,    71,   117,
      -1,    33,    34,    76,    36,    -1,    -1,    39,    -1,    -1,
      -1,    -1,    44,    -1,     6,     7,     8,     9,    50,    -1,
      -1,    -1,    -1,    55,    -1,    57,    58,    -1,    60,    -1,
      -1,    63,    24,    -1,    66,    -1,    -1,    -1,    -1,    71,
      -1,    -1,    -1,    -1,    76,     7,     8,     9,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    51,
      52,    53,    24,    -1,    56,    -1,    -1,    29,    60,    -1,
      -1,    -1,    -1,    -1,    66,    67,    68,    69,    70,    -1,
      -1,    73,    -1,    -1,    76,    -1,    78,    -1,    -1,    51,
      52,    53,    -1,    -1,    56,     7,     8,     9,    60,    -1,
      -1,    -1,    -1,    -1,    66,    67,    68,    69,    70,    -1,
      -1,    73,    24,    -1,    76,    -1,    78,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,     7,     8,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    51,
      52,    53,    24,    -1,    56,    -1,    -1,    -1,    60,    -1,
      -1,    -1,    -1,    -1,    66,    67,    68,    69,    70,    -1,
      -1,    73,    -1,    -1,    76,    -1,    78,    -1,    -1,    51,
      52,    53,    -1,    -1,    56,    -1,    -1,    -1,    60,    -1,
      -1,    -1,    -1,    -1,    66,    67,    68,    69,    70,    -1,
      -1,    73,    -1,    -1,    76,    -1,    78,    20,    21,    22,
      23,    24,    -1,    -1,    27,    28,    -1,    -1,    -1,    -1,
      33,    34,    -1,    36,    -1,    -1,    39,    -1,    -1,    -1,
      -1,    44,    -1,    -1,    -1,    -1,    -1,    50,    -1,    -1,
      -1,    -1,    55,    -1,    57,    58,    -1,    60,    20,    21,
      63,    -1,    24,    66,    -1,    27,    28,    -1,    71,    -1,
      -1,    33,    34,    76,    36,    37,    -1,    39,    -1,    -1,
      -1,    -1,    44,    -1,    -1,    -1,    -1,    -1,    50,    -1,
      -1,    -1,    -1,    55,    -1,    57,    58,    -1,    60,    -1,
      -1,    63,    -1,    -1,    66,    -1,    -1,    -1,    -1,    71,
      -1,    20,    21,    -1,    76,    24,    -1,    -1,    27,    28,
      -1,    -1,    -1,    -1,    33,    34,    35,    36,    -1,    -1,
      39,    -1,    -1,    -1,    -1,    44,    -1,    -1,    -1,    -1,
      -1,    50,    -1,    -1,    -1,    -1,    55,    -1,    57,    58,
      -1,    60,    20,    21,    63,    -1,    24,    66,    -1,    27,
      28,    -1,    71,    31,    -1,    33,    34,    76,    36,    -1,
      -1,    39,    -1,    -1,    -1,    -1,    44,    -1,    -1,    -1,
      -1,    -1,    50,    -1,    -1,    -1,    -1,    55,    -1,    57,
      58,    -1,    60,    20,    21,    63,    -1,    24,    66,    -1,
      27,    28,    -1,    71,    -1,    -1,    33,    34,    76,    36,
      -1,    -1,    39,    -1,    -1,    -1,    -1,    44,    -1,    -1,
      -1,    -1,    -1,    50,    -1,    -1,    -1,    -1,    55,    -1,
      57,    58,    -1,    60,    -1,    62,    63,    -1,    -1,    66,
      -1,    -1,    -1,    -1,    71,    -1,    20,    21,    -1,    76,
      24,    -1,    -1,    27,    28,    -1,    -1,    -1,    -1,    33,
      34,    -1,    36,    -1,    -1,    39,    -1,    -1,    -1,    -1,
      44,    -1,    -1,    -1,    -1,    -1,    50,    -1,    -1,    -1,
      -1,    55,    -1,    57,    58,    -1,    60,    -1,    -1,    63,
      -1,    -1,    66,    -1,    -1,    -1,    -1,    71,    -1,    -1,
      -1,    -1,    76
};

  /* YYSTOS[STATE-NUM] -- The (internal number of the) accessing
     symbol of state STATE-NUM.  */
static const yytype_uint8 yystos[] =
{
       0,    48,    88,    89,    49,     0,    90,     4,     3,    20,
      21,    24,    27,    28,    33,    34,    36,    39,    44,    50,
      54,    55,    57,    58,    60,    63,    66,    71,    76,    93,
      94,    95,    96,   105,   130,   134,   135,   138,   139,   140,
     144,    96,    97,    98,   144,   100,   101,   144,     7,     8,
       9,    51,    52,    53,    56,    67,    68,    69,    70,    73,
      78,    96,   116,   117,   118,   119,   120,   121,   122,   123,
     124,   125,   126,   127,   128,   129,   130,   131,   132,    22,
      23,    24,    36,    39,   116,     4,    34,    38,   103,    42,
     144,    45,   144,    96,    59,    61,    94,   116,     4,    71,
       6,    76,   112,   113,   114,   115,   116,    74,    22,    23,
      76,   112,   114,    75,     6,    76,    75,    72,   121,   144,
     129,   112,    14,    32,    13,    12,    11,    10,    15,    16,
      17,    18,    72,    79,    80,    81,    78,    82,    19,    83,
      84,    85,    86,     4,    91,    92,    93,   116,   144,    72,
     116,     4,   112,    64,    65,    77,    94,   115,   116,    72,
      75,   144,   144,   144,    97,    68,    69,    73,    99,   133,
     100,    78,   102,   131,   117,     4,    94,   118,   119,   120,
     121,   123,   123,   123,   123,   123,   123,   123,   124,   125,
     125,   126,   127,   128,   128,   129,    91,    37,    92,     4,
      43,   116,     4,    71,   145,    21,   136,   137,   138,   139,
     140,    72,    62,    73,    77,   116,   115,    76,   114,   141,
     141,    77,    75,   132,    91,    29,    35,   103,    91,   116,
      40,    45,   111,    98,    31,     4,     4,    24,    66,   116,
      46,    47,   142,   143,   144,     4,     4,    99,    30,   107,
     108,   109,    94,   116,    37,     4,   116,    29,   115,    31,
       4,    54,   136,   136,    25,    26,    55,     4,   144,   144,
      77,    75,   114,    91,    91,   116,    29,   110,   108,    31,
     106,    91,    41,   104,   145,   145,    44,   136,     4,    66,
      66,    66,   136,   114,   114,   142,    31,    31,    32,     4,
      31,    28,    62,   116,     4,    92,    91,   141,    76,    76,
      22,    23,     4,    91,    28,    91,   111,     4,   143,   143,
      91,    62,    91,    77,    77,    31,     4,     4,    24,    91,
      91,    31,    31,    24,    24
};

  /* YYR1[YYN] -- Symbol number of symbol that rule YYN derives.  */
static const yytype_uint8 yyr1[] =
{
       0,    87,    88,    89,    89,    90,    90,    90,    91,    91,
      92,    92,    93,    94,    94,    94,    94,    94,    95,    95,
      95,    95,    95,    95,    95,    95,    95,    95,    95,    95,
      95,    95,    95,    95,    95,    95,    95,    95,    95,    95,
      95,    96,    96,    97,    97,    98,    98,    98,    99,    99,
     100,   100,   101,   102,   102,   103,   103,   104,   104,   105,
     105,   105,   106,   106,   107,   107,   108,   108,   109,   110,
     110,   111,   111,   111,   112,   112,   113,   113,   114,   114,
     115,   115,   116,   116,   117,   117,   118,   118,   119,   119,
     120,   120,   121,   121,   122,   122,   122,   122,   122,   122,
     122,   122,   123,   123,   124,   124,   124,   125,   125,   126,
     126,   127,   127,   127,   128,   128,   129,   129,   129,   129,
     130,   130,   131,   131,   131,   131,   131,   131,   131,   132,
     132,   132,   132,   133,   133,   133,   134,   134,   135,   136,
     136,   136,   136,   136,   137,   137,   137,   138,   138,   139,
     139,   140,   140,   140,   141,   141,   142,   142,   143,   143,
     143,   144,   144,   145,   145
};

  /* YYR2[YYN] -- Number of symbols on the right hand side of rule YYN.  */
static const yytype_uint8 yyr2[] =
{
       0,     2,     3,     0,     3,     0,     2,     2,     0,     1,
       1,     2,     2,     1,     2,     1,     3,     2,     2,     3,
       4,     2,     1,     5,     6,     6,     4,     1,     2,     2,
       2,     2,     2,     5,     1,     4,     4,     2,    10,     8,
       7,     1,     3,     1,     3,     1,     4,     2,     1,     3,
       1,     3,     3,     1,     2,     1,     1,     0,     2,     9,
       4,     7,     0,     2,     0,     1,     1,     2,     5,     0,
       3,     0,     4,     5,     1,     3,     1,     1,     0,     1,
       1,     3,     1,     3,     1,     3,     1,     3,     1,     3,
       1,     3,     1,     2,     1,     3,     3,     3,     3,     3,
       3,     3,     1,     3,     1,     3,     3,     1,     3,     1,
       3,     1,     3,     3,     1,     3,     1,     1,     2,     2,
       1,     2,     1,     1,     1,     1,     1,     1,     1,     1,
       1,     1,     1,     1,     1,     1,     3,     1,     7,     0,
       3,     4,     4,     3,     9,    11,    11,     8,     8,     0,
       1,     2,     1,     1,     1,     3,     1,     3,     2,     3,
       3,     1,     1,     1,     1
};


#define yyerrok         (yyerrstatus = 0)
#define yyclearin       (yychar = YYEMPTY)
#define YYEMPTY         (-2)
#define YYEOF           0

#define YYACCEPT        goto yyacceptlab
#define YYABORT         goto yyabortlab
#define YYERROR         goto yyerrorlab


#define YYRECOVERING()  (!!yyerrstatus)

#define YYBACKUP(Token, Value)                                  \
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
      yyerror (ctx, YY_("syntax error: cannot back up")); \
      YYERROR;                                                  \
    }                                                           \
while (0)

/* Error token number */
#define YYTERROR        1
#define YYERRCODE       256



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

/* This macro is provided for backward compatibility. */
#ifndef YY_LOCATION_PRINT
# define YY_LOCATION_PRINT(File, Loc) ((void) 0)
#endif


# define YY_SYMBOL_PRINT(Title, Type, Value, Location)                    \
do {                                                                      \
  if (yydebug)                                                            \
    {                                                                     \
      YYFPRINTF (stderr, "%s ", Title);                                   \
      yy_symbol_print (stderr,                                            \
                  Type, Value, ctx); \
      YYFPRINTF (stderr, "\n");                                           \
    }                                                                     \
} while (0)


/*----------------------------------------.
| Print this symbol's value on YYOUTPUT.  |
`----------------------------------------*/

static void
yy_symbol_value_print (FILE *yyoutput, int yytype, YYSTYPE const * const yyvaluep, parser_ctx_t *ctx)
{
  FILE *yyo = yyoutput;
  YYUSE (yyo);
  YYUSE (ctx);
  if (!yyvaluep)
    return;
# ifdef YYPRINT
  if (yytype < YYNTOKENS)
    YYPRINT (yyoutput, yytoknum[yytype], *yyvaluep);
# endif
  YYUSE (yytype);
}


/*--------------------------------.
| Print this symbol on YYOUTPUT.  |
`--------------------------------*/

static void
yy_symbol_print (FILE *yyoutput, int yytype, YYSTYPE const * const yyvaluep, parser_ctx_t *ctx)
{
  YYFPRINTF (yyoutput, "%s %s (",
             yytype < YYNTOKENS ? "token" : "nterm", yytname[yytype]);

  yy_symbol_value_print (yyoutput, yytype, yyvaluep, ctx);
  YYFPRINTF (yyoutput, ")");
}

/*------------------------------------------------------------------.
| yy_stack_print -- Print the state stack from its BOTTOM up to its |
| TOP (included).                                                   |
`------------------------------------------------------------------*/

static void
yy_stack_print (yytype_int16 *yybottom, yytype_int16 *yytop)
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
yy_reduce_print (yytype_int16 *yyssp, YYSTYPE *yyvsp, int yyrule, parser_ctx_t *ctx)
{
  unsigned long int yylno = yyrline[yyrule];
  int yynrhs = yyr2[yyrule];
  int yyi;
  YYFPRINTF (stderr, "Reducing stack by rule %d (line %lu):\n",
             yyrule - 1, yylno);
  /* The symbols being reduced.  */
  for (yyi = 0; yyi < yynrhs; yyi++)
    {
      YYFPRINTF (stderr, "   $%d = ", yyi + 1);
      yy_symbol_print (stderr,
                       yystos[yyssp[yyi + 1 - yynrhs]],
                       &(yyvsp[(yyi + 1) - (yynrhs)])
                                              , ctx);
      YYFPRINTF (stderr, "\n");
    }
}

# define YY_REDUCE_PRINT(Rule)          \
do {                                    \
  if (yydebug)                          \
    yy_reduce_print (yyssp, yyvsp, Rule, ctx); \
} while (0)

/* Nonzero means print parse trace.  It is left uninitialized so that
   multiple parsers can coexist.  */
int yydebug;
#else /* !YYDEBUG */
# define YYDPRINTF(Args)
# define YY_SYMBOL_PRINT(Title, Type, Value, Location)
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


#if YYERROR_VERBOSE

# ifndef yystrlen
#  if defined __GLIBC__ && defined _STRING_H
#   define yystrlen strlen
#  else
/* Return the length of YYSTR.  */
static YYSIZE_T
yystrlen (const char *yystr)
{
  YYSIZE_T yylen;
  for (yylen = 0; yystr[yylen]; yylen++)
    continue;
  return yylen;
}
#  endif
# endif

# ifndef yystpcpy
#  if defined __GLIBC__ && defined _STRING_H && defined _GNU_SOURCE
#   define yystpcpy stpcpy
#  else
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
#  endif
# endif

# ifndef yytnamerr
/* Copy to YYRES the contents of YYSTR after stripping away unnecessary
   quotes and backslashes, so that it's suitable for yyerror.  The
   heuristic is that double-quoting is unnecessary unless the string
   contains an apostrophe, a comma, or backslash (other than
   backslash-backslash).  YYSTR is taken from yytname.  If YYRES is
   null, do not copy; instead, return the length of what the result
   would have been.  */
static YYSIZE_T
yytnamerr (char *yyres, const char *yystr)
{
  if (*yystr == '"')
    {
      YYSIZE_T yyn = 0;
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
            /* Fall through.  */
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

  if (! yyres)
    return yystrlen (yystr);

  return yystpcpy (yyres, yystr) - yyres;
}
# endif

/* Copy into *YYMSG, which is of size *YYMSG_ALLOC, an error message
   about the unexpected token YYTOKEN for the state stack whose top is
   YYSSP.

   Return 0 if *YYMSG was successfully written.  Return 1 if *YYMSG is
   not large enough to hold the message.  In that case, also set
   *YYMSG_ALLOC to the required number of bytes.  Return 2 if the
   required number of bytes is too large to store.  */
static int
yysyntax_error (YYSIZE_T *yymsg_alloc, char **yymsg,
                yytype_int16 *yyssp, int yytoken)
{
  YYSIZE_T yysize0 = yytnamerr (YY_NULLPTR, yytname[yytoken]);
  YYSIZE_T yysize = yysize0;
  enum { YYERROR_VERBOSE_ARGS_MAXIMUM = 5 };
  /* Internationalized format string. */
  const char *yyformat = YY_NULLPTR;
  /* Arguments of yyformat. */
  char const *yyarg[YYERROR_VERBOSE_ARGS_MAXIMUM];
  /* Number of reported tokens (one for the "unexpected", one per
     "expected"). */
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
  if (yytoken != YYEMPTY)
    {
      int yyn = yypact[*yyssp];
      yyarg[yycount++] = yytname[yytoken];
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
            if (yycheck[yyx + yyn] == yyx && yyx != YYTERROR
                && !yytable_value_is_error (yytable[yyx + yyn]))
              {
                if (yycount == YYERROR_VERBOSE_ARGS_MAXIMUM)
                  {
                    yycount = 1;
                    yysize = yysize0;
                    break;
                  }
                yyarg[yycount++] = yytname[yyx];
                {
                  YYSIZE_T yysize1 = yysize + yytnamerr (YY_NULLPTR, yytname[yyx]);
                  if (! (yysize <= yysize1
                         && yysize1 <= YYSTACK_ALLOC_MAXIMUM))
                    return 2;
                  yysize = yysize1;
                }
              }
        }
    }

  switch (yycount)
    {
# define YYCASE_(N, S)                      \
      case N:                               \
        yyformat = S;                       \
      break
      YYCASE_(0, YY_("syntax error"));
      YYCASE_(1, YY_("syntax error, unexpected %s"));
      YYCASE_(2, YY_("syntax error, unexpected %s, expecting %s"));
      YYCASE_(3, YY_("syntax error, unexpected %s, expecting %s or %s"));
      YYCASE_(4, YY_("syntax error, unexpected %s, expecting %s or %s or %s"));
      YYCASE_(5, YY_("syntax error, unexpected %s, expecting %s or %s or %s or %s"));
# undef YYCASE_
    }

  {
    YYSIZE_T yysize1 = yysize + yystrlen (yyformat);
    if (! (yysize <= yysize1 && yysize1 <= YYSTACK_ALLOC_MAXIMUM))
      return 2;
    yysize = yysize1;
  }

  if (*yymsg_alloc < yysize)
    {
      *yymsg_alloc = 2 * yysize;
      if (! (yysize <= *yymsg_alloc
             && *yymsg_alloc <= YYSTACK_ALLOC_MAXIMUM))
        *yymsg_alloc = YYSTACK_ALLOC_MAXIMUM;
      return 1;
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
          yyp += yytnamerr (yyp, yyarg[yyi++]);
          yyformat += 2;
        }
      else
        {
          yyp++;
          yyformat++;
        }
  }
  return 0;
}
#endif /* YYERROR_VERBOSE */

/*-----------------------------------------------.
| Release the memory associated to this symbol.  |
`-----------------------------------------------*/

static void
yydestruct (const char *yymsg, int yytype, YYSTYPE *yyvaluep, parser_ctx_t *ctx)
{
  YYUSE (yyvaluep);
  YYUSE (ctx);
  if (!yymsg)
    yymsg = "Deleting";
  YY_SYMBOL_PRINT (yymsg, yytype, yyvaluep, yylocationp);

  YY_IGNORE_MAYBE_UNINITIALIZED_BEGIN
  YYUSE (yytype);
  YY_IGNORE_MAYBE_UNINITIALIZED_END
}




/*----------.
| yyparse.  |
`----------*/

int
yyparse (parser_ctx_t *ctx)
{
/* The lookahead symbol.  */
int yychar;


/* The semantic value of the lookahead symbol.  */
/* Default value used for initialization, for pacifying older GCCs
   or non-GCC compilers.  */
YY_INITIAL_VALUE (static YYSTYPE yyval_default;)
YYSTYPE yylval YY_INITIAL_VALUE (= yyval_default);

    /* Number of syntax errors so far.  */
    int yynerrs;

    int yystate;
    /* Number of tokens to shift before error messages enabled.  */
    int yyerrstatus;

    /* The stacks and their tools:
       'yyss': related to states.
       'yyvs': related to semantic values.

       Refer to the stacks through separate pointers, to allow yyoverflow
       to reallocate them elsewhere.  */

    /* The state stack.  */
    yytype_int16 yyssa[YYINITDEPTH];
    yytype_int16 *yyss;
    yytype_int16 *yyssp;

    /* The semantic value stack.  */
    YYSTYPE yyvsa[YYINITDEPTH];
    YYSTYPE *yyvs;
    YYSTYPE *yyvsp;

    YYSIZE_T yystacksize;

  int yyn;
  int yyresult;
  /* Lookahead token as an internal (translated) token number.  */
  int yytoken = 0;
  /* The variables used to return semantic value and location from the
     action routines.  */
  YYSTYPE yyval;

#if YYERROR_VERBOSE
  /* Buffer for error messages, and its allocated size.  */
  char yymsgbuf[128];
  char *yymsg = yymsgbuf;
  YYSIZE_T yymsg_alloc = sizeof yymsgbuf;
#endif

#define YYPOPSTACK(N)   (yyvsp -= (N), yyssp -= (N))

  /* The number of symbols on the RHS of the reduced rule.
     Keep to zero when no symbol should be popped.  */
  int yylen = 0;

  yyssp = yyss = yyssa;
  yyvsp = yyvs = yyvsa;
  yystacksize = YYINITDEPTH;

  YYDPRINTF ((stderr, "Starting parse\n"));

  yystate = 0;
  yyerrstatus = 0;
  yynerrs = 0;
  yychar = YYEMPTY; /* Cause a token to be read.  */
  goto yysetstate;

/*------------------------------------------------------------.
| yynewstate -- Push a new state, which is found in yystate.  |
`------------------------------------------------------------*/
 yynewstate:
  /* In all cases, when you get here, the value and location stacks
     have just been pushed.  So pushing a state here evens the stacks.  */
  yyssp++;

 yysetstate:
  *yyssp = yystate;

  if (yyss + yystacksize - 1 <= yyssp)
    {
      /* Get the current used size of the three stacks, in elements.  */
      YYSIZE_T yysize = yyssp - yyss + 1;

#ifdef yyoverflow
      {
        /* Give user a chance to reallocate the stack.  Use copies of
           these so that the &'s don't force the real ones into
           memory.  */
        YYSTYPE *yyvs1 = yyvs;
        yytype_int16 *yyss1 = yyss;

        /* Each stack pointer address is followed by the size of the
           data in use in that stack, in bytes.  This used to be a
           conditional around just the two extra args, but that might
           be undefined if yyoverflow is a macro.  */
        yyoverflow (YY_("memory exhausted"),
                    &yyss1, yysize * sizeof (*yyssp),
                    &yyvs1, yysize * sizeof (*yyvsp),
                    &yystacksize);

        yyss = yyss1;
        yyvs = yyvs1;
      }
#else /* no yyoverflow */
# ifndef YYSTACK_RELOCATE
      goto yyexhaustedlab;
# else
      /* Extend the stack our own way.  */
      if (YYMAXDEPTH <= yystacksize)
        goto yyexhaustedlab;
      yystacksize *= 2;
      if (YYMAXDEPTH < yystacksize)
        yystacksize = YYMAXDEPTH;

      {
        yytype_int16 *yyss1 = yyss;
        union yyalloc *yyptr =
          (union yyalloc *) YYSTACK_ALLOC (YYSTACK_BYTES (yystacksize));
        if (! yyptr)
          goto yyexhaustedlab;
        YYSTACK_RELOCATE (yyss_alloc, yyss);
        YYSTACK_RELOCATE (yyvs_alloc, yyvs);
#  undef YYSTACK_RELOCATE
        if (yyss1 != yyssa)
          YYSTACK_FREE (yyss1);
      }
# endif
#endif /* no yyoverflow */

      yyssp = yyss + yysize - 1;
      yyvsp = yyvs + yysize - 1;

      YYDPRINTF ((stderr, "Stack size increased to %lu\n",
                  (unsigned long int) yystacksize));

      if (yyss + yystacksize - 1 <= yyssp)
        YYABORT;
    }

  YYDPRINTF ((stderr, "Entering state %d\n", yystate));

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

  /* YYCHAR is either YYEMPTY or YYEOF or a valid lookahead symbol.  */
  if (yychar == YYEMPTY)
    {
      YYDPRINTF ((stderr, "Reading a token: "));
      yychar = yylex (&yylval, ctx);
    }

  if (yychar <= YYEOF)
    {
      yychar = yytoken = YYEOF;
      YYDPRINTF ((stderr, "Now at end of input.\n"));
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

  /* Discard the shifted token.  */
  yychar = YYEMPTY;

  yystate = yyn;
  YY_IGNORE_MAYBE_UNINITIALIZED_BEGIN
  *++yyvsp = yylval;
  YY_IGNORE_MAYBE_UNINITIALIZED_END

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
| yyreduce -- Do a reduction.  |
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
        case 2:
#line 147 "parser.y" /* yacc.c:1646  */
    { parse_complete(ctx, (yyvsp[-2].boolean)); }
#line 1717 "parser.tab.c" /* yacc.c:1646  */
    break;

  case 3:
#line 150 "parser.y" /* yacc.c:1646  */
    { (yyval.boolean) = FALSE; }
#line 1723 "parser.tab.c" /* yacc.c:1646  */
    break;

  case 4:
#line 151 "parser.y" /* yacc.c:1646  */
    { (yyval.boolean) = TRUE; }
#line 1729 "parser.tab.c" /* yacc.c:1646  */
    break;

  case 6:
#line 155 "parser.y" /* yacc.c:1646  */
    { source_add_statement(ctx, (yyvsp[0].statement)); }
#line 1735 "parser.tab.c" /* yacc.c:1646  */
    break;

  case 7:
#line 156 "parser.y" /* yacc.c:1646  */
    { source_add_class(ctx, (yyvsp[0].class_decl)); }
#line 1741 "parser.tab.c" /* yacc.c:1646  */
    break;

  case 8:
#line 159 "parser.y" /* yacc.c:1646  */
    { (yyval.statement) = NULL; }
#line 1747 "parser.tab.c" /* yacc.c:1646  */
    break;

  case 9:
#line 160 "parser.y" /* yacc.c:1646  */
    { (yyval.statement) = (yyvsp[0].statement); }
#line 1753 "parser.tab.c" /* yacc.c:1646  */
    break;

  case 10:
#line 163 "parser.y" /* yacc.c:1646  */
    { (yyval.statement) = (yyvsp[0].statement); }
#line 1759 "parser.tab.c" /* yacc.c:1646  */
    break;

  case 11:
#line 164 "parser.y" /* yacc.c:1646  */
    { (yyval.statement) = link_statements((yyvsp[-1].statement), (yyvsp[0].statement)); }
#line 1765 "parser.tab.c" /* yacc.c:1646  */
    break;

  case 12:
#line 167 "parser.y" /* yacc.c:1646  */
    { (yyval.statement) = (yyvsp[-1].statement); }
#line 1771 "parser.tab.c" /* yacc.c:1646  */
    break;

  case 13:
#line 170 "parser.y" /* yacc.c:1646  */
    { (yyval.statement) = NULL; }
#line 1777 "parser.tab.c" /* yacc.c:1646  */
    break;

  case 14:
#line 171 "parser.y" /* yacc.c:1646  */
    { (yyval.statement) = (yyvsp[0].statement); }
#line 1783 "parser.tab.c" /* yacc.c:1646  */
    break;

  case 15:
#line 172 "parser.y" /* yacc.c:1646  */
    { (yyval.statement) = (yyvsp[0].statement); }
#line 1789 "parser.tab.c" /* yacc.c:1646  */
    break;

  case 16:
#line 173 "parser.y" /* yacc.c:1646  */
    { (yyvsp[-2].statement)->next = (yyvsp[0].statement); (yyval.statement) = (yyvsp[-2].statement); }
#line 1795 "parser.tab.c" /* yacc.c:1646  */
    break;

  case 17:
#line 174 "parser.y" /* yacc.c:1646  */
    { (yyval.statement) = (yyvsp[-1].statement); }
#line 1801 "parser.tab.c" /* yacc.c:1646  */
    break;

  case 18:
#line 177 "parser.y" /* yacc.c:1646  */
    { (yyvsp[-1].member)->args = (yyvsp[0].expression); (yyval.statement) = new_call_statement(ctx, FALSE, (yyvsp[-1].member)); CHECK_ERROR; }
#line 1807 "parser.tab.c" /* yacc.c:1646  */
    break;

  case 19:
#line 178 "parser.y" /* yacc.c:1646  */
    { (yyvsp[-1].member)->args = (yyvsp[0].expression); (yyval.statement) = new_call_statement(ctx, TRUE, (yyvsp[-1].member)); CHECK_ERROR; }
#line 1813 "parser.tab.c" /* yacc.c:1646  */
    break;

  case 20:
#line 180 "parser.y" /* yacc.c:1646  */
    { (yyvsp[-3].member)->args = (yyvsp[-2].expression); (yyval.statement) = new_assign_statement(ctx, (yyvsp[-3].member), (yyvsp[0].expression)); CHECK_ERROR; }
#line 1819 "parser.tab.c" /* yacc.c:1646  */
    break;

  case 21:
#line 181 "parser.y" /* yacc.c:1646  */
    { (yyval.statement) = new_dim_statement(ctx, (yyvsp[0].dim_decl)); CHECK_ERROR; }
#line 1825 "parser.tab.c" /* yacc.c:1646  */
    break;

  case 22:
#line 182 "parser.y" /* yacc.c:1646  */
    { (yyval.statement) = (yyvsp[0].statement); }
#line 1831 "parser.tab.c" /* yacc.c:1646  */
    break;

  case 23:
#line 184 "parser.y" /* yacc.c:1646  */
    { (yyval.statement) = new_while_statement(ctx, STAT_WHILE, (yyvsp[-3].expression), (yyvsp[-1].statement)); CHECK_ERROR; }
#line 1837 "parser.tab.c" /* yacc.c:1646  */
    break;

  case 24:
#line 186 "parser.y" /* yacc.c:1646  */
    { (yyval.statement) = new_while_statement(ctx, (yyvsp[-4].boolean) ? STAT_WHILELOOP : STAT_UNTIL, (yyvsp[-3].expression), (yyvsp[-1].statement));
                                              CHECK_ERROR; }
#line 1844 "parser.tab.c" /* yacc.c:1646  */
    break;

  case 25:
#line 189 "parser.y" /* yacc.c:1646  */
    { (yyval.statement) = new_while_statement(ctx, (yyvsp[-1].boolean) ? STAT_DOWHILE : STAT_DOUNTIL, (yyvsp[0].expression), (yyvsp[-3].statement));
                                              CHECK_ERROR; }
#line 1851 "parser.tab.c" /* yacc.c:1646  */
    break;

  case 26:
#line 191 "parser.y" /* yacc.c:1646  */
    { (yyval.statement) = new_while_statement(ctx, STAT_DOWHILE, NULL, (yyvsp[-1].statement)); CHECK_ERROR; }
#line 1857 "parser.tab.c" /* yacc.c:1646  */
    break;

  case 27:
#line 192 "parser.y" /* yacc.c:1646  */
    { (yyval.statement) = new_function_statement(ctx, (yyvsp[0].func_decl)); CHECK_ERROR; }
#line 1863 "parser.tab.c" /* yacc.c:1646  */
    break;

  case 28:
#line 193 "parser.y" /* yacc.c:1646  */
    { (yyval.statement) = new_statement(ctx, STAT_EXITDO, 0); CHECK_ERROR; }
#line 1869 "parser.tab.c" /* yacc.c:1646  */
    break;

  case 29:
#line 194 "parser.y" /* yacc.c:1646  */
    { (yyval.statement) = new_statement(ctx, STAT_EXITFOR, 0); CHECK_ERROR; }
#line 1875 "parser.tab.c" /* yacc.c:1646  */
    break;

  case 30:
#line 195 "parser.y" /* yacc.c:1646  */
    { (yyval.statement) = new_statement(ctx, STAT_EXITFUNC, 0); CHECK_ERROR; }
#line 1881 "parser.tab.c" /* yacc.c:1646  */
    break;

  case 31:
#line 196 "parser.y" /* yacc.c:1646  */
    { (yyval.statement) = new_statement(ctx, STAT_EXITPROP, 0); CHECK_ERROR; }
#line 1887 "parser.tab.c" /* yacc.c:1646  */
    break;

  case 32:
#line 197 "parser.y" /* yacc.c:1646  */
    { (yyval.statement) = new_statement(ctx, STAT_EXITSUB, 0); CHECK_ERROR; }
#line 1893 "parser.tab.c" /* yacc.c:1646  */
    break;

  case 33:
#line 199 "parser.y" /* yacc.c:1646  */
    { (yyvsp[-3].member)->args = (yyvsp[-2].expression); (yyval.statement) = new_set_statement(ctx, (yyvsp[-3].member), (yyvsp[0].expression)); CHECK_ERROR; }
#line 1899 "parser.tab.c" /* yacc.c:1646  */
    break;

  case 34:
#line 200 "parser.y" /* yacc.c:1646  */
    { (yyval.statement) = new_statement(ctx, STAT_STOP, 0); CHECK_ERROR; }
#line 1905 "parser.tab.c" /* yacc.c:1646  */
    break;

  case 35:
#line 201 "parser.y" /* yacc.c:1646  */
    { (yyval.statement) = new_onerror_statement(ctx, TRUE); CHECK_ERROR; }
#line 1911 "parser.tab.c" /* yacc.c:1646  */
    break;

  case 36:
#line 202 "parser.y" /* yacc.c:1646  */
    { (yyval.statement) = new_onerror_statement(ctx, FALSE); CHECK_ERROR; }
#line 1917 "parser.tab.c" /* yacc.c:1646  */
    break;

  case 37:
#line 203 "parser.y" /* yacc.c:1646  */
    { (yyval.statement) = new_const_statement(ctx, (yyvsp[0].const_decl)); CHECK_ERROR; }
#line 1923 "parser.tab.c" /* yacc.c:1646  */
    break;

  case 38:
#line 205 "parser.y" /* yacc.c:1646  */
    { (yyval.statement) = new_forto_statement(ctx, (yyvsp[-8].string), (yyvsp[-6].expression), (yyvsp[-4].expression), (yyvsp[-3].expression), (yyvsp[-1].statement)); CHECK_ERROR; }
#line 1929 "parser.tab.c" /* yacc.c:1646  */
    break;

  case 39:
#line 207 "parser.y" /* yacc.c:1646  */
    { (yyval.statement) = new_foreach_statement(ctx, (yyvsp[-5].string), (yyvsp[-3].expression), (yyvsp[-1].statement)); }
#line 1935 "parser.tab.c" /* yacc.c:1646  */
    break;

  case 40:
#line 209 "parser.y" /* yacc.c:1646  */
    { (yyval.statement) = new_select_statement(ctx, (yyvsp[-4].expression), (yyvsp[-2].case_clausule)); }
#line 1941 "parser.tab.c" /* yacc.c:1646  */
    break;

  case 41:
#line 212 "parser.y" /* yacc.c:1646  */
    { (yyval.member) = new_member_expression(ctx, NULL, (yyvsp[0].string)); CHECK_ERROR; }
#line 1947 "parser.tab.c" /* yacc.c:1646  */
    break;

  case 42:
#line 213 "parser.y" /* yacc.c:1646  */
    { (yyval.member) = new_member_expression(ctx, (yyvsp[-2].expression), (yyvsp[0].string)); CHECK_ERROR; }
#line 1953 "parser.tab.c" /* yacc.c:1646  */
    break;

  case 43:
#line 216 "parser.y" /* yacc.c:1646  */
    { (yyval.dim_decl) = (yyvsp[0].dim_decl); }
#line 1959 "parser.tab.c" /* yacc.c:1646  */
    break;

  case 44:
#line 217 "parser.y" /* yacc.c:1646  */
    { (yyvsp[-2].dim_decl)->next = (yyvsp[0].dim_decl); (yyval.dim_decl) = (yyvsp[-2].dim_decl); }
#line 1965 "parser.tab.c" /* yacc.c:1646  */
    break;

  case 45:
#line 220 "parser.y" /* yacc.c:1646  */
    { (yyval.dim_decl) = new_dim_decl(ctx, (yyvsp[0].string), FALSE, NULL); CHECK_ERROR; }
#line 1971 "parser.tab.c" /* yacc.c:1646  */
    break;

  case 46:
#line 221 "parser.y" /* yacc.c:1646  */
    { (yyval.dim_decl) = new_dim_decl(ctx, (yyvsp[-3].string), TRUE, (yyvsp[-1].dim_list)); CHECK_ERROR; }
#line 1977 "parser.tab.c" /* yacc.c:1646  */
    break;

  case 47:
#line 222 "parser.y" /* yacc.c:1646  */
    { (yyval.dim_decl) = new_dim_decl(ctx, (yyvsp[-1].string), TRUE, NULL); CHECK_ERROR; }
#line 1983 "parser.tab.c" /* yacc.c:1646  */
    break;

  case 48:
#line 225 "parser.y" /* yacc.c:1646  */
    { (yyval.dim_list) = new_dim(ctx, (yyvsp[0].uint), NULL); }
#line 1989 "parser.tab.c" /* yacc.c:1646  */
    break;

  case 49:
#line 226 "parser.y" /* yacc.c:1646  */
    { (yyval.dim_list) = new_dim(ctx, (yyvsp[-2].uint), (yyvsp[0].dim_list)); }
#line 1995 "parser.tab.c" /* yacc.c:1646  */
    break;

  case 50:
#line 229 "parser.y" /* yacc.c:1646  */
    { (yyval.const_decl) = (yyvsp[0].const_decl); }
#line 2001 "parser.tab.c" /* yacc.c:1646  */
    break;

  case 51:
#line 230 "parser.y" /* yacc.c:1646  */
    { (yyvsp[-2].const_decl)->next = (yyvsp[0].const_decl); (yyval.const_decl) = (yyvsp[-2].const_decl); }
#line 2007 "parser.tab.c" /* yacc.c:1646  */
    break;

  case 52:
#line 233 "parser.y" /* yacc.c:1646  */
    { (yyval.const_decl) = new_const_decl(ctx, (yyvsp[-2].string), (yyvsp[0].expression)); CHECK_ERROR; }
#line 2013 "parser.tab.c" /* yacc.c:1646  */
    break;

  case 53:
#line 236 "parser.y" /* yacc.c:1646  */
    { (yyval.expression) = (yyvsp[0].expression); }
#line 2019 "parser.tab.c" /* yacc.c:1646  */
    break;

  case 54:
#line 237 "parser.y" /* yacc.c:1646  */
    { (yyval.expression) = new_unary_expression(ctx, EXPR_NEG, (yyvsp[0].expression)); CHECK_ERROR; }
#line 2025 "parser.tab.c" /* yacc.c:1646  */
    break;

  case 55:
#line 240 "parser.y" /* yacc.c:1646  */
    { (yyval.boolean) = TRUE; }
#line 2031 "parser.tab.c" /* yacc.c:1646  */
    break;

  case 56:
#line 241 "parser.y" /* yacc.c:1646  */
    { (yyval.boolean) = FALSE; }
#line 2037 "parser.tab.c" /* yacc.c:1646  */
    break;

  case 57:
#line 244 "parser.y" /* yacc.c:1646  */
    { (yyval.expression) = NULL;}
#line 2043 "parser.tab.c" /* yacc.c:1646  */
    break;

  case 58:
#line 245 "parser.y" /* yacc.c:1646  */
    { (yyval.expression) = (yyvsp[0].expression); }
#line 2049 "parser.tab.c" /* yacc.c:1646  */
    break;

  case 59:
#line 249 "parser.y" /* yacc.c:1646  */
    { (yyval.statement) = new_if_statement(ctx, (yyvsp[-7].expression), (yyvsp[-4].statement), (yyvsp[-3].elseif), (yyvsp[-2].statement)); CHECK_ERROR; }
#line 2055 "parser.tab.c" /* yacc.c:1646  */
    break;

  case 60:
#line 250 "parser.y" /* yacc.c:1646  */
    { (yyval.statement) = new_if_statement(ctx, (yyvsp[-2].expression), (yyvsp[0].statement), NULL, NULL); CHECK_ERROR; }
#line 2061 "parser.tab.c" /* yacc.c:1646  */
    break;

  case 61:
#line 252 "parser.y" /* yacc.c:1646  */
    { (yyval.statement) = new_if_statement(ctx, (yyvsp[-5].expression), (yyvsp[-3].statement), NULL, (yyvsp[-1].statement)); CHECK_ERROR; }
#line 2067 "parser.tab.c" /* yacc.c:1646  */
    break;

  case 64:
#line 259 "parser.y" /* yacc.c:1646  */
    { (yyval.elseif) = NULL; }
#line 2073 "parser.tab.c" /* yacc.c:1646  */
    break;

  case 65:
#line 260 "parser.y" /* yacc.c:1646  */
    { (yyval.elseif) = (yyvsp[0].elseif); }
#line 2079 "parser.tab.c" /* yacc.c:1646  */
    break;

  case 66:
#line 263 "parser.y" /* yacc.c:1646  */
    { (yyval.elseif) = (yyvsp[0].elseif); }
#line 2085 "parser.tab.c" /* yacc.c:1646  */
    break;

  case 67:
#line 264 "parser.y" /* yacc.c:1646  */
    { (yyvsp[-1].elseif)->next = (yyvsp[0].elseif); (yyval.elseif) = (yyvsp[-1].elseif); }
#line 2091 "parser.tab.c" /* yacc.c:1646  */
    break;

  case 68:
#line 268 "parser.y" /* yacc.c:1646  */
    { (yyval.elseif) = new_elseif_decl(ctx, (yyvsp[-3].expression), (yyvsp[0].statement)); }
#line 2097 "parser.tab.c" /* yacc.c:1646  */
    break;

  case 69:
#line 271 "parser.y" /* yacc.c:1646  */
    { (yyval.statement) = NULL; }
#line 2103 "parser.tab.c" /* yacc.c:1646  */
    break;

  case 70:
#line 272 "parser.y" /* yacc.c:1646  */
    { (yyval.statement) = (yyvsp[0].statement); }
#line 2109 "parser.tab.c" /* yacc.c:1646  */
    break;

  case 71:
#line 275 "parser.y" /* yacc.c:1646  */
    { (yyval.case_clausule) = NULL; }
#line 2115 "parser.tab.c" /* yacc.c:1646  */
    break;

  case 72:
#line 276 "parser.y" /* yacc.c:1646  */
    { (yyval.case_clausule) = new_case_clausule(ctx, NULL, (yyvsp[0].statement), NULL); }
#line 2121 "parser.tab.c" /* yacc.c:1646  */
    break;

  case 73:
#line 278 "parser.y" /* yacc.c:1646  */
    { (yyval.case_clausule) = new_case_clausule(ctx, (yyvsp[-3].expression), (yyvsp[-1].statement), (yyvsp[0].case_clausule)); }
#line 2127 "parser.tab.c" /* yacc.c:1646  */
    break;

  case 74:
#line 281 "parser.y" /* yacc.c:1646  */
    { (yyval.expression) = NULL; }
#line 2133 "parser.tab.c" /* yacc.c:1646  */
    break;

  case 75:
#line 282 "parser.y" /* yacc.c:1646  */
    { (yyval.expression) = (yyvsp[-1].expression); }
#line 2139 "parser.tab.c" /* yacc.c:1646  */
    break;

  case 76:
#line 285 "parser.y" /* yacc.c:1646  */
    { (yyval.expression) = NULL; }
#line 2145 "parser.tab.c" /* yacc.c:1646  */
    break;

  case 77:
#line 286 "parser.y" /* yacc.c:1646  */
    { (yyval.expression) = (yyvsp[0].expression); }
#line 2151 "parser.tab.c" /* yacc.c:1646  */
    break;

  case 80:
#line 293 "parser.y" /* yacc.c:1646  */
    { (yyval.expression) = (yyvsp[0].expression); }
#line 2157 "parser.tab.c" /* yacc.c:1646  */
    break;

  case 81:
#line 294 "parser.y" /* yacc.c:1646  */
    { (yyvsp[-2].expression)->next = (yyvsp[0].expression); (yyval.expression) = (yyvsp[-2].expression); }
#line 2163 "parser.tab.c" /* yacc.c:1646  */
    break;

  case 82:
#line 297 "parser.y" /* yacc.c:1646  */
    { (yyval.expression) = (yyvsp[0].expression); }
#line 2169 "parser.tab.c" /* yacc.c:1646  */
    break;

  case 83:
#line 298 "parser.y" /* yacc.c:1646  */
    { (yyval.expression) = new_binary_expression(ctx, EXPR_IMP, (yyvsp[-2].expression), (yyvsp[0].expression)); CHECK_ERROR; }
#line 2175 "parser.tab.c" /* yacc.c:1646  */
    break;

  case 84:
#line 301 "parser.y" /* yacc.c:1646  */
    { (yyval.expression) = (yyvsp[0].expression); }
#line 2181 "parser.tab.c" /* yacc.c:1646  */
    break;

  case 85:
#line 302 "parser.y" /* yacc.c:1646  */
    { (yyval.expression) = new_binary_expression(ctx, EXPR_EQV, (yyvsp[-2].expression), (yyvsp[0].expression)); CHECK_ERROR; }
#line 2187 "parser.tab.c" /* yacc.c:1646  */
    break;

  case 86:
#line 305 "parser.y" /* yacc.c:1646  */
    { (yyval.expression) = (yyvsp[0].expression); }
#line 2193 "parser.tab.c" /* yacc.c:1646  */
    break;

  case 87:
#line 306 "parser.y" /* yacc.c:1646  */
    { (yyval.expression) = new_binary_expression(ctx, EXPR_XOR, (yyvsp[-2].expression), (yyvsp[0].expression)); CHECK_ERROR; }
#line 2199 "parser.tab.c" /* yacc.c:1646  */
    break;

  case 88:
#line 309 "parser.y" /* yacc.c:1646  */
    { (yyval.expression) = (yyvsp[0].expression); }
#line 2205 "parser.tab.c" /* yacc.c:1646  */
    break;

  case 89:
#line 310 "parser.y" /* yacc.c:1646  */
    { (yyval.expression) = new_binary_expression(ctx, EXPR_OR, (yyvsp[-2].expression), (yyvsp[0].expression)); CHECK_ERROR; }
#line 2211 "parser.tab.c" /* yacc.c:1646  */
    break;

  case 90:
#line 313 "parser.y" /* yacc.c:1646  */
    { (yyval.expression) = (yyvsp[0].expression); }
#line 2217 "parser.tab.c" /* yacc.c:1646  */
    break;

  case 91:
#line 314 "parser.y" /* yacc.c:1646  */
    { (yyval.expression) = new_binary_expression(ctx, EXPR_AND, (yyvsp[-2].expression), (yyvsp[0].expression)); CHECK_ERROR; }
#line 2223 "parser.tab.c" /* yacc.c:1646  */
    break;

  case 92:
#line 317 "parser.y" /* yacc.c:1646  */
    { (yyval.expression) = (yyvsp[0].expression); }
#line 2229 "parser.tab.c" /* yacc.c:1646  */
    break;

  case 93:
#line 318 "parser.y" /* yacc.c:1646  */
    { (yyval.expression) = new_unary_expression(ctx, EXPR_NOT, (yyvsp[0].expression)); CHECK_ERROR; }
#line 2235 "parser.tab.c" /* yacc.c:1646  */
    break;

  case 94:
#line 321 "parser.y" /* yacc.c:1646  */
    { (yyval.expression) = (yyvsp[0].expression); }
#line 2241 "parser.tab.c" /* yacc.c:1646  */
    break;

  case 95:
#line 322 "parser.y" /* yacc.c:1646  */
    { (yyval.expression) = new_binary_expression(ctx, EXPR_EQUAL, (yyvsp[-2].expression), (yyvsp[0].expression)); CHECK_ERROR; }
#line 2247 "parser.tab.c" /* yacc.c:1646  */
    break;

  case 96:
#line 323 "parser.y" /* yacc.c:1646  */
    { (yyval.expression) = new_binary_expression(ctx, EXPR_NEQUAL, (yyvsp[-2].expression), (yyvsp[0].expression)); CHECK_ERROR; }
#line 2253 "parser.tab.c" /* yacc.c:1646  */
    break;

  case 97:
#line 324 "parser.y" /* yacc.c:1646  */
    { (yyval.expression) = new_binary_expression(ctx, EXPR_GT, (yyvsp[-2].expression), (yyvsp[0].expression)); CHECK_ERROR; }
#line 2259 "parser.tab.c" /* yacc.c:1646  */
    break;

  case 98:
#line 325 "parser.y" /* yacc.c:1646  */
    { (yyval.expression) = new_binary_expression(ctx, EXPR_LT, (yyvsp[-2].expression), (yyvsp[0].expression)); CHECK_ERROR; }
#line 2265 "parser.tab.c" /* yacc.c:1646  */
    break;

  case 99:
#line 326 "parser.y" /* yacc.c:1646  */
    { (yyval.expression) = new_binary_expression(ctx, EXPR_GTEQ, (yyvsp[-2].expression), (yyvsp[0].expression)); CHECK_ERROR; }
#line 2271 "parser.tab.c" /* yacc.c:1646  */
    break;

  case 100:
#line 327 "parser.y" /* yacc.c:1646  */
    { (yyval.expression) = new_binary_expression(ctx, EXPR_LTEQ, (yyvsp[-2].expression), (yyvsp[0].expression)); CHECK_ERROR; }
#line 2277 "parser.tab.c" /* yacc.c:1646  */
    break;

  case 101:
#line 328 "parser.y" /* yacc.c:1646  */
    { (yyval.expression) = new_binary_expression(ctx, EXPR_IS, (yyvsp[-2].expression), (yyvsp[0].expression)); CHECK_ERROR; }
#line 2283 "parser.tab.c" /* yacc.c:1646  */
    break;

  case 102:
#line 331 "parser.y" /* yacc.c:1646  */
    { (yyval.expression) = (yyvsp[0].expression); }
#line 2289 "parser.tab.c" /* yacc.c:1646  */
    break;

  case 103:
#line 332 "parser.y" /* yacc.c:1646  */
    { (yyval.expression) = new_binary_expression(ctx, EXPR_CONCAT, (yyvsp[-2].expression), (yyvsp[0].expression)); CHECK_ERROR; }
#line 2295 "parser.tab.c" /* yacc.c:1646  */
    break;

  case 104:
#line 335 "parser.y" /* yacc.c:1646  */
    { (yyval.expression) = (yyvsp[0].expression); }
#line 2301 "parser.tab.c" /* yacc.c:1646  */
    break;

  case 105:
#line 336 "parser.y" /* yacc.c:1646  */
    { (yyval.expression) = new_binary_expression(ctx, EXPR_ADD, (yyvsp[-2].expression), (yyvsp[0].expression)); CHECK_ERROR; }
#line 2307 "parser.tab.c" /* yacc.c:1646  */
    break;

  case 106:
#line 337 "parser.y" /* yacc.c:1646  */
    { (yyval.expression) = new_binary_expression(ctx, EXPR_SUB, (yyvsp[-2].expression), (yyvsp[0].expression)); CHECK_ERROR; }
#line 2313 "parser.tab.c" /* yacc.c:1646  */
    break;

  case 107:
#line 340 "parser.y" /* yacc.c:1646  */
    { (yyval.expression) = (yyvsp[0].expression); }
#line 2319 "parser.tab.c" /* yacc.c:1646  */
    break;

  case 108:
#line 341 "parser.y" /* yacc.c:1646  */
    { (yyval.expression) = new_binary_expression(ctx, EXPR_MOD, (yyvsp[-2].expression), (yyvsp[0].expression)); CHECK_ERROR; }
#line 2325 "parser.tab.c" /* yacc.c:1646  */
    break;

  case 109:
#line 344 "parser.y" /* yacc.c:1646  */
    { (yyval.expression) = (yyvsp[0].expression); }
#line 2331 "parser.tab.c" /* yacc.c:1646  */
    break;

  case 110:
#line 346 "parser.y" /* yacc.c:1646  */
    { (yyval.expression) = new_binary_expression(ctx, EXPR_IDIV, (yyvsp[-2].expression), (yyvsp[0].expression)); CHECK_ERROR; }
#line 2337 "parser.tab.c" /* yacc.c:1646  */
    break;

  case 111:
#line 349 "parser.y" /* yacc.c:1646  */
    { (yyval.expression) = (yyvsp[0].expression); }
#line 2343 "parser.tab.c" /* yacc.c:1646  */
    break;

  case 112:
#line 351 "parser.y" /* yacc.c:1646  */
    { (yyval.expression) = new_binary_expression(ctx, EXPR_MUL, (yyvsp[-2].expression), (yyvsp[0].expression)); CHECK_ERROR; }
#line 2349 "parser.tab.c" /* yacc.c:1646  */
    break;

  case 113:
#line 353 "parser.y" /* yacc.c:1646  */
    { (yyval.expression) = new_binary_expression(ctx, EXPR_DIV, (yyvsp[-2].expression), (yyvsp[0].expression)); CHECK_ERROR; }
#line 2355 "parser.tab.c" /* yacc.c:1646  */
    break;

  case 114:
#line 356 "parser.y" /* yacc.c:1646  */
    { (yyval.expression) = (yyvsp[0].expression); }
#line 2361 "parser.tab.c" /* yacc.c:1646  */
    break;

  case 115:
#line 357 "parser.y" /* yacc.c:1646  */
    { (yyval.expression) = new_binary_expression(ctx, EXPR_EXP, (yyvsp[-2].expression), (yyvsp[0].expression)); CHECK_ERROR; }
#line 2367 "parser.tab.c" /* yacc.c:1646  */
    break;

  case 116:
#line 360 "parser.y" /* yacc.c:1646  */
    { (yyval.expression) = (yyvsp[0].expression); }
#line 2373 "parser.tab.c" /* yacc.c:1646  */
    break;

  case 117:
#line 361 "parser.y" /* yacc.c:1646  */
    { (yyval.expression) = (yyvsp[0].expression); }
#line 2379 "parser.tab.c" /* yacc.c:1646  */
    break;

  case 118:
#line 362 "parser.y" /* yacc.c:1646  */
    { (yyval.expression) = new_new_expression(ctx, (yyvsp[0].string)); CHECK_ERROR; }
#line 2385 "parser.tab.c" /* yacc.c:1646  */
    break;

  case 119:
#line 363 "parser.y" /* yacc.c:1646  */
    { (yyval.expression) = new_unary_expression(ctx, EXPR_NEG, (yyvsp[0].expression)); CHECK_ERROR; }
#line 2391 "parser.tab.c" /* yacc.c:1646  */
    break;

  case 120:
#line 366 "parser.y" /* yacc.c:1646  */
    { (yyval.expression) = (yyvsp[0].expression); }
#line 2397 "parser.tab.c" /* yacc.c:1646  */
    break;

  case 121:
#line 367 "parser.y" /* yacc.c:1646  */
    { (yyvsp[-1].member)->args = (yyvsp[0].expression); (yyval.expression) = &(yyvsp[-1].member)->expr; }
#line 2403 "parser.tab.c" /* yacc.c:1646  */
    break;

  case 122:
#line 370 "parser.y" /* yacc.c:1646  */
    { (yyval.expression) = new_bool_expression(ctx, VARIANT_TRUE); CHECK_ERROR; }
#line 2409 "parser.tab.c" /* yacc.c:1646  */
    break;

  case 123:
#line 371 "parser.y" /* yacc.c:1646  */
    { (yyval.expression) = new_bool_expression(ctx, VARIANT_FALSE); CHECK_ERROR; }
#line 2415 "parser.tab.c" /* yacc.c:1646  */
    break;

  case 124:
#line 372 "parser.y" /* yacc.c:1646  */
    { (yyval.expression) = new_string_expression(ctx, (yyvsp[0].string)); CHECK_ERROR; }
#line 2421 "parser.tab.c" /* yacc.c:1646  */
    break;

  case 125:
#line 373 "parser.y" /* yacc.c:1646  */
    { (yyval.expression) = (yyvsp[0].expression); }
#line 2427 "parser.tab.c" /* yacc.c:1646  */
    break;

  case 126:
#line 374 "parser.y" /* yacc.c:1646  */
    { (yyval.expression) = new_expression(ctx, EXPR_EMPTY, 0); CHECK_ERROR; }
#line 2433 "parser.tab.c" /* yacc.c:1646  */
    break;

  case 127:
#line 375 "parser.y" /* yacc.c:1646  */
    { (yyval.expression) = new_expression(ctx, EXPR_NULL, 0); CHECK_ERROR; }
#line 2439 "parser.tab.c" /* yacc.c:1646  */
    break;

  case 128:
#line 376 "parser.y" /* yacc.c:1646  */
    { (yyval.expression) = new_expression(ctx, EXPR_NOTHING, 0); CHECK_ERROR; }
#line 2445 "parser.tab.c" /* yacc.c:1646  */
    break;

  case 129:
#line 379 "parser.y" /* yacc.c:1646  */
    { (yyval.expression) = new_long_expression(ctx, EXPR_USHORT, (yyvsp[0].lng)); CHECK_ERROR; }
#line 2451 "parser.tab.c" /* yacc.c:1646  */
    break;

  case 130:
#line 380 "parser.y" /* yacc.c:1646  */
    { (yyval.expression) = new_long_expression(ctx, EXPR_USHORT, 0); CHECK_ERROR; }
#line 2457 "parser.tab.c" /* yacc.c:1646  */
    break;

  case 131:
#line 381 "parser.y" /* yacc.c:1646  */
    { (yyval.expression) = new_long_expression(ctx, EXPR_ULONG, (yyvsp[0].lng)); CHECK_ERROR; }
#line 2463 "parser.tab.c" /* yacc.c:1646  */
    break;

  case 132:
#line 382 "parser.y" /* yacc.c:1646  */
    { (yyval.expression) = new_double_expression(ctx, (yyvsp[0].dbl)); CHECK_ERROR; }
#line 2469 "parser.tab.c" /* yacc.c:1646  */
    break;

  case 133:
#line 385 "parser.y" /* yacc.c:1646  */
    { (yyval.uint) = (yyvsp[0].lng); }
#line 2475 "parser.tab.c" /* yacc.c:1646  */
    break;

  case 134:
#line 386 "parser.y" /* yacc.c:1646  */
    { (yyval.uint) = 0; }
#line 2481 "parser.tab.c" /* yacc.c:1646  */
    break;

  case 135:
#line 387 "parser.y" /* yacc.c:1646  */
    { (yyval.uint) = (yyvsp[0].lng); }
#line 2487 "parser.tab.c" /* yacc.c:1646  */
    break;

  case 136:
#line 390 "parser.y" /* yacc.c:1646  */
    { (yyval.expression) = new_unary_expression(ctx, EXPR_BRACKETS, (yyvsp[-1].expression)); }
#line 2493 "parser.tab.c" /* yacc.c:1646  */
    break;

  case 137:
#line 391 "parser.y" /* yacc.c:1646  */
    { (yyval.expression) = new_expression(ctx, EXPR_ME, 0); CHECK_ERROR; }
#line 2499 "parser.tab.c" /* yacc.c:1646  */
    break;

  case 138:
#line 394 "parser.y" /* yacc.c:1646  */
    { (yyvsp[-3].class_decl)->name = (yyvsp[-5].string); (yyval.class_decl) = (yyvsp[-3].class_decl); }
#line 2505 "parser.tab.c" /* yacc.c:1646  */
    break;

  case 139:
#line 397 "parser.y" /* yacc.c:1646  */
    { (yyval.class_decl) = new_class_decl(ctx); }
#line 2511 "parser.tab.c" /* yacc.c:1646  */
    break;

  case 140:
#line 398 "parser.y" /* yacc.c:1646  */
    { (yyval.class_decl) = add_class_function(ctx, (yyvsp[0].class_decl), (yyvsp[-2].func_decl)); CHECK_ERROR; }
#line 2517 "parser.tab.c" /* yacc.c:1646  */
    break;

  case 141:
#line 400 "parser.y" /* yacc.c:1646  */
    { dim_decl_t *dim_decl = new_dim_decl(ctx, (yyvsp[-2].string), FALSE, NULL); CHECK_ERROR;
                                                  (yyval.class_decl) = add_dim_prop(ctx, (yyvsp[0].class_decl), dim_decl, (yyvsp[-3].uint)); CHECK_ERROR; }
#line 2524 "parser.tab.c" /* yacc.c:1646  */
    break;

  case 142:
#line 402 "parser.y" /* yacc.c:1646  */
    { (yyval.class_decl) = add_dim_prop(ctx, (yyvsp[0].class_decl), (yyvsp[-2].dim_decl), 0); CHECK_ERROR; }
#line 2530 "parser.tab.c" /* yacc.c:1646  */
    break;

  case 143:
#line 403 "parser.y" /* yacc.c:1646  */
    { (yyval.class_decl) = add_class_function(ctx, (yyvsp[0].class_decl), (yyvsp[-2].func_decl)); CHECK_ERROR; }
#line 2536 "parser.tab.c" /* yacc.c:1646  */
    break;

  case 144:
#line 407 "parser.y" /* yacc.c:1646  */
    { (yyval.func_decl) = new_function_decl(ctx, (yyvsp[-5].string), FUNC_PROPGET, (yyvsp[-8].uint), (yyvsp[-4].arg_decl), (yyvsp[-2].statement)); CHECK_ERROR; }
#line 2542 "parser.tab.c" /* yacc.c:1646  */
    break;

  case 145:
#line 409 "parser.y" /* yacc.c:1646  */
    { (yyval.func_decl) = new_function_decl(ctx, (yyvsp[-7].string), FUNC_PROPLET, (yyvsp[-10].uint), (yyvsp[-5].arg_decl), (yyvsp[-2].statement)); CHECK_ERROR; }
#line 2548 "parser.tab.c" /* yacc.c:1646  */
    break;

  case 146:
#line 411 "parser.y" /* yacc.c:1646  */
    { (yyval.func_decl) = new_function_decl(ctx, (yyvsp[-7].string), FUNC_PROPSET, (yyvsp[-10].uint), (yyvsp[-5].arg_decl), (yyvsp[-2].statement)); CHECK_ERROR; }
#line 2554 "parser.tab.c" /* yacc.c:1646  */
    break;

  case 147:
#line 415 "parser.y" /* yacc.c:1646  */
    { (yyval.func_decl) = new_function_decl(ctx, (yyvsp[-5].string), FUNC_SUB, (yyvsp[-7].uint), (yyvsp[-4].arg_decl), (yyvsp[-2].statement)); CHECK_ERROR; }
#line 2560 "parser.tab.c" /* yacc.c:1646  */
    break;

  case 148:
#line 417 "parser.y" /* yacc.c:1646  */
    { (yyval.func_decl) = new_function_decl(ctx, (yyvsp[-5].string), FUNC_FUNCTION, (yyvsp[-7].uint), (yyvsp[-4].arg_decl), (yyvsp[-2].statement)); CHECK_ERROR; }
#line 2566 "parser.tab.c" /* yacc.c:1646  */
    break;

  case 149:
#line 420 "parser.y" /* yacc.c:1646  */
    { (yyval.uint) = 0; }
#line 2572 "parser.tab.c" /* yacc.c:1646  */
    break;

  case 150:
#line 421 "parser.y" /* yacc.c:1646  */
    { (yyval.uint) = (yyvsp[0].uint); }
#line 2578 "parser.tab.c" /* yacc.c:1646  */
    break;

  case 151:
#line 424 "parser.y" /* yacc.c:1646  */
    { (yyval.uint) = STORAGE_IS_DEFAULT; }
#line 2584 "parser.tab.c" /* yacc.c:1646  */
    break;

  case 152:
#line 425 "parser.y" /* yacc.c:1646  */
    { (yyval.uint) = 0; }
#line 2590 "parser.tab.c" /* yacc.c:1646  */
    break;

  case 153:
#line 426 "parser.y" /* yacc.c:1646  */
    { (yyval.uint) = STORAGE_IS_PRIVATE; }
#line 2596 "parser.tab.c" /* yacc.c:1646  */
    break;

  case 154:
#line 429 "parser.y" /* yacc.c:1646  */
    { (yyval.arg_decl) = NULL; }
#line 2602 "parser.tab.c" /* yacc.c:1646  */
    break;

  case 155:
#line 430 "parser.y" /* yacc.c:1646  */
    { (yyval.arg_decl) = (yyvsp[-1].arg_decl); }
#line 2608 "parser.tab.c" /* yacc.c:1646  */
    break;

  case 156:
#line 433 "parser.y" /* yacc.c:1646  */
    { (yyval.arg_decl) = (yyvsp[0].arg_decl); }
#line 2614 "parser.tab.c" /* yacc.c:1646  */
    break;

  case 157:
#line 434 "parser.y" /* yacc.c:1646  */
    { (yyvsp[-2].arg_decl)->next = (yyvsp[0].arg_decl); (yyval.arg_decl) = (yyvsp[-2].arg_decl); }
#line 2620 "parser.tab.c" /* yacc.c:1646  */
    break;

  case 158:
#line 437 "parser.y" /* yacc.c:1646  */
    { (yyval.arg_decl) = new_argument_decl(ctx, (yyvsp[-1].string), TRUE); }
#line 2626 "parser.tab.c" /* yacc.c:1646  */
    break;

  case 159:
#line 438 "parser.y" /* yacc.c:1646  */
    { (yyval.arg_decl) = new_argument_decl(ctx, (yyvsp[-1].string), TRUE); }
#line 2632 "parser.tab.c" /* yacc.c:1646  */
    break;

  case 160:
#line 439 "parser.y" /* yacc.c:1646  */
    { (yyval.arg_decl) = new_argument_decl(ctx, (yyvsp[-1].string), FALSE); }
#line 2638 "parser.tab.c" /* yacc.c:1646  */
    break;

  case 161:
#line 443 "parser.y" /* yacc.c:1646  */
    { (yyval.string) = (yyvsp[0].string); }
#line 2644 "parser.tab.c" /* yacc.c:1646  */
    break;

  case 162:
#line 444 "parser.y" /* yacc.c:1646  */
    { (yyval.string) = propertyW; }
#line 2650 "parser.tab.c" /* yacc.c:1646  */
    break;


#line 2654 "parser.tab.c" /* yacc.c:1646  */
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
  YY_SYMBOL_PRINT ("-> $$ =", yyr1[yyn], &yyval, &yyloc);

  YYPOPSTACK (yylen);
  yylen = 0;
  YY_STACK_PRINT (yyss, yyssp);

  *++yyvsp = yyval;

  /* Now 'shift' the result of the reduction.  Determine what state
     that goes to, based on the state we popped back to and the rule
     number reduced by.  */

  yyn = yyr1[yyn];

  yystate = yypgoto[yyn - YYNTOKENS] + *yyssp;
  if (0 <= yystate && yystate <= YYLAST && yycheck[yystate] == *yyssp)
    yystate = yytable[yystate];
  else
    yystate = yydefgoto[yyn - YYNTOKENS];

  goto yynewstate;


/*--------------------------------------.
| yyerrlab -- here on detecting error.  |
`--------------------------------------*/
yyerrlab:
  /* Make sure we have latest lookahead translation.  See comments at
     user semantic actions for why this is necessary.  */
  yytoken = yychar == YYEMPTY ? YYEMPTY : YYTRANSLATE (yychar);

  /* If not already recovering from an error, report this error.  */
  if (!yyerrstatus)
    {
      ++yynerrs;
#if ! YYERROR_VERBOSE
      yyerror (ctx, YY_("syntax error"));
#else
# define YYSYNTAX_ERROR yysyntax_error (&yymsg_alloc, &yymsg, \
                                        yyssp, yytoken)
      {
        char const *yymsgp = YY_("syntax error");
        int yysyntax_error_status;
        yysyntax_error_status = YYSYNTAX_ERROR;
        if (yysyntax_error_status == 0)
          yymsgp = yymsg;
        else if (yysyntax_error_status == 1)
          {
            if (yymsg != yymsgbuf)
              YYSTACK_FREE (yymsg);
            yymsg = (char *) YYSTACK_ALLOC (yymsg_alloc);
            if (!yymsg)
              {
                yymsg = yymsgbuf;
                yymsg_alloc = sizeof yymsgbuf;
                yysyntax_error_status = 2;
              }
            else
              {
                yysyntax_error_status = YYSYNTAX_ERROR;
                yymsgp = yymsg;
              }
          }
        yyerror (ctx, yymsgp);
        if (yysyntax_error_status == 2)
          goto yyexhaustedlab;
      }
# undef YYSYNTAX_ERROR
#endif
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
                      yytoken, &yylval, ctx);
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

  /* Pacify compilers like GCC when the user code never invokes
     YYERROR and the label yyerrorlab therefore never appears in user
     code.  */
  if (/*CONSTCOND*/ 0)
     goto yyerrorlab;

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

  for (;;)
    {
      yyn = yypact[yystate];
      if (!yypact_value_is_default (yyn))
        {
          yyn += YYTERROR;
          if (0 <= yyn && yyn <= YYLAST && yycheck[yyn] == YYTERROR)
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
                  yystos[yystate], yyvsp, ctx);
      YYPOPSTACK (1);
      yystate = *yyssp;
      YY_STACK_PRINT (yyss, yyssp);
    }

  YY_IGNORE_MAYBE_UNINITIALIZED_BEGIN
  *++yyvsp = yylval;
  YY_IGNORE_MAYBE_UNINITIALIZED_END


  /* Shift the error token.  */
  YY_SYMBOL_PRINT ("Shifting", yystos[yyn], yyvsp, yylsp);

  yystate = yyn;
  goto yynewstate;


/*-------------------------------------.
| yyacceptlab -- YYACCEPT comes here.  |
`-------------------------------------*/
yyacceptlab:
  yyresult = 0;
  goto yyreturn;

/*-----------------------------------.
| yyabortlab -- YYABORT comes here.  |
`-----------------------------------*/
yyabortlab:
  yyresult = 1;
  goto yyreturn;

#if !defined yyoverflow || YYERROR_VERBOSE
/*-------------------------------------------------.
| yyexhaustedlab -- memory exhaustion comes here.  |
`-------------------------------------------------*/
yyexhaustedlab:
  yyerror (ctx, YY_("memory exhausted"));
  yyresult = 2;
  /* Fall through.  */
#endif

yyreturn:
  if (yychar != YYEMPTY)
    {
      /* Make sure we have latest lookahead translation.  See comments at
         user semantic actions for why this is necessary.  */
      yytoken = YYTRANSLATE (yychar);
      yydestruct ("Cleanup: discarding lookahead",
                  yytoken, &yylval, ctx);
    }
  /* Do not reclaim the symbols of the rule whose action triggered
     this YYABORT or YYACCEPT.  */
  YYPOPSTACK (yylen);
  YY_STACK_PRINT (yyss, yyssp);
  while (yyssp != yyss)
    {
      yydestruct ("Cleanup: popping",
                  yystos[*yyssp], yyvsp, ctx);
      YYPOPSTACK (1);
    }
#ifndef yyoverflow
  if (yyss != yyssa)
    YYSTACK_FREE (yyss);
#endif
#if YYERROR_VERBOSE
  if (yymsg != yymsgbuf)
    YYSTACK_FREE (yymsg);
#endif
  return yyresult;
}
#line 451 "parser.y" /* yacc.c:1906  */


static int parser_error(parser_ctx_t *ctx, const char *str)
{
    return 0;
}

static void source_add_statement(parser_ctx_t *ctx, statement_t *stat)
{
    if(!stat)
        return;

    if(ctx->stats) {
        ctx->stats_tail->next = stat;
        ctx->stats_tail = stat;
    }else {
        ctx->stats = ctx->stats_tail = stat;
    }
}

static void source_add_class(parser_ctx_t *ctx, class_decl_t *class_decl)
{
    class_decl->next = ctx->class_decls;
    ctx->class_decls = class_decl;
}

static void parse_complete(parser_ctx_t *ctx, BOOL option_explicit)
{
    ctx->parse_complete = TRUE;
    ctx->option_explicit = option_explicit;
}

static void *new_expression(parser_ctx_t *ctx, expression_type_t type, size_t size)
{
    expression_t *expr;

    expr = parser_alloc(ctx, size ? size : sizeof(*expr));
    if(expr) {
        expr->type = type;
        expr->next = NULL;
    }

    return expr;
}

static expression_t *new_bool_expression(parser_ctx_t *ctx, VARIANT_BOOL value)
{
    bool_expression_t *expr;

    expr = new_expression(ctx, EXPR_BOOL, sizeof(*expr));
    if(!expr)
        return NULL;

    expr->value = value;
    return &expr->expr;
}

static expression_t *new_string_expression(parser_ctx_t *ctx, const WCHAR *value)
{
    string_expression_t *expr;

    expr = new_expression(ctx, EXPR_STRING, sizeof(*expr));
    if(!expr)
        return NULL;

    expr->value = value;
    return &expr->expr;
}

static expression_t *new_long_expression(parser_ctx_t *ctx, expression_type_t type, LONG value)
{
    int_expression_t *expr;

    expr = new_expression(ctx, type, sizeof(*expr));
    if(!expr)
        return NULL;

    expr->value = value;
    return &expr->expr;
}

static expression_t *new_double_expression(parser_ctx_t *ctx, double value)
{
    double_expression_t *expr;

    expr = new_expression(ctx, EXPR_DOUBLE, sizeof(*expr));
    if(!expr)
        return NULL;

    expr->value = value;
    return &expr->expr;
}

static expression_t *new_unary_expression(parser_ctx_t *ctx, expression_type_t type, expression_t *subexpr)
{
    unary_expression_t *expr;

    expr = new_expression(ctx, type, sizeof(*expr));
    if(!expr)
        return NULL;

    expr->subexpr = subexpr;
    return &expr->expr;
}

static expression_t *new_binary_expression(parser_ctx_t *ctx, expression_type_t type, expression_t *left, expression_t *right)
{
    binary_expression_t *expr;

    expr = new_expression(ctx, type, sizeof(*expr));
    if(!expr)
        return NULL;

    expr->left = left;
    expr->right = right;
    return &expr->expr;
}

static member_expression_t *new_member_expression(parser_ctx_t *ctx, expression_t *obj_expr, const WCHAR *identifier)
{
    member_expression_t *expr;

    expr = new_expression(ctx, EXPR_MEMBER, sizeof(*expr));
    if(!expr)
        return NULL;

    expr->obj_expr = obj_expr;
    expr->identifier = identifier;
    expr->args = NULL;
    return expr;
}

static expression_t *new_new_expression(parser_ctx_t *ctx, const WCHAR *identifier)
{
    string_expression_t *expr;

    expr = new_expression(ctx, EXPR_NEW, sizeof(*expr));
    if(!expr)
        return NULL;

    expr->value = identifier;
    return &expr->expr;
}

static void *new_statement(parser_ctx_t *ctx, statement_type_t type, size_t size)
{
    statement_t *stat;

    stat = parser_alloc(ctx, size ? size : sizeof(*stat));
    if(stat) {
        stat->type = type;
        stat->next = NULL;
    }

    return stat;
}

static statement_t *new_call_statement(parser_ctx_t *ctx, BOOL is_strict, member_expression_t *expr)
{
    call_statement_t *stat;

    stat = new_statement(ctx, STAT_CALL, sizeof(*stat));
    if(!stat)
        return NULL;

    stat->expr = expr;
    stat->is_strict = is_strict;
    return &stat->stat;
}

static statement_t *new_assign_statement(parser_ctx_t *ctx, member_expression_t *left, expression_t *right)
{
    assign_statement_t *stat;

    stat = new_statement(ctx, STAT_ASSIGN, sizeof(*stat));
    if(!stat)
        return NULL;

    stat->member_expr = left;
    stat->value_expr = right;
    return &stat->stat;
}

static statement_t *new_set_statement(parser_ctx_t *ctx, member_expression_t *left, expression_t *right)
{
    assign_statement_t *stat;

    stat = new_statement(ctx, STAT_SET, sizeof(*stat));
    if(!stat)
        return NULL;

    stat->member_expr = left;
    stat->value_expr = right;
    return &stat->stat;
}

static dim_decl_t *new_dim_decl(parser_ctx_t *ctx, const WCHAR *name, BOOL is_array, dim_list_t *dims)
{
    dim_decl_t *decl;

    decl = parser_alloc(ctx, sizeof(*decl));
    if(!decl)
        return NULL;

    decl->name = name;
    decl->is_array = is_array;
    decl->dims = dims;
    decl->next = NULL;
    return decl;
}

static dim_list_t *new_dim(parser_ctx_t *ctx, unsigned val, dim_list_t *next)
{
    dim_list_t *ret;

    ret = parser_alloc(ctx, sizeof(*ret));
    if(!ret)
        return NULL;

    ret->val = val;
    ret->next = next;
    return ret;
}

static statement_t *new_dim_statement(parser_ctx_t *ctx, dim_decl_t *decls)
{
    dim_statement_t *stat;

    stat = new_statement(ctx, STAT_DIM, sizeof(*stat));
    if(!stat)
        return NULL;

    stat->dim_decls = decls;
    return &stat->stat;
}

static elseif_decl_t *new_elseif_decl(parser_ctx_t *ctx, expression_t *expr, statement_t *stat)
{
    elseif_decl_t *decl;

    decl = parser_alloc(ctx, sizeof(*decl));
    if(!decl)
        return NULL;

    decl->expr = expr;
    decl->stat = stat;
    decl->next = NULL;
    return decl;
}

static statement_t *new_while_statement(parser_ctx_t *ctx, statement_type_t type, expression_t *expr, statement_t *body)
{
    while_statement_t *stat;

    stat = new_statement(ctx, type, sizeof(*stat));
    if(!stat)
        return NULL;

    stat->expr = expr;
    stat->body = body;
    return &stat->stat;
}

static statement_t *new_forto_statement(parser_ctx_t *ctx, const WCHAR *identifier, expression_t *from_expr,
        expression_t *to_expr, expression_t *step_expr, statement_t *body)
{
    forto_statement_t *stat;

    stat = new_statement(ctx, STAT_FORTO, sizeof(*stat));
    if(!stat)
        return NULL;

    stat->identifier = identifier;
    stat->from_expr = from_expr;
    stat->to_expr = to_expr;
    stat->step_expr = step_expr;
    stat->body = body;
    return &stat->stat;
}

static statement_t *new_foreach_statement(parser_ctx_t *ctx, const WCHAR *identifier, expression_t *group_expr,
        statement_t *body)
{
    foreach_statement_t *stat;

    stat = new_statement(ctx, STAT_FOREACH, sizeof(*stat));
    if(!stat)
        return NULL;

    stat->identifier = identifier;
    stat->group_expr = group_expr;
    stat->body = body;
    return &stat->stat;
}

static statement_t *new_if_statement(parser_ctx_t *ctx, expression_t *expr, statement_t *if_stat, elseif_decl_t *elseif_decl,
        statement_t *else_stat)
{
    if_statement_t *stat;

    stat = new_statement(ctx, STAT_IF, sizeof(*stat));
    if(!stat)
        return NULL;

    stat->expr = expr;
    stat->if_stat = if_stat;
    stat->elseifs = elseif_decl;
    stat->else_stat = else_stat;
    return &stat->stat;
}

static statement_t *new_select_statement(parser_ctx_t *ctx, expression_t *expr, case_clausule_t *case_clausules)
{
    select_statement_t *stat;

    stat = new_statement(ctx, STAT_SELECT, sizeof(*stat));
    if(!stat)
        return NULL;

    stat->expr = expr;
    stat->case_clausules = case_clausules;
    return &stat->stat;
}

static case_clausule_t *new_case_clausule(parser_ctx_t *ctx, expression_t *expr, statement_t *stat, case_clausule_t *next)
{
    case_clausule_t *ret;

    ret = parser_alloc(ctx, sizeof(*ret));
    if(!ret)
        return NULL;

    ret->expr = expr;
    ret->stat = stat;
    ret->next = next;
    return ret;
}

static statement_t *new_onerror_statement(parser_ctx_t *ctx, BOOL resume_next)
{
    onerror_statement_t *stat;

    stat = new_statement(ctx, STAT_ONERROR, sizeof(*stat));
    if(!stat)
        return NULL;

    stat->resume_next = resume_next;
    return &stat->stat;
}

static arg_decl_t *new_argument_decl(parser_ctx_t *ctx, const WCHAR *name, BOOL by_ref)
{
    arg_decl_t *arg_decl;

    arg_decl = parser_alloc(ctx, sizeof(*arg_decl));
    if(!arg_decl)
        return NULL;

    arg_decl->name = name;
    arg_decl->by_ref = by_ref;
    arg_decl->next = NULL;
    return arg_decl;
}

static function_decl_t *new_function_decl(parser_ctx_t *ctx, const WCHAR *name, function_type_t type,
        unsigned storage_flags, arg_decl_t *arg_decl, statement_t *body)
{
    function_decl_t *decl;

    if(storage_flags & STORAGE_IS_DEFAULT) {
        if(type == FUNC_PROPGET) {
            type = FUNC_DEFGET;
        }else {
            FIXME("Invalid default property\n");
            ctx->hres = E_FAIL;
            return NULL;
        }
    }

    decl = parser_alloc(ctx, sizeof(*decl));
    if(!decl)
        return NULL;

    decl->name = name;
    decl->type = type;
    decl->is_public = !(storage_flags & STORAGE_IS_PRIVATE);
    decl->args = arg_decl;
    decl->body = body;
    decl->next = NULL;
    decl->next_prop_func = NULL;
    return decl;
}

static statement_t *new_function_statement(parser_ctx_t *ctx, function_decl_t *decl)
{
    function_statement_t *stat;

    stat = new_statement(ctx, STAT_FUNC, sizeof(*stat));
    if(!stat)
        return NULL;

    stat->func_decl = decl;
    return &stat->stat;
}

static class_decl_t *new_class_decl(parser_ctx_t *ctx)
{
    class_decl_t *class_decl;

    class_decl = parser_alloc(ctx, sizeof(*class_decl));
    if(!class_decl)
        return NULL;

    class_decl->funcs = NULL;
    class_decl->props = NULL;
    class_decl->next = NULL;
    return class_decl;
}

static class_decl_t *add_class_function(parser_ctx_t *ctx, class_decl_t *class_decl, function_decl_t *decl)
{
    function_decl_t *iter;

    for(iter = class_decl->funcs; iter; iter = iter->next) {
        if(!strcmpiW(iter->name, decl->name)) {
            if(decl->type == FUNC_SUB || decl->type == FUNC_FUNCTION) {
                FIXME("Redefinition of %s::%s\n", debugstr_w(class_decl->name), debugstr_w(decl->name));
                ctx->hres = E_FAIL;
                return NULL;
            }

            while(1) {
                if(iter->type == decl->type) {
                    FIXME("Redefinition of %s::%s\n", debugstr_w(class_decl->name), debugstr_w(decl->name));
                    ctx->hres = E_FAIL;
                    return NULL;
                }
                if(!iter->next_prop_func)
                    break;
                iter = iter->next_prop_func;
            }

            iter->next_prop_func = decl;
            return class_decl;
        }
    }

    decl->next = class_decl->funcs;
    class_decl->funcs = decl;
    return class_decl;
}

static class_decl_t *add_dim_prop(parser_ctx_t *ctx, class_decl_t *class_decl, dim_decl_t *dim_decl, unsigned storage_flags)
{
    if(storage_flags & STORAGE_IS_DEFAULT) {
        FIXME("variant prop van't be default value\n");
        ctx->hres = E_FAIL;
        return NULL;
    }

    dim_decl->is_public = !(storage_flags & STORAGE_IS_PRIVATE);
    dim_decl->next = class_decl->props;
    class_decl->props = dim_decl;
    return class_decl;
}

static const_decl_t *new_const_decl(parser_ctx_t *ctx, const WCHAR *name, expression_t *expr)
{
    const_decl_t *decl;

    decl = parser_alloc(ctx, sizeof(*decl));
    if(!decl)
        return NULL;

    decl->name = name;
    decl->value_expr = expr;
    decl->next = NULL;
    return decl;
}

static statement_t *new_const_statement(parser_ctx_t *ctx, const_decl_t *decls)
{
    const_statement_t *stat;

    stat = new_statement(ctx, STAT_CONST, sizeof(*stat));
    if(!stat)
        return NULL;

    stat->decls = decls;
    return &stat->stat;
}

static statement_t *link_statements(statement_t *head, statement_t *tail)
{
    statement_t *iter;

    for(iter = head; iter->next; iter = iter->next);
    iter->next = tail;

    return head;
}

void *parser_alloc(parser_ctx_t *ctx, size_t size)
{
    void *ret;

    ret = heap_pool_alloc(&ctx->heap, size);
    if(!ret)
        ctx->hres = E_OUTOFMEMORY;
    return ret;
}

HRESULT parse_script(parser_ctx_t *ctx, const WCHAR *code, const WCHAR *delimiter)
{
    const WCHAR html_delimiterW[] = {'<','/','s','c','r','i','p','t','>',0};

    ctx->code = ctx->ptr = code;
    ctx->end = ctx->code + strlenW(ctx->code);

    heap_pool_init(&ctx->heap);

    ctx->parse_complete = FALSE;
    ctx->hres = S_OK;

    ctx->last_token = tNL;
    ctx->last_nl = 0;
    ctx->stats = ctx->stats_tail = NULL;
    ctx->class_decls = NULL;
    ctx->option_explicit = FALSE;
    ctx->is_html = delimiter && !strcmpiW(delimiter, html_delimiterW);

    parser_parse(ctx);

    if(FAILED(ctx->hres))
        return ctx->hres;
    if(!ctx->parse_complete) {
        FIXME("parser failed around %s\n", debugstr_w(ctx->code+20 > ctx->ptr ? ctx->code : ctx->ptr-20));
        return E_FAIL;
    }

    return S_OK;
}

void parser_release(parser_ctx_t *ctx)
{
    heap_pool_free(&ctx->heap);
}
