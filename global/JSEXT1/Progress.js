/*
  Show progress for operations that take time.

  ---

  **Example**

  ---

      var p = new Progress;

      while (var i=0; i<10; i++) {
        p.status("calculating...", i/10*100);
      }

      p.close();

  ---

  Progress objects can be nested. The status of the
  outer progress object is shown when an inner progress object
  is closed. Progress display is cleared when the outer progress
  object is closed.

 */

(function() {
  
  if (stdin.isatty()) {
    var P=function() {
      arguments.callee.stack.push(this);
    }
    
    P.stack=[];

    P.prototype = {
    
    /*

          p.status(msg, pct)

      Updates the progress status.

      ### Arguments ###

      * _msg_: A string
      * _pct_: A number between 0 and 100.
      
     */
    
    status: function(msg, pct) {
      this.msg=msg;
      this.pct=pct;
      this.update();
    },
    
    update: function() {
      print("\r",this.msg,String.fromCharCode(27)+"[K");
      stdout.flush();
    },

    /*
          p.close()

      Closes a progress object, clearing its display.
     */
    
    close: function() {
      var s=P.stack.pop();
      if (P.stack.length) {
	s.update();
      } else {
	print("\r",String.fromCharCode(27)+"[K");
	stdout.flush();
      }
    }
    }
    
    return P;
  } else {
    var P=function(){};
    P.prototype.status=function(){};
    P.prototype.close=function(){};
    
    return P;
  }
})()

