/*
obj = parse(code, default_dl)

Examines a C program. The program should already have been processed
by cpp() and ctoxml().  Recognizes the programming
constructs used to declare functions, structs, unions, global variables,
macros, #defines and enums. Also recognizes the following #pragma
directives.

### Pragma directives ###

    #pragma JSEXT dl "filename"

Instructs _parse_ to try to resolve global variables and functions
in the given file, which should be a dynamic library.
Applies to all symbols in the program, whether they are defined
before or after the pragma directive. Libraries are searched
in the order that they are mentioned in the program.

    #pragma JSEXT dl main

Instructs _parse_ to try to resolve global variables and functions
in the executing program. On Linux, this can be used to resolve
all C library symbols and symbols from the SpiderMonkey API.


### Arguments ###

* code: An XML object returned from ctoxml()
* default_dl: A Dl object which is used for symbol resolution
  in addition to any specified by #pragma JSEXT dl.

### Return value ###

Returns an object containing the following properties:

* _live_: Object containing live pointers, types, dl objects etc.
* _expsym_: Object containing one key per symbol which is to be imported/exprted.
* _su_: Object containing one string property per declaration of structs and unions.
* _sym_: Object containing one string property per symbol
*/

(function() {

return function(code, default_dl) {

  // Contains the evaluated code. Used during
  // processing to evaluate sizeof() expressions.

  var that={};

  // Contains symbols
  var sym={};

  // Contains declarations of structs and unions
  var su={};

  // Contains dependencies
  var dep={};

  // Contains symbols to be exported
  var expsym={};

  // Count number of dl files
  var ndl=0;

  loaddls.call(that);

  parse_inner.call(that); // Make 'that' 'this'
  initmacro.call(that);
  allmacros();

  for(var i in expsym) include_dep(i);

  return {
    exported_symbols: expsym,
    structs_and_unions: su,
    sym: sym,
  };


  function include_dep(sym) {
    for (var i in dep[sym]) {
      if (!expsym[i]) {
        expsym[i] = true;
        include_dep(i);
      }
    }
  }


  function parse_inner() {
    var tu;

    for each(tu in code.*) {
      var tmpdep={};

      switch(String(tu.name())) {
      // ignored values: "pragma", "line", maybe others

      case 'd': // declaration
      case 'fdef': // function definition
        var decl = multiDeclaration(tu, tmpdep);
        var id, expr;

        for (var i in decl) {
          if (!decl[i].id) // Struct declaration without typedef
            break;

          if (tu.typedef.length()) {
            id = decl[i].id;
            expr = decl[i].type;
          } else {
            // Resolve pointer
            var dlid = find_dl(decl[i].id);
            if(!dlid) break;

            id = decl[i].id;

            expr = "this['" + dlid + "'].pointer('" + decl[i].id + "'," + decl[i].type + ")" + (decl[i].isFunc ? ".$" : "");

            tmpdep[dlid]=true;
          }

          sym[id] = expr;

          try {
            with(this) eval("this['" + id + "']=" + expr);
          } catch(x) {
            print("Error while evaluating ", id, ":\n", x, "\n");
          }

          dep[id] = tmpdep;

          expsym[id] = true;
        }
        break;

      case 'define': // macro definition
        // Defer processing until later
        expsym[tu.id] = true;
        break;

      } // swtich
    }
  }


  function allmacros() {
    for each(var def in code.define) {
      if(expsym[def.id]) macro.call(that,def);
    }
  }


  function loaddls() {
    // Find dls

    if(default_dl) this['dl ' + (ndl++)] = default_dl;

    for each(var pragma in code.pragma) {
      var match = pragma.match(/JSEXT[ \t]+dl[ \t]+((\"([^\"]*)\")|(main))[ \t]*$/);
      if(match) {
        if(match[3]) this['dl ' + (ndl++)] = Dl(match[3]);
        else if(match[5]) this['dl ' + (ndl++)] = Dl(null);
      }
    }

    for (var i=0; i<ndl; i++) {
      var id='dl '+i;
      var filename = this[id].filename || "";
      if(filename) filename = "'" + filename.replace(/\\/g, "\\\\") + "'";
      sym[id]="Dl("+filename+")";
    }
  }


  function multiDeclaration(decl, dep) {
    var ret=[];
    for(var length = decl.*.length(); length--; ) {
      var declor=decl.*[length];
      var unidec=declaration(decl, dep, declor);
      if(!unidec || !unidec.id) break;
      ret.push(unidec);
    }
    return ret.reverse();
  }


  function declaration(decl, dep, declor) {
    var ret = indir(dirtype(decl, dep), declor, dep);

    if (ret.fd) {
      var callConv = decl..stdcall.length() ? "stdcall" : "cdecl";
      var dirfunc = "Type['function'](" + ret.type + ",[" + ret.params + "]," + ret.elipsis + ",'" + callConv + "')";
      var ret2=indir(dirfunc, ret.fd.*[0], dep);
      if (ret.fd.*[0].name() == "id") ret2.isFunc = true;
      return ret2;
    }

    return ret;
  }


  function find_dl(ident) {
    for(var i = 0; i < ndl; i++) {
      if(that['dl ' + i].symbolExists(ident))
        return "dl " + i;
    }
    return null;
  }


  function dirtype(decl, dep) {
    var type;
    var lasttype;
    var size=0;
    var signed="";

    if(decl.dt.length()) {
      // Defined type
      dep[decl.dt] = true;
      return "this['" + decl.dt + "']";
    }

    if(decl.struct.length()) {
      // Struct declaration
      if (decl.struct.@id.length()) {
        suDeclare(decl.struct);
        dep["struct "+decl.struct.@id]=true;
        return "this['struct "+decl.struct.@id + "']";
      }
      return "Type.struct("+suMembers(decl.struct, dep)+")";
    }

    if (decl.union.length()) {
      // Union declaration
      if (decl.union.@id.length()) {
        suDeclare(decl.union);
        dep["union " + decl.union.@id] = true;
        return "this['union " + decl.union.@id + "']";
      }
      return "Type.union(" + suMembers(decl.union, dep) + ")";
    }

    if (decl.enum.length()) {
      // Enum declaration
      enumMembers(decl);
      return "Type.int";
    }

    if(decl.t.length()) {
      // Primitive declaration or return value of a function
      for each (type in decl.t) {
        if(type == "signed") signed = "signed_";
        if(type == "unsigned") signed = "unsigned_";
        if(type == "char") size -= 2;
        if(type == "short") size--;
        if(type == "__int64") size = 3;
        if(type == "long" || type == "double") size++;
        if(type == "int" || type == "long" || type == "char" || type == "short" || type == "__int64" || type == "signed" || type == "unsigned") lasttype = "int";
        if(type == "float" || type == "double") lasttype = "float";
        if(type == "__builtin_va_list") lasttype = "valist";
        if(type == "void") lasttype = "void";
      }

      if(lasttype == "int") size += 2;

      if(lasttype == "int") return "Type." + signed + ["char", "short", "int", "long", "long_long", "int64"][size];
      if(lasttype == "float") return "Type." + ["float", "double", "long_double"][size];
      return "Type['" + lasttype + "']";
    }
  }


  function suMembers(su, dep) {
    var members=[];
    var n=0;

    for each (var member in su.*) {
      var expr=multiDeclaration(member, dep);

      for (var i=0; i<expr.length; i++) {
        if(!expr[i].id) expr[i].id = "anonymous" + (n++);

        members.push("{" + (expr[i].id ? "name:'" + expr[i].id + "'," : "") + "type:" + expr[i].type + "}");
      }
    }
    return members;
  }


  function enumMembers(decl) {
    var val=0;
    var prevsym;

    for each(var sm in decl.enum.*) {
      if (sm.name()=="id") {
        if(prevsym !== undefined) {
          that[prevsym] = val;
          sym[prevsym] = String(val);
          expsym[prevsym] = true;
          val++;
        }
        prevsym = sm;
      } else {
        var expr = inner_eval(sm);
        try {
          // I'm using .call() because I assume |expr| may explicitly refer to "this"
          (function() { with(this) val = eval(expr); }).call(that);
        } catch(x) {
          print("enumMembers\n", x, "\n");
        }
      }
    }
    if (prevsym !== undefined) {
      that[prevsym]=val;
      sym[prevsym]=String(val);
      expsym[prevsym]=true;
    }
  }


  function suDeclare(su_xml) {
    const id = su_xml.name() + " " + su_xml.@id;
    expsym[id] = true;

    if (!su[id]) {
      var expr="Type."+su_xml.name()+"()";
      su[id]=expr;
      try {
        that[id] = eval(expr);
      } catch(x) {
        print("suDeclare\n", x, "\n", su_xml, "\n");
      }
    }

    if (su_xml.*.length()) {
      var tmpdep={};

      var members=suMembers(su_xml, tmpdep);
      var exprs=[];

      for(var i = 0; i < members.length; i++) exprs.push("this['" + id + "'][" + i + "]=" + members[i]);

      sym[id] = "(" + exprs + ",this['" + id + "'])";

      try {
        (function() { with(this) eval(exprs.join(";")); }).call(that);
      } catch(x) {
        print("suDeclare2\n", x, "\n", su_xml, "\n");
      }

      dep[id]=tmpdep;
    }
  }


  function inner_eval(expr) {
    if(expr.name() == "c") return String(expr);
    if(expr.name() == "id") return sym[expr];
    if(expr.name() == "p") return "(" + inner_eval(expr.*[0]) + ")";
    if(expr.name() == "op") return _inner_eval_op(expr);
  }


  function _inner_eval_op(expr) {
    if(expr.@op == "sizeof" && expr.@type == "t") {
      var decl=expr.*[0];
      var declor=decl[0].*[decl[0].*.length()-1];
      var d=declaration(decl, {}, declor);
      return (function(){with(this){return eval(d.type).sizeof}}).call(that);
    }

    if(expr.@op == "sizeof" && expr.@type == "e") {
      if (that[expr..id])
        return that[expr..id].type.sizeof;
      // assume it's a string
      return Type.char.sizeof * (String(expr..s).length + 1);
    }

    if(expr.*.length() == 1) return expr.@op + inner_eval(expr.*[0]);
    if(expr.*.length() == 2) return inner_eval(expr.*[0]) + expr.@op + inner_eval(expr.*[1]);
    if(expr.*.length() == 3) return inner_eval(expr.*[0]) + "?"  + inner_eval(expr.*[1]) + ":" + inner_eval(expr.*[2]);
  }


  function indir(dirtype, declor, dep) {

    if (!declor) { // void x(int, double)
      return {
        type: dirtype
      }
    }

    var a="";
    var b="";

    switch(String(declor.name())) {

    case 'ptr':
      var asterisk=declor.a;
      while (asterisk.length()) {
        a += "Type.pointer(";
        b += ")";
        asterisk = asterisk.a;
      }
      var inner=indir(a + dirtype + b, declor.*[1], dep);
      return inner;

    case 'ix':
      if (declor.*.length()>1) {
        a = "Type.array(";
        b = "," + eval(inner_eval(declor.*[1])) + ")";
      } else { // x[]
        a = "Type.pointer(";
        b = ")";
      }
      var inner=indir(a + dirtype + b, declor.*[0], dep);
      //     inner.type=a+inner.type+b;
      return inner;

    case 'p':
      return indir(dirtype, declor.*[0], dep);

    case 'bitfield':
      if(declor.*.length() == 1) return indir("Type.bitfield(" + dirtype + "," + eval(inner_eval(declor.*[0])) + ")", <id>$</id>, dep);
      return indir("Type.bitfield(" + dirtype + "," + eval(inner_eval(declor.*[1])) + ")", declor.*[0], dep);

    case 'fd':
      var params=[];
      var param;
      var isvoid=false;

      for each (param in declor.pm.d) {
        var param_type = declaration(param, dep, param.*[param.*.length() - 1]);

        params.push("{'const':" + (param["const"].length() > 0) + "," + (param_type.id ? "name:'" + param_type.id + "'," : "") + "type:" + param_type.type + "}");

        if (param_type.type == "Type['void']") isvoid = true;
      }

      var elipsis = declor.pm.elipsis.length() > 0;

      // Note: Empty argument lists, per the ANSI C standard
      // signify an undefined argument list, not an empty one.
      // Represented by an empty argument list plus elipsis.

      if (params.length == 0) {
        elipsis = true;
      }

      // On the other hand; an argument list with one "void"
      // element is supposed to be an empty argument list.
      // Represented by an empty argument list sans elipsis.

      if (params.length == 1 && isvoid) {
        params = [];
      }

      return {
        type: dirtype || "Type.int", // Default int return
        params: params,
        elipsis: elipsis,
        fd: declor
      }

    case 'id':
      return {
        type: dirtype,
        id: declor
      }

    default:  // typedef void (*) (void)

      return {
        type: dirtype
      }

    }
  }


/*
Tries to coerce a C macro into a JavaScript expression
*/

  var typelist;

  function initmacro() {
    typelist=[];
    for (var i in this) {
      if ((this[i] instanceof Type) && i[0]!='$') {
        typelist[i] = true;
      }
    }
  }

  function macro(macro) {
    if (macro.id in sym)
      return; // Don't overwrite other syms with macros

    // Change 123L into 123

    var v=" "+macro.v+" ";
    v=v.replace(/([^a-zA-Z_0-9])([0-9]+)[lLuUfF]/g,"$1$2");
    v=v.replace(/([^a-zA-Z_0-9])(0x[0-9a-fA-F]+)[lLuU]/g,"$1$2");

    // remove typecasts

    v = v.replace(/\( *([a-zA-Z_][a-zA-Z_0-9]*) *\)/g, function(str, sym) {
        return typelist.hasOwnProperty(sym) ? "" : str;
      });

    v=v.replace(/\([ ]*(unsigned +|signed +)?(long +|short +)?(int|char|long|short|signed|unsigned|double|float)[ ]*\)/,"");

    // change struct indexing
    v=v.replace(/->[ \t]*([a-zA-Z_][a-zA-Z_0-9]*)/g,".member(0,'$1').$$");
    // remove L before string like in L"string"
    v=v.replace(/^([^"])*L"/,'$1"');

    if (macro.pm.length()) { // #define x(y) y

      var params=[];
      for each (var param in macro.pm) {
        if(param != "") // Used to mark empty arg list
          params.push(param);
      }

      var expr = "with(this){function (" + params + ") {return " + v + "}}";

      try {
        this[macro.id] = eval(expr);
        sym[macro.id] = expr;
      } catch(x) {}

    } else if (v=="  ") { // #define x

      this[macro.id]=null;
      sym[macro.id]="null";

    } else { // #define x 42
      v = v.replace(/^[ \t]+|[ \t]+$/g, "");

      if(this['struct ' + v]) { // #define _io_file _IO_FILE
        this['struct ' + macro.id] = this['struct ' + v];
        sym['struct ' + macro.id] = "this['struct " + v + "']";
        if(expsym['struct ' + v]) expsym['struct ' + macro.id] = true;
      }

      if(this['union ' + v]) { // #define _io_file _IO_FILE
        this['union ' + macro.id] = this['union ' + v];
        sym['union ' + macro.id] = "this['union " + v + "']";
        if(expsym['union ' + v]) expsym['union ' + macro.id] = true;
      }

      // alter references to global variables
      //      v=v.replace(/([^0-9A-Za-z_.'"])([a-zA-Z_][a-zA-Z_0-9]*)/g,"$1this['$2']");

      if (this[v]) { // #define stdin stdin
        this[macro.id] = this[v];
        sym[macro.id] = sym[v];
      } else {
        try {
          with(this) eval("this['" + macro.id + "']=" + v);
          sym[macro.id] = v;
        } catch(x) {}
      }
    }
  }
}

})()

