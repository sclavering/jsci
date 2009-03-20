#include <stdio.h>
#include <string.h>
#include "stringhash.h"
#include "strbuf.h"
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <stdlib.h>
#include "expr_scan.h"
  
static struct strbuf *cpp_STDOUT;
static char *cpp_error;
#define PUTS(x) strbuf_cat(cpp_STDOUT,x)

static struct strbuf *comments;

struct inputfile {
  char *buf;
  char *ptr;
  int lineno;
  char *filename;
  char *path;
};

char **quotepath=0;//[]={"",0};
char **stdpath=0;
char *include=0;
char *include_add;
int added_paths=0;

char *builtin[]={"stddef.h",
		 "#ifndef _STDDEF_H\n"
		 "#define _STDDEF_H\n"
		 "\n"
		 "typedef unsigned long size_t;\n"
		 "typedef short wchar_t;\n"
		 "typedef int ptrdiff_t;\n"
     "#define inline _inline\n"
     "#define NULL 0\n"
		 "\n"
		 "#endif\n",

		 "stdarg.h",
		 "#ifndef _STDARG_H\n"
		 "#define _STDARG_H\n"
		 "\n"
		 "typedef __builtin_va_list __gnuc_va_list;\n"
		 "typedef __builtin_va_list va_list;\n"
		 "\n"
		 "#endif\n",

		 "float.h",
   	 "",

		 0};




struct strbuf *buf;

struct inputfile *file;
int *IF;
struct inputfile *curfile;
int *curIF;

stringhash *macros;

#define NL 13
#define LF 10


static void addpath(char *path);

// Report error

void cpperror(char *str) {
  if (curfile && curfile->filename) {
    cpp_error=malloc(strlen(curfile->filename)+80);
	sprintf(cpp_error,"%s:%d:1: %s", curfile->filename, curfile->lineno, str);
  } else {
	cpp_error=strdup(str);
  }
}

// Read one character from buf

#define cpp_getchar() (*buf->ptr++)


char *spc=" \t";
struct strbuf *tok;
//char trigraph[256];
char ws[256];
char horiz_ws[256];
char alnum[256];
char alpha[256];
signed char paren[256];

// Read any whitespace characters from buf into tok

void read_horiz_ws() {
  char c;

  char *start=buf->ptr, *at=buf->ptr;
  while ((c=*at) && horiz_ws[c]) {
    at++;
  }
  strbuf_ncpy(tok,start,at-start);
  buf->ptr=at;
}

// Read a string quoted by " or ', as specified by the argument into tok
// assumes that first quote is already in tok.

void read_string(char quote) {
  for (;;) {
    int d=cpp_getchar();
    if (d==0) {
      buf->ptr--;
      return;
    }
    if (d=='\\') {
      strbuf_catchar(tok,d);
      d=cpp_getchar();
      if (d==0) {
	buf->ptr--;
	return;
      }
    } else if (d==quote) {
      strbuf_catchar(tok,d);
      return;
    }
    strbuf_catchar(tok,d);
  }
}

// Read one token from buf to tok. May be whitespace, text, a string or something else.
// Does not necessarily correspond to C tokens. Example: >>= is three tokens here, one in C.

__inline void read_token_raw() {
  char c;
  char alnum_c;
  char horiz_ws_c;

  char d;
  char *at;

  char *start=buf->ptr;
  //  c=cpp_getchar();
  c=*(buf->ptr++);
  if (c==0) {
    strbuf_clear(tok);
    buf->ptr--;
    return;
  }

  //  strbuf_catchar(tok,c);

  if (c=='"') {
    strbuf_cpy(tok,"\"");
    read_string('"');
    return;
  }
  if (c=='\'') {
    strbuf_cpy(tok,"'");
    read_string('\'');
    return;
  }

  if (c=='#') { // Accept ## as token, and #hello as token
    c=cpp_getchar();
    if (c==0) {
      strbuf_cpy(tok,"#");
      buf->ptr--;
      return;
    }
    //    strbuf_catchar(tok,c);
    if (c=='#') {
      strbuf_cpy(tok,"##");
      return;
    }
  }

  alnum_c=alnum[(unsigned char)c];
  horiz_ws_c=horiz_ws[(unsigned char)c];

  at=buf->ptr;
  while ((d=*at) &&
	 d!='#' &&
	 alnum[(unsigned char)d] == alnum_c &&
	 horiz_ws[(unsigned char)d] == horiz_ws_c) at++;

  strbuf_ncpy(tok,start,at-start);
  buf->ptr=at;

}

