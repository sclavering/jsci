function() {
    var db=new JSEXT1.Mysql({db:'jsext', user:'root'});
    try {db.exec("drop table test");}catch(x){}
    db.exec("create table test (a int, b longtext NOT NULL, c blob)");
    db.exec("insert into test values (?,?,?)",5,"hæi\r\n\r\ndu",new JSEXT1.StringFile("hæi"));
    var v=db.query("select b, c from test")[0];
    for (var i=0; i<v.b.length; i++)
	print(v.b.charCodeAt(i),"\n");
    print("\n");
    v.c=v.c.read();
    for (var i=0; i<v.c.length; i++)
	print(v.c.charCodeAt(i),"\n");
    print(db.query("select length(b), length(c) from test")[0].toSource());
    db.exec("drop table test");
    db.close();
}