	case CC_TRACKBAR:
        SendMessage(gParams->ctl[n].hCtl, TBM_SETRANGEMIN, FALSE /*redraw*/,
                    gParams->ctl[n].minval);
        SendMessage(gParams->ctl[n].hCtl, TBM_SETRANGEMAX, TRUE /*redraw*/,
                    gParams->ctl[n].maxval);
        break;

    case CC_PUSHBUTTON:
    case CC_RADIOBUTTON:
    case CC_GROUPBOX:
        if (gParams->ctl[n].minval < 0) gParams->ctl[n].minval = 0;
        if (gParams->ctl[n].maxval > 1) gParams->ctl[n].maxval = 1;
        break;

    case CC_CHECKBOX:
        if (gParams->ctl[n].minval < 0) gParams->ctl[n].minval = 0;
        if (gParams->ctl[n].maxval > 2) gParams->ctl[n].maxval = 2;
        break;

    case CC_LISTBOX:
    case CC_LISTBAR:
    case CC_COMBOBOX:
        if (gParams->ctl[n].minval < -1) gParams->ctl[n].minval = -1;
        if (gParams->ctl[n].maxval > 0xFFFF) gParams->ctl[n].maxval = 0xFFFF;
        break;


    case CC_STATICTEXT:
    case CC_FRAME:
    case CC_RECT:
    case CC_BITMAP:
    case CC_ICON:
    case CC_METAFILE:
    case CC_OWNERDRAW:
        //allow any arbitrary range for these controls
    default:
        break;
    } //switch class

    //force value to lie within new range, and update position
    //and numeric readout if necessary (must be done AFTER
    //setting the scrollbar range)
    setCtlVal(n, gParams->ctl[n].val);
    return retVal;
} /*fm_setCtlRange*/

/**********************************************************************/
/* setCtlLineSize(n, a)
/*
/*  Sets the small step ("line size") value for control n to a, and
/*  returns the previous small step value.
/*
/**********************************************************************/
int setCtlLineSize(int n, int a)
{
    int prevValue;
    if (n < 0 || n >= N_CTLS || !gParams->ctl[n].inuse) {
        return -1;
    }
    prevValue = gParams->ctl[n].lineSize;
    gParams->ctl[n].lineSize = a;
    switch (gParams->ctl[n].ctlclass) {
    case CC_STANDARD:
	case CC_SLIDER:
    case CC_SCROLLBAR:
        if (gParams->ctl[n].minval > gParams->ctl[n].maxval) {
            //range is inverted...
            //make sure lineSize is negative...
            if (gParams->ctl[n].lineSize > 0)
                gParams->ctl[n].lineSize = -gParams->ctl[n].lineSize;
        }
        else {
            //normal range (min <= max)
            //make sure lineSize is positive...
            if (gParams->ctl[n].lineSize < 0)
                gParams->ctl[n].lineSize = -gParams->ctl[n].lineSize;
        }
		if (gParams->ctl[n].ctlclass == CC_SLIDER) 
			SendMessage(gParams->ctl[n].hCtl, TBM_SETLINESIZE, 0 /*unused*/, a);
		break; 
    case CC_TRACKBAR:
        SendMessage(gParams->ctl[n].hCtl, TBM_SETLINESIZE, 0 /*unused*/, a);
        break;
    default:
        break;
    } //switch class
    return prevValue;
} /*setCtlLineSize*/

/**********************************************************************/
/* setCtlPageSize(n, b)
/*
/*  Sets the large step ("page size") value for control n to b, and
/*  returns the previous large step value.
/*
/**********************************************************************/
int setCtlPageSize(int n, int b)
{
    int prevValue;
    if (n < 0 || n >= sizeof(gParams->ctl)/sizeof(gParams->ctl[0]) ||
        !gParams->ctl[n].inuse) {
        return -1;
    }
    prevValue = gParams->ctl[n].pageSize;
    gParams->ctl[n].pageSize = b;
    switch (gParams->ctl[n].ctlclass) {
    case CC_STANDARD:
	case CC_SLIDER:
    case CC_SCROLLBAR:
        if (gParams->ctl[n].minval > gParams->ctl[n].maxval) {
            //range is inverted...
            //make sure pageSize is negative...
            if (gParams->ctl[n].pageSize > 0)
                gParams->ctl[n].pageSize = -gParams->ctl[n].pageSize;
        }
        else {
            //normal range (min <= max)
            //make sure pageSize is positive...
            if (gParams->ctl[n].pageSize < 0)
                gParams->ctl[n].pageSize = -gParams->ctl[n].pageSize;
        }
		if (gParams->ctl[n].ctlclass == CC_SLIDER) 
			SendMessage(gParams->ctl[n].hCtl, TBM_SETPAGESIZE, 0 /*unused*/, b);
        break;
    case CC_TRACKBAR:
        SendMessage(gParams->ctl[n].hCtl, TBM_SETPAGESIZE, 0 /*unused*/, b);
        break;
    default:
        break;
    } //switch class
    return prevValue;
} /*fm_setCtlPageSize*/


