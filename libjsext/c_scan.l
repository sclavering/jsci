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

D                       [0-9]
L                       [a-zA-Z_]
H                       [a-fA-F0-9]
E                       [Ee][+-]?{D}+
FS                      (f|F|l|L)
IS                      (u|U|l|L)*

%{
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "strbuf.h"
#include "c_gram.h"
#include "ctoxml.h"
static void count(void);
//extern int echo;
extern char *ctoxml_cterm;
extern char *ctoxml_cterm2;
extern int ctoxml_clineno;
extern char *ctoxml_filename;
static void preprocessor(void);

int check_type(void);
void ctoxml_linecomment(char *comment);
void ctoxml_comment(char **at);

%}

%%
"/*"                    { ctoxml_comment(0); }
\/\/[^\n]*              { count(); ctoxml_linecomment(0); }
"auto"                  { count(); return AUTO; }
"break"                 { count(); return BREAK; }
"case"                  { count(); return CASE; }
"char"                  { count(); return CHAR; }
"const"                 { count(); return CONST; }
"continue"              { count(); return CONTINUE; }
"default"               { count(); return DEFAULT; }
"do"                    { count(); return DO; }
"double"                { count(); return DOUBLE; }
"else"                  { count(); return ELSE; }
"enum"                  { count(); return ENUM; }
"extern"                { count(); return EXTERN; }
"float"                 { count(); return FLOAT; }
"for"                   { count(); return FOR; }
"goto"                  { count(); return GOTO; }
"if"                    { count(); return IF; }
"int"                   { count(); return INT; }
"long"                  { count(); return LONG; }
"register"              { count(); return REGISTER; }
"return"                { count(); return RETURN; }
"short"                 { count(); return SHORT; }
"signed"                { count(); return SIGNED; }
"sizeof"                { count(); return SIZEOF; }
"static"                { count(); return STATIC; }
"inline"                { count(); return INLINE; }
"_inline"               { count(); return INLINE; }
"__inline"              { count(); return INLINE; }
"struct"                { count(); return STRUCT; }
"switch"                { count(); return SWITCH; }
"typedef"               { count(); return TYPEDEF; }
"union"                 { count(); return UNION; }
"unsigned"              { count(); return UNSIGNED; }
"void"                  { count(); return VOID; }
"__builtin_va_list"     { count(); return VA; }
"__int64"               { count(); return INT64; }
"__stdcall"             { count(); return STDCALL; }
"__cdecl"               { count(); return CDECL; }
"_cdecl"                { count(); return CDECL; }
"volatile"              { count(); return VOLATILE; }
"while"                 { count(); return WHILE; }
"__declspec"            { count(); return DECLSPEC; }
_?_asm[\t ]+\{[^}]*\}   { count(); ctoxml_cterm=strdup(ctoxml_ctext); return ASM; }
_?_asm[^{}\n]*          { count(); ctoxml_cterm=strdup(ctoxml_ctext); return ASM; }

{L}({L}|{D})*           { count(); ctoxml_cterm=strdup(ctoxml_ctext); return check_type(); }

0[xX]{H}+{IS}?          { count(); ctoxml_cterm=strdup(ctoxml_ctext); return CONSTANT; }
0{D}+{IS}?              { count(); ctoxml_cterm=strdup(ctoxml_ctext); return CONSTANT; }
{D}+{IS}?               { count(); ctoxml_cterm=strdup(ctoxml_ctext); return CONSTANT; }
'(\\.|[^\\'])+'         { count(); ctoxml_cterm=strdup(ctoxml_ctext); return CONSTANT; }

{D}+{E}{FS}?            { count(); ctoxml_cterm=strdup(ctoxml_ctext); return CONSTANT; }
{D}*"."{D}+({E})?{FS}?  { count(); ctoxml_cterm=strdup(ctoxml_ctext); return CONSTANT; }
{D}+"."{D}*({E})?{FS}?  { count(); ctoxml_cterm=strdup(ctoxml_ctext); return CONSTANT; }

\"(\\.|[^\\"])*\"       { count(); ctoxml_cterm=strdup(ctoxml_ctext); return STRING_LITERAL; }
L\"(\\.|[^\\"])*\"      { count(); ctoxml_cterm=strdup(ctoxml_ctext+1); return STRING_LITERAL; }

