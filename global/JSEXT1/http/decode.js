/*
  Similar to [[decodeURIComponent]], but also decodes + characters
  to spaces.
 */

  function (qry) {
    if (qry===undefined)
      return;
    qry=qry.replace(/\+/g," ");
    qry=qry.replace(/%../g,function(nn){return String.fromCharCode(parseInt(nn.substr(1),16))});
    return qry;
    //    return decodeURIComponent(qry.replace(/\+/g," "));
  }
