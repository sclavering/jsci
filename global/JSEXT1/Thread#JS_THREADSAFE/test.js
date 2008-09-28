function() {
    var share=[];
  var th=[];
  for (var i=0; i<10; i++)
    th.push(JSEXT1.Thread(thfunc));

  for (var i=0; i<10; i++) {
    th[i].join();
    print("join ",i,"\n");
  }
  
  print(share.length,"\n");

  function thfunc() {
    for (var i=0; i<10; i++) {
	for (var j=0; j<1000; j++)
	    share.push(32);
	print(JSEXT1.Thread.getId()," ",i," ",share.length,"\n");
	for (var j=0; j<1000; j++)
	    share.push(32);
	clib.sleep(1);
    }
  }
}