"..."                   { count(); return ELIPSIS; }
">>="                   { count(); return RIGHT_ASSIGN; }
"<<="                   { count(); return LEFT_ASSIGN; }
"+="                    { count(); return ADD_ASSIGN; }
"-="                    { count(); return SUB_ASSIGN; }
"*="                    { count(); return MUL_ASSIGN; }
"/="                    { count(); return DIV_ASSIGN; }
"%="                    { count(); return MOD_ASSIGN; }
"&="                    { count(); return AND_ASSIGN; }
"^="                    { count(); return XOR_ASSIGN; }
"|="                    { count(); return OR_ASSIGN; }
">>"                    { count(); return RIGHT_OP; }
"<<"                    { count(); return LEFT_OP; }
"++"                    { count(); return INC_OP; }
"--"                    { count(); return DEC_OP; }
"->"                    { count(); return PTR_OP; }
"&&"                    { count(); return AND_OP; }
"||"                    { count(); return OR_OP; }
"<="                    { count(); return LE_OP; }
">="                    { count(); return GE_OP; }
"=="                    { count(); return EQ_OP; }
"!="                    { count(); return NE_OP; }
";"                     { count(); return ';'; }
"{"                     { count(); return '{'; }
"}"                     { count(); return '}'; }
","                     { count(); return ','; }
":"                     { count(); return ':'; }
"="                     { count(); return '='; }
"("                     { count(); return '('; }
")"                     { count(); return ')'; }
"["                     { count(); return '['; }
"]"                     { count(); return ']'; }
"."                     { count(); return '.'; }
"&"                     { count(); return '&'; }
"!"                     { count(); return '!'; }
"~"                     { count(); return '~'; }
"-"                     { count(); return '-'; }
"+"                     { count(); return '+'; }
"*"                     { count(); return '*'; }
"/"                     { count(); return '/'; }
"%"                     { count(); return '%'; }
"<"                     { count(); return '<'; }
">"                     { count(); return '>'; }
"^"                     { count(); return '^'; }
"|"                     { count(); return '|'; }
"?"                     { count(); return '?'; }

[ \t\v\f\n]             { count(); }

\#.*                    { count(); preprocessor(); }

.                       { /* ignore bad characters */ }

%%

ctoxml_cwrap() {
  return 1;
}

int ctoxml_ccolumn = 0;
int ctoxml_cfilepos = 0;

void ctoxml_comment(char **at) {
  static struct strbuf *dstr = 0;
  struct Xml *commxml;
  int state = 0;
  if(dstr == 0) dstr = strbuf_new();

  if(!at) {
    ctoxml_ccolumn += 2;
    ctoxml_cfilepos += 2;
  }

  while(state != 2) {
    char c;
    if(at) c = *((*at)++);
    else c = input();
    if(c == 0) {
      if(at == 0) break;
      at = 0;
      c = input();
      if(c == 0) break;
    }

    strbuf_catchar(dstr, c);

    if(!at) {
      if(c == '\n') {
        ctoxml_ccolumn = 0;
        ++ctoxml_clineno;
      } else if(c == '\t') {
        ctoxml_ccolumn = ((ctoxml_ccolumn + 7) / 8) * 8;
      } else {
        ++ctoxml_ccolumn;
      }
    }

    switch(state) {
      case 0:
        if(c == '*') state = 1;
        break;
      case 1:
        if(c == '/') state = 2;
        else if(c != '*') state = 0;
        break;
    }
  }
  if(state == 2) dstr->buf[dstr->len - 2] = 0;

  commxml = xml_text("comment", "type", strdup("block"), NULL, strdup(dstr->buf));
  xml_print(commxml);
  xml_free(commxml);

  strbuf_clear(dstr);
}


void ctoxml_linecomment(char *comment) {
  struct Xml *commxml;
  if(!comment) comment = ctoxml_ctext + 2;
  commxml = xml_text("comment", "type", strdup("line"), NULL, strdup(comment));
  xml_print(commxml);
  xml_free(commxml);
}


