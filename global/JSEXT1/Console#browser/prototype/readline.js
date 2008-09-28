function(func, startstring, startpos) {
  var readline=arguments.callee;

  this.write(this.Options.prompt);
  var self=this;
  var text=this.element.childNodes[this.element.childNodes.length-1];
  var input=document.createElement("INPUT");
  input.type="text";
  if (startstring)
    input.value=startstring;
  if (startpos)
    setpos(startpos);
  setTimeout(setsize,0);
  this.element.onresize=setsize;
  input.className=Options.className;
  this.element.appendChild(input);
  input.onkeydown=keydown;
  input.onkeypress=keypress;
  setTimeout(function(){input.focus()},0);
  var historyline=this.history.length;
  this.history.push("");
  var oldOnClick=this.element.onclick;
  this.element.onclick=function(e) {
    input.focus();
    if (oldOnClick)
      oldOnClick.call(this,e);
  }

  var ignorepress=0;

  function setsize() {
    var width=self.element.clientWidth;
    if (text.tagName=="SPAN")
      width-=text.offsetLeft+text.offsetWidth;
    width-=8; //!!
    input.style.width=width;
    return true;
  }

  function keypress(e) {
    if (ignorepress)
      ignorepress--;
    else
      return keyfunc.call(this, e);
  }

  function keydown(e) {
    ignorepress=1;
    return keyfunc.call(this, e);
  }

  function keyfunc(e) {
    var input=this;
    var keynum;

    if(window.event) // IE
	keynum = window.event.keyCode;
    else if(e.which) // Netscape/Firefox/Opera
	keynum = e.which;

    if (keynum===9) { // tab
      if (self.complete===false) {
      } else if (self.complete) { // second tab
	enter();
	var line=input.value;
	var list=document.createElement("DIV");
	list.className=self.Options.className;
	list.innerHTML=self.complete.join("   ");
	self.element.appendChild(list);
	setTimeout(function(){
	  readline.call(self, func, line, line.length);
	},0);
	self.complete=false;
      } else if (self.Options.complete!==undefined) {
	var complete=self.Options.complete(input.value);
	if (complete && complete.length==1) {
	  input.value=complete[0];
	  movetoend();
	} else if (complete.length>1) {
	  self.complete=complete;
	}
      }
    } else {
      delete self.complete;
    }

    switch(keynum) {
    case 38: // arrow up
      self.history[historyline]=this.value;
      if (historyline>0)
	historyline--;
      this.value=self.history[historyline];
      movetoend();
      break;
    case 40: // arrow down
      self.history[historyline]=this.value;
      if (historyline<self.history.length-1)
	historyline++;
      this.value=self.history[historyline];
      movetoend();
      break;
    case 13: // enter
      enter();
      self.history[self.history.length-1]=input.value;
      if (input.value==="")
	self.history.pop();
      func(this.value);
      break;
    }

    function enter() {
      self.element.removeChild(input);
      self.write(input.value+"\n");
      self.element.onclick=oldOnClick;
    }

    return true;
  }

  function setpos(pos) {
    if ("selectionStart" in input) { // opera etc.
      input.selectionStart=pos;
      input.selectionEnd=pos;
    } else if ("selection" in document) { // ie
      setTimeout(function(){
	var rng=input.createTextRange();
	rng.moveStart('character', pos);
	rng.select();
      },0);
    }
  }
  
  function movetoend() {
    setpos(input.value.length);
  }

}
