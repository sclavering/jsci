/*
      id = setInterval (func, ms)

  Sets a new interval event, which will be executed after _ms_
  milliseconds and every _ms_ milliseconds thereafter. Events
  are executed in the same thread that registers them. The first
  opportunity for executing events is when the program's main
  function exits. The program does not terminate while interval
  events exist.

 */

function(func, ms) {
  var events=setTimeout.events();

  var evt=new Number(new Date().valueOf()+ms);
  evt.func=func;
  evt.id=events.length;
  evt.repeat=ms;
  events.push(evt);

  events.updated=true;
  return evt.id;
}
