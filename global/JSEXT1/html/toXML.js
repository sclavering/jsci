/*
  Converts a string containing an html document into an xml object.
  Cleans up certain errors like attributes without quotes and
  differences between html and xml like the presence of empty tags.
 */

(function() {
  var taglist=[];
  for (var i in emptyTags) {
    taglist.push(i);
  }
  var emptytagregex=new RegExp("<(("+taglist.join("|")+")( [^>]*)?)>","igm");

  return function(html) {
    function attrListFunc(str, start, attrs) {
      attrs=" "+attrs+" ";

      // a="hei" -> a=>
      var strings=[];
      attrs=attrs.replace(/"[^\"]*"|'[^\']*'/gm,function(str) {
	  strings.push(str);
	  return '>';
	});

      // a=xyz -> a=<
      var emptystrings=[];
      attrs=attrs.replace(/\=([^> \n\t]+)/gm,function(str,value) {
	  emptystrings.push(value);
	  return "=<";
	});

      print(attrs,"\n");
      // a -> a=""
      attrs=attrs.replace(/ ([a-zA-Z0-9]+) /gm,function(str,attr) {
	  return " "+attr+'="" ';
	});

      // a=> -> a="hei"
      attrs=attrs.replace(/>/gm,function() {
	  return strings.shift();
	});

      // a=< -> a="xyz"
      attrs=attrs.replace(/</gm,function() {
	  return '"'+emptystrings.shift()+'"';
	});

      return "<"+start+attrs.substr(1,attrs.length-2)+">";
    }

    // remove doctype
    html=html.replace(/^<![^>]*>/,"");

    // remove /> from everywhere, shouldn't be there anyway
    html=html.replace(/\/>/gm,">");

    // do something to empty and unquoted attributes
    html=html.replace(/<([^/][a-zA-Z0-9]+ )([^>]*)>/gm,attrListFunc);

    // do something to empty tags
    html=html.replace(emptytagregex,"<$1/>");
    
    var oldignorews=XML.ignoreWhitespace;
    XML.ignoreWhitespace=false;
    var oldignorecm=XML.ignoreComments;
    XML.ignoreComments=false;
    var ret=new XML(html);
    XML.ignoreWhitespace=oldignorews;
    XML.ignoreComments=oldignorecm;
    return ret;
  }
})()
