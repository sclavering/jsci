// The prototype for imagick.Wand

({
  _init: function() {
    _libwand.MagickWandGenesis();
    this._raw = _libwand.NewMagickWand();
  },

  destroy: function() {
    _libwand.ClearMagickWand(this._raw);
    this._raw = null;
  },

  newImage: function(width, height, background_pixel) {
    return _libwand.MagickNewImage(this._raw, width, height, background_pixel._raw);
  },

  readImage: function(filename) {
    return _libwand.MagickReadImage(this._raw, filename);
  },

  get imageHeight() {
    return _libwand.MagickGetImageHeight(this._raw);
  },

  get imageWidth() {
    return _libwand.MagickGetImageWidth(this._raw);
  },

  // The curious name just mirrors the C API name
  getImagePixelColor: function(x, y) {
    const pw = new PixelWand();
    const r = _libwand.MagickGetImagePixelColor(this._raw, x, y, pw._raw);
    return r ? pw : null;
  },

  thumbnailImage: function(width, height) {
    return _libwand.MagickThumbnailImage(this._raw, width, height);
  },

  cropImage: function(width, height, x, y) {
    return _libwand.MagickCropImage(this._raw, width, height, x, y);
  },

  borderImage: function(color_pixel, width, height) {
    return _libwand.MagickBorderImage(this._raw, color_pixel._raw, width, height);
  },

  annotateImage: function(drawing_wand, x, y, angle, text) {
    return _libwand.MagickAnnotateImage(this._raw, drawing_wand._raw, x, y, angle, text);
  },

  setImageFormat: function(type) {
    return _libwand.MagickSetImageFormat(this._raw, type);
  },

  writeImage: function(filename) {
    return _libwand.MagickWriteImage(this._raw, filename);
  },
})

