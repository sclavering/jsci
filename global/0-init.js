(function() {
  jsxlib.load.call(this, "0-ffi.js");
  this.jsxcore = jsxlib; // backwards compatibility for CGI/FastCGI programs

  clib = load("clib.jswrapper");
  for(var i = 0; clib['dl ' + i]; i++) ;

  const path = jsxlib.environment.JSX_HOME + '/global/';
  this.JSEXT1 = {
    $path: path + 'JSEXT1',

    encodeUTF8: jsxlib.encodeUTF8,
    decodeUTF8: jsxlib.decodeUTF8,
    encodeJSON: jsxlib.encodeJSON,
    decodeJSON: jsxlib.decodeJSON,
    encodeBase64: jsxlib.encodeBase64,
    decodeBase64: jsxlib.decodeBase64,
  };

  const ActiveDirectory = JSEXT1.ActiveDirectory = load('JSEXT1/ActiveDirectory.js');
  JSEXT1.os = ActiveDirectory.handlers.js.call(JSEXT1, 'os', ".js"); // ActiveDirectory needs its .stat()
  ActiveDirectory.call(this, path);
  ActiveDirectory.call(JSEXT1, JSEXT1.$path);

  // For out-of-tree code using the old names.  We can't just use a JSEXT1.js, because it would be ignored
  JSEXT1.__defineGetter__('dir', function() { return JSEXT1.os.dir; });
  JSEXT1.__defineGetter__('exists', function() { return JSEXT1.os.exists; });
  JSEXT1.__defineGetter__('isdir', function() { return JSEXT1.os.isdir; });
  JSEXT1.__defineGetter__('stat', function() { return JSEXT1.os.stat; });

  // it's painful setting these from C++ code, so we don't
  const empty_tag_names = jsxlib.stringifyHTML.empty_tag_names;
  empty_tag_names.br = true;
  empty_tag_names.hr = true;
  empty_tag_names.link = true;
  empty_tag_names.meta = true;
  empty_tag_names.img = true;
  empty_tag_names.input = true;
  const boolean_attribute_names = jsxlib.stringifyHTML.boolean_attribute_names;
  boolean_attribute_names.checked = true;
  boolean_attribute_names.selected = true;
  boolean_attribute_names.disabled = true;
  const boolean_attribute_falsey_values = jsxlib.stringifyHTML.boolean_attribute_falsey_values;
  boolean_attribute_falsey_values["false"] = true;
  boolean_attribute_falsey_values["0"] = true;
  boolean_attribute_falsey_values[""] = true;

  // And now run as FastCGI, CGI, or an interactive shell

  if(environment.JSEXT_FCGI) {
    return JSEXT1.fcgi();
  }

  if(environment.GATEWAY_INTERFACE) {
    new JSEXT1.CGI();
    return;
  }

  return JSEXT1.interactive();
})()
