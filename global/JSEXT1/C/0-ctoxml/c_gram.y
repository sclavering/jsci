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
  struct Xml *xml;
  char *str;
}

%type <xml> primary_expr postfix_expr argument_expr_list unary_expr cast_expr multiplicative_expr additive_expr shift_expr relational_expr equality_expr and_expr exclusive_or_expr inclusive_or_expr logical_and_expr logical_or_expr conditional_expr assignment_expr expr constant_expr declaration declaration_specifiers init_declarator_list init_declarator struct_or_union_specifier struct_declaration_list struct_declaration struct_declarator_list struct_declarator enum_specifier enumerator_list enumerator declarator direct_declarator pointer identifier_list parameter_type_list parameter_list parameter_declaration type_name abstract_declarator direct_abstract_declarator initializer initializer_list statement labeled_statement compound_statement declaration_list statement_list expression_statement selection_statement iteration_statement jump_statement external_definition function_definition translation_unit type_specifier specifier_qualifier_list type_qualifier_list file storage_class_specifier type_qualifier asm_statement declspec callspec direct_declarator2 declarator2 declarator_list2 strings callspecs

%type <str> identifier assignment_operator unary_operator struct_or_union typedeffed_name

%token IDENTIFIER CONSTANT STRING_LITERAL SIZEOF
%token PTR_OP INC_OP DEC_OP LEFT_OP RIGHT_OP LE_OP GE_OP EQ_OP NE_OP
%token AND_OP OR_OP MUL_ASSIGN DIV_ASSIGN MOD_ASSIGN ADD_ASSIGN
%token SUB_ASSIGN LEFT_ASSIGN RIGHT_ASSIGN AND_ASSIGN
%token XOR_ASSIGN OR_ASSIGN TYPE_NAME ASM DECLSPEC STDCALL CDECL

%token TYPEDEF EXTERN STATIC AUTO REGISTER INLINE
%token CHAR SHORT INT INT64 LONG SIGNED UNSIGNED FLOAT DOUBLE CONST VOLATILE VOID VA
%token STRUCT UNION ENUM ELIPSIS RANGE

%token CASE DEFAULT IF ELSE SWITCH WHILE DO FOR GOTO CONTINUE BREAK RETURN

%start file

%{
#include "ctoxml.h"
#include <string.h>
#include <stdlib.h>
extern char *ctoxml_cterm;
extern char *ctoxml_cterm2;
extern int ctoxml_clineno;
extern char *ctoxml_filename;
void ctoxml_cerror (char const *msg);
%}

%%

primary_expr
	: identifier     { $$ = xml_text("id", 0, $1); }
	| CONSTANT       { 

    if (ctoxml_cterm[0]=='\'') {
      char txt[20];
      c_unescape(ctoxml_cterm);

      sprintf(txt,"%d",(int)(ctoxml_cterm[0]));
      $$ = xml_text("c",0,strdup(txt));
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
	    $$ = xml_text("c",attrib,strdup(value),0,ctoxml_cterm);
	  else
	    $$ = xml_text("c",0,ctoxml_cterm);
    }
    
}
	| strings	 { $$ = $1; }
	| '(' expr ')'   { $$ = xml("p",NULL,$2,NULL); }
	;

strings
	: STRING_LITERAL { c_unescape(ctoxml_cterm); ctoxml_cterm[strlen(ctoxml_cterm)-1]=0; $$ = xml_text("s", NULL, strdup(ctoxml_cterm+1)); free(ctoxml_cterm);}
	| strings STRING_LITERAL { c_unescape(ctoxml_cterm); ctoxml_cterm[strlen(ctoxml_cterm)-1]=0; $$ = xml_link($1, xml_text("s", NULL, strdup(ctoxml_cterm+1))); free(ctoxml_cterm);}
	;

