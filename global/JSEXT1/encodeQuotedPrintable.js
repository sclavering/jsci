function(str) {
  str=str.replace(/[\u0080-\u00ff=\r\n]|[\t ]$/mg,function(c) {
      return "="+("0"+c.charCodeAt(0).toString(16).toUpperCase()).substr(-2);
    });
  str=str.replace(/\=0D=0A/g,"\r\n");
  str=str.replace(/([^\r\n]{78,78})/g,"$1=\r\n"); // folding
  return str;
}
