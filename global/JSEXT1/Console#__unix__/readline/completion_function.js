(function(curdir) {
  
  return function(func) {
    lib.rl_completion_entry_function.$=iterate_completion;
    curdir.completion=func
  }

})(this)
