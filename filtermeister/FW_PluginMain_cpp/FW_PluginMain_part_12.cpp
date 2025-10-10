

void DoContinue (GPtr globals)
{
    
    if (TestAbort ())
    {
        gResult = 1;
        return;
    }
        
    if (!WarnAdvanceStateAvailable())
    {
        DoFilterRectNoAdvanceState (globals);
    
        if (!DoNextRect (globals))
            {
#if BIGDOCUMENT
				PISetRect32(&fmc.inRect32, 0, 0, 0, 0);
                PISetRect32(&fmc.outRect32, 0, 0, 0, 0);
				SetOutRect(fmc.inRect32);
				SetOutRect(fmc.outRect32);
#else
                PISetRect (&gStuff->inRect, 0, 0, 0, 0);
                PISetRect (&gStuff->outRect, 0, 0, 0, 0);
#endif
            }
    }
    else
    {
#if BIGDOCUMENT
		PISetRect32(&fmc.inRect32, 0, 0, 0, 0);
        PISetRect32(&fmc.outRect32, 0, 0, 0, 0);
		SetOutRect(fmc.inRect32);
		SetOutRect(fmc.outRect32);
#else
        PISetRect (&gStuff->inRect, 0, 0, 0, 0);
        PISetRect (&gStuff->outRect, 0, 0, 0, 0);
#endif
    }
}


/*****************************************************************************/

/* Requests first part of the image to be filtered. */

void DoInitialRect (GPtr globals)
{

#if BIGDOCUMENT
	fmc.filterRect32 = GetFilterRect();	//needed?
    fmc.inRect32        = fmc.filterRect32;
    fmc.inRect32.bottom = fmc.inRect32.top + 1;
	SetInRect(fmc.inRect32);		 
    fmc.outRect32 = fmc.inRect32;
	SetOutRect(fmc.outRect32);
#else
    gStuff->inRect        = gStuff->filterRect;
    gStuff->inRect.bottom = gStuff->inRect.top + 1;
    gStuff->outRect = gStuff->inRect;
#endif

    gStuff->inLoPlane = 0;
    gStuff->inHiPlane = gStuff->planes - 1;
 	   

    gStuff->outLoPlane = gStuff->inLoPlane;
    gStuff->outHiPlane = gStuff->inHiPlane;

}
/*****************************************************************************/

/* Request the next area. */

Boolean DoNextRect (GPtr globals)
{

#if BIGDOCUMENT
	fmc.inRect32 = GetInRect();
    fmc.inRect32.top    = fmc.inRect32.top    + 1;
    fmc.inRect32.bottom = fmc.inRect32.bottom + 1;
	SetOutRect(fmc.inRect32);
    
    fmc.outRect32 = fmc.inRect32;
	SetOutRect(fmc.outRect32);

    return fmc.inRect32.top < fmc.filterRect32.bottom;
#else
    gStuff->inRect.top    = gStuff->inRect.top    + 1;
    gStuff->inRect.bottom = gStuff->inRect.bottom + 1;   
    gStuff->outRect = gStuff->inRect;
    return gStuff->inRect.top < gStuff->filterRect.bottom;
#endif

}

/*****************************************************************************/

/* Do any necessary clean-up. */


void DoFinish (GPtr globals)
{

#ifndef NOTRACE
    Trace (globals, "DoFinish:\n"
                    " Nothing much to do!");
#endif

#ifdef SCRIPTABLE
    WriteScriptParameters();
#endif

	finalCleanup();

}

void finalCleanup(void){

	int i;

	OnFilterExit();

	
#if 0
	//Clean up threads
	terminateThread(0);
#endif

    // Kill any async sound that might still be running...
    //playSoundWave(0);

    //Added by Harald Heim
    //Delete Array if available
    for (i=0; i<N_ARRAYS; i++) freeArray(i);

#if defined(STYLER)
	//Delete image lists
	for (i=0; i<10; i++) freeImageList(i);
#endif    

    //Delete Fonts
    for (i=0; i<N_FONTS; i++){
            if (Font[i] != 0) DeleteObject(Font[i]);
    }

    //Added by Harald Heim
    //free distance buffer if necessary
    if (bufferID != 0){
        UnlockBuffer(bufferID);
        FreeBuffer(bufferID);
        bufferID = 0;
    }

#if defined(IMGSOURCE5) || defined(IMGSOURCE6) //|| definded(IMGSOURCE)
	exitIS();
#endif
	
}




