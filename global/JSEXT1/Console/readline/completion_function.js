(function(curdir) {
  
  return function(func) {
    JSEXT1.libreadline.rl_completion_entry_function.$=iterate_completion;
    curdir.completion=func
  }

})(this)
