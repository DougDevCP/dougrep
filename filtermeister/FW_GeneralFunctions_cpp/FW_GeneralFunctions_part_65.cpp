                (HMENU) (INT_PTR)(IDC_CTLBASE+n),        // control ID
				(HINSTANCE)hDllInstance,
                NULL);                          // pointer not needed 

            if (!hCtl) return FALSE;    //failed to create window.
            gParams->ctl[n].hCtl = hCtl;

			//Redirect messages to main window
			SendMessage(hCtl, TB_SETPARENT, (WPARAM)fmc.hDlg, 0);

			//TabCtrl_SetMinTabWidth (gParams->ctl[n].hCtl, 20);
			//SendMessage(hCtl, TB_SETBUTTONWIDTH, 0, MAKELPARAM(50,50));  //redraw=FALSE
			
            // Turn off XP Visual Styles
            gParams->ctl[n].inuse = TRUE;
			// setCtlTheme(n, (int)gParams->ctl[n].theme);

            // Set control font to same as dialog font
            {
                HFONT hDlgFont;
                if ((hDlgFont = (HFONT)SendMessage(fmc.hDlg, WM_GETFONT, 0, 0L)) != NULL)
                {
                    gParams->ctl[n].hFont = hDlgFont;
                    SendMessage(hCtl, WM_SETFONT, (WPARAM)hDlgFont, 0L);  //redraw=FALSE
                }
            }

            setCtlText(n,t);

		}
        break;

    case CC_UNUSED:
    case CC_SPINNER:
    case CC_UPDOWN:
    case CC_PROGRESSBAR:
	case CC_TOOLTIP:
    case CC_ANIMATION:
    case CC_PREVIEW:
    default:
        break;
    } //switch

    gParams->ctl[n].inuse = TRUE;
    gParams->ctl[n].tabControl = -1;    //no tab control
    gParams->ctl[n].tabSheet = -1;      //no tab sheet
    setCtlRange(n, gParams->ctl[n].minval, gParams->ctl[n].maxval);
    setCtlLineSize(n, gParams->ctl[n].lineSize);
    setCtlPageSize(n, gParams->ctl[n].pageSize);
    setCtlVal(n, gParams->ctl[n].initVal);
    enableCtl(n, e); // visible/enabled
	setCtlAnchor (n,ANCHOR_RIGHT);

    // Set default tooltips for predefined controls...
    switch (n) {

/*
    //Don't need them in my FocalBlade Plugin
    case CTL_OK:
        setCtlToolTip(n, (INT_PTR)"Apply filter to main image", 0);
        break;
    case CTL_CANCEL:
        setCtlToolTip(n, (INT_PTR)"Exit without applying filter", 0);
        break;
*/

    case CTL_LOGO:
        setCtlToolTip(n, "About this filter", 0);
        break;
    case CTL_PREVIEW:
    case CTL_PROGRESS:
    case CTL_HOST:
    default:
        break;
    }//switch n

    // update cached control values...
#if 0
    fmc.pre_ctl[n] = gParams->ctl[n].val;   //already done by setCtlVal()
#endif
    return TRUE;    //success
} /*createCtl*/

/**********************************************************************/
/* initCtl(n)
/*
/*  Initializes control n to default values.
/*
/**********************************************************************/
int initCtl(int n)
{
    char szBuffer[1024];

    if (n < 0 || n >= N_CTLS) {
        return FALSE;
    }

    sprintf(szBuffer, "Control &%d:", n);
    createCtl(n,
                 CC_STANDARD,
                 szBuffer,
#if 0
                 270 - (n/10 % 10)*5,   //x
                 10*(n % 10) + 5,       //y
                 90,                    //w
                 8,                     //h
#else
                 -1,                    //x default
                 -1,                    //y default
                 -1,                    //h default
                 -1,                    //w default
#endif
                 0,                     //styles
                 0,                     //extended styles
                 0,                     //properties
                 3);                    //visible/enabled
    return TRUE;
} /*fm_initCtl*/


/////// BUGNOTE:
/////// Should not use negative x, y coords as flags, since they
/////// may legitimately be negative; e.g., in a multi-monitor
/////// environment.  !!!!!!! ?????? !!!!!!

