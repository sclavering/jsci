(function() {

for(var i in jsxlib) this[i] = jsxlib[i]; // lift load(), Type, etc to globals
JSEXT1 = this;
load('global/0-ffi.js');
clib = load('global/clib.jswrapper');
File = load('global/JSEXT1/File.js');
stdout = load('global/stdout.js');
stdin = load('global/stdin.js');
stderr = load('global/stderr.js');
println = clib.puts; // print() isn't working

/*
makewrapper.js

Usage:

  /path/to/cpp foo.h -dD -undef --std=gnu99 | JSEXT_INI=makewrapper.js [JSX_WRAPPER_FOR=libfoo.so] /path/to/jsext > foo.js

Include "libfoo.so", which could be a full path, to resolve symbols by dlopen'ing that shared library; otherwise symbols are resolved in the main js binary or stuff already linked to (which is fine for e.g. libc functions).

cpp is assumed to be the one from GCC.  Tested using "cpp (GCC) 4.2.4 (Ubuntu 4.2.4-1ubuntu4)"

  -dD: Keeps "#define FOO 42" and similar (so we can include such constants in the js wrapper), but still expands macros (so that function prototypes are in the form we can parse)
  -undef: stops cpp leaving __extension__ cruft in libmysql.h's output (a gcc-ism)
  --std=gnu99: is just to try and get clib.h's output closer to what our old libcpp was doing
*/


// Token kinds.
const token_list = [
  "tk_skip",
  "tk_const_char", "tk_const_number", "tk_const_string",
  "tk_ident", "tk_typedef_name",
  // "tk_binop",
  'tk_multiplicative_op', 'tk_additive_op', 'tk_shift_op', 'tk_relational_op', 'tk_equality_op', 'tk_&', 'tk_^', 'tk_|', 'tk_&&', 'tk_||',
  "tk_assignment_op",
  "tk_punctuation", "tk_keyword",
  // We use the lexer to identify some things that are more typically nonterminals in the grammar
  "tk_type_qualifier", // type_qualifier
  "tk_storage_class_specifier", // storage_class_specifier
  "tk_type_specifier_keyword",  // part of type_specifier: a literal like "int" or "unsigned"
  "tk_type_specifier_keyword_special", // a prefix like like "struct"
];
const tokens = {};
for(let i = 0; i != token_list.length; ++i) tokens[token_list[i]] = i;


const [lexer_re, match_handlers] = (function() {
  const keyword_aliases = { '_inline': 'inline', '__inline': 'inline' };

  const keywords = [
    [['const', 'volatile'], tokens.tk_type_qualifier], // type_qualifier
    [['extern', 'static', 'auto', 'register', 'inline'], tokens.tk_storage_class_specifier], // storage_class_specifier
    [['char', 'short', 'int', '__int64', 'long', 'signed', 'unsigned', 'float', 'double', '__builtin_va_list'], tokens.tk_type_specifier_keyword],
    [['enum', 'struct', 'union', 'void'], tokens.tk_type_specifier_keyword_special],
  ];

  const keyword_lookup = {};
  for each(let [words, val] in keywords) for each(let word in words) keyword_lookup[word] = val;

  function esc(literal) literal.replace(/\{|\}|\(|\)|\[|\]|\^|\$|\.|\?|\*|\+|\|/g, "\\$&");

  const ops = [
    [["...", "++", "--", "->", ";", "{", "}", ",", ":", "=", "(", ")", "[", "]", ".", "?", "~"], tokens.tk_punctuation],
    [['*=', '/=', '%=', '+=', '-=', '<<=', '>>=', '&=', '^=', '|=', '='], tokens.tk_assignment_op],
    // [['||', '&&', '&', '|', '^', '==', '!=', '<=', '>=', '<<', '>>', '<', '>', '+', '-', '*', '/', '%'], tokens.tk_binop],
    [['*', '/', '%'], tokens.tk_multiplicative_op],
    [['+', '-'], tokens.tk_additive_op],
    [['<<', '>>'], tokens.tk_shift_op],
    [['<', '>', '<=', '>='], tokens.tk_relational_op],
    [['==', '!='], tokens.tk_equality_op],
    [['&'], tokens['tk_&']],
    [['^'], tokens['tk_^']],
    [['|'], tokens['tk_|']],
    [['&&'], tokens['tk_&&']],
    [['||'], tokens['tk_||']],
  ];
  const op_re = RegExp(Array.concat.apply(null, [x[0] for each(x in ops)]).map(esc).join("|"));
  const op_kind_map = {};
  for each(let pair in ops) for each(let op in pair[0]) op_kind_map[op] = pair[1];

  const lex_parts = [
    // whitespace
    [/[ \t\v\f\n]/, tokens.tk_skip],
    // variables and typenames, etc.
    [/[a-zA-Z_][a-zA-Z_0-9]*/, function(str, parser) {
      if(str in keyword_aliases) str = keyword_aliases[str];
      return [str, keyword_lookup[str] || (str in parser._typedefs ? tokens.tk_typedef_name : tokens.tk_ident)];
    }],
    // operators
    [op_re, function(str, parser) [str, op_kind_map[str]]],
    // #line, #define, etc
    [/#[^\n]*\n/, function(str, parser) {
      // keep everything but line number things (e.g. '# 1 "/usr/lib/gcc/i486-linux-gnu/4.2.4/include/stdarg.h" 1 3 4')
      if(str[1] != ' ') parser.preprocessor_directives.push(str.slice(0, -1)); // consumer can't cope with the \n's
      return [str, tokens.tk_skip];
    }],
    // numeric literals
    [/\d+[Ee][+-]?\d+[fFlL]?|\d*\.\d+(?:[Ee][+-]?\d+)?[fFlL]?/, tokens.tk_const_number],
    [/0[xX][a-fA-F0-9]+[uUlL]*|\d+[uUlL]*/, tokens.tk_const_number],
    // char literal.
    [/'(?:\\.|[^\\'])'/, tokens.tk_const_char], // xxx unescape
    // string literals
    [/L?"(?:\\.|[^\\"])*"/, function(str) {
      if(str[0] == 'L') CParser.prototype.ParseError("Encountered an L\"...\" string literal, which we don't support");
      return [str.slice(1, -1), tokens.tk_const_string]; // xxx unescape
    }],
    // C comments
    [/\/\*|\/\//, function() CParser.prototype.ParseError("Encountered a C comment.  cpp should have already removed these")],
  ];

  const re_bits = [];
  const match_handlers = [NaN]; // the NaN is so the interesting values start at index 1, to match the re.exec() results

  for each(let part in lex_parts) {
    re_bits.push(String.slice(part[0], 1, -1)); // serialise the regex, and remove the leading and trailing /
    match_handlers.push(part[1]);
  }

  // "y" means "sticky", which always matches starting from .lastIndex property of the regex, with an implicit ^
  const re = new RegExp("(" + re_bits.join(")|(") + ")", "y");

  return [re, match_handlers];
})();


