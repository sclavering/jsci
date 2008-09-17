/*
         wsdl['export'](services: Object, [options: Object])

     If you wish to export functions using wsdl, create a jsx file like this:

         function() {
	   this.responseHeaders.contentType="application/xml";
           return JSEXT1.wsdl['export'].call(this,
                      {
                        myService: {
                               name1: func1,
                               name2: func2,
                               name3: func3
                        }
                      });
         }

     Alternatively, if you wish to export all functions in a directory, create this jsx file
     in the parent directory with the same name as the directory plus _.jsx_:

         function() {
           return JSEXT1.wsdl['export'].call(this, {myService: {arguments.callee});
         }

     When the jsx file is accessed without a SOAPAction header field, the file will generate a
     wsdl file which points future SOAP requests back to the same jsx file. These requests
     will per the SOAP specification have a SOAPAction header, which allows the jsx file to
     forward the call to the appropriate function.

     Remember that jsx files must be accessed with a (possibly empty) query string to be executed.
     So, if your jsx file is at url

         http://myserver.com/webservice.jsx

     then access it using this URL:

         http://myserver.com/webservice.jsx?

     All arguments of exported functions are assumed to be strings unless otherwise specified.
     All arguments of exported functions are required. The return type of exported functions
     is a assumed to be a string unless otherwise specified.
     To specify the argument types, add a _argumentTypes_ property to the function. This should be
     an array where element no. 0 corresponds to the first argument, etc. The
     elements should be examples of the kind of data you want, i.e. any (possibly empty) string
     indicates a string type, any integer (including 0) indicates an integer, any fractional number
     (including 0.1) indicates a double, any boolean (there
     are exactly two of them) indicates a boolean type and a date indicates a date. If your function expects an
     object, then the sample object should contain sample values. All properties will be required.
     If your function expects an
     array, then the sample array should contain one element, which is a sample of the kind
     of values the array should contain. All elements of a SOAP array will be of the same type.
     The array length is undefined. To specify a return
     type for the function, add _returnType_ property to the function, containing a sample value.

     Options are

     * _requestURL_: The url to the service entry point. Default is the current url.

     ### NOTE ###
     
     "export" is a reserved word in JavaScript, so it is necessary to enclose
     it in quotes.

    */

