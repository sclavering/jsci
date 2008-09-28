function() {
  if (!this.closed) {
    windows.DestroyWindow(this.Frame.Wnd);
    this.closed=true;
  }
}

