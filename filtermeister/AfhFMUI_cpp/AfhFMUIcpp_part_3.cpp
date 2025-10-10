    return CallWindowProc(defWndProc, hwnd, message, wParam, lParam);
}
// end

//Declare
BOOL PerformControlAction(int n, FME e, int previous, int ctlMsg, int ctlCode,
                          BOOL fActionable, BOOL fProcessed, int overrideBusy);


#define IDC_PROXY   101


BOOL fDragging = FALSE;
BOOL fDrawing = FALSE;
BOOL fSizing = FALSE;
BOOL fScaling = FALSE;
BOOL fPaint = FALSE;
BOOL fMaximized = FALSE;
BOOL fMinimized = FALSE;
//BOOL fMoving = FALSE;
BOOL fDialogDisplayed = false;


#ifdef START_SCALEFACTOR_1
static long scaleFactor = 1;
#else
static long scaleFactor = -888; // -888 means compute optimal scale factor
#endif



/*****************************************************************************/

/* Calculates the rectangle we will want to use for the proxy for an image. */

void GetProxyItemRect (HWND hDlg, Rect *proxyItemRect)
    {

    RECT    wRect;

    GetWindowRect(GetDlgItem(hDlg, IDC_PROXY), &wRect); 
    ScreenToClient (hDlg, (LPPOINT)&wRect);
    ScreenToClient (hDlg, (LPPOINT)&(wRect.right));

    proxyItemRect->left = (int16)wRect.left;
    proxyItemRect->top  = (int16)wRect.top;
    proxyItemRect->right = (int16)wRect.right;
    proxyItemRect->bottom = (int16)wRect.bottom;
    
    }

/*****************************************************************************/



static unsigned8 *pRGBbuffer = NULL;    //remember to deallocate at exit!!!!
BufferID pRGBbufferID;
static int pRGBbufferSize = 0;

#ifndef APP

