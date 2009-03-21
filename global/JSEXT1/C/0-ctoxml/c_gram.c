/* A Bison parser, made by GNU Bison 2.3.  */

/* Skeleton implementation for Bison's Yacc-like parsers in C

   Copyright (C) 1984, 1989, 1990, 2000, 2001, 2002, 2003, 2004, 2005, 2006
   Free Software Foundation, Inc.

   This program is free software; you can redistribute it and/or modify
   it under the terms of the GNU General Public License as published by
   the Free Software Foundation; either version 2, or (at your option)
   any later version.

   This program is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
   GNU General Public License for more details.

   You should have received a copy of the GNU General Public License
   along with this program; if not, write to the Free Software
   Foundation, Inc., 51 Franklin Street, Fifth Floor,
   Boston, MA 02110-1301, USA.  */

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
#define YYBISON_VERSION "2.3"

/* Skeleton name.  */
#define YYSKELETON_NAME "yacc.c"

/* Pure parsers.  */
#define YYPURE 0

/* Using locations.  */
#define YYLSP_NEEDED 0

/* Substitute the variable and function names.  */
#define yyparse ctoxml_cparse
#define yylex   ctoxml_clex
#define yyerror ctoxml_cerror
#define yylval  ctoxml_clval
#define yychar  ctoxml_cchar
#define yydebug ctoxml_cdebug
#define yynerrs ctoxml_cnerrs


/* Tokens.  */
#ifndef YYTOKENTYPE
# define YYTOKENTYPE
   /* Put the tokens into the symbol table, so that GDB and other debuggers
      know about them.  */
   enum yytokentype {
     IDENTIFIER = 258,
     CONSTANT = 259,
     STRING_LITERAL = 260,
     SIZEOF = 261,
     PTR_OP = 262,
     INC_OP = 263,
     DEC_OP = 264,
     LEFT_OP = 265,
     RIGHT_OP = 266,
     LE_OP = 267,
     GE_OP = 268,
     EQ_OP = 269,
     NE_OP = 270,
     AND_OP = 271,
     OR_OP = 272,
     MUL_ASSIGN = 273,
     DIV_ASSIGN = 274,
     MOD_ASSIGN = 275,
     ADD_ASSIGN = 276,
     SUB_ASSIGN = 277,
     LEFT_ASSIGN = 278,
     RIGHT_ASSIGN = 279,
     AND_ASSIGN = 280,
     XOR_ASSIGN = 281,
     OR_ASSIGN = 282,
     TYPE_NAME = 283,
     ASM = 284,
     DECLSPEC = 285,
     STDCALL = 286,
     CDECL = 287,
     TYPEDEF = 288,
     EXTERN = 289,
     STATIC = 290,
     AUTO = 291,
     REGISTER = 292,
     INLINE = 293,
     CHAR = 294,
     SHORT = 295,
     INT = 296,
     INT64 = 297,
     LONG = 298,
     SIGNED = 299,
     UNSIGNED = 300,
     FLOAT = 301,
     DOUBLE = 302,
     CONST = 303,
     VOLATILE = 304,
     VOID = 305,
     VA = 306,
     STRUCT = 307,
     UNION = 308,
     ENUM = 309,
     ELIPSIS = 310,
     RANGE = 311,
     CASE = 312,
     DEFAULT = 313,
     IF = 314,
     ELSE = 315,
     SWITCH = 316,
     WHILE = 317,
     DO = 318,
     FOR = 319,
     GOTO = 320,
     CONTINUE = 321,
     BREAK = 322,
     RETURN = 323
   };
#endif
/* Tokens.  */
#define IDENTIFIER 258
#define CONSTANT 259
#define STRING_LITERAL 260
#define SIZEOF 261
#define PTR_OP 262
#define INC_OP 263
#define DEC_OP 264
#define LEFT_OP 265
#define RIGHT_OP 266
#define LE_OP 267
#define GE_OP 268
#define EQ_OP 269
#define NE_OP 270
#define AND_OP 271
#define OR_OP 272
#define MUL_ASSIGN 273
#define DIV_ASSIGN 274
#define MOD_ASSIGN 275
#define ADD_ASSIGN 276
#define SUB_ASSIGN 277
#define LEFT_ASSIGN 278
#define RIGHT_ASSIGN 279
#define AND_ASSIGN 280
#define XOR_ASSIGN 281
#define OR_ASSIGN 282
#define TYPE_NAME 283
#define ASM 284
#define DECLSPEC 285
#define STDCALL 286
#define CDECL 287
#define TYPEDEF 288
#define EXTERN 289
#define STATIC 290
#define AUTO 291
#define REGISTER 292
#define INLINE 293
#define CHAR 294
#define SHORT 295
#define INT 296
#define INT64 297
#define LONG 298
#define SIGNED 299
#define UNSIGNED 300
#define FLOAT 301
#define DOUBLE 302
#define CONST 303
#define VOLATILE 304
#define VOID 305
#define VA 306
#define STRUCT 307
#define UNION 308
#define ENUM 309
#define ELIPSIS 310
#define RANGE 311
#define CASE 312
#define DEFAULT 313
#define IF 314
#define ELSE 315
#define SWITCH 316
#define WHILE 317
#define DO 318
#define FOR 319
#define GOTO 320
#define CONTINUE 321
#define BREAK 322
#define RETURN 323




/* Copy the first part of user declarations.  */


/* Enabling traces.  */
#ifndef YYDEBUG
# define YYDEBUG 0
#endif

/* Enabling verbose error messages.  */
#ifdef YYERROR_VERBOSE
# undef YYERROR_VERBOSE
# define YYERROR_VERBOSE 1
#else
# define YYERROR_VERBOSE 0
#endif

/* Enabling the token table.  */
#ifndef YYTOKEN_TABLE
# define YYTOKEN_TABLE 0
#endif

#if ! defined YYSTYPE && ! defined YYSTYPE_IS_DECLARED
typedef union YYSTYPE
#line 28 "c_gram.y"
{
  struct Xml *xml;
  char *str;
}
/* Line 187 of yacc.c.  */
#line 246 "c_gram.c"
	YYSTYPE;
# define yystype YYSTYPE /* obsolescent; will be withdrawn */
# define YYSTYPE_IS_DECLARED 1
# define YYSTYPE_IS_TRIVIAL 1
#endif



/* Copy the second part of user declarations.  */
#line 51 "c_gram.y"

#include "ctoxml.h"
#include <string.h>
#include <stdlib.h>
extern char *ctoxml_cterm;
extern char *ctoxml_cterm2;
extern int ctoxml_clineno;
extern char *ctoxml_filename;
void ctoxml_cerror (char const *msg);


/* Line 216 of yacc.c.  */
#line 269 "c_gram.c"

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
#elif (defined __STDC__ || defined __C99__FUNC__ \
     || defined __cplusplus || defined _MSC_VER)
typedef signed char yytype_int8;
#else
typedef short int yytype_int8;
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
# elif ! defined YYSIZE_T && (defined __STDC__ || defined __C99__FUNC__ \
     || defined __cplusplus || defined _MSC_VER)
#  include <stddef.h> /* INFRINGES ON USER NAME SPACE */
#  define YYSIZE_T size_t
# else
#  define YYSIZE_T unsigned int
# endif
#endif

#define YYSIZE_MAXIMUM ((YYSIZE_T) -1)

#ifndef YY_
# if YYENABLE_NLS
#  if ENABLE_NLS
#   include <libintl.h> /* INFRINGES ON USER NAME SPACE */
#   define YY_(msgid) dgettext ("bison-runtime", msgid)
#  endif
# endif
# ifndef YY_
#  define YY_(msgid) msgid
# endif
#endif

/* Suppress unused-variable warnings by "using" E.  */
#if ! defined lint || defined __GNUC__
# define YYUSE(e) ((void) (e))
#else
# define YYUSE(e) /* empty */
#endif

/* Identity function, used to suppress warnings about constant conditions.  */
#ifndef lint
# define YYID(n) (n)
#else
#if (defined __STDC__ || defined __C99__FUNC__ \
     || defined __cplusplus || defined _MSC_VER)
static int
YYID (int i)
#else
static int
YYID (i)
    int i;
#endif
{
  return i;
}
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
#    if ! defined _ALLOCA_H && ! defined _STDLIB_H && (defined __STDC__ || defined __C99__FUNC__ \
     || defined __cplusplus || defined _MSC_VER)
#     include <stdlib.h> /* INFRINGES ON USER NAME SPACE */
#     ifndef _STDLIB_H
#      define _STDLIB_H 1
#     endif
#    endif
#   endif
#  endif
# endif

# ifdef YYSTACK_ALLOC
   /* Pacify GCC's `empty if-body' warning.  */
#  define YYSTACK_FREE(Ptr) do { /* empty */; } while (YYID (0))
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
#  if (defined __cplusplus && ! defined _STDLIB_H \
       && ! ((defined YYMALLOC || defined malloc) \
	     && (defined YYFREE || defined free)))
#   include <stdlib.h> /* INFRINGES ON USER NAME SPACE */
#   ifndef _STDLIB_H
#    define _STDLIB_H 1
#   endif
#  endif
#  ifndef YYMALLOC
#   define YYMALLOC malloc
#   if ! defined malloc && ! defined _STDLIB_H && (defined __STDC__ || defined __C99__FUNC__ \
     || defined __cplusplus || defined _MSC_VER)
void *malloc (YYSIZE_T); /* INFRINGES ON USER NAME SPACE */
#   endif
#  endif
#  ifndef YYFREE
#   define YYFREE free
#   if ! defined free && ! defined _STDLIB_H && (defined __STDC__ || defined __C99__FUNC__ \
     || defined __cplusplus || defined _MSC_VER)
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
  yytype_int16 yyss;
  YYSTYPE yyvs;
  };

/* The size of the maximum gap between one aligned stack and the next.  */
# define YYSTACK_GAP_MAXIMUM (sizeof (union yyalloc) - 1)

/* The size of an array large to enough to hold all stacks, each with
   N elements.  */
# define YYSTACK_BYTES(N) \
     ((N) * (sizeof (yytype_int16) + sizeof (YYSTYPE)) \
      + YYSTACK_GAP_MAXIMUM)

/* Copy COUNT objects from FROM to TO.  The source and destination do
   not overlap.  */
# ifndef YYCOPY
#  if defined __GNUC__ && 1 < __GNUC__
#   define YYCOPY(To, From, Count) \
      __builtin_memcpy (To, From, (Count) * sizeof (*(From)))
#  else
#   define YYCOPY(To, From, Count)		\
      do					\
	{					\
	  YYSIZE_T yyi;				\
	  for (yyi = 0; yyi < (Count); yyi++)	\
	    (To)[yyi] = (From)[yyi];		\
	}					\
      while (YYID (0))
#  endif
# endif

/* Relocate STACK from its old location to the new one.  The
   local variables YYSIZE and YYSTACKSIZE give the old and new number of
   elements in the stack, and YYPTR gives the new location of the
   stack.  Advance YYPTR to a properly aligned location for the next
   stack.  */
# define YYSTACK_RELOCATE(Stack)					\
    do									\
      {									\
	YYSIZE_T yynewbytes;						\
	YYCOPY (&yyptr->Stack, Stack, yysize);				\
	Stack = &yyptr->Stack;						\
	yynewbytes = yystacksize * sizeof (*Stack) + YYSTACK_GAP_MAXIMUM; \
	yyptr += yynewbytes / sizeof (*yyptr);				\
      }									\
    while (YYID (0))

#endif

/* YYFINAL -- State number of the termination state.  */
#define YYFINAL  82
/* YYLAST -- Last index in YYTABLE.  */
#define YYLAST   1725

/* YYNTOKENS -- Number of terminals.  */
#define YYNTOKENS  93
/* YYNNTS -- Number of nonterminals.  */
#define YYNNTS  75
/* YYNRULES -- Number of rules.  */
#define YYNRULES  263
/* YYNRULES -- Number of states.  */
#define YYNSTATES  449

/* YYTRANSLATE(YYLEX) -- Bison symbol number corresponding to YYLEX.  */
#define YYUNDEFTOK  2
#define YYMAXUTOK   323

#define YYTRANSLATE(YYX)						\
  ((unsigned int) (YYX) <= YYMAXUTOK ? yytranslate[YYX] : YYUNDEFTOK)

/* YYTRANSLATE[YYLEX] -- Bison symbol number corresponding to YYLEX.  */
static const yytype_uint8 yytranslate[] =
{
       0,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,    80,     2,     2,     2,    82,    75,     2,
      69,    70,    76,    77,    74,    78,    73,    81,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,    88,    90,
      83,    89,    84,    87,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,    71,     2,    72,    85,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,    91,    86,    92,    79,     2,     2,     2,
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
      65,    66,    67,    68
};

#if YYDEBUG
/* YYPRHS[YYN] -- Index of the first RHS symbol of rule number YYN in
   YYRHS.  */
static const yytype_uint16 yyprhs[] =
{
       0,     0,     3,     5,     7,     9,    13,    15,    18,    20,
      25,    29,    34,    38,    42,    45,    48,    50,    54,    56,
      59,    62,    65,    68,    73,    75,    77,    79,    81,    83,
      85,    87,    92,    94,    98,   102,   106,   108,   112,   116,
     118,   122,   126,   128,   132,   136,   140,   144,   146,   150,
     154,   156,   160,   162,   166,   168,   172,   174,   178,   180,
     184,   186,   192,   194,   198,   200,   202,   204,   206,   208,
     210,   212,   214,   216,   218,   220,   222,   226,   228,   231,
     235,   240,   246,   250,   256,   263,   268,   270,   273,   275,
     278,   280,   283,   285,   288,   290,   294,   296,   300,   302,
     306,   311,   319,   321,   323,   325,   327,   329,   331,   333,
     335,   337,   339,   341,   343,   345,   347,   349,   351,   353,
     355,   357,   363,   369,   374,   377,   380,   382,   384,   386,
     389,   392,   396,   398,   401,   403,   406,   408,   412,   414,
     417,   421,   426,   432,   438,   445,   448,   450,   454,   456,
     460,   462,   464,   466,   468,   470,   473,   475,   478,   482,
     485,   487,   491,   495,   500,   504,   509,   514,   516,   519,
     523,   526,   528,   530,   534,   538,   543,   547,   552,   557,
     559,   562,   565,   569,   571,   574,   576,   580,   582,   586,
     589,   592,   594,   596,   600,   602,   605,   607,   609,   612,
     616,   619,   623,   627,   632,   635,   639,   643,   648,   650,
     654,   659,   661,   665,   667,   669,   671,   673,   675,   677,
     679,   683,   688,   692,   694,   697,   701,   703,   706,   708,
     710,   713,   716,   718,   721,   727,   735,   741,   747,   755,
     762,   770,   778,   787,   795,   804,   813,   823,   827,   830,
     833,   836,   840,   842,   845,   847,   849,   852,   856,   860,
     865,   867,   869,   871
};