struct paramlist {
  char *paramname;
  struct paramlist *next;
};

struct macro {
  char *ident;
  struct paramlist *params;
  char *def;
};

void read_token_macro();

// Read tokens in buffer, doing macro replacement.
// Put output back into buffer. Contains @ before recursive tokens

void expand_macro() {
  struct strbuf *macro_p=strbuf_new();
  
  for (;;) {
    read_token_macro();
    if (cpp_error) break;
    if (tok->len==0) break;
    if (strcmp(tok->buf,"##")==0) { // cat
      char *end;

      // Read white space after ##
      read_horiz_ws();
      end=macro_p->buf+macro_p->len;
      // Take back white space before ##
      while (end>macro_p->buf && horiz_ws[(unsigned char)end[-1]])
	end--;
      *end=0;
      macro_p->len=end-macro_p->buf;
      read_token_macro();
      if (tok->buf[0]=='@') {
	strbuf_cat(macro_p,tok->buf+1);
      } else {
	strbuf_cat(macro_p,tok->buf);
      }
    } else {
      strbuf_cat(macro_p,tok->buf);
    }
  }

  strbuf_cpy(buf,macro_p->buf);
  strbuf_free(macro_p);
}

void read_token();

// Read tokens in buffer, doing macro replacement.
// Remove @ before tokens

void expand() {
  struct strbuf *macro_p=strbuf_new();

  expand_macro();
  
  for (;;) {
    read_token();
    if (tok->len==0) break;
    if (tok->buf[0]=='@') {
      strbuf_cat(macro_p,tok->buf+1);
    } else {
      strbuf_cat(macro_p,tok->buf);
    }
  }
  
  strbuf_cpy(buf,macro_p->buf);
  strbuf_free(macro_p);
}

void read_textline();

// Read anything that may count as a parameter to a macro
// Return ')' if last parameter, ',' if not last, EOF if end of line encountered.

int read_param() {
  int parencount=0;
  int inquote=0;
  char c=0;

  read_horiz_ws();
  strbuf_clear(tok);

  for (;;) {
    // Loop until end of parameter

    for (;;) { // read more lines if necessary
      char *p;
      c=cpp_getchar();
      if (c!=0) break;
      buf->ptr--;
      strbuf_catchar(tok,' ');
      p=strdup(tok->buf);
      read_textline();
      strbuf_cpy(tok,p);
      strbuf_catchar(tok,' ');
      free(p);
    }

    // Detect end of parameter
    if (parencount==0 && inquote==0) {
      if (c==',' || c==')') break;
    }

    if (inquote==0) {
      // Detect beginning/end of parens
      parencount+=paren[(unsigned char)c];
      // Detect beginning of string
      if (c=='"') inquote='"';
      if (c=='\'') inquote='\'';
    } else {

      // Detect end of string
      if (c==inquote) inquote=0;
    }

    // pass backslashed letters through
    if (c=='\\') {
      strbuf_catchar(tok,c);
      c=cpp_getchar();
      if (c==0) {
	buf->ptr--;
	break;
      }
    }
    strbuf_catchar(tok,c);
  }
  return c;
}

// Read next token, doing macro replacement. In case of recursion, some tokens will have
// a @ inserted before them. The token is read from the buffer and written to tok.

