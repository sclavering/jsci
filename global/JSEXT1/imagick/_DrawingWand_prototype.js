({
  _init: function() {
    this._raw = _libwand.NewDrawingWand();
  },

  GRAVITY_NORTHWEST: _libwand.NorthWestGravity,
  GRAVITY_NORTH: _libwand.NorthGravity,
  GRAVITY_NORTHEAST: _libwand.NorthEastGravity,
  GRAVITY_WEST: _libwand.WestGravity,
  GRAVITY_CENTER: _libwand.CenterGravity,
  GRAVITY_EAST: _libwand.EastGravity,
  GRAVITY_SOUTHWEST: _libwand.SouthWestGravity,
  GRAVITY_SOUTH: _libwand.SouthGravity,
  GRAVITY_SOUTHEAST: _libwand.SouthEastGravity,

  set fillColor(pixelwand) {
    _libwand.DrawSetFillColor(this._raw, pixelwand._raw);
    return pixelwand;
  },

  get fontFamily() {
    return _libwand.DrawGetFontFamily(this._raw);
  },

  set fontFamily(val) {
    _libwand.DrawSetFontFamily(this._raw, val);
    return this.fontFamily;
  },

  get fontSize() {
    return _libwand.DrawGetFontSize(this._raw);
  },

  set fontSize(val) {
    _libwand.DrawSetFontSize(this._raw, val);
    return this.fontSize;
  },

  get gravity() {
    return _libwand.DrawGetGravity(this._raw);
  },

  set gravity(val) {
    _libwand.DrawSetGravity(this._raw, val);
    return this.gravity;
  },
})


/*
See http://www.imagemagick.org/api/drawing-wand.php
  ClearDrawingWand
  CloneDrawingWand
  DestroyDrawingWand
  DrawAffine
  DrawAnnotation
  DrawArc
  DrawBezier
  DrawCircle
  DrawClearException
  DrawComposite
  DrawColor
  DrawComment
  DrawEllipse
  DrawGetClipPath
  DrawGetClipRule
  DrawGetClipUnits
  DrawGetException
  DrawGetExceptionType
  DrawGetFillColor
  DrawGetFillOpacity
  DrawGetFillRule
  DrawGetFont
  DrawGetFontFamily
  DrawGetFontSize
  DrawGetFontStretch
  DrawGetFontStyle
  DrawGetFontWeight
  DrawGetGravity
  DrawGetStrokeAntialias
  DrawGetStrokeColor
  DrawGetStrokeDashArray
  DrawGetStrokeDashOffset
  DrawGetStrokeLineCap
  DrawGetStrokeLineJoin
  DrawGetStrokeMiterLimit
  DrawGetStrokeOpacity
  DrawGetStrokeWidth
  DrawGetTextAlignment
  DrawGetTextAntialias
  DrawGetTextDecoration
  DrawGetTextEncoding
  DrawGetVectorGraphics
  DrawGetTextUnderColor
  DrawLine
  DrawMatte
  DrawPathClose
  DrawPathCurveToAbsolute
  DrawPathCurveToRelative
  DrawPathCurveToQuadraticBezierAbsolute
  DrawPathCurveToQuadraticBezierRelative
  DrawPathCurveToQuadraticBezierSmoothAbsolute
  DrawPathCurveToQuadraticBezierSmoothAbsolute
  DrawPathCurveToSmoothAbsolute
  DrawPathCurveToSmoothRelative
  DrawPathEllipticArcAbsolute
  DrawPathEllipticArcRelative
  DrawPathFinish
  DrawPathLineToAbsolute
  DrawPathLineToRelative
  DrawPathLineToHorizontalAbsolute
  DrawPathLineToHorizontalRelative
  DrawPathLineToVerticalAbsolute
  DrawPathLineToVerticalRelative
  DrawPathMoveToAbsolute
  DrawPathMoveToRelative
  DrawPathStart
  DrawPoint
  DrawPolygon
  DrawPolyline
  DrawPopClipPath
  DrawPopDefs
  DrawPopPattern
  DrawPushClipPath
  DrawPushDefs
  DrawPushPattern
  DrawRectangle
  DrawRender
  DrawResetVectorGraphics
  DrawRotate
  DrawRoundRectangle
  DrawScale
  DrawSetClipPath
  DrawSetClipRule
  DrawSetClipUnits
  DrawSetFillColor
  DrawSetFillOpacity
  DrawSetFillPatternURL
  DrawSetFillRule
  DrawSetFont
  DrawSetFontFamily
  DrawSetFontSize
  DrawSetFontStretch
  DrawSetFontStyle
  DrawSetFontWeight
  DrawSetGravity
  DrawSetStrokeColor
  DrawSetStrokePatternURL
  DrawSetStrokeAntialias
  DrawSetStrokeDashArray
  DrawSetStrokeDashOffset
  DrawSetStrokeLineCap
  DrawSetStrokeLineJoin
  DrawSetStrokeMiterLimit
  DrawSetStrokeOpacity
  DrawSetStrokeWidth
  DrawSetTextAlignment
  DrawSetTextAntialias
  DrawSetTextDecoration
  DrawSetTextEncoding
  DrawSetTextUnderColor
  DrawSetVectorGraphics
  DrawSkewX
  DrawSkewY
  DrawTranslate
  DrawSetViewbox
  IsDrawingWand
  NewDrawingWand
  PeekDrawingWand
  PopDrawingWand
  PushDrawingWand
*/
