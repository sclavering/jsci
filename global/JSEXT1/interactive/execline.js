/*
  Used by [[$curdir]] to evaluate commands typed on the
  console.
*/


Function("_line"," \
	  try { \
	    var _rval=eval(_line); \
	    _line=_line.replace(/^[ \\t\\n\\r]*/,'').replace(/[ \\t\\n\\r]*$/,''); \
	    var _lastchar=_line.substr(_line.length-1); \
	    if (_lastchar!=';' && _lastchar!='}' && _lastchar!='>' && _rval!==undefined) { \
		print(String(_rval)+'\\n'); \
	    } \
	  } catch(_err) { \
	      if (_err.fileName && _err.lineNumber) \
                print('Line '+_err.lineNumber+' in '+_err.fileName+':'); \
	      print((_err.message || _err)+'\\n'); \
              if (_err.stack) { \
                var stack=_err.stack.split('\\n'); \
                while (!stack.pop().match(/execline/)); \
                stack.pop(); \
                print(stack.join('\\n')+'\\n'); \
              } \
	  } \
      ");

