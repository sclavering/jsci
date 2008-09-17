/*
    access(pathname, [mode=""])

access  checks  whether the process would be allowed to read, write or test for existence of the file (or other file system object) whose name is
pathname.  If pathname is a symbolic link permissions of the file referred to by this symbolic link are tested.

_mode_ is a string consisting of zero or more of "r", "w", "x"

"r", "w" "x" request checking whether the file exists and has read, write and  execute  permissions,  respectively.   The empty string  just  requests
checking for the existence of the file.

The  tests depend on the permissions of the directories occurring in the path to the file, as given in pathname, and on the permissions of directories and files referred to by symbolic links encountered on the way.

The check is done with the process's real UID and GID, rather than with the effective IDs as is done when actually attempting an operation.  This
is to allow set-UID programs to easily determine the invoking user's authority.

Only  access  bits are checked, not the file type or contents.  Therefore, if a directory is found to be "writable," it probably means that files
can be created in the directory, and not that the directory can be written as a file.  Similarly, a DOS file may be found to be "executable," but the execve(2) call will still fail.

If  the  process has appropriate privileges, an implementation may indicate success for "x" even if none of the execute file permission bits are set.

### RETURN VALUE ###

On success (all requested permissions granted), true is returned.  On error (at least one bit in mode asked for a permission that is  denied,  or
some other error occurred), false is returned, and errno is set appropriately.

*/

(function() {

  var m={r: clib.R_OK,
	 w: clib.W_OK,
	 x: clib.X_OK,
	 R: clib.R_OK,
	 W: clib.W_OK,
	 X: clib.X_OK};

  return function(pathname, mode) {
    var mask=clib.F_OK;

    if (mode)
      for (var i=0; i<mode.length; i++)
	mask |= m[mode[i]];

    var ret=clib.access(pathname, mask);
    return ret==0;
  }

})()

