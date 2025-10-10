    // Question: Does this automatically take care of all
    // needed background erases?  If so, we can delete the
    // logic that does this everywhere else...
    // To test this, uncomment the following line:
    bEraseBackground = FALSE;
    if (scaleFactor > previousScaleFactor || gFmcp->enlargeFactor != prevEnlargeFactor) {
        // If scale factor has increased, proxy image will
        // be smaller, so we need to erase the background.
        // (Actually, don't need to erase background if image
        // is still clipped to preview window at the new scale
        // factor -- we can do this optimization later if
        // unneeded flashing is a problem.)
        bEraseBackground = TRUE;
    }
    previousScaleFactor = scaleFactor;
	prevEnlargeFactor = gFmcp->enlargeFactor;

#ifdef APP
    if (image != app.inputIndex) bEraseBackground = TRUE; 
    image = app.inputIndex;
#endif

    // Invalidate Proxy Item
    hProxy = GetDlgItem(hDlg, IDC_PROXY);
    GetWindowRect(hProxy, &imageRect);
    ScreenToClient (hDlg, (LPPOINT)&imageRect);
    ScreenToClient (hDlg, (LPPOINT)&(imageRect.right));
    InvalidateRect (hDlg, &imageRect, bEraseBackground);

} /*UpdateProxy*/

/*****************************************************************************/

int doAction(CTLACTION action)
{
    int iCtl; // Added by Ognen Genchev
    //Perform the specified action for this control,,,
    switch(action) {
    
    case CA_SIZE: //Invoke FME_SIZE && FME_EXITSIZE events

        gFmcp->n = 0;
        gFmcp->e = FME_SIZE;
        gFmcp->previous = 0;
        gFmcp->ctlMsg = WM_SIZE;
        gFmcp->ctlCode = 0;
        gFmcp->ctlClass = gParams->ctl[0].ctlclass;

        call_FFP(FFP_OnCtl,(INT_PTR)gFmcp); //OnCtl(gFmcp);
        
        //Make sure that the preview is displayed correctly
        GetProxyItemRect (MyHDlg, &gProxyRect); 
        SetupFilterRecordForProxy (MyGlobals, &scaleFactor, scaleFactor, &imageOffset);
        gFmcp->n = 0;
        gFmcp->e = FME_EXITSIZE;
        gFmcp->previous = 0;
        gFmcp->ctlMsg = WM_EXITSIZEMOVE;
        gFmcp->ctlCode = 0;
        gFmcp->ctlClass = gParams->ctl[0].ctlclass;

        call_FFP(FFP_OnCtl,(INT_PTR)gFmcp); //OnCtl(gFmcp);
        break;

    case CA_PREVIEW:
        // Update the preview...
        if (!fDragging) {
            if (gResult == noErr) {
                // Do Filtering operation
                DoFilterRect (MyGlobals);
                // Invalidate Proxy Item
                UpdateProxy(MyHDlg, FALSE);
            }//noErr
        }
        break;
    case CA_APPLY:
		okEventPerformed = 1;//OK Button was pressed

        /*Fix by Ognen Genchev
            Cancel button had the same behaviour as the OK button, remembering the modified value.
        Now it is fixed by storing .val in .initVal thus when pressing cancel button, the previous
        value is remembered.
            Another thing is, default initial values were not stored properly upon CANCEL execution.
            The value written in .cancelVal at plugin initialization fixes this.
      */
        for (iCtl = 0;  iCtl < N_CTLS;  iCtl++) {
            gParams->ctl[iCtl].initVal = gParams->ctl[iCtl].val;
            gParams->ctl[iCtl].cancelVal = gParams->ctl[iCtl].val;
            // end
        }

        // Apply filter to original source image...
        SendMessage(MyHDlg, WM_COMMAND, IDOK, 0);
        break;
    case CA_CANCEL:
        cancelEventPerformed = 1;//Cancel Button was pressed
        // Fix by Ognen Genchev for proper cancel event execution
        for (iCtl = 0;  iCtl < N_CTLS;  iCtl++) {
            gParams->ctl[iCtl].initVal = gParams->ctl[iCtl].cancelVal;
        }
        // end
		// Exit without doing anything...
        SendMessage(MyHDlg, WM_COMMAND, IDCANCEL, 0);
        break;
    case CA_ABOUT:
        // Display the ABOUT dialog box...
        //SendMessage(MyHDlg, WM_COMMAND, IDC_HELP2, -1);
		DoAbout(globals,2);
        break;
    case CA_RESET:
        //Info("action=RESET is not yet implemented.");
        break;
    case CA_NONE:
    default:
        // No action...
        break;
    }//switch action
    return TRUE;
} /*DoAction*/