postfix_expr
	: primary_expr                             { $$ = $1; }
	| postfix_expr '[' expr ']'                { $$ = xml("ix",NULL,$1,$3,NULL); }
	| postfix_expr '(' ')'                     { $$ = xml("call",NULL,$1,NULL); }
	| postfix_expr '(' argument_expr_list ')'  { $$ = xml("call",NULL,$1,$3,NULL); }
	| postfix_expr '.' identifier              { $$ = xml("mb",NULL,$1,xml_text("id",NULL,$3),NULL); }
	| postfix_expr PTR_OP identifier           { $$ = xml("ptr",NULL,$1, xml_text("id",NULL,$3),NULL); }
	| postfix_expr INC_OP                      { $$ = xml("op","op",strdup("++"), "post",strdup("1"),NULL, $1,NULL); }
	| postfix_expr DEC_OP                      { $$ = xml("op","op",strdup("--"), "post",strdup("1"),NULL, $1,NULL); }
	;

argument_expr_list
	: assignment_expr                          { $$ = $1; }
	| argument_expr_list ',' assignment_expr   { $$ = xml_link($1, $3); }
	;

unary_expr
	: postfix_expr                             { $$ = $1; }
	| INC_OP unary_expr                        { $$ = xml("op","op",strdup("++"),"pre",strdup("1"), NULL, $2,NULL); }
	| DEC_OP unary_expr                        { $$ = xml("op","op",strdup("--"),"pre",strdup("1"), NULL, $2,NULL); }
	| unary_operator cast_expr                 { $$ = xml("op", "op", strdup($1), NULL, $2,NULL); }
	| SIZEOF unary_expr                        { $$ = xml("op", "op", strdup("sizeof"), "type",strdup("e"), NULL,$2,NULL); }
	| SIZEOF '(' type_name ')'                 { $$ = xml("op", "op", strdup("sizeof"), "type",strdup("t"), NULL,$3,NULL); }
	;

unary_operator
	: '&' { $$ = "&"; }
	| '*' { $$ = "*"; }
	| '+' { $$ = "+"; }
	| '-' { $$ = "-"; }
	| '~' { $$ = "~"; }
	| '!' { $$ = "!"; }
	;

cast_expr
	: unary_expr                   { $$ = $1; }
	| '(' type_name ')' cast_expr  { $$ = xml("cast", NULL, $2, $4,NULL); }
	;

multiplicative_expr
	: cast_expr                         { $$ = $1; }
	| multiplicative_expr '*' cast_expr { $$ = xml("op","op",strdup("*"), NULL,$1, $3,NULL); }
	| multiplicative_expr '/' cast_expr { $$ = xml("op","op",strdup("/"), NULL,$1, $3,NULL); }
	| multiplicative_expr '%' cast_expr { $$ = xml("op","op",strdup("%"), NULL,$1, $3,NULL); }
	;

additive_expr
	: multiplicative_expr                    { $$ = $1; }
	| additive_expr '+' multiplicative_expr  { $$ = xml("op","op",strdup("+"), NULL,$1, $3,NULL); }
	| additive_expr '-' multiplicative_expr  { $$ = xml("op","op",strdup("-"), NULL,$1, $3,NULL); }
	;

shift_expr
	: additive_expr                      { $$ = $1; }
	| shift_expr LEFT_OP additive_expr   { $$ = xml("op","op",strdup("<<"), NULL,$1, $3,NULL); }
	| shift_expr RIGHT_OP additive_expr  { $$ = xml("op","op",strdup(">>"), NULL,$1, $3,NULL); }
	;

relational_expr
	: shift_expr                         { $$ = $1; }
	| relational_expr '<' shift_expr     { $$ = xml("op","op",strdup("<"), NULL,$1, $3,NULL); }
	| relational_expr '>' shift_expr     { $$ = xml("op","op",strdup(">"), NULL,$1, $3,NULL); }
	| relational_expr LE_OP shift_expr   { $$ = xml("op","op",strdup("<="), NULL,$1, $3,NULL); }
	| relational_expr GE_OP shift_expr   { $$ = xml("op","op",strdup(">="), NULL,$1, $3,NULL); }
	;

equality_expr
	: relational_expr                      { $$ = $1; }
	| equality_expr EQ_OP relational_expr  { $$ = xml("op","op",strdup("=="), NULL,$1, $3,NULL); }
	| equality_expr NE_OP relational_expr  { $$ = xml("op","op",strdup("!="), NULL,$1, $3,NULL); }
	;

and_expr
	: equality_expr               { $$ = $1; }
	| and_expr '&' equality_expr  { $$ = xml("op","op",strdup("&"), NULL,$1, $3,NULL); }
	;