/*
Magick Wand Image Methods, see http://www.imagemagick.org/api/magick-image.php
  GetImageFromMagickWand
  MagickAdaptiveBlurImage
  MagickAdaptiveResizeImage
  MagickAdaptiveSharpenImage
  MagickAdaptiveThresholdImage
  MagickAddImage
  MagickAddNoiseImage
  MagickAffineTransformImage
  MagickAnnotateImage
  MagickAnimateImages
  MagickAppendImages
  MagickAverageImages
  MagickBlackThresholdImage
  MagickBlurImage
  MagickBorderImage
  MagickCharcoalImage
  MagickChopImage
  MagickClipImage
  MagickClipImagePath
  MagickClutImage
  MagickCoalesceImages
  MagickColorizeImage
  MagickCombineImages
  MagickCommentImage
  MagickCompareImageChannels
  MagickCompareImageLayers
  MagickCompareImage
  MagickCompositeImage
  MagickContrastImage
  MagickContrastStretchImage
  MagickConvolveImage
  MagickCropImage
  MagickCycleColormapImage
  MagickConstituteImage
  MagickDecipherImage
  MagickDeconstructImages
  MagickDeskewImage
  MagickDespeckleImage
  MagickDestroyImage
  MagickDisplayImage
  MagickDisplayImages
  MagickDistortImage
  MagickDrawImage
  MagickEdgeImage
  MagickEmbossImage
  MagickEncipherImage
  MagickEnhanceImage
  MagickEqualizeImage
  MagickEvaluateImage
  MagickExtentImage
  MagickFlipImage
  MagickFloodfillPaintImage
  MagickFlopImage
  MagickFrameImage
  MagickFxImage
  MagickGammaImage
  MagickGaussianBlurImage
  MagickGetImage
  MagickGetImageAlphaChannel
  MagickGetImageClipMask
  MagickGetImageBackgroundColor
  MagickGetImageBlob
  MagickGetImageBlob
  MagickGetImageBluePrimary
  MagickGetImageBorderColor
  MagickGetImageChannelDepth
  MagickGetImageChannelDistortion
  MagickGetImageChannelDistortions
  MagickGetImageChannelMean
  MagickGetImageChannelRange
  MagickGetImageChannelStatistics
  MagickGetImageColormapColor
  MagickGetImageColors
  MagickGetImageColorspace
  MagickGetImageCompose
  MagickGetImageCompression
  MagickGetImageCompression
  MagickGetImageDelay
  MagickGetImageDepth
  MagickGetImageDistortion
  MagickGetImageDispose
  MagickGetImageFilename
  MagickGetImageFormat
  MagickGetImageGamma
  MagickGetImageGravity
  MagickGetImageGreenPrimary
  MagickGetImageHeight - wand.imageHeight
  MagickGetImageHistogram
  MagickGetImageInterlaceScheme
  MagickGetImageInterpolateMethod
  MagickGetImageIterations
  MagickGetImageLength
  MagickGetImageMatteColor
  MagickGetImageOrientation
  MagickGetImagePage
  MagickGetImagePixelColor
  MagickGetImagePixels
  MagickGetImageRedPrimary
  MagickGetImageRegion
  MagickGetImageRenderingIntent
  MagickGetImageResolution
  MagickGetImageScene
  MagickGetImageSignature
  MagickGetImageTicksPerSecond
  MagickGetImageType
  MagickGetImageUnits
  MagickGetImageVirtualPixelMethod
  MagickGetImageWhitePoint 
  MagickGetImageWidth - wand.imageWidth
  MagickGetNumberImages
  MagickGetImageTotalInkDensity
  MagickHasNextImage
  MagickHasPreviousImage
  MagickIdentifyImage
  MagickImplodeImage
  MagickLabelImage
  MagickLevelImage
  MagickLinearStretchImage
  MagickLiquidRescaleImage
  MagickMagnifyImage
  MagickMedianFilterImage
  MagickMergeImageLayers
  MagickMinifyImage
  MagickModulateImage
  MagickMontageImage
  MagickMorphImages
  MagickMotionBlurImage
  MagickNegateImage
  MagickNewImage
  MagickNextImage
  MagickNormalizeImage
  MagickOilPaintImage
  MagickOpaquePaintImage
  MagickOptimizeImageLayers
  MagickOrderedPosterizeImage
  MagickPingImage
  MagickPingImageBlob
  MagickPingImageFile
  MagickPolaroidImage
  MagickPosterizeImage
  MagickPreviewImages
  MagickPreviousImage
  MagickQuantizeImage
  MagickQuantizeImages
  MagickRadialBlurImage
  MagickRaiseImage
  MagickRandomThresholdImage
  MagickReadImage
  MagickReadImageBlob
  MagickReadImageFile
  MagickRecolorImage
  MagickReduceNoiseImage
  MagickRemapImage
  MagickRemoveImage
  MagickResampleImage
  MagickResetImagePage
  MagickResizeImage
  MagickRollImage
  MagickRotateImage
  MagickSampleImage
  MagickScaleImage
  MagickSegmentImage
  MagickSeparateImageChannel
  MagickSepiaToneImage
  MagickSetImage
  MagickSetImageAlphaChannel
  MagickSetImageBackgroundColor
  MagickSetImageBias
  MagickSetImageBluePrimary
  MagickSetImageBorderColor
  MagickSetImageChannelDepth
  MagickSetImageClipMask
  MagickSetImageColormapColor
  MagickSetImageColorspace
  MagickSetImageCompose
  MagickSetImageCompression
  MagickSetImageCompressionQuality
  MagickSetImageDelay
  MagickSetImageDepth
  MagickSetImageDispose
  MagickSetImageExtent
  MagickSetImageFilename
  MagickSetImageFormat
  MagickSetImageGamma
  MagickSetImageGravity
  MagickSetImageGreenPrimary
  MagickSetImageInterlaceScheme
  MagickSetImageInterpolateMethod
  MagickSetImageIterations
  MagickSetImageMatte
  MagickSetImageMatteColor
  MagickSetImageOpacity
  MagickSetImageOrientation
  MagickSetImagePage
  MagickSetImagePixels
  MagickSetImageProgressMonitor
  MagickSetImageRedPrimary
  MagickSetImageRenderingIntent
  MagickSetImageResolution
  MagickSetImageScene
  MagickSetImageTicksPerSecond
  MagickSetImageType
  MagickSetImageUnits
  MagickSetImageWhitePoint
  MagickShadeImage
  MagickShadowImage
  MagickSharpenImage
  MagickShaveImage
  MagickShearImage
  MagickSigmoidalContrastImage
  MagickSketchImage
  MagickSolarizeImage
  MagickSpliceImage
  MagickSpreadImage
  MagickSteganoImage
  MagickStereoImage
  MagickStripImage
  MagickSwirlImage
  MagickTextureImage
  MagickThresholdImage
  MagickThumbnailImage
  MagickTintImage
  MagickTransformImage
  MagickTransparentPaintImage
  MagickTransposeImage
  MagickTransverseImage
  MagickTrimImage
  MagickUniqueImageColors
  MagickUnsharpMaskImage
  MagickVignetteImage
  MagickWaveImage
  MagickWhiteThresholdImage
  MagickWriteImage
  MagickWriteImageFile
  MagickWriteImages
  MagickWriteImagesFile
*/