/* YYRHS -- A `-1'-separated list of the rules' RHS.  */
static const yytype_int16 yyrhs[] =
{
     167,     0,    -1,   166,    -1,     4,    -1,    95,    -1,    69,
     114,    70,    -1,     5,    -1,    95,     5,    -1,    94,    -1,
      96,    71,   114,    72,    -1,    96,    69,    70,    -1,    96,
      69,    97,    70,    -1,    96,    73,   166,    -1,    96,     7,
     166,    -1,    96,     8,    -1,    96,     9,    -1,   112,    -1,
      97,    74,   112,    -1,    96,    -1,     8,    98,    -1,     9,
      98,    -1,    99,   100,    -1,     6,    98,    -1,     6,    69,
     147,    70,    -1,    75,    -1,    76,    -1,    77,    -1,    78,
      -1,    79,    -1,    80,    -1,    98,    -1,    69,   147,    70,
     100,    -1,   100,    -1,   101,    76,   100,    -1,   101,    81,
     100,    -1,   101,    82,   100,    -1,   101,    -1,   102,    77,
     101,    -1,   102,    78,   101,    -1,   102,    -1,   103,    10,
     102,    -1,   103,    11,   102,    -1,   103,    -1,   104,    83,
     103,    -1,   104,    84,   103,    -1,   104,    12,   103,    -1,
     104,    13,   103,    -1,   104,    -1,   105,    14,   104,    -1,
     105,    15,   104,    -1,   105,    -1,   106,    75,   105,    -1,
     106,    -1,   107,    85,   106,    -1,   107,    -1,   108,    86,
     107,    -1,   108,    -1,   109,    16,   108,    -1,   109,    -1,
     110,    17,   109,    -1,   110,    -1,   110,    87,   114,    88,
     111,    -1,   111,    -1,    98,   113,   112,    -1,    89,    -1,
      18,    -1,    19,    -1,    20,    -1,    21,    -1,    22,    -1,
      23,    -1,    24,    -1,    25,    -1,    26,    -1,    27,    -1,
     112,    -1,   114,    74,   112,    -1,   111,    -1,   117,    90,
      -1,   117,   118,    90,    -1,    33,   117,   119,    90,    -1,
      33,   117,    74,   119,    90,    -1,    33,   117,    90,    -1,
     121,    33,   117,   119,    90,    -1,   121,    33,   117,    74,
     119,    90,    -1,   121,    33,   117,    90,    -1,   122,    -1,
     122,   117,    -1,   123,    -1,   123,   117,    -1,   134,    -1,
     134,   117,    -1,   121,    -1,   121,   117,    -1,   120,    -1,
     118,    74,   120,    -1,   139,    -1,   119,    74,   139,    -1,
     137,    -1,   137,    89,   150,    -1,    30,    69,   166,    70,
      -1,    30,    69,   166,    69,    95,    70,    70,    -1,    34,
      -1,    35,    -1,    38,    -1,    36,    -1,    37,    -1,    39,
      -1,    40,    -1,    41,    -1,    42,    -1,    43,    -1,    44,
      -1,    45,    -1,    46,    -1,    47,    -1,    50,    -1,    51,
      -1,   124,    -1,   131,    -1,   165,    -1,   125,   166,    91,
     126,    92,    -1,   125,   165,    91,   126,    92,    -1,   125,
      91,   126,    92,    -1,   125,   166,    -1,   125,   165,    -1,
      52,    -1,    53,    -1,   127,    -1,   126,   127,    -1,   128,
      90,    -1,   128,   129,    90,    -1,   123,    -1,   123,   128,
      -1,   134,    -1,   134,   128,    -1,   130,    -1,   129,    74,
     130,    -1,   137,    -1,    88,   115,    -1,   137,    88,   115,
      -1,    54,    91,   132,    92,    -1,    54,   166,    91,   132,
      92,    -1,    54,    91,   132,    74,    92,    -1,    54,   166,
      91,   132,    74,    92,    -1,    54,   166,    -1,   133,    -1,
     132,    74,   133,    -1,   166,    -1,   166,    89,   115,    -1,
      48,    -1,    49,    -1,    32,    -1,    31,    -1,   135,    -1,
     136,   135,    -1,   138,    -1,   141,   138,    -1,   141,   136,
     138,    -1,   135,   137,    -1,   166,    -1,    69,   137,    70,
      -1,   138,    71,    72,    -1,   138,    71,   115,    72,    -1,
     138,    69,    70,    -1,   138,    69,   143,    70,    -1,   138,
      69,   146,    70,    -1,   138,    -1,   141,   140,    -1,   141,
     135,   140,    -1,   135,   137,    -1,   166,    -1,   165,    -1,
      69,   139,    70,    -1,   140,    71,    72,    -1,   140,    71,
     115,    72,    -1,   140,    69,    70,    -1,   140,    69,   143,
      70,    -1,   140,    69,   146,    70,    -1,    76,    -1,    76,
     142,    -1,    76,   141,    -1,    76,   142,   141,    -1,   134,
      -1,   142,   134,    -1,   144,    -1,   144,    74,    55,    -1,
     145,    -1,   144,    74,   145,    -1,   117,   139,    -1,   117,
     148,    -1,   117,    -1,   166,    -1,   146,    74,   166,    -1,
     128,    -1,   128,   148,    -1,   141,    -1,   149,    -1,   141,
     149,    -1,    69,   148,    70,    -1,    71,    72,    -1,    71,
     115,    72,    -1,   149,    71,    72,    -1,   149,    71,   115,
      72,    -1,    69,    70,    -1,    69,   143,    70,    -1,   149,
      69,    70,    -1,   149,    69,   143,    70,    -1,   112,    -1,
      91,   151,    92,    -1,    91,   151,    74,    92,    -1,   150,
      -1,   151,    74,   150,    -1,   153,    -1,   155,    -1,   158,
      -1,   159,    -1,   160,    -1,   161,    -1,   154,    -1,   166,
      88,   152,    -1,    57,   115,    88,   152,    -1,    58,    88,
     152,    -1,    29,    -1,    91,    92,    -1,    91,   157,    92,
      -1,   116,    -1,   156,   116,    -1,   116,    -1,   152,    -1,
     157,   152,    -1,   157,   116,    -1,    90,    -1,   114,    90,
      -1,    59,    69,   114,    70,   152,    -1,    59,    69,   114,
      70,   152,    60,   152,    -1,    61,    69,   114,    70,   152,
      -1,    62,    69,   114,    70,   152,    -1,    63,   152,    62,
      69,   114,    70,    90,    -1,    64,    69,    90,    90,    70,
     152,    -1,    64,    69,    90,    90,   114,    70,   152,    -1,
      64,    69,    90,   114,    90,    70,   152,    -1,    64,    69,
      90,   114,    90,   114,    70,   152,    -1,    64,    69,   114,
      90,    90,    70,   152,    -1,    64,    69,   114,    90,    90,
     114,    70,   152,    -1,    64,    69,   114,    90,   114,    90,
      70,   152,    -1,    64,    69,   114,    90,   114,    90,   114,
      70,   152,    -1,    65,   166,    90,    -1,    66,    90,    -1,
      67,    90,    -1,    68,    90,    -1,    68,   114,    90,    -1,
     163,    -1,   162,   163,    -1,   164,    -1,   116,    -1,   137,
     155,    -1,   137,   156,   155,    -1,   117,   137,   155,    -1,
     117,   137,   156,   155,    -1,    28,    -1,     3,    -1,   162,
      -1,    -1
};

/* YYRLINE[YYN] -- source line where rule number YYN was defined.  */
static const yytype_uint16 yyrline[] =
{
       0,    65,    65,    66,   100,   101,   105,   106,   110,   111,
     112,   113,   114,   115,   116,   117,   121,   122,   126,   127,
     128,   129,   130,   131,   135,   136,   137,   138,   139,   140,
     144,   145,   149,   150,   151,   152,   156,   157,   158,   162,
     163,   164,   168,   169,   170,   171,   172,   176,   177,   178,
     182,   183,   187,   188,   192,   193,   197,   198,   202,   203,
     207,   208,   212,   213,   217,   218,   219,   220,   221,   222,
     223,   224,   225,   226,   227,   231,   232,   236,   240,   241,
     242,   243,   244,   245,   246,   247,   251,   252,   253,   254,
     255,   256,   257,   258,   262,   263,   267,   268,   272,   273,
     277,   278,   282,   283,   284,   285,   286,   290,   291,   292,
     293,   294,   295,   296,   297,   298,   299,   300,   301,   302,
     303,   307,   308,   309,   310,   311,   315,   316,   320,   321,
     325,   326,   330,   331,   332,   333,   337,   338,   342,   343,
     344,   348,   349,   350,   351,   352,   356,   357,   361,   362,
     366,   367,   371,   372,   376,   377,   381,   382,   383,   384,
     388,   389,   390,   391,   392,   393,   394,   398,   399,   400,
     401,   405,   406,   407,   408,   409,   410,   411,   412,   416,
     417,   418,   419,   423,   424,   428,   429,   433,   434,   438,
     439,   440,   444,   445,   449,   450,   454,   455,   456,   460,
     461,   462,   463,   464,   465,   466,   467,   468,   472,   473,
     474,   478,   479,   483,   484,   485,   486,   487,   488,   489,
     493,   494,   495,   499,   503,   504,   508,   509,   513,   514,
     515,   516,   520,   521,   525,   526,   527,   531,   532,   533,
     534,   535,   536,   537,   538,   539,   540,   544,   545,   546,
     547,   548,   552,   553,   557,   558,   562,   563,   564,   565,
     569,   573,   577,   578
};
#endif

#if YYDEBUG || YYERROR_VERBOSE || YYTOKEN_TABLE
/* YYTNAME[SYMBOL-NUM] -- String name of the symbol SYMBOL-NUM.
   First, the terminals, then, starting at YYNTOKENS, nonterminals.  */
static const char *const yytname[] =
{
  "$end", "error", "$undefined", "IDENTIFIER", "CONSTANT",
  "STRING_LITERAL", "SIZEOF", "PTR_OP", "INC_OP", "DEC_OP", "LEFT_OP",
  "RIGHT_OP", "LE_OP", "GE_OP", "EQ_OP", "NE_OP", "AND_OP", "OR_OP",
  "MUL_ASSIGN", "DIV_ASSIGN", "MOD_ASSIGN", "ADD_ASSIGN", "SUB_ASSIGN",
  "LEFT_ASSIGN", "RIGHT_ASSIGN", "AND_ASSIGN", "XOR_ASSIGN", "OR_ASSIGN",
  "TYPE_NAME", "ASM", "DECLSPEC", "STDCALL", "CDECL", "TYPEDEF", "EXTERN",
  "STATIC", "AUTO", "REGISTER", "INLINE", "CHAR", "SHORT", "INT", "INT64",
  "LONG", "SIGNED", "UNSIGNED", "FLOAT", "DOUBLE", "CONST", "VOLATILE",
  "VOID", "VA", "STRUCT", "UNION", "ENUM", "ELIPSIS", "RANGE", "CASE",
  "DEFAULT", "IF", "ELSE", "SWITCH", "WHILE", "DO", "FOR", "GOTO",
  "CONTINUE", "BREAK", "RETURN", "'('", "')'", "'['", "']'", "'.'", "','",
  "'&'", "'*'", "'+'", "'-'", "'~'", "'!'", "'/'", "'%'", "'<'", "'>'",
  "'^'", "'|'", "'?'", "':'", "'='", "';'", "'{'", "'}'", "$accept",
  "primary_expr", "strings", "postfix_expr", "argument_expr_list",
  "unary_expr", "unary_operator", "cast_expr", "multiplicative_expr",
  "additive_expr", "shift_expr", "relational_expr", "equality_expr",
  "and_expr", "exclusive_or_expr", "inclusive_or_expr", "logical_and_expr",
  "logical_or_expr", "conditional_expr", "assignment_expr",
  "assignment_operator", "expr", "constant_expr", "declaration",
  "declaration_specifiers", "init_declarator_list", "declarator_list2",
  "init_declarator", "declspec", "storage_class_specifier",
  "type_specifier", "struct_or_union_specifier", "struct_or_union",
  "struct_declaration_list", "struct_declaration",
  "specifier_qualifier_list", "struct_declarator_list",
  "struct_declarator", "enum_specifier", "enumerator_list", "enumerator",
  "type_qualifier", "callspec", "callspecs", "declarator",
  "direct_declarator", "declarator2", "direct_declarator2", "pointer",
  "type_qualifier_list", "parameter_type_list", "parameter_list",
  "parameter_declaration", "identifier_list", "type_name",
  "abstract_declarator", "direct_abstract_declarator", "initializer",
  "initializer_list", "statement", "labeled_statement", "asm_statement",
  "compound_statement", "declaration_list", "statement_list",
  "expression_statement", "selection_statement", "iteration_statement",
  "jump_statement", "translation_unit", "external_definition",
  "function_definition", "typedeffed_name", "identifier", "file", 0
};
#endif

# ifdef YYPRINT
/* YYTOKNUM[YYLEX-NUM] -- Internal token number corresponding to
   token YYLEX-NUM.  */
static const yytype_uint16 yytoknum[] =
{
       0,   256,   257,   258,   259,   260,   261,   262,   263,   264,
     265,   266,   267,   268,   269,   270,   271,   272,   273,   274,
     275,   276,   277,   278,   279,   280,   281,   282,   283,   284,
     285,   286,   287,   288,   289,   290,   291,   292,   293,   294,
     295,   296,   297,   298,   299,   300,   301,   302,   303,   304,
     305,   306,   307,   308,   309,   310,   311,   312,   313,   314,
     315,   316,   317,   318,   319,   320,   321,   322,   323,    40,
      41,    91,    93,    46,    44,    38,    42,    43,    45,   126,
      33,    47,    37,    60,    62,    94,   124,    63,    58,    61,
      59,   123,   125
};
# endif

/* YYR1[YYN] -- Symbol number of symbol that rule YYN derives.  */
static const yytype_uint8 yyr1[] =
{
       0,    93,    94,    94,    94,    94,    95,    95,    96,    96,
      96,    96,    96,    96,    96,    96,    97,    97,    98,    98,
      98,    98,    98,    98,    99,    99,    99,    99,    99,    99,
     100,   100,   101,   101,   101,   101,   102,   102,   102,   103,
     103,   103,   104,   104,   104,   104,   104,   105,   105,   105,
     106,   106,   107,   107,   108,   108,   109,   109,   110,   110,
     111,   111,   112,   112,   113,   113,   113,   113,   113,   113,
     113,   113,   113,   113,   113,   114,   114,   115,   116,   116,
     116,   116,   116,   116,   116,   116,   117,   117,   117,   117,
     117,   117,   117,   117,   118,   118,   119,   119,   120,   120,
     121,   121,   122,   122,   122,   122,   122,   123,   123,   123,
     123,   123,   123,   123,   123,   123,   123,   123,   123,   123,
     123,   124,   124,   124,   124,   124,   125,   125,   126,   126,
     127,   127,   128,   128,   128,   128,   129,   129,   130,   130,
     130,   131,   131,   131,   131,   131,   132,   132,   133,   133,
     134,   134,   135,   135,   136,   136,   137,   137,   137,   137,
     138,   138,   138,   138,   138,   138,   138,   139,   139,   139,
     139,   140,   140,   140,   140,   140,   140,   140,   140,   141,
     141,   141,   141,   142,   142,   143,   143,   144,   144,   145,
     145,   145,   146,   146,   147,   147,   148,   148,   148,   149,
     149,   149,   149,   149,   149,   149,   149,   149,   150,   150,
     150,   151,   151,   152,   152,   152,   152,   152,   152,   152,
     153,   153,   153,   154,   155,   155,   156,   156,   157,   157,
     157,   157,   158,   158,   159,   159,   159,   160,   160,   160,
     160,   160,   160,   160,   160,   160,   160,   161,   161,   161,
     161,   161,   162,   162,   163,   163,   164,   164,   164,   164,
     165,   166,   167,   167
};

/* YYR2[YYN] -- Number of symbols composing right hand side of rule YYN.  */
static const yytype_uint8 yyr2[] =
{
       0,     2,     1,     1,     1,     3,     1,     2,     1,     4,
       3,     4,     3,     3,     2,     2,     1,     3,     1,     2,
       2,     2,     2,     4,     1,     1,     1,     1,     1,     1,
       1,     4,     1,     3,     3,     3,     1,     3,     3,     1,
       3,     3,     1,     3,     3,     3,     3,     1,     3,     3,
       1,     3,     1,     3,     1,     3,     1,     3,     1,     3,
       1,     5,     1,     3,     1,     1,     1,     1,     1,     1,
       1,     1,     1,     1,     1,     1,     3,     1,     2,     3,
       4,     5,     3,     5,     6,     4,     1,     2,     1,     2,
       1,     2,     1,     2,     1,     3,     1,     3,     1,     3,
       4,     7,     1,     1,     1,     1,     1,     1,     1,     1,
       1,     1,     1,     1,     1,     1,     1,     1,     1,     1,
       1,     5,     5,     4,     2,     2,     1,     1,     1,     2,
       2,     3,     1,     2,     1,     2,     1,     3,     1,     2,
       3,     4,     5,     5,     6,     2,     1,     3,     1,     3,
       1,     1,     1,     1,     1,     2,     1,     2,     3,     2,
       1,     3,     3,     4,     3,     4,     4,     1,     2,     3,
       2,     1,     1,     3,     3,     4,     3,     4,     4,     1,
       2,     2,     3,     1,     2,     1,     3,     1,     3,     2,
       2,     1,     1,     3,     1,     2,     1,     1,     2,     3,
       2,     3,     3,     4,     2,     3,     3,     4,     1,     3,
       4,     1,     3,     1,     1,     1,     1,     1,     1,     1,
       3,     4,     3,     1,     2,     3,     1,     2,     1,     1,
       2,     2,     1,     2,     5,     7,     5,     5,     7,     6,
       7,     7,     8,     7,     8,     8,     9,     3,     2,     2,
       2,     3,     1,     2,     1,     1,     2,     3,     3,     4,
       1,     1,     1,     0
};

/* YYDEFACT[STATE-NAME] -- Default rule to reduce with in state
   STATE-NUM when YYTABLE doesn't specify something else to do.  Zero
   means the default is an error.  */
static const yytype_uint16 yydefact[] =
{
     263,   261,   260,     0,   153,   152,     0,   102,   103,   105,
     106,   104,   107,   108,   109,   110,   111,   112,   113,   114,
     115,   150,   151,   116,   117,   126,   127,     0,     0,   179,
     255,     0,    92,    86,    88,   118,     0,   119,    90,     0,
       0,   156,     0,   262,   252,   254,   120,   160,     0,     0,
       0,    92,     0,   145,     0,   183,   181,   180,    78,     0,
      94,    98,     0,    93,    87,    89,     0,   125,   124,    91,
     159,     0,   226,     0,   256,     0,     0,     0,   154,     0,
     157,   253,     1,     0,     0,    82,     0,     0,   167,    96,
       0,     0,   146,   148,     0,   161,   184,   182,     0,    79,
       0,   258,     0,     0,   132,     0,   128,     0,   134,     0,
       0,     3,     6,     0,     0,     0,   223,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,    24,
      25,    26,    27,    28,    29,   232,   224,     8,     4,    18,
      30,     0,    32,    36,    39,    42,    47,    50,    52,    54,
      56,    58,    60,    62,    75,     0,   228,   229,   213,   219,
     214,     0,   215,   216,   217,   218,     2,    98,   227,   257,
     164,   191,     0,   185,   187,     0,   192,   162,    30,    77,
       0,     2,   155,   158,     0,   100,     0,     0,    80,   170,
       0,     0,   168,   172,   171,     0,   141,     0,     0,    95,
       0,   208,    99,   259,     0,    85,     0,   133,   123,   129,
       0,   130,     0,   136,   138,   135,     0,     0,     0,    22,
       0,    19,    20,     0,     0,     0,     0,     0,     0,     0,
       0,   248,   249,   250,     0,     0,   194,     0,     7,     0,
      14,    15,     0,     0,     0,    65,    66,    67,    68,    69,
      70,    71,    72,    73,    74,    64,     0,    21,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,   233,   225,
     231,   230,     0,     0,     0,   189,   196,   190,   197,   165,
       0,   166,     0,   163,     0,    81,    97,     0,   169,     0,
       0,   143,   147,   149,     0,   142,   211,     0,     0,    83,
     139,     0,   131,     0,   122,   121,     0,     0,   222,     0,
       0,     0,     0,     0,     0,   247,   251,     5,     0,   196,
     195,     0,    13,    10,     0,    16,     0,    12,    63,    33,
      34,    35,    37,    38,    40,    41,    45,    46,    43,    44,
      48,    49,    51,    53,    55,    57,    59,     0,    76,   220,
     204,   196,     0,     0,   200,     0,     0,   198,     0,     0,
     186,   188,   193,     0,   173,   176,     0,     0,   174,     0,
     144,     0,   209,    84,   137,   140,    23,   221,     0,     0,
       0,     0,     0,     0,     0,    31,    11,     0,     9,     0,
     205,   199,   201,   206,     0,   202,     0,   101,   177,   178,
     175,   210,   212,   234,   236,   237,     0,     0,     0,     0,
       0,     0,    17,    61,   207,   203,     0,     0,   239,     0,
       0,     0,     0,     0,     0,   235,   238,   240,   241,     0,
     243,     0,     0,     0,   242,   244,   245,     0,   246
};