exclusive_or_expr
	: and_expr                         { $$ = $1; }
	| exclusive_or_expr '^' and_expr   { $$ = xml("op","op",strdup("^"), NULL,$1, $3,NULL); }
	;

inclusive_or_expr
	: exclusive_or_expr                         { $$ = $1; }
	| inclusive_or_expr '|' exclusive_or_expr   { $$ = xml("op","op",strdup("|"), NULL,$1, $3,NULL); }
	;

logical_and_expr
	: inclusive_or_expr                           { $$ = $1; }
	| logical_and_expr AND_OP inclusive_or_expr   { $$ = xml("op","op",strdup("&&"), NULL,$1, $3,NULL); }
	;

logical_or_expr
	: logical_and_expr                         { $$ = $1; }
	| logical_or_expr OR_OP logical_and_expr   { $$ = xml("op","op",strdup("||"), NULL,$1, $3,NULL); }
	;

conditional_expr
	: logical_or_expr                                { $$ = $1; }
	| logical_or_expr '?' expr ':' conditional_expr  { $$ = xml("op","op",strdup("?:"), NULL,$1, $3,$5,NULL); }
	;

assignment_expr
	: conditional_expr                                 { $$ = $1; }
	| unary_expr assignment_operator assignment_expr   { $$ = xml("op","op",strdup($2), NULL,$1, $3,NULL); }
	;

assignment_operator
	: '='             { $$ = "="; }
	| MUL_ASSIGN      { $$ = "*="; }
	| DIV_ASSIGN      { $$ = "/="; }
	| MOD_ASSIGN      { $$ = "%="; }
	| ADD_ASSIGN      { $$ = "+="; }
	| SUB_ASSIGN      { $$ = "-="; }
	| LEFT_ASSIGN     { $$ = "<<="; }
	| RIGHT_ASSIGN    { $$ = ">>="; }
	| AND_ASSIGN      { $$ = "&="; }
	| XOR_ASSIGN      { $$ = "^="; }
	| OR_ASSIGN       { $$ = "|="; }
	;

expr
	: assignment_expr           { $$ = $1; }
	| expr ',' assignment_expr  { $$ = xml("op","op",strdup(","), NULL,$1, $3,NULL); }
	;

constant_expr
	: conditional_expr          { $$ = $1; }
	;

declaration
	: declaration_specifiers ';'                        { $$ = xml("d",NULL,$1, NULL); }
	| declaration_specifiers init_declarator_list ';'   { $$ = xml("d",NULL,$1,$2,NULL); }
	| TYPEDEF declaration_specifiers declarator_list2 ';'   { $$ = xml("d",NULL,xml("typedef",NULL,NULL),$2,$3,NULL); ctoxml_typedef($$); }
	| TYPEDEF declaration_specifiers ',' declarator_list2 ';'   { $$ = xml("d",NULL,xml("typedef",NULL,NULL),$2,$4,NULL); ctoxml_typedef($$); }
	| TYPEDEF declaration_specifiers ';'   { $$ = xml("d",NULL,xml("typedef",NULL,NULL),$2,NULL); ctoxml_typedef($$); }
	| declspec TYPEDEF declaration_specifiers declarator_list2 ';'   { $$ = xml("d",NULL,xml("typedef",NULL,NULL),$3,$4,NULL); ctoxml_typedef($$); }
	| declspec TYPEDEF declaration_specifiers ',' declarator_list2 ';'   { $$ = xml("d",NULL,xml("typedef",NULL,NULL),$3,$5,NULL); ctoxml_typedef($$); }
	| declspec TYPEDEF declaration_specifiers ';'   { $$ = xml("d",NULL,xml("typedef",NULL,NULL),$3,NULL); ctoxml_typedef($$); }
	;

declaration_specifiers
	: storage_class_specifier                         { $$ = $1; }
	| storage_class_specifier declaration_specifiers  { $$ = xml_link($1,$2); }
	| type_specifier                                  { $$ = $1; }
	| type_specifier declaration_specifiers           { $$ = xml_link($1,$2); }
	| type_qualifier                                  { $$ = $1; }
	| type_qualifier declaration_specifiers           { $$ = xml_link($1,$2); }
	| declspec                                        { $$ = $1; }
	| declspec declaration_specifiers                 { $$ = xml_link($1,$2); }
	;