function do_nothing(id, js) {};


function CParser() {
  this._lexer_re = new RegExp(lexer_re);
  this.preprocessor_directives = [];
  this._typedefs = {}; // a set.  lexing C requires tracking typedefs to disambiguate parts of the grammar
  this._srccode = null;
  this._nexttok = null;
}

CParser.prototype = {
  Parse: function(code, grammar_rule) {
    this._defs = [];
    this._srccode = code;
    this._lexer_re.lastIndex = 0;
    this._nexttok = null;
    this[grammar_rule || 'translation_unit']();
    return this._defs;
  },

  // can pass id '' to just get some code to execute
  Declare: function(id, js_code) {
    this._defs.push([id, js_code]);
  },


  // lexer methods

  LexerPos: function() {
    return this._lexer_re.lastIndex;
  },

  LexerRewind: function(pos) {
    this._lexer_re.lastIndex = pos;
  },

  LexToken: function() {
    while(true) {
      let match = this._lexer_re.exec(this._srccode);
      if(!match) return null;

      let i = 1;
      while(!match[i]) ++i; // find which pattern was matched
      let tokstr = match[0], handler = match_handlers[i]; // handler is a function, or a numeric constant
      if(typeof handler == "function") [tokstr, handler] = handler(match[0], this);
      if(handler === tokens.tk_skip) continue;
      let tok = new String(tokstr);
      tok.tok_kind = handler;
      return tok;
    }
  },


  // methods that aren't grammar symbols

  Peek: function Peek() {
    return this._nexttok || (this._nexttok = this.LexToken());
  },

  PeekIf: function PeekIf(value) {
    const t = this.Peek();
    return t && t == value ? t : null;
  },

  Next: function Next(value) {
    const t = this.Peek();
    if(!t) this.ParseError("ran out of tokens");
    if(value && t != value) this.ParseError("expecting token '" + value + "'");
    this._Advance();
    return t;
  },

  NextAsKind: function NextAsKind(token_kind) {
    const t = this.Peek();
    if(!t) this.ParseError("ran out of tokens");
    if(t.tok_kind != token_kind) this.ParseError("expecting token of kind " + token_kind);
    this._Advance();
    return t;
  },

  NextIf: function NextIf(value) {
    const t = this.PeekIf(value);
    if(t) this._Advance();
    return t;
  },

  NextIfKind: function(kind) {
    const t = this.Peek();
    if(!t || t.tok_kind != kind) return null;
    this._Advance();
    return t;
  },

  _Advance: function _Next(no_errors) {
    if(!this._nexttok) throw Error("CParser: _Advance() called when Peek() returned null");
    this._nexttok = null;
  },

  // Error subclasses are painful in js, so just set a parameter instead
  ParseError: function ParseError(msg) {
    const pos = this.LexerPos()
    const snippet = (this._srccode.slice(Math.max(0, pos - 50), pos) + "^^" + this._srccode.slice(pos, pos + 50)).split("\n").join("\n\t");
    const e = Error(msg + "\n\tAt char #" + pos + " marked by ^^ in the following:\n\t" + snippet + "\n");
    e.isParseError = true;
    throw e;
  },

  Try: function Try(symbol_name, arg0, arg1) {
    const ix = this.LexerPos(), _nexttok = this._nexttok;
    try {
      return this[symbol_name](arg0 || null, arg1 || null);
    } catch(e if e.isParseError) {
      this._nexttok = _nexttok;
      this.LexerRewind(ix);
      return null;
    }
  },


  // grammar symbol methods

  primary_expr: function primary_expr() {
    // primary_expr  :  identifier  |  CONSTANT  |  strings  |  '(' expr ')'
    const tk = this.Next(), tkstr = String(tk);
    switch(tk.tok_kind) {
      case tokens.tk_ident:
        return 'w.' + tkstr;
      case tokens.tk_const_char:
        return uneval(tkstr);
      case tokens.tk_const_number:
        return tkstr; // JS number syntax is mostly the same as C's
      case tokens.tk_const_string: {
        let t, str = tkstr;
        while((t = this.NextIfKind(tokens.tk_const_string))) str += t;
        return uneval(str);
      }
    }
    if(tkstr === '(') {
      let e = this.expr();
      this.Next(')');
      return '(' + e + ')';
    }
    this.ParseError("expected a constant");
  },

  postfix_expr: function postfix_expr() {
    // postfix_expr  :  primary_expr ('[' expression ']'  |  '(' argument_expr_list ')'  |  '.' IDENTIFIER  |  '->' IDENTIFIER  |  '++'  |  '--')*
    let e = this.primary_expr();
    while(true) {
      let t = String(this.Peek());
      switch(t) {
        case '[': this.Next(); e += '[' + this.expr() + ']'; this.Next(']'); continue;
        case '.': this.Next(); e += '.' + this.identifier(); continue;
        case '->': this.Next(); e += '.' + this.identifier(); continue;
        case '++': case '--': e += t; this.Next(); continue;
        case '(': this.Next(); e += '(' + this.argument_expr_list_CP() + ')'; continue;
      }
      break;
    }
    return e;
  },

  argument_expr_list_CP: function argument_expr_list_CP() {
    // argument_expr_list  :  (assignment_expr (',' assignment_expr)*)? ')'
    if(this.NextIf(')')) return [];
    let ael = this.assignment_expr();
    while(this.NextIf(',')) ael += this.assignment_expr();
    this.Next(')');
    return ael;
  },

  unary_expr: function unary_expr() {
    /*
    unary_expr
      : postfix_expr
      | '++' unary_expr
      | '--' unary_expr
      | unary_operator cast_expr
      | 'sizeof' unary_expr
      | 'sizeof' '(' type_name_CP
    */
    let tk = String(this.Peek());
    switch(tk) {
      case '++':
      case '--': {
        this.Next(); let e = this.unary_expr();
        return '(' + tk + e + ')'; // parentheses may be gratuitous
      }
      case 'sizeof': {
        this.Next();
        // xxx one of these two cases is probably wrong
        if(this.NextIf('(')) return 'Type.sizeof(' + this.type_name_CP(do_nothing) + ')';
        return 'Type.sizeof(' + this.unary_expr() + ')';
      }
      case '&':
      case '*':
      case '+':
      case '-':
      case '~':
      case '!':
        // xxx some of these don't exist in JS, so need special casing
        this.Next(); let e = this.cast_expr();
        return '(' + tk + e + ')';
    }
    return this.postfix_expr();
  },

  cast_expr: function cast_expr() {
    // cast_expr  : '(' type_name_CP cast_expr  |  unary_expr
    // the peek isn't sufficient, because a unary_expr can be parenthesised
    return (this.PeekIf('(') && this.Try('cast_expr_real')) || this.unary_expr();
  },

  cast_expr_real: function cast_expr_real() {
    // cast_expr_real: '(' type_name_CP cast_expr
    this.Next('(');
    this.type_name_CP(do_nothing); // ignore the cast for now
    return this.cast_expr();
  },

  multiplicative_expr: function multiplicative_expr() {
    // multiplicative_expr: cast_expr (('*'|'/'|'%') cast_expr)*
    return this.ExprHelper('cast_expr', tokens.tk_multiplicative_op);
  },

  additive_expr: function additive_expr() {
    // additive_expr: multiplicative_expr (('+'|'-') multiplicative_expr)*
    return this.ExprHelper('multiplicative_expr', tokens.tk_additive_op);
  },

  shift_expr: function shift_expr() {
    // shift_expr: additive_expr (('<<'|'>>') additive_expr)*
    return this.ExprHelper('additive_expr', tokens.tk_shift_op);
  },

  relational_expr: function relational_expr() {
    // relational_expr: shift_expr (('<'|'>'|'<='|'>=') shift_expr)*
    return this.ExprHelper('shift_expr', tokens.tk_relational_op);
  },

  equality_expr: function equality_expr() {
    // equality_expr: relational_expr (('=='|'!=') relational_expr)*
    return this.ExprHelper('relational_expr', tokens.tk_equality_op);
  },

  and_expr: function and_expr() {
    // and_expr: equality_expr ('&' equality_expr)*
    return this.ExprHelper('equality_expr', tokens['tk_&']);
  },

  xor_expr: function xor_expr() {
    // xor_expr: and_expr ('^' and_expr)*
    return this.ExprHelper('and_expr', tokens['tk_^']);
  },

  inclusive_or_expr: function inclusive_or_expr() {
    // inclusive_or_expr: xor_expr ('|' xor_expr)*
    return this.ExprHelper('xor_expr', tokens['tk_|']);
  },

  logical_and_expr: function logical_and_expr() {
    // logical_and_expr: inclusive_or_expr ('&&' inclusive_or_expr)*
    return this.ExprHelper('inclusive_or_expr', tokens['tk_&&']);
  },

  logical_or_expr: function logical_or_expr() {
    // logical_or_expr: logical_and_expr ('||' logical_and_expr)*
    return this.ExprHelper('logical_and_expr', tokens['tk_||']);
  },

  // the shift_expr/additive_expr/etc. productions all follow this basic pattern
  ExprHelper: function ExprHelper(base_name, op_token_kind) {
    let js = this[base_name]();
    while(true) {
      let t = this.NextIfKind(op_token_kind);
      if(!t) break;
      js = '(' + js + ' ' + t + ' ' + this[base_name]() + ')';
    }
    return js;
  },

  conditional_expr: function conditional_expr() {
    // conditional_expr: logical_or_expr ('?' expr ':' conditional_expr)?
    const e0 = this.logical_or_expr();
    if(!this.NextIf('?')) return e0;
    const e1 = this.expr();
    this.Next(':');
    const e2 = this.conditional_expr();
    return '(' + e0 + ' ? ' + e1 + ' : ' + e2 + ')';
  },

  assignment_expr: function assignment_expr() {
    // assignment_expr  :  unary_expr assignment_operator assignment_expr  |  conditional_expr
    // we must Try() the entire first branch, not just Try('unary_expr'), because things like constants...
    return this.Try('assignment_expr_branch1') || this.conditional_expr();
  },
  assignment_expr_branch1: function assignment_expr_branch1() {
    const ue = this.unary_expr(), op = this.NextAsKind(tokens.tk_assignment_op), e1 = this.assignment_expr();
    return '(' + ue + ' ' + op + ' ' + e1 + ')';
  },

  expr: function expr() {
    // expr: assignment_expr (',' assignment_expr)*
    let js = this.assignment_expr();
    while(this.NextIf(',')) js = '(' + js + ',' + this.assignment_expr() + ')';;
    return js;
  },

  constant_expr: function constant_expr() this.conditional_expr(),

  external_definition: function external_definition() {
    // external_definition:  function_definition  |  typedef  |  declaration
    // typedef: 'typedef' basic_type init_declarator_list_optional
    // declaration:  storage_class_specifiers basic_type init_declarator_list_optional
    // function_definition:  storage_class_specifiers basic_type declarator compound_statement

    if(this.NextIf('typedef')) {
      let js = this.basic_type(do_nothing);
      let decls = this.init_declarator_list_optional(js);
      for each(let [id, js] in decls) {
        if(!id) this.ParseError("??? typedef'ing without an identifier");
        this._typedefs[id] = true;
        this._defs.push([id, js]);
      }
      return;
    }

    // We don't actually do anything with them; though maybe we should skip declarations with "inline"
    let t;
    while((t = this.NextIfKind(tokens.tk_storage_class_specifier))) ;

    let bt = this.basic_type(do_nothing);
    const idl = this.Try('init_declarator_list_optional', bt);
    if(idl) {
      for each(let [id, js] in idl)
        if(id)
          this._defs.push([id, 'lib.pointer(' + uneval(id) + ', ' + js + ')']);
      return;
    }

    let decl = this.declarator(bt);
    if(decl[0]) this._defs.push(decl);
    this.compound_statement();
  },

  basic_type: function basic_type() {
    // In most grammars there are "declaration_specifiers" and "specifier_qualifier_list" which are each roughly:
    //     (storage_class_specifier | type_specifier | type_qualifier)+
    // We hoist "storage_class_specifier" to "external_definition", and use this instead of both of them:
    //     basic_type:  type_qualifier* type_specifier* type_qualifier*
    this.type_qualifiers();
    let js = this.type_specifier();
    this.type_qualifiers();
    return js;
  },

  type_qualifiers: function type_qualifiers() {
    // type_qualifiers:  type_qualifier*
    while(this.NextIfKind(tokens.tk_type_qualifier)) ;
    // our ffi layer doesn't care about const/volatile, so we return nothing
  },

  init_declarator_list_optional: function init_declarator_list_optional(js) {
    // init_declarator_list_optional: (init_declarator (',' init_declarator)*)? ';'
    let decls = [];
    if(this.NextIf(';')) return decls;
    do { decls.push(this.init_declarator(js)); } while(this.NextIf(','));
    this.Next(';');
    return decls;
  },

  init_declarator: function init_declarator(type) {
    // declarator ('=' initializer)?
    let decl = this.declarator(type);
    if(this.NextIf('=')) this.initializer();
    return decl;
  },

  type_specifier: function type_specifier() {
    /*
    type_specifier
      : ('char' | 'short' | 'int' | 'long' | 'float' | 'double' | 'signed' | 'unsigned')+
      | 'void'
      | 'enum' enum_specifier_guts
      | ('struct' | 'union') struct_or_union_specifier_guts
      | tk_typedef_name
    Note that a normal grammar the + is on type_specifier, and disallowing "int struct ..." is done outside the parser.
    */
    const t = this.NextIfKind(tokens.tk_typedef_name);
    if(t) return 'w.' + String(t);
    const kw = this.NextIfKind(tokens.tk_type_specifier_keyword_special);
    if(kw) {
      switch(String(kw)) {
        case "struct":
        case "union":
          return this.struct_or_union_specifier_guts(String(kw));
        case "enum":
          return this.enum_specifier_guts();
        case "void":
          return "Type['void']";
      }
    }
    let a = '', b = '', c = 'int';
    while((tk = this.NextIfKind(tokens.tk_type_specifier_keyword))) {
      switch((tk = String(tk))) {
        case 'unsigned': case 'signed': a = tk; break;
        case 'short': case 'long': b = b ? b + '_' + tk : tk; break; // note: has to handle "long long"
        case 'char': case 'int': case 'float': case 'double': c = tk; break;
      }
    }
    if(!a && !b && !c) this.ParseError("expecting struct/union/enum, <typedef>, or int/long/etc.");
    // xxx ctypes lacks long_double
    if(b == 'long' && c == 'double') return 'Type.double';
    // ctypes uses long_long, not long_long_int, making this a littly fiddly
    return 'Type.' + a + (a ? '_' : '') + b + (b && c == 'int' ? '' : b ? '_' + c : c);
  },

  enum_specifier_guts: function() {
    // enum_specifier_guts:  '{' enumerator_list_CB  |  IDENTIFIER '{' enumerator_list_CB  |  IDENTIFIER
    let tok0 = this.Next();
    if(tok0 == '{' || this.NextIf('{')) this.enumerator_list_CB();
    return 'Type.int'; // all enums are ints
  },

  enumerator_list_CB: function() {
    // enumerator_list_CB:  enumerator (',' enumerator)* ','? '}'
    this.enumerator();
    while(this.NextIf(',') && !this.PeekIf('}')) this.enumerator();
    this.Next('}');
  },

  enumerator: function enumerator() {
    // enumerator: IDENTIFIER ('=' constant_expr)?
    this.identifier();
    if(this.NextIf('=')) this.constant_expr();
  },

  struct_or_union_specifier_guts: function struct_or_union_specifier_guts(suKind) {
    // struct_or_union_specifier_guts:  IDENTIFIER  |  IDENTIFIER? struct_declaration_list
    // note: we fail parsing "extern struct _IO_FILE *stdin;" in stdio.h without the tk_typedef_name case
    const id_tk = this.NextIfKind(tokens.tk_ident) || this.NextIfKind(tokens.tk_typedef_name);
    const id = id_tk ? suKind + '$' + String(id_tk) : null;
    if(!id && !this.PeekIf('{')) this.ParseError("expecting '{' or an identifier");

    let js = 'Type.' + suKind + '()';
    let su_fields = this.struct_declaration_list();
    if(su_fields) {
      let su_fields_str = ['{ name:' + uneval(decl[0] || '') + ', type: ' + decl[1] + ' }' for each(decl in su_fields)].join(', ');
      if(id) {
        // The struct may be recursive
        this.Declare(id, js);
        this.Declare('', 'Type.replace_members(w.' + id + ', ' + su_fields_str + ')');
        return 'w.' + id;
      }
      return '(function(){ let t = ' + js + '; Type.replace_members(t, ' + su_fields_str + '); return t; })()';
    }
    if(id) return 'w.' + id;
    throw new Error("struct_or_union_specifier_guts: unreachable");
  },

  struct_declaration_list: function struct_declaration_list() {
    // struct_declaration_list:  '{' struct_declaration+ '}'
    if(!this.NextIf('{')) return null;
    let fields = [];
    do { fields.push.apply(fields, this.struct_declaration()) } while(!this.NextIf('}'));
    return fields;
  },

  struct_declaration: function struct_declaration() {
    // struct_declaration: basic_type struct_declarator_list? ';'
    // struct_declarator_list:  struct_declarator (',' struct_declarator)*
    const js = this.basic_type();
    const fields = [];
    if(this.PeekIf(';')) {
      fields.push(['', js]);
    } else {
      do { fields.push(this.struct_declarator(js)); } while(this.NextIf(','));
    }
    this.Next(';');
    return fields;
  },

  struct_declarator: function struct_declarator(js) {
    // struct_declarator:  declarator (':' constant_expr)?  |  ':' constant_expr
    if(this.NextIf(':')) {
      this.constant_expr();
      return ['', js]; // xxx should be using the bitfield width somehow
    } else {
      let decl = this.declarator(js);
      if(this.NextIf(':')) this.constant_expr(); // xxx this should modify the preceding declarator;
      return decl;
    }
  },

  declarator: function declarator(js) {
    // declarator:  pointer* direct_declarator  |  pointer+
    // pointer: '*' type_qualifier*
    // xxx the plain pointer case is needed to handle stuff like: "int f(int (*)(int));" (the "(*)" part specifically).  Our yacc grammar doesn't seem to have had such a rule though.
    let has_ptr = false;
    while(this.NextIf('*')) {
      js = 'Type.pointer(' + js + ')';
      this.type_qualifiers();
      has_ptr = true;
    }
    let dd = this.maybe_direct_declarator(js);
    if(dd) return dd;
    if(!has_ptr) this.ParseError("declarator: expecting either a pointer or direct_declarator or both");
    return ['', js];
  },

  maybe_direct_declarator: function direct_declarator(js) {
    // For normal declarators, this would be:
    // direct_declarator: (identifier | '(' declarator ')') declarator_suffixes
    // ... but we also use this for parameter types, where an identifier can be omitted
    const id = this.maybe_identifier();
    if(id) {
      js = this.declarator_suffixes(js);
      return [id, js];
    }
    if(this.NextIf('(')) {
      // xxx we ought to Try this, because declarator_suffixes can also start with a '('
      const [id2, dd] = this.declarator(js); // xxx pretty damn sure this is wrong
      this.Next(')');
      return [id2, this.declarator_suffixes(dd)];
    }
    return null;
  },

  declarator_suffixes: function declarator_suffixes(js) {
    // declarator_suffixes:  declarator_suffix*
    // declarator_suffix:  '[' constant_expr? ']'  |  '(' parameter_type_list_CP
    while(true) {
      if(this.NextIf('[')) {
        let ce = this.PeekIf(']') ? null : this.constant_expr();
        this.Next(']');
        js = ce ? 'Type.array(' +js + ', ' + ce + ')' : 'Type.pointer(' + js + ')';
      } else if(this.NextIf('(')) {
        let paramjs = this.parameter_type_list_CP();
        js = "Type['function'](" + js + ', [' + paramjs + "], false, 'cdecl')";
      } else {
        break;
      }
    }
    return js;
  },

  parameter_type_list_CP: function parameter_type_list_CP() {
    // parameter_type_list_CP: (parameter_list (',' '...')?)? ')'   // the ')' has been moved here from the callers
    // parameter_list: parameter_declaration (',' parameter_declaration)*
    if(this.NextIf(')')) return [];
    let params = [];
    function decl_param(id, js) params.push(js); // ctypes doesn't care about paramater names
    this.parameter_declaration(decl_param);
    while(this.NextIf(',')) {
      // xxx I think ctypes can't handle varargs functions.  for now, let's just generate a defective wrapper
      if(this.NextIf('...')) break;
      this.parameter_declaration(decl_param);
    }
    this.Next(')');
    return params.join(', ');
  },

  parameter_declaration: function parameter_declaration() {
    // parameter_declaration: basic_type declarator?
    let js = this.basic_type();
    return this.Try('declarator', js) || js;
  },

  type_name_CP: function type_name_CP() {
    // type_name_CP: basic_type declarator? ')'
    let bt = this.basic_type(do_nothing);
    if(this.NextIf(')')) return bt;
    let [id, js] = this.declarator(bt);
    this.Next(')');
    return id ? 'w.' + id : js;
  },

  initializer: function initializer() {
    // (initializer_list has been merged into this)
    // initializer  :  assignment_expr  |  '{' initializer (',' initializer)* ','? '}'
    if(!this.NextIf('{')) {
      this.assignment_expr();
      return;
    }
    while(true) {
      this.initializer();
      let t = this.Next();
      if(t == '}') break;
      if(t != ',') this.ParseError("initializer: expected a ',' or '}'");
      if(this.PeekIf('}')) break; // handle trailing comma
    }
  },

  // we don't actually care about statements
  statement: function() {
    /* statement
      // expression_statement
      : expr? ';'
      // labeled_statement
      | IDENTIFIER ':' statement
      | 'case' constant_expr ':' statement
      | 'default' ':' statement
      // compound_statement (inlined)
      '{' statement* '}'
      // selection_statement
      | 'if' '(' expr ')' statement ('else' statement)?
      | 'switch' '(' expr ')' statement
      // iteration_statement
      | 'while' '(' expr ')' statement
      | 'do' statement 'while' '(' expr ')' ';'
      | 'for' '(' expr? ';' expr? ';' expr? ')' statement
      // jump_statement
      | 'goto' IDENTIFIER ';'
      | 'continue' ';'
      | 'break' ';'
      | 'return' ';'
      | 'return' expr ';'
    xxx probably need to allow a declaration here?
    */
    const t0 = this.Peek();
    switch(String(t0)) {
      // switch and while happen to have the same structure
      case 'switch':
      case 'while':
        this.Next(); this.Next('('); this.expr(); this.Next(')'); this.statement();
        return;
      case 'do':
        this.Next(); this.statement(); this.Next('while'); this.Next('('); this.expr(); this.Next(')'); this.Next(';');
        return;
      case 'if':
        this.Next(); this.Next('('); this.expr(); this.Next(')'); if(this.NextIf('else')) this.statement();
        return;
      case 'for':
        this.Next(); this.Next('(');
        if(!this.PeekIf(';')) this.expr(); this.Next(';');
        if(!this.PeekIf(';')) this.expr(); this.Next(';');
        if(!this.PeekIf(')')) this.expr(); this.Next(')'); this.statement();
        return;
      case 'goto':
        this.Next(); this.identifier(); this.Next(';');
        return;
      case 'continue':
      case 'break':
        this.Next(); this.Next(';');
        return;
      case 'return':
        this.Next(); if(!this.PeekIf(';')) this.expr(); this.Next(';');
        return;
      case 'case':
        this.Next(); this.constant_expr(); this.Next(':'); this.statement();
        return;
      case '{':
        this.compound_statement();
        return;
    }
    // xxx should allow an expression_statement here, but it doesn't matter when just parsing .h files
    throw Error("not implemented");
  },

  compound_statement: function() {
    // compound_statement: '{' statement* '}'
    this.Next('{');
    while(!this.NextIf('}')) this.statement();
  },

  maybe_identifier: function() {
    const t = this.NextIfKind(tokens.tk_ident);
    return t ? String(t) : null;
  },

  identifier: function identifier() {
    return String(this.NextAsKind(tokens.tk_ident));
  },

  // this is the usual entry point
  translation_unit: function translation_unit() {
    // translation_unit: external_definition+
    while(this.Peek()) this.external_definition();
  },

  Debug: function(str, thing) {
    println(str + ' peek [' + this.Peek() + '] ' + uneval(thing) + '\n');
  },
};



