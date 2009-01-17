(function() {
  if (JSEXT_config.JS_THREADSAFE) 
    return function() {
      var curthread=JSEXT1.Thread.getCurrent();
      if (!curthread.events)
	curthread.events=[];
      return curthread.events;
    }

  else

    return function() {
      if (!$curdir.event_array)
	$curdir.event_array=[];
      return $curdir.event_array;
    }

 })()
