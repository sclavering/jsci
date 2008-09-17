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

D			[0-9]
L			[a-zA-Z_]
H			[a-fA-F0-9]
E			[Ee][+-]?{D}+
FS			(f|F|l|L)
IS			(u|U|l|L)*

%{
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "c_gram.h"
#include "ctoxml.h"
static void count(void);
//extern int echo;
extern char *ctoxml_cterm;
extern char *ctoxml_cterm2;
extern int ctoxml_clineno;
extern char *ctoxml_filename;
static void preprocessor(void);
%}


%a 5000
%e 1500
%n 1000
%o 10000
%p 10000
ws								[ \f\v\t]

digit							[0-9]
hex								[0-9A-Fa-f]
letter							[A-Z_a-z]
simple_escape_sequence			(\\\'|\\\"|\\\?|\\\\|\\a|\\b|\\f|\\n|\\r|\\t|\\v)
octal_escape_sequence			(\\[0-7]|\\[0-7][0-7]|\\[0-7][0-7][0-7])
hexadecimal_escape_sequence		(\\x{hex}+)
escape_sequence					({simple_escape_sequence}|{octal_escape_sequence}|{hexadecimal_escape_sequence})
universal_character_name		(\\u{hex}{hex}{hex}{hex}|\\U{hex}{hex}{hex}{hex}{hex}{hex}{hex}{hex})
non_digit						({letter}|{universal_character_name})
identifier						({non_digit}({non_digit}|{digit})*)

character_lit					(L?\'([^\'\\\n]|\\.)*)
character_literal				({character_lit}\')

string_lit						(L?\"([^\"\\\n]|\\.)*)
string_literal					({string_lit}\")

pp_number						(\.?{digit}({digit}|{non_digit}|[eE][-+]|\.)*)


%%

"/*"			{ comment(); }
^.*\n							{ LEX_SAVE_LINE(yytext, yyleng); REJECT; }
^{ws}*"#".*						{ /* Throw away preprocessor lines - hopefully only #line and equivalent. */ }

{character_lit}\'				{ LEX_CHARACTER_TOKEN(yytext, yyleng-1); };
{character_lit}\\				{ ERRMSG("End of line assumed to terminate character with trailing escape.");
								  LEX_CHARACTER_TOKEN(yytext, yyleng-1); };
{character_lit}					{ ERRMSG("End of line assumed to terminate character.");
								  LEX_CHARACTER_TOKEN(yytext, yyleng); };

{string_lit}\"					{ LEX_STRING_TOKEN(yytext, yyleng-1); };
{string_lit}\\					{ ERRMSG("End of line assumed to terminate string with trailing escape.");
								  LEX_STRING_TOKEN(yytext, yyleng-1); };
{string_lit}					{ ERRMSG("End of line assumed to terminate string.");
								  LEX_STRING_TOKEN(yytext, yyleng); };

"asm"							{ LEX_STATIC_TOKEN(ASM); }
"auto"							{ LEX_STATIC_TOKEN(AUTO); }
"bool"							{ LEX_C_STATIC_TOKEN(BOOL); }
"break"							{ LEX_STATIC_TOKEN(BREAK); }
"case"							{ LEX_STATIC_TOKEN(CASE); }
"catch"							{ LEX_C_STATIC_TOKEN(CATCH); }
"char"							{ LEX_STATIC_TOKEN(CHAR); }
"class"							{ LEX_C_STATIC_TOKEN(CLASS); }
"const"							{ LEX_STATIC_TOKEN(CONST); }
"const_cast"					{ LEX_C_STATIC_TOKEN(CONST_CAST); }
"continue"						{ LEX_STATIC_TOKEN(CONTINUE); }
"default"						{ LEX_STATIC_TOKEN(DEFAULT); }
"delete"						{ LEX_C_STATIC_TOKEN(DELETE); }
"do"							{ LEX_STATIC_TOKEN(DO); }
"double"						{ LEX_STATIC_TOKEN(DOUBLE); }
"dynamic_cast"					{ LEX_C_STATIC_TOKEN(DYNAMIC_CAST); }
"else"							{ LEX_STATIC_TOKEN(ELSE); }
"enum"							{ LEX_STATIC_TOKEN(ENUM); }
"explicit"						{ LEX_C_STATIC_TOKEN(EXPLICIT); }
"export"						{ LEX_C_STATIC_TOKEN(EXPORT); }
"extern"						{ LEX_STATIC_TOKEN(EXTERN); }
"false"							{ LEX_C_STATIC_TOKEN(FALSE); }
"float"							{ LEX_STATIC_TOKEN(FLOAT); }
"for"							{ LEX_STATIC_TOKEN(FOR); }
"friend"						{ LEX_STATIC_TOKEN(FRIEND); }
"goto"							{ LEX_STATIC_TOKEN(GOTO); }
"if"							{ LEX_STATIC_TOKEN(IF); }
"inline"						{ LEX_C_STATIC_TOKEN(INLINE); }
"_inline"		{ count(); return(INLINE); }
"__inline"		{ count(); return(INLINE); }
"int"							{ LEX_STATIC_TOKEN(INT); }
"long"							{ LEX_STATIC_TOKEN(LONG); }
"mutable"						{ LEX_C_STATIC_TOKEN(MUTABLE); }
"namespace"						{ LEX_C_STATIC_TOKEN(NAMESPACE); }
"new"							{ LEX_C_STATIC_TOKEN(NEW); }
"operator"						{ LEX_C_STATIC_TOKEN(OPERATOR); }
"private"						{ LEX_C_STATIC_TOKEN(PRIVATE); }
"protected"						{ LEX_C_STATIC_TOKEN(PROTECTED); }
"public"						{ LEX_C_STATIC_TOKEN(PUBLIC); }
"register"						{ LEX_STATIC_TOKEN(REGISTER); }
"reinterpret_cast"				{ LEX_C_STATIC_TOKEN(REINTERPRET_CAST); }
"return"						{ LEX_STATIC_TOKEN(RETURN); }
"short"							{ LEX_STATIC_TOKEN(SHORT); }
"signed"						{ LEX_C_STATIC_TOKEN(SIGNED); }
"sizeof"						{ LEX_STATIC_TOKEN(SIZEOF); }
"static"						{ LEX_STATIC_TOKEN(STATIC); }
"static_cast"					{ LEX_C_STATIC_TOKEN(STATIC_CAST); }
"struct"						{ LEX_STATIC_TOKEN(STRUCT); }
"switch"						{ LEX_STATIC_TOKEN(SWITCH); }
"template"						{ LEX_C_STATIC_TOKEN(TEMPLATE); }
"this"							{ LEX_C_STATIC_TOKEN(THIS); }
"throw"							{ LEX_C_STATIC_TOKEN(THROW); }
"true"							{ LEX_C_STATIC_TOKEN(TRUE); }
"try"							{ LEX_C_STATIC_TOKEN(TRY); }
"typedef"						{ LEX_STATIC_TOKEN(TYPEDEF); }
"typeid"						{ LEX_C_STATIC_TOKEN(TYPEID); }
"typename"						{ LEX_C_STATIC_TOKEN(TYPENAME); }
"union"							{ LEX_STATIC_TOKEN(UNION); }
"unsigned"						{ LEX_STATIC_TOKEN(UNSIGNED); }
"using"							{ LEX_C_STATIC_TOKEN(USING); }
"virtual"						{ LEX_STATIC_TOKEN(VIRTUAL); }
"void"							{ LEX_STATIC_TOKEN(VOID); }
"volatile"						{ LEX_STATIC_TOKEN(VOLATILE); }
"wchar_t"						{ LEX_C_STATIC_TOKEN(WCHAR_T); }
"while"							{ LEX_STATIC_TOKEN(WHILE); }

"::"							{ LEX_C_STATIC_TOKEN(SCOPE); }
"..."							{ LEX_STATIC_TOKEN(ELLIPSIS); }
"<<"							{ LEX_STATIC_TOKEN(SHL); }
">>"							{ LEX_STATIC_TOKEN(SHR); }
"=="							{ LEX_STATIC_TOKEN(EQ); }
"!="							{ LEX_STATIC_TOKEN(NE); }
"<="							{ LEX_STATIC_TOKEN(LE); }
">="							{ LEX_STATIC_TOKEN(GE); }
"&&"							{ LEX_STATIC_TOKEN(LOG_AND); }
"||"							{ LEX_STATIC_TOKEN(LOG_OR); }
"++"							{ LEX_STATIC_TOKEN(INC); }
"--"							{ LEX_STATIC_TOKEN(DEC); }
"->*"							{ LEX_STATIC_TOKEN(ARROW_STAR); }
"->"							{ LEX_STATIC_TOKEN(ARROW); }
".*"							{ LEX_STATIC_TOKEN(DOT_STAR); }
"+="							{ LEX_STATIC_TOKEN(ASS_ADD); }
"-="							{ LEX_STATIC_TOKEN(ASS_SUB); }
"*="							{ LEX_STATIC_TOKEN(ASS_MUL); }
"/="							{ LEX_STATIC_TOKEN(ASS_DIV); }
"%="							{ LEX_STATIC_TOKEN(ASS_MOD); }
"^="							{ LEX_STATIC_TOKEN(ASS_XOR); }
"&="							{ LEX_STATIC_TOKEN(ASS_AND); }
"|="							{ LEX_STATIC_TOKEN(ASS_OR); }
">>="							{ LEX_STATIC_TOKEN(ASS_SHR); }
"<<="							{ LEX_STATIC_TOKEN(ASS_SHL); }

{pp_number}						{ LEX_NUMBER_TOKEN(yytext, yyleng); }

{identifier}					{ LEX_IDENTIFIER_TOKEN(yytext, yyleng); }

{escape_sequence}				|
{universal_character_name}		{ LEX_ESCAPED_TOKEN(yytext, yyleng); }

\n								|
{ws}+							{ /* Throw away whitespace */ }
.								{ LEX_ASCII_TOKEN(yytext[0]); }



"/*"			{ comment(); }
\/\/[^\n]*		{ count(); }
"auto"			{ count(); return(AUTO); }
"break"			{ count(); return(BREAK); }
"case"			{ count(); return(CASE); }
"char"			{ count(); return(CHAR); }
"const"			{ count(); return(CONST); }
"continue"		{ count(); return(CONTINUE); }
"default"		{ count(); return(DEFAULT); }
"do"			{ count(); return(DO); }
"double"		{ count(); return(DOUBLE); }
"else"			{ count(); return(ELSE); }
"enum"			{ count(); return(ENUM); }
"extern"		{ count(); return(EXTERN); }
"float"			{ count(); return(FLOAT); }
"for"			{ count(); return(FOR); }
"goto"			{ count(); return(GOTO); }
"if"			{ count(); return(IF); }
"int"			{ count(); return(INT); }
"long"			{ count(); return(LONG); }
"register"		{ count(); return(REGISTER); }
"return"		{ count(); return(RETURN); }
"short"			{ count(); return(SHORT); }
"signed"		{ count(); return(SIGNED); }
"sizeof"		{ count(); return(SIZEOF); }
"static"		{ count(); return(STATIC); }
"_inline"		{ count(); return(INLINE); }
"__inline"		{ count(); return(INLINE); }
"struct"		{ count(); return(STRUCT); }
"switch"		{ count(); return(SWITCH); }
"typedef"		{ count(); return(TYPEDEF); }
"union"			{ count(); return(UNION); }
"unsigned"		{ count(); return(UNSIGNED); }
"void"			{ count(); return(VOID); }
"__builtin_va_list"     { count(); return(VA); }
"__int64"		{ count(); return(INT64); }
"__stdcall"		{ count(); return(STDCALL); }
"__cdecl"		{ count(); return(CDECL); }
"_cdecl"		{ count(); return(CDECL); }
"volatile"		{ count(); return(VOLATILE); }
"while"			{ count(); return(WHILE); }
"__declspec"		{ count(); return(DECLSPEC); }
_?_asm[\t ]+\{[^}]*\}    { count(); ctoxml_cterm=strdup(ctoxml_ctext); return(ASM); }
_?_asm[^{}\n]*          { count(); ctoxml_cterm=strdup(ctoxml_ctext); return(ASM); }

{L}({L}|{D})*		{ count(); ctoxml_cterm=strdup(ctoxml_ctext); return(check_type()); }

0[xX]{H}+{IS}?		{ count(); ctoxml_cterm=strdup(ctoxml_ctext); return(CONSTANT); }
0{D}+{IS}?		{ count(); ctoxml_cterm=strdup(ctoxml_ctext); return(CONSTANT); }
{D}+{IS}?		{ count(); ctoxml_cterm=strdup(ctoxml_ctext); return(CONSTANT); }
'(\\.|[^\\'])+'		{ count(); ctoxml_cterm=strdup(ctoxml_ctext); return(CONSTANT); }

{D}+{E}{FS}?		{ count(); ctoxml_cterm=strdup(ctoxml_ctext); return(CONSTANT); }
{D}*"."{D}+({E})?{FS}?	{ count(); ctoxml_cterm=strdup(ctoxml_ctext); return(CONSTANT); }
{D}+"."{D}*({E})?{FS}?	{ count(); ctoxml_cterm=strdup(ctoxml_ctext); return(CONSTANT); }

\"(\\.|[^\\"])*\"	{ count(); ctoxml_cterm=strdup(ctoxml_ctext); return(STRING_LITERAL); }