/* YYDEFGOTO[NTERM-NUM].  */
static const yytype_int16 yydefgoto[] =
{
      -1,   137,   138,   139,   334,   140,   141,   142,   143,   144,
     145,   146,   147,   148,   149,   150,   151,   152,   153,   154,
     256,   155,   180,    30,   171,    59,    86,    60,    51,    33,
      34,    35,    36,   105,   106,   107,   212,   213,    37,    91,
      92,    38,    39,    79,    40,    41,    89,   192,    42,    57,
     362,   173,   174,   175,   237,   363,   288,   202,   307,   157,
     158,   159,   160,    75,   161,   162,   163,   164,   165,    43,
      44,    45,    46,   181,    48
};

/* YYPACT[STATE-NUM] -- Index in YYTABLE of the portion describing
   STATE-NUM.  */
#define YYPACT_NINF -247
static const yytype_int16 yypact[] =
{
    1254,  -247,  -247,   -25,  -247,  -247,  1644,  -247,  -247,  -247,
    -247,  -247,  -247,  -247,  -247,  -247,  -247,  -247,  -247,  -247,
    -247,  -247,  -247,  -247,  -247,  -247,  -247,    35,   433,   -30,
    -247,   259,  1617,  1644,  1644,  -247,    34,  -247,  1644,   433,
    1477,   166,   446,  1254,  -247,  -247,  -247,  -247,    20,    71,
     157,  1644,    71,   -44,    31,  -247,  -247,   -30,  -247,   -49,
    -247,  1450,  1644,  -247,  -247,  -247,  1671,    46,    51,  -247,
    -247,   541,  -247,   259,  -247,  1477,  1306,   825,  -247,   446,
     166,  -247,  -247,   -16,   433,  -247,   -26,   433,   166,  -247,
     351,   -57,  -247,    58,    71,  -247,  -247,  -247,   433,  -247,
     714,  -247,  1477,   229,  1671,  1128,  -247,   239,  1671,  1671,
    1671,  -247,  -247,  1071,  1133,  1133,  -247,  1149,    68,    96,
     106,   154,   697,   159,    71,    95,   102,   731,   887,  -247,
    -247,  -247,  -247,  -247,  -247,  -247,  -247,  -247,   195,   449,
     618,  1149,  -247,   199,   210,   205,   126,   370,   171,   121,
     148,   237,    -5,  -247,  -247,    54,  -247,  -247,  -247,  -247,
    -247,   619,  -247,  -247,  -247,  -247,   176,   189,  -247,  -247,
    -247,   437,   236,   222,  -247,    44,  -247,  -247,  -247,  -247,
     261,  -247,  -247,   166,   304,  -247,    59,   433,  -247,  -247,
     433,    89,   174,  -247,  -247,     5,  -247,  1149,     3,  -247,
     714,  -247,  -247,  -247,   433,  -247,    78,  -247,  -247,  -247,
    1149,  -247,    83,  -247,   251,  -247,  1390,  1417,   887,  -247,
    1149,  -247,  -247,   256,   697,  1149,  1149,  1149,   298,   776,
     281,  -247,  -247,  -247,    93,   100,   265,   292,  -247,    71,
    -247,  -247,   903,  1149,    71,  -247,  -247,  -247,  -247,  -247,
    -247,  -247,  -247,  -247,  -247,  -247,  1149,  -247,  1149,  1149,
    1149,  1149,  1149,  1149,  1149,  1149,  1149,  1149,  1149,  1149,
    1149,  1149,  1149,  1149,  1149,  1149,  1149,  1149,  -247,  -247,
    -247,  -247,   697,  1202,   915,  -247,   226,  -247,   198,  -247,
    1589,  -247,    71,  -247,    11,  -247,  -247,   316,   174,  1358,
     939,  -247,  -247,  -247,     7,  -247,  -247,     4,   131,  -247,
    -247,   384,  -247,  1149,  -247,  -247,   321,   697,  -247,   123,
     219,   272,   342,   793,   137,  -247,  -247,  -247,  1508,   208,
    -247,  1149,  -247,  -247,   277,  -247,   278,  -247,  -247,  -247,
    -247,  -247,   199,   199,   210,   210,   205,   205,   205,   205,
     126,   126,   370,   171,   121,   148,   237,    12,  -247,  -247,
    -247,   253,   343,   361,  -247,   349,  1202,   198,  1552,   993,
    -247,  -247,  -247,   373,  -247,  -247,   381,   283,  -247,   387,
    -247,   419,  -247,  -247,  -247,  -247,  -247,  -247,   697,   697,
     697,  1149,  1017,   140,   809,  -247,  -247,  1149,  -247,  1149,
    -247,  -247,  -247,  -247,   392,  -247,   398,  -247,  -247,  -247,
    -247,  -247,  -247,   416,  -247,  -247,   285,   697,   306,  1029,
    1041,   146,  -247,  -247,  -247,  -247,   697,   389,  -247,   697,
     697,   363,   697,   364,  1055,  -247,  -247,  -247,  -247,   697,
    -247,   697,   697,   397,  -247,  -247,  -247,   697,  -247
};

/* YYPGOTO[NTERM-NUM].  */
static const yytype_int16 yypgoto[] =
{
    -247,  -247,   302,  -247,  -247,   135,  -247,  -128,   116,    57,
     225,   215,   232,   233,   234,   238,   242,  -247,   -62,   -79,
    -247,   138,  -106,    79,   412,  -247,   -70,   421,    80,  -247,
     264,  -247,  -247,   391,   -71,   -39,  -247,   212,  -247,   427,
    -191,     1,   -18,  -247,    29,     9,  -164,   333,   -28,  -247,
     -74,  -247,   235,   227,   309,  -166,  -246,  -197,  -247,  -116,
    -247,  -247,   414,   467,  -247,  -247,  -247,  -247,  -247,  -247,
     488,  -247,   -27,     0,  -247
};

/* YYTABLE[YYPACT[STATE-NUM]].  What to do in state STATE-NUM.  If
   positive, shift that token.  If negative, reduce the rule which
   number is the opposite.  If zero, do what YYDEFACT says.
   If YYTABLE_NINF, syntax error.  */
#define YYTABLE_NINF -1
static const yytype_uint16 yytable[] =
{
      47,    56,   172,   306,   302,   287,   228,   285,     1,    67,
       1,   223,   275,   257,   186,   179,   238,   195,    21,    22,
      82,   201,    90,   296,    78,    98,   297,    53,    47,    97,
      55,    47,    87,   206,   209,   196,    68,     1,     1,    47,
     367,    99,    47,    47,    49,   281,    29,    94,   187,    83,
      47,    80,    93,   184,   185,   179,    90,    54,    96,    88,
      61,   182,     2,   193,   188,   207,    87,   108,    70,   215,
     330,   166,   191,    47,     1,    90,   176,   304,   381,    47,
      32,   373,   276,   367,    47,    87,   277,    47,   183,   236,
     194,   303,     1,    88,    93,   305,   382,   301,    47,   380,
     399,    95,   167,    47,   310,   108,   108,    47,   318,   108,
     108,   108,    88,   302,   291,   367,   189,     2,   292,    72,
      32,   201,   166,    32,   230,    66,    52,   167,   277,   108,
     339,   340,   341,   187,   308,   179,   214,   109,   265,   266,
      72,    32,   110,   286,   278,   209,   209,   197,   179,   295,
     156,    32,   187,    87,   168,    32,   224,   311,   190,    90,
       1,   166,    90,   335,   193,   225,   359,   277,   309,    87,
     327,    47,    87,   312,   277,   226,    90,   338,   365,   236,
      88,   168,    32,   326,   412,   231,    87,    47,     4,     5,
      47,   194,   232,   388,   379,    93,    88,   277,   358,    88,
     238,   387,   297,   395,    47,   187,   272,   385,   329,   267,
     268,   277,   178,    88,   277,   263,   264,   108,   108,   108,
     277,   383,   179,   227,   166,   376,    28,   394,   229,     1,
     419,    84,     1,    29,   273,    76,   434,    77,   179,   332,
     280,    32,     1,   299,   337,   300,   271,    85,   219,   221,
     222,   179,   178,   274,     2,   361,     1,     4,     5,   193,
       4,     5,     1,   406,   282,   234,   235,   368,   191,   369,
       4,     5,   413,   414,   415,   258,   178,   328,   100,   284,
     259,   260,   166,    47,     4,     5,   194,   261,   262,   389,
       4,     5,   372,   277,   404,   366,   290,   284,    28,   176,
     329,   428,   201,   204,    93,    29,   289,   179,    28,   112,
     435,    47,    54,   437,   438,    29,   440,   166,   422,   205,
     344,   345,   283,   444,   284,   445,   446,   210,    28,   211,
     104,   448,   178,   293,   328,    29,   284,   423,   286,   313,
     214,    29,   390,    78,   317,   178,   277,   396,    87,    58,
     398,   397,   277,   409,     1,   427,   235,   292,   235,   277,
     322,    47,   331,   319,   320,   321,    47,   324,   104,   104,
      80,   325,   104,   104,   104,    88,   429,   342,   343,     2,
     277,   336,     4,     5,   269,   270,   374,     1,   166,   166,
     166,   386,   104,   178,   178,   178,   178,   178,   178,   178,
     178,   178,   178,   178,   178,   178,   178,   178,   178,   178,
     178,   391,    31,   400,   357,     4,     5,   166,    50,   178,
     190,   402,     1,   111,   112,   113,   166,   114,   115,   166,
     166,   401,   166,   439,   441,   178,     1,   277,   277,   166,
       1,   166,   166,   407,    63,    64,    65,   166,   178,     1,
      69,   408,    73,    28,    74,    31,   239,   240,   241,   410,
      29,   393,   424,    63,     4,     5,   178,   447,     4,     5,
     425,   277,   210,    73,   103,   101,   426,     4,     5,   436,
     104,   104,   104,    73,   350,   351,   294,    73,   128,   169,
     346,   347,   348,   349,   129,   130,   131,   132,   133,   134,
     216,   217,    28,   352,   178,   353,   283,   354,   284,    29,
     200,   411,   355,    29,    73,    28,   203,   356,   242,   199,
     243,   198,   244,   384,   298,   371,   377,   316,   102,   416,
     418,    81,   421,     0,   178,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     1,   111,   112,   113,     0,   114,
     115,     0,     0,     0,     0,     0,     0,   431,   433,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     2,
     116,     3,   443,    73,     6,     7,     8,     9,    10,    11,
      12,    13,    14,    15,    16,    17,    18,    19,    20,    21,
      22,    23,    24,    25,    26,    27,     0,     0,   117,   118,
     119,     0,   120,   121,   122,   123,   124,   125,   126,   127,
     128,     0,     0,     0,     0,     0,   129,   130,   131,   132,
     133,   134,     1,   111,   112,   113,     0,   114,   115,     0,
       0,   135,    71,   136,     0,     0,   245,   246,   247,   248,
     249,   250,   251,   252,   253,   254,     0,     2,   116,     3,
       0,     0,     6,     7,     8,     9,    10,    11,    12,    13,
      14,    15,    16,    17,    18,    19,    20,    21,    22,    23,
      24,    25,    26,    27,     0,     0,   117,   118,   119,     0,
     120,   121,   122,   123,   124,   125,   126,   127,   128,     0,
       0,     0,     0,     0,   129,   130,   131,   132,   133,   134,
       1,   111,   112,   113,     0,   114,   115,   255,     0,   135,
      71,   279,     0,     0,     0,     0,     0,     1,   111,   112,
     113,     0,   114,   115,     0,     0,   116,     0,     0,     0,
       0,     0,     0,     0,     1,   111,   112,   113,     0,   114,
     115,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,   117,   118,   119,     0,   120,   121,
     122,   123,   124,   125,   126,   127,   128,     0,     0,     0,
       0,     0,   129,   130,   131,   132,   133,   134,     0,     1,
     111,   112,   113,   128,   114,   115,     0,   135,    71,   129,
     130,   131,   132,   133,   134,     0,     1,   111,   112,   113,
     128,   114,   115,     0,     0,   200,   129,   130,   131,   132,
     133,   134,     1,   111,   112,   113,     0,   114,   115,     0,
       0,   233,     0,     0,     0,     0,     0,     0,     1,   111,
     112,   113,     0,   114,   115,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,   128,     0,     0,     0,     0,
       0,   129,   130,   131,   132,   133,   134,     0,     0,     0,
       0,     0,   128,     0,     0,     0,   323,     0,   129,   130,
     131,   132,   133,   134,     0,     0,     0,     0,   128,     0,
       0,     0,     0,   392,   129,   130,   131,   132,   133,   134,
       1,   111,   112,   113,   128,   114,   115,   177,     0,   420,
     129,   130,   131,   132,   133,   134,     1,   111,   112,   113,
       0,   114,   115,     0,     0,     2,     0,     0,     1,   111,
     112,   113,     0,   114,   115,     0,    12,    13,    14,    15,
      16,    17,    18,    19,    20,    21,    22,    23,    24,    25,
      26,    27,     1,   111,   112,   113,     0,   114,   115,     0,
       0,     0,     0,     0,     0,     0,   128,     0,     0,     0,
       0,     0,   129,   130,   131,   132,   133,   134,     0,     0,
       0,     0,   128,   333,     0,     0,     0,     0,   129,   130,
     131,   132,   133,   134,   128,     0,     0,   364,     0,     0,
     129,   130,   131,   132,   133,   134,     1,   111,   112,   113,
       0,   114,   115,     0,     0,     0,     0,     0,   128,     0,
       0,   378,     0,     0,   129,   130,   131,   132,   133,   134,
       1,   111,   112,   113,     0,   114,   115,     0,     0,     0,
       0,     0,     1,   111,   112,   113,     0,   114,   115,     0,
       0,     0,     0,     0,     1,   111,   112,   113,     0,   114,
     115,     0,     0,     0,     0,     0,     0,     0,     1,   111,
     112,   113,   128,   114,   115,   405,     0,     0,   129,   130,
     131,   132,   133,   134,     1,   111,   112,   113,     0,   114,
     115,     0,     0,     0,     0,     0,   128,   417,     0,     0,
       0,     0,   129,   130,   131,   132,   133,   134,   128,   430,
       0,     0,     0,     0,   129,   130,   131,   132,   133,   134,
     128,   432,     0,     0,     0,     0,   129,   130,   131,   132,
     133,   134,     0,     0,   128,   442,     0,     0,     0,     0,
     129,   130,   131,   132,   133,   134,     1,   111,   112,   113,
     218,   114,   115,     0,     0,     0,   129,   130,   131,   132,
     133,   134,     1,   111,   112,   113,     2,   114,   115,     0,
       0,     0,     0,     0,     0,     0,     0,    12,    13,    14,
      15,    16,    17,    18,    19,    20,    21,    22,    23,    24,
      25,    26,    27,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,   220,     0,     0,     1,     0,     0,   129,   130,
     131,   132,   133,   134,     0,     0,     0,     0,   128,     0,
     208,     0,     0,     0,   129,   130,   131,   132,   133,   134,
       2,     0,     3,     4,     5,     0,     7,     8,     9,    10,
      11,    12,    13,    14,    15,    16,    17,    18,    19,    20,
      21,    22,    23,    24,    25,    26,    27,     1,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,   283,   360,   284,     0,     0,     0,     0,    29,     0,
       0,     0,     2,     0,     3,     4,     5,     6,     7,     8,
       9,    10,    11,    12,    13,    14,    15,    16,    17,    18,
      19,    20,    21,    22,    23,    24,    25,    26,    27,     1,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,    28,     0,     0,     0,     0,     0,     0,
      29,     0,     0,     0,     2,     0,     3,     0,     0,     0,
       7,     8,     9,    10,    11,    12,    13,    14,    15,    16,
      17,    18,    19,    20,    21,    22,    23,    24,    25,    26,
      27,     1,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,   170,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     2,     0,     3,     0,
       0,     0,     7,     8,     9,    10,    11,    12,    13,    14,
      15,    16,    17,    18,    19,    20,    21,    22,    23,    24,
      25,    26,    27,     0,     0,     0,     0,     0,     2,     0,
       0,     0,     0,     0,     0,     0,     0,     0,   375,    12,
      13,    14,    15,    16,    17,    18,    19,    20,    21,    22,
      23,    24,    25,    26,    27,     2,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,    12,    13,    14,    15,
      16,    17,    18,    19,    20,    21,    22,    23,    24,    25,
      26,    27,     0,     0,     0,     0,     0,     0,     2,     0,
       3,     0,   314,     6,     7,     8,     9,    10,    11,    12,
      13,    14,    15,    16,    17,    18,    19,    20,    21,    22,
      23,    24,    25,    26,    27,     2,     0,     3,     0,   315,
       6,     7,     8,     9,    10,    11,    12,    13,    14,    15,
      16,    17,    18,    19,    20,    21,    22,    23,    24,    25,
      26,    27,     0,     0,     0,     0,     2,     0,     3,   100,
       0,    71,     7,     8,     9,    10,    11,    12,    13,    14,
      15,    16,    17,    18,    19,    20,    21,    22,    23,    24,
      25,    26,    27,     0,     0,     0,     0,     0,    71,     0,
       0,     0,     0,     0,     0,     0,     0,   328,   360,   284,
       2,     0,     3,     0,    29,     0,     7,     8,     9,    10,
      11,    12,    13,    14,    15,    16,    17,    18,    19,    20,
      21,    22,    23,    24,    25,    26,    27,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     2,     0,     3,
       0,     0,   403,     7,     8,     9,    10,    11,    12,    13,
      14,    15,    16,    17,    18,    19,    20,    21,    22,    23,
      24,    25,    26,    27,   370,     2,     0,     3,     0,     0,
      62,     7,     8,     9,    10,    11,    12,    13,    14,    15,
      16,    17,    18,    19,    20,    21,    22,    23,    24,    25,
      26,    27,     2,     0,     3,     0,     0,     0,     7,     8,
       9,    10,    11,    12,    13,    14,    15,    16,    17,    18,
      19,    20,    21,    22,    23,    24,    25,    26,    27,     2,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
      12,    13,    14,    15,    16,    17,    18,    19,    20,    21,
      22,    23,    24,    25,    26,    27
};