init_declarator_list
	: init_declarator                           { $$ = $1; }
	| init_declarator_list ',' init_declarator  { $$ = xml_link($1, $3); }
	;

declarator_list2
	: declarator2                           { $$ = $1; }
	| declarator_list2 ',' declarator2  { $$ = xml_link($1, $3); }
	;

init_declarator
	: declarator                  { $$ = $1; }
	| declarator '=' initializer  { $$ = xml("init",NULL,$1,$3,NULL); }
	;

declspec
	: DECLSPEC '(' identifier ')' { $$ = xml_text("declspec","type",$3,NULL,NULL); }
	| DECLSPEC '(' identifier '(' strings ')' ')' { $$ = xml("declspec","type",$3,NULL,$5,NULL); }
	;

storage_class_specifier
	: EXTERN   { $$ = xml("extern",NULL,NULL); }
	| STATIC   { $$ = xml("static",NULL,NULL); }
	| INLINE   { $$ = xml("inline",NULL,NULL); }
	| AUTO     { $$ = xml("auto",NULL,NULL); }
	| REGISTER { $$ = xml("register",NULL,NULL); }
	;

type_specifier
	: CHAR                        { $$ = xml_text("t",NULL,strdup("char")); }
	| SHORT                       { $$ = xml_text("t",NULL,strdup("short")); }
	| INT                         { $$ = xml_text("t",NULL,strdup("int")); }
	| INT64                       { $$ = xml_text("t",NULL,strdup("__int64")); }
	| LONG                        { $$ = xml_text("t",NULL,strdup("long")); }
	| SIGNED                      { $$ = xml_text("t",NULL,strdup("signed")); }
	| UNSIGNED                    { $$ = xml_text("t",NULL,strdup("unsigned")); }
	| FLOAT                       { $$ = xml_text("t",NULL,strdup("float")); }
	| DOUBLE                      { $$ = xml_text("t",NULL,strdup("double")); }
	| VOID                        { $$ = xml_text("t",NULL,strdup("void")); }
	| VA                          { $$ = xml_text("t",NULL,strdup("__builtin_va_list")); }
	| struct_or_union_specifier   { $$ = $1; }
	| enum_specifier              { $$ = $1; }
	| typedeffed_name             { $$ = xml_text("dt",NULL,$1); }
	;

struct_or_union_specifier
	: struct_or_union identifier '{' struct_declaration_list '}'  { $$ = xml($1,"id",$2,NULL,$4,NULL); }
	| struct_or_union typedeffed_name '{' struct_declaration_list '}'  { $$ = xml($1,"id",$2,NULL,$4,NULL); }
	| struct_or_union '{' struct_declaration_list '}'             { $$ = xml($1,NULL,$3,NULL); }
	| struct_or_union identifier                                  { $$ = xml($1,"id",$2,NULL,NULL); }
	| struct_or_union typedeffed_name                             { $$ = xml($1,"id",$2,NULL,NULL); }
	;

struct_or_union
	: STRUCT { $$ = "struct"; }
	| UNION  { $$ = "union"; }
	;

struct_declaration_list
	: struct_declaration                          { $$ = $1; }
	| struct_declaration_list struct_declaration  { $$=xml_link($1,$2); }
	;

struct_declaration
	: specifier_qualifier_list ';'			      { $$ = xml("d",NULL,$1,NULL); }
	| specifier_qualifier_list struct_declarator_list ';' { $$ = xml("d",NULL,$1,$2,NULL); }
	;

specifier_qualifier_list
	: type_specifier                           { $$ = $1; }
	| type_specifier specifier_qualifier_list  { $$ = xml_link($1,$2); }
	| type_qualifier                           { $$ = $1; }
	| type_qualifier specifier_qualifier_list  { $$ = xml_link($1,$2); }
	;

struct_declarator_list
	: struct_declarator                             { $$ = $1; }
	| struct_declarator_list ',' struct_declarator  { $$ = xml_link($1,$3); }
	;

struct_declarator
	: declarator                    { $$ = $1; }
	| ':' constant_expr             { $$ = xml("bitfield",NULL,$2,NULL); }
	| declarator ':' constant_expr  { $$ = xml("bitfield",NULL,$1,$3,NULL); }
	;