/*****************************************************************************/
/* Some utility functions to support proxies in dialog */

//long long2fixed(long value) {
//    return value << 16;
//}


/*****************************************************************************/

void scaleRect(Rect *l, long n, long d) {					//What if coords are negative???
    // Carefully control rounding so scaled Rect is always
    // a (scaled) subset of the original Rect (we assume the
    // original Rect has no negative coordinates -- otherwise
    // we need to be even more careful!).
    l->left = (int16)((l->left * n + d - 1) / d);   // round up
    l->top = (int16)((l->top * n + d - 1) / d);     // round up
    l->right = (int16)((l->right * n) / d);         // round down
    l->bottom = (int16)((l->bottom * n) / d);       // round down
}

void scaleRect32(VRect *l, long n, long d) {				//What if coords are negative???
    // Carefully control rounding so scaled VRect is always
    // a (scaled) subset of the original VRect (we assume the
    // original VRect has no negative coordinates -- otherwise
    // we need to be even more careful!).
    l->left = ((l->left * n + d - 1) / d);   // round up
    l->top = ((l->top * n + d - 1) / d);     // round up
    l->right = ((l->right * n) / d);         // round down
    l->bottom = ((l->bottom * n) / d);       // round down
}

void shrinkRect(Rect *l, long x, long y) {
    l->left += (int16)x;
    l->top += (int16)y;
    l->right -= (int16)x;
    l->bottom -= (int16)y;
}

void shrinkRect32(VRect *l, long x, long y) {
    l->left += x;
    l->top += y;
    l->right -= x;
    l->bottom -= y;
}
void copyRect(Rect *l, Rect *r) { //make this a macro???
    l->left = r->left;
    l->top = r->top;
    l->right = r->right;
    l->bottom = r->bottom;
}

void copyVRect(VRect *l, const VRect *r)
{
	l->left = r->left;
    l->top = r->top;
    l->right = r->right;
    l->bottom = r->bottom;
}

void copyRect32(VRect *l, const VRect *r) {	//make this a macro???
	*l = *r;
}

void PISetRect32 (VRect *rect, int left, int top, int right, int bottom)
{
	rect->left = left;
	rect->top = top;
	rect->right = right;
	rect->bottom = bottom;
}


/*****************************************************************************/

/* Test for the display pixels proc and if it isn't available, show an alert. */

Boolean CheckProxiesSupported (DisplayPixelsProc displayPixels)
    {
    
    Boolean available = (displayPixels != NULL);
    
    if (!available)
        ShowAlert (16999);
        
    return available;
    
    }

/***********************************************************************************/

/* Computes the scaled down rectangle and the scale factor for the proxy */

#if BIGDOCUMENT
void CalcProxyScaleFactor (const VRect *filterRect, Rect *proxyRect, long *scaleFactor)
#else
void CalcProxyScaleFactor (const Rect *filterRect, Rect *proxyRect, long *scaleFactor)
#endif
{

#if BIGDOCUMENT
    int filterHeight;
    int filterWidth;
    int itemHeight;
    int itemWidth;
    VPoint fraction;
#else
    short filterHeight;
    short filterWidth;
    short itemHeight;
    short itemWidth;
    Point fraction;
#endif

    shrinkRect (proxyRect, 1, 1);        // for the border

    filterHeight = filterRect->bottom - filterRect->top;
    filterWidth  = filterRect->right  - filterRect->left;
    
    itemHeight = proxyRect->bottom - proxyRect->top;
    itemWidth  = proxyRect->right  - proxyRect->left;
    
    if (itemHeight > filterHeight)
        itemHeight = filterHeight;
        
    if (itemWidth > filterWidth)
        itemWidth = filterWidth;
    
    fraction.h = (filterWidth + itemWidth - 1) / itemWidth;
    fraction.v = (filterHeight + itemHeight - 1) / itemHeight;

#if 1
    if (fraction.h > fraction.v) 
        *scaleFactor = fraction.h;
    else
        *scaleFactor = fraction.v;