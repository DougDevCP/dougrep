	int i;
	for (i = 0; sf > desirableScaleFactor[i]; ++i) {
	}
	assert (sf <= desirableScaleFactor[i]);
	if (sf < desirableScaleFactor[i])
		return desirableScaleFactor[i-1];
	return desirableScaleFactor[i];
} /*roundDownScaleFactor*/

#if 0
/* Round down to a desirable scale factor */
int roundDownScaleFactor2(int sf)
{
	int i = sizeof(desirableScaleFactor)/sizeof(desirableScaleFactor[0]) - 1;
	for ( ; i >= 0; --i) {
		if (sf >= desirableScaleFactor[i])
			return desirableScaleFactor[i];
	}
	return desirableScaleFactor[i];
} /*roundDownScaleFactor2*/
#endif

/*****************************************************************************/

void SetupFilterRecordForProxy (GPtr globals, long *scaleFactor, long prevScaleFactor,
                                POINT *offset)
{
    long    maxScaleFactor;
#if BIGDOCUMENT					 
	int     proxW, proxH, inW, inH;
    VRect   scaledImage;
    VRect   savedImage, savedOut; 
    #ifndef DONT_USE_MASK
       VRect   savedMask;
    #endif
#else
    short   proxW, proxH, inW, inH;
    Rect    scaledImage;
    Rect    savedImage, savedOut;
#ifndef DONT_USE_MASK
    Rect    savedMask;
#endif
#endif
    int     oops;
    static long oldScaleFactor = 9999;
    static BOOL scaleFactorChanged = FALSE;
    BOOL    proxyWasTrimmed;
	
    fmc.doingProxy = TRUE;

#if BIGDOCUMENT		
    fmc.filterRect32 = GetFilterRect();
	CalcProxyScaleFactor (&fmc.filterRect32, &gProxyRect, &maxScaleFactor);
#else
    CalcProxyScaleFactor (&gStuff->filterRect, &gProxyRect, &maxScaleFactor);
#endif

    if (*scaleFactor == -888)
	{   // -888 means use calculated optimal scale factor...
        *scaleFactor = maxScaleFactor;
    }

#ifdef LIMIT_MAX_SCALEFACTOR
    // Optionally limit scalefactor to <= maxScaleFactor.
    if (*scaleFactor > maxScaleFactor) *scaleFactor = maxScaleFactor;
#endif

    // Make sure scale factor is in range [1,MAX_SCALE_FACTOR].
    if (*scaleFactor < 1) *scaleFactor = 1;
    if (*scaleFactor > MAX_SCALE_FACTOR) *scaleFactor = MAX_SCALE_FACTOR;  //Allow -888 to work fine for 30k+ images

#if LOWZOOM
	*scaleFactor = roundUpScaleFactor(*scaleFactor);
#else
    // Skip scalefactor 13 or 15...
    if (*scaleFactor == 13 || *scaleFactor == 15)
        (*scaleFactor)++;  //use next higher scaleFactor (14 or 16)
	//Info ("%d",*scaleFactor);
	//*scaleFactor = roundUpScaleFactor(*scaleFactor);
	//Info ("%d",*scaleFactor);
#endif
    

    if (*scaleFactor != oldScaleFactor)
    {
        scaleFactorChanged = TRUE;
        oldScaleFactor = *scaleFactor;
        fmc.zoomFactor = *scaleFactor;
    }

#if BIGDOCUMENT
	fmc.inRect32 = fmc.filterRect32;
	scaleRect32(&fmc.inRect32, 1, *scaleFactor);
	scaledImage = fmc.inRect32;
#else
    copyRect (&gStuff->inRect, &gStuff->filterRect);
    scaleRect (&gStuff->inRect, 1, *scaleFactor);
    copyRect (&scaledImage, &gStuff->inRect);
#endif

    // Trim (scaled) inRect to fit within proxyRect if necessary...
    proxyWasTrimmed = FALSE;
    proxW = gProxyRect.right - gProxyRect.left;
    proxH = gProxyRect.bottom - gProxyRect.top;

	//Scale down the processed image area to make the whole image scrollable at >100% zoom
	proxW /= fmc.enlargeFactor;
	proxH /= fmc.enlargeFactor;

#if BIGDOCUMENT
    inW = fmc.inRect32.right - fmc.inRect32.left;
    inH = fmc.inRect32.bottom - fmc.inRect32.top;
    if (inW > proxW) {
        fmc.inRect32.left += (inW - proxW)/2;
        fmc.inRect32.right = fmc.inRect32.left + proxW;
        proxyWasTrimmed = TRUE;
    }
    if (inH > proxH) {
        fmc.inRect32.top += (inH - proxH)/2;
        fmc.inRect32.bottom = fmc.inRect32.top + proxH;
        proxyWasTrimmed = TRUE;
    }
#else
    inW = gStuff->inRect.right - gStuff->inRect.left;
    inH = gStuff->inRect.bottom - gStuff->inRect.top;
    if (inW > proxW) {
        gStuff->inRect.left += (inW - proxW)/2;
        gStuff->inRect.right = gStuff->inRect.left + proxW;
        proxyWasTrimmed = TRUE;
    }
    if (inH > proxH) {
        gStuff->inRect.top += (inH - proxH)/2;
        gStuff->inRect.bottom = gStuff->inRect.top + proxH;
        proxyWasTrimmed = TRUE;
    }
#endif

    // Apply (scaled) offset...
/*#ifdef HIGHZOOM
    //This code causes image cut off in the preview
#if BIGDOCUMENT
    fmc.inRect32.left   += (offset->x / (*scaleFactor) / fmc.enlargeFactor);
    fmc.inRect32.right  += (offset->x / (*scaleFactor) / fmc.enlargeFactor);
    fmc.inRect32.top    += (offset->y / (*scaleFactor) / fmc.enlargeFactor);
    fmc.inRect32.bottom += (offset->y / (*scaleFactor) / fmc.enlargeFactor);
#else
    gStuff->inRect.left   += (short) (offset->x / (*scaleFactor) / fmc.enlargeFactor);
    gStuff->inRect.right  += (short) (offset->x / (*scaleFactor) / fmc.enlargeFactor);
    gStuff->inRect.top    += (short) (offset->y / (*scaleFactor) / fmc.enlargeFactor);
    gStuff->inRect.bottom += (short) (offset->y / (*scaleFactor) / fmc.enlargeFactor);
#endif
#else */

#if BIGDOCUMENT
	fmc.inRect32.left   += (offset->x / *scaleFactor);
    fmc.inRect32.right  += (offset->x / *scaleFactor);
    fmc.inRect32.top    += (offset->y / *scaleFactor);
    fmc.inRect32.bottom += (offset->y / *scaleFactor);		   
#else
	gStuff->inRect.left   += (short) (offset->x / *scaleFactor);
    gStuff->inRect.right  += (short) (offset->x / *scaleFactor);
    gStuff->inRect.top    += (short) (offset->y / *scaleFactor);
    gStuff->inRect.bottom += (short) (offset->y / *scaleFactor);
#endif
//#endif

	//setCtlTextv(0, "%d, %d - %d, %d", gStuff->inRect.left,gStuff->inRect.right,gStuff->inRect.top,gStuff->inRect.bottom ); 

#if BIGDOCUMENT
    if (fmc.inRect32.left < scaledImage.left)
    {   // Oops.  Too far left...
        oops = scaledImage.left - fmc.inRect32.left;
        fmc.inRect32.left  += oops;
        fmc.inRect32.right += oops;
        offset->x += oops * (*scaleFactor);
    }
    else if (fmc.inRect32.right > scaledImage.right)
    {   // Oops.  Too far right...
        oops = fmc.inRect32.right - scaledImage.right;
        fmc.inRect32.left  -= oops;
        fmc.inRect32.right -= oops;
        offset->x -= oops * (*scaleFactor);
    }
    if (fmc.inRect32.top < scaledImage.top)
    {   // Oops.  Too far up...
        oops = scaledImage.top - fmc.inRect32.top;
        fmc.inRect32.top    += oops;
        fmc.inRect32.bottom += oops;
        offset->y += oops * (*scaleFactor);
    }
    else if (fmc.inRect32.bottom > scaledImage.bottom)
    {   // Oops.  Too far down...
        oops = fmc.inRect32.bottom - scaledImage.bottom;
        fmc.inRect32.top    -= oops;
        fmc.inRect32.bottom -= oops;
        offset->y -= oops * (*scaleFactor);
    }
	SetInRect(fmc.inRect32);

    if ((gParams->flags & (AFH_DRAGGING_PROXY|AFH_ZERO_OUTPUT_DRAG|AFH_DRAG_FILTERED_OUTPUT))
                       == (AFH_DRAGGING_PROXY|AFH_ZERO_OUTPUT_DRAG))
    {   // Don't need to set up output Rect.  Could save some time.
        PISetRect32(&fmc.outRect32, 0, 0, 0, 0);
    }
    else
    {
		fmc.outRect32 = fmc.inRect32;
    }
	SetOutRect(fmc.outRect32);
#else
    if (gStuff->inRect.left < scaledImage.left)
    {   // Oops.  Too far left...
        oops = scaledImage.left - gStuff->inRect.left;
        gStuff->inRect.left  += oops;
        gStuff->inRect.right += oops;
        offset->x += oops * (*scaleFactor);
    }
    else if (gStuff->inRect.right > scaledImage.right)
    {   // Oops.  Too far right...
        oops = gStuff->inRect.right - scaledImage.right;
        gStuff->inRect.left  -= oops;
        gStuff->inRect.right -= oops;
        offset->x -= oops * (*scaleFactor);
    }
    if (gStuff->inRect.top < scaledImage.top)
    {   // Oops.  Too far up...
        oops = scaledImage.top - gStuff->inRect.top;
        gStuff->inRect.top    += oops;
        gStuff->inRect.bottom += oops;
        offset->y += oops * (*scaleFactor);
    }
    else if (gStuff->inRect.bottom > scaledImage.bottom)
    {   // Oops.  Too far down...
        oops = gStuff->inRect.bottom - scaledImage.bottom;
        gStuff->inRect.top    -= oops;
        gStuff->inRect.bottom -= oops;
        offset->y -= oops * (*scaleFactor);
    }

    if ((gParams->flags & (AFH_DRAGGING_PROXY|AFH_ZERO_OUTPUT_DRAG|AFH_DRAG_FILTERED_OUTPUT))
                       == (AFH_DRAGGING_PROXY|AFH_ZERO_OUTPUT_DRAG))
    {   // Don't need to set up output Rect.  Could save some time.
        PISetRect (&gStuff->outRect, 0, 0, 0, 0);
    }
    else
    {
        copyRect (&gStuff->outRect, &gStuff->inRect);
    }
#endif	 

    gStuff->inLoPlane = gStuff->outLoPlane = 0;
    gStuff->inHiPlane = gStuff->outHiPlane = gStuff->planes - 1;

#if BIGDOCUMENT
  #ifndef DONT_USE_MASK
	if (gStuff->haveMask)
		fmc.maskRect32 = fmc.inRect32;
    else
  #endif
        PISetRect32 (&fmc.maskRect32, 0, 0, 0, 0);
	SetMaskRect(fmc.maskRect32);
#else				 
  #ifndef DONT_USE_MASK
    if (gStuff->haveMask)
        copyRect(&gStuff->maskRect, &gStuff->inRect);
    else
  #endif
        PISetRect (&gStuff->maskRect, 0, 0, 0, 0);
#endif

    gStuff->inputRate = gStuff->maskRate = long2fixed(*scaleFactor);
 
    if (gParams->flags & AFH_USE_HOST_PADDING)
    {
        gStuff->inputPadding = 0;   // 0=black.
        gStuff->outputPadding = 0;  // 0=black.
        gStuff->maskPadding = 0;    // 0=black (non-selected).
    }
    else
    {
        gStuff->inputPadding = plugInDoesNotWantPadding;
        gStuff->outputPadding = plugInDoesNotWantPadding;
        gStuff->maskPadding = plugInDoesNotWantPadding;
    }



    // If the Big Gulp option is set, try to speed up proxy
    // dragging by requesting the entire (scaled) image whenever
    // the scale factor first changes (and the scaled image
    // needs to be trimmed to fit in the proxy window).
    if ((gParams->flags & AFH_PROXY_BIG_GULP) && scaleFactorChanged && proxyWasTrimmed)
    {
        short tempResult;

        scaleFactorChanged = FALSE;
        // Save the current inRect, outRect, and (maybe) maskRect,
        // and set them to the full (scaled) image rectangle.
#if BIGDOCUMENT
		savedImage = fmc.inRect32;