static const yytype_int16 yycheck[] =
{
       0,    29,    76,   200,   195,   171,   122,   171,     3,    36,
       3,   117,    17,   141,    84,    77,     5,    74,    48,    49,
       0,   100,    50,   187,    42,    74,   190,    27,    28,    57,
      29,    31,    50,   103,   105,    92,    36,     3,     3,    39,
     286,    90,    42,    43,    69,   161,    76,    91,    74,    49,
      50,    42,    52,    69,    70,   117,    84,    28,    57,    50,
      31,    79,    28,    90,    90,   104,    84,    66,    39,   108,
     236,    71,    90,    73,     3,   103,    76,    74,    74,    79,
       0,    70,    87,   329,    84,   103,    74,    87,    79,   128,
      90,   197,     3,    84,    94,    92,    92,    92,    98,    92,
      88,    70,    73,   103,   210,   104,   105,   107,   224,   108,
     109,   110,   103,   304,    70,   361,    87,    28,    74,    40,
      40,   200,   122,    43,   124,    91,    91,    98,    74,   128,
     258,   259,   260,    74,   204,   197,   107,    91,    12,    13,
      61,    61,    91,   171,    90,   216,   217,    89,   210,    90,
      71,    71,    74,   171,    75,    75,    88,    74,    69,   187,
       3,   161,   190,   242,   191,    69,   282,    74,    90,   187,
      70,   171,   190,    90,    74,    69,   204,   256,   284,   218,
     171,   102,   102,    90,   381,    90,   204,   187,    31,    32,
     190,   191,    90,    70,   300,   195,   187,    74,   277,   190,
       5,   317,   366,   331,   204,    74,    85,   313,   236,    83,
      84,    74,    77,   204,    74,    10,    11,   216,   217,   218,
      74,    90,   284,    69,   224,   299,    69,    90,    69,     3,
      90,    74,     3,    76,    86,    69,    90,    71,   300,   239,
     161,   161,     3,    69,   244,    71,    75,    90,   113,   114,
     115,   313,   117,    16,    28,   283,     3,    31,    32,   286,
      31,    32,     3,   369,    88,   127,   128,    69,   286,    71,
      31,    32,   388,   389,   390,    76,   141,    69,    89,    71,
      81,    82,   282,   283,    31,    32,   286,    77,    78,    70,
      31,    32,   292,    74,   368,    69,    74,    71,    69,   299,
     328,   417,   381,    74,   304,    76,    70,   369,    69,     5,
     426,   311,   283,   429,   430,    76,   432,   317,   397,    90,
     263,   264,    69,   439,    71,   441,   442,    88,    69,    90,
      66,   447,   197,    72,    69,    76,    71,   399,   366,    88,
     311,    76,    70,   361,    88,   210,    74,    70,   366,    90,
      72,    74,    74,    70,     3,    70,   218,    74,   220,    74,
      62,   361,    70,   225,   226,   227,   366,   229,   104,   105,
     361,    90,   108,   109,   110,   366,    70,   261,   262,    28,
      74,   243,    31,    32,    14,    15,    70,     3,   388,   389,
     390,    70,   128,   258,   259,   260,   261,   262,   263,   264,
     265,   266,   267,   268,   269,   270,   271,   272,   273,   274,
     275,    69,     0,    70,   276,    31,    32,   417,     6,   284,
      69,    72,     3,     4,     5,     6,   426,     8,     9,   429,
     430,    70,   432,    70,    70,   300,     3,    74,    74,   439,
       3,   441,   442,    70,    32,    33,    34,   447,   313,     3,
      38,    70,    40,    69,    40,    43,     7,     8,     9,    72,
      76,   323,    70,    51,    31,    32,   331,    70,    31,    32,
      72,    74,    88,    61,    62,    61,    60,    31,    32,    90,
     216,   217,   218,    71,   269,   270,   184,    75,    69,    75,
     265,   266,   267,   268,    75,    76,    77,    78,    79,    80,
     109,   110,    69,   271,   369,   272,    69,   273,    71,    76,
      91,    92,   274,    76,   102,    69,   102,   275,    69,    98,
      71,    94,    73,   311,   191,   290,   299,   218,    61,   391,
     392,    43,   394,    -1,   399,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,     3,     4,     5,     6,    -1,     8,
       9,    -1,    -1,    -1,    -1,    -1,    -1,   419,   420,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    28,
      29,    30,   434,   161,    33,    34,    35,    36,    37,    38,
      39,    40,    41,    42,    43,    44,    45,    46,    47,    48,
      49,    50,    51,    52,    53,    54,    -1,    -1,    57,    58,
      59,    -1,    61,    62,    63,    64,    65,    66,    67,    68,
      69,    -1,    -1,    -1,    -1,    -1,    75,    76,    77,    78,
      79,    80,     3,     4,     5,     6,    -1,     8,     9,    -1,
      -1,    90,    91,    92,    -1,    -1,    18,    19,    20,    21,
      22,    23,    24,    25,    26,    27,    -1,    28,    29,    30,
      -1,    -1,    33,    34,    35,    36,    37,    38,    39,    40,
      41,    42,    43,    44,    45,    46,    47,    48,    49,    50,
      51,    52,    53,    54,    -1,    -1,    57,    58,    59,    -1,
      61,    62,    63,    64,    65,    66,    67,    68,    69,    -1,
      -1,    -1,    -1,    -1,    75,    76,    77,    78,    79,    80,
       3,     4,     5,     6,    -1,     8,     9,    89,    -1,    90,
      91,    92,    -1,    -1,    -1,    -1,    -1,     3,     4,     5,
       6,    -1,     8,     9,    -1,    -1,    29,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,     3,     4,     5,     6,    -1,     8,
       9,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    57,    58,    59,    -1,    61,    62,
      63,    64,    65,    66,    67,    68,    69,    -1,    -1,    -1,
      -1,    -1,    75,    76,    77,    78,    79,    80,    -1,     3,
       4,     5,     6,    69,     8,     9,    -1,    90,    91,    75,
      76,    77,    78,    79,    80,    -1,     3,     4,     5,     6,
      69,     8,     9,    -1,    -1,    91,    75,    76,    77,    78,
      79,    80,     3,     4,     5,     6,    -1,     8,     9,    -1,
      -1,    90,    -1,    -1,    -1,    -1,    -1,    -1,     3,     4,
       5,     6,    -1,     8,     9,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    69,    -1,    -1,    -1,    -1,
      -1,    75,    76,    77,    78,    79,    80,    -1,    -1,    -1,
      -1,    -1,    69,    -1,    -1,    -1,    90,    -1,    75,    76,
      77,    78,    79,    80,    -1,    -1,    -1,    -1,    69,    -1,
      -1,    -1,    -1,    90,    75,    76,    77,    78,    79,    80,
       3,     4,     5,     6,    69,     8,     9,    72,    -1,    90,
      75,    76,    77,    78,    79,    80,     3,     4,     5,     6,
      -1,     8,     9,    -1,    -1,    28,    -1,    -1,     3,     4,
       5,     6,    -1,     8,     9,    -1,    39,    40,    41,    42,
      43,    44,    45,    46,    47,    48,    49,    50,    51,    52,
      53,    54,     3,     4,     5,     6,    -1,     8,     9,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    69,    -1,    -1,    -1,
      -1,    -1,    75,    76,    77,    78,    79,    80,    -1,    -1,
      -1,    -1,    69,    70,    -1,    -1,    -1,    -1,    75,    76,
      77,    78,    79,    80,    69,    -1,    -1,    72,    -1,    -1,
      75,    76,    77,    78,    79,    80,     3,     4,     5,     6,
      -1,     8,     9,    -1,    -1,    -1,    -1,    -1,    69,    -1,
      -1,    72,    -1,    -1,    75,    76,    77,    78,    79,    80,
       3,     4,     5,     6,    -1,     8,     9,    -1,    -1,    -1,
      -1,    -1,     3,     4,     5,     6,    -1,     8,     9,    -1,
      -1,    -1,    -1,    -1,     3,     4,     5,     6,    -1,     8,
       9,    -1,    -1,    -1,    -1,    -1,    -1,    -1,     3,     4,
       5,     6,    69,     8,     9,    72,    -1,    -1,    75,    76,
      77,    78,    79,    80,     3,     4,     5,     6,    -1,     8,
       9,    -1,    -1,    -1,    -1,    -1,    69,    70,    -1,    -1,
      -1,    -1,    75,    76,    77,    78,    79,    80,    69,    70,
      -1,    -1,    -1,    -1,    75,    76,    77,    78,    79,    80,
      69,    70,    -1,    -1,    -1,    -1,    75,    76,    77,    78,
      79,    80,    -1,    -1,    69,    70,    -1,    -1,    -1,    -1,
      75,    76,    77,    78,    79,    80,     3,     4,     5,     6,
      69,     8,     9,    -1,    -1,    -1,    75,    76,    77,    78,
      79,    80,     3,     4,     5,     6,    28,     8,     9,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    39,    40,    41,
      42,    43,    44,    45,    46,    47,    48,    49,    50,    51,
      52,    53,    54,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    69,    -1,    -1,     3,    -1,    -1,    75,    76,
      77,    78,    79,    80,    -1,    -1,    -1,    -1,    69,    -1,
      92,    -1,    -1,    -1,    75,    76,    77,    78,    79,    80,
      28,    -1,    30,    31,    32,    -1,    34,    35,    36,    37,
      38,    39,    40,    41,    42,    43,    44,    45,    46,    47,
      48,    49,    50,    51,    52,    53,    54,     3,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    69,    70,    71,    -1,    -1,    -1,    -1,    76,    -1,
      -1,    -1,    28,    -1,    30,    31,    32,    33,    34,    35,
      36,    37,    38,    39,    40,    41,    42,    43,    44,    45,
      46,    47,    48,    49,    50,    51,    52,    53,    54,     3,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    69,    -1,    -1,    -1,    -1,    -1,    -1,
      76,    -1,    -1,    -1,    28,    -1,    30,    -1,    -1,    -1,
      34,    35,    36,    37,    38,    39,    40,    41,    42,    43,
      44,    45,    46,    47,    48,    49,    50,    51,    52,    53,
      54,     3,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    70,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    28,    -1,    30,    -1,
      -1,    -1,    34,    35,    36,    37,    38,    39,    40,    41,
      42,    43,    44,    45,    46,    47,    48,    49,    50,    51,
      52,    53,    54,    -1,    -1,    -1,    -1,    -1,    28,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    70,    39,
      40,    41,    42,    43,    44,    45,    46,    47,    48,    49,
      50,    51,    52,    53,    54,    28,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    39,    40,    41,    42,
      43,    44,    45,    46,    47,    48,    49,    50,    51,    52,
      53,    54,    -1,    -1,    -1,    -1,    -1,    -1,    28,    -1,
      30,    -1,    92,    33,    34,    35,    36,    37,    38,    39,
      40,    41,    42,    43,    44,    45,    46,    47,    48,    49,
      50,    51,    52,    53,    54,    28,    -1,    30,    -1,    92,
      33,    34,    35,    36,    37,    38,    39,    40,    41,    42,
      43,    44,    45,    46,    47,    48,    49,    50,    51,    52,
      53,    54,    -1,    -1,    -1,    -1,    28,    -1,    30,    89,
      -1,    91,    34,    35,    36,    37,    38,    39,    40,    41,
      42,    43,    44,    45,    46,    47,    48,    49,    50,    51,
      52,    53,    54,    -1,    -1,    -1,    -1,    -1,    91,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    69,    70,    71,
      28,    -1,    30,    -1,    76,    -1,    34,    35,    36,    37,
      38,    39,    40,    41,    42,    43,    44,    45,    46,    47,
      48,    49,    50,    51,    52,    53,    54,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    28,    -1,    30,
      -1,    -1,    70,    34,    35,    36,    37,    38,    39,    40,
      41,    42,    43,    44,    45,    46,    47,    48,    49,    50,
      51,    52,    53,    54,    55,    28,    -1,    30,    -1,    -1,
      33,    34,    35,    36,    37,    38,    39,    40,    41,    42,
      43,    44,    45,    46,    47,    48,    49,    50,    51,    52,
      53,    54,    28,    -1,    30,    -1,    -1,    -1,    34,    35,
      36,    37,    38,    39,    40,    41,    42,    43,    44,    45,
      46,    47,    48,    49,    50,    51,    52,    53,    54,    28,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      39,    40,    41,    42,    43,    44,    45,    46,    47,    48,
      49,    50,    51,    52,    53,    54
};

/* YYSTOS[STATE-NUM] -- The (internal number of the) accessing
   symbol of state STATE-NUM.  */
static const yytype_uint8 yystos[] =
{
       0,     3,    28,    30,    31,    32,    33,    34,    35,    36,
      37,    38,    39,    40,    41,    42,    43,    44,    45,    46,
      47,    48,    49,    50,    51,    52,    53,    54,    69,    76,
     116,   117,   121,   122,   123,   124,   125,   131,   134,   135,
     137,   138,   141,   162,   163,   164,   165,   166,   167,    69,
     117,   121,    91,   166,   137,   134,   141,   142,    90,   118,
     120,   137,    33,   117,   117,   117,    91,   165,   166,   117,
     137,    91,   116,   117,   155,   156,    69,    71,   135,   136,
     138,   163,     0,   166,    74,    90,   119,   135,   138,   139,
     141,   132,   133,   166,    91,    70,   134,   141,    74,    90,
      89,   155,   156,   117,   123,   126,   127,   128,   134,    91,
      91,     4,     5,     6,     8,     9,    29,    57,    58,    59,
      61,    62,    63,    64,    65,    66,    67,    68,    69,    75,
      76,    77,    78,    79,    80,    90,    92,    94,    95,    96,
      98,    99,   100,   101,   102,   103,   104,   105,   106,   107,
     108,   109,   110,   111,   112,   114,   116,   152,   153,   154,
     155,   157,   158,   159,   160,   161,   166,   137,   116,   155,
      70,   117,   143,   144,   145,   146,   166,    72,    98,   111,
     115,   166,   135,   138,    69,    70,   119,    74,    90,   137,
      69,   135,   140,   165,   166,    74,    92,    89,   132,   120,
      91,   112,   150,   155,    74,    90,   119,   128,    92,   127,
      88,    90,   129,   130,   137,   128,   126,   126,    69,    98,
      69,    98,    98,   115,    88,    69,    69,    69,   152,    69,
     166,    90,    90,    90,   114,   114,   128,   147,     5,     7,
       8,     9,    69,    71,    73,    18,    19,    20,    21,    22,
      23,    24,    25,    26,    27,    89,   113,   100,    76,    81,
      82,    77,    78,    10,    11,    12,    13,    83,    84,    14,
      15,    75,    85,    86,    16,    17,    87,    74,    90,    92,
     116,   152,    88,    69,    71,   139,   141,   148,   149,    70,
      74,    70,    74,    72,    95,    90,   139,   139,   140,    69,
      71,    92,   133,   115,    74,    92,   150,   151,   119,    90,
     115,    74,    90,    88,    92,    92,   147,    88,   152,   114,
     114,   114,    62,    90,   114,    90,    90,    70,    69,   141,
     148,    70,   166,    70,    97,   112,   114,   166,   112,   100,
     100,   100,   101,   101,   102,   102,   103,   103,   103,   103,
     104,   104,   105,   106,   107,   108,   109,   114,   112,   152,
      70,   141,   143,   148,    72,   115,    69,   149,    69,    71,
      55,   145,   166,    70,    70,    70,   143,   146,    72,   115,
      92,    74,    92,    90,   130,   115,    70,   152,    70,    70,
      70,    69,    90,   114,    90,   100,    70,    74,    72,    88,
      70,    70,    72,    70,   143,    72,   115,    70,    70,    70,
      72,    92,   150,   152,   152,   152,   114,    70,   114,    90,
      90,   114,   112,   111,    70,    72,    60,    70,   152,    70,
      70,   114,    70,   114,    90,   152,    90,   152,   152,    70,
     152,    70,    70,   114,   152,   152,   152,    70,   152
};

