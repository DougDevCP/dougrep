	int val;

	if (t==0){ //Full window
		
		GetWindowRect(fmc.hDlg, &rc);

	} else { //Client area

		GetClientRect(fmc.hDlg, &rc);
	}

	switch (w){
		case 0: val = PixelsToHDBUs(rc.left); break;
		case 1: val = PixelsToVDBUs(rc.top); break;
		case 2: val = PixelsToHDBUs(rc.right - rc.left); break;
		case 3: val = PixelsToVDBUs(rc.bottom - rc.top); break;
	}

	return val;

}


/**********************************************************************/
/* getDialogWidth()
/*  Gets the width of the client area of the dialog.
/*
/**********************************************************************/
int getDialogWidth(void)
{
    RECT rcParent;
      
    //Get Pixel values
    GetClientRect(fmc.hDlg, &rcParent);
    //Convert to DBU
    return PixelsToHDBUs(rcParent.right);

}/*fm_getDialogWidth*/

int getDialogPixelWidth(void)
{
    RECT rcParent;
    GetClientRect(fmc.hDlg, &rcParent);
    return rcParent.right;
}

/**********************************************************************/
/* getDialogHeight()
/*  Gets the height of the client area of the dialog.
/*
/**********************************************************************/
int getDialogHeight(void)
{
    RECT rcParent;

    //Get Pixel values
    GetClientRect(fmc.hDlg, &rcParent);
    //Convert to DBU
    return PixelsToVDBUs(rcParent.bottom);

}/*fm_getDialogHeight*/

int getDialogPixelHeight(void)
{
    RECT rcParent;
    GetClientRect(fmc.hDlg, &rcParent);
    return rcParent.bottom;
}


/**********************************************************************/
/* setDialogStyle(TitleBar |...)
/*  Sets various styles of the dialog.
/*
/**********************************************************************/
int setDialogStyle(int flags)
{
#if 0
    //disallow MaxBox for now -- it allows dialog to be maximized by
    //double-clicking the titlebar (NoSysMenu must also be specified).
    //The problem is that if you exit while maximized, then re-invoke,
    //it comes back in normal mode but with max size, so can't then
    //"unmaximize" the dialog.  Yecch.
    flags &= ~WS_MAXIMIZEBOX;
#endif
    SetWindowLong(fmc.hDlg, GWL_STYLE,
        GetWindowLong(fmc.hDlg, GWL_STYLE) | flags);
    //use SWP_FRAMECHANGED to force nonclient area update...
    return SetWindowPos(fmc.hDlg, NULL,
                        0, 0, 0, 0,
                        SWP_FRAMECHANGED|SWP_NOZORDER|SWP_NOACTIVATE|SWP_NOSIZE|SWP_NOMOVE);
} /*fm_setDialogStyle*/

/**********************************************************************/
/* clearDialogStyle(TitleBar |...)
/*  Clears various styles of the dialog.
/*
/**********************************************************************/
int clearDialogStyle(int flags)
{
    SetWindowLong(fmc.hDlg, GWL_STYLE,
        GetWindowLong(fmc.hDlg, GWL_STYLE) & ~flags);
    //use SWP_FRAMECHANGED to force nonclient area update...
    return SetWindowPos(fmc.hDlg, NULL,
                        0, 0, 0, 0,
                        SWP_FRAMECHANGED|SWP_NOZORDER|SWP_NOACTIVATE|SWP_NOSIZE|SWP_NOMOVE);
} /*fm_clearDialogStyle*/

// !!!! Should recalc Edit-mode dialog size when TOOLWINDOW is set/cleared
// !!!! and when dialog is resized!!!!


/**********************************************************************/
/* setDialogStyleEx(ToolWindow |...)
/*  Sets various extended styles of the dialog.
/*
/**********************************************************************/
int setDialogStyleEx(int flags)
{
    SetWindowLong(fmc.hDlg, GWL_EXSTYLE,
        GetWindowLong(fmc.hDlg, GWL_EXSTYLE) | flags);
    //use SWP_FRAMECHANGED to force nonclient area update...
    return SetWindowPos(fmc.hDlg, NULL,
                        0, 0, 0, 0,
                        SWP_FRAMECHANGED|SWP_NOZORDER|SWP_NOACTIVATE|SWP_NOSIZE|SWP_NOMOVE);
} /*fm_setDialogStyleEx*/

/**********************************************************************/
/* clearDialogStyleEx(ToolWindow |...)
/*  Clears various extended styles of the dialog.
/*
/**********************************************************************/
int clearDialogStyleEx(int flags)
{
    SetWindowLong(fmc.hDlg, GWL_EXSTYLE,
        GetWindowLong(fmc.hDlg, GWL_EXSTYLE) & ~flags);
    //use SWP_FRAMECHANGED to force nonclient area update...
    return SetWindowPos(fmc.hDlg, NULL,
                        0, 0, 0, 0,
                        SWP_FRAMECHANGED|SWP_NOZORDER|SWP_NOACTIVATE|SWP_NOSIZE|SWP_NOMOVE);
} /*fm_clearDialogStyleEx*/

/**********************************************************************/
/* setDialogText("title")
/*  Sets the caption in the title bar.
/*
/**********************************************************************/
int setDialogText(char * title)
{
    strncpy(gParams->gDialogTitle, (char *)title, MAX_LABEL_SIZE+1);
    gParams->gDialogTitle[MAX_LABEL_SIZE] = '\0';   //ensure null-terminated
    return SetWindowText(fmc.hDlg, formatString(gParams->gDialogTitle));
} /*fm_setDialogText*/

