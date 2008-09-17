function() {
  var c1=new JSEXT1.Postgresql({});
  try {c1.query("drop database jsx_test")}catch(x){print(x,"\n")}
  c1.query("create database jsx_test");

  var c=new JSEXT1.Postgresql({dbname:'jsx_test'});
  c.query("create table fnomm ( a int, b int, c timestamptz, d bytea, e text)");
  c.query("insert into fnomm values (5,7,?,?,?)",new Date(), "a\0b",4);
  c.query("insert into fnomm values (5,$1,now(),'heo','txt')",99);
  print(c.query("select * from fnomm").toSource(),"\n");
  print(c.query("select length(d) as d , length(e) as e from fnomm").toSource(),"\n");
  c.query("drop table fnomm");

  var db=c;
    try {db.exec("drop table test");}catch(x){}
    db.exec("create table test (a int, b text, c bytea)");
    db.exec("insert into test values (?,?,?)",5,"hæi",new JSEXT1.StringFile("hæi"));
    var v=db.query("select b, c from test")[0];
    for (var i=0; i<v.b.length; i++)
	print(v.b.charCodeAt(i),"\n");
    print("\n");
    v.c=v.c.read();
    for (var i=0; i<v.c.length; i++)
	print(v.c.charCodeAt(i),"\n");
    db.exec("drop table test");
    db.close();

  c.close();

  c1.query("drop database jsx_test");

  c1.close();

}
