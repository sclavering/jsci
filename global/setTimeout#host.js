/*
      id = setTimeout (func, ms)

  Sets a new timeout event, which will be executed after _ms_
  milliseconds. Events
  are executed in the same thread that registers them. The first
  opportunity for executing events is when the program's main
  function exits. The program does not terminate until the event is
  cleared or executed.

*/

function(func, ms) {
  var events=arguments.callee.events();

  var evt=new Number(new Date().valueOf()+ms);
  evt.func=func;
  evt.id=events.length;
  events.push(evt);

  events.updated=true;
  return evt.id;
}
