function() {
  c=JSEXT1.ftp.connect("sunsite.uio.no");
  c.login("ftp","a@");
  print(c.syst());
}
