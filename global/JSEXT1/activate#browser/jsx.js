function (name,extension) {
  return JSEXT1.http['function'](this.$url+"/"+encodeURIComponent(name)+extension);
}
