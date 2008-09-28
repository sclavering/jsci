function(Options) {

  var Options=windows.MergeOptions(Options, {
    WindowName: "console",
    prompt: "> ",
    Width: 600,
    Height: 371,
    ClassName: "jsext.console",
    Menu: [
	{
	  STRING: "&File",
	  SubMenu: [
	      {
		STRING: "E&xit",
		OnCOMMAND: function() {
		  windows.PostQuitMessage(0);
		}
	      }
	  ]
	},
	{
	  STRING: "F&ormat",
	  SubMenu: [
	      {
		STRING: "&Font...",
		OnCOMMAND: function() {
		  var lf=windows.ChooseFont(console.lf);
		  if (lf) {
		    console.lf=lf;
		    var font=windows.CreateFont(lf);
		    console.Edit.SETFONT(font);
		  }
		}
	      }
	  ]
	}
    ],
    Background: windows.COLOR_BACKGROUND+1
  });
  
  this.Options=Options;
  var console=this;
  
  var Class=windows.RegisterClass(Options.ClassName,Options,
      {
	OVERLAPPEDWINDOW: true,
	VISIBLE: false
      }, {
	Style: windows.Flags.WS
      });
  
  Class.prototype.OnDESTROY=function() {
    windows.PostQuitMessage(0);
    return 0;
  }
  
  this.Frame=new Class(Options);
  
  this.Edit=this.Frame.NewRICHEDIT(
    {
      MULTILINE:true,
      VSCROLL:true, 
      HSCROLL:true
    }
  );
  
  this.lf={FaceName: "Verdana", Height: -12};
  this.font=windows.CreateFont(this.lf)
  this.Edit.SETFONT(console.font);
  
  this.Frame.OnSIZE=
    this.Frame.OnSHOWWINDOW=function() {
      var rect={};
      windows.GetClientRect(console.Frame.Wnd, rect);
      windows.SetWindowPos(console.Edit.Wnd,
			   null,
			   rect.left,
			   rect.top,
			   rect.right,
			   rect.bottom,
			   windows.SWP_NOACTIVATE | windows.SWP_NOZORDER);
    }
  
  this.Frame.OnSETFOCUS=function() {
    windows.SetFocus(console.Edit.Wnd);
  }
  
  this.Edit.prevlinecount=1;
  this.Edit.listening=false;
  this.Edit.pastedbuf="";
  
  this.Edit.OnUPDATE=function() {
    var lc=this.GETLINECOUNT();
    var pos=this.GETSEL()[0];
    var plc=this.prevlinecount;
    this.prevlinecount=lc;
    
    if (lc!=plc) with (this) {
      // Lost or gained lines
      
      var lineno=LINEFROMCHAR(pos);
      var newlines=lc-plc;
      
      if (newlines>0) {
	// Gained lines
	for (var i=lc; i>lineno; i--) {
	  console.promptlen[i]=console.promptlen[i-newlines];
	}
	
	for (var i=0; i<newlines; i++) {
	  console.promptlen[lineno-i]=0;
	}
      } else {
	// Lost lines
	for (var i=lineno+1; i<lc; i++) {
	  console.promptlen[i]=console.promptlen[i-newlines];
	}
	for (var i=lc; i<console.promptlen.length; i++)
	  delete console.promptlen[i];
      }
    }
    
    if (lc>plc && this.GETUNDONAME()!="TYPING" && this.listening) with (this) {
      this.pastedbuf+=GETLINE(lineno-(lc-plc)).substr(console.promptlen[lineno-(lc-plc)])+"\n";
      for (var i=lineno-(lc-plc)+1; i<lineno; i++)
      this.pastedbuf+=GETLINE(i)+"\n";
    }
    
    if (lc>plc && this.GETUNDONAME()=="TYPING" && this.listening) with (this) {
      // User pressed enter
      listening=false;
      var newLine=GETLINE(lineno-1).replace(/[\r\n]/g,"");
      var afterenter=GETLINE(lineno).replace(/[\r\n]/g,"");
      if (afterenter!="") { // Was not at end of line
	this.SETSEL(pos-newLine.length-1, pos-1);
	newLine+=afterenter;
	this.REPLACESEL(newLine);
	this.SETSEL(pos+afterenter.length, pos+2*afterenter.length);
	this.REPLACESEL("");
      }
      console.newLine=this.pastedbuf+newLine.substr(console.promptlen[lineno-1]);
      this.pastedbuf="";
    }
  }
  
  this.Frame.Show();
  this.promptlen=[];
}

