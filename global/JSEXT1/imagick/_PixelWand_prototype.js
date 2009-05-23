// The prototype for imagick.PixelWand

({
  _init: function(raw_pixel_wand) {
    this._raw = _libwand.NewPixelWand();
  },

  set color(str) {
    const x = _libwand.PixelSetColor(this._raw, str);
    return x ? str : '';
  },

  get color() {
    return {
      r: _libwand.PixelGetRed(this._raw),
      g: _libwand.PixelGetGreen(this._raw),
      b: _libwand.PixelGetBlue(this._raw),
      a: _libwand.PixelGetAlpha(this._raw),
    };
  },
})

/*
Pixel Wand Methods, see http://www.imagemagick.org/api/pixel-wand.php
  ClearPixelWand
  ClonePixelWand
  ClonePixelWands
  DestroyPixelWand
  DestroyPixelWands
  IsPixelWandSimilar
  IsPixelWand
  NewPixelWand
  NewPixelWands
  PixelClearException
  PixelGetAlpha
  PixelGetAlphaQuantum
  PixelGetBlack
  PixelGetBlackQuantum
  PixelGetBlue
  PixelGetBlueQuantum
  PixelGetColorAsString
  PixelGetColorAsNormalizedString
  PixelGetColorCount
  PixelGetCyan
  PixelGetCyanQuantum
  PixelGetException
  PixelGetExceptionType
  PixelGetFuzz
  PixelGetGreen
  PixelGetGreenQuantum
  PixelGetHSL
  PixelGetIndex
  PixelGetMagenta
  PixelGetMagentaQuantum
  PixelGetOpacity
  PixelGetOpacityQuantum
  PixelGetQuantumColor
  PixelGetRed
  PixelGetRedQuantum
  PixelGetYellow
  PixelGetYellowQuantum
  PixelSetAlpha
  PixelSetAlphaQuantum
  PixelSetBlack
  PixelSetBlackQuantum
  PixelSetBlue
  PixelSetBlueQuantum
  PixelSetColor
  PixelSetColorCount
  PixelSetColorFromWand
  PixelSetCyan
  PixelSetCyanQuantum
  PixelSetFuzz
  PixelSetGreen
  PixelSetGreenQuantum
  PixelSetHSL
  PixelSetIndex
  PixelSetMagenta
  PixelSetMagentaQuantum
  PixelSetMagickColor
  PixelSetOpacity
  PixelSetOpacityQuantum
  PixelSetQuantumColor
  PixelSetRed
  PixelSetRedQuantum
  PixelSetYellow
  PixelSetYellowQuantum
*/