void preprocessor(void) {
  char *directive;
  char *comment;

  comment = strstr(ctoxml_ctext, "/*");
  if(comment) {
    *comment = 0;
    comment += 2;
    ctoxml_comment(&comment);
  } else {
    comment = ctoxml_ctext;
  }

  comment = strstr(comment, "//");
  if (comment) {
    *comment = 0;
    comment += 2;
    ctoxml_linecomment(comment);
  }

  directive = ctoxml_ctext+1;
  directive += strspn(directive," \t");

  if(strncmp(directive, "define", 6) == 0) {
    int spclen = strspn(directive + 6, " \t"); // Find start of identifier
    if(spclen && directive[6 + spclen]) {
      char *startparen;
      char *endparen;
      char *macrovalue;
      struct Xml *ident_xml;
      struct Xml *def;
      char *macroname = strdup(directive + spclen + 6); // Copy identifier and rest of define

      startparen = strchr(macroname,'(');
      if(startparen && strcspn(macroname, " \t") > startparen - macroname) {
        endparen = strchr(macroname, ')');
        macrovalue = endparen + 1;
        macrovalue += strspn(macrovalue, " \t");
        macrovalue = strdup(macrovalue);
        *startparen = 0; // Terminate identifier
      } else {
        char *identend = macroname + strcspn(macroname, " \t"); // Find end of identifier
        startparen = 0;
        macrovalue = identend + strspn(identend, " \t");
        macrovalue = strdup(macrovalue);
        *identend = 0; // Terminate identifier;
      }

      ident_xml = xml_text("id", NULL, macroname);
      def = xml("define", NULL, ident_xml, xml_text("v", NULL, macrovalue), NULL);

      if(startparen) {
        char *ident = startparen + 1;
        if(ident >= endparen) xml_link(ident_xml, xml_text("pm", NULL, strdup("")));
        while(ident < endparen) {
          char *identend = ident + strcspn(ident, " \t,)");
          *identend = 0;
          xml_link(ident_xml, xml_text("pm", NULL, strdup(ident)));
          ident = identend + 1;
          ident += strspn(ident, " \t,");
        }
      }

      xml_print(def);
      xml_free(def);
    }
  }
  else if(strncmp(directive, "pragma", 6) == 0) {
    struct Xml *pragma;
    ctoxml_cterm = directive[6] ? strdup(directive + 7) : strdup("");
    pragma = xml_text("pragma", NULL, ctoxml_cterm);
    xml_print(pragma);
    xml_free(pragma);
  }
  else if(atoi(ctoxml_ctext + 1) || strncmp(directive, "line ", 5) == 0) {
    char *filenameend;
    char *tmpfilename = strchr(ctoxml_ctext, ' ');
    char *lineno;
    struct Xml *linexml;

    if(!(tmpfilename && tmpfilename[0])) return;
    ctoxml_clineno = atoi(tmpfilename + 1);

    tmpfilename = strchr(tmpfilename + 1, '\"');
    if(!(tmpfilename && tmpfilename[0])) return;

    tmpfilename++;

    ctoxml_filename = strdup(tmpfilename);
    filenameend = strchr(ctoxml_filename + 1, '\"');
    if(filenameend) *filenameend = 0;
    strncpy(ctoxml_filename_errmsg, ctoxml_filename, 80);

    lineno = malloc(10);
    sprintf(lineno, "%d", ctoxml_clineno);
    linexml = xml("line", "file", ctoxml_filename, "line", lineno, NULL, NULL);
    xml_print(linexml);
    xml_free(linexml);
    ctoxml_clineno--;
  }
}


void count(void) {
  int i;
  for (i = 0; ctoxml_ctext[i] != '\0'; i++) {
    if (ctoxml_ctext[i] == '\n') {
      ctoxml_ccolumn = 0;
      ctoxml_clineno ++;
    } else if (ctoxml_ctext[i] == '\t') {
      ctoxml_ccolumn = ((ctoxml_ccolumn+7)/8)*8;
    } else
      ctoxml_ccolumn ++;
    ctoxml_cfilepos++;
  }
}

char *ctoxml_cterm=0;
char *ctoxml_filename=0;
char ctoxml_filename_errmsg[80];
char *ctoxml_cterm2=0;
int ctoxml_clineno;

int check_type(void) {
  if(stringhash_search(ctoxml_typedefs, ctoxml_ctext)) return TYPE_NAME;
  return IDENTIFIER;
}
