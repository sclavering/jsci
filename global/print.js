/*

      print(string1 [, string2 [, string3 ...]])

  Writes arguments to [[$curdir.stdout]].

 */

function() {
  for (var i=0; i<arguments.length; i++) {
      stdout.write(arguments[i]);
  }
}

