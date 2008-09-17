/* A Bison parser, made by GNU Bison 2.0.  */

/* Skeleton parser for Yacc-like parsing with Bison,
   Copyright (C) 1984, 1989, 1990, 2000, 2001, 2002, 2003, 2004 Free Software Foundation, Inc.

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
   Foundation, Inc., 59 Temple Place - Suite 330,
   Boston, MA 02111-1307, USA.  */

/* As a special exception, when this file is copied by Bison into a
   Bison output file, you may use that output file without restriction.
   This special exception was added by the Free Software Foundation
   in version 1.24 of Bison.  */

/* Tokens.  */
#ifndef YYTOKENTYPE
# define YYTOKENTYPE
   /* Put the tokens into the symbol table, so that GDB and other debuggers
      know about them.  */
   enum yytokentype {
     IDENTIFIER = 258,
     PP_NUMBER = 259,
     CHARACTER_CONSTANT = 260,
     STRING_LITERAL = 261,
     RIGHT_OP = 262,
     LEFT_OP = 263,
     AND_OP = 264,
     OR_OP = 265,
     LE_OP = 266,
     GE_OP = 267,
     EQ_OP = 268,
     NE_OP = 269
   };
#endif
#define IDENTIFIER 258
#define PP_NUMBER 259
#define CHARACTER_CONSTANT 260
#define STRING_LITERAL 261
#define RIGHT_OP 262
#define LEFT_OP 263
#define AND_OP 264
#define OR_OP 265
#define LE_OP 266
#define GE_OP 267
#define EQ_OP 268
#define NE_OP 269




#if ! defined (YYSTYPE) && ! defined (YYSTYPE_IS_DECLARED)
#line 28 "expr_gram.y"
typedef union YYSTYPE {
  int value;
  char *str;
} YYSTYPE;
/* Line 1318 of yacc.c.  */
#line 70 "expr_gram.h"
# define yystype YYSTYPE /* obsolescent; will be withdrawn */
# define YYSTYPE_IS_DECLARED 1
# define YYSTYPE_IS_TRIVIAL 1
#endif

extern YYSTYPE cpp_exprlval;



