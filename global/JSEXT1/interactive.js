/*
interactive()

A REPL for js.  Statements are evaluated as they are entered. The value of statements that are not terminated by a semicolon are printed on the console.
*/
(function interactive() {

  const normalprompt = "jsx> ";
  const contprompt = ".... ";
  const histfile = environment.HOME + "/.jsext_history";

  var promptTxt = normalprompt;
  var cmdbuf = "";
  var completion_cache = null;

  JSEXT1.libreadline.read_history(histfile);
  JSEXT1.libreadline.rl_completion_entry_function.$ = iterate_completion;

  for(;;) {
    clib.fflush(clib.stdout.$);
    clib.fflush(clib.stderr.$);
    var ptr = JSEXT1.libreadline.readline(promptTxt);
    if(ptr === null) { // EOF
      JSEXT1.libreadline.write_history(histfile);
      print("\n");
      break;
    }

    var line = ptr.string();
    clib.free(ptr);
    if(line) JSEXT1.libreadline.add_history(String(line));
    cmdbuf += line;
    if(isCompilableUnit(cmdbuf)) {
      execline(cmdbuf);
      cmdbuf = "";
      promptTxt = normalprompt;
    } else {
      promptTxt = contprompt;
    }
  }
  return 0;


function execline(line) {
  try {
    line = line.replace(/^[ \t\n\r]+|[ \t\n\r]+$/g, '');
    const rval = do_eval(line);
    if(!/[;}]$/.test(line) && rval !== undefined) print_result(rval);
  } catch(err) {
    if(err.fileName && err.lineNumber) print('Line ' + err.lineNumber + ' in ' + err.fileName + ':');
    print((err.message || err) + '\n');
    if(err.stack) {
      var stack = err.stack.split('\n');
      while(!stack.pop().match(/\bdo_eval\b/));
      stack.pop();
      print(stack.join('\n') + '\n');
    }
  }
}

function print_result(val) {
  // uneval() is a far more useful default than String()
  // Terminals are typically UTF8 these days, and it's certianly better than just truncating each 16-bit value
  print(jsxlib.encodeUTF8(uneval(val)), '\n');
}


function do_eval($$code) {
  return eval($$code);
}


/*
libreadline passes us a word/token to get completions for, and calls us repeatedly to get the full list of results one at a time.  We have to malloc() the return values, and libreadline will free() them.
*/
function iterate_completion(text_ptr, state) {
  if(state == 0) completion_cache = get_completion_list(text_ptr.string());
  if(!completion_cache || state >= completion_cache.length) return null;
  return clib.strdup(String(completion_cache[state]));
}


  // Completes method/field names for expressions like |foo.bar.baz|
  // Doesn't handle e.g.: "foo".<tab><tab> (i.e. tab-completing methods of strings from a literal)
  function get_completion_list(word) {
    try {
      const parts = word.split(".");
      var cur = (function(){ return this; })(); // get the global object
      for(var i = 0; i < parts.length - 1; ++i) {
        if(!(parts[i] in cur)) return;
        cur = cur[parts[i]];
      }

      const ret = [];
      const lastword = parts[parts.length - 1];
      const firstwords = word.substr(0, word.length - lastword.length);
      for(var i in cur) {
        if(i.substr(0, lastword.length) == lastword)
          ret.push(firstwords + i);
      }
      return ret;
    } catch(x) {
    }
  }

})
