(function() {

const nothing = <></>; // simplifies some of the XML construction in the parser

// Token kinds.
const token_list = [
  "tk_skip",
  "tk_const_char", "tk_const_number", "tk_const_string",
  "tk_ident", "tk_typedef_name",
  "tk_binop", "tk_assignment_op",
  "tk_punctuation", "tk_keyword",
  // We use the lexer to identify some things that are more typically nonterminals in the grammar
  "tk_type_qualifier", // type_qualifier
  "tk_storage_class_specifier", // storage_class_specifier
  "tk_type_specifier_keyword",  // part of type_specifier: a literal like "int", or an prefix like "struct"
];
const tokens = {};
for(let i = 0; i != token_list.length; ++i) tokens[token_list[i]] = i;


const [lexer_re, match_handlers] = (function() {
  const keyword_aliases = { '_inline': 'inline', '__inline': 'inline' };

  const keywords = [
    [['const', 'volatile'], tokens.tk_type_qualifier], // type_qualifier
    [['extern', 'static', 'auto', 'register', 'inline'], tokens.tk_storage_class_specifier], // storage_class_specifier
    [['char', 'short', 'int', '__int64', 'long', 'signed', 'unsigned', 'float', 'double', 'void', '__builtin_va_list'], tokens.tk_type_specifier_keyword],
    [['enum', 'struct', 'union'], tokens.tk_type_specifier_keyword],
  ];

  const keyword_lookup = {};
  for each(let [words, val] in keywords) for each(let word in words) keyword_lookup[word] = val;

  function esc(literal) literal.replace(/\{|\}|\(|\)|\[|\]|\^|\$|\.|\?|\*|\+|\|/g, "\\$&");

  const ops = [
    [["...", "++", "--", "->", ";", "{", "}", ",", ":", "=", "(", ")", "[", "]", ".", "?", "~"], tokens.tk_punctuation],
    [['*=', '/=', '%=', '+=', '-=', '<<=', '>>=', '&=', '^=', '|=', '='], tokens.tk_assignment_op],
    [['||', '&&', '&', '|', '^', '==', '!=', '<=', '>=', '<<', '>>', '<', '>', '+', '-', '*', '/', '%'], tokens.tk_binop],
  ];
  const op_re = RegExp(Array.concat.apply(null, [x[0] for each(x in ops)]).map(esc).join("|"));
  const op_kind_map = {};
  for each(let pair in ops) for each(let op in pair[0]) op_kind_map[op] = pair[1];

  const lex_parts = [
    // whitespace
    [/[ \t\v\f\n]/, tokens.tk_skip],
    // variables and typenames, etc.
    [/[a-zA-Z_][a-zA-Z_0-9]*/, function(str, lexer) {
      if(str in keyword_aliases) str = keyword_aliases[str];
      return [str, keyword_lookup[str] || (str in lexer._typedef_names ? tokens.tk_typedef_name : tokens.tk_ident)];
    }],
    // operators
    [op_re, function(str, lexer) [str, op_kind_map[str]]],
    // #line, #define, etc
    [/#[^\n]*\n/, function(str, lexer) {
      // keep everything but line number things (e.g. '# 1 "/usr/lib/gcc/i486-linux-gnu/4.2.4/include/stdarg.h" 1 3 4')
      if(str[1] != ' ') lexer._preprocessor_lines.push(str.slice(0, -1)); // consumer can't cope with the \n's
      return [str, tokens.tk_skip];
    }],
    // numeric literals
    [/\d+[Ee][+-]?\d+[fFlL]?|\d*\.\d+(?:[Ee][+-]?\d+)?[fFlL]?/, tokens.tk_const_number],
    [/0[xX][a-fA-F0-9]+[uUlL]*|\d+[uUlL]*/, tokens.tk_const_number],
    // char literal.
    [/'(?:\\.|[^\\'])'/, tokens.tk_const_char],
    // string literals
    [/"(?:\\.|[^\\"])*"/, tokens.tk_const_string],
    [/L"(?:\\.|[^\\"])*/, tokens.tk_const_string],
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
})()



