({
  get Wand() {
    if(!this._Wand) {
      this._Wand = function Wand() { this._init.apply(this, arguments); }
      this._Wand.prototype = this._Wand_prototype;
    }
    return this._Wand;
  },
  _Wand: null,

  get PixelWand() {
    if(!this._PixelWand) {
      this._PixelWand = function PixelWand() { this._init.apply(this, arguments); }
      this._PixelWand.prototype = this._PixelWand_prototype;
    }
    return this._PixelWand;
  },
  _PixelWand: null,

  get DrawingWand() {
    if(!this._DrawingWand) {
      this._DrawingWand = function DrawingWand() { this._init.apply(this, arguments); }
      this._DrawingWand.prototype = this._DrawingWand_prototype;
    }
    return this._DrawingWand;
  },
  _DrawingWand: null,
})
