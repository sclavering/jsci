  /*

    Decodes a URI into different parts.

        obj = http.decodeURI(string)
    
    Returns an object containing the following properties:
    
    * _protocol_: [[String]], the part before ://
    * _username_: [[String]], the part between // and : (or between // and @ if no password is given)
    * _password_: [[String]], the part between : and @
    * _host_: [[String]], the part after // (or after @ if username and password are given)
    * _port_: [[String]], the part after :
    * _path_: [[String]], the part after /
    * _qry_: [[Object]], the part after ?, decoded with [[$curdir.decodeQry]]
    * _qryString_: [[String]], the part after ? or undefined if no ?
    * _section_: [[String]], the part after #
    * _fullPath_: [[String]], everything after /
    
    Makes use of this RegExp:
    
        proto         user      passwd       host     
        /([^:]+):\/\/((([^:@]*)(:([^@]*))?@)?(([^:\/]*)  ...
    
        port        path           qry       section
        (:([0-9]+))?)(\/[^\?]*))?(\?([^#]*))?(#(.*))?/
  */
    
  function(uri) {
    var proto=uri.match(/^([^:]+):/);
    if (proto && proto[1].length>1)
      switch(proto[1]) {
      case 'mailto':
	//                   proto  user      qry       
	var parts=uri.match(/mailto:([^?]*)(\?(.*))?/);
	var ret=decodeQry(parts[3] || "");
	ret.protocol="mailto";
	ret.to=parts[1].split(",");
	if (ret.cc)
	  ret.cc=ret.cc.split(",");
	if (ret.bcc)
	  ret.bcc=ret.bcc.split(",");
	return ret;
	break;
      default:
	//                   proto         user      passwd       host      port        path            qry       section
        var parts=uri.match(/([^:]+):\/\/((([^:@]*)(:([^@]*))?@)?(([^:\/]*)(:([0-9]+))?)(\/[^\?#]*)?)(\?([^#]*))?(#(.*))?/);
        return {
	  protocol:parts[1],
	  username:parts[3]?parts[4]:undefined,
	  password:parts[3]?parts[6]:undefined,
	  host:parts[8],
	  port:parts[9]?parts[10]:undefined,
	  path:parts[11]?parts[11].replace(/%../g,function(nn){return String.fromCharCode(parseInt(nn.substr(1),16))}):undefined,
	  qry:decodeQry(parts[13]),
	  qryString:parts[13],
	  section:parts[14]?this.decode(parts[15]):undefined,
	  fullPath:parts[11]+(parts[12] || "")+(parts[14] || "")
	};
      }

    //                   path          qry       section
    var parts=uri.match(/([^\?#]*)?(\?([^#]*))?(#(.*))?/);
    return {
      path:parts[1]?parts[1].replace(/%../g,function(nn){return String.fromCharCode(parseInt(nn.substr(1),16))}):undefined,
      qry:decodeQry(parts[3]),
      qryString:parts[3],
      section:parts[5]?this.decode(parts[5]):undefined,
      fullPath:uri
    };
  }

