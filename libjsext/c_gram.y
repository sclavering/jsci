/*
* Copyright (c) 2006, Berges Allmenndigitale R�dgivningstjeneste
* All rights reserved.
* Redistribution and use in source and binary forms, with or without
* modification, are permitted provided that the following conditions are met:
*
*     * Redistributions of source code must retain the above copyright
*       notice, this list of conditions and the following disclaimer.
*     * Redistributions in binary form must reproduce the above copyright
*       notice, this list of conditions and the following disclaimer in the
*       documentation and/or other materials provided with the distribution.
*     * Neither the name of Berges Allmenndigitale R�dgivningstjeneste nor the
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
  struct _XmlNode *xml;
  char *str;
}

%type <xml> primary_expr postfix_expr argument_expr_list unary_expr cast_expr multiplicative_expr additive_expr shift_expr relational_expr equality_expr and_expr exclusive_or_expr inclusive_or_expr logical_and_expr logical_or_expr conditional_expr assignment_expr expr constant_expr declaration declaration_specifiers init_declarator_list init_declarator struct_or_union_specifier struct_declaration_list struct_declaration struct_declarator_list struct_declarator enum_specifier enumerator_list enumerator declarator direct_declarator pointer parameter_type_list parameter_list parameter_declaration type_name abstract_declarator direct_abstract_declarator initializer initializer_list statement labeled_statement compound_statement declaration_list statement_list expression_statement selection_statement iteration_statement jump_statement external_definition function_definition translation_unit type_specifier specifier_qualifier_list type_qualifier_list file storage_class_specifier type_qualifier asm_statement direct_declarator2 declarator2 declarator_list2 strings

%type <str> identifier assignment_operator unary_operator struct_or_union typedeffed_name

%token IDENTIFIER CONSTANT STRING_LITERAL SIZEOF
%token PTR_OP INC_OP DEC_OP LEFT_OP RIGHT_OP LE_OP GE_OP EQ_OP NE_OP
%token AND_OP OR_OP MUL_ASSIGN DIV_ASSIGN MOD_ASSIGN ADD_ASSIGN
%token SUB_ASSIGN LEFT_ASSIGN RIGHT_ASSIGN AND_ASSIGN
%token XOR_ASSIGN OR_ASSIGN TYPE_NAME ASM

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
extern int ctoxml_clineno;
extern char *ctoxml_filename;
void ctoxml_cerror (char const *msg);
XmlNode *parse_constant(void);
void ctoxml_deftype_to_ident(XmlNode *e);
void ctoxml_typedef(XmlNode *e);

%}

%%

primary_expr
  : identifier                                             { $$ = xml_text("id", $1); }
  | CONSTANT                                               { $$ = parse_constant(); }
  | strings                                                { $$ = $1; }
  | '(' expr ')'                                           { $$ = xml("p", $2, 0); }
  ;

strings
  : STRING_LITERAL { c_unescape(ctoxml_cterm); ctoxml_cterm[strlen(ctoxml_cterm)-1]=0; $$ = xml_text("s", ctoxml_cterm + 1); free(ctoxml_cterm);}
  | strings STRING_LITERAL { c_unescape(ctoxml_cterm); ctoxml_cterm[strlen(ctoxml_cterm)-1]=0; $$ = xml_link($1, xml_text("s", ctoxml_cterm + 1)); free(ctoxml_cterm);}
  ;

postfix_expr
  : primary_expr                                           { $$ = $1; }
  | postfix_expr '[' expr ']'                              { $$ = xml("ix", $1, $3, 0); }
  | postfix_expr '(' ')'                                   { $$ = xml("call", $1, 0); }
  | postfix_expr '(' argument_expr_list ')'                { $$ = xml("call", $1, $3, 0); }
  | postfix_expr '.' identifier                            { $$ = xml("mb", $1, xml_text("id", $3), 0); }
  | postfix_expr PTR_OP identifier                         { $$ = xml("ptr", $1, xml_text("id", $3), 0); }
  | postfix_expr INC_OP                                    { $$ = xml_attrs(xml("op", $1, 0), "op", "++", "post", "1", 0); }
  | postfix_expr DEC_OP                                    { $$ = xml_attrs(xml("op", $1, 0), "op", "--", "post", "1", 0); }
  ;

argument_expr_list
  : assignment_expr                                        { $$ = $1; }
  | argument_expr_list ',' assignment_expr                 { $$ = xml_link($1, $3); }
  ;

unary_expr
  : postfix_expr                                           { $$ = $1; }
  | INC_OP unary_expr                                      { $$ = xml_attrs(xml("op", $2, 0), "op", "++", "pre", "1", 0); }
  | DEC_OP unary_expr                                      { $$ = xml_attrs(xml("op", $2, 0), "op", "--", "pre", "1", 0); }
  | unary_operator cast_expr                               { $$ = xml_attrs(xml("op", $2, 0), "op", $1, 0); }
  | SIZEOF unary_expr                                      { $$ = xml_attrs(xml("op", $2, 0), "op", "sizeof", "type", "e", 0); }
  | SIZEOF '(' type_name ')'                               { $$ = xml_attrs(xml("op", $3, 0), "op", "sizeof", "type", "t", 0); }
  ;

unary_operator
  : '&'                                                    { $$ = "&"; }
  | '*'                                                    { $$ = "*"; }
  | '+'                                                    { $$ = "+"; }
  | '-'                                                    { $$ = "-"; }
  | '~'                                                    { $$ = "~"; }
  | '!'                                                    { $$ = "!"; }
  ;

cast_expr
  : unary_expr                                             { $$ = $1; }
  | '(' type_name ')' cast_expr                            { $$ = xml("cast", $2, $4, 0); }
  ;

multiplicative_expr
  : cast_expr                                              { $$ = $1; }
  | multiplicative_expr '*' cast_expr                      { $$ = xml_attrs(xml("op", $1, $3, 0), "op", "*", 0); }
  | multiplicative_expr '/' cast_expr                      { $$ = xml_attrs(xml("op", $1, $3, 0), "op", "/", 0); }
  | multiplicative_expr '%' cast_expr                      { $$ = xml_attrs(xml("op", $1, $3, 0), "op", "%", 0); }
  ;

additive_expr
  : multiplicative_expr                                    { $$ = $1; }
  | additive_expr '+' multiplicative_expr                  { $$ = xml_attrs(xml("op", $1, $3, 0), "op", "+", 0); }
  | additive_expr '-' multiplicative_expr                  { $$ = xml_attrs(xml("op", $1, $3, 0), "op", "-", 0); }
  ;

shift_expr
  : additive_expr                                          { $$ = $1; }
  | shift_expr LEFT_OP additive_expr                       { $$ = xml_attrs(xml("op", $1, $3, 0), "op", "<<", 0); }
  | shift_expr RIGHT_OP additive_expr                      { $$ = xml_attrs(xml("op", $1, $3, 0), "op", ">>", 0); }
  ;

relational_expr
  : shift_expr                                             { $$ = $1; }
  | relational_expr '<' shift_expr                         { $$ = xml_attrs(xml("op", $1, $3, 0), "op", "<", 0); }
  | relational_expr '>' shift_expr                         { $$ = xml_attrs(xml("op", $1, $3, 0), "op", ">", 0); }
  | relational_expr LE_OP shift_expr                       { $$ = xml_attrs(xml("op", $1, $3, 0), "op", "<=", 0); }
  | relational_expr GE_OP shift_expr                       { $$ = xml_attrs(xml("op", $1, $3, 0), "op", ">=", 0); }
  ;

equality_expr
  : relational_expr                                        { $$ = $1; }
  | equality_expr EQ_OP relational_expr                    { $$ = xml_attrs(xml("op", $1, $3, 0), "op", "==", 0); }
  | equality_expr NE_OP relational_expr                    { $$ = xml_attrs(xml("op", $1, $3, 0), "op", "!=", 0); }
  ;

and_expr
  : equality_expr                                          { $$ = $1; }
  | and_expr '&' equality_expr                             { $$ = xml_attrs(xml("op", $1, $3, 0), "op", "&", 0); }
  ;

exclusive_or_expr
  : and_expr                                               { $$ = $1; }
  | exclusive_or_expr '^' and_expr                         { $$ = xml_attrs(xml("op", $1, $3, 0), "op", "^", 0); }
  ;

inclusive_or_expr
  : exclusive_or_expr                                      { $$ = $1; }
  | inclusive_or_expr '|' exclusive_or_expr                { $$ = xml_attrs(xml("op", $1, $3, 0), "op", "|", 0); }
  ;

logical_and_expr
  : inclusive_or_expr                                      { $$ = $1; }
  | logical_and_expr AND_OP inclusive_or_expr              { $$ = xml_attrs(xml("op", $1, $3, 0), "op", "&&", 0); }
  ;

logical_or_expr
  : logical_and_expr                                       { $$ = $1; }
  | logical_or_expr OR_OP logical_and_expr                 { $$ = xml_attrs(xml("op", $1, $3, 0), "op", "||", 0); }
  ;

conditional_expr
  : logical_or_expr                                        { $$ = $1; }
  | logical_or_expr '?' expr ':' conditional_expr          { $$ = xml_attrs(xml("op", $1, $3, $5, 0), "op", "?:", 0); }
  ;

assignment_expr
  : conditional_expr                                       { $$ = $1; }
  | unary_expr assignment_operator assignment_expr         { $$ = xml_attrs(xml("op", $1, $3, 0), "op", $2, 0); }
  ;

assignment_operator
  : '='                                                    { $$ = "="; }
  | MUL_ASSIGN                                             { $$ = "*="; }
  | DIV_ASSIGN                                             { $$ = "/="; }
  | MOD_ASSIGN                                             { $$ = "%="; }
  | ADD_ASSIGN                                             { $$ = "+="; }
  | SUB_ASSIGN                                             { $$ = "-="; }
  | LEFT_ASSIGN                                            { $$ = "<<="; }
  | RIGHT_ASSIGN                                           { $$ = ">>="; }
  | AND_ASSIGN                                             { $$ = "&="; }
  | XOR_ASSIGN                                             { $$ = "^="; }
  | OR_ASSIGN                                              { $$ = "|="; }
  ;

expr
  : assignment_expr                                        { $$ = $1; }
  | expr ',' assignment_expr                               { $$ = xml_attrs(xml("op", $1, $3), "op", ",", 0); }
  ;

constant_expr
  : conditional_expr                                       { $$ = $1; }
  ;

declaration
  : declaration_specifiers ';'                             { $$ = xml("d", $1, 0); }
  | declaration_specifiers init_declarator_list ';'        { $$ = xml("d", $1, $2, 0); }
  | TYPEDEF declaration_specifiers declarator_list2 ';'    { $$ = xml("d", xml("typedef", 0), $2, $3,0); ctoxml_typedef($$); }
  | TYPEDEF declaration_specifiers ',' declarator_list2 ';' { $$ = xml("d", xml("typedef", 0), $2, $4, 0); ctoxml_typedef($$); }
  | TYPEDEF declaration_specifiers ';'                     { $$ = xml("d", xml("typedef", 0), $2, 0); ctoxml_typedef($$); }
  ;

declaration_specifiers
  : storage_class_specifier                                { $$ = $1; }
  | storage_class_specifier declaration_specifiers         { $$ = xml_link($1,$2); }
  | type_specifier                                         { $$ = $1; }
  | type_specifier declaration_specifiers                  { $$ = xml_link($1,$2); }
  | type_qualifier                                         { $$ = $1; }
  | type_qualifier declaration_specifiers                  { $$ = xml_link($1,$2); }
  ;

init_declarator_list
  : init_declarator                                        { $$ = $1; }
  | init_declarator_list ',' init_declarator               { $$ = xml_link($1, $3); }
  ;

declarator_list2
  : declarator2                                            { $$ = $1; }
  | declarator_list2 ',' declarator2                       { $$ = xml_link($1, $3); }
  ;

init_declarator
  : declarator                                             { $$ = $1; }
  | declarator '=' initializer                             { $$ = xml("init", $1, $3, 0); }
  ;

storage_class_specifier
  : EXTERN                                                 { $$ = xml("extern", 0); }
  | STATIC                                                 { $$ = xml("static", 0); }
  | INLINE                                                 { $$ = xml("inline", 0); }
  | AUTO                                                   { $$ = xml("auto", 0); }
  | REGISTER                                               { $$ = xml("register", 0); }
  ;

type_specifier
  : CHAR                                                   { $$ = xml_text("t", "char"); }
  | SHORT                                                  { $$ = xml_text("t", "short"); }
  | INT                                                    { $$ = xml_text("t", "int"); }
  | INT64                                                  { $$ = xml_text("t", "__int64"); }
  | LONG                                                   { $$ = xml_text("t", "long"); }
  | SIGNED                                                 { $$ = xml_text("t", "signed"); }
  | UNSIGNED                                               { $$ = xml_text("t", "unsigned"); }
  | FLOAT                                                  { $$ = xml_text("t", "float"); }
  | DOUBLE                                                 { $$ = xml_text("t", "double"); }
  | VOID                                                   { $$ = xml_text("t", "void"); }
  | VA                                                     { $$ = xml_text("t", "__builtin_va_list"); }
  | struct_or_union_specifier                              { $$ = $1; }
  | enum_specifier                                         { $$ = $1; }
  | typedeffed_name                                        { $$ = xml_text("dt", $1); }
  ;

struct_or_union_specifier
  : struct_or_union identifier '{' struct_declaration_list '}'       { $$ = xml_attrs(xml($1, $4, 0), "id", $2, 0); }
  | struct_or_union typedeffed_name '{' struct_declaration_list '}'  { $$ = xml_attrs(xml($1, $4, 0), "id", $2, 0); }
  | struct_or_union '{' struct_declaration_list '}'                  { $$ = xml($1, $3, 0); }
  | struct_or_union identifier                                       { $$ = xml_attrs(xml($1, 0), "id", $2, 0); }
  | struct_or_union typedeffed_name                                  { $$ = xml_attrs(xml($1, 0), "id", $2, 0); }
  ;

struct_or_union
  : STRUCT { $$ = "struct"; }
  | UNION  { $$ = "union"; }
  ;

struct_declaration_list
  : struct_declaration                                     { $$ = $1; }
  | struct_declaration_list struct_declaration             { $$ = xml_link($1, $2); }
  ;

struct_declaration
  : specifier_qualifier_list ';'                           { $$ = xml("d", $1, 0); }
  | specifier_qualifier_list struct_declarator_list ';'    { $$ = xml("d", $1, $2, 0); }
  ;

specifier_qualifier_list
  : type_specifier                                         { $$ = $1; }
  | type_specifier specifier_qualifier_list                { $$ = xml_link($1, $2); }
  | type_qualifier                                         { $$ = $1; }
  | type_qualifier specifier_qualifier_list                { $$ = xml_link($1, $2); }
  ;

struct_declarator_list
  : struct_declarator                                      { $$ = $1; }
  | struct_declarator_list ',' struct_declarator           { $$ = xml_link($1, $3); }
  ;

struct_declarator
  : declarator                                             { $$ = $1; }
  | ':' constant_expr                                      { $$ = xml("bitfield", $2, 0); }
  | declarator ':' constant_expr                           { $$ = xml("bitfield", $1, $3, 0); }
  ;

enum_specifier
  : ENUM '{' enumerator_list '}'                           { $$ = xml("enum", $3, 0); }
  | ENUM identifier '{' enumerator_list '}'                { $$ = xml_attrs(xml("enum", $4, 0), "id", $2, 0); }
  | ENUM '{' enumerator_list ',' '}'                       { $$ = xml("enum", $3, 0); }
  | ENUM identifier '{' enumerator_list ',' '}'            { $$ = xml_attrs(xml("enum", $4, 0), "id", $2, 0); }
  | ENUM identifier                                        { $$ = xml_attrs(xml("enum", 0), "id", $2, 0); }
  ;

enumerator_list
  : enumerator                                             { $$ = $1; }
  | enumerator_list ',' enumerator                         { $$ = xml_link($1, $3); }
  ;

enumerator
  : identifier                                             { $$ = xml_text("id", $1); }
  | identifier '=' constant_expr                           { $$ = xml_link(xml_text("id", $1), $3); }
  ;

type_qualifier
  : CONST                                                  { $$ = xml("const", 0); }
  | VOLATILE                                               { $$ = xml("volatile", 0); }
  ;

declarator
  : direct_declarator                                      { $$ = $1; }
  | pointer direct_declarator                              { $$ = xml("ptr", $1, $2, 0); }
  ;

direct_declarator
  : identifier                                             { $$ = xml_text("id", $1); }
  | '(' declarator ')'                                     { $$ = xml("p", $2, 0); }
  | direct_declarator '[' ']'                              { $$ = xml("ix", $1, 0); }
  | direct_declarator '[' constant_expr ']'                { $$ = xml("ix", $1, $3, 0); }
  | direct_declarator '(' ')'                              { $$ = xml("fd", $1, xml("pm", 0), 0); }
  | direct_declarator '(' parameter_type_list ')'          { $$ = xml("fd", $1, xml("pm", $3, 0), 0); }
  ;

declarator2
  : direct_declarator                                      { $$ = $1; }
  | pointer direct_declarator2                             { $$ = xml("ptr", $1, $2, 0); }
  ;

direct_declarator2
  : identifier                                             { $$ = xml_text("id", $1); }
  | typedeffed_name                                        { $$ = xml_text("id", $1); }
  | '(' declarator2 ')'                                    { $$ = xml("p", $2, 0); }
  | direct_declarator2 '[' ']'                             { $$ = xml("ix", $1, 0); }
  | direct_declarator2 '[' constant_expr ']'               { $$ = xml("ix", $1, $3, 0); }
  | direct_declarator2 '(' ')'                             { $$ = xml("fd", $1, xml("pm", 0), 0); }
  | direct_declarator2 '(' parameter_type_list ')'         { $$ = xml("fd", $1, xml("pm", $3, 0), 0); }
  ;

pointer
  : '*'                                                    { $$ = xml("a", 0); }
  | '*' type_qualifier_list                                { $$ = xml("a", $2, 0); }
  | '*' pointer                                            { $$ = xml("a", $2, 0); }
  | '*' type_qualifier_list pointer                        { $$ = xml("a", $2, $3, 0); }
  ;

type_qualifier_list
  : type_qualifier                                         { $$ = $1; }
  | type_qualifier_list type_qualifier                     { $$ = xml_link($1, $2); }
  ;

parameter_type_list
  : parameter_list                                         { $$ = $1; }
  | parameter_list ',' ELIPSIS                             { $$ = xml_link($1, xml("elipsis", 0)); }
  ;

parameter_list
  : parameter_declaration                                  { $$ = $1; }
  | parameter_list ',' parameter_declaration               { $$ = xml_link($1, $3); }
  ;

parameter_declaration
  : declaration_specifiers declarator2                     { $$ = xml("d", $1, $2, 0); ctoxml_deftype_to_ident($$); }
  | declaration_specifiers abstract_declarator             { $$ = xml("d", $1, $2, 0); ctoxml_deftype_to_ident($$); }
  | declaration_specifiers                                 { $$ = xml("d", $1, 0); ctoxml_deftype_to_ident($$); }
  ;

type_name
  : specifier_qualifier_list                               { $$ = xml("d", $1, 0); }
  | specifier_qualifier_list abstract_declarator           { $$ = xml("d", xml_link($1, $2), 0); }
  ;

abstract_declarator
  : pointer                                                { $$ = xml("ptr", $1, 0); }
  | direct_abstract_declarator                             { $$ = $1; }
  | pointer direct_abstract_declarator                     { $$ = xml("ptr", $1, $2, 0); }
  ;

direct_abstract_declarator
  : '(' abstract_declarator ')'                            { $$ = xml("p", $2, 0); }
  | '[' ']'                                                { $$ = xml("ix", 0); }
  | '[' constant_expr ']'                                  { $$ = xml("ix", $2, 0); }
  | direct_abstract_declarator '[' ']'                     { $$ = xml("ix", $1, 0); }
  | direct_abstract_declarator '[' constant_expr ']'       { $$ = xml("ix", $1, $3, 0); }
  | '(' ')'                                                { $$ = xml("fd", 0); }
  | '(' parameter_type_list ')'                            { $$ = xml("fd", $2, 0); }
  | direct_abstract_declarator '(' ')'                     { $$ = xml("fd", $1, 0); }
  | direct_abstract_declarator '(' parameter_type_list ')' { $$ = xml("fd", $1, $3, 0); }
  ;

initializer
  : assignment_expr                                        { $$ = $1; }
  | '{' initializer_list '}'                               { $$ = xml("p", $2, 0); }
  | '{' initializer_list ',' '}'                           { $$ = xml("p", $2, 0); }
  ;

initializer_list
  : initializer                                            { $$ = $1; }
  | initializer_list ',' initializer                       { $$ = xml_link($1, $3); }
  ;

statement
  : labeled_statement                                      { }
  | compound_statement                                     { }
  | expression_statement                                   { }
  | selection_statement                                    { }
  | iteration_statement                                    { }
  | jump_statement                                         { }
  | asm_statement                                          { }
  ;

labeled_statement
  : identifier ':' statement                               { }
  | CASE constant_expr ':' statement                       { }
  | DEFAULT ':' statement                                  { }
  ;

asm_statement
  : ASM                                                    { }
  ;

compound_statement
  : '{' '}'                                                { }
  | '{' statement_list '}'                                 { }
  ;

declaration_list
  : declaration                                            { $$ = $1; }
  | declaration_list declaration                           { $$ = xml_link($1, $2); }
  ;

statement_list
  : declaration                                            { }
  | statement                                              { }
  | statement_list statement                               { }
  | statement_list declaration                             { }
  ;

expression_statement
  : ';'                                                    { }
  | expr ';'                                               { }
  ;

selection_statement
  : IF '(' expr ')' statement                              { }
  | IF '(' expr ')' statement ELSE statement               { }
  | SWITCH '(' expr ')' statement                          { }
  ;

iteration_statement
  : WHILE '(' expr ')' statement                           { }
  | DO statement WHILE '(' expr ')' ';'                    { }
  | FOR '(' expression_statement expression_statement ')'  { }
  | FOR '(' expression_statement expression_statement expr ')' { }
  ;

jump_statement
  : GOTO identifier ';'                                    { }
  | CONTINUE ';'                                           { }
  | BREAK ';'                                              { }
  | RETURN ';'                                             { }
  | RETURN expr ';'                                        { }
  ;

translation_unit
  : external_definition                                    { $$ = $1; }
  | translation_unit external_definition                   { $$ = xml_link($1, $2); }
  ;

external_definition
  : function_definition                                    { $$ = $1; }
  | declaration                                            { $$ = $1; }
  ;

function_definition
  : declarator compound_statement                                          { $$ = xml("fdef", $1, 0); }
  | declarator declaration_list compound_statement                         { $$ = xml("fdef", $1, $2, 0); }
  | declaration_specifiers declarator compound_statement                   { $$ = xml("fdef", $1, $2, 0); }
  | declaration_specifiers declarator declaration_list compound_statement  { $$ = xml("fdef", $1, $2, $3, 0); }
  ;

typedeffed_name
  : TYPE_NAME                                              { $$ = ctoxml_cterm; }
  ;

identifier
  : IDENTIFIER                                             { $$ = ctoxml_cterm; }
  ;

file
  : translation_unit                                       { xml_push(cparser_root, $1); }
  |                                                        {  }
  ;

%%

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


XmlNode *parse_constant(void) {
  if(ctoxml_cterm[0] == '\'') {
    char txt[20];
    c_unescape(ctoxml_cterm);
    sprintf(txt, "%d", (int) (ctoxml_cterm[0]));
    XmlNode *rv = xml_text("c", txt);
    free(ctoxml_cterm);
    return rv;
  }
  char *attrib = 0;
  char *value = 0;
  if(ctoxml_cterm[0] != '0') {
    char lc = ctoxml_cterm[strlen(ctoxml_cterm) - 1];
    switch(lc) {
      case 'f': attrib="length"; value="f"; break;
      case 'F': attrib="length"; value="F"; break;
      case 'l': attrib="length"; value="l"; break;
      case 'L': attrib="length"; value="L"; break;
      case 'u': attrib="length"; value="u"; break;
      case 'U': attrib="length"; value="U"; break;
    }
    if(lc == 'f' || lc == 'F' || lc == 'l' || lc == 'L' || lc == 'u' || lc == 'U') {
      ctoxml_cterm[strlen(ctoxml_cterm) - 1] = 0;
    }
  }
  if(attrib) return xml_attrs(xml_text("c", ctoxml_cterm), attrib, value, 0);
  return xml_text("c", ctoxml_cterm);
}


static void deftypes(XmlNode *e, XmlNode *td) {
  // find all ident tags and insert them into typedefs container
  XmlNode *i = e;
  do {
    if(i->text && strcmp(i->tag, "id") == 0) {
      jsval tmp = JSVAL_TRUE;
      JS_SetProperty(cparser_jscx, JSVAL_TO_OBJECT(cparser_typedefs), i->text, &tmp);
    }
    if(i->inner && (strcmp(i->tag, "ptr") == 0 || strcmp(i->tag, "ix") == 0 || strcmp(i->tag, "p") == 0 || strcmp(i->tag, "fd") == 0)) {
      deftypes(i->inner, td);
    }
    i = i->next;
  } while(i != e);
}


void ctoxml_deftype_to_ident(XmlNode *e) {
  XmlNode *ptr = e->inner->last;
  XmlNode *ptrstop = e->inner;

  while(strcmp(ptrstop->tag, "const") == 0 || strcmp(ptrstop->tag, "typedef") == 0 || strcmp(ptrstop->tag, "volatile") == 0) {
    ptrstop = ptrstop->next;
  }

  if(ptrstop == ptr) return; // only one token, must be dt

  while((strcmp(ptr->tag, "id") == 0 || strcmp(ptr->tag, "ptr") == 0 || strcmp(ptr->tag, "ix") == 0) && ptr->last != ptrstop) {
    ptr = ptr->last;
  }

  if(strcmp(ptr->tag, "dt") == 0) ptr->tag = "id";
}


void ctoxml_typedef(XmlNode *e) {
  // check if identifier is replaced with a deftype, in which case it is a redefinition
  ctoxml_deftype_to_ident(e);
  deftypes(e->inner,e);
}