#define yyerrok		(yyerrstatus = 0)
#define yyclearin	(yychar = YYEMPTY)
#define YYEMPTY		(-2)
#define YYEOF		0

#define YYACCEPT	goto yyacceptlab
#define YYABORT		goto yyabortlab
#define YYERROR		goto yyerrorlab


/* Like YYERROR except do call yyerror.  This remains here temporarily
   to ease the transition to the new meaning of YYERROR, for GCC.
   Once GCC version 2 has supplanted version 1, this can go.  */

#define YYFAIL		goto yyerrlab

#define YYRECOVERING()  (!!yyerrstatus)

#define YYBACKUP(Token, Value)					\
do								\
  if (yychar == YYEMPTY && yylen == 1)				\
    {								\
      yychar = (Token);						\
      yylval = (Value);						\
      yytoken = YYTRANSLATE (yychar);				\
      YYPOPSTACK (1);						\
      goto yybackup;						\
    }								\
  else								\
    {								\
      yyerror (YY_("syntax error: cannot back up")); \
      YYERROR;							\
    }								\
while (YYID (0))


#define YYTERROR	1
#define YYERRCODE	256


/* YYLLOC_DEFAULT -- Set CURRENT to span from RHS[1] to RHS[N].
   If N is 0, then set CURRENT to the empty location which ends
   the previous symbol: RHS[0] (always defined).  */

#define YYRHSLOC(Rhs, K) ((Rhs)[K])
#ifndef YYLLOC_DEFAULT
# define YYLLOC_DEFAULT(Current, Rhs, N)				\
    do									\
      if (YYID (N))                                                    \
	{								\
	  (Current).first_line   = YYRHSLOC (Rhs, 1).first_line;	\
	  (Current).first_column = YYRHSLOC (Rhs, 1).first_column;	\
	  (Current).last_line    = YYRHSLOC (Rhs, N).last_line;		\
	  (Current).last_column  = YYRHSLOC (Rhs, N).last_column;	\
	}								\
      else								\
	{								\
	  (Current).first_line   = (Current).last_line   =		\
	    YYRHSLOC (Rhs, 0).last_line;				\
	  (Current).first_column = (Current).last_column =		\
	    YYRHSLOC (Rhs, 0).last_column;				\
	}								\
    while (YYID (0))
#endif


/* YY_LOCATION_PRINT -- Print the location on the stream.
   This macro was not mandated originally: define only if we know
   we won't break user code: when these are the locations we know.  */

#ifndef YY_LOCATION_PRINT
# if YYLTYPE_IS_TRIVIAL
#  define YY_LOCATION_PRINT(File, Loc)			\
     fprintf (File, "%d.%d-%d.%d",			\
	      (Loc).first_line, (Loc).first_column,	\
	      (Loc).last_line,  (Loc).last_column)
# else
#  define YY_LOCATION_PRINT(File, Loc) ((void) 0)
# endif
#endif


/* YYLEX -- calling `yylex' with the right arguments.  */

#ifdef YYLEX_PARAM
# define YYLEX yylex (YYLEX_PARAM)
#else
# define YYLEX yylex ()
#endif

/* Enable debugging if requested.  */
#if YYDEBUG

# ifndef YYFPRINTF
#  include <stdio.h> /* INFRINGES ON USER NAME SPACE */
#  define YYFPRINTF fprintf
# endif

# define YYDPRINTF(Args)			\
do {						\
  if (yydebug)					\
    YYFPRINTF Args;				\
} while (YYID (0))

# define YY_SYMBOL_PRINT(Title, Type, Value, Location)			  \
do {									  \
  if (yydebug)								  \
    {									  \
      YYFPRINTF (stderr, "%s ", Title);					  \
      yy_symbol_print (stderr,						  \
		  Type, Value); \
      YYFPRINTF (stderr, "\n");						  \
    }									  \
} while (YYID (0))


/*--------------------------------.
| Print this symbol on YYOUTPUT.  |
`--------------------------------*/

/*ARGSUSED*/
#if (defined __STDC__ || defined __C99__FUNC__ \
     || defined __cplusplus || defined _MSC_VER)
static void
yy_symbol_value_print (FILE *yyoutput, int yytype, YYSTYPE const * const yyvaluep)
#else
static void
yy_symbol_value_print (yyoutput, yytype, yyvaluep)
    FILE *yyoutput;
    int yytype;
    YYSTYPE const * const yyvaluep;
#endif
{
  if (!yyvaluep)
    return;
# ifdef YYPRINT
  if (yytype < YYNTOKENS)
    YYPRINT (yyoutput, yytoknum[yytype], *yyvaluep);
# else
  YYUSE (yyoutput);
# endif
  switch (yytype)
    {
      default:
	break;
    }
}


/*--------------------------------.
| Print this symbol on YYOUTPUT.  |
`--------------------------------*/

#if (defined __STDC__ || defined __C99__FUNC__ \
     || defined __cplusplus || defined _MSC_VER)
static void
yy_symbol_print (FILE *yyoutput, int yytype, YYSTYPE const * const yyvaluep)
#else
static void
yy_symbol_print (yyoutput, yytype, yyvaluep)
    FILE *yyoutput;
    int yytype;
    YYSTYPE const * const yyvaluep;
#endif
{
  if (yytype < YYNTOKENS)
    YYFPRINTF (yyoutput, "token %s (", yytname[yytype]);
  else
    YYFPRINTF (yyoutput, "nterm %s (", yytname[yytype]);

  yy_symbol_value_print (yyoutput, yytype, yyvaluep);
  YYFPRINTF (yyoutput, ")");
}

/*------------------------------------------------------------------.
| yy_stack_print -- Print the state stack from its BOTTOM up to its |
| TOP (included).                                                   |
`------------------------------------------------------------------*/

#if (defined __STDC__ || defined __C99__FUNC__ \
     || defined __cplusplus || defined _MSC_VER)
static void
yy_stack_print (yytype_int16 *bottom, yytype_int16 *top)
#else
static void
yy_stack_print (bottom, top)
    yytype_int16 *bottom;
    yytype_int16 *top;
#endif
{
  YYFPRINTF (stderr, "Stack now");
  for (; bottom <= top; ++bottom)
    YYFPRINTF (stderr, " %d", *bottom);
  YYFPRINTF (stderr, "\n");
}

# define YY_STACK_PRINT(Bottom, Top)				\
do {								\
  if (yydebug)							\
    yy_stack_print ((Bottom), (Top));				\
} while (YYID (0))


/*------------------------------------------------.
| Report that the YYRULE is going to be reduced.  |
`------------------------------------------------*/

#if (defined __STDC__ || defined __C99__FUNC__ \
     || defined __cplusplus || defined _MSC_VER)
static void
yy_reduce_print (YYSTYPE *yyvsp, int yyrule)
#else
static void
yy_reduce_print (yyvsp, yyrule)
    YYSTYPE *yyvsp;
    int yyrule;
#endif
{
  int yynrhs = yyr2[yyrule];
  int yyi;
  unsigned long int yylno = yyrline[yyrule];
  YYFPRINTF (stderr, "Reducing stack by rule %d (line %lu):\n",
	     yyrule - 1, yylno);
  /* The symbols being reduced.  */
  for (yyi = 0; yyi < yynrhs; yyi++)
    {
      fprintf (stderr, "   $%d = ", yyi + 1);
      yy_symbol_print (stderr, yyrhs[yyprhs[yyrule] + yyi],
		       &(yyvsp[(yyi + 1) - (yynrhs)])
		       		       );
      fprintf (stderr, "\n");
    }
}

# define YY_REDUCE_PRINT(Rule)		\
do {					\
  if (yydebug)				\
    yy_reduce_print (yyvsp, Rule); \
} while (YYID (0))

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
#ifndef	YYINITDEPTH
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
#if (defined __STDC__ || defined __C99__FUNC__ \
     || defined __cplusplus || defined _MSC_VER)
static YYSIZE_T
yystrlen (const char *yystr)
#else
static YYSIZE_T
yystrlen (yystr)
    const char *yystr;
#endif
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
#if (defined __STDC__ || defined __C99__FUNC__ \
     || defined __cplusplus || defined _MSC_VER)
static char *
yystpcpy (char *yydest, const char *yysrc)
#else
static char *
yystpcpy (yydest, yysrc)
    char *yydest;
    const char *yysrc;
#endif
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

/* Copy into YYRESULT an error message about the unexpected token
   YYCHAR while in state YYSTATE.  Return the number of bytes copied,
   including the terminating null byte.  If YYRESULT is null, do not
   copy anything; just return the number of bytes that would be
   copied.  As a special case, return 0 if an ordinary "syntax error"
   message will do.  Return YYSIZE_MAXIMUM if overflow occurs during
   size calculation.  */
static YYSIZE_T
yysyntax_error (char *yyresult, int yystate, int yychar)
{
  int yyn = yypact[yystate];

  if (! (YYPACT_NINF < yyn && yyn <= YYLAST))
    return 0;
  else
    {
      int yytype = YYTRANSLATE (yychar);
      YYSIZE_T yysize0 = yytnamerr (0, yytname[yytype]);
      YYSIZE_T yysize = yysize0;
      YYSIZE_T yysize1;
      int yysize_overflow = 0;
      enum { YYERROR_VERBOSE_ARGS_MAXIMUM = 5 };
      char const *yyarg[YYERROR_VERBOSE_ARGS_MAXIMUM];
      int yyx;

# if 0
      /* This is so xgettext sees the translatable formats that are
	 constructed on the fly.  */
      YY_("syntax error, unexpected %s");
      YY_("syntax error, unexpected %s, expecting %s");
      YY_("syntax error, unexpected %s, expecting %s or %s");
      YY_("syntax error, unexpected %s, expecting %s or %s or %s");
      YY_("syntax error, unexpected %s, expecting %s or %s or %s or %s");
# endif
      char *yyfmt;
      char const *yyf;
      static char const yyunexpected[] = "syntax error, unexpected %s";
      static char const yyexpecting[] = ", expecting %s";
      static char const yyor[] = " or %s";
      char yyformat[sizeof yyunexpected
		    + sizeof yyexpecting - 1
		    + ((YYERROR_VERBOSE_ARGS_MAXIMUM - 2)
		       * (sizeof yyor - 1))];
      char const *yyprefix = yyexpecting;

      /* Start YYX at -YYN if negative to avoid negative indexes in
	 YYCHECK.  */
      int yyxbegin = yyn < 0 ? -yyn : 0;

      /* Stay within bounds of both yycheck and yytname.  */
      int yychecklim = YYLAST - yyn + 1;
      int yyxend = yychecklim < YYNTOKENS ? yychecklim : YYNTOKENS;
      int yycount = 1;

      yyarg[0] = yytname[yytype];
      yyfmt = yystpcpy (yyformat, yyunexpected);

      for (yyx = yyxbegin; yyx < yyxend; ++yyx)
	if (yycheck[yyx + yyn] == yyx && yyx != YYTERROR)
	  {
	    if (yycount == YYERROR_VERBOSE_ARGS_MAXIMUM)
	      {
		yycount = 1;
		yysize = yysize0;
		yyformat[sizeof yyunexpected - 1] = '\0';
		break;
	      }
	    yyarg[yycount++] = yytname[yyx];
	    yysize1 = yysize + yytnamerr (0, yytname[yyx]);
	    yysize_overflow |= (yysize1 < yysize);
	    yysize = yysize1;
	    yyfmt = yystpcpy (yyfmt, yyprefix);
	    yyprefix = yyor;
	  }

      yyf = YY_(yyformat);
      yysize1 = yysize + yystrlen (yyf);
      yysize_overflow |= (yysize1 < yysize);
      yysize = yysize1;

      if (yysize_overflow)
	return YYSIZE_MAXIMUM;

      if (yyresult)
	{
	  /* Avoid sprintf, as that infringes on the user's name space.
	     Don't have undefined behavior even if the translation
	     produced a string with the wrong number of "%s"s.  */
	  char *yyp = yyresult;
	  int yyi = 0;
	  while ((*yyp = *yyf) != '\0')
	    {
	      if (*yyp == '%' && yyf[1] == 's' && yyi < yycount)
		{
		  yyp += yytnamerr (yyp, yyarg[yyi++]);
		  yyf += 2;
		}
	      else
		{
		  yyp++;
		  yyf++;
		}
	    }
	}
      return yysize;
    }
}
#endif /* YYERROR_VERBOSE */


/*-----------------------------------------------.
| Release the memory associated to this symbol.  |
`-----------------------------------------------*/

/*ARGSUSED*/
#if (defined __STDC__ || defined __C99__FUNC__ \
     || defined __cplusplus || defined _MSC_VER)
static void
yydestruct (const char *yymsg, int yytype, YYSTYPE *yyvaluep)
#else
static void
yydestruct (yymsg, yytype, yyvaluep)
    const char *yymsg;
    int yytype;
    YYSTYPE *yyvaluep;
#endif
{
  YYUSE (yyvaluep);

  if (!yymsg)
    yymsg = "Deleting";
  YY_SYMBOL_PRINT (yymsg, yytype, yyvaluep, yylocationp);

  switch (yytype)
    {

      default:
	break;
    }
}


/* Prevent warnings from -Wmissing-prototypes.  */

#ifdef YYPARSE_PARAM
#if defined __STDC__ || defined __cplusplus
int yyparse (void *YYPARSE_PARAM);
#else
int yyparse ();
#endif
#else /* ! YYPARSE_PARAM */
#if defined __STDC__ || defined __cplusplus
int yyparse (void);
#else
int yyparse ();
#endif
#endif /* ! YYPARSE_PARAM */



/* The look-ahead symbol.  */
int yychar;

/* The semantic value of the look-ahead symbol.  */
YYSTYPE yylval;

/* Number of syntax errors so far.  */
int yynerrs;



/*----------.
| yyparse.  |
`----------*/

#ifdef YYPARSE_PARAM
#if (defined __STDC__ || defined __C99__FUNC__ \
     || defined __cplusplus || defined _MSC_VER)
int
yyparse (void *YYPARSE_PARAM)
#else
int
yyparse (YYPARSE_PARAM)
    void *YYPARSE_PARAM;
#endif
#else /* ! YYPARSE_PARAM */
#if (defined __STDC__ || defined __C99__FUNC__ \
     || defined __cplusplus || defined _MSC_VER)
int
yyparse (void)
#else
int
yyparse ()

