/*

      http.listDir()

  Creates an HTML document containing a listing of the current directory.
  The function is called without arguments, but with a _this_ object
  which must contain the following properties:

  * _requestURL_: A string
  * _filename_: A string
  * _hostdir_: An [[$parent.ActiveDirectory]] object

  The function does not return a value, but prints its result to
  [[stdout]].

 */

function() {
  var parts=decodeURI(this.requestURL);
  var filename=this.filename;

  if (filename.substr(-1)!=JSEXT_config.sep)
    filename+=JSEXT_config.sep;

  var dirlist=$parent.dir(this.hostdir.$path+filename);
  dirlist.sort();
  print('<!DOCTYPE HTML PUBLIC "-//W3C//DTD HTML 3.2 Final//EN">\n');
  print("<html><head><title>"+filename+"</title></head>\n<body><table>\n");
  var parurl=parts.path.replace(/[^/]*\/?$/,"");

  if (parurl!="") {
    var parpath=this.hostdir.$path+parurl.replace("/",JSEXT_config.sep);
    stat=$parent.stat(parpath);
    print("<tr><td><a href='"+parurl+"'>..</a></td><td align=right>"+stat.size+" &nbsp; </td><td>"+stat.mtime+"</td></tr>\n");
  }
  for (var i=0; i<dirlist.length; i++) {
    var stat=$parent.stat(this.hostdir.$path+filename+JSEXT_config.sep+dirlist[i]);
    if ($parent.isdir(this.hostdir.$path+filename+JSEXT_config.sep+dirlist[i]))
      dirlist[i]+="/";
    print("<tr><td><a href='"+dirlist[i]+"'>"+dirlist[i]+"</a></td><td align=right>"+stat.size+" &nbsp; </td><td>"+stat.mtime+"</td></tr>\n");
  }
  print("</table></body></html>");
}
      
