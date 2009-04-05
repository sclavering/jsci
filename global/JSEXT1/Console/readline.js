/*
    readline(prompt) => line

Many programs provide a command line interface, such as mail, ftp, and sh. For such programs, the default behaviour of Readline is sufficient. This section describes how to use Readline in the simplest way possible.
     

The function readline() prints a prompt prompt and then reads and returns a single line of text from the user. If prompt is undefined or the empty string, no prompt is displayed.

If readline encounters an EOF while reading the line, and the line is empty at that point, then undefined is returned. Otherwise, the line is ended just as if a newline had been typed. 

If you want the user to be able to get at the line later, (with C-p for example), you must call [[history.add]] () to save the line away in a history list of such lines. 

     history.add (line);
    
*/

function (prompt) {
    if (prompt)
      prompt = String(prompt);
    else
      prompt = null;

    var ret = JSEXT1.libreadline.readline(prompt);

    if (ret===null) // encountered EOF
      return;

    var str_ret=ret.string();
    clib.free(ret);
    return str_ret;
}
