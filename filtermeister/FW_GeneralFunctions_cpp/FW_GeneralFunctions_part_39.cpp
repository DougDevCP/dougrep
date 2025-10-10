{ //scope
#if BIGDOCUMENT
	VRect scaledFilterRect;
#else
	Rect scaledFilterRect;
#endif
    OSErr err = noErr;	

#if BIGDOCUMENT
    fmc.outRect32 = GetOutRect();	//needed??
#endif

    fmc.scaleFactor = gStuff->inputRate >> 16;
    fmc.srcPtr = (unsigned8 *) MyAdjustedInData;
    fmc.dstPtr = (unsigned8 *) gStuff->outData;
    fmc.mskData = (unsigned8 *) MyAdjustedMaskData;
#if BIGDOCUMENT
    fmc.rows = fmc.outRect32.bottom - fmc.outRect32.top;
    fmc.columns = fmc.outRect32.right - fmc.outRect32.left;
#else
    fmc.rows = gStuff->outRect.bottom - gStuff->outRect.top;
    fmc.columns = gStuff->outRect.right - gStuff->outRect.left;
#endif
    fmc.srcRowDelta = gStuff->inRowBytes;
    fmc.srcColDelta = gStuff->inHiPlane - gStuff->inLoPlane + 1;
    fmc.inPlanes = fmc.srcColDelta;
    fmc.dstRowDelta = gStuff->outRowBytes;
    fmc.dstColDelta = gStuff->planes;
    fmc.mskRowDelta = gStuff->maskRowBytes;
    fmc.mskColDelta = 1;

#ifndef DONT_USE_MASK
    if (gStuff->haveMask)
    {   // mask stuff already set up for selection mask...
        gNoMask = FALSE;
    }
    else
#endif
    if (fmc.inPlanes >= 4)
    {   // Channel 3 is probably an alpha channel, so
        // use the alpha channel (layer transparency mask)
        // instead of the selection mask
        fmc.mskData = (unsigned8 *) MyAdjustedInData + 3;
        fmc.mskRowDelta = fmc.srcRowDelta;
        fmc.mskColDelta = fmc.srcColDelta;
        gNoMask = FALSE;
    }
    else
    {   // No selection mask and no transparency channel, so
        // we'll just fake a mask that includes the entire
        // filterRect (which will give us a nice rectangular
        // bevel).
        gNoMask = TRUE;
    }

#if BIGDOCUMENT
	//scaledFilterRect = GetFilterRect();
	fmc.filterRect32 = GetFilterRect();	//needed?
	scaledFilterRect = fmc.filterRect32;
	scaleRect32(&scaledFilterRect, 1, fmc.scaleFactor);
    X0 = scaledFilterRect.left;
    Y0 = scaledFilterRect.top;
	fmc.inRect32 = GetInRect();	//needed??
    fmc.x_start = fmc.inRect32.left - X0;
    fmc.y_start = fmc.inRect32.top - Y0;
#else
    copyRect (&scaledFilterRect, &gStuff->filterRect);
    scaleRect (&scaledFilterRect, 1, fmc.scaleFactor);
    X0 = scaledFilterRect.left;
    Y0 = scaledFilterRect.top;
    fmc.x_start = gStuff->inRect.left - X0;
    fmc.y_start = gStuff->inRect.top - Y0;
#endif  


    fmc.x_end = fmc.x_start + fmc.columns;
    fmc.y_end = fmc.y_start + fmc.rows;

    fmc.X = fmc.xmax = scaledFilterRect.right - X0;
    fmc.Y = fmc.ymax = scaledFilterRect.bottom - Y0;

}//scope

    return true;

}


