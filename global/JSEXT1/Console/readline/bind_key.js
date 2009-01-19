function (key, func) {
  if (typeof(key) == "string")
    key=string.charCodeAt(0);
  lib.rl_bind_key(key, func);
}
