/*
    write(uri, [...,] content)

Opens _uri_ using [[open]], writes entire contents and closes.
_content_ should be a string. Any parameters between the uri and
comment are passed to the [[open]] function.
*/

function (uri) {
  var content;
  if (arguments.length>1) {
    content=arguments[arguments.length-1];
    arguments[arguments.length-1]="w";
  }
  var f=open.apply(null,arguments);
  if (content)
    f.write(content);
  f.close();
}
