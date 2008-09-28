function(str) {
  str=String(str);
  var lines=str.split("\n");
  if (this.element.childNodes.length) {
    var lastline=this.element.childNodes[this.element.childNodes.length-1];
    if (lastline.tagName==="SPAN") {
      lines[0]=lastline.firstChild.data+lines[0];
      this.element.removeChild(lastline);
    }
  }
  for (var i=0; i<lines.length-1; i++) {
    var line=document.createElement("DIV");
    line.className=Options.className;
    var inner;
    if (lines[i]=="")
      inner=String.fromCharCode(160);
    else
      inner=lines[i].replace(/  /g,String.fromCharCode(32,160));
    line.appendChild(document.createTextNode(inner));
    this.element.appendChild(line);
  }
  if (lines[i]!=="") {
    var line=document.createElement("SPAN");
    line.className=Options.className;
    line.appendChild(document.createTextNode(lines[i]));
    this.element.appendChild(line);
  }
}