void read_token_macro() {
  for (;;) {
    struct macro *thismacro=0;
    char *lineend;
    struct paramlist *param;
    struct macro *newmacro;
    struct macro *oldmacro;

    read_token_raw();
    if (macros && alpha[(unsigned char)tok->buf[0]]) thismacro=stringhash_search(macros, tok->buf);
    if (!thismacro) break;

    // Define pseudomacros for parameter replacement
    
    param=thismacro->params;

    if (param) {
      stringhash *tmpmacros;
      stringhash *parammacros;
      stringhash *selfmacros;
      struct macro *thismacrocopy;
      struct strbuf *tmpbuf;
      struct macro *newmacro;
      struct macro *quotemacro;

      read_horiz_ws();
      if (*buf->ptr!='(') {
//	cpperror("Macro needs parameters");
		  // should not be expanded
	    strbuf_cpy(tok,thismacro->ident)
	return;
      }
	  cpp_getchar();

      parammacros=stringhash_new(); // Contains paramname1->paramvalue1 etc
      selfmacros=stringhash_new();  // Contains only self macro to expand params
      // all other macros will be expanded later
      thismacrocopy=(struct macro *)malloc(sizeof(struct macro));
      memcpy(thismacrocopy,thismacro,sizeof(struct macro));
      stringhash_insert(selfmacros,strdup(thismacro->ident),thismacrocopy);

      tmpmacros=macros;

      macros=selfmacros;

      while (param) {
	int pret=read_param();
	if (pret==EOF) {
	  cpperror("Syntax error in macro parameter list");
	  break;
	} else if ((pret==')') != (param->next==0)) {
//	  cpperror("Wrong number of macro parameters"); // it's okay, just a warning
	  break;
	}

	tmpbuf=buf;
	buf=strbuf_new();
	strbuf_cpy(buf,tok->buf);
	expand_macro(); // Run macro on its own parameters - no danger of recursion
	
	newmacro=(struct macro *)calloc(sizeof(struct macro)+buf->len+1,1);
	newmacro->ident=strdup(param->paramname);
	newmacro->def=(char *)(newmacro+1);
	strcpy(newmacro->def,buf->buf);
	stringhash_insert(parammacros,newmacro->ident,newmacro);

	quotemacro=(struct macro *)calloc(sizeof(struct macro)+buf->len+3,1);
	quotemacro->ident=(char *)malloc(strlen(param->paramname)+2);
	quotemacro->ident[0]='#';
	strcpy(quotemacro->ident+1,param->paramname);
	quotemacro->def=(char *)(quotemacro+1);
	strcpy(quotemacro->def+1,buf->buf);
	quotemacro->def[0]='"';
	quotemacro->def[buf->len+1]='"';
	quotemacro->def[buf->len+2]=0;
	stringhash_insert(parammacros,quotemacro->ident,quotemacro);
	
	strbuf_free(buf);
	buf=tmpbuf;

	param=param->next;
      }
      
      stringhash_destroy(selfmacros);

      lineend=strdup(buf->ptr); // Remember tail of input buffer
      strbuf_cpy(buf,thismacro->def); // Copy macro to input buffer
      
      // Replace parameters
      
      macros=parammacros;
      
      expand_macro();
      
      // Restore proper macro table
      
      stringhash_destroy(parammacros);
      macros=tmpmacros;
    } else {
      lineend=strdup(buf->ptr); // Remember tail of input buffer
      strbuf_cpy(buf,thismacro->def); // Copy macro to input buffer
    }

    // Make a macro named the same as thyself, defined to "@thyself" to avoid recursion

    newmacro=(struct macro *)calloc(sizeof(struct macro)+strlen(thismacro->ident)+2,1);
    newmacro->ident=strdup(thismacro->ident);
    newmacro->def=(char *)(newmacro+1);
    sprintf(newmacro->def,"@%s",thismacro->ident);

    // Backup old macro

    oldmacro=(struct macro *)malloc(sizeof(struct macro));
    memcpy(oldmacro,thismacro,sizeof(struct macro));
    oldmacro->ident=strdup(thismacro->ident);

    // Replace old macro

    stringhash_remove(macros,oldmacro->ident);
    stringhash_insert(macros,newmacro->ident,newmacro);

    expand_macro();

    // Restore old macro

    stringhash_remove(macros,oldmacro->ident);
    stringhash_insert(macros,oldmacro->ident,oldmacro);

    // Restore line end

    strbuf_cat(buf,lineend);
    free(lineend);

  }
}

