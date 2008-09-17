  /*
        normalize(filename) -> filename

    Removes .. and . from the path
  */
  
  function(filename) {
    var parts=filename.split(JSEXT_config.sep);
    var topmost=0;
    for (var i=0; i<parts.length; i++) {
      switch(parts[i]) {
      case JSEXT_config.pardir:
	if (i>topmost) {
	  parts.splice(i-1,2);
	  i-=2;
	} else if (i==topmost && parts[0]=="")
	  throw new Error("Invalid path");
	else if (i==topmost)
	  topmost++;
	break;
      case JSEXT_config.curdir:
	parts.splice(i,1);
	i-=1;
	break;
      case "":
	if (i==0)
	  topmost=1;
	else {
	  parts.splice(i,1);
	  i-=1;
	}
	break;
      default:
	break;
      }
    }
    if (parts.length==1 && parts[0]=="")
      return JSEXT_config.sep;
    return parts.join(JSEXT_config.sep);
  }
