function() {
    var c=JSEXT1.smtp.connect("smtp.tele2.no");
    var msg=c.send({
	    from: 'Lure, Dure <sveinb@pvv.org>',
		to: ['Fnure, Gnure <sveinb@pvv.org>',
		     'Brun, bj�rn <sveinb@pvv.org>'],
		subject: "ikke noe rart",
		attachments: {vedlegg1:"streng",
		    vedlegg2:new JSEXT1.File("/tmp/fil")}});
    msg.write("Sl�ving");
    msg.close();
    c.close();
}