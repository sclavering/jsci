function(attribute, defaultNamespace) {
  var attrparts=String(attribute).split(":");
  if (attrparts.length==1) {
    if (defaultNamespace)
      return QName(defaultNamespace, attribute);
    else
      attrparts.unshift("");
  }
  return QName(attribute.namespace(attrparts[0]), attrparts[1]);
}