"..."			{ count(); return(ELIPSIS); }
">>="			{ count(); return(RIGHT_ASSIGN); }
"<<="			{ count(); return(LEFT_ASSIGN); }
"+="			{ count(); return(ADD_ASSIGN); }
"-="			{ count(); return(SUB_ASSIGN); }
"*="			{ count(); return(MUL_ASSIGN); }
"/="			{ count(); return(DIV_ASSIGN); }
"%="			{ count(); return(MOD_ASSIGN); }
"&="			{ count(); return(AND_ASSIGN); }
"^="			{ count(); return(XOR_ASSIGN); }
"|="			{ count(); return(OR_ASSIGN); }
">>"			{ count(); return(RIGHT_OP); }
"<<"			{ count(); return(LEFT_OP); }
"++"			{ count(); return(INC_OP); }
"--"			{ count(); return(DEC_OP); }
"->"			{ count(); return(PTR_OP); }
"&&"			{ count(); return(AND_OP); }
"||"			{ count(); return(OR_OP); }
"<="			{ count(); return(LE_OP); }
">="			{ count(); return(GE_OP); }
"=="			{ count(); return(EQ_OP); }
"!="			{ count(); return(NE_OP); }
";"			{ count(); return(';'); }
"{"			{ count(); return('{'); }
"}"			{ count(); return('}'); }
","			{ count(); return(','); }
":"			{ count(); return(':'); }
"="			{ count(); return('='); }
"("			{ count(); return('('); }
")"			{ count(); return(')'); }
"["			{ count(); return('['); }
"]"			{ count(); return(']'); }
"."			{ count(); return('.'); }
"&"			{ count(); return('&'); }
"!"			{ count(); return('!'); }
"~"			{ count(); return('~'); }
"-"			{ count(); return('-'); }
"+"			{ count(); return('+'); }
"*"			{ count(); return('*'); }
"/"			{ count(); return('/'); }
"%"			{ count(); return('%'); }
"<"			{ count(); return('<'); }
">"			{ count(); return('>'); }
"^"			{ count(); return('^'); }
"|"			{ count(); return('|'); }
"?"			{ count(); return('?'); }