// Print current line number and file name

void line_number() {
  char lineno[10];
  strbuf_cat(cpp_STDOUT, "#line ");
  sprintf(lineno,"%d",curfile->lineno+1);
  strbuf_cat(cpp_STDOUT, lineno);
  strbuf_cat(cpp_STDOUT, " \"");
  strbuf_cat(cpp_STDOUT, curfile->filename);
  strbuf_cat(cpp_STDOUT, "\"");
}

// Normalize line endings to LF
// Remove \LF sequences, moving LF to next linefeed.
// Add LF at end if missing
// 

void cleanup_lines(char *file) {

  // Normalize line endings to LF

  char *in=file;
  char *out=file;
  int swallowed=0;

  while (*in) {
	  if (*in==NL) {
		*in=LF;
//		if (in[1]==LF) in[1]=' ';
	  }
	++in;
  }

  // Remove \LF sequences
  in=file;
  out=file;
  while (*in) {
    if (in[0]=='\\' && in[1]==LF) {
      in+=2;
      swallowed++;
    } else if (in[0]==LF) {
      int i;
      for (i=0; i<=swallowed; i++)
	*(out++)=LF;
      swallowed=0;
      in++;
    } else {
      *(out++)=*(in++);
    }
  }

  // Add LF at end if missing
  if (in[-1]!=LF) *(out++)=LF;

  *out=0;
}

// Read one line of text.

void read_line_raw() {
  char *n=strchr(curfile->ptr,LF);
  curfile->lineno++;
  if (n) {
    strbuf_ncpy(buf,curfile->ptr,n-curfile->ptr);
    curfile->ptr=n+1;
  } else { // EOF
    strbuf_cpy(buf,curfile->ptr);
    free(curfile->buf);
    free(curfile->filename);
    curfile--;
    if (curfile>=file) {
      line_number();
    }
    return;
  }
}

int comment=0;


// Read one line of text, storing comments in 'comments'

void read_line() {
  char c;

  read_line_raw();

  while ((c=*buf->ptr)) {
    char d=buf->ptr[1];

    if (comment==2) { // Inside block comment
      // Find end of block comment
      char *end=strstr(buf->ptr,"*/");
      if (end) {
	end+=2;
	comment=0;
	strbuf_ncat(comments,buf->ptr, end-buf->ptr);
	memset(buf->ptr,' ',end-buf->ptr);
	buf->ptr=end;
      } else {
	strbuf_cat(comments,buf->ptr);
	memset(buf->ptr,' ',buf->buf + buf->len - buf->ptr);
	break;
      }
    } else {
      if (c=='/' && d=='*') { // Start of block comment
	comment=2;
      } else if (c=='/' && d=='/') { // Line comment
	strbuf_cat(comments, buf->ptr);
	strbuf_catchar(comments,'\n');
	memset(buf->ptr,' ',buf->buf + buf->len - buf->ptr);
	break;
      } else if (c=='"' || c=='\'') { // String
	buf->ptr++;
	read_string(c);
      } else {
	buf->ptr++;
      }
    }
  }
  if (comment)
    strbuf_catchar(comments,'\n');

  buf->ptr=buf->buf;
}

// Read next token, doing macro expansion. Removes any @s that are inserted ahead of some tokens
// to avoid recursion in macro expansion

