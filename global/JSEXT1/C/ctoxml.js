/*

    xml = ctoxml(code)

Runs a C program through a parser and returns an XML object
containing the parse tree.

*/

(function(){

  var libctoxml=Dl($path+JSEXT_config.sep+'libctoxml'+JSEXT_config.dlext);
  var ctoxml=libctoxml.pointer('ctoxml',Type['function'](Type.pointer(Type.char),[{'const':false,name:'C',type:Type.pointer(Type.char)},{'const':false,name:'errorpos',type:Type.pointer(Type.int)}],false,'cdecl')).$;
  var free=libctoxml.pointer('ctoxml_free',Type['function'](Type['void'],[{'const':false,name:'ptr',type:Type.pointer(Type['void'])}],false,'cdecl')).$;

  return function (code) {
    var errorpos=[-1];
    var ret=ctoxml(code,errorpos);
    if (errorpos[0]!=-1) {
      free(ret);
      throw("C.ctoxml: syntax error at "+errorpos);
    }
    
    var ret2=new XML(ret.string());
    free(ret);
    return ret2;
  }
})()