function Lexer(str, preprocessor_lines, typedef_names_set) {
  this._str = str;
  this._re = new RegExp(lexer_re);
  this._preprocessor_lines = preprocessor_lines;
  this._typedef_names = typedef_names_set;
}

Lexer.prototype = {
  pos: function() {
    return this._re.lastIndex;
  },

  rewind: function(pos) {
    this._re.lastIndex = pos;
  },

  gettok: function() {
    while(true) {
      let match = this._re.exec(this._str);
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
};



function ExprTreeLeaf(e) {
  this.e = e;
}
ExprTreeLeaf.prototype = {
  addR: function(op, prec, leaf) {
    return new ExprTreeNode(op, prec, this, leaf);
  },
  xml: function() {
    return this.e;
  },
};

function ExprTreeNode(op, prec, l, r) {
  this.op = op; this.prec = prec; this.l = l; this.r = r;
}
ExprTreeNode.prototype = {
  addR: function(op, prec, e) {
    if(prec < this.prec) return new ExprTreeNode(op, prec, this, e);
    // higher precedence, so insert into our right branch
    this.r = this.r.addR(op, prec, e);
    return this;
  },
  xml: function() {
    return <binary_op op={ this.op }>{ this.l.xml() }{ this.r.xml() }</binary_op>;
  },
};



function Parser(srccode) {
  this._srccode = srccode;
  this.preprocessor_directives = [];
  this._typedefs = {}; // a set.  lexing C requires tracking typedefs to disambiguate parts of the grammar
  this._lexer = new Lexer(srccode, this.preprocessor_directives, this._typedefs);
  this._nexttok = 0;
}

Parser.prototype = {
  // methods that aren't grammar symbols

  Peek: function Peek(arg) {
    if(arg !== undefined) this.ParseError("Peek() passed an arg");
    return this._Current();
  },

  PeekIf: function PeekIf(value) {
    if(value === undefined) this.ParseError("PeekIf() passed undefined")
    const t = this._Current();
    return t && t == value ? t : null;
  },

  PeekIfKind: function PeekIfKind(token_kind) {
    const t = this._Current();
    return t && t.tok_kind == token_kind ? t : null;
  },

  Next: function Next(value) {
    const t = this._Next();
    if(value && t != value) this.ParseError("expecting token of value/kind '" + value + "' but got '" + t + "'");
    return t;
  },

  NextAsKind: function NextAsKind(token_kind) {
    const t = this.Next();
    if(t.tok_kind != token_kind) this.ParseError("expecting token of value/kind " + token_kind + " but got '" + t + "'");
    return t;
  },

  NextIf: function NextIf(value) {
    return this.PeekIf(value) ? this._Next() : null;
  },

  NextIfKind: function(kind) {
    return this.PeekIfKind(kind) ? this.Next() : null;
  },

  _Current: function _Current() {
    if(!this._nexttok) this._nexttok = this._Next(true);
    return this._nexttok;
  },

  _Next: function _Next(no_errors) {
    if(this._nexttok) {
      const t = this._nexttok;
      this._nexttok = null;
      return t;
    }
    const t2 = this._lexer.gettok();
    if(!t2 && !no_errors) this.ParseError("ran out of tokens");
    return t2;
  },

  // Error subclasses are painful in js, so just set a parameter instead
  ParseError: function ParseError(msg) {
    const pos = this._lexer.pos()
    const snippet = (this._srccode.slice(Math.max(0, pos - 50), pos) + "^^" + this._srccode.slice(pos, pos + 50)).split("\n").join("\n\t");
    const e = Error(msg + "\n\tAt char #" + pos + " marked by ^^ in the following:\n\t" + snippet + "\n");
    e.isParseError = true;
    throw e;
  },

  Try: function Try(symbol_name) {
    const ix = this._lexer.pos(), _nexttok = this._nexttok;
    try {
      return this[symbol_name]();
    } catch(e if e.isParseError) {
      this._nexttok = _nexttok;
      this._lexer.rewind(ix);
      return null;
    }
  },

  // The ghastly bit where we feed typedef's names back to the lexer to resolve the inherent ambiguity in the grammar
  RecordTypedef: function(node) {
    // yacc code also does some weird thing where it looks for a <dt/> and converts it to an <id/>.  not sure why.
    for each(let id in node.id) this._typedefs[String(id)] = true;
    for each(let ptr in node.ptr) this.RecordTypedef(ptr);
    for each(let ix in node.ix) this.RecordTypedef(ix);
    for each(let p in node.p) this.RecordTypedef(p);
    // Handle stuff like "typedef int (*list_walk_action)(void *foo, void *);".  This will correctly ignore optional parameter names (if included), because they're inside a <pm/> that we don't recurse into
    for each(let fd in node.fd) this.RecordTypedef(fd);
    return node;
  },


  // grammar symbol methods

  primary_expr: function primary_expr() {
    // primary_expr  :  identifier  |  CONSTANT  |  strings  |  '(' expr ')'
    const tok = this.Next();
    switch(tok.tok_kind) {
      case tokens.tk_ident:
        return <id>{ tok }</id>;
      case tokens.tk_const_char:
        // xxx unescape
        return <c>{ tok }</c>;
      case tokens.tk_const_number: {
        let match = tok.match(/^(.*)([fFlLuU])$/);
        if(tok[0] != '0' && match) return <c length={ match[2] }>{ match[1] }</c>;
        return <c>{ tok }</c>;
      }
      case tokens.tk_const_string: {
        // xxx unescape
        let xml = <><s>{ tok }</s></>;
        while(this.Peek(tokens.tk_const_string)) xml += <s>{ this.Next() }</s>;
        return xml;
      }
    }
    if(tok == '(') {
      let e = this.expr();
      this.Next(')');
      return <p>{ e }</p>;
    }
    this.ParseError("expected a constant");
  },

  postfix_expr: function postfix_expr() {
    // postfix_expr  :  primary_expr ('[' expression ']'  |  '(' argument_expr_list ')'  |  '.' IDENTIFIER  |  '->' IDENTIFIER  |  '++'  |  '--')*
    let e = this.primary_expr();
    while(true) {
      let t = this.Peek();
      switch(String(t)) {
        case '[': this.Next(); e = <ix>{ e }{ this.expr() }</ix>; this.Next(']'); continue;
        case '.': this.Next(); e = <mb>{ e }{ this.identifier() }</mb>; continue;
        case '->': this.Next(); e = <ptr>{ e }{ this.identifier() }</ptr>; continue;
        case '++': this.Next(); e = <unary_op op="++" post="1">{ e }</unary_op>; continue;
        case '--': this.Next(); e = <unary_op op="--" post="1">{ e }</unary_op>; continue;
        case '(': this.Next(); e = <call>{ e }{ this.argument_expr_list() }</call>; continue;
      }
      break;
    }
    return e;
  },

  argument_expr_list: function argument_expr_list() {
    // This was originally  "argument_expr_list  :  assignment_expr (',' assignment_expr)*"  but we are using:
    // argument_expr_list  :  (assignment_expr (',' assignment_expr)*)? ')'
    if(this.NextIf(')')) return nothing;
    let ael = <>{ this.assignment_expr() }</>;
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
    switch(String(this.Peek())) {
      case '++':
      case '--':
        return <unary_op op={ this.Next() } pre="1">{ this.unary_expr() }</unary_op>;
      case 'sizeof': {
        this.Next();
        if(this.NextIf('(')) return <sizeof_type>{ this.type_name_CP() }</sizeof_type>;
        return <sizeof_expr>{ this.unary_expr() }</sizeof_expr>;
      }
      // unary_operator cast_expr
      case '&':
      case '*':
      case '+':
      case '-':
      case '~':
      case '!':
        return <unary_op op={ this.Next() }>{ this.cast_expr() }</unary_op>;
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
    return <cast>{ this.type_name_CP() }{ this.cast_expr() }</cast>;
  },

  logical_or_expr: function() {
    const [op_list, op_prec] = this._opinfo();
    let etree = new ExprTreeLeaf(this.cast_expr());
    while(true) {
      let t = this.NextIfKind(tokens.tk_binop);
      if(!t) break;
      etree = etree.addR(t, op_prec[t], new ExprTreeLeaf(this.cast_expr()));
    }
    return etree.xml();
  },
  _opinfo: function() {
    const prec_list = [
      ['||'],                          // logical_or_expr: logical_and_expr ('||' logical_and_expr)*
      ['&&'],                          // logical_and_expr: inclusive_or_expr ('&&' inclusive_or_expr)*
      ['|'],                           // inclusive_or_expr: xor_expr ('|' xor_expr)*
      ['^'],                           // xor_expr: and_expr ('^' and_expr)*
      ['&'],                           // and_expr: equality_expr ('&' equality_expr)*
      ['==', '!='],                    // equality_expr: relational_expr (('=='|'!=') relational_expr)*
      ['<', '>', '<=', '>='],          // relational_expr: shift_expr (('<'|'>'|'<='|'>=') shift_expr)*
      ['<<', '>>'],                    // shift_expr: additive_expr (('<<'|'>>') additive_expr)*
      ['+', '-'],                      // additive_expr: multiplicative_expr (('+'|'-') multiplicative_expr)*
      ['*', '/', '%']                  // multiplicative_expr: cast_expr (('*'|'/'|'%') cast_expr)*
    ];
    const op_list = Array.concat.apply(null, prec_list);
    const op_prec = {};
    for(let i = 0; i != prec_list.length; ++i) for each(let sym in prec_list[i]) op_prec[sym] = i + 1;
    this._opinfo = function() [op_list, op_prec];
    return this._opinfo();
  },

  conditional_expr: function conditional_expr() {
    // conditional_expr: logical_or_expr ('?' expr ':' conditional_expr)?
    const e0 = this.logical_or_expr();
    if(!this.NextIf('?')) return e0;
    const e1 = this.expr();
    this.Next(':');
    const e2 = this.conditional_expr();
    return <conditional_op>{ e0 }{ e1 }{ e2 }</conditional_op>;
  },

  assignment_expr: function assignment_expr() {
    // assignment_expr  :  unary_expr assignment_operator assignment_expr  |  conditional_expr
    // we must Try() the entire first branch, not just Try('unary_expr'), because things like constants...
    return this.Try('assignment_expr_branch1') || this.conditional_expr();
  },
  assignment_expr_branch1: function assignment_expr_branch1() {
    const ue = this.unary_expr(), op = this.NextAsKind(tokens.tk_assignment_op), e1 = this.assignment_expr();
    return <binary_op op={ op }>{ ue }{ e1 }</binary_op>;
  },

  expr: function expr() {
    // expr: assignment_expr (',' assignment_expr)*
    let e = this.assignment_expr();
    while(this.NextIf(',')) e = <binary_op op=",">{ e }{ this.assignment_expr() }</binary_op>;
    return e;
  },

  constant_expr: function constant_expr() this.conditional_expr(),

  declaration_specifiers: function declaration_specifiers() {
    // declaration_specifiers  :  (storage_class_specifier | type_specifier | type_qualifier)+
    let dss = <></>;
    while(true) {
      let t = this.maybe_storage_class_specifier()
        || this.maybe_type_specifier()
        || this.maybe_type_qualifier();
      if(!t) break;
      dss += t;
    }
    if(!dss.length()) this.ParseError("declaration_specifiers: expected at least one item, but encountered '" + this.Peek() + "' instead");
    return dss;
  },

  init_declarator_list_optional: function init_declarator_list_optional() {
    // init_declarator_list_optional: (init_declarator (',' init_declarator)*)? ';'
    if(this.NextIf(';')) return nothing;
    let res = <>{ this.init_declarator() }</>;
    while(this.NextIf(',')) res += this.init_declarator();
    this.Next(';');
    return res;
  },

  init_declarator: function init_declarator() {
    // declarator ('=' initializer)?
    const decl = this.declarator();
    if(this.NextIf('=')) return <init>{ decl }{ this.initializer() }</init>;
    return decl;
  },

  maybe_storage_class_specifier: function maybe_storage_class_specifier() {
    const t = this.NextIfKind(tokens.tk_storage_class_specifier);
    return t ? <{ t }/> : null;
  },

  maybe_type_specifier: function maybe_type_specifier(null_on_error) {
    /*
    type_specifier
      : 'void' | 'char' | 'short' | 'int' | 'long' | 'float' | 'double' | 'signed' | 'unsigned'
      | 'enum' enum_specifier_guts
      | ('struct' | 'union') struct_or_union_specifier_guts
      | tk_typedef_name
    Note that all the callers want the null-on-failure behaviour, so there's no non-maybe_ version
    */
    const t = this.NextIfKind(tokens.tk_typedef_name);
    if(t) return <dt>{ t }</dt>;
    const kw = this.NextIfKind(tokens.tk_type_specifier_keyword);
    if(!kw) return null;
    switch(String(kw)) {
      case "struct":
      case "union":
        return this.struct_or_union_specifier_guts(String(kw));
      case "enum":
        return this.enum_specifier_guts();
    }
    return <t>{ kw }</t>;
  },

  enum_specifier_guts: function() {
    // Based on:
    //   enum_specifier: 'enum' '{' enumerator_list '}'
    //     | 'enum' IDENTIFIER '{' enumerator_list '}'
    //     | 'enum' IDENTIFIER
    // Based on enum_specifier, but we match the 'enum' in the caller, and the '}' in enumerator_list, so:
    //   enum_specifier_guts
    //     : '{' enumerator_list
    //     | IDENTIFIER '{' enumerator_list
    //     | IDENTIFIER
    let tok0 = this.Next();
    if(tok0 == '{') return <enum>{ this.enumerator_list() }</enum>;
    if(this.NextIf('{')) return <enum id={ tok0 }>{ this.enumerator_list() }</enum>;
    return <enum id={ tok0 }/>;
  },

  enumerator_list: function() {
    // Based on:
    //   enumerator_list: enumerator (',' enumerator)*
    // but we also include the closing '}', and allow a trailing ',', so:
    //   enumerator_list: enumerator (',' enumerator)* ','? '}'
    let el = <>{ this.enumerator() }</>;
    while(this.NextIf(',')) {
      if(this.PeekIf('}')) break;
      el += this.enumerator();
    }
    this.Next('}');
    return el;
  },

  enumerator: function enumerator() {
    // enumerator: IDENTIFIER ('=' constant_expr)?
    const id = this.identifier();
    if(this.NextIf('=')) return <>{ id }{ this.constant_expr() }</>;
    return id;
  },

  struct_or_union_specifier_guts: function struct_or_union_specifier_guts(nodeName) {
    // Based on struct_or_union_specifier, but excluding the leading struct_or_union, and the trailing '}'
    //  : '{' struct_declaration_list
    //  | IDENTIFIER '{' struct_declaration_list
    //  | IDENTIFIER
    if(this.NextIf('{')) return <{ nodeName }>{ this.struct_declaration_list() }</{ nodeName }>;
    const id = this.identifier_or_typedef_name();
    if(this.NextIf('{')) return <{ nodeName } id={ id }>{ this.struct_declaration_list() }</{ nodeName }>;
    return <{ nodeName } id={ id }/>;
  },

  struct_declaration_list: function struct_declaration_list() {
    // struct_declaration_list: struct_declaration+
    // modified to match the trailing '}' too
    let sdl = <>{ this.struct_declaration() }</>;
    while(!this.NextIf('}')) sdl += this.struct_declaration();
    return sdl;
  },

  struct_declaration: function struct_declaration() {
    // struct_declaration: specifier_qualifier_list struct_declarator_list
    return <d>{ this.specifier_qualifier_list() }{ this.struct_declarator_list() }</d>;
  },

  specifier_qualifier_list: function specifier_qualifier_list() {
    // specifier_qualifier_list: ( type_qualifier | type_specifier )+
    let sql = <></>;
    while(true) {
      let sq = this.maybe_type_qualifier() || this.maybe_type_specifier();
      if(!sq) break;
      sql += sq;
    }
    if(!sql.length()) this.ParseError("specifier_qualifier_list: coudln't find any type_qualifier or type_specifier");
    return sql;
  },

  struct_declarator_list: function struct_declarator_list() {
    // struct_declarator_list:  ';'  |  struct_declarator (',' struct_declarator)* ';'
    if(this.NextIf(';')) return nothing;
    let sdl = <>{ this.struct_declarator() }</>;
    while(this.NextIf(',')) sdl += this.struct_declarator();
    this.Next(';');
    return sdl;
  },

  struct_declarator: function struct_declarator() {
    // struct_declarator:  declarator (':' constant_expr)?  |  ':' constant_expr
    if(this.NextIf(':')) return <bitfield>{ this.constant_expr() }</bitfield>;
    const d = this.declarator();
    return this.NextIf(':') ? <bitfield>{ d }{ this.constant_expr() }</bitfield> : d;
  },

  declarator: function declarator() {
    // declarator:  pointer* direct_declarator  |  pointer+
    // xxx the plain pointer case is needed to handle stuff like: "int f(int (*)(int));" (the "(*)" part specifically).  Our yacc grammar doesn't seem to have had such a rule though.
    let p, ptrs = <></>;
    while((p = this.maybe_pointer())) ptrs += p;
    const dd0 = this.maybe_direct_declarator(), dd = dd0 || nothing;
    if(!ptrs.length() && !dd0) this.ParseError("declarator: expecting either a pointer or direct_declarator or both");
    return ptrs.length() ? <ptr>{ ptrs }{ dd }</ptr> : dd;
  },

  maybe_pointer: function maybe_pointer() {
    // pointer: '*' type_qualifier*
    if(!this.NextIf('*')) return null;
    let t, a = <a/>;
    while((t = this.NextIfKind(tokens.tk_type_qualifier))) a["@" + t] = "true";
    return a;
  },

  maybe_direct_declarator: function direct_declarator() {
    // direct_declarator: (identifier | '(' declarator ')') declarator_suffixes
    const id = this.maybe_identifier();
    if(id) return this.declarator_suffixes(id);
    if(this.NextIf('(')) {
      const dd = this.declarator();
      this.Next(')');
      return this.declarator_suffixes(dd);
    }
    return null;
  },

  declarator_suffixes: function declarator_suffixes(dd) {
    // declarator_suffixes:  declarator_suffix*
    // declarator_suffix:  '[' constant_expr? ']'  |  '(' parameter_type_list_CP
    while(true) {
      if(this.NextIf('[')) {
        let ce = this.PeekIf(']') ? null : this.constant_expr();
        this.Next(']');
        dd = <ix>{ dd }{ ce || nothing }</ix>;
      } else if(this.NextIf('(')) {
        dd = <fd>{ dd }<pm>{ this.parameter_type_list_CP() }</pm></fd>;
      }
      break;
    }
    return dd;
  },

  maybe_type_qualifier: function maybe_type_qualifier() {
    const t = this.NextIfKind(tokens.tk_type_qualifier);
    return t ? <{ t }/> : null;
  },

  parameter_type_list_CP: function parameter_type_list_CP() {
    // parameter_type_list_CP: (parameter_list (',' '...')?)? ')'   // the ')' has been moved here from the callers
    // parameter_list: parameter_declaration (',' parameter_declaration)*
    if(this.NextIf(')')) return nothing;
    let ptl = <>{ this.parameter_declaration() }</>;
    while(this.NextIf(',')) {
      if(this.NextIf('...')) {
        ptl += <elipsis/>;
        break;
      }
      ptl += this.parameter_declaration();
    }
    this.Next(')');
    return ptl;
  },

  parameter_declaration: function parameter_declaration() {
    // parameter_declaration: declaration_specifiers declarator?
    return <d>{ this.declaration_specifiers() }{ this.Try('declarator') || nothing }</d>;
  },

  type_name_CP: function type_name_CP() {
    // type_name_CP: specifier_qualifier_list declarator? ')'
    const tn = <d>{ this.specifier_qualifier_list() }{ this.PeekIf(')') ? nothing : this.declarator() }</d>;
    this.Next(')');
    return tn;
  },

  initializer: function initializer() {
    // (initializer_list has been merged into this)
    // initializer  :  assignment_expr  |  '{' initializer (',' initializer)* ','? '}'
    if(!this.NextIf('{')) return this.assignment_expr();
    let il = <></>;
    while(true) {
      il += this.initializer();
      let t = this.Next();
      if(t == '}') break;
      if(t != ',') this.ParseError("initializer: expected a ',' or '}', but got '" + t + "'");
      if(this.PeekIf('}')) break; // handle trailing comma
    }
    return <p>{ il }</p>;
  },

  // we don't actually care about statements
  statement: function() {
    /* statement
      : expression_statement
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
      | 'for' '(' expression_statement expression_statement expr? ')' statement
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
        return nothing;
      case 'do':
        this.Next(); this.statement(); this.Next('while'); this.Next('('); this.expr(); this.Next(')'); this.Next(';');
        return nothing;
      case 'if':
        this.Next(); this.Next('('); this.expr(); this.Next(')'); if(this.NextIf('else')) this.statement();
        return nothing;
      case 'for':
        this.Next(); this.Next('('); this.expression_statement(); this.expression_statement(); if(!this.PeekIf(')')) this.expr(); this.Next(')'); this.statement();
        return nothing;
      case 'goto':
        this.Next(); this.identifier(); this.Next(';');
        return nothing;
      case 'continue':
      case 'break':
        this.Next(); this.Next(';');
        return nothing;
      case 'return':
        this.Next(); if(!this.PeekIf(';')) this.expr(); this.Next(';');
        return nothing;
      case 'case':
        this.Next(); this.constant_expr(); this.Next(':'); this.statement();
        return nothing;
      case '{':
        this.compound_statement();
        return nothing;
    }
    throw Error("not implemented");
  },

  compound_statement: function() {
    // compound_statement: '{' statement* '}'
    this.Next('{');
    while(!this.NextIf('}')) this.statement();
    return nothing;
  },

  expression_statement: function expression_statement() {
    if(!this.PeekIf(';')) this.expr();
    this.Next(';');
    return nothing;
  },

  external_definition: function external_definition() {
    // external_definition: function_definition  |  declaration
    // declaration: 'typedef'? declaration_specifiers init_declarator_list_optional
    // function_definition: declaration_specifiers declarator compound_statement
    if(this.NextIf('typedef')) return this.RecordTypedef(<d><typedef/>{ this.declaration_specifiers() }{ this.init_declarator_list_optional() }</d>);
    const ds = this.declaration_specifiers();
    const idl = this.Try('init_declarator_list_optional');
    if(idl !== null) return <d>{ ds }{ idl }</d>;
    return <fdef>{ ds }{ this.declarator() }{ this.compound_statement() }</fdef>;
  },

  identifier_or_typedef_name: function() {
    const t = this.Next();
    if(t.tok_kind == tokens.tk_ident || t.tok_kind == tokens.tk_typedef_name) return String(t);
    this.ParseError("expecting an identifier or typedef'd name");
  },

  maybe_identifier: function() {
    const t = this.NextIfKind(tokens.tk_ident);
    return t ? <id>{ t }</id> : null;
  },

  identifier: function identifier() {
    const t = this.Next();
    if(t.tok_kind != tokens.tk_ident) this.ParseError("expected an identifier, but got '" + t + "'");
    return <id>{ t }</id>;
  },

  // this is the usual entry point
  translation_unit: function translation_unit() {
    // translation_unit: external_definition+
    let ed, eds = <></>;
    while(this.Peek()) eds += this.external_definition();
    return <C>{ eds }</C>;
  },

  Debug: function(str, thing) {
    print(str, ' peek [', this.Peek(), '] ', uneval(thing), '\n');
  },
};


return Parser;

})()