void read_token() {
  read_token_macro();
  if (tok->buf[0]=='@') {
    memmove(tok->buf,tok->buf+1,tok->len+1);
    tok->len--;
  }
}

// Read a list of parameter names as it appears in a macro definition

struct paramlist *read_param_names() {
  struct paramlist *ret=0;
  struct paramlist *last=0;
  int paramno=0;
  buf->ptr++; // (
  for (;;) {
    struct paramlist *np;

    read_horiz_ws();
    for (;;) {
      read_token_raw();
      if (tok->len) break;
      read_textline();
      read_horiz_ws();
    }
    if (tok->buf[0]==')') return ret;
    if (tok->buf[0]==',') continue;
    np=(struct paramlist *)malloc(sizeof(struct paramlist));
    np->paramname=strdup(tok->buf);
    if (last) last->next=np;
    else ret=np;
    last=np;
    np->next=0;
  }
}

extern int evalval;

// Evaluate expression in buffer, return value of expression

int eval() {
  // Replace defined(tok) with @tok, to avoid expansion of macro argument

  YY_BUFFER_STATE I;
  int res;
  struct strbuf *locbuf=strbuf_new();

  for(;;) {
    read_token_raw();
    if (tok->len==0) break;
    if (strcmp(tok->buf,"defined")==0) {
      int parens=1;
      read_horiz_ws();
      if (buf->ptr[0]!='(') {
	parens=0;
      } else {
	buf->ptr++;
	read_horiz_ws();
      }
      read_token_raw();
      strbuf_catchar(locbuf,'@');
      strbuf_cat(locbuf,tok->buf);
      if (parens) {
	read_horiz_ws();
	if (buf->ptr[0]!=')') {
	  cpperror("Syntax error");
	  break;
	}
	buf->ptr++;
      }
    } else {
      strbuf_cat(locbuf,tok->buf);
    }
  }

  strbuf_cpy(buf,locbuf->buf);
  strbuf_free(locbuf);
  expand_macro();

  I=cpp_expr_scan_string(buf->ptr);
  res=cpp_exprparse();
  if (res) cpperror("Syntax error in expression");
  cpp_expr_delete_buffer(I);
  return evalval;
}

// Search a path for a file. Return non-null if failed to open

int pathfopen(char *filename, char *path[]) {
  FILE *file=0;
  struct strbuf *buf;
  
  if (!path) return -1;
  
  buf=strbuf_new();
  
  curfile++;
  
  while (*path) {
    if(*path[0] != '/') {
      strbuf_cpy(buf,curfile[-1].path);
    } else {
      strbuf_clear(buf);
    }
    strbuf_cat(buf,*path);
    
    if(buf->len) strbuf_catchar(buf, '/');
    
    strbuf_cat(buf,filename);

	file=fopen(buf->buf,"r");
    
    if (file) {
      char *lastslash;
      int truelen;
      struct stat sb;
      
      curfile->filename=strdup(buf->buf);
      curfile->path=strdup(buf->buf);
      lastslash = strrchr(curfile->path, '/');
      if (lastslash) {
	*lastslash=0;
      } else {
	curfile->path[0]=0;
      }
      curfile->lineno=0;
      
      fstat(fileno(file), &sb);
      
      curfile->ptr=curfile->buf=(char *)malloc(sb.st_size+2); // 1 for \0, 1 for extra LF
      truelen=fread(curfile->buf,1,sb.st_size,file);
      curfile->buf[truelen]=0;
      fclose(file);
      
      cleanup_lines(curfile->buf);
      
      break;
    }
    //  not_found:
    path++;
  }
    
  strbuf_free(buf);

  if (!file) { // Search built-in header files
    char **bp=builtin;
    
    while (*bp) {
      if (strcmp(bp[0], filename)==0) {
	curfile->filename=strdup(filename);
	curfile->path=strdup("<built-in>");
	curfile->lineno=0;
	curfile->ptr=curfile->buf=strdup(bp[1]);
	cleanup_lines(curfile->buf);

	file=stdin;
	break;
      }
      bp+=2;
    }
  }

  if (file) {
    line_number();
    return 0;
  } else {
    curfile--;
    return -1;
  }
}
  