function(services, options) {
  options=options || {};

  var requestURL=options.requestURL || this.requestURL;

  if (this.requestHeaders && this.requestHeaders.soapaction) {
    // serve a request
    
    var SOAPAction = this.requestHeaders.soapaction;
    if (SOAPAction.substr(0,1)!='"' || SOAPAction.substr(-1,1)!='"')
      throw new Error("Invalid SOAPAction '"+SOAPAction+"'");
    SOAPAction = SOAPAction.substr(1,SOAPAction.length-2);
    var SAparts = SOAPAction.split("-");
    
    var functions = services[SAparts[0]];
    var func = functions[SAparts[1]];

    if (!func)
      throw new Error("unknown action: '"+SOAPAction+"'");

    var doc=$parent.soap.toXML(stdin.read());
    var args=doc.soapenv::Body.*[0];

    var expectname=requestURL+"::"+SOAPAction+"RequestDocument";
    if (args.name()!=expectname)
      throw new Error("Unexpected body element '"+args.name()+"', expected '"+expectname+"'");

    if (!func.wsdl)
      makeWsdlParts(SOAPAction, func);
 
    var fwsdl=func.wsdl[SOAPAction];

    args=$parent.soap.decode(args, fwsdl.requestSchema);

    var argarray=[];
    for (var i=0; i<fwsdl.argnames.length; i++)
      argarray[i]=args[fwsdl.argnames[i]];

    var ret=func.apply(this, argarray);
    ret=$parent.soap.encode("return",ret);
    
    ret=   <soap:Envelope xmlns:soap={soapenv} xmlns:xsi={xsi} xmlns:xsd={xsd}>
             <soap:Body>
               <{SOAPAction+"ResponseDocument"} xmlns={requestURL}>
                 {ret}
               </{SOAPAction+"ResponseDocument"}>
             </soap:Body>
           </soap:Envelope>

    return ret;

  } else {
    // produce a wsdl file

    var schema=new XMLList;
    var messages=new XMLList;
    var porttypes=new XMLList;
    var bindings=new XMLList;
    var xservices=new XMLList;

    for (var s in services) {

      var ports=new XMLList;
      var functions=services[s];

      for (var i in functions) {
	if (functions.hasOwnProperty(i) && typeof functions[i] === "function") {

	  if (!functions[i].wsdl || !functions[i].wsdl[s+i])
	    makeWsdlParts(s, i, functions[i]);

	  var fwsdl=functions[i].wsdl[s+"-"+i];

	  messages += fwsdl.requestMessage;
	  messages += fwsdl.responseMessage;
	  porttypes += fwsdl.porttype;
	  bindings += fwsdl.binding;
	  schema += fwsdl.requestSchema;
	  schema += fwsdl.responseSchema;
	  ports += fwsdl.port;

	}
      }

      xservices += <service name={s}>
                     {ports}
                   </service>
    }

    return <definitions name={options.name}
               xmlns:tns={requestURL}
               xmlns:xsd={xsd}
               xmlns:soap={soap}
               targetNamespace={requestURL}
	       xmlns={wsdl} >

                <types>
                  <xsd:schema
                    elementFormDefault="qualified"
                    targetNamespace={requestURL} >

                    {schema}

                  </xsd:schema>
                </types>

		{messages}
                {porttypes}
		{bindings}
                {xservices}

         </definitions>;
  }


  function makeWsdlParts(service, operation, func) {
    var type;
    var SOAPAction=service + "-" + operation;

    if (!func.wsdl)
      func.wsdl = {};

    func.wsdl[SOAPAction] = {};
    var wsdl = func.wsdl[SOAPAction];

    wsdl.requestMessage =
                  <message name={SOAPAction+"Request"}>
                    <part name="parameters" element={"tns:"+SOAPAction+"RequestDocument"}/>
                  </message>

    wsdl.responseMessage =
                  <message name={SOAPAction+"Response"}>
                    <part name="parameters" element={"tns:"+SOAPAction+"ResponseDocument"}/>
                  </message>

    wsdl.porttype =
               <portType name={SOAPAction+"PortType"}>
                 <operation name={i}>
                   <input message={"tns:"+SOAPAction+"Request"}/>
                   <output message={"tns:"+SOAPAction+"Response"}/>
                 </operation>
               </portType>

    wsdl.port =
              <port name={SOAPAction+"Port"} binding={"tns:"+SOAPAction+"Binding"}>
                <soap:address location={requestURL} xmlns:soap={soap} />
	      </port>

    wsdl.binding =
                  <binding name={SOAPAction+"Binding"} type={"tns:"+SOAPAction+"PortType"} xmlns:soap={soap}>
                    <soap:binding style="document"
                      transport="http://schemas.xmlsoap.org/soap/http" />
                    <operation name={operation}>
                      <soap:operation soapAction={SOAPAction}/>
                      <input>
                        <soap:body use="literal" />
                      </input>
                      <output>
                        <soap:body use="literal" />
                      </output>
                    </operation>
                  </binding>

    var args=new XMLList;

    var src=func.toSource();
    wsdl.argnames=src.match(/\(([^()]*)\)/)[1].split(", ");
    if (wsdl.argnames[0]=="")
      wsdl.argnames=[];

    for (var j=0; j<wsdl.argnames.length; j++) {
      if (func.argumentTypes && func.argumentTypes[j] !== undefined)
        args += $parent.soap.xsdElement(wsdl.argnames[j], func.argumentTypes[j]);
      else
        args += $parent.soap.xsdElement(wsdl.argnames[j], "");
    }

    if (func.returnType !== undefined)
      var retelem = $parent.soap.xsdElement("return", func.returnType);
    else
      var retelem = $parent.soap.xsdElement("return", "");

    wsdl.requestSchema =
              <xsd:element name={SOAPAction+"RequestDocument"} xmlns:xsd={xsd}>
                <xsd:complexType>
                  <xsd:sequence>
                    {args}
                  </xsd:sequence>
                </xsd:complexType>
              </xsd:element>

    wsdl.responseSchema =
              <xsd:element name={SOAPAction+"ResponseDocument"} xmlns:xsd={xsd}>
                <xsd:complexType>
                  <xsd:sequence>
                    {retelem}
                  </xsd:sequence>
                </xsd:complexType>
              </xsd:element>

  }


}
