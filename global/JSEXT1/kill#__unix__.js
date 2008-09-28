/*
      kill (pid, sig)

  Send a signal to another process. Availability: Unix
*/

function (pid, signum) {
  if (clib.kill(pid, signum))
    throw new Error(os.error("kill"));
}

