/*

      obj = man (topic)

  Returns extracted comments about a given topic.

  ### Arguments ###

  * _topic_: A string on the form "a.b.c", naming the manual topic.
    Even words that in JavaScript are reserved should be dot-separated

  ### Return value ###

  Returns an object containing a _text_ property containing
  a string.

 */

function(topic) {

  if (!arguments.callee.$alldoc)
    arguments.callee.$alldoc=new ActiveDirectory((function(){return this;})().$path, arguments.callee.FileExtensions);

  if (!topic)
    return;

  var found;
    
  var topicparts=topic.split('.');
  var cur=arguments.callee.$alldoc;
  var hasOwnProperty=Object.prototype.hasOwnProperty;
    
  while (topicparts.length) {
    var topicpart=topicparts.shift();
    
    if (!hasOwnProperty.call(cur, topicpart))
      return;
    
    cur=cur[topicpart];
    
  }
  
  return cur.$doc;
  
}