// Parse a line commencing with #

void parse_directive() {

  buf->ptr++; // #
  read_horiz_ws();
  read_token_raw();

  switch(tok->buf[0]) {
    struct macro *thismacro;
    int bracket;
    char *pathend;

  case 'd':
    if (strcmp(tok->buf,"define")==0) {
      if (*curIF==1) {
        strbuf_cat(cpp_STDOUT, buf->buf);
	thismacro=(struct macro *)malloc(sizeof(struct macro));
	read_horiz_ws();
	read_token_raw();
	thismacro->ident=strdup(tok->buf);
	thismacro->params=0;
	if (*buf->ptr=='(') thismacro->params=read_param_names();
	read_horiz_ws();
	thismacro->def=strdup(buf->ptr);
	stringhash_remove(macros, thismacro->ident);
	stringhash_insert(macros, thismacro->ident, thismacro);
      }
      return;
    }
    break;
  case 'i':
    if (strcmp(tok->buf,"if")==0) {
      read_horiz_ws();
      curIF++;
      *curIF=(curIF[-1]==1 && eval()!=0)?1:0;
      return;
    } else if (strcmp(tok->buf,"ifdef")==0) {
      read_horiz_ws();
      read_token_raw();
      curIF++;
      *curIF=(curIF[-1]==1 && stringhash_search(macros, tok->buf)!=0)?1:0;
      return;
    } else if (strcmp(tok->buf,"ifndef")==0) {
      read_horiz_ws();
      read_token_raw();
      curIF++;
      *curIF=(curIF[-1]==1 && stringhash_search(macros, tok->buf)==0)?1:0;
      return;
    } else if (strcmp(tok->buf,"include")==0) {
      if (*curIF==1) {
	read_horiz_ws();
	if (buf->ptr[0]!='<' && buf->ptr[0]!='"') {
	  expand();
	  if (cpp_error) return;
	}
	if (!buf->ptr[0]) {
	  cpperror("Malformed include directive");
	  break;
	}
	bracket=buf->ptr[0]=='<';
	pathend=strchr(buf->ptr+1,buf->ptr[0]=='<'?'>':'"');
	if (!pathend) {
	  cpperror("Malformed include directive");
	  break;
	}

	pathend[0]=0;

	if (pathfopen(buf->ptr+1,bracket?stdpath:quotepath)) {
	  char *errmsg=malloc(strlen(buf->ptr+1)+50);
	  sprintf(errmsg,"Could not open file \"%s\"", buf->ptr+1);
	  cpperror(errmsg);
	  free(errmsg);
	} else {
	  //	  swallowed=0; // Don't echo newline at beginning of included file
	}
      }
      return;
    }
    break;
  case 'e':
    if (strcmp(tok->buf,"else")==0) {
      *curIF = (curIF[-1]==1 && curIF[0]==0)?1:0;
      return;
    } else if (strcmp(tok->buf,"elif")==0) {
      switch(*curIF) {
      case 0:
	read_horiz_ws();
	*curIF=(curIF[-1]==1 && eval()!=0)?1:0;
	break;
      case 1:
	*curIF=-1;
	break;
      case -1:
	break;
      }
      return;
    } else if (strcmp(tok->buf,"endif")==0) {
      curIF--;
      return;
    } else if (strcmp(tok->buf,"error")==0) {
      if (*curIF==1) {
	read_horiz_ws();
	cpperror(buf->ptr);
      }
      return;
    }
    break;
  case 'u':
    if (strcmp(tok->buf,"undef")==0) {
      if (*curIF==1) {
        strbuf_cat(cpp_STDOUT, buf->buf);
        read_horiz_ws();
        read_token_raw();
        stringhash_remove(macros, tok->buf);
      }
      return;
    }
    break;
  case 'p':
    if (strcmp(tok->buf,"pragma")==0) {
      if (*curIF==1) {
        strbuf_cat(cpp_STDOUT, buf->buf);
        read_horiz_ws();
        read_token_raw();
	if (strcmp(tok->buf,"JSEXT")==0) {
	  read_horiz_ws();
	  read_token_raw();
	  if (strcmp(tok->buf,"path")==0) {
	    read_horiz_ws();
	    read_token_raw();
	    tok->buf[tok->len-1]=0;
	    addpath(tok->buf+1);
	  }
	}
      }
      return;
    }
    break;
  case 'l':
    if (strcmp(tok->buf,"line")==0) {
      if (*curIF==1) {
	read_horiz_ws();
	read_token_raw();
	curfile->lineno=atoi(tok->buf);
	read_horiz_ws();
	free(curfile->filename);
	curfile->filename=strdup(buf->ptr+1);
	curfile->filename[strlen(buf->ptr+1)-1]=0;
	line_number();
	//	swallowed=0;
	return;
      }
    }
    break;
  }
  if (*curIF==1)
    cpperror("Unknown preprocessor directive");
  // otherwise, ignore unknown preprocessor directives
}

