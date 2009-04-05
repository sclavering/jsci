function (key, func) {
  if (typeof(key) == "string")
    key=string.charCodeAt(0);
  JSEXT1.libreadline.rl_bind_key(key, func);
}