enum_specifier
	: ENUM '{' enumerator_list '}'             { $$ = xml("enum",NULL,$3,NULL); }
	| ENUM identifier '{' enumerator_list '}'  { $$ = xml("enum","id",$2,NULL,$4,NULL); }
	| ENUM '{' enumerator_list ',' '}'             { $$ = xml("enum",NULL,$3,NULL); }
	| ENUM identifier '{' enumerator_list ',' '}'  { $$ = xml("enum","id",$2,NULL,$4,NULL); }
	| ENUM identifier                          { $$ = xml("enum","id",$2,NULL,NULL); }
	;

enumerator_list
	: enumerator                      { $$ = $1; }
	| enumerator_list ',' enumerator  { $$ = xml_link($1,$3); }
	;

enumerator
	: identifier                    { $$ = xml_text("id",NULL,$1); }
	| identifier '=' constant_expr  { $$ = xml_link(xml_text("id",NULL,$1),$3); }
	;

type_qualifier
	: CONST                       { $$ = xml("const",NULL,NULL); }
	| VOLATILE                    { $$ = xml("volatile",NULL,NULL); }
	;

callspec
	: CDECL    { $$ = xml("cdecl",NULL,NULL); }
	| STDCALL  { $$ = xml("stdcall",NULL,NULL); }
	;

callspecs
	: callspec    { $$ = $1; }
	| callspecs callspec  { $$ = xml_link($1,$2); }
	;

declarator
	: direct_declarator          { $$ = $1; }
	| pointer direct_declarator  { $$ = xml("ptr",NULL,$1,$2,NULL); }
	| pointer callspecs direct_declarator  { $$ = xml("ptr",NULL,$1,xml_link($2,$3),NULL); }
	| callspec declarator          { $$ = xml_link($1,$2); }
	;

direct_declarator
	: identifier                                           { $$ = xml_text("id",NULL,$1); }
	| '(' declarator ')'                                   { $$ = xml("p",NULL,$2,NULL); }
	| direct_declarator '[' ']'                            { $$ = xml("ix",NULL,$1,NULL); }
	| direct_declarator '[' constant_expr ']'              { $$ = xml("ix",NULL,$1,$3,NULL); }
	| direct_declarator '(' ')'                            { $$ = xml("fd",NULL,$1,xml("pm",NULL,NULL),NULL); }
	| direct_declarator '(' parameter_type_list ')'        { $$ = xml("fd",NULL,$1,xml("pm",NULL,$3,NULL),NULL); }
	| direct_declarator '(' identifier_list ')'            { $$ = xml("fd",NULL,$1,xml("pm",NULL,$3,NULL),NULL); }
	;

declarator2
	: direct_declarator          { $$ = $1; }
	| pointer direct_declarator2  { $$ = xml("ptr",NULL,$1,$2,NULL); }
	| pointer callspec direct_declarator2  { $$ = xml("ptr",NULL,$1,xml_link($2,$3),NULL); }
	| callspec declarator          { $$ = xml_link($1,$2); }
	;

direct_declarator2
	: identifier                                           { $$ = xml_text("id",NULL,$1); }
	| typedeffed_name                                           { $$ = xml_text("id",NULL,$1); }
	| '(' declarator2 ')'                                   { $$ = xml("p",NULL,$2,NULL); }
	| direct_declarator2 '[' ']'                            { $$ = xml("ix",NULL,$1,NULL); }
	| direct_declarator2 '[' constant_expr ']'              { $$ = xml("ix",NULL,$1,$3,NULL); }
	| direct_declarator2 '(' ')'                            { $$ = xml("fd",NULL,$1,xml("pm",NULL,NULL),NULL); }
	| direct_declarator2 '(' parameter_type_list ')'        { $$ = xml("fd",NULL,$1,xml("pm",NULL,$3,NULL),NULL); }
	| direct_declarator2 '(' identifier_list ')'            { $$ = xml("fd",NULL,$1,xml("pm",NULL,$3,NULL),NULL); }
	;

pointer
	: '*'                              { $$ = xml("a",NULL,NULL); }
	| '*' type_qualifier_list          { $$ = xml("a",NULL,$2,NULL); }
	| '*' pointer                      { $$ = xml("a",NULL,$2,NULL); }
	| '*' type_qualifier_list pointer  { $$ = xml("a",NULL,$2,$3,NULL); }
	;