//Note: Requires rect coordinates in original image size
int requestRect (int inLeft, int inTop, int inRight, int inBottom, int scaleFactor)
{
    long    maxScaleFactor;
#if BIGDOCUMENT
	VRect    scaledImage;
#else
    Rect    scaledImage;
#endif

    /*
    //Scale params to 100%
    inLeft *= scaleFactor;
    inTop *= scaleFactor;
    inRight *= scaleFactor;
    inBottom *= scaleFactor;
    */		  

    //Only run in proxy mode
    if (!fmc.doingProxy) return false;

#if BIGDOCUMENT
        fmc.inRect32.left    = inLeft;
        fmc.inRect32.right   = inRight;
        fmc.inRect32.top     = inTop;
        fmc.inRect32.bottom  = inBottom;
		SetInRect(fmc.inRect32);

        fmc.filterRect32 = GetFilterRect();
		CalcProxyScaleFactor (&fmc.filterRect32, &gProxyRect, &maxScaleFactor);
#else
        gStuff->inRect.left   = (short)inLeft;
        gStuff->inRect.right  = (short)inRight;
        gStuff->inRect.top    = (short)inTop;
        gStuff->inRect.bottom = (short)inBottom;
        CalcProxyScaleFactor (&gStuff->filterRect, &gProxyRect, &maxScaleFactor); //Calc maxScaleFactor
#endif

    if (scaleFactor == -888)
    {   // -888 means use calculated optimal scale factor...
        scaleFactor = maxScaleFactor;
    }

// Make sure scale factor is in range [1,MAX_SCALE_FACTOR].
    if (scaleFactor < 1) scaleFactor = 1;
    if (scaleFactor > MAX_SCALE_FACTOR) scaleFactor = MAX_SCALE_FACTOR;

#if LOWZOOM
	scaleFactor = roundUpScaleFactor(scaleFactor);
#else
    // Skip scalefactor 13 or 15...
    if (scaleFactor == 13 || scaleFactor == 15)
        scaleFactor++;  //use next higher scaleFactor (14 or 16)
#endif
		   
#if BIGDOCUMENT
	//fmc.inRect32 = fmc.filterRect32; 
    fmc.inRect32 = GetInRect();	//needed??
	scaleRect32(&fmc.inRect32, 1, scaleFactor);
	scaledImage = fmc.inRect32;
    SetInRect(fmc.inRect32);	//needed??
#else
    //copyRect (&gStuff->inRect, &gStuff->filterRect);
    scaleRect (&gStuff->inRect, 1, scaleFactor);
    copyRect (&scaledImage, &gStuff->inRect);
#endif

    if ((gParams->flags & (AFH_DRAGGING_PROXY|AFH_ZERO_OUTPUT_DRAG|AFH_DRAG_FILTERED_OUTPUT))
                       == (AFH_DRAGGING_PROXY|AFH_ZERO_OUTPUT_DRAG)) 
    {   // Don't need to set up output Rect.  Could save some time.
       #if BIGDOCUMENT
            PISetRect32(&fmc.outRect32, 0, 0, 0, 0); 
            SetOutRect(fmc.outRect32);
       #else
            PISetRect (&gStuff->outRect, 0, 0, 0, 0);
	   #endif
    } else {
       #if BIGDOCUMENT
			fmc.outRect32 = fmc.inRect32;
            SetOutRect(fmc.outRect32);
       #else
            copyRect (&gStuff->outRect, &gStuff->inRect);
       #endif
    }

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

    gStuff->inputRate = gStuff->maskRate = long2fixed(scaleFactor);
    
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

    // Now call AdvanceState() for real, to get the actual proxy pixels.
    addPadding(globals);

    gResult = AdvanceState();

    if (gResult != noErr) {
        // We use the special combo MB_ICONHAND|MB_SYSTEMMODAL
        // to ensure that the message box will be displayed
        // regardless of low memory...
        if (gResult == memFullErr) {
            msgBox( MB_ICONHAND|MB_SYSTEMMODAL,
                    fmc.filterTitleText,//"FilterMeister",
                    "Not enough memory to perform requestRect() at this zoom level."
                    );
            return false;
        }
        else {
            msgBox( MB_ICONHAND|MB_SYSTEMMODAL,
                    fmc.filterTitleText,//"FilterMeister",
                    "Unknown error (%d) while perfoming requestRect()\n",
                    gResult
                    );
            return false;
        }
    }
    
    subtractPadding(globals);

    // Need to set outRect correctly for ShowOutput();
#if BIGDOCUMENT
    fmc.inRect32 = GetInRect();	//needed??	//This was commented out before
	fmc.outRect32 = fmc.inRect32;
	SetOutRect(fmc.outRect32);
#else
    copyRect (&gStuff->outRect, &gStuff->inRect);
#endif

    // Now gStuff->inData should have the (possibly padded) subsampled version of input data,
    // gStuff->maskData may have the (possibly padded) subsampled version of the mask data,
    // and gStuff->outData should be a buffer the size of input data that
    // could be used as a temporary buffer to do intermediate filtering
    // to display proxies

    // MyAdjustedInData and MyAdjustedMaskData will point to the start of the NON-padded
    // data.

    //Setup Values

    //fmc.scaleFactor = gStuff->inputRate >> 16; //Don't use because it confuses restoreImageData()
    fmc.srcPtr = (unsigned8 *) MyAdjustedInData;
    fmc.dstPtr = (unsigned8 *) gStuff->outData;
    fmc.mskData = (unsigned8 *) MyAdjustedMaskData;
#if BIGDOCUMENT
	fmc.outRect32 = GetOutRect();	//needed??
	fmc.rows = fmc.outRect32.bottom - fmc.outRect32.top;
    fmc.columns = fmc.outRect32.right - fmc.outRect32.left;
#else
    fmc.rows = gStuff->outRect.bottom - gStuff->outRect.top;
    fmc.columns = gStuff->outRect.right - gStuff->outRect.left;
#endif
    fmc.srcRowDelta = gStuff->inRowBytes;
    fmc.srcColDelta = gStuff->inHiPlane - gStuff->inLoPlane + 1;
    fmc.inPlanes = fmc.srcColDelta;
    fmc.dstRowDelta = gStuff->outRowBytes;
    fmc.dstColDelta = gStuff->planes;
    fmc.mskRowDelta = gStuff->maskRowBytes;
    fmc.mskColDelta = 1;

#ifndef DONT_USE_MASK
    if (gStuff->haveMask)
    {   // mask stuff already set up for selection mask...
        gNoMask = FALSE;
    }
    else
#endif
    if (fmc.inPlanes >= 4)
    {   // Channel 3 is probably an alpha channel, so
        // use the alpha channel (layer transparency mask)
        // instead of the selection mask
        fmc.mskData = (unsigned8 *) MyAdjustedInData + 3;
        fmc.mskRowDelta = fmc.srcRowDelta;
        fmc.mskColDelta = fmc.srcColDelta;
        gNoMask = FALSE;
    }
    else
    {   // No selection mask and no transparency channel, so
        // we'll just fake a mask that includes the entire
        // filterRect (which will give us a nice rectangular
        // bevel).
        gNoMask = TRUE;
    }
