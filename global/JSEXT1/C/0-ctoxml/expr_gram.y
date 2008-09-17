/*
* Copyright (c) 2006, Berges Allmenndigitale Rådgivningstjeneste
* All rights reserved.
* Redistribution and use in source and binary forms, with or without
* modification, are permitted provided that the following conditions are met:
*
*     * Redistributions of source code must retain the above copyright
*       notice, this list of conditions and the following disclaimer.
*     * Redistributions in binary form must reproduce the above copyright
*       notice, this list of conditions and the following disclaimer in the
*       documentation and/or other materials provided with the distribution.
*     * Neither the name of Berges Allmenndigitale Rådgivningstjeneste nor the
*       names of its contributors may be used to endorse or promote products
*       derived from this software without specific prior written permission.
*
* THIS SOFTWARE IS PROVIDED BY THE BERGES AND CONTRIBUTORS ``AS IS'' AND ANY
* EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED
* WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE
* DISCLAIMED. IN NO EVENT SHALL THE BERGES AND CONTRIBUTORS BE LIABLE FOR ANY
* DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES
* (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES;
* LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND
* ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
* (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS
* SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
*/

%union {
  int value;
  char *str;
}

%type <value> primary_expr constant_expression postfix_expr unary_expr cast_expr multiplicative_expr additive_expr shift_expr relational_expr equality_expr and_expr exclusive_or_expr inclusive_or_expr logical_and_expr logical_or_expr conditional_expr
%type <str> identifier

%token IDENTIFIER PP_NUMBER CHARACTER_CONSTANT STRING_LITERAL
%token RIGHT_OP LEFT_OP AND_OP OR_OP LE_OP GE_OP EQ_OP NE_OP

%start constant_expression

%{

#include "stringhash.h"

extern char *cpp_exprterm;
extern stringhash *macros;
extern int evalval;

%}

%%

constant_expression
	: conditional_expr { evalval=$1; }
	;

primary_expr
	: CHARACTER_CONSTANT { 
  int val;
  char txt[20];
  if (cpp_exprterm[1]=='\\') {
    char *s=cpp_exprterm+2;
    switch(s[0]) {
    case 'a': val='\a'; break;
    case 't': val='\t'; break;
    case 'v': val='\v'; break;
    case 'b': val='\b'; break;
    case 'r': val='\r'; break;
    case 'f': val='\f'; break;
    case 'n': val='\n'; break;
    case '\\': val='\\'; break;
    case '?': val='\?'; break;
    case '\'': val='\''; break;
    case '\"': val='\"'; break;
    case 'x': val=strtoul(s+1,&s,16); break;
    }
    s++;
  } else {
    val=cpp_exprterm[1];
  }
  $$ = val;
  free(cpp_exprterm);
	}
        | PP_NUMBER	{ $$ = atoi(cpp_exprterm); free(cpp_exprterm); }
	| STRING_LITERAL { $$ = 1; free(cpp_exprterm); }
	| identifier     { $$ = 0; free(cpp_exprterm); }
	| '(' constant_expression ')'   { $$ = $2; }
	;

postfix_expr
	: primary_expr                             { $$ = $1; }
        | '@' identifier			   { $$ = stringhash_search(macros, $2) != 0; free($2); }
	;

unary_expr
	: postfix_expr                             { $$ = $1; }
	| '-' cast_expr                 	   { $$ = -$2; }
	| '+' cast_expr                 	   { $$ = $2; }
	| '~' cast_expr                 	   { $$ = ~$2; }
	| '!' cast_expr                 	   { $$ = !$2; }
	;

cast_expr
	: unary_expr                   { $$ = $1; }
	;

multiplicative_expr
	: cast_expr                         { $$ = $1; }
	| multiplicative_expr '*' cast_expr { $$ = $1*$3; }
	| multiplicative_expr '/' cast_expr { $$ = $1/$3; }
	| multiplicative_expr '%' cast_expr { $$ = $1%$3; }
	;

additive_expr
	: multiplicative_expr                    { $$ = $1; }
	| additive_expr '+' multiplicative_expr  { $$ = $1+$3; }
	| additive_expr '-' multiplicative_expr  { $$ = $1-$3; }
	;

shift_expr
	: additive_expr                      { $$ = $1; }
	| shift_expr LEFT_OP additive_expr   { $$ = $1 << $3; }
	| shift_expr RIGHT_OP additive_expr  { $$ = $1 >> $3; }
	;

relational_expr
	: shift_expr                         { $$ = $1; }
	| relational_expr '<' shift_expr     { $$ = $1 < $3; }
	| relational_expr '>' shift_expr     { $$ = $1 > $3; }
	| relational_expr LE_OP shift_expr   { $$ = $1 <= $3; }
	| relational_expr GE_OP shift_expr   { $$ = $1 >= $3; }
	;

equality_expr
	: relational_expr                      { $$ = $1; }
	| equality_expr EQ_OP relational_expr  { $$ = $1 == $3; }
	| equality_expr NE_OP relational_expr  { $$ = $1 != $3; }
	;

and_expr
	: equality_expr               { $$ = $1; }
	| and_expr '&' equality_expr  { $$ = $1 & $3; }
	;

exclusive_or_expr
	: and_expr                         { $$ = $1; }
	| exclusive_or_expr '^' and_expr   { $$ = $1 ^ $3; }
	;

inclusive_or_expr
	: exclusive_or_expr                         { $$ = $1; }
	| inclusive_or_expr '|' exclusive_or_expr   { $$ = $1 | $3; }
	;

logical_and_expr
	: inclusive_or_expr                           { $$ = $1; }
	| logical_and_expr AND_OP inclusive_or_expr   { $$ = $1 && $3; }
	;

logical_or_expr
	: logical_and_expr                         { $$ = $1; }
	| logical_or_expr OR_OP logical_and_expr   { $$ = $1 || $3; }
	;

conditional_expr
	: logical_or_expr                                { $$ = $1; }
	| logical_or_expr '?' constant_expression ':' conditional_expr  { $$ = $1?$3:$5; }
	;

identifier
	: IDENTIFIER { $$ = cpp_exprterm; }
	;

%%

int evalval;

cpp_exprerror(char *s)
{
}