type_qualifier_list
	: type_qualifier                      { $$ = $1; }
	| type_qualifier_list type_qualifier  { $$ = xml_link($1,$2); }
	;

parameter_type_list
	: parameter_list              { $$ = $1; }
	| parameter_list ',' ELIPSIS  { $$ = xml_link($1,xml("elipsis",NULL,NULL)); }
	;

parameter_list
	: parameter_declaration                    { $$ = $1; }
	| parameter_list ',' parameter_declaration { $$ = xml_link($1,$3); }
	;

parameter_declaration
	: declaration_specifiers declarator2           { $$ = xml("d",NULL,$1,$2,NULL); ctoxml_deftype_to_ident($$); }
	| declaration_specifiers abstract_declarator   { $$ = xml("d",NULL,$1,$2,NULL); ctoxml_deftype_to_ident($$); }
	| declaration_specifiers                       { $$ = xml("d",NULL,$1,NULL); ctoxml_deftype_to_ident($$); }
	;

identifier_list
	: identifier                      { $$ = xml_text("id",NULL,$1); }
	| identifier_list ',' identifier  { $$ = xml_link($1,xml_text("id",NULL,$3)); }
	;

type_name
	: specifier_qualifier_list                      { $$ = xml("d",NULL,$1,NULL); }
	| specifier_qualifier_list abstract_declarator  { $$ = xml("d",NULL,xml_link($1,$2),NULL); }
	;

abstract_declarator
	: pointer                             { $$ = xml("ptr",NULL,$1,NULL); }
	| direct_abstract_declarator          { $$ = $1; }
	| pointer direct_abstract_declarator  { $$ = xml("ptr",NULL,$1,$2,NULL); }
	;

direct_abstract_declarator
	: '(' abstract_declarator ')'                             { $$ = xml("p",NULL,$2,NULL); }
	| '[' ']'                                                 { $$ = xml("ix",NULL,NULL); }
	| '[' constant_expr ']'                                   { $$ = xml("ix",NULL,$2,NULL); }
	| direct_abstract_declarator '[' ']'                      { $$ = xml("ix",NULL,$1,NULL); }
	| direct_abstract_declarator '[' constant_expr ']'        { $$ = xml("ix",NULL,$1,$3,NULL); }
	| '(' ')'                                                 { $$ = xml("fd",NULL,NULL); }
	| '(' parameter_type_list ')'                             { $$ = xml("fd",NULL,$2,NULL); }
	| direct_abstract_declarator '(' ')'                      { $$ = xml("fd",NULL,$1,NULL); }
	| direct_abstract_declarator '(' parameter_type_list ')'  { $$ = xml("fd",NULL,$1,$3,NULL); }
	;

initializer
	: assignment_expr               { $$ = $1; }
	| '{' initializer_list '}'      { $$ = xml("p",NULL,$2,NULL); }
	| '{' initializer_list ',' '}'  { $$ = xml("p",NULL,$2,NULL); }
	;

initializer_list
	: initializer                        { $$ = $1; }
	| initializer_list ',' initializer   { $$ = xml_link($1,$3); }
	;

statement
	: labeled_statement       { $$ = $1; }
	| compound_statement      { $$ = $1; }
	| expression_statement    { $$ = $1; }
	| selection_statement     { $$ = $1; }
	| iteration_statement     { $$ = $1; }
	| jump_statement          { $$ = $1; }
	| asm_statement		  { $$ = $1; }
	;

labeled_statement
	: identifier ':' statement          { $$ = xml_link(xml_text("label",NULL,$1),$3); }
	| CASE constant_expr ':' statement  { $$ = xml_link(xml("case",NULL,$2,NULL),$4); }
	| DEFAULT ':' statement             { $$ = xml_link(xml("default",NULL,NULL),$3); }
	;

asm_statement
	: ASM					    { $$ = xml_text("asm",NULL,ctoxml_cterm); }
	;

compound_statement
	: '{' '}'                                   { $$ = xml("p",NULL,NULL); }
	| '{' statement_list '}'                    { $$ = xml("p",NULL,$2,NULL); }
	;

