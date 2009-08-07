(function() {

return function(filename) {
  const srccode = runcpp(filename);
  const parser = new CParser();
  const xml = parser.Parse(srccode);
  const parsed = getInfoFromXML(xml, parser);
  return jswrapper(parsed);
};


function runcpp(filename) {
  /*
  cpp is assumed to be the one from GCC.  Tested using "cpp (GCC) 4.2.4 (Ubuntu 4.2.4-1ubuntu3)"
  
  -dD
    keeps "#define FOO 42" and similar, so we can include such constants in the .jswrapper
    expands macros, so that function prototypes are in the form we can parse
  -undef stops cpp leaving __extension__ crap in libmysql.h's output (a gcc-ism)
  --std=gnu99 is just to try and get clib.h's output closer to what our old libcpp was doing
  */
  return JSEXT1.File.read("/usr/bin/cpp '" + filename + "' -dD -undef --std=gnu99 |");
}
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
  var getters = "";
  for(var i in fragment) {
    var code = fragment[i];
    if(/^(?:-?\d+|null|undefined)$/.test(code)) {
      getters += 'obj[' + uneval(i) + '] = ' + code + ';\n';
    } else {
      getters += 'obj.__defineGetter__(' + uneval(i) + ', function() { return getter_helper.call(obj, ' + uneval(i) + ', ' + uneval(fragment[i]) + '); });\n';
    }
  }

  return "\n\
(function(){ \n\
 \n\
function getter_helper(key, code) { \n\
  delete obj[key]; \n\
  return obj[key] = eval(code); \n\
} \n\
 \n\
const obj = {}; \n\
" + getters + " \n\
return obj; \n\
})()\n";
};


/*
obj = getInfoFromXML(code)

Use the XML created by CParser to find declarations of functions, typedefs, structs, global variables etc (and also #define's), and generate a mapping from symbols to javascript code using Type/Dl/Pointer to provide access to them.

To actually call a function or read/write a global variable, the library it is from has to be dlopen()'d (via the Dl constructor).  The .h file can use pragmas to add libraries to the list to be dlopen()'d.  They will be searched in the order specified, though the location of the #pragma is otherwise unimportant (it can come after a symbol from that .so file).

 - #pragma JSEXT dl "filename.so"
    Try to resolve symbols in filename.so
 - #pragma JSEXT dl main
    Try resolving symbols in the main program binary.  This can be used for libc, the SpiderMonkey API, etc.
*/
function getInfoFromXML(code, parser) {
  const live = {}; // Contains the evaluated code. Used during processing to evaluate sizeof() expressions.
  const sym = {};  // Contains symbols
  const su = {};   // Contains declarations of structs and unions
  const typelist = [];
  var ndl = 0;     // Count number of dl files

  loaddls();
  parse_inner();
  initmacro();
  allmacros();

  const src = {};
  for(let i in su) src[i] = sym[i] ? "this['" + i + "']=" + su[i] + ";" + sym[i] : su[i];
  for(let i in sym) if(!src[i]) src[i] = sym[i];
  return src;


  function parse_inner() {
    for each(let tu in code.*) {
      switch(String(tu.name())) {
      case 'd': // declaration
      case 'fdef': // function definition
        var decl = multiDeclaration(tu);
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

            expr = "this['" + dlid + "'].pointer('" + decl[i].id + "'," + decl[i].type + ")";
          }

          sym[id] = expr;

          liveeval("this['" + id + "']=" + expr);
        }
        break;
      } // swtich
    }
  }


  function loaddls() {
    for each(var pragma in parser.preprocessor_directives) {
      var match = pragma.match(/^#pragma[ \t]+JSEXT[ \t]+dl[ \t]+(?:"([^"]*)"|(main))[ \t]*$/);
      if(!match) continue;
      var id = 'dl ' + (ndl++);
      var filename = match[1] || ""
      live[id] = filename ? Dl(match[1]) : Dl();
      sym[id] = "Dl(" + (filename ? "'" + filename.replace(/\\/g, "\\\\") + "'" : "") + ")";
    }
  }


  function multiDeclaration(decl) {
    var ret=[];
    for(var length = decl.*.length(); length--; ) {
      var declor=decl.*[length];
      var unidec = declaration(decl, declor);
      if(!unidec || !unidec.id) break;
      ret.push(unidec);
    }
    return ret.reverse();
  }


  function declaration(decl, declor) {
    var ret = indir(dirtype(decl), declor);
    if(!ret.fd) return ret;
    var callConv = decl..stdcall.length() ? "stdcall" : "cdecl";
    var dirfunc = "Type['function'](" + ret.type + ",[" + ret.params + "]," + ret.elipsis + ",'" + callConv + "')";
    return indir(dirfunc, ret.fd.*[0]);
  }


  function find_dl(ident) {
    for(var i = 0; i < ndl; i++) {
      if(live['dl ' + i].symbolExists(ident))
        return "dl " + i;
    }
    return null;
  }


  function dirtype(decl) {
    decl = decl.basic_type;

    // Defined type
    if(decl.dt.length()) return "this['" + decl.dt + "']";

    if(decl.struct.length()) {
      // Struct declaration
      let id = String(decl.struct.@id);
      if(id) {
        suDeclare(decl.struct);
        return "this['struct " + id + "']";
      }
      return "Type.struct(" + suMembers(decl.struct) + ")";
    }

    if(decl.union.length()) {
      // Union declaration
      let id = String(decl.union.@id);
      if(id) {
        suDeclare(decl.union);
        return "this['union " + id + "']";
      }
      return "Type.union(" + suMembers(decl.union) + ")";
    }

    if (decl.enum.length()) {
      // Enum declaration
      enumMembers(decl);
      return "Type.int";
    }

    if(decl.t.length()) {
      let lasttype, signed = "", size = 0;
      // Primitive declaration or return value of a function
      for each(let type in decl.t) {
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


  function suMembers(su) {
    var members=[];
    var n=0;

    for each (var member in su.*) {
      var expr = multiDeclaration(member);

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
          live[prevsym] = val;
          sym[prevsym] = String(val);
          val++;
        }
        prevsym = sm;
      } else {
        val = eval_expr(sm);
      }
    }
    if (prevsym !== undefined) {
      live[prevsym] = val;
      sym[prevsym]=String(val);
    }
  }


  function suDeclare(su_xml) {
    const struct_or_union = su_xml.name();
    const id = struct_or_union + " " + su_xml.@id;

    if (!su[id]) {
      const expr = su[id] = "Type." + struct_or_union + "()";
      live[id] = liveeval(expr);
    }

    if(!su_xml.*.length()) return;

    // structs can be have members of the same struct type, so our getter creates a stub, then replace its innards

    var members = suMembers(su_xml);
    const suref = "this['" + id + "']";
    sym[id] = "Type.replace_members(" + suref + "," + members.join(',') + ")," + suref;
    liveeval(sym[id]);
  }


  // Evaluate a constant expression.  Typical scenarios are array lengths, and macro constants using shift exprs.
  function eval_expr(expr) {
    switch(String(expr.name())) {
      case "literal_string": return String(expr);
      case "literal_char": return String(expr);
      case "literal_number": {
        // xxx treat unsigned literals differently, to fix e.g. "#define RLIM_INFINITY ((unsigned long int)(~0UL))"
        let str = String(expr); str = str.replace(str[1] && str[1] == 'x' ? /[lLuU]+$/ : /[fFlLuU]+$/, '');
        // We use eval because Number() treats literals with leading 0 as decimal
        return eval(str);
      }
      case "id": {
        // Some .h files use #define like a typedef, but returning a Type wouldn't work, since its uneval() is useless.
        let thing = eval(sym[expr]);
        if(thing instanceof Type) return undefined;
        return thing;
      }
      case "p":
        return eval_expr(expr.*[0]);
      case "sizeof_type": {
        let decl = expr.*[0];
        let declor = decl[0].*[decl[0].*.length() - 1];
        let d = declaration(decl, declor);
        return Type.sizeof(liveeval(d.type));
      }
      case "sizeof_expr":
        if(live[expr..id]) return Type.sizeof(live[expr..id].type);
        // assume it's a string
        return Type.sizeof(Type.char) * (String(expr..s).length + 1);
      case "prefix_op":
      case "postfix_op":
        return eval_unary(expr.@op, eval_expr(expr.*[0]));
      case "binary_op":
        return eval(eval_expr(expr.*[0]) + ' ' + expr.@op + ' ' + eval_expr(expr.*[1]));
      case "conditional_op":
        return eval_expr(expr.*[0]) ? eval_expr(expr.*[1]) : eval_expr(expr.*[2]);
      case "cast":
        return eval_expr(expr.*[1]); // ignore the cast
    }
    throw Error("eval_expr called on " + uneval(expr));
  }

  function eval_unary(op, val) {
    switch(String(op)) {
      case "++": return val + 1;
      case "--": return val - 1;
      case "-": return -val;
      case "~": return ~val;
    }
    throw Error("eval_unary called on '" + op + "' and " + uneval(val));
  }


  function indir(dirtype, declor) {
    if (!declor) { // void x(int, double)
      return {
        type: dirtype
      }
    }

    var a="";
    var b="";

    switch(String(declor.name())) {

    case 'ptr': {
      const ptrcount = declor.a.length();
      for(let i = 0; i != ptrcount; ++i) {
        a += "Type.pointer(";
        b += ")";
      }
      return indir(a + dirtype + b, declor.*[ptrcount]);
    }

    case 'ix':
      if (declor.*.length()>1) {
        a = "Type.array(";
        b = "," + uneval(eval_expr(declor.*[1])) + ")";
      } else { // x[]
        a = "Type.pointer(";
        b = ")";
      }
      return indir(a + dirtype + b, declor.*[0]);

    case 'p':
      return indir(dirtype, declor.*[0]);

    case 'bitfield':
      if(declor.*.length() == 1) return indir("Type.bitfield(" + dirtype + "," + uneval(eval_expr(declor.*[0])) + ")", <id>$</id>);
      return indir("Type.bitfield(" + dirtype + "," + uneval(eval_expr(declor.*[1])) + ")", declor.*[0]);

    case 'fd':
      var params=[];
      var isvoid=false;

      for each(var param in declor.pm.d) {
        var param_type = declaration(param, param.*[param.*.length() - 1]);
        params.push(param_type.type);
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

  function initmacro() {
    for(let i in live) if((live[i] instanceof Type) && i[0]!='$') typelist[i] = true;
  }


  function allmacros() {
    for each(var thing in parser.preprocessor_directives) {
      let m = thing.match(/^#define[ \t]+([^ \t]+)[ \t]+(.*)\s*$/);
      if(!m) continue;
      let id = m[1], expansion = m[2];
      // Ignore function-like macros, and macros that are redefining a symbol we already have
      if(id.indexOf("(") != -1 || sym[id]) continue;
      if(!expansion) {
        sym[id] = "null";
        continue;
      }
      try {
        let exml = parser.Parse(expansion, 'expr');
        let v = eval_expr(exml);
        if(v !== undefined) sym[id] = uneval(live[id] = v);
      } catch(e) {
//         print("Error parsing this define:\n  ", thing, "\n", e, "\n", e.stack, "\n\n\n");
      }
    }
  }


  function liveeval(expr) {
    try {
      return (function() { with(live) return eval(expr); }).call(live);
    } catch(e) {}
    return undefined;
  }
}

})()

