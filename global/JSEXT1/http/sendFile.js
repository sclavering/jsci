/*

      sendFile()

  Prints the contents of a file on [[stdout]] and sets the
  value of _this.responseHeaders.contentType_ to a mime type
  depending on the file's filename extension. Also sets
  the value of _this.responseHeaders.lastModified_ to the file's
  _mtime_ value.

  If a file exists with the same name as the file being sent, only
  with filename extension '.expires', it will be read and assigned
  to _this.responseHeaders.expires_.

  The function is called without arguments, but with a _this_ object
  which must contain the following properties:

  * _responseHeaders_: An object
  * _filename_: A string
  * _hostdir_: An [[$parent.ActiveDirectory]] object

  The function does not return a value, but prints its result to
  [[stdout]].


 */

function () {
  var dir=this.hostdir;
  var filename=this.filename;

  var modtime=$parent.stat(dir.$path + filename).mtime;
  
  if (this.requestHeaders.ifModifiedSince &&
      Date.parse(this.requestHeaders.ifModifiedSince)
      >= modtime &&
      !this.requestHeaders.cacheControl) {
    this.responseLine="304 Not Modified";
    return;
  }
  
  var extension=filename.match(/\.([^.]*)$/);
  if (extension)
    extension=extension[1];

  if ($parent.mime.type[extension])
    this.responseHeaders.contentType=$parent.mime.type[extension];
  
  var expirefile=filename.replace(/\.[^.]*$/,"")+".expires";

  if ($parent.access(dir.$path + expirefile, "r")) {
    this.responseHeaders.expires=$parent.read(dir.$path + expirefile);
  }
  
  var file=new $parent.File(dir.$path + filename);
  
  this.responseHeaders.lastModified=file.stat().mtime.toUTCString();
  this.responseHeaders.cacheControl = "public";
  this.responseHeaders.etag = "\""+$parent.md5(file)+"\"";
  file.seek(0);
  while (!file.eof()) {
    var buf=file.read(65536);
    stdout.write(buf);
  }


  file.close();
}
      
