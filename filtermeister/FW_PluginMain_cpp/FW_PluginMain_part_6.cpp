		fmc.inRect32 = scaledImage;
		SetInRect(fmc.inRect32);

		savedOut = fmc.outRect32;

        if ((gParams->flags & (AFH_DRAGGING_PROXY|AFH_ZERO_OUTPUT_DRAG|AFH_DRAG_FILTERED_OUTPUT))
                           == (AFH_DRAGGING_PROXY|AFH_ZERO_OUTPUT_DRAG))
            // Don't need to set up output Rect.  Could save some time.
            PISetRect32(&fmc.outRect32, 0, 0, 0, 0);
        else
			fmc.outRect32 = scaledImage;
		SetOutRect(fmc.outRect32);

    #ifndef DONT_USE_MASK
        if (gStuff->haveMask)
        {
			savedMask = fmc.maskRect32;
			fmc.maskRect32 = scaledImage;
			SetMaskRect(fmc.maskRect32);
        }
    #endif
#else
        copyRect (&savedImage, &gStuff->inRect);
        copyRect (&gStuff->inRect, &scaledImage);
        copyRect (&savedOut, &gStuff->outRect);

        if ((gParams->flags & (AFH_DRAGGING_PROXY|AFH_ZERO_OUTPUT_DRAG|AFH_DRAG_FILTERED_OUTPUT))
                           == (AFH_DRAGGING_PROXY|AFH_ZERO_OUTPUT_DRAG))
        {   // Don't need to set up output Rect.  Could save some time.
            PISetRect (&gStuff->outRect, 0, 0, 0, 0);
        }
        else
        {
            copyRect (&gStuff->outRect, &scaledImage);
        }

   #ifndef DONT_USE_MASK
        if (gStuff->haveMask)
        {
            copyRect (&savedMask, &gStuff->maskRect);
            copyRect (&gStuff->maskRect, &scaledImage);
        }
   #endif
#endif

        addPadding(globals);



        // Call AdvanceState() to try to gulp in the entire (scaled) image.
        // The result is ignored, because we don't care if it fails (due to
        // insufficient memory).
        tempResult = AdvanceState();


#if BIGDOCUMENT
        // Restore the current values of inRect, outRect, and maskRect.
		fmc.inRect32 = savedImage;
		SetInRect(fmc.inRect32);
		fmc.outRect32 = savedOut;
		SetOutRect(fmc.outRect32);
   #ifndef DONT_USE_MASK
        if (gStuff->haveMask)
        {
			fmc.maskRect32 = savedMask;
			SetMaskRect(fmc.maskRect32);
        }
   #endif
#else
        // Restore the current values of inRect, outRect, and maskRect.
        copyRect (&gStuff->inRect, &savedImage);
        copyRect (&gStuff->outRect, &savedOut);
   #ifndef DONT_USE_MASK
        if (gStuff->haveMask)
        
                copyRect (&gStuff->maskRect, &savedMask);
   #endif
#endif
    } /*if Big Gulp*/

    

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
                    "Not enough memory to generate a preview image at this zoom level."
                    );
        }
        else {
            msgBox( MB_ICONHAND|MB_SYSTEMMODAL,
                    fmc.filterTitleText,//"FilterMeister",
                    "Unknown error (%d) while trying to generate a preview image.",
                    gResult
                    );
        }
    }
    
    subtractPadding(globals);

    // Need to set outRect correctly for ShowOutput();
#if BIGDOCUMENT
    fmc.inRect32 = GetInRect();	//needed??
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

} /*SetupFilterRecordForProxy*/

/*****************************************************************************/

/* Prepare to filter an image.  If the plug-in filter needs a large amount
   of buffer memory, this routine should set the bufferSpace field to the
   number of bytes required. */

void DoPrepare (GPtr globals)
{

	int maxSpace = gStuff->maxSpace;

    ValidateParameters (globals);

    if (gResult != noErr) return;

#ifndef NOTRACE
    Trace (globals, "DoPrepare:\n"
        " gStuff->bufferSpace = %d\n"
        " (before setting it to 0)",
        gStuff->bufferSpace);
#endif
    
    
    //Added by Harald Heim, February 2003
    //Suppress dialog when there is no filter parameter block
    //For Plugin Commander Pro
    if (gStuff == NULL)
        gParams->queryForParameters = FALSE; 


/*
	InitContextRecord();
	if (!triggerEvent(0,FME_DOPREPARE,0)){
		//Restore values in case false was returned;
		gStuff->maxSpace = maxSpace;	
		gStuff->bufferSpace = 0;
	}
*/

    gStuff->bufferSpace = 0;


    
    //Added by Harald Heim, Sep 26, 2003
    //Make Photoshop free up all the memory ??? -> Doesn't work
    //gStuff->maxSpace = 0;



    //Added by Harald Heim, Jul 6, 2002
    //if (gStuff->isFloating==FALSE && gStuff->haveMask==TRUE) 
    //      gStuff->autoMask = FALSE;

    
}

