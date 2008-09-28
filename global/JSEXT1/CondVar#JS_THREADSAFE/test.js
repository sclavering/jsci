function() {
  var cv=new $parent.CondVar;
  var val=0;

  x=$parent.Thread(thread);
  while(cv.wait(function(){print("wakeup ",val,"\n");return val>5;}));

  print("val=",val,"\n");
  x.join();

  function thread() {
    for (var i=0; i<10; i++) {
      print("update ",cv.update(function(){return ++val;}),"\n");
    }
  }
}
