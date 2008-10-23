({
  /*
    Similar to [[decodeURIComponent]], but also decodes + characters
    to spaces.
  */
  decode: function(qry) {
    if(qry === undefined) return;
    qry = qry.replace(/\+/g, " ");
    qry = qry.replace(/%../g, function(nn) { return String.fromCharCode(parseInt(nn.substr(1), 16)); });
    return qry;
    //    return decodeURIComponent(qry.replace(/\+/g," "));
  },


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
  decodeURI: function(uri) {
    var proto = uri.match(/^([^:]+):/);
    if(proto && proto[1].length > 1) {
      switch(proto[1]) {
        case 'mailto':
          //                   proto  user      qry
          var parts = uri.match(/mailto:([^?]*)(\?(.*))?/);
          var ret = this.decodeQry(parts[3] || "");
          ret.protocol="mailto";
          ret.to = parts[1].split(",");
          if(ret.cc) ret.cc = ret.cc.split(",");
          if(ret.bcc) ret.bcc = ret.bcc.split(",");
          return ret;
          break;
        default:
          //           proto         user      passwd       host      port        path            qry       section
          var parts = uri.match(/([^:]+):\/\/((([^:@]*)(:([^@]*))?@)?(([^:\/]*)(:([0-9]+))?)(\/[^\?#]*)?)(\?([^#]*))?(#(.*))?/);
          return {
            protocol: parts[1],
            username: parts[3] ? parts[4]: undefined,
            password: parts[3] ? parts[6]: undefined,
            host: parts[8],
            port: parts[9] ? parts[10]: undefined,
            path: parts[11] ? parts[11].replace(/%../g, function(nn){return String.fromCharCode(parseInt(nn.substr(1), 16)); }) : undefined,
            qry: this.decodeQry(parts[13]),
            qryString: parts[13],
            section: parts[14] ? this.decode(parts[15]) : undefined,
            fullPath: parts[11] + (parts[12] || "") + (parts[14] || ""),
          };
      }
    }

    //                   path          qry       section
    var parts = uri.match(/([^\?#]*)?(\?([^#]*))?(#(.*))?/);
    return {
      path: parts[1] ? parts[1].replace(/%../g, function(nn){ return String.fromCharCode(parseInt(nn.substr(1), 16)); }) : undefined,
      qry: this.decodeQry(parts[3]),
      qryString: parts[3],
      section: parts[5] ? this.decode(parts[5]) : undefined,
      fullPath: uri,
    };
  },


  /*
    Decodes a query string.

        obj = http.decodeQry(str)

    _str_ should be the part of an url to the right of the question mark, i.e.

        foo=a&two&bar=b

    Will return an object containing the properties

        {
          foo:"a",
          two:null,
          bar:"b"
        }

    Query strings not matching this pattern are decoded as if they were
    the innards of JSON brackets, i.e.

        1,2,"three",[4]

    Will return the array

        [1,2,"three",[4]]
  */
  decodeQry: function(qry) {
    if(qry == undefined) return;
    try {
      return $parent.decodeJSON("[" + this.decode(qry) + "]");
    } catch(x) {
      // normal a=x&b=y format
      const get = {};
      const vars = qry.split("&");
      for(var i = 0; i != vars.length; ++i) this._add_form_var_to(this.decode(vars[i]), get);
      return get;
    }
    return {};
  },

  // Interpret a single part of a query string, e.g. foo=bar, and set property object.foo = "bar"
  // If the "foo" ends in [], the object.foo is created an array, and "bar" becomes a value in that array.
  // The CGI class handles more complex behavior, like interpreting the dots in a name like 'foo.bar.baz'
  _add_form_var_to: function(name_and_value, obj) {
    const eq_ix = name_and_value.indexOf('=');
    const val = name_and_value.slice(eq_ix + 1);
    var full_name = name_and_value.slice(0, eq_ix);
    const is_array_var = /\[\]$/.test(full_name);
    if(is_array_var) full_name = full_name.slice(0, full_name.length - 2);

    // Don't allow __proto__ or anything like that to be replaced
    if(full_name in Object.prototype) return;

    if(is_array_var) {
      // note: use |new Array| here rather than just [] so that the instanceof works
      if(!((full_name in obj) && obj[full_name] instanceof Array)) obj[full_name] = new Array();
      obj[full_name].push(val);
    } else {
      obj[full_name] = val;
    }
  },
})