/**********************************************************************/
/*  setDialogPos(fAbs, x, y, w, h)
/*
/*  Sets the position and size of the dialog window.  If fAbs is true,
/*  x and y are absolute screen coordinates; otherwise, x and y are
/*  relative to the upper-left corner of the client area in the host
/*  application's main window.  If x and y are set to -1, the dialog
/*  window will be centered within the host client area or the working
/*  area of the entire screen, depending on whether fAbs is false or
/*  true, respectively.  Otherwise, if either x or y is negative, the
/*  position of the dialog will not be changed. If w or h is negative,
/*  the size of the dialog window will not be changed.
/*
/*  All measurements are in dialog box units (DBUs).
/*
/**********************************************************************/
int setDialogPos(int fAbs, int x, int y, int w, int h)
{
    UINT flags = SWP_NOZORDER | SWP_NOACTIVATE;
    HWND hParent;
    int  nHeight;
    int  nWidth;
    RECT rcDialog;
    RECT rcParent;
    RECT workArea;
    int  xScreen;
    int  yScreen;

#if 0
    Info("setDialogPos(%d, %d, %d, %d, %d)", fAbs, x, y, w, h);
#endif

    /* Convert dialog units to pixels */
    {
        RECT r;
        r.left = x;
        r.top = y;
        r.right = w;
        r.bottom = h;
        MapDialogRect(fmc.hDlg, &r);
        // Need to add SM_CYCAPTION, SM_CXDLGFRAME, etc., depending on dialog box style?
        // Or could add the difference between dialog window size and dialog client size,
        // so it automatically adjusts for caption/no caption, border style, etc.?
        if (x >= 0) x = r.left;
        if (y >= 0) y = r.top;
#if 1
        GetWindowRect(fmc.hDlg, &rcDialog);
        GetClientRect(fmc.hDlg, &rcParent);
        if (w >= 0) w = r.right + (rcDialog.right-rcDialog.left) - (rcParent.right-rcParent.left);
        if (h >= 0) h = r.bottom + (rcDialog.bottom-rcDialog.top) - (rcParent.bottom-rcParent.top);

#else
        if (w >= 0) w = r.right + 2*GetSystemMetrics(SM_CXDLGFRAME);
        if (h >= 0) h = r.bottom + 2*GetSystemMetrics(SM_CYDLGFRAME) + GetSystemMetrics(SM_CYCAPTION);
#endif
    }

    if (w < 0 || h < 0) {
        // don't resize the dialog
        flags |= SWP_NOSIZE;
        GetWindowRect(fmc.hDlg, &rcDialog);
        nWidth  = rcDialog.right  - rcDialog.left;
        nHeight = rcDialog.bottom - rcDialog.top;
    }
    else {
        nWidth = w;
        nHeight = h;
        //could optimize following by doing it only for stretched images
        //and vertical gradients...
        flags |= SWP_NOCOPYBITS;  //so background will be redrawn when changing size
    }

    if (x == -1 && y == -1) {
        // center the dialog on screen if fAbs (or no parent);
        // else center in client area of parent.
        if  (fAbs || !(hParent = GetParent(fmc.hDlg)))
            hParent = GetDesktopWindow();

        GetClientRect(hParent, &rcParent);
        ClientToScreen(hParent, (LPPOINT)&rcParent.left);  // point(left,  top)
        ClientToScreen(hParent, (LPPOINT)&rcParent.right); // point(right, bottom)

        
        //Get Screen area without the task bar 
        //-> the bottom of the dialog isn't hidden by the taskbar anymore
        SystemParametersInfo(SPI_GETWORKAREA, 0, &workArea, 0);

        x = max(rcParent.right - rcParent.left - nWidth, 0) / 2
            + rcParent.left;
        //xScreen = GetSystemMetrics(SM_CXSCREEN);
        xScreen = workArea.right - workArea.left;
        if  (x + nWidth > xScreen)
            x = max (0, xScreen - nWidth);

        y = max(rcParent.bottom - rcParent.top - nHeight, 0) / 3
            + rcParent.top;        
        //yScreen = GetSystemMetrics(SM_CYSCREEN);
        yScreen = workArea.bottom - workArea.top;
        if  (y + nHeight > yScreen)
            y = max(0 , yScreen - nHeight);
    }
    else if (x < 0 || y < 0) {
        // don't move the dialog
        flags |= SWP_NOMOVE;
    }
    else if (!fAbs) {
        // Compute coords relative to client area of parent...
        if  ( ! (hParent = GetParent(fmc.hDlg)))
            hParent = GetDesktopWindow();

        GetClientRect(hParent, &rcParent);
        ClientToScreen(hParent, (LPPOINT)&rcParent.left);  // point(left,  top)

        x += rcParent.left;
        y += rcParent.top;

    }


    return SetWindowPos(fmc.hDlg, NULL,
                        x, y, nWidth, nHeight,
                        flags);

} /*fm_setDialogPos*/


/**********************************************************************/
/* setDialogMaxMin()
/*  Set the minum and maximum size of the dialog
/*
/**********************************************************************/
int setDialogMinMax(int mintrackX, int mintrackY, int maxtrackX, int maxtrackY) //int maxsizeX,int maxsizeY,int maxposX, int maxposY,
{

    RECT rcDialog;
    RECT rcParent;
    int borderWidth, borderHeight;

    //Calulate window border width and height to add to MaxMinInfo
    GetWindowRect(fmc.hDlg, &rcDialog);
    GetClientRect(fmc.hDlg, &rcParent);
    borderWidth = (rcDialog.right-rcDialog.left) - (rcParent.right-rcParent.left);
    borderHeight = (rcDialog.bottom-rcDialog.top) - (rcParent.bottom-rcParent.top);
	
    //Set sizes
	if (mintrackX<=0) 
        MaxMinInfo.ptMinTrackSize.x = 0;
    else
        MaxMinInfo.ptMinTrackSize.x = HDBUsToPixels(mintrackX) + borderWidth;

	if (mintrackY<=0) 
        MaxMinInfo.ptMinTrackSize.y =0;
    else
        MaxMinInfo.ptMinTrackSize.y = VDBUsToPixels(mintrackY) + borderHeight;

	if (maxtrackX<=0) 
        MaxMinInfo.ptMaxTrackSize.x = 0;
    else
        MaxMinInfo.ptMaxTrackSize.x = HDBUsToPixels(maxtrackX) + borderWidth; 

	if (maxtrackY<=0) 
        MaxMinInfo.ptMaxTrackSize.y = 0;
    else
        MaxMinInfo.ptMaxTrackSize.y = VDBUsToPixels(maxtrackY) + borderHeight;

    return true;

}/*fm_getDialogWidth*/



int getDialogPos(int w, int t)
{

	RECT rc;