({
  /*
  Parses a URL (or URI, or IRI) into different parts.

    obj = url.parse(string)

  Returns an object containing the following properties:

  * _protocol_: [[String]], the part before ://
  * _username_: [[String]], the part between // and : (or between // and @ if no password is given)
  * _password_: [[String]], the part between : and @
  * _host_: [[String]], the part after // (or after @ if username and password are given)
  * _port_: [[String]], the part after :
  * _path_: [[String]], the part after /
  * _qry_: [[Object]], the part after ?, decoded with [[$curdir.parse_query]]
  * _qryString_: [[String]], the part after ? or undefined if no ?
  * _section_: [[String]], the part after #
  * _fullPath_: [[String]], everything after /

  Note: uses regexps internally.
  */
  parse: function(uri) {
    var proto = uri.match(/^([^:]+):/);
    if(proto && proto[1].length > 1) {
          //                     proto         user      passwd       host      port        path            qry       section
          var parts = uri.match(/([^:]+):\/\/((([^:@]*)(:([^@]*))?@)?(([^:\/]*)(:([0-9]+))?)(\/[^\?#]*)?)(\?([^#]*))?(#(.*))?/);
          return {
            protocol: parts[1],
            username: parts[3] ? parts[4]: undefined,
            password: parts[3] ? parts[6]: undefined,
            host: parts[8],
            port: parts[9] ? parts[10]: undefined,
            path: parts[11] ? parts[11].replace(/%../g, function(nn){return String.fromCharCode(parseInt(nn.substr(1), 16)); }) : undefined,
            qry: this.parse_query(parts[13]),
            qryString: parts[13],
            section: parts[14] ? this._urldecode(parts[15]) : undefined,
            fullPath: parts[11] + (parts[12] || "") + (parts[14] || ""),
          };
    }

    //                      path          qry      section
    var parts = uri.match(/([^\?#]*)?(\?([^#]*))?(#(.*))?/);
    return {
      path: parts[1] ? parts[1].replace(/%../g, function(nn) { return String.fromCharCode(parseInt(nn.substr(1), 16)); }) : undefined,
      qry: this.parse_query(parts[3]),
      qryString: parts[3],
      section: parts[5] ? this._urldecode(parts[5]) : undefined,
      fullPath: uri,
    };
  },


  /*
  obj = url.parse_query(str)
  
  Parses a query string (the part of a URL after the ?), which should be of the
  form "a=b&c=d&e", and returns an object like { a: "b", c: "d", e: null }.
  
  Where a key exists more than once, the last occurrence is used.
  */
  parse_query: function(qry) {
    if(qry == undefined) return;
    const get = {};
    const vars = qry.split("&");
    for(var i = 0; i != vars.length; ++i) this._add_form_var_to(this._urldecode(vars[i]), get);
    return get;
  },

  // Similar to [[decodeURIComponent]], but also decodes + characters to spaces.
  _urldecode: function(qry) {
    if(qry === undefined) return;
    qry = qry.replace(/\+/g, " ");
    qry = qry.replace(/%../g, function(nn) { return String.fromCharCode(parseInt(nn.substr(1), 16)); });
    return qry;
    //    return decodeURIComponent(qry.replace(/\+/g," "));
  },


  // Interpret a single part of a query string, e.g. foo=bar, and set property object.foo = "bar"
  // If the "foo" ends in [], the object.foo is created an array, and "bar" becomes a value in that array.
  // The CGI class handles more complex behavior, like interpreting the dots in a name like 'foo.bar.baz'
  _add_form_var_to: function(name_and_value, obj) {
    const eq_ix = name_and_value.indexOf('='), has_eq = eq_ix !== -1;
    var full_name = has_eq ? name_and_value.slice(0, eq_ix) : name_and_value;
    const val = has_eq ? name_and_value.slice(eq_ix + 1) : '';
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
