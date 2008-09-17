/*
         obj = wsdl['import'](wsdl, [service1, [service2, ...]])

     Imports services from a WSDL file. The
     specified services are imported. If no services are specified, all services
     described in the wsdl file are imported. The _wsdl_ argument could
     be a wsdl file in an [[XML]] object or a url to one.

     ### Return value ###

     An object containing the services. Each service is an object containing
     imported functions.

     ### NOTE ###
     
     "import" is a reserved word in JavaScript, so it is necessary to enclose
     it in quotes.

*/

function(doc) {
  if (typeof doc!=="xml")
    doc=$parent.soap.toXML($parent.read(doc));

  var hasOwnProperty=Object.prototype.hasOwnProperty;

  var ret={};
  if (arguments.length>1) {
    for (var i=1; i<arguments.length; i++) {
      ret[arguments[i]]=importService(doc.wsdl::service.(@name=arguments[i]));
    }
  } else {
    for each (var service in doc.wsdl::service) {
      ret[service.@name]=importService(service);
    }
  }


  function importService(service) {
    var ret={};

    var port=service.wsdl::port;
    var location=port.soap::address.@location;
    var bindingName=$parent.soap.toQName(port.@binding);
    var binding=doc.wsdl::binding.(@name==bindingName.localName);
    if (binding.length()==0)
      throw new Error("Unknown binding '"+bindingName.localName+"'");
    
    if (binding.soap::binding.@transport!="http://schemas.xmlsoap.org/soap/http")
      throw new Error("Binding '"+binding.@name+"' uses unknown transport '"+binding.soap::binding.@transport+"'");
    
    var style=binding.soap::binding.@style;
	
    var portTypeName=$parent.soap.toQName(binding.@type);
    var portType=doc.wsdl::portType.(@name==portTypeName.localName);

    if (portType.length==0)
      throw new Error("Unknown port type '"+portTypeName.localName+"'");

    for each (var operation in portType.wsdl::operation) {
      var name=operation.@name;
      var SOAPAction=binding.wsdl::operation.(@name==name).soap::operation.@soapAction;

      var input=operation.wsdl::input;
      if (input.length()==0)
	throw new Error("Missing operation input '"+portTypeName.localName+"'");
	
      var output=operation.wsdl::output;
      if (output.length()==0)
	throw new Error("Missing operation output '"+portTypeName.localName+"'");
      
      if (input.@message.length()==0)
	throw new Error("Missing input message name '"+portTypeName.localName+"'");
      if (output.@message.length()==0)
	throw new Error("Missing output message name '"+portTypeName.localName+"'");

      var inputMessage=doc.wsdl::message.(@name==$parent.soap.toQName(input.@message).localName);
      var outputMessage=doc.wsdl::message.(@name==$parent.soap.toQName(output.@message).localName);

      if (style=="document") {
	var inputDocument=doc.wsdl::types.xsd::schema.xsd::element.(@name==toQName(inputMessage.wsdl::part.@element).localName);
	var outputDocument=doc.wsdl::types.xsd::schema.xsd::element.(@name==toQName(outputMessage.wsdl::part.@element).localName);
      } else {
	var inputDocument=inputMessage;
	var outputDocument=outputMessage;
      }

      ret[name]=$parent.soap['function'](location, SOAPAction, style, name, inputDocument, outputDocument, doc.wsdl::types.xsd::schema);

    }

    return ret;
  }


  return ret;
}
