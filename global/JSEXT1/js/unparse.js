/*
      str = unparse (statements [, indent])

  Creates JavaScript source code from a parse tree.

  ### Arguments ###

  * _statements_: An array as returned in _ret.statements_ from
    [[$curdir.parse]].
  * _indent_: A number

  ### Return value ###

  A string

 */

function(list, indent) {

  if (!list || !list.length)
    return "";

  indent = indent || "";

  var escapes = {
            '\b': '\\b',
            '\t': '\\t',
            '\n': '\\n',
            '\f': '\\f',
            '\r': '\\r',
            '"' : '\\"',
            '/' : '\\/',
            '\\': '\\\\'
  },

    escape = function(str) {

// If the string looks like an identifier, then we can return it as is.
// If the string contains no control characters, no quote characters, and no
// backslash characters, then we can simply slap some quotes around it.
// Otherwise we must also replace the offending characters with safe
// sequences.


    if (/["\/\\\x00-\x1f\u0100-\uffff]/.test(str)) {
            return str.replace(/["\/\\\x00-\x1f\u0100-\uffff]/g, function (a) {
                var c = escapes[a];
                if (c) {
                    return c;
                }
                c = a.charCodeAt();
                return '\\u' +
                    ("000"+c.toString(16)).substr(-4);
            });
        }
        return str;
    }


  var unparselist=arguments.callee;

  var i;
  var ret=[];
  for (i=0; i<list.length; i++) {
    ret.push(_unparse(list[i]));
  }
  return indent+ret.join("\n"+indent);

  function arglist(list) {
    var ret=[];
    var i;
    for (i=0; i<list.length; i++) {
      ret.push(unparse(list[i]));
    }
    return ret.join(", ");
  }

  function _unparse(stmt) {
    var up=unparse(stmt);
    return (stmt.label?(stmt.label+": "):"")+up+(stmt.block?"":";");
  }

  function varlist(stmt) {
    var defs=[];
    for (var i=0; i<stmt.vars.length; i++) {
      if (stmt.init[i])
	defs.push(unparse(stmt.vars[i])+" = "+unparse(stmt.init[i]));
      else
	defs.push(unparse(stmt.vars[i]));
    }
    return "var "+defs.join(", ");
  }

  function unparse(stmt) {
    switch(stmt.type) {
    case '(identifier)':
      return stmt.value;
    }

    switch(stmt.id) {
    case '?':
      return unparse(stmt.expr1)+"?"+unparse(stmt.expr2)+":"+unparse(stmt.expr3);
    case '=':
    case '+=':
    case '-=':
    case '*=':
    case '/=':
    case '%=':
    case '&=':
    case '|=':
    case '^=':
    case '<<=':
    case '>>=':
    case '>>>=':
    case '||':
    case '&&':
    case '|':
    case '^':
    case '&':
    case '==':
    case '===':
    case '!=':
    case '!==':
    case '<':
    case '>':
    case '<=':
    case '>=':
    case '<<':
    case '>>':
    case '>>>':
    case 'in':
    case 'instanceof':
    case '-':
    case '*':
    case '/':
    case '%':
    case '+':
    case '!':
    case '~':
    case '++':
    case '--':
    case 'typeof':
      return (stmt.left?unparse(stmt.left)+" ":"")+stmt.id+(stmt.right?" "+unparse(stmt.right):"");
    case '(string)':
      return '"'+escape(stmt.value)+'"'
    case '(number)':
      return stmt.value;
    case 'try':
      var ret="try {\n"+
	unparselist(stmt.tryblock, indent+"  ")+"\n"+indent+"}";
      if (stmt.catchident)
	ret+=" catch ("+unparse(stmt.catchident)+") {\n"+
	  unparselist(stmt.catchblock, indent+"  ")+"\n"+indent+"}";
      if (stmt.finallyblock)
	ret+=" finally {\n"+
	  unparselist(stmt.finallyblock, indent+"  ")+"\n"+indent+"}";
      return ret;
    case 'for':
      var str="for ";
      if (stmt.each)
	str+="each ";
      if (stmt.forin) {
	if (stmt.varloopvar)
	  return str+"(var "+unparse(stmt.loopvar)+" in " +unparse(stmt.expr)+") {\n"+
	    unparselist(stmt.block, indent+"  ")+"\n"+indent+"}";
	else
	  return str+"("+unparse(stmt.loopvar)+" in " +unparse(stmt.expr)+") {\n"+
	    unparselist(stmt.block, indent+"  ")+"\n"+indent+"}";
      } else {
	if (stmt.varlist1)
	  return str+"("+varlist(stmt.varlist1)+"; " +unparse(stmt.expr2)+"; "+arglist(stmt.list3)+") {\n"+
	    unparselist(stmt.block, indent+"  ")+"\n"+indent+"}";
	else
	  return str+"("+(stmt.list1?arglist(stmt.list1):"")+"; " +(stmt.expr2?unparse(stmt.expr2):"")+"; "+(stmt.list3?arglist(stmt.list3):"")+") {\n"+
	    unparselist(stmt.block, indent+"  ")+"\n"+indent+"}";
      }
    case '{':
      var r=[];
      for (var i in stmt.properties)
	if (stmt.properties.hasOwnProperty(i)) {
	  r.push('"'+escape(i)+'": '+unparse(stmt.properties[i]));
	}
      return "{\n"+indent+"  "+r.join(",\n"+indent+"  ")+"\n"+indent+"}";
    case '[':
      if (stmt.elements)
	return "["+arglist(stmt.elements)+"]";
      else
	return unparse(stmt.left)+"["+unparse(stmt.right)+"]";
    case '.':
      return unparse(stmt.left)+"."+stmt.right;
    case '(':
      if (stmt.expr)
	return "("+unparse(stmt.expr)+")";
      else
	return unparse(stmt.func)+"("+arglist(stmt.params)+")";
    case '(regex)':
      return "/"+stmt.value+"/"+stmt.flags;
    case 'new':
      	return "new "+unparse(stmt.func)+"("+arglist(stmt.params)+")";
    case 'if':
      if (stmt.elseblock && stmt.elseblock.length!==undefined)
	return "if ("+unparse(stmt.expr)+") {\n"+
	  unparselist(stmt.ifblock, indent+"  ")+"\n"+indent+"} else {\n"+
	  unparselist(stmt.elseblock, indent+"  ")+"\n"+indent+"}";
      else if (stmt.elseblock)
	return "if ("+unparse(stmt.expr)+") {\n"+
	  unparselist(stmt.ifblock, indent+"  ")+"\n"+indent+"} else "+
	  unparse(stmt.elseblock);
      else
	return "if ("+unparse(stmt.expr)+") {\n"+
	  unparselist(stmt.ifblock, indent+"  ")+"\n"+indent+"}";
    case 'with':
    case 'while':
      return stmt.id+" ("+unparse(stmt.expr)+") {\n"+
	unparselist(stmt.block, indent+"  ")+"\n"+indent+"}";
    case 'do':
      return stmt.id+" {\n"+
	unparselist(stmt.block, indent+"  ")+"\n"+indent+"} while ("+unparse(stmt.expr)+")";
    case 'throw':
    case 'break':
    case 'return':
    case 'delete':
      if (stmt.expr)
	return stmt.id+" "+unparse(stmt.expr);
      else
	return stmt.id;
    case 'switch':
      var cases=[];
      var Case;
      for (var i=0; i<stmt.cases.length; i++) {
	if (stmt.cases[i].id==="default")
	  Case="default:";
	else
	  Case='case '+unparse(stmt.cases[i])+":";
	if (stmt.cases[i].statements)
	  cases.push(Case+"\n"+unparselist(stmt.cases[i].statements, indent+"  ")+"\n");
	else
	  cases.push(Case+"\n");
      }
      return "switch ("+unparse(stmt.condition)+") {\n"+indent+cases.join("\n"+indent)+indent+"}";
    case 'var':
      return varlist(stmt);
    case 'function':
      return "function "+(stmt.func['(name)'] || "")+"("+(stmt.func['(params)'] || "")+") {\n"+
	unparselist(stmt.func['(body)'], indent+"  ")+"\n"+indent+"}";
    case 'debugger':
    case 'continue':
    case 'Infinity':
    case 'NaN':
    case 'true':
    case 'false':
    case 'null':
    case 'this':
    case 'undefined':
    case 'arguments':
    case 'eval':
      return stmt.id;
    default:
      print("unknown "+stmt.toSource()+"\n"+stmt.__proto__.toSource()+"\n");
      return "###";
    }
  }

}