BOOL PerformControlAction(int n, FME e, int previous, int ctlMsg, int ctlCode,
                          BOOL fActionable, BOOL fProcessed, int overrideBusy)
{
    static int busy = 0;

    if (n < 0 || n >= N_CTLS) return FALSE;

    // Prevent reentry if reentry count != 0
    if (busy && overrideBusy==0) {
#if defined(_DEBUG) && 0
        OutputDebugString("PerformControlAction is BUSY!\n");
#endif
        return FALSE;
    }
    busy++;

#if SKIN
	if (isSkinActive() && gParams->skinStyle>0){
		if (gParams->ctl[n].ctlclass == CC_PUSHBUTTON || gParams->ctl[n].ctlclass == CC_COMBOBOX || gParams->ctl[n].ctlclass == CC_STANDARD || gParams->ctl[n].ctlclass == CC_CHECKBOX){
			if ( (e==FME_MOUSEOVER)) skinDraw(n,3);
			else if (e==FME_MOUSEOUT) skinDraw(n,0);
		}
	}
#endif


#ifdef APP
    if (performAppAction(n, e, previous, ctlMsg, ctlCode, fActionable, fProcessed, overrideBusy)){
        busy--;
        return fProcessed;
    }
#endif

	{
        int res;
        gFmcp->n = n;
        gFmcp->e = e;
        //Added by Harald Heim, May 22, 2003
        gFmcp->previous = previous;

        gFmcp->ctlMsg = ctlMsg;
        gFmcp->ctlCode = ctlCode;
        gFmcp->ctlClass = gParams->ctl[n].ctlclass;

        res = call_FFP(FFP_OnCtl,(INT_PTR)gFmcp); //res = OnCtl(gFmcp);

        if (res) {
            busy--; //decrement reentry count
            return TRUE;   //fully processed, skip default action
        }
    }
    //Perform the default action for this control...
    if (fActionable) {
        doAction(gParams->ctl[n].action);
    }

    busy--; //decrement reentry count
    return fProcessed;   //processed or not
} /*PerformControlAction*/

/*****************************************************************************/

static int xFrom, xTo;
static int yFrom, yTo;

static void scaleControl (HWND hDlg, int nID)
{
    RECT r;
    HWND hCtl = GetDlgItem(hDlg, nID);

    if (hCtl) {
        GetWindowRect(hCtl, &r);                        //screen coords
        MapWindowPoints(NULL, hDlg, (LPPOINT)&r, 2);    //...to client coords
        // Undo the scaling...
        r.left   = (r.left * xTo)/xFrom;    // Order of multiply/divide is important!!
        r.right  = (r.right * xTo)/xFrom;
        r.top    = (r.top * yTo)/yFrom;
        r.bottom = (r.bottom * yTo)/yFrom;
        // Resize and reposition the window.
        SetWindowPos(hCtl, NULL,
                     r.left, r.top,
                     r.right - r.left,
                     r.bottom - r.top,
                     SWP_NOZORDER | SWP_NOACTIVATE);
    }//if
} /*scaleControl*/

static void scaleWindow (HWND hwnd)
{
    RECT r;

    GetWindowRect(hwnd, &r);    //screen coords
    SetWindowPos(hwnd, NULL,
                 0, 0,
                 (r.right - r.left)*xTo/xFrom,
                 (r.bottom - r.top)*yTo/yFrom,
                 SWP_NOMOVE | SWP_NOZORDER | SWP_NOACTIVATE);

} /*scaleWindow*/



/*****************************************************************************/

// Search PATH and FM_PATH for specified file name.
// Returns pointer to full path name if found, else
// returns pointer to original file name.
// CAUTION: Full path name is stored in a static buffer!

const char *FmSearchFile(const char *szFileName)
{
    static char pathbuffer[_MAX_PATH+1];

    /* Search for file in PATH environment variable: */
    _searchenv(szFileName, "PATH", pathbuffer);
    if (*pathbuffer != '\0') {
        //found it in PATH search list...
#if 0
        Info("Found file in PATH:\n%s", pathbuffer);
#endif
        return pathbuffer;
    }
    else {
        /* Search for file in FM_PATH environment variable: */
        _searchenv(szFileName, "FM_PATH", pathbuffer);
        if (*pathbuffer != '\0') {
            //found it in FM_PATH search list...
#if 0
        Info("Found file in FM_PATH:\n%s", pathbuffer);
#endif
            return pathbuffer;
        }
    }
    // If not found in PATH or FM_PATH, then just return
    // the file name verbatim...
#if 0
    Info("Not found in PATH or FM_PATH:\n%s", szFileName);
#endif
    return szFileName;
} /*FmSearchFile*/




LRESULT CALLBACK KeyHookProc( int code, WPARAM wParam, LPARAM lParam){
	
	if (code==HC_ACTION){
		
        if (GetForegroundWindow() == MyHDlg){ //Only run for foreground window

            //Not in combo boxes, only if Alt additionally
            if ( (wParam >= 0x41 && wParam <= 0x5A) || wParam == VK_SUBTRACT ){
                if (getCtlClass(getCtlFocus()) == CC_COMBOBOX) 
                    if (getAsyncKeyStateF(VK_MENU) > -32767)
                        return CallNextHookEx ( KeyHook, code, wParam, lParam);
            }

            //if (wParam==WM_KEYDOWN){
		    if (!(lParam & 2147483648)){ //Check if bit 31 is not set -> key down
		        
				if (!(lParam & 1073741824)){ //Do not call repeatedly when key is pressed down

					//KBDLLHOOKSTRUCT  *pKeyBoard = (KBDLLHOOKSTRUCT *)lParam;
					//triggerEvent((int)pKeyBoard->vkCode,FME_KEYDOWN,0);
					
					triggerEvent((int)wParam,FME_KEYDOWN,0);
				}

		    } else { //WM_KEYUP

			    triggerEvent((int)wParam,FME_KEYUP,0);

		    }

        }

	}
	
	return CallNextHookEx ( KeyHook, code, wParam, lParam);