#endif
#endif
{
  
  int yystate;
  int yyn;
  int yyresult;
  /* Number of tokens to shift before error messages enabled.  */
  int yyerrstatus;
  /* Look-ahead token as an internal (translated) token number.  */
  int yytoken = 0;
#if YYERROR_VERBOSE
  /* Buffer for error messages, and its allocated size.  */
  char yymsgbuf[128];
  char *yymsg = yymsgbuf;
  YYSIZE_T yymsg_alloc = sizeof yymsgbuf;
#endif

  /* Three stacks and their tools:
     `yyss': related to states,
     `yyvs': related to semantic values,
     `yyls': related to locations.

     Refer to the stacks thru separate pointers, to allow yyoverflow
     to reallocate them elsewhere.  */

  /* The state stack.  */
  yytype_int16 yyssa[YYINITDEPTH];
  yytype_int16 *yyss = yyssa;
  yytype_int16 *yyssp;

  /* The semantic value stack.  */
  YYSTYPE yyvsa[YYINITDEPTH];
  YYSTYPE *yyvs = yyvsa;
  YYSTYPE *yyvsp;



#define YYPOPSTACK(N)   (yyvsp -= (N), yyssp -= (N))

  YYSIZE_T yystacksize = YYINITDEPTH;

  /* The variables used to return semantic value and location from the
     action routines.  */
  YYSTYPE yyval;


  /* The number of symbols on the RHS of the reduced rule.
     Keep to zero when no symbol should be popped.  */
  int yylen = 0;

  YYDPRINTF ((stderr, "Starting parse\n"));

  yystate = 0;
  yyerrstatus = 0;
  yynerrs = 0;
  yychar = YYEMPTY;		/* Cause a token to be read.  */

  /* Initialize stack pointers.
     Waste one element of value and location stack
     so that they stay on the same level as the state stack.
     The wasted elements are never initialized.  */

  yyssp = yyss;
  yyvsp = yyvs;

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
	YYSTACK_RELOCATE (yyss);
	YYSTACK_RELOCATE (yyvs);

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

  goto yybackup;

/*-----------.
| yybackup.  |
`-----------*/
yybackup:

  /* Do appropriate processing given the current state.  Read a
     look-ahead token if we need one and don't already have one.  */

  /* First try to decide what to do without reference to look-ahead token.  */
  yyn = yypact[yystate];
  if (yyn == YYPACT_NINF)
    goto yydefault;

  /* Not known => get a look-ahead token if don't already have one.  */

  /* YYCHAR is either YYEMPTY or YYEOF or a valid look-ahead symbol.  */
  if (yychar == YYEMPTY)
    {
      YYDPRINTF ((stderr, "Reading a token: "));
      yychar = YYLEX;
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
      if (yyn == 0 || yyn == YYTABLE_NINF)
	goto yyerrlab;
      yyn = -yyn;
      goto yyreduce;
    }

  if (yyn == YYFINAL)
    YYACCEPT;

  /* Count tokens shifted since error; after three, turn off error
     status.  */
  if (yyerrstatus)
    yyerrstatus--;

  /* Shift the look-ahead token.  */
  YY_SYMBOL_PRINT ("Shifting", yytoken, &yylval, &yylloc);

  /* Discard the shifted token unless it is eof.  */
  if (yychar != YYEOF)
    yychar = YYEMPTY;

  yystate = yyn;
  *++yyvsp = yylval;

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
     `$$ = $1'.

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
#line 65 "c_gram.y"
    { (yyval.xml) = xml_text("id", 0, (yyvsp[(1) - (1)].str)); }
    break;

  case 3:
#line 66 "c_gram.y"
    { 

    if (ctoxml_cterm[0]=='\'') {
      char txt[20];
      c_unescape(ctoxml_cterm);

      sprintf(txt,"%d",(int)(ctoxml_cterm[0]));
      (yyval.xml) = xml_text("c",0,strdup(txt));
      free(ctoxml_cterm);

    } else {
	  char *attrib=0;
	  char *value=0;
	  if (ctoxml_cterm[0]!='0') {
	    char lc=ctoxml_cterm[strlen(ctoxml_cterm)-1];
	    switch(lc) {
	    case 'f': attrib="length"; value="f"; break;
	    case 'F': attrib="length"; value="F"; break;
	    case 'l': attrib="length"; value="l"; break;
	    case 'L': attrib="length"; value="L"; break;
	    case 'u': attrib="length"; value="u"; break;
	    case 'U': attrib="length"; value="U"; break;
	    }
	    if (lc=='f' || lc=='F' || lc=='l' || lc=='L' || lc=='u' || lc=='U') {
	      ctoxml_cterm[strlen(ctoxml_cterm)-1]=0;
	    }
	  }
	  if (attrib)
	    (yyval.xml) = xml_text("c",attrib,strdup(value),0,ctoxml_cterm);
	  else
	    (yyval.xml) = xml_text("c",0,ctoxml_cterm);
    }
    
}
    break;

  case 4:
#line 100 "c_gram.y"
    { (yyval.xml) = (yyvsp[(1) - (1)].xml); }
    break;

  case 5:
#line 101 "c_gram.y"
    { (yyval.xml) = xml("p",NULL,(yyvsp[(2) - (3)].xml),NULL); }
    break;

  case 6:
#line 105 "c_gram.y"
    { c_unescape(ctoxml_cterm); ctoxml_cterm[strlen(ctoxml_cterm)-1]=0; (yyval.xml) = xml_text("s", NULL, strdup(ctoxml_cterm+1)); free(ctoxml_cterm);}
    break;

  case 7:
#line 106 "c_gram.y"
    { c_unescape(ctoxml_cterm); ctoxml_cterm[strlen(ctoxml_cterm)-1]=0; (yyval.xml) = xml_link((yyvsp[(1) - (2)].xml), xml_text("s", NULL, strdup(ctoxml_cterm+1))); free(ctoxml_cterm);}
    break;

  case 8:
#line 110 "c_gram.y"
    { (yyval.xml) = (yyvsp[(1) - (1)].xml); }
    break;

  case 9:
#line 111 "c_gram.y"
    { (yyval.xml) = xml("ix",NULL,(yyvsp[(1) - (4)].xml),(yyvsp[(3) - (4)].xml),NULL); }
    break;

  case 10:
#line 112 "c_gram.y"
    { (yyval.xml) = xml("call",NULL,(yyvsp[(1) - (3)].xml),NULL); }
    break;

  case 11:
#line 113 "c_gram.y"
    { (yyval.xml) = xml("call",NULL,(yyvsp[(1) - (4)].xml),(yyvsp[(3) - (4)].xml),NULL); }
    break;

  case 12:
#line 114 "c_gram.y"
    { (yyval.xml) = xml("mb",NULL,(yyvsp[(1) - (3)].xml),xml_text("id",NULL,(yyvsp[(3) - (3)].str)),NULL); }
    break;

  case 13:
#line 115 "c_gram.y"
    { (yyval.xml) = xml("ptr",NULL,(yyvsp[(1) - (3)].xml), xml_text("id",NULL,(yyvsp[(3) - (3)].str)),NULL); }
    break;

  case 14:
#line 116 "c_gram.y"
    { (yyval.xml) = xml("op","op",strdup("++"), "post",strdup("1"),NULL, (yyvsp[(1) - (2)].xml),NULL); }
    break;

  case 15:
#line 117 "c_gram.y"
    { (yyval.xml) = xml("op","op",strdup("--"), "post",strdup("1"),NULL, (yyvsp[(1) - (2)].xml),NULL); }
    break;

  case 16:
#line 121 "c_gram.y"
    { (yyval.xml) = (yyvsp[(1) - (1)].xml); }
    break;

  case 17:
#line 122 "c_gram.y"
    { (yyval.xml) = xml_link((yyvsp[(1) - (3)].xml), (yyvsp[(3) - (3)].xml)); }
    break;

  case 18:
#line 126 "c_gram.y"
    { (yyval.xml) = (yyvsp[(1) - (1)].xml); }
    break;

  case 19:
#line 127 "c_gram.y"
    { (yyval.xml) = xml("op","op",strdup("++"),"pre",strdup("1"), NULL, (yyvsp[(2) - (2)].xml),NULL); }
    break;

  case 20:
#line 128 "c_gram.y"
    { (yyval.xml) = xml("op","op",strdup("--"),"pre",strdup("1"), NULL, (yyvsp[(2) - (2)].xml),NULL); }
    break;

  case 21:
#line 129 "c_gram.y"
    { (yyval.xml) = xml("op", "op", strdup((yyvsp[(1) - (2)].str)), NULL, (yyvsp[(2) - (2)].xml),NULL); }
    break;

  case 22:
#line 130 "c_gram.y"
    { (yyval.xml) = xml("op", "op", strdup("sizeof"), "type",strdup("e"), NULL,(yyvsp[(2) - (2)].xml),NULL); }
    break;

  case 23:
#line 131 "c_gram.y"
    { (yyval.xml) = xml("op", "op", strdup("sizeof"), "type",strdup("t"), NULL,(yyvsp[(3) - (4)].xml),NULL); }
    break;

  case 24:
#line 135 "c_gram.y"
    { (yyval.str) = "&"; }
    break;

  case 25:
#line 136 "c_gram.y"
    { (yyval.str) = "*"; }
    break;

  case 26:
#line 137 "c_gram.y"
    { (yyval.str) = "+"; }
    break;

  case 27:
#line 138 "c_gram.y"
    { (yyval.str) = "-"; }
    break;

  case 28:
#line 139 "c_gram.y"
    { (yyval.str) = "~"; }
    break;

  case 29:
#line 140 "c_gram.y"
    { (yyval.str) = "!"; }
    break;

  case 30:
#line 144 "c_gram.y"
    { (yyval.xml) = (yyvsp[(1) - (1)].xml); }
    break;

  case 31:
#line 145 "c_gram.y"
    { (yyval.xml) = xml("cast", NULL, (yyvsp[(2) - (4)].xml), (yyvsp[(4) - (4)].xml),NULL); }
    break;

  case 32:
#line 149 "c_gram.y"
    { (yyval.xml) = (yyvsp[(1) - (1)].xml); }
    break;

  case 33:
#line 150 "c_gram.y"
    { (yyval.xml) = xml("op","op",strdup("*"), NULL,(yyvsp[(1) - (3)].xml), (yyvsp[(3) - (3)].xml),NULL); }
    break;

  case 34:
#line 151 "c_gram.y"
    { (yyval.xml) = xml("op","op",strdup("/"), NULL,(yyvsp[(1) - (3)].xml), (yyvsp[(3) - (3)].xml),NULL); }
    break;

  case 35:
#line 152 "c_gram.y"
    { (yyval.xml) = xml("op","op",strdup("%"), NULL,(yyvsp[(1) - (3)].xml), (yyvsp[(3) - (3)].xml),NULL); }
    break;

  case 36:
#line 156 "c_gram.y"
    { (yyval.xml) = (yyvsp[(1) - (1)].xml); }
    break;

  case 37:
#line 157 "c_gram.y"
    { (yyval.xml) = xml("op","op",strdup("+"), NULL,(yyvsp[(1) - (3)].xml), (yyvsp[(3) - (3)].xml),NULL); }
    break;

  case 38:
#line 158 "c_gram.y"
    { (yyval.xml) = xml("op","op",strdup("-"), NULL,(yyvsp[(1) - (3)].xml), (yyvsp[(3) - (3)].xml),NULL); }
    break;

  case 39:
#line 162 "c_gram.y"
    { (yyval.xml) = (yyvsp[(1) - (1)].xml); }
    break;

  case 40:
#line 163 "c_gram.y"
    { (yyval.xml) = xml("op","op",strdup("<<"), NULL,(yyvsp[(1) - (3)].xml), (yyvsp[(3) - (3)].xml),NULL); }
    break;

  case 41:
#line 164 "c_gram.y"
    { (yyval.xml) = xml("op","op",strdup(">>"), NULL,(yyvsp[(1) - (3)].xml), (yyvsp[(3) - (3)].xml),NULL); }
    break;

  case 42:
#line 168 "c_gram.y"
    { (yyval.xml) = (yyvsp[(1) - (1)].xml); }
    break;

  case 43:
#line 169 "c_gram.y"
    { (yyval.xml) = xml("op","op",strdup("<"), NULL,(yyvsp[(1) - (3)].xml), (yyvsp[(3) - (3)].xml),NULL); }
    break;

  case 44:
#line 170 "c_gram.y"
    { (yyval.xml) = xml("op","op",strdup(">"), NULL,(yyvsp[(1) - (3)].xml), (yyvsp[(3) - (3)].xml),NULL); }
    break;

  case 45:
#line 171 "c_gram.y"
    { (yyval.xml) = xml("op","op",strdup("<="), NULL,(yyvsp[(1) - (3)].xml), (yyvsp[(3) - (3)].xml),NULL); }
    break;

  case 46:
#line 172 "c_gram.y"
    { (yyval.xml) = xml("op","op",strdup(">="), NULL,(yyvsp[(1) - (3)].xml), (yyvsp[(3) - (3)].xml),NULL); }
    break;

  case 47:
#line 176 "c_gram.y"
    { (yyval.xml) = (yyvsp[(1) - (1)].xml); }
    break;

  case 48:
#line 177 "c_gram.y"
    { (yyval.xml) = xml("op","op",strdup("=="), NULL,(yyvsp[(1) - (3)].xml), (yyvsp[(3) - (3)].xml),NULL); }
    break;

  case 49:
#line 178 "c_gram.y"
    { (yyval.xml) = xml("op","op",strdup("!="), NULL,(yyvsp[(1) - (3)].xml), (yyvsp[(3) - (3)].xml),NULL); }
    break;

  case 50:
#line 182 "c_gram.y"
    { (yyval.xml) = (yyvsp[(1) - (1)].xml); }
    break;

  case 51:
#line 183 "c_gram.y"
    { (yyval.xml) = xml("op","op",strdup("&"), NULL,(yyvsp[(1) - (3)].xml), (yyvsp[(3) - (3)].xml),NULL); }
    break;

  case 52:
#line 187 "c_gram.y"
    { (yyval.xml) = (yyvsp[(1) - (1)].xml); }
    break;

  case 53:
#line 188 "c_gram.y"
    { (yyval.xml) = xml("op","op",strdup("^"), NULL,(yyvsp[(1) - (3)].xml), (yyvsp[(3) - (3)].xml),NULL); }
    break;

  case 54:
#line 192 "c_gram.y"
    { (yyval.xml) = (yyvsp[(1) - (1)].xml); }
    break;

  case 55:
#line 193 "c_gram.y"
    { (yyval.xml) = xml("op","op",strdup("|"), NULL,(yyvsp[(1) - (3)].xml), (yyvsp[(3) - (3)].xml),NULL); }
    break;

  case 56:
#line 197 "c_gram.y"
    { (yyval.xml) = (yyvsp[(1) - (1)].xml); }
    break;

  case 57:
#line 198 "c_gram.y"
    { (yyval.xml) = xml("op","op",strdup("&&"), NULL,(yyvsp[(1) - (3)].xml), (yyvsp[(3) - (3)].xml),NULL); }
    break;

  case 58:
#line 202 "c_gram.y"
    { (yyval.xml) = (yyvsp[(1) - (1)].xml); }
    break;

  case 59:
#line 203 "c_gram.y"
    { (yyval.xml) = xml("op","op",strdup("||"), NULL,(yyvsp[(1) - (3)].xml), (yyvsp[(3) - (3)].xml),NULL); }
    break;

  case 60:
#line 207 "c_gram.y"
    { (yyval.xml) = (yyvsp[(1) - (1)].xml); }
    break;

  case 61:
#line 208 "c_gram.y"
    { (yyval.xml) = xml("op","op",strdup("?:"), NULL,(yyvsp[(1) - (5)].xml), (yyvsp[(3) - (5)].xml),(yyvsp[(5) - (5)].xml),NULL); }
    break;

  case 62:
#line 212 "c_gram.y"
    { (yyval.xml) = (yyvsp[(1) - (1)].xml); }
    break;

  case 63:
#line 213 "c_gram.y"
    { (yyval.xml) = xml("op","op",strdup((yyvsp[(2) - (3)].str)), NULL,(yyvsp[(1) - (3)].xml), (yyvsp[(3) - (3)].xml),NULL); }
    break;

  case 64:
#line 217 "c_gram.y"
    { (yyval.str) = "="; }
    break;

  case 65:
#line 218 "c_gram.y"
    { (yyval.str) = "*="; }
    break;

  case 66:
#line 219 "c_gram.y"
    { (yyval.str) = "/="; }
    break;

  case 67:
#line 220 "c_gram.y"
    { (yyval.str) = "%="; }
    break;

  case 68:
#line 221 "c_gram.y"
    { (yyval.str) = "+="; }
    break;

  case 69:
#line 222 "c_gram.y"
    { (yyval.str) = "-="; }
    break;

  case 70:
#line 223 "c_gram.y"
    { (yyval.str) = "<<="; }
    break;

  case 71:
#line 224 "c_gram.y"
    { (yyval.str) = ">>="; }
    break;

  case 72:
#line 225 "c_gram.y"
    { (yyval.str) = "&="; }
    break;

  case 73:
#line 226 "c_gram.y"
    { (yyval.str) = "^="; }
    break;

  case 74:
#line 227 "c_gram.y"
    { (yyval.str) = "|="; }
    break;

  case 75:
#line 231 "c_gram.y"
    { (yyval.xml) = (yyvsp[(1) - (1)].xml); }
    break;

  case 76:
#line 232 "c_gram.y"
    { (yyval.xml) = xml("op","op",strdup(","), NULL,(yyvsp[(1) - (3)].xml), (yyvsp[(3) - (3)].xml),NULL); }
    break;

  case 77:
#line 236 "c_gram.y"
    { (yyval.xml) = (yyvsp[(1) - (1)].xml); }
    break;

  case 78:
#line 240 "c_gram.y"
    { (yyval.xml) = xml("d",NULL,(yyvsp[(1) - (2)].xml), NULL); }
    break;

  case 79:
#line 241 "c_gram.y"
    { (yyval.xml) = xml("d",NULL,(yyvsp[(1) - (3)].xml),(yyvsp[(2) - (3)].xml),NULL); }
    break;

  case 80:
#line 242 "c_gram.y"
    { (yyval.xml) = xml("d",NULL,xml("typedef",NULL,NULL),(yyvsp[(2) - (4)].xml),(yyvsp[(3) - (4)].xml),NULL); ctoxml_typedef((yyval.xml)); }
    break;

  case 81:
#line 243 "c_gram.y"
    { (yyval.xml) = xml("d",NULL,xml("typedef",NULL,NULL),(yyvsp[(2) - (5)].xml),(yyvsp[(4) - (5)].xml),NULL); ctoxml_typedef((yyval.xml)); }
    break;

  case 82:
#line 244 "c_gram.y"
    { (yyval.xml) = xml("d",NULL,xml("typedef",NULL,NULL),(yyvsp[(2) - (3)].xml),NULL); ctoxml_typedef((yyval.xml)); }
    break;

  case 83:
#line 245 "c_gram.y"
    { (yyval.xml) = xml("d",NULL,xml("typedef",NULL,NULL),(yyvsp[(3) - (5)].xml),(yyvsp[(4) - (5)].xml),NULL); ctoxml_typedef((yyval.xml)); }
    break;

  case 84:
#line 246 "c_gram.y"
    { (yyval.xml) = xml("d",NULL,xml("typedef",NULL,NULL),(yyvsp[(3) - (6)].xml),(yyvsp[(5) - (6)].xml),NULL); ctoxml_typedef((yyval.xml)); }
    break;

  case 85:
#line 247 "c_gram.y"
    { (yyval.xml) = xml("d",NULL,xml("typedef",NULL,NULL),(yyvsp[(3) - (4)].xml),NULL); ctoxml_typedef((yyval.xml)); }
    break;

  case 86:
#line 251 "c_gram.y"
    { (yyval.xml) = (yyvsp[(1) - (1)].xml); }
    break;

  case 87:
#line 252 "c_gram.y"
    { (yyval.xml) = xml_link((yyvsp[(1) - (2)].xml),(yyvsp[(2) - (2)].xml)); }
    break;

  case 88:
#line 253 "c_gram.y"
    { (yyval.xml) = (yyvsp[(1) - (1)].xml); }
    break;

  case 89:
#line 254 "c_gram.y"
    { (yyval.xml) = xml_link((yyvsp[(1) - (2)].xml),(yyvsp[(2) - (2)].xml)); }
    break;

  case 90:
#line 255 "c_gram.y"
    { (yyval.xml) = (yyvsp[(1) - (1)].xml); }
    break;

  case 91:
#line 256 "c_gram.y"
    { (yyval.xml) = xml_link((yyvsp[(1) - (2)].xml),(yyvsp[(2) - (2)].xml)); }
    break;

  case 92:
#line 257 "c_gram.y"
    { (yyval.xml) = (yyvsp[(1) - (1)].xml); }
    break;

  case 93:
#line 258 "c_gram.y"
    { (yyval.xml) = xml_link((yyvsp[(1) - (2)].xml),(yyvsp[(2) - (2)].xml)); }
    break;

  case 94:
#line 262 "c_gram.y"
    { (yyval.xml) = (yyvsp[(1) - (1)].xml); }
    break;

  case 95:
#line 263 "c_gram.y"
    { (yyval.xml) = xml_link((yyvsp[(1) - (3)].xml), (yyvsp[(3) - (3)].xml)); }
    break;

  case 96:
#line 267 "c_gram.y"
    { (yyval.xml) = (yyvsp[(1) - (1)].xml); }
    break;

  case 97:
#line 268 "c_gram.y"
    { (yyval.xml) = xml_link((yyvsp[(1) - (3)].xml), (yyvsp[(3) - (3)].xml)); }
    break;

  case 98:
#line 272 "c_gram.y"
    { (yyval.xml) = (yyvsp[(1) - (1)].xml); }
    break;

  case 99:
#line 273 "c_gram.y"
    { (yyval.xml) = xml("init",NULL,(yyvsp[(1) - (3)].xml),(yyvsp[(3) - (3)].xml),NULL); }
    break;

  case 100:
#line 277 "c_gram.y"
    { (yyval.xml) = xml_text("declspec","type",(yyvsp[(3) - (4)].str),NULL,NULL); }
    break;

  case 101:
#line 278 "c_gram.y"
    { (yyval.xml) = xml("declspec","type",(yyvsp[(3) - (7)].str),NULL,(yyvsp[(5) - (7)].xml),NULL); }
    break;

  case 102:
#line 282 "c_gram.y"
    { (yyval.xml) = xml("extern",NULL,NULL); }
    break;

  case 103:
#line 283 "c_gram.y"
    { (yyval.xml) = xml("static",NULL,NULL); }
    break;

  case 104:
#line 284 "c_gram.y"
    { (yyval.xml) = xml("inline",NULL,NULL); }
    break;

  case 105:
#line 285 "c_gram.y"
    { (yyval.xml) = xml("auto",NULL,NULL); }
    break;

  case 106:
#line 286 "c_gram.y"
    { (yyval.xml) = xml("register",NULL,NULL); }
    break;

  case 107:
#line 290 "c_gram.y"
    { (yyval.xml) = xml_text("t",NULL,strdup("char")); }
    break;

  case 108:
#line 291 "c_gram.y"
    { (yyval.xml) = xml_text("t",NULL,strdup("short")); }
    break;

  case 109:
#line 292 "c_gram.y"
    { (yyval.xml) = xml_text("t",NULL,strdup("int")); }
    break;

  case 110:
#line 293 "c_gram.y"
    { (yyval.xml) = xml_text("t",NULL,strdup("__int64")); }
    break;

  case 111:
#line 294 "c_gram.y"
    { (yyval.xml) = xml_text("t",NULL,strdup("long")); }
    break;

  case 112:
#line 295 "c_gram.y"
    { (yyval.xml) = xml_text("t",NULL,strdup("signed")); }
    break;

  case 113:
#line 296 "c_gram.y"
    { (yyval.xml) = xml_text("t",NULL,strdup("unsigned")); }
    break;

  case 114:
#line 297 "c_gram.y"
    { (yyval.xml) = xml_text("t",NULL,strdup("float")); }
    break;

  case 115:
#line 298 "c_gram.y"
    { (yyval.xml) = xml_text("t",NULL,strdup("double")); }
    break;

  case 116:
#line 299 "c_gram.y"
    { (yyval.xml) = xml_text("t",NULL,strdup("void")); }
    break;

  case 117:
#line 300 "c_gram.y"
    { (yyval.xml) = xml_text("t",NULL,strdup("__builtin_va_list")); }
    break;

  case 118:
#line 301 "c_gram.y"
    { (yyval.xml) = (yyvsp[(1) - (1)].xml); }
    break;

  case 119:
#line 302 "c_gram.y"
    { (yyval.xml) = (yyvsp[(1) - (1)].xml); }
    break;

  case 120:
#line 303 "c_gram.y"
    { (yyval.xml) = xml_text("dt",NULL,(yyvsp[(1) - (1)].str)); }
    break;

  case 121:
#line 307 "c_gram.y"
    { (yyval.xml) = xml((yyvsp[(1) - (5)].str),"id",(yyvsp[(2) - (5)].str),NULL,(yyvsp[(4) - (5)].xml),NULL); }
    break;

  case 122:
#line 308 "c_gram.y"
    { (yyval.xml) = xml((yyvsp[(1) - (5)].str),"id",(yyvsp[(2) - (5)].str),NULL,(yyvsp[(4) - (5)].xml),NULL); }
    break;

  case 123:
#line 309 "c_gram.y"
    { (yyval.xml) = xml((yyvsp[(1) - (4)].str),NULL,(yyvsp[(3) - (4)].xml),NULL); }
    break;

  case 124:
#line 310 "c_gram.y"
    { (yyval.xml) = xml((yyvsp[(1) - (2)].str),"id",(yyvsp[(2) - (2)].str),NULL,NULL); }
    break;

  case 125:
#line 311 "c_gram.y"
    { (yyval.xml) = xml((yyvsp[(1) - (2)].str),"id",(yyvsp[(2) - (2)].str),NULL,NULL); }
    break;

  case 126:
#line 315 "c_gram.y"
    { (yyval.str) = "struct"; }
    break;

  case 127:
#line 316 "c_gram.y"
    { (yyval.str) = "union"; }
    break;

  case 128:
#line 320 "c_gram.y"
    { (yyval.xml) = (yyvsp[(1) - (1)].xml); }
    break;

  case 129:
#line 321 "c_gram.y"
    { (yyval.xml)=xml_link((yyvsp[(1) - (2)].xml),(yyvsp[(2) - (2)].xml)); }
    break;

  case 130:
#line 325 "c_gram.y"
    { (yyval.xml) = xml("d",NULL,(yyvsp[(1) - (2)].xml),NULL); }
    break;

  case 131:
#line 326 "c_gram.y"
    { (yyval.xml) = xml("d",NULL,(yyvsp[(1) - (3)].xml),(yyvsp[(2) - (3)].xml),NULL); }
    break;

  case 132:
#line 330 "c_gram.y"
    { (yyval.xml) = (yyvsp[(1) - (1)].xml); }
    break;

  case 133:
#line 331 "c_gram.y"
    { (yyval.xml) = xml_link((yyvsp[(1) - (2)].xml),(yyvsp[(2) - (2)].xml)); }
    break;

  case 134:
#line 332 "c_gram.y"
    { (yyval.xml) = (yyvsp[(1) - (1)].xml); }
    break;

  case 135:
#line 333 "c_gram.y"
    { (yyval.xml) = xml_link((yyvsp[(1) - (2)].xml),(yyvsp[(2) - (2)].xml)); }
    break;

  case 136:
#line 337 "c_gram.y"
    { (yyval.xml) = (yyvsp[(1) - (1)].xml); }
    break;

  case 137:
#line 338 "c_gram.y"
    { (yyval.xml) = xml_link((yyvsp[(1) - (3)].xml),(yyvsp[(3) - (3)].xml)); }
    break;

  case 138:
#line 342 "c_gram.y"
    { (yyval.xml) = (yyvsp[(1) - (1)].xml); }
    break;

  case 139:
#line 343 "c_gram.y"
    { (yyval.xml) = xml("bitfield",NULL,(yyvsp[(2) - (2)].xml),NULL); }
    break;

  case 140:
#line 344 "c_gram.y"
    { (yyval.xml) = xml("bitfield",NULL,(yyvsp[(1) - (3)].xml),(yyvsp[(3) - (3)].xml),NULL); }
    break;

  case 141:
#line 348 "c_gram.y"
    { (yyval.xml) = xml("enum",NULL,(yyvsp[(3) - (4)].xml),NULL); }
    break;

  case 142:
#line 349 "c_gram.y"
    { (yyval.xml) = xml("enum","id",(yyvsp[(2) - (5)].str),NULL,(yyvsp[(4) - (5)].xml),NULL); }
    break;

  case 143:
#line 350 "c_gram.y"
    { (yyval.xml) = xml("enum",NULL,(yyvsp[(3) - (5)].xml),NULL); }
    break;

  case 144:
#line 351 "c_gram.y"
    { (yyval.xml) = xml("enum","id",(yyvsp[(2) - (6)].str),NULL,(yyvsp[(4) - (6)].xml),NULL); }
    break;

  case 145:
#line 352 "c_gram.y"
    { (yyval.xml) = xml("enum","id",(yyvsp[(2) - (2)].str),NULL,NULL); }
    break;

  case 146:
#line 356 "c_gram.y"
    { (yyval.xml) = (yyvsp[(1) - (1)].xml); }
    break;

  case 147:
#line 357 "c_gram.y"
    { (yyval.xml) = xml_link((yyvsp[(1) - (3)].xml),(yyvsp[(3) - (3)].xml)); }
    break;

  case 148:
#line 361 "c_gram.y"
    { (yyval.xml) = xml_text("id",NULL,(yyvsp[(1) - (1)].str)); }
    break;

  case 149:
#line 362 "c_gram.y"
    { (yyval.xml) = xml_link(xml_text("id",NULL,(yyvsp[(1) - (3)].str)),(yyvsp[(3) - (3)].xml)); }
    break;

  case 150:
#line 366 "c_gram.y"
    { (yyval.xml) = xml("const",NULL,NULL); }
    break;

  case 151:
#line 367 "c_gram.y"
    { (yyval.xml) = xml("volatile",NULL,NULL); }
    break;

  case 152:
#line 371 "c_gram.y"
    { (yyval.xml) = xml("cdecl",NULL,NULL); }
    break;

  case 153:
#line 372 "c_gram.y"
    { (yyval.xml) = xml("stdcall",NULL,NULL); }
    break;

  case 154:
#line 376 "c_gram.y"
    { (yyval.xml) = (yyvsp[(1) - (1)].xml); }
    break;

  case 155:
#line 377 "c_gram.y"
    { (yyval.xml) = xml_link((yyvsp[(1) - (2)].xml),(yyvsp[(2) - (2)].xml)); }
    break;

  case 156:
#line 381 "c_gram.y"
    { (yyval.xml) = (yyvsp[(1) - (1)].xml); }
    break;

  case 157:
#line 382 "c_gram.y"
    { (yyval.xml) = xml("ptr",NULL,(yyvsp[(1) - (2)].xml),(yyvsp[(2) - (2)].xml),NULL); }
    break;

  case 158:
#line 383 "c_gram.y"
    { (yyval.xml) = xml("ptr",NULL,(yyvsp[(1) - (3)].xml),xml_link((yyvsp[(2) - (3)].xml),(yyvsp[(3) - (3)].xml)),NULL); }
    break;

  case 159:
#line 384 "c_gram.y"
    { (yyval.xml) = xml_link((yyvsp[(1) - (2)].xml),(yyvsp[(2) - (2)].xml)); }
    break;

  case 160:
#line 388 "c_gram.y"
    { (yyval.xml) = xml_text("id",NULL,(yyvsp[(1) - (1)].str)); }
    break;

  case 161:
#line 389 "c_gram.y"
    { (yyval.xml) = xml("p",NULL,(yyvsp[(2) - (3)].xml),NULL); }
    break;

  case 162:
#line 390 "c_gram.y"
    { (yyval.xml) = xml("ix",NULL,(yyvsp[(1) - (3)].xml),NULL); }
    break;

  case 163:
#line 391 "c_gram.y"
    { (yyval.xml) = xml("ix",NULL,(yyvsp[(1) - (4)].xml),(yyvsp[(3) - (4)].xml),NULL); }
    break;

  case 164:
#line 392 "c_gram.y"
    { (yyval.xml) = xml("fd",NULL,(yyvsp[(1) - (3)].xml),xml("pm",NULL,NULL),NULL); }
    break;

  case 165:
#line 393 "c_gram.y"
    { (yyval.xml) = xml("fd",NULL,(yyvsp[(1) - (4)].xml),xml("pm",NULL,(yyvsp[(3) - (4)].xml),NULL),NULL); }
    break;

  case 166:
#line 394 "c_gram.y"
    { (yyval.xml) = xml("fd",NULL,(yyvsp[(1) - (4)].xml),xml("pm",NULL,(yyvsp[(3) - (4)].xml),NULL),NULL); }
    break;

  case 167:
#line 398 "c_gram.y"
    { (yyval.xml) = (yyvsp[(1) - (1)].xml); }
    break;

  case 168:
#line 399 "c_gram.y"
    { (yyval.xml) = xml("ptr",NULL,(yyvsp[(1) - (2)].xml),(yyvsp[(2) - (2)].xml),NULL); }
    break;

  case 169:
#line 400 "c_gram.y"
    { (yyval.xml) = xml("ptr",NULL,(yyvsp[(1) - (3)].xml),xml_link((yyvsp[(2) - (3)].xml),(yyvsp[(3) - (3)].xml)),NULL); }
    break;

  case 170:
#line 401 "c_gram.y"
    { (yyval.xml) = xml_link((yyvsp[(1) - (2)].xml),(yyvsp[(2) - (2)].xml)); }
    break;

  case 171:
#line 405 "c_gram.y"
    { (yyval.xml) = xml_text("id",NULL,(yyvsp[(1) - (1)].str)); }
    break;

  case 172:
#line 406 "c_gram.y"
    { (yyval.xml) = xml_text("id",NULL,(yyvsp[(1) - (1)].str)); }
    break;

  case 173:
#line 407 "c_gram.y"
    { (yyval.xml) = xml("p",NULL,(yyvsp[(2) - (3)].xml),NULL); }
    break;

  case 174:
#line 408 "c_gram.y"
    { (yyval.xml) = xml("ix",NULL,(yyvsp[(1) - (3)].xml),NULL); }
    break;

  case 175:
#line 409 "c_gram.y"
    { (yyval.xml) = xml("ix",NULL,(yyvsp[(1) - (4)].xml),(yyvsp[(3) - (4)].xml),NULL); }
    break;

  case 176:
#line 410 "c_gram.y"
    { (yyval.xml) = xml("fd",NULL,(yyvsp[(1) - (3)].xml),xml("pm",NULL,NULL),NULL); }
    break;

  case 177:
#line 411 "c_gram.y"
    { (yyval.xml) = xml("fd",NULL,(yyvsp[(1) - (4)].xml),xml("pm",NULL,(yyvsp[(3) - (4)].xml),NULL),NULL); }
    break;

  case 178:
#line 412 "c_gram.y"
    { (yyval.xml) = xml("fd",NULL,(yyvsp[(1) - (4)].xml),xml("pm",NULL,(yyvsp[(3) - (4)].xml),NULL),NULL); }
    break;

  case 179:
#line 416 "c_gram.y"
    { (yyval.xml) = xml("a",NULL,NULL); }
    break;

  case 180:
#line 417 "c_gram.y"
    { (yyval.xml) = xml("a",NULL,(yyvsp[(2) - (2)].xml),NULL); }
    break;

  case 181:
#line 418 "c_gram.y"
    { (yyval.xml) = xml("a",NULL,(yyvsp[(2) - (2)].xml),NULL); }
    break;

  case 182:
#line 419 "c_gram.y"
    { (yyval.xml) = xml("a",NULL,(yyvsp[(2) - (3)].xml),(yyvsp[(3) - (3)].xml),NULL); }
    break;

  case 183:
#line 423 "c_gram.y"
    { (yyval.xml) = (yyvsp[(1) - (1)].xml); }
    break;

  case 184:
#line 424 "c_gram.y"
    { (yyval.xml) = xml_link((yyvsp[(1) - (2)].xml),(yyvsp[(2) - (2)].xml)); }
    break;

  case 185:
#line 428 "c_gram.y"
    { (yyval.xml) = (yyvsp[(1) - (1)].xml); }
    break;

  case 186:
#line 429 "c_gram.y"
    { (yyval.xml) = xml_link((yyvsp[(1) - (3)].xml),xml("elipsis",NULL,NULL)); }
    break;

  case 187:
#line 433 "c_gram.y"
    { (yyval.xml) = (yyvsp[(1) - (1)].xml); }
    break;

  case 188:
#line 434 "c_gram.y"
    { (yyval.xml) = xml_link((yyvsp[(1) - (3)].xml),(yyvsp[(3) - (3)].xml)); }
    break;

  case 189:
#line 438 "c_gram.y"
    { (yyval.xml) = xml("d",NULL,(yyvsp[(1) - (2)].xml),(yyvsp[(2) - (2)].xml),NULL); ctoxml_deftype_to_ident((yyval.xml)); }
    break;

  case 190:
#line 439 "c_gram.y"
    { (yyval.xml) = xml("d",NULL,(yyvsp[(1) - (2)].xml),(yyvsp[(2) - (2)].xml),NULL); ctoxml_deftype_to_ident((yyval.xml)); }
    break;

  case 191:
#line 440 "c_gram.y"
    { (yyval.xml) = xml("d",NULL,(yyvsp[(1) - (1)].xml),NULL); ctoxml_deftype_to_ident((yyval.xml)); }
    break;

  case 192:
#line 444 "c_gram.y"
    { (yyval.xml) = xml_text("id",NULL,(yyvsp[(1) - (1)].str)); }
    break;

  case 193:
#line 445 "c_gram.y"
    { (yyval.xml) = xml_link((yyvsp[(1) - (3)].xml),xml_text("id",NULL,(yyvsp[(3) - (3)].str))); }
    break;

  case 194:
#line 449 "c_gram.y"
    { (yyval.xml) = xml("d",NULL,(yyvsp[(1) - (1)].xml),NULL); }
    break;

  case 195:
#line 450 "c_gram.y"
    { (yyval.xml) = xml("d",NULL,xml_link((yyvsp[(1) - (2)].xml),(yyvsp[(2) - (2)].xml)),NULL); }
    break;

  case 196:
#line 454 "c_gram.y"
    { (yyval.xml) = xml("ptr",NULL,(yyvsp[(1) - (1)].xml),NULL); }
    break;

  case 197:
#line 455 "c_gram.y"
    { (yyval.xml) = (yyvsp[(1) - (1)].xml); }
    break;

  case 198:
#line 456 "c_gram.y"
    { (yyval.xml) = xml("ptr",NULL,(yyvsp[(1) - (2)].xml),(yyvsp[(2) - (2)].xml),NULL); }
    break;

  case 199:
#line 460 "c_gram.y"
    { (yyval.xml) = xml("p",NULL,(yyvsp[(2) - (3)].xml),NULL); }
    break;

  case 200:
#line 461 "c_gram.y"
    { (yyval.xml) = xml("ix",NULL,NULL); }
    break;

  case 201:
#line 462 "c_gram.y"
    { (yyval.xml) = xml("ix",NULL,(yyvsp[(2) - (3)].xml),NULL); }
    break;

  case 202:
#line 463 "c_gram.y"
    { (yyval.xml) = xml("ix",NULL,(yyvsp[(1) - (3)].xml),NULL); }
    break;

  case 203:
#line 464 "c_gram.y"
    { (yyval.xml) = xml("ix",NULL,(yyvsp[(1) - (4)].xml),(yyvsp[(3) - (4)].xml),NULL); }
    break;

  case 204:
#line 465 "c_gram.y"
    { (yyval.xml) = xml("fd",NULL,NULL); }
    break;

  case 205:
#line 466 "c_gram.y"
    { (yyval.xml) = xml("fd",NULL,(yyvsp[(2) - (3)].xml),NULL); }
    break;

  case 206:
#line 467 "c_gram.y"
    { (yyval.xml) = xml("fd",NULL,(yyvsp[(1) - (3)].xml),NULL); }
    break;

  case 207:
#line 468 "c_gram.y"
    { (yyval.xml) = xml("fd",NULL,(yyvsp[(1) - (4)].xml),(yyvsp[(3) - (4)].xml),NULL); }
    break;

  case 208:
#line 472 "c_gram.y"
    { (yyval.xml) = (yyvsp[(1) - (1)].xml); }
    break;

  case 209:
#line 473 "c_gram.y"
    { (yyval.xml) = xml("p",NULL,(yyvsp[(2) - (3)].xml),NULL); }
    break;

  case 210:
#line 474 "c_gram.y"
    { (yyval.xml) = xml("p",NULL,(yyvsp[(2) - (4)].xml),NULL); }
    break;

  case 211:
#line 478 "c_gram.y"
    { (yyval.xml) = (yyvsp[(1) - (1)].xml); }
    break;

  case 212:
#line 479 "c_gram.y"
    { (yyval.xml) = xml_link((yyvsp[(1) - (3)].xml),(yyvsp[(3) - (3)].xml)); }
    break;

  case 213:
#line 483 "c_gram.y"
    { (yyval.xml) = (yyvsp[(1) - (1)].xml); }
    break;

  case 214:
#line 484 "c_gram.y"
    { (yyval.xml) = (yyvsp[(1) - (1)].xml); }
    break;

  case 215:
#line 485 "c_gram.y"
    { (yyval.xml) = (yyvsp[(1) - (1)].xml); }
    break;

  case 216:
#line 486 "c_gram.y"
    { (yyval.xml) = (yyvsp[(1) - (1)].xml); }
    break;

  case 217:
#line 487 "c_gram.y"
    { (yyval.xml) = (yyvsp[(1) - (1)].xml); }
    break;

  case 218:
#line 488 "c_gram.y"
    { (yyval.xml) = (yyvsp[(1) - (1)].xml); }
    break;

  case 219:
#line 489 "c_gram.y"
    { (yyval.xml) = (yyvsp[(1) - (1)].xml); }
    break;

  case 220:
#line 493 "c_gram.y"
    { (yyval.xml) = xml_link(xml_text("label",NULL,(yyvsp[(1) - (3)].str)),(yyvsp[(3) - (3)].xml)); }
    break;

  case 221:
#line 494 "c_gram.y"
    { (yyval.xml) = xml_link(xml("case",NULL,(yyvsp[(2) - (4)].xml),NULL),(yyvsp[(4) - (4)].xml)); }
    break;

  case 222:
#line 495 "c_gram.y"
    { (yyval.xml) = xml_link(xml("default",NULL,NULL),(yyvsp[(3) - (3)].xml)); }
    break;

  case 223:
#line 499 "c_gram.y"
    { (yyval.xml) = xml_text("asm",NULL,ctoxml_cterm); }
    break;

  case 224:
#line 503 "c_gram.y"
    { (yyval.xml) = xml("p",NULL,NULL); }
    break;

  case 225:
#line 504 "c_gram.y"
    { (yyval.xml) = xml("p",NULL,(yyvsp[(2) - (3)].xml),NULL); }
    break;

  case 226:
#line 508 "c_gram.y"
    { (yyval.xml) = (yyvsp[(1) - (1)].xml); }
    break;

  case 227:
#line 509 "c_gram.y"
    { (yyval.xml) = xml_link((yyvsp[(1) - (2)].xml), (yyvsp[(2) - (2)].xml)); }
    break;

  case 228:
#line 513 "c_gram.y"
    { (yyval.xml) = (yyvsp[(1) - (1)].xml); }
    break;

  case 229:
#line 514 "c_gram.y"
    { (yyval.xml) = (yyvsp[(1) - (1)].xml); }
    break;

  case 230:
#line 515 "c_gram.y"
    { (yyval.xml) = xml_link((yyvsp[(1) - (2)].xml),(yyvsp[(2) - (2)].xml)); }
    break;

  case 231:
#line 516 "c_gram.y"
    { (yyval.xml) = xml_link((yyvsp[(1) - (2)].xml),(yyvsp[(2) - (2)].xml)); }
    break;

  case 232:
#line 520 "c_gram.y"
    { (yyval.xml) = xml("e",NULL,NULL); }
    break;

  case 233:
#line 521 "c_gram.y"
    { (yyval.xml) = (yyvsp[(1) - (2)].xml); }
    break;

  case 234:
#line 525 "c_gram.y"
    { (yyval.xml) = xml("if",NULL,(yyvsp[(3) - (5)].xml),(yyvsp[(5) - (5)].xml),NULL); }
    break;

  case 235:
#line 526 "c_gram.y"
    { (yyval.xml) = xml("if",NULL,(yyvsp[(3) - (7)].xml),(yyvsp[(5) - (7)].xml),(yyvsp[(7) - (7)].xml),NULL); }
    break;

  case 236:
#line 527 "c_gram.y"
    { (yyval.xml) = xml("switch",NULL,(yyvsp[(3) - (5)].xml),(yyvsp[(5) - (5)].xml),NULL); }
    break;

  case 237:
#line 531 "c_gram.y"
    { (yyval.xml) = xml("while",NULL,(yyvsp[(3) - (5)].xml),(yyvsp[(5) - (5)].xml),NULL); }
    break;

  case 238:
#line 532 "c_gram.y"
    { (yyval.xml) = xml("do",NULL,(yyvsp[(2) - (7)].xml),(yyvsp[(5) - (7)].xml),NULL); }
    break;

  case 239:
#line 533 "c_gram.y"
    { (yyval.xml) = xml("for",NULL,xml("e",NULL,NULL),xml("e",NULL,NULL),xml("e",NULL,NULL),(yyvsp[(6) - (6)].xml),NULL); }
    break;

  case 240:
#line 534 "c_gram.y"
    { (yyval.xml) = xml("for",NULL,xml("e",NULL,NULL),xml("e",NULL,NULL),(yyvsp[(5) - (7)].xml),(yyvsp[(7) - (7)].xml),NULL); }
    break;

  case 241:
#line 535 "c_gram.y"
    { (yyval.xml) = xml("for",NULL,xml("e",NULL,NULL),(yyvsp[(4) - (7)].xml),xml("e",NULL,NULL),(yyvsp[(7) - (7)].xml),NULL); }
    break;

  case 242:
#line 536 "c_gram.y"
    { (yyval.xml) = xml("for",NULL,xml("e",NULL,NULL),(yyvsp[(4) - (8)].xml),(yyvsp[(6) - (8)].xml),(yyvsp[(8) - (8)].xml),NULL); }
    break;

  case 243:
#line 537 "c_gram.y"
    { (yyval.xml) = xml("for",NULL,(yyvsp[(3) - (7)].xml),xml("e",NULL,NULL),xml("e",NULL,NULL),(yyvsp[(7) - (7)].xml),NULL); }
    break;

  case 244:
#line 538 "c_gram.y"
    { (yyval.xml) = xml("for",NULL,(yyvsp[(3) - (8)].xml),xml("e",NULL,NULL),(yyvsp[(6) - (8)].xml),(yyvsp[(8) - (8)].xml),NULL); }
    break;

  case 245:
#line 539 "c_gram.y"
    { (yyval.xml) = xml("for",NULL,(yyvsp[(3) - (8)].xml),(yyvsp[(5) - (8)].xml),xml("e",NULL,NULL),(yyvsp[(8) - (8)].xml),NULL); }
    break;

  case 246:
#line 540 "c_gram.y"
    { (yyval.xml) = xml("for",NULL,(yyvsp[(3) - (9)].xml),(yyvsp[(5) - (9)].xml),(yyvsp[(7) - (9)].xml),(yyvsp[(9) - (9)].xml),NULL); }
    break;

  case 247:
#line 544 "c_gram.y"
    { (yyval.xml) = xml_text("goto",NULL,(yyvsp[(2) - (3)].str)); }
    break;

  case 248:
#line 545 "c_gram.y"
    { (yyval.xml) = xml("continue",NULL,NULL); }
    break;

  case 249:
#line 546 "c_gram.y"
    { (yyval.xml) = xml("break",NULL,NULL); }
    break;

  case 250:
#line 547 "c_gram.y"
    { (yyval.xml) = xml("return",NULL,NULL); }
    break;

  case 251:
#line 548 "c_gram.y"
    { (yyval.xml) = xml("return",NULL,(yyvsp[(2) - (3)].xml),NULL); }
    break;

  case 252:
#line 552 "c_gram.y"
    { xml_print((yyvsp[(1) - (1)].xml)); xml_free((yyvsp[(1) - (1)].xml)); }
    break;

  case 253:
#line 553 "c_gram.y"
    { xml_print((yyvsp[(2) - (2)].xml)); xml_free((yyvsp[(2) - (2)].xml)); }
    break;

  case 254:
#line 557 "c_gram.y"
    { (yyval.xml) = (yyvsp[(1) - (1)].xml); }
    break;

  case 255:
#line 558 "c_gram.y"
    { (yyval.xml) = (yyvsp[(1) - (1)].xml); }
    break;

  case 256:
#line 562 "c_gram.y"
    { (yyval.xml) = xml("fdef",NULL,(yyvsp[(1) - (2)].xml),(yyvsp[(2) - (2)].xml),NULL); }
    break;

  case 257:
#line 563 "c_gram.y"
    { (yyval.xml) = xml("fdef",NULL,(yyvsp[(1) - (3)].xml),(yyvsp[(2) - (3)].xml),(yyvsp[(3) - (3)].xml),NULL); }
    break;

  case 258:
#line 564 "c_gram.y"
    { (yyval.xml) = xml("fdef",NULL,(yyvsp[(1) - (3)].xml),(yyvsp[(2) - (3)].xml),(yyvsp[(3) - (3)].xml),NULL); }
    break;

  case 259:
#line 565 "c_gram.y"
    { (yyval.xml) = xml("fdef",NULL,(yyvsp[(1) - (4)].xml),(yyvsp[(2) - (4)].xml),(yyvsp[(3) - (4)].xml),(yyvsp[(4) - (4)].xml),NULL); }
    break;

  case 260:
#line 569 "c_gram.y"
    { (yyval.str) = ctoxml_cterm; }
    break;

  case 261:
#line 573 "c_gram.y"
    { (yyval.str) = ctoxml_cterm; }
    break;

  case 262:
#line 577 "c_gram.y"
    {  }
    break;

  case 263:
#line 578 "c_gram.y"
    {  }
    break;


/* Line 1267 of yacc.c.  */
#line 3521 "c_gram.c"
      default: break;
    }
  YY_SYMBOL_PRINT ("-> $$ =", yyr1[yyn], &yyval, &yyloc);

  YYPOPSTACK (yylen);
  yylen = 0;
  YY_STACK_PRINT (yyss, yyssp);

  *++yyvsp = yyval;


  /* Now `shift' the result of the reduction.  Determine what state
     that goes to, based on the state we popped back to and the rule
     number reduced by.  */

  yyn = yyr1[yyn];

  yystate = yypgoto[yyn - YYNTOKENS] + *yyssp;
  if (0 <= yystate && yystate <= YYLAST && yycheck[yystate] == *yyssp)
    yystate = yytable[yystate];
  else
    yystate = yydefgoto[yyn - YYNTOKENS];

  goto yynewstate;


/*------------------------------------.
| yyerrlab -- here on detecting error |
`------------------------------------*/
yyerrlab:
  /* If not already recovering from an error, report this error.  */
  if (!yyerrstatus)
    {
      ++yynerrs;
#if ! YYERROR_VERBOSE
      yyerror (YY_("syntax error"));
#else
      {
	YYSIZE_T yysize = yysyntax_error (0, yystate, yychar);
	if (yymsg_alloc < yysize && yymsg_alloc < YYSTACK_ALLOC_MAXIMUM)
	  {
	    YYSIZE_T yyalloc = 2 * yysize;
	    if (! (yysize <= yyalloc && yyalloc <= YYSTACK_ALLOC_MAXIMUM))
	      yyalloc = YYSTACK_ALLOC_MAXIMUM;
	    if (yymsg != yymsgbuf)
	      YYSTACK_FREE (yymsg);
	    yymsg = (char *) YYSTACK_ALLOC (yyalloc);
	    if (yymsg)
	      yymsg_alloc = yyalloc;
	    else
	      {
		yymsg = yymsgbuf;
		yymsg_alloc = sizeof yymsgbuf;
	      }
	  }

	if (0 < yysize && yysize <= yymsg_alloc)
	  {
	    (void) yysyntax_error (yymsg, yystate, yychar);
	    yyerror (yymsg);
	  }
	else
	  {
	    yyerror (YY_("syntax error"));
	    if (yysize != 0)
	      goto yyexhaustedlab;
	  }
      }
#endif
    }



  if (yyerrstatus == 3)
    {
      /* If just tried and failed to reuse look-ahead token after an
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
		      yytoken, &yylval);
	  yychar = YYEMPTY;
	}
    }

  /* Else will try to reuse look-ahead token after shifting the error
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

  /* Do not reclaim the symbols of the rule which action triggered
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
  yyerrstatus = 3;	/* Each real token shifted decrements this.  */

  for (;;)
    {
      yyn = yypact[yystate];
      if (yyn != YYPACT_NINF)
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
		  yystos[yystate], yyvsp);
      YYPOPSTACK (1);
      yystate = *yyssp;
      YY_STACK_PRINT (yyss, yyssp);
    }

  if (yyn == YYFINAL)
    YYACCEPT;

  *++yyvsp = yylval;


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

#ifndef yyoverflow
/*-------------------------------------------------.
| yyexhaustedlab -- memory exhaustion comes here.  |
`-------------------------------------------------*/
yyexhaustedlab:
  yyerror (YY_("memory exhausted"));
  yyresult = 2;
  /* Fall through.  */
#endif

yyreturn:
  if (yychar != YYEOF && yychar != YYEMPTY)
     yydestruct ("Cleanup: discarding lookahead",
		 yytoken, &yylval);
  /* Do not reclaim the symbols of the rule which action triggered
     this YYABORT or YYACCEPT.  */
  YYPOPSTACK (yylen);
  YY_STACK_PRINT (yyss, yyssp);
  while (yyssp != yyss)
    {
      yydestruct ("Cleanup: popping",
		  yystos[*yyssp], yyvsp);
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
  /* Make sure YYID is used.  */
  return YYID (yyresult);
}


#line 581 "c_gram.y"


#include <stdio.h>

extern int column;

void ctoxml_cerror (char const *msg) {
  fprintf(stderr,"%s:%d: %s\n",ctoxml_filename_errmsg,ctoxml_clineno,msg);
}

void c_unescape(char *in) {
  char *out = in;
  while(*in) {
    if(*in == '\\') {
      int val;
      ++in;
      switch(*(in++)) {
      case 'a': val = '\a'; break;
        case 't': val = '\t'; break;
        case 'v': val = '\v'; break;
        case 'b': val = '\b'; break;
        case 'r': val = '\r'; break;
        case 'f': val = '\f'; break;
        case 'n': val = '\n'; break;
        case '\\': val = '\\'; break;
        case '?': val = '\?'; break;
        case '\'': val = '\''; break;
        case '\"': val = '\"'; break;
        case 'x': val = strtoul(in, &in, 16); break;
      }
      *(out++) = val;
    } else {
      *(out++) = *(in++);
    }
  }
  *out = 0;
}

