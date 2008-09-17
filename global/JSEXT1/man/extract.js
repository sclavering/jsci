/*
         extract.call(obj, filename)
         
     Returns doc for a .js file.

     ### Arguments ###

     * _obj_: The $parent object used when loading the .js file.
     * _filename_: The name of the .js file being extracted

     ### Return value ###

     An object containing the following properties:

     * _$doc_: Documentation for the object being defined by the file.

     If the file returns an object with properties, the return object
     will have properties by the same names, being objects with
     documentation in _$doc_ properties.

     ---

     **Example**

     ---

         / * This object contains some properties * /
         ({
	   / * The answer * /
	     a: 42,
           / * A function which computes the answer * /
	     b: function(){return 42;}
	 })
 
     ---

     Extracting documentation from this file returns the following object:

     ---

         ({
           $doc:{
             text:"This object contains some properties \n"
           },
           a:{
             $doc:{
               text:"The answer \n",
               lineNumber:4
             }
           },
           b:{
             $doc:{
               text:"A function which computes the answer \n",
               lineNumber:6
             }
           }
         })

     ---
*/

function(filename) {

  function extractComments(ret) {
    var hasOwnProperty=Object.prototype.hasOwnProperty;

    if (ret.$doc)
      extractOwnComments(ret);

    for (var i in ret) {
      if (hasOwnProperty.call(ret,i) && i!="$doc")
	extractComments(ret[i]);
    }	

  }

  function stripIndent(str) {
    if (!str)
      return;
    var lines=str.split("\n");
    var minindent=Infinity;
    for (var i in lines) {
      var thisindent=indent.findIndent(lines[i],8);
      if (thisindent>=0 && thisindent < minindent)
	minindent=thisindent;
    }
    for (var i in lines) {
      var line=indent.removeIndent(lines[i],8,minindent);
      if (line) {
	lines[i]=line.string();
	clib.free(line);
      }
    }
    return lines.join("\n")+"\n";
  }

  function extractOwnComments(ret) {
    if (ret.$doc[1]==0) {
      // Find first comment in file
	ret.$doc={text:stripIndent(source.firstComment())};
    } else {
      // Find last comment before line number
	ret.$doc={text:stripIndent(source.commentBefore(ret.$doc[1])), lineNumber:ret.$doc[1]};
    }
  }

  function findchildren(obj, ret, id) {
    var found=false;
    for (var i in def[id]) {

      if (def[id][i][0]==filename) {
	ret[i]={$doc: def[id][i]};
	found=true;
      }
      var childid=libextract.id(obj[i]);
      if (childid && findchildren(obj[i], ret[i], childid))
	found=true;
    }
    return found;
  }
  

  function SourceFile(filename) {
    var file=new $parent.File(filename,"r");
    this.text = file.read();
    file.close();


    this.lines=[];

    var offs=0;
    for (var i=1; ; i++) {
      this.lines[i] = offs;
      var line=this.text.substr(offs).match(/^[^\r\n]*(\r\n|\r|\n)/);
      if (!line) break;
      offs += line[0].length;
    }
  }

  SourceFile.prototype.firstComment=function() {
    var beginComment = this.text.indexOf("/*");
    if (beginComment==-1) return;
    var endComment = this.text.indexOf("*/", beginComment);
    if (endComment==-1) return;
    return this.text.substr(beginComment+2, endComment-beginComment-2);
  }

  SourceFile.prototype.endCommentBefore=function(linenumber) {
    var pos=this.text.lastIndexOf("*/", this.lines[linenumber]);
    if (this.text.substr(pos+2, this.lines[linenumber]-pos-2).replace(/[ \t\r\n]*/,"")) return -1;
    return pos;
  }

  SourceFile.prototype.commentBefore=function(linenumber) {
    var endComment = this.endCommentBefore(linenumber);
    if (endComment==-1) return;
    var beginComment = this.text.lastIndexOf("/*", endComment);
    if (beginComment == -1) return;
    return this.text.substr(beginComment+2, endComment-beginComment-2);
  }


  if ($parent.isrelative(filename))
    filename=$parent.getcwd()+JSEXT_config.sep+filename;

  var container={};

  libextract.start();

  try {

    container.root=(function(){return load.call(this,filename);}).call(this);
    var ex=libextract.stop();

  } catch(x) {

    libextract.stop();
    throw(x);

  }

  var def=[];

  for (var i=0; i<ex.length;) {
    var _filename=ex[i++];
    var linenumber=ex[i++];
    var symbol=ex[i++];
    var parentid=ex[i++];
    if (!(parentid in def))
      def[parentid]=[];
    def[parentid][symbol]=[_filename, linenumber];
  }
  
  var ret={$doc: [filename, 0]};
    
  findchildren(container.root, ret, libextract.id(container.root));
    
  var source=new SourceFile(filename);

  extractComments(ret);
  
  return ret;

}
