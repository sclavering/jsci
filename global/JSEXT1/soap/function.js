/*

    func = function(url, SOAPAction, inputschema, outputschema, schema)

Returns a new function which, when called, sends a SOAP request
to a server and decodes the response.

*/

function (url, SOAPAction, style, name, inputSchema, outputSchema, schema) {
  var wsdl=$parent.wsdl.wsdl;

  if (style=="document")
    var args=inputSchema.xsd::complexType.xsd::sequence.xsd::element;
  else
    var args=inputSchema.wsdl::part;

  function inner() {
    var doc=new XMLList;

    for (var i=0; i<args.length(); i++) {
      if (arguments[i]!==undefined)
	doc += encode(String(args[i].@name), arguments[i]);
    }

    if (style=="document")
      doc=<{"tns:"+inputSchema.@name} xmlns:tns={schema.@targetNamespace}>
            {doc}
  	  </{"tns:"+inputSchema.@name}>
    else
      doc=<{"tns:"+name} xmlns:tns={schema.@targetNamespace}>
            {doc}
          </{"tns:"+name}>



    var req=<soapenv:Envelope xmlns:soapenv={soapenv} xmlns:xsi={xsi} xmlns:xsd={xsd}>
	      <soapenv:Body>
		{doc}
	      </soapenv:Body>
            </soapenv:Envelope>

    var ret=$parent.http.soap(url, SOAPAction, req);
		  
    var fault=ret.soapenv::Body.soapenv::Fault;
    if (fault.length()>0)
      throw new Error(String(fault.*::faultstring));

    if (style=="document") {
      var ret=decode(ret.soapenv::Body.*[0], outputSchema, schema);
      for (var i in ret)
	return ret[i];
    } else {
      return decode(ret.soapenv::Body.*[0].*[0], outputSchema.*[0], schema);
    }
  }

  inner.toString=function() {
    var argnames=[];
    
    for (var i=0; i<args.length(); i++) {
      argnames.push(args[i].@name);
    }

    return "function ("+argnames.join(", ")+") {\n  SOAP function\n  url: "+url+"\n  SOAPAction: "+SOAPAction+"\n}";
  }

  return inner;
}
