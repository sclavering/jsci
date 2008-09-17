/*
      str = trim (str2)

  Removes whitespace from the beginning and end of a string.
 */


function(string) {
  return string.replace(/^[ \t\n\r]*/,"").replace(/[ \t\n\r]*$/,"");
}