// Parse a line not commencing with #

void parse_textline() {
  for (;;) {
    if (!*buf->ptr) break;
    read_horiz_ws();
    if (!*buf->ptr) break;
    if(*curIF == 1) strbuf_cat(cpp_STDOUT, tok->buf);
    read_token();
    if(*curIF == 1) strbuf_cat(cpp_STDOUT, tok->buf);
  }
}

// Read lines until one without preprocessor directives is encountered

void read_textline() {
  while (curfile>=file) {
    read_line();
    strbuf_cat(cpp_STDOUT, "\n");
    read_horiz_ws();
    if (buf->ptr[0]=='#') {
      parse_directive();
    }
    else break;
  }
}

// Parse

void parse() {
  while (curfile>=file) {
    read_line();
    read_horiz_ws();
    if (buf->ptr[0]=='#') {
      parse_directive();
    } else {
      parse_textline();
    }
    if (comment==0) {
      if(*curIF == 1) strbuf_cat(cpp_STDOUT, comments->buf);
      strbuf_clear(comments);
    }
    if (!(comment && *curIF==1)) // not accumulating comment
      strbuf_cat(cpp_STDOUT, "\n");

    if (cpp_error) return;
  }
}


// Initialize character classes

void initclass() {
  char *c;
  memset(alnum,0,256);
  memset(horiz_ws,0,256);
  memset(paren,0,256);
  memset(alpha,0,256);

  for (c=" \t";*c;c++) {
    horiz_ws[*c]=1;
  }

  for (c="([{";*c;c++) {
    paren[*c]=1;
  }

  for (c=")]}";*c;c++) {
    paren[*c]=-1;
  }

  for (c="_@#ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz";*c;c++) {
    alpha[*c]=1;
  }

  for (c="_@#ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz0123456789";*c;c++) {
    alnum[*c]=1;
  }

  /*   trigraph['=']='#'; */
  /*   trigraph['/']='\\'; */
  /*   trigraph['\'']='^'; */
  /*   trigraph['(']='['; */
  /*   trigraph[')']=']'; */
  /*   trigraph['!']='|'; */
  /*   trigraph['<']='{'; */
  /*   trigraph['>']='}'; */
  /*   trigraph['-']='~'; */
}

// Initialize text buffers, file buffers and IF stack

