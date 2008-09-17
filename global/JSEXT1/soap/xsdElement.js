/*
       xml = xsdElement(name, sample)

   Returns an XML Schema fragment.

 */

function(name, sample) {
  
  if ((typeof sample=="object") && (sample instanceof Array)) {
    var type=xsdTypeOf(sample[0]);
    if (typeof type === "string") {
      return <xsd:element minOccurs="1" maxOccurs="unbounded" name={name} type={"xsd:"+type} xmlns:xsd={xsd}/>
    } else {
      return <xsd:element minOccurs="1" maxOccurs="unbounded" name={name} xmlns:xsd={xsd}>
               {type}
             </xsd:element>
    }
  }

  var type=xsdTypeOf(sample);

  if (typeof type==="string")
    return <xsd:element minOccurs="1" maxOccurs="1" name={name} type={"xsd:"+type} xmlns:xsd={xsd}/>
  else
    return <xsd:element minOccurs="1" maxOccurs="1" name={name} xmlns:xsd={xsd}>
             {type}
           </xsd:element>
  

}
