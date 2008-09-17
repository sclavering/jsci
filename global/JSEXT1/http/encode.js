/*
  Similar to [[encodeURIComponent]], but also encodes spaces into +.
 */

function (txt) {
  txt=txt.replace(/[^0-9A-Za-z_ ]/g,function(l){return "%"+("0"+l.charCodeAt(0).toString(16)).substr(-2);});
  txt=txt.replace(/ /g,"+");
  return txt;
  //  return encodeURIComponent(txt);
}