int setCtlThumbSize(int n, int a)
{
    
    if (a>0 && (gParams->ctl[n].ctlclass == CC_SCROLLBAR || gParams->ctl[n].ctlclass == CC_STANDARD)){
    	
		SCROLLINFO si;
    	LPSCROLLINFO lpsi;

		if (a > gParams->ctl[n].maxval - gParams->ctl[n].minval - 2) 
			a = gParams->ctl[n].maxval - gParams->ctl[n].minval - 2; //avoid crash
    	
    	si.cbSize = sizeof(si);
    	si.fMask = SIF_PAGE; //| SIF_RANGE;
    	si.nPage = a;
    	//si.nMin = gParams->ctl[n].minval;
    	//si.nMax = gParams->ctl[n].maxval;

		lpsi = &si;
    	
    	SendMessage(gParams->ctl[n].hCtl, SBM_SETSCROLLINFO, TRUE, (LPARAM) lpsi);

    	gParams->ctl[n].thumbSize = a;
		//setCtlPageSize (n,a);

    	return true;
    
	} else if (a>0 && (gParams->ctl[n].ctlclass == CC_TRACKBAR || gParams->ctl[n].ctlclass == CC_SLIDER)){

        setCtlStyle(n,TBS_FIXEDLENGTH);
        SendMessage (gParams->ctl[n].hCtl, TBM_SETTHUMBLENGTH, (WPARAM) VDBUsToPixels(a), 0);
        // thumbSize set to 0 for proper bitmap display, previously had taken the value from a.
        // the a variable must be exactly the width of the thumb image for proper display.
        gParams->ctl[n].thumbSize = 0;

        return true;
    }
   
   return false;
   
} /*fm_setCtlThumbSize*/


/*************************************************************/
//
//  A couple of local helper functions...
//
/*************************************************************/

// Modified by Ognen Genchev. Added the xx and yy parameters.
void computeBuddy1Pos(int n, int x, int y, int w, int h, int xx, int yy, RECT *pr)
{
    // Compute position of the numedit buddy for a
    // STANDARD control.
    
    //Changed by Harald Heim, May 1, 2003
    pr->right = gParams->ctl[n].editwidth; //24; //20;               //width
    pr->bottom = gParams->ctl[n].editheight; //10;              //height

    if (gParams->ctl[n].style & SBS_VERT) { //gParams->ctl[n].ctlclass!=CC_LISTBAR && gParams->ctl[n].ctlclass!=CC_COMBOBOX && 
        //vertical scrollbar
        pr->left = x + (w - pr->right)/2;
//#ifdef LABEL_ON_TOP
        pr->top = y + h + 2;
//#else
       pr->top = y - pr->bottom - 2;
//#endif
    }
    else {
        //horizontal scrollbar
        pr->left = x + gParams->ctl[n].editXpos + xx;
        pr->top = y + gParams->ctl[n].editYpos + yy;

        // Commented out by Ognen Genchev
        // pr->left = x + w + 5;
        // pr->top = y + (h - pr->bottom)/2;
        // end
    }
    //map DBUs to pixels...
    MapDialogRect(fmc.hDlg, pr);
} /*computeBuddy1Pos*/


void computeBuddy2Pos(int n, int x, int y, int w, int h, RECT *pr)
{
    // Compute position of the static text buddy for a
    // STANDARD control.
#if 0
    pr->right = gParams->ctl[n].sb.w2;               //width (pixels)
    pr->bottom = gParams->ctl[n].sb.h2;              //height (pixels)
    if (gParams->ctl[n].ctlclass!= CC_LISTBAR && gParams->ctl[n].style & SBS_VERT) {
        //vertical scrollbar
        pr->left = x + (w - pr->right)/2;
//#ifdef LABEL_ON_TOP
        pr->top = y - 10;
//#else
//        pr->top = y + h + 2;
//#endif
    }
    else {
        //horizontal scrollbar
        pr->left = x - pr->right - 5;
#if 1
        pr->top = y + (h - pr->bottom)/2 + 1;
#else
        pr->top = y + (h - pr->bottom)/2;
#endif
    }
    //map DBUs to pixels...
    MapDialogRect(fmc.hDlg, pr);

#else

    pr->right = w;
    pr->bottom = h;
    if (gParams->ctl[n].ctlclass!=CC_LISTBAR && gParams->ctl[n].ctlclass!=CC_COMBOBOX && gParams->ctl[n].style & SBS_VERT) {
        //vertical scrollbar
        pr->left = x;
//#ifdef LABEL_ON_TOP
        pr->top = y - 10;
//#else
//        pr->top = y + h + 2;
//#endif
        //map DBUs to pixels...
        MapDialogRect(fmc.hDlg, pr);
        pr->left += (pr->right - gParams->ctl[n].sb.w2)/2;
    }
    else {

		//if (n==N_FXCTLS+68) Info ("%d, %d",pr->bottom,gParams->ctl[n].sb.h2);
		
		//horizontal scrollbar
        pr->left = x - 5;
        pr->top = y;
        //map DBUs to pixels...
        MapDialogRect(fmc.hDlg, pr);
        pr->left -= gParams->ctl[n].sb.w2;
		
		if (gParams->ctl[n].ctlclass==CC_COMBOBOX)
			pr->top += 4;
#if STYLER
		else if (gParams->ctl[n].ctlclass==CC_LISTBAR)
			pr->top += (pr->bottom - 20)/2;
#endif
		else{
			pr->top += (pr->bottom - gParams->ctl[n].sb.h2)/2;
		}

    }
    pr->right = gParams->ctl[n].sb.w2;               //width (pixels)
    pr->bottom = gParams->ctl[n].sb.h2;              //height (pixels)

#endif
    //Info("ctl%d Buddy2 pos = %d, %d, %d, %d", n, pr->left, pr->top, pr->right, pr->bottom);
} /*computeBuddy2Pos*/



int pixelUnit=0;

/**********************************************************************/
/* setCtlPos(n, x, y, w, h)
/*
/*  Sets the position of the upper lefthand corner of control n to 
/*  coordinates (x,y) within the client area of the dialog box; sets 
/*  the width of the control to w and the height to h.  All measurements 
/*  are in dialog box units (DBUs).  To leave a particular parameter