void ShowOutputBuffer( GPtr globals, HWND hDlg, BOOL fDragging)
{
    PSPixelMap pixels;
    PSPixelMask mask;
    PAINTSTRUCT  ps;
    RECT  itemBounds;
    RECT  wRect;
#if BIGDOCUMENT
	VRect iRect;
#else
	Rect  iRect;
#endif
    int proxyWidth;
    int proxyHeight;
#if 1
    RECT  rcIntersect;
#endif
    POINT   mapOrigin; 
    HDC     hDC;

	long  scaleFactor = gStuff->inputRate >> 16;

    //startClock();

    if (gResult != noErr) {
        // Got an error somewhere along the way...
        // Get out quick in case the error was returned by
        // AdvanceState(), in which case the input/output data
        // pointers are bogus and can cause severe dyspepsia.
        return;
    }

    // If AFH_DRAG_FILTERED_OUTPUT is set, turn off fDragging locally so we
    // can display the filtered output instead of the raw input...
    if (gParams->flags & AFH_DRAG_FILTERED_OUTPUT)
    {
        fDragging = FALSE;
    }

    GetWindowRect(GetDlgItem(hDlg, IDC_PROXY), &wRect);
    proxyWidth = wRect.right - wRect.left;
    proxyHeight = wRect.bottom - wRect.top;
    mapOrigin.x = 0;
    mapOrigin.y = 0;
    ClientToScreen(hDlg, &mapOrigin);

#if BIGDOCUMENT
	iRect = GetOutRect();
#else
    copyRect(&iRect,&gStuff->outRect);
#endif
#if HIGHZOOM
	//Scale up for display
	if (gFmcp->enlargeFactor != 1){
		iRect.left *= gFmcp->enlargeFactor;
		iRect.right *= gFmcp->enlargeFactor;
		iRect.top *= gFmcp->enlargeFactor;
		iRect.bottom *= gFmcp->enlargeFactor;
	}
#endif

    // Following rounded in different directions if Proxy was shifted off top or left of screen,
    // since the screen coords went negative, and on x86 architecture, integer division always truncates
    // towards zero.  This resulted in some strange off-by-one-pixel artifacts if the proxy
    // got shifted offscreen and then back onscreen.  Fix is to convert wRect to client coords
    // *before* division by 2, so the dividend will always be positive and truncate consistently
    // *down* towards zero.
    //itemBounds.left = ((wRect.right + wRect.left - gStuff->outRect.right + gStuff->outRect.left) / 2) - mapOrigin.x;
    //itemBounds.top = ((wRect.bottom + wRect.top - gStuff->outRect.bottom + gStuff->outRect.top) / 2) - mapOrigin.y;

    /*itemBounds.left = (wRect.right - mapOrigin.x + wRect.left - mapOrigin.x - (gStuff->outRect.right - gStuff->outRect.left)) / 2;
    itemBounds.top = (wRect.bottom - mapOrigin.y + wRect.top - mapOrigin.y - (gStuff->outRect.bottom - gStuff->outRect.top)) / 2;
    itemBounds.right = itemBounds.left + (gStuff->outRect.right - gStuff->outRect.left);
    itemBounds.bottom = itemBounds.top + (gStuff->outRect.bottom - gStuff->outRect.top);*/
	



    //Temporary Deactivated

    if (iRect.bottom - iRect.top <= wRect.bottom - wRect.top)
    {    
        itemBounds.top = (wRect.bottom - mapOrigin.y + wRect.top - mapOrigin.y - (iRect.bottom - iRect.top)) / 2;
        itemBounds.bottom = itemBounds.top + (iRect.bottom - iRect.top);
    } else { // in case preview is sized down
        itemBounds.top = (wRect.bottom - mapOrigin.y + wRect.top - mapOrigin.y - (wRect.bottom - wRect.top))/2;
        itemBounds.bottom = itemBounds.top + (wRect.bottom - wRect.top);
    }

    if (iRect.right - iRect.left <= wRect.right - wRect.left)
    {
        itemBounds.left = (wRect.right - mapOrigin.x + wRect.left - mapOrigin.x - (iRect.right - iRect.left)) / 2;
        itemBounds.right = itemBounds.left + (iRect.right - iRect.left);
    } else { // in case preview is sized down
        itemBounds.left = (wRect.right - mapOrigin.x + wRect.left - mapOrigin.x - (wRect.right - wRect.left))/2;
        itemBounds.right = itemBounds.left + (wRect.right - wRect.left);
    }



    hDC = BeginPaint(hDlg, &ps);

#if 1
    if (IntersectRect(&rcIntersect, &itemBounds, &ps.rcPaint)) {
#endif

    wRect = itemBounds;
    InflateRect(&wRect, 1, 1);
    //FrameRect(hDC, &wRect, GetStockObject(BLACK_BRUSH)); //Moved down


    pixels.version = 1;
    pixels.bounds.top = iRect.top;
    pixels.bounds.left = iRect.left;
    /*pixels.bounds.bottom = gStuff->outRect.bottom;
    pixels.bounds.right = gStuff->outRect.right;*/
    //Info ("%d,%d - %d,%d",itemBounds.top,itemBounds.bottom,gStuff->outRect.top,gStuff->outRect.bottom);

    if ( iRect.bottom - iRect.top <= itemBounds.bottom - itemBounds.top)
    {
        pixels.bounds.bottom = iRect.bottom;
    } else { // in case preview is sized down
        pixels.bounds.bottom = iRect.top + (itemBounds.bottom - itemBounds.top);
    }

    if ( iRect.right - iRect.left <= itemBounds.right - itemBounds.left)
    {
        pixels.bounds.right = iRect.right;
    } else { // in case preview is sized down
        pixels.bounds.right = iRect.left  + (itemBounds.right - itemBounds.left);
    }


    pixels.imageMode = gStuff->imageMode;
    pixels.rowBytes = fDragging ? gStuff->inRowBytes : gStuff->outRowBytes;
    pixels.colBytes = fDragging ? gStuff->inHiPlane - gStuff->inLoPlane + 1
                                : gStuff->outHiPlane - gStuff->outLoPlane + 1;
    pixels.planeBytes = 1;
    pixels.baseAddr = fDragging ? MyAdjustedInData : gStuff->outData;


#ifdef HIGHZOOM
	// >100% Zoom
	//if (pixels.imageMode == plugInModeRGB48 || pixels.imageMode == plugInModeLab48 || pixels.imageMode == plugInModeDeepMultichannel) 
	if (gFmcp->enlargeFactor != 1)
	{
        unsigned8 *inPtr1 = (unsigned8 *)pixels.baseAddr;
        unsigned8 *inBase = (unsigned8 *)pixels.baseAddr;
		unsigned8 *outPtr;
        int iRows = pixels.bounds.bottom - pixels.bounds.top;
		int iCols = pixels.bounds.right - pixels.bounds.left;
		
		int iR,pR;
		int maxRC;
        //int xStart= (iCols - iCols/gFmcp->enlargeFactor)/2;
		//int yStart= (iRows - iRows/gFmcp->enlargeFactor)/2;
		int xStart= 0;
		int yStart= 0;
		
        int rgb16 = pixels.imageMode == plugInModeRGB48 || pixels.imageMode == plugInModeLab48 || pixels.imageMode == plugInModeDeepMultichannel;
        int rgb8 = pixels.imageMode == plugInModeRGBColor || pixels.imageMode == plugInModeLabColor || pixels.imageMode == plugInModeMultichannel;
        int gray16 = pixels.imageMode == plugInModeGray16;
        int gray8 = pixels.imageMode == plugInModeGrayScale;
        int cmyk8 = pixels.imageMode == plugInModeCMYKColor;
        int cmyk16 = pixels.imageMode == plugInModeCMYK64;
        int r,g,b,k,a,iPos,j;
		
		
        if (pRGBbufferSize != proxyWidth*proxyHeight*(gStuff->inHiPlane+1)){ //3 //Only reallocate if size has to be changed
            if (pRGBbuffer) {
                if (pRGBbufferID != 0){
			        UnlockBuffer(pRGBbufferID);
			        FreeBuffer(pRGBbufferID); 
		        } else
			        free(pRGBbuffer);
		        pRGBbufferID = 0;
                pRGBbuffer = NULL;
				pRGBbufferSize = 0;
            }
            
            // Allocate a buffer large enough to cover the entire proxy window.
            if (AllocateBuffer( proxyWidth*proxyHeight * (gStuff->inHiPlane+1), &pRGBbufferID) == noErr){
				pRGBbuffer = (unsigned8 *)LockBuffer(pRGBbufferID, true);
			} else { //Fall back to Windows allocation
				pRGBbuffer = (unsigned8 *)calloc(proxyWidth*proxyHeight, (gStuff->inHiPlane+1)); //3
				pRGBbufferID=0;
			}


            if (!pRGBbuffer) {
#if 1
                ErrorOk("Can't allocate proxy preview buffer for >100% zoom!");
#endif
                pRGBbufferSize = 0;
                goto egress;    
            } else {

                pRGBbufferSize = proxyWidth*proxyHeight*(gStuff->inHiPlane+1); //3
            }
        }


		if (rgb16 || gray16 || cmyk16){
            pR = pixels.rowBytes / 2;
		}else{
            pR = pixels.rowBytes;
		}

		maxRC = (iRows-1)/gFmcp->enlargeFactor*pR + (iCols-1)/gFmcp->enlargeFactor*pixels.colBytes;
		
        outPtr = pRGBbuffer;
        while (iRows--) {

            iR = (-yStart + iRows/gFmcp->enlargeFactor)*pR;
    
            j = iCols;
            while (j--) {
                // Unroll 3 planes...
                iPos = maxRC - (iR  + (-xStart + j/gFmcp->enlargeFactor)*pixels.colBytes); //*pC

                if (rgb8){
                    outPtr[0] = inBase[iPos + 0];
                    outPtr[1] = inBase[iPos + 1];
                    outPtr[2] = inBase[iPos + 2];
                    if (gStuff->inHiPlane==3) {
					    outPtr[3] = inBase[iPos + 3];
				    }
                } else if (rgb16){
                    r = ((int16 *)inBase)[iPos + 0];
                    g = ((int16 *)inBase)[iPos + 1];
                    b = ((int16 *)inBase)[iPos + 2];
					outPtr[0] = (r >> 7) | (r >> 15);
                    outPtr[1] = (g >> 7) | (g >> 15);
                    outPtr[2] = (b >> 7) | (b >> 15);
                    if (gStuff->inHiPlane==3) {
					    a = ((int16 *)inBase)[iPos + 3];
					    outPtr[3] = (a >> 7) | (a >> 15);
				    }
                } else if (gray16){