[ \t\v\f\n]		{ count(); }

\#.*                    { count(); preprocessor(); }

.			{ /* ignore bad characters */ }

%%

ctoxml_cwrap()
{
	return(1);
}

comment()
{
	char c, c1;

loop:
	while ((c = input()) != '*' && c != 0)
//		putchar(c);
;

	if ((c1 = input()) != '/' && c != 0)
	{
		yyunput(c1, ctoxml_ctext);
		goto loop;
	}

//	if (c != 0)
//		putchar(c1);
}

void preprocessor(void) {
  char *directive=ctoxml_ctext+1;
  directive+=strspn(directive," \t");

  if (strncmp(directive,"define",6)==0) {
    int spclen=strspn(directive+6," \t"); // Find start of identifier
    if (spclen && directive[6+spclen]) {
      char *startparen;
      char *endparen;
      char *macrovalue;
      struct Xml *ident_xml;
      struct Xml *def;
      char *macroname=strdup(directive+spclen+6); // Copy identifier and rest of define

      startparen=strchr(macroname,'(');
      if (startparen && strcspn(macroname," \t")>startparen-macroname) {
	  endparen=strchr(macroname,')');
          macrovalue=endparen+1;
	  macrovalue+=strspn(macrovalue," \t");
	  macrovalue=strdup(macrovalue);
	  *startparen=0; // Terminate identifier
      } else {
          char *identend=macroname+strcspn(macroname," \t"); // Find end of identifier
          startparen=0;
	  macrovalue=identend+strspn(identend," \t");
	  macrovalue=strdup(macrovalue);
          *identend=0; // Terminate identifier;
      }

      def=xml("define",0,
	      (ident_xml=xml_text("id",0,macroname)),
	      xml_text("v",0,macrovalue),0);

      if (startparen) {
        char *ident=startparen+1;
	while (ident<endparen) {
	    char *identend=ident+strcspn(ident," \t,)");
            *identend=0;
	    xml_link(ident_xml,xml_text("pm",0,strdup(ident)));
            ident=identend+1;
            ident+=strspn(ident," \t,");
        }
      }
     
      xml_print(def);
      xml_free(def);
//      stringhash_insert(ctoxml_defines, defname, defval);
    }
  } else if (strncmp(directive,"pragma",6)==0) {
    struct Xml *pragma;

    if (directive[6])
      ctoxml_cterm=strdup(directive+7);
    else
      ctoxml_cterm=strdup("");
    
    pragma = xml_text("pragma",0,ctoxml_cterm);
    xml_print(pragma);
    xml_free(pragma);

  } else if (atoi(ctoxml_ctext+1) || strncmp(directive, "line ",5)==0) {
    char *filenameend;
    char *tmpfilename=strchr(ctoxml_ctext, ' ');
    char *lineno;
    struct Xml *linexml;

    if (!(tmpfilename && tmpfilename[0])) return;
    ctoxml_clineno=atoi(tmpfilename+1);
    
    tmpfilename=strchr(tmpfilename+1, '\"');
    if (!(tmpfilename && tmpfilename[0])) return;
    
    tmpfilename++;

    ctoxml_filename=strdup(tmpfilename);
    filenameend=strchr(ctoxml_filename+1, '\"');
    if (filenameend) *filenameend=0;
    strncpy(ctoxml_filename_errmsg,ctoxml_filename,80);

    lineno=malloc(10);
    sprintf(lineno,"%d",ctoxml_clineno);
    linexml = xml("line","file",ctoxml_filename,"line",lineno,0,0);
    xml_print(linexml);
    xml_free(linexml);
    ctoxml_clineno--;
  }
}

int ctoxml_ccolumn = 0;
int ctoxml_cfilepos = 0;

void count(void)
{
  int i;
  for (i = 0; ctoxml_ctext[i] != '\0'; i++) {
    if (ctoxml_ctext[i] == '\n') {
      ctoxml_ccolumn = 0;
      ctoxml_clineno ++;
    }
    else if (ctoxml_ctext[i] == '\t') {
      ctoxml_ccolumn = ((ctoxml_ccolumn+7)/8)*8;
    }
    ctoxml_cfilepos++;
  }
      //	if (echo) ECHO;
}

char *ctoxml_cterm=0;
char *ctoxml_filename=0;
char ctoxml_filename_errmsg[80];
char *ctoxml_cterm2=0;
int ctoxml_clineno;

int check_type()
{
	if (stringhash_search(ctoxml_typedefs,ctoxml_ctext)) {
		return(TYPE_NAME);
	}
	return(IDENTIFIER);
}
