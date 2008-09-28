/*
  An object containing the mime types of the most common filename
  extensions. Example: type.txt = "text/plain".

  Tries to read /etc/mime.types. If that fails, falls back to
  a short built-in list.
*/


(function() {

  try {
    var ret={};

    var f=$parent.read("/etc/mime.types").split("\n");
    for (var i in f)
      if (!f[i].match(/^#/)) {
	var l=f[i].split(/[ \t]+/g);
	for (var j=1; j<l.length; j++)
	  ret[l[j]]=l[0];
      }
    ret.jsx="application/javascript";//ret.js;
    return ret;
  } catch (x) {
    return {
    txt: "text/plain",
    html: "text/html",
    js: "application/javascript",
    jsx: "application/javascript",
    mp3: "audio/mpeg",
    jpg: "image/jpeg",
    gif: "image/gif",
    png: "image/png",
    xml: "text/xml",
    css: "text/css"
    };
  }

})()
