function() {
  var str="abc abc hei hei abc abc hei hei abc abc hei hei";
  var file=new JSEXT1.StringFile;
  var x=new JSEXT1.zlib.Deflator(file,undefined,true);
  x.write(str);
  x.close();
  file.close();
  print(file.tell(),"\n");
  file.seek(0);
  var y=new JSEXT1.zlib.Inflator(file,true);
  print(y.read(),"\n");
}
