/*
    file = File.tmp() 
       
Return a new file object opened in update mode ("w+"). The file has no directory entries associated with it and will be automatically deleted once there are no file descriptors for the file. Availability: Unix, Windows. 
*/
  
  
  function() {
    return new $curdir(clib.tmpfile());
  }
