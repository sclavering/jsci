function () {
  var normalprompt="jsext$ ";
  var contprompt="$ ";
  
  var cmdbuf="";
  var cons=new JSEXT1.Console({prompt: normalprompt, complete: complete});

  cons.readline(doline);
  
  function doline(txt) {
    cmdbuf+=txt;
    var ret=execline(cmdbuf);
    if (ret===false) {
      cons.Options.prompt=contprompt;
    } else {
      cons.Options.prompt=normalprompt;
      cmdbuf="";
      if (typeof ret == "string")
	cons.write(ret);
    }

    setTimeout(function(){
      cons.readline(doline);
    },0);
  }

}
