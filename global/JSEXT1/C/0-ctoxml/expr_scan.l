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
#include "expr_gram.h"
static void count(void);
//extern int echo;
extern char *cpp_exprterm;
extern int cpp_exprlineno;
extern char *cpp_exprfilename;
static int preprocessor();
%}

%%

{L}({L}|{D})*		{ count(); cpp_exprterm=strdup(cpp_exprtext); return(IDENTIFIER); }

0[xX]{H}+{IS}?		{ count(); cpp_exprterm=strdup(cpp_exprtext); return(PP_NUMBER); }
0{D}+{IS}?		{ count(); cpp_exprterm=strdup(cpp_exprtext); return(PP_NUMBER); }
{D}+{IS}?		{ count(); cpp_exprterm=strdup(cpp_exprtext); return(PP_NUMBER); }
{D}+{E}{FS}?		{ count(); cpp_exprterm=strdup(cpp_exprtext); return(PP_NUMBER); }
{D}*"."{D}+({E})?{FS}?	{ count(); cpp_exprterm=strdup(cpp_exprtext); return(PP_NUMBER); }
{D}+"."{D}*({E})?{FS}?	{ count(); cpp_exprterm=strdup(cpp_exprtext); return(PP_NUMBER); }

'(\\.|[^\\'])+'		{ count(); cpp_exprterm=strdup(cpp_exprtext); return(CHARACTER_CONSTANT); }

\"(\\.|[^\\"])*\"	{ count(); cpp_exprterm=strdup(cpp_exprtext); return(STRING_LITERAL); }

[ \t\v\f]		{ count(); }

">>"			{ count(); return(RIGHT_OP); }
"<<"			{ count(); return(LEFT_OP); }
"&&"			{ count(); return(AND_OP); }
"||"			{ count(); return(OR_OP); }
"<="			{ count(); return(LE_OP); }
">="			{ count(); return(GE_OP); }
"=="			{ count(); return(EQ_OP); }
"!="			{ count(); return(NE_OP); }
":"			{ count(); return(':'); }
"("			{ count(); return('('); }
")"			{ count(); return(')'); }
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
"@"			{ count(); return('@'); }

%%

cpp_exprwrap()
{
	return(1);
}

int cpp_exprcolumn = 0;
int cpp_exprfilepos = 0;

void count(void)
{
  int i;
  for (i = 0; cpp_exprtext[i] != '\0'; i++) {
    if (cpp_exprtext[i] == '\n') {
      cpp_exprcolumn = 0;
      cpp_exprlineno ++;
    }
    else if (cpp_exprtext[i] == '\t') {
      cpp_exprcolumn = ((cpp_exprcolumn+7)/8)*8;
    }
    cpp_exprfilepos++;
  }
      //	if (echo) ECHO;
}

char *cpp_exprterm;
char *cpp_exprfilename;
int cpp_exprlineno;