void initbuf() {
  char *posix_stdpath[]={"/usr/local/include", "/usr/include", 0};
  quotepath=(char **)malloc(sizeof(char *)*100);
	quotepath[0]="";
	stdpath=quotepath+1;
  include=(char *)malloc(4096);
  int i=0;
  char **p_in=posix_stdpath;
  while (*p_in) {
    stdpath[i++]=*p_in;
    p_in++;
  }
  include_add=include;
  stdpath[i]=0;

  buf=strbuf_new();
  tok=strbuf_new();
  comments=strbuf_new();
  file=(struct inputfile *)calloc(sizeof(struct inputfile),100);
  curfile=file;
  IF=(int *)calloc(sizeof(int),100);
  IF[0]=1;
  curIF=IF;
}

void freebuf() {
  strbuf_free(buf);
  strbuf_free(tok);
  free(file);
  free(IF);
}

void define(char *name, char *value) {
  struct macro *newmacro=(struct macro *)calloc(sizeof(struct macro)+strlen(value)+1,1);
  newmacro->ident=strdup(name);
  newmacro->def=(char *)(newmacro+1);
  strcpy(newmacro->def,value);

  stringhash_insert(macros,newmacro->ident,newmacro);
}

void initmacros() {
  struct macro *stdc;
  char **defs;

  char *stddefs[]={"__STDC__","1",
		   "__STDC_VERSION__","199901",
#ifdef __x86_64__
		   "__x86_64__","1",
#endif
		   0};
  
  macros=stringhash_new();

  for (defs=stddefs; *defs; defs+=2) {
	stdc=(struct macro *)malloc(sizeof(struct macro)+strlen(defs[1])+1);
	stdc->ident=strdup(*defs);
    stdc->def=(char *)(stdc+1);
    strcpy(stdc->def,defs[1]);
    stdc->params=0;
    stringhash_insert(macros,stdc->ident,stdc);
  }
}

void freemacros() {
  stringhash_itr *I;

  if (stringhash_size(macros)==0) goto done_inside;

  I=stringhash_iterator(macros);

  do {
    struct macro *m=stringhash_iterator_value(I);
    struct paramlist *p=m->params;
    while (p) {
      struct paramlist *n=p->next;
      free(p->paramname);
      free(p);
      p=n;
    }
  } while (stringhash_iterator_advance(I));

  free(I);

 done_inside:
  stringhash_destroy(macros);
}


static void addpath(char *path) {
  char **p=stdpath;
  char *oldpath=include_add;

  strcpy(include_add,path);

  while (*p)
    p++;

  do {
    p[1]=*p;
    p--;
  } while (p>=stdpath);
  p[1]=include_add;

  include_add+=strlen(include_add)+1;
  added_paths++;
}
 
static void delpath() {
  char **p=stdpath;
  while (*p) {
    p[0]=p[1];
    p++;
  }
}
 

void cpp_free(char *C) {
	free(C);
}


char *cpp(char *C, char **errorpos, char **include_path) {
  char *ret;
  char **p;
  int i;

  cpp_error=0;
  initclass();
  initbuf();

  if (include_path) {
    p=include_path;
    while (*p) {
      p++;
    }
    while (p>include_path) {
      p--;
      addpath(*p);
    }
  }

  file->lineno=0;
  file->filename=strdup("<string>");
  file->path=strdup("");
  file->ptr=file->buf=malloc(strlen(C)+2); // 1 for \0, 1 for extra LF
  strcpy(file->buf,C);
  cleanup_lines(file->buf);

  initmacros();

  cpp_STDOUT=strbuf_new();

  line_number();
  strbuf_cat(cpp_STDOUT, "\n");
  parse();

  //  if (!cpp_error)
  //    printmacros();

  freebuf();
  freemacros();

  ret=realloc(cpp_STDOUT->buf,cpp_STDOUT->len+1);
  free(cpp_STDOUT);

  if (errorpos) *errorpos=cpp_error;
  else if (cpp_error) free(cpp_error);

  for (i=0; i<added_paths; i++)
    delpath();

  return ret;
}