/**********************************************************************/
/* setDialogTextv("format", ...)
/*  Sets the caption in the title bar with printf-style formatting.
/*
/**********************************************************************/
int setDialogTextv(char * iFmt,...)
{
    char szBuffer[1024];
    va_list ap;

    va_start(ap, iFmt);
    _vsnprintf(szBuffer, sizeof(szBuffer), (char *)iFmt, ap);
    szBuffer[sizeof(szBuffer)-1] = 0;   //ensure NUL-terminated
    va_end(ap);
    return setDialogText(szBuffer);
} /*fm_setDialogTextv*/

/**********************************************************************/
/* setDialogGradient(color1,color2,direction)
/*  Fills the background of the client area of the dialog box with
/*  a vertical (direction=0) or horizontal (direction=1) linear gradient.
/*
/**********************************************************************/
int setDialogGradient(int color1, int color2, int direction)
{
    if (gParams->gDialogGradientColor1 == (COLORREF)color1 &&
        gParams->gDialogGradientColor2 == (COLORREF)color2 &&
        gParams->gDialogGradientDirection == direction) {
        //quick exit if no change...
        return TRUE;
    }

    gParams->gDialogGradientColor1 = color1;
    gParams->gDialogGradientColor2 = color2;
    gParams->gDialogGradientDirection = direction;
    /* Force the background to be redrawn */
#if 0
    InvalidateRect(fmc.hDlg,
                          NULL, /*entire client area */
                          TRUE); /*erase background*/
    return UpdateWindow(fmc.hDlg);   //force a WM_PAINT
#elif 1
    return RedrawWindow(
            fmc.hDlg,   // handle of window
            NULL,   // address of structure with update rectangle, NULL=>entire client area
            NULL,   // handle of update region
            RDW_ERASE|RDW_INVALIDATE|RDW_ERASENOW|RDW_UPDATENOW     // array of redraw flags
       );
#else
    //What if just send WM_ERASEBKGND???
    return (int)SendMessage(fmc.hDlg, WM_ERASEBKGND, 0, 0 );
#endif
} /*fm_setDialogGradient*/

/**********************************************************************/
/* setDialogColor(color)
/*  Sets the background color for the client area of the dialog box.
/*
/**********************************************************************/
int setDialogColor(int color)
{
    return setDialogGradient(color, color, 0);
} /*fm_setDialogColor*/

int setDialogSysColor(int con)
{
	int color = (int)GetSysColor(con);
    return setDialogGradient(color, color, 0);
} /*fm_setDialogSysColor*/

int getDialogColor()
{
	return gParams->gDialogGradientColor1;
}


/**********************************************************************/
/* setDialogImage(filename)
/*  Applies an image from the specified file as the background image
/*  for the client area of the dialog box, tiling or stretching it to
/*  fit if requested by setDialogImageMode.
/*
/**********************************************************************/
int setDialogImage(char * filename)
{
    strncpy(gParams->gDialogImageFileName, (char *)filename, _MAX_PATH+1);
    gParams->gDialogImageFileName[_MAX_PATH] = '\0';   //ensure null-terminated
    gParams->gDialogImage_save = gDialogImage = gParams->gDialogImageFileName;
    /* Force the background to be redrawn */
    return RedrawWindow(
            fmc.hDlg,   // handle of window
            NULL,   // address of structure with update rectangle, NULL=>entire client area
            NULL,   // handle of update region
            RDW_ERASE|RDW_INVALIDATE|RDW_ERASENOW|RDW_UPDATENOW     // array of redraw flags
       );
} /*fm_setDialogImage*/

/**********************************************************************/
/* setDialogImageMode({EXACT==0|TILED==1|STRETCHED==2}, <stretch_mode>)
/*  Specifies whether the background image is to the tiled or
/*  stretched to fill the entire dialog, or used exactly as is.
/*  If the image is to be stretched, then the stretch mode is
/*  also specified.
/*
/**********************************************************************/
int setDialogImageMode(int mode, int stretchMode)
{
    gParams->gDialogImageMode = mode;
    if (stretchMode != 0) gParams->gDialogImageStretchMode = stretchMode;
    /* Force the background to be redrawn */
    return RedrawWindow(
            fmc.hDlg,   // handle of window
            NULL,   // address of structure with update rectangle, NULL=>entire client area
            NULL,   // handle of update region
            RDW_ERASE|RDW_INVALIDATE|RDW_ERASENOW|RDW_UPDATENOW     // array of redraw flags
       );
} /*setDialogImageMode*/

/**********************************************************************/
/* setDialogDragMode({TitleBar==0|Background==1|None==2})
/*  Determines whether the filter dialog box can be dragged by the
/*  title bar only (0), by dragging the title bar or anywhere on the
/*  dialog background (1), or not at all (2).  The default mode is 0.
/**********************************************************************/
int setDialogDragMode(int mode)
{
    gParams->gDialogDragMode = mode;
    return TRUE;
} /*fm_setDialogDragMode*/

/**********************************************************************/
/* setDialogRegion(<region_expression>)
/*  Sets the clipping region (outline) of the filter dialog.
/*
/**********************************************************************/
int setDialogRegion(INT_PTR rgn)
{
    //gParams->gDialogRegion = (HRGN)rgn; //save across invocations
    return SetWindowRgn(
                fmc.hDlg,   // handle to window whose window region is to be set
                (HRGN)rgn,  // handle to region 
                TRUE    // window redraw flag 
                       );
    //delete the component regions with DeleteObj()???
} /*fm_setDialogRegion*/
