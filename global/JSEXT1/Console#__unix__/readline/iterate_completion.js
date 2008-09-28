(function(curdir) {
  var fullname=[];
  while (curdir.$parent) {
    fullname.push(curdir.$name);
    curdir=curdir.$parent;
  }

  fullname=fullname.reverse().join(".");

  return Function("text","state"," \
  var readline="+fullname+"; \
  if (state==0) \
    readline.completion_list=readline.completion(text.string()); \
  if (typeof readline.completion_list != 'object' || \
      readline.completion_list === null || \
      state >= readline.completion_list.length) \
    return null; \
  return clib.strdup(String(readline.completion_list[state])); \
");

})(this)