/*****************************************************************************/

//do initialization at start of each filter run...

static void InitBeforeFilter(void)
{
    int i;

    //initialize random number generator for the proxy image
    srand(1);

    // Set gamma = 1.0...
    //fm_setGamma(1.0);

    //initialize anonymous cells
    if (Dont_Initialize_Cells == 0)
    {
        for (i = 0; i < N_CELLS; i++) {
            fmc.cell[i] = 0;
        }
    }

    // Cache current control values in fmc.pre_ctl[].
    for (i=0; i < N_CTLS; i++) {
        fmc.pre_ctl[i] = gParams->ctl[i].val;
    }

    //init fmc.scaleFactor, etc. ?????

    call_FFP(FFP_OnFilterStart,(INT_PTR)&fmc);//OnFilterStart(&fmc);


} /*InitBeforeFilter*/

/*****************************************************************************/

void DoStart (GPtr globals)
{

    //Added by Harald Heim
    int i;
	Boolean displayDialog = true;
	//OSErr err;


    ValidateParameters (globals);

    if (gResult != noErr) return;

#ifndef NOTRACE
    Trace (globals, "DoStart:\n"
        " gParams->queryForParameters = %#x",
        gParams->queryForParameters);
#endif
    
    // Initialize static parts of the FM context record.
    InitContextRecord();

    // Synchronize FM context flags to global (parameter) flags...
    fmc.isTileable = (gParams->flags & AFH_IS_TILEABLE) != 0;

    // Copy info from host's filter record...
    
    fmc.hostSerialNumber = gStuff->serialNumber;
    fmc.displayDialog = gParams->queryForParameters;
    fmc.doingScripting = 0;
	fmc.planes = gStuff->planes;

#if BIGDOCUMENT
	// Following routines check at runtime whether to use 16- or 32-bit host fields...
	fmc.imageSize32	= GetImageSize();
	fmc.filterRect32 = GetFilterRect();
	fmc.inRect32 = GetInRect();
	fmc.outRect32 = GetOutRect();
	fmc.maskRect32 = GetMaskRect();
	fmc.floatCoord32 = GetFloatCoord();
	fmc.wholeSize32 = GetWholeSize();	

    // Following two fields are only exposed in the FD's dictionary...
	fmc.filterRectWidth32   = fmc.filterRect32.right - fmc.filterRect32.left;
	fmc.filterRectHeight32  = fmc.filterRect32.bottom - fmc.filterRect32.top;

	//Fill anyway -> in case used by filter code
	fmc.imageHeight = fmc.imageSize32.v;
    fmc.imageWidth = fmc.imageSize32.h;
	fmc.filterRectWidth = fmc.filterRect32.right - fmc.filterRect32.left;
    fmc.filterRectHeight = gStuff->filterRect.bottom - gStuff->filterRect.top;
    fmc.filterRectLeft = fmc.filterRect32.left;
    fmc.filterRectTop = fmc.filterRect32.top;
    fmc.filterRectRight = fmc.filterRect32.right;
    fmc.filterRectBottom = fmc.filterRect32.bottom;
    fmc.wholeWidth = fmc.wholeSize32.h;
    fmc.wholeHeight = fmc.wholeSize32.v;
#else
    fmc.imageHeight = gStuff->imageSize.v;
    fmc.imageWidth = gStuff->imageSize.h;
    fmc.filterRectLeft = gStuff->filterRect.left;
    fmc.filterRectTop = gStuff->filterRect.top;
    fmc.filterRectRight = gStuff->filterRect.right;
    fmc.filterRectBottom = gStuff->filterRect.bottom;
    fmc.filterRectWidth = gStuff->filterRect.right - gStuff->filterRect.left;
    fmc.filterRectHeight = gStuff->filterRect.bottom - gStuff->filterRect.top;
    fmc.wholeWidth = gStuff->wholeSize.h;
    fmc.wholeHeight = gStuff->wholeSize.v;
#endif

    fmc.planes = gStuff->planes; 
    
    if (gStuff->imageMode == plugInModeCMYKColor || gStuff->imageMode == plugInModeCMYK64)
        fmc.planesWithoutAlpha = 4;
    else if (gStuff->imageMode == plugInModeGrayScale || gStuff->imageMode == plugInModeGray16 
        || gStuff->imageMode == plugInModeDuotone || gStuff->imageMode == plugInModeDuotone16
        || gStuff->imageMode == plugInModeBitmap || gStuff->imageMode == plugInModeIndexedColor)
        fmc.planesWithoutAlpha = 1;
    else /*if (gStuff->imageMode ==  plugInModeMultichannel || gStuff->imageMode == plugInModeDeepMultichannel