declaration_list
	: declaration                    { $$ = $1; }
	| declaration_list declaration   { $$ = xml_link($1, $2); }
	;

statement_list
	: declaration                 { $$ = $1; }
	| statement                   { $$ = $1; }
	| statement_list statement    { $$ = xml_link($1,$2); }
	| statement_list declaration  { $$ = xml_link($1,$2); }
	;

expression_statement
	: ';'       { $$ = xml("e",NULL,NULL); }
	| expr ';'  { $$ = $1; }
	;

selection_statement
	: IF '(' expr ')' statement                 { $$ = xml("if",NULL,$3,$5,NULL); }
	| IF '(' expr ')' statement ELSE statement  { $$ = xml("if",NULL,$3,$5,$7,NULL); }
	| SWITCH '(' expr ')' statement             { $$ = xml("switch",NULL,$3,$5,NULL); }
	;

iteration_statement
	: WHILE '(' expr ')' statement                 { $$ = xml("while",NULL,$3,$5,NULL); }
	| DO statement WHILE '(' expr ')' ';'          { $$ = xml("do",NULL,$2,$5,NULL); }
	| FOR '(' ';' ';' ')' statement                { $$ = xml("for",NULL,xml("e",NULL,NULL),xml("e",NULL,NULL),xml("e",NULL,NULL),$6,NULL); }
	| FOR '(' ';' ';' expr ')' statement           { $$ = xml("for",NULL,xml("e",NULL,NULL),xml("e",NULL,NULL),$5,$7,NULL); }
	| FOR '(' ';' expr ';' ')' statement           { $$ = xml("for",NULL,xml("e",NULL,NULL),$4,xml("e",NULL,NULL),$7,NULL); }
	| FOR '(' ';' expr ';' expr ')' statement      { $$ = xml("for",NULL,xml("e",NULL,NULL),$4,$6,$8,NULL); }
	| FOR '(' expr ';' ';' ')' statement           { $$ = xml("for",NULL,$3,xml("e",NULL,NULL),xml("e",NULL,NULL),$7,NULL); }
	| FOR '(' expr ';' ';' expr ')' statement      { $$ = xml("for",NULL,$3,xml("e",NULL,NULL),$6,$8,NULL); }
	| FOR '(' expr ';' expr ';' ')' statement      { $$ = xml("for",NULL,$3,$5,xml("e",NULL,NULL),$8,NULL); }
	| FOR '(' expr ';' expr ';' expr ')' statement { $$ = xml("for",NULL,$3,$5,$7,$9,NULL); }
	;

jump_statement
	: GOTO identifier ';'  { $$ = xml_text("goto",NULL,$2); }
	| CONTINUE ';'         { $$ = xml("continue",NULL,NULL); }
	| BREAK ';'            { $$ = xml("break",NULL,NULL); }
	| RETURN ';'           { $$ = xml("return",NULL,NULL); }
	| RETURN expr ';'      { $$ = xml("return",NULL,$2,NULL); }
	;

translation_unit
	: external_definition			{ xml_print($1); xml_free($1); }
	| translation_unit external_definition	{ xml_print($2); xml_free($2); }
	;

external_definition
	: function_definition     	{ $$ = $1; }
	| declaration             	{ $$ = $1; }
	;

function_definition
	: declarator compound_statement                                          { $$ = xml("fdef",NULL,$1,$2,NULL); }
	| declarator declaration_list compound_statement                         { $$ = xml("fdef",NULL,$1,$2,$3,NULL); }
	| declaration_specifiers declarator compound_statement                   { $$ = xml("fdef",NULL,$1,$2,$3,NULL); }
	| declaration_specifiers declarator declaration_list compound_statement  { $$ = xml("fdef",NULL,$1,$2,$3,$4,NULL); }
	;

typedeffed_name
	: TYPE_NAME		      { $$ = ctoxml_cterm; }
	;

identifier
	: IDENTIFIER	{ $$ = ctoxml_cterm; }
	;

file
	: translation_unit	{  }
	|			{  }
	;

%%

#include <stdio.h>

extern int column;

void ctoxml_cerror (char const *msg) {
  fprintf(stderr,"%s:%d: %s\n",ctoxml_filename_errmsg,ctoxml_clineno,msg);
}
