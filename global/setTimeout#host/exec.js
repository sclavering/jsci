function() {
  var events=$curdir.events();
  if (!events.updated)
      return;
  for (;;) {
    if (events.updated) {
      var sortevents=events.sort();
      events.updated=false;
    }
    var evt=sortevents.shift();
    if (!evt) break;
    delete events[evt.id];

    var now=new Date().valueOf();
    var time=evt.valueOf()-now;

    if (time>0) {
      clib.usleep(time*1000);
    }

    if (evt.repeat) {
      var newevt=new Number((time>0?evt.valueOf():now)+evt.repeat);
      newevt.func=evt.func;
      newevt.repeat=evt.repeat;
      newevt.id=evt.id;
      events[newevt.id]=newevt;
      events.updated=true;
    }

    if (typeof evt.func==="function")
      evt.func();
    else
      eval(evt.func);
  }
}
