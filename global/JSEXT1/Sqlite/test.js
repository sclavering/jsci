function() {
    x=new $curdir;
    x.exec("create table hoi (id int, text varchar(255), primary key(id))");
    x.exec("insert into hoi values (5,'svein')");
    x.exec("insert into hoi values (?,?)",6,'svein');
    var p=x.prepare("insert into hoi values (?,?)");
    p.exec(7,"svein");
    p.free();

    var res=x.prepare("select * from hoi");
    var row;
    while (row=res.row()) {
	print(row.toSource(),"\n");
    }
    res.free();

    x.exec("create table hai (a blob, b text)");
    var db=x;
    db.exec("create table test (a int, b text, c blob)");
    db.exec("insert into test values (?,?,?)",5,"hæi",new JSEXT1.StringFile("hæi"));
    var v=db.query("select b, c from test")[0];
    for (var i=0; i<v.b.length; i++)
	print(v.b.charCodeAt(i),"\n");
    print("\n");
    v.c=v.c.read();
    for (var i=0; i<v.c.length; i++)
	print(v.c.charCodeAt(i),"\n");
    db.exec("drop table test");
    x.close();


}