/*
Note: the old libcpp hard-coded some standard .h files:

  float.h: empty
  stddef.h:
		 #ifndef _STDDEF_H
		 #define _STDDEF_H
		 typedef unsigned long size_t;
		 typedef short wchar_t;
		 typedef int ptrdiff_t;
     #define inline _inline
     #define NULL 0
		 #endif
   stdarg.h:
		 #ifndef _STDARG_H
		 #define _STDARG_H
		 typedef __builtin_va_list __gnuc_va_list;
		 typedef __builtin_va_list va_list;
		 #endif

Since switching to using GCC's cpp, we lack clib.va_list, and have loads of constants from float.h
*/


function jswrapper(fragment) {
  const getters = [];
  for(var i in fragment) {
    var code = fragment[i];
    // We use comma expressions sometimes, which need parenthesising to work as desired
    if(!/^(?:-?\d+|null|undefined)$/.test(code)) code = '(' + code + ')';
    getters.push('  ' + i + ': ' + code);
  }

  return "({\n" + getters.join(",\n") + ",\n})\n";
};



function main(libfilename, srccode) {
  const parser = new CParser();
  const defs = parser.Parse(srccode);
  let jscode = '(function(){\nconst w = {};\n';
  jscode += 'const lib = new Dl(' + uneval(libfilename) + ');\n';

  const lib = new Dl(libfilename), w = {};
  for each(let [k, v] in defs) {
    let str = (k ? 'w.' + k + ' = ' : '') + v + ';\n';
    try {
      eval(str);
      jscode += str;
    } catch(e) {
      jscode += '// omitting "' + k + '" because: ' + e.message.split('\n', 1)[0] + '\n';
      jscode += '// ' + str;
    }
  }
  jscode += 'return w;\n})()\n';
  return jscode;
};


try {
println(main(environment.JSX_WRAPPER_FOR || '', stdin.read()));
} catch(e) { println(e.message + '\n\n'); println(e.stack); throw e; }


return 0;

})()
