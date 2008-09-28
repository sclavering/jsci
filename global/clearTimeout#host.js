/*
      clearTimeout (id)

  Clears a timeout event.
 */

function(id) {
  var events=setTimeout.events();
  delete events[id];
  events.updated=true;
}
