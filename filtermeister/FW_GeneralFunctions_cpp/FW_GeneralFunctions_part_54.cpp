    //Well, even this doesn't work for trackbars, and it's
    //probably vast overkill for other controls...
    GetWindowRect(hCtl, &wRect); 
    ScreenToClient (fmc.hDlg, (LPPOINT)&wRect);
    ScreenToClient (fmc.hDlg, (LPPOINT)&(wRect.right));
    return RedrawWindow(
#if 0
            //just updating the control window doesn't work :(
            hCtl,   // handle of control window
#else
            fmc.hDlg,   // handle of entire dialog window
#endif
#if 1
            &wRect, // address of structure with update rectangle
#else
            NULL,   // address of structure with update rectangle, NULL=>entire client area
#endif
            NULL,   // handle of update region
            RDW_ERASE|RDW_INVALIDATE|RDW_ERASENOW|RDW_UPDATENOW     // array of redraw flags
       );

#endif
    // REMEMBER TO DELETE THE BRUSH WHEN EXITING????
} /*fm_setCtlColor*/


int setCtlSysColor(int n, int con){

	return setCtlColor (n, (int)GetSysColor(con));

}

int setCtlFontSysColor(int n, int con){

	return setCtlFontColor (n, (int)GetSysColor(con));

}


/**********************************************************************/
/* getCtlColor(n)
/*
/*  Returns the current background color of control n as an RGB-triple,
/*  or -1 if n is not a valid control.
/*
/**********************************************************************/
int getCtlColor(int n)
{
    int color = gParams->ctl[n].bkColor;

    if (n < 0 || n >= N_CTLS || !gParams->ctl[n].inuse) {
        return -1;
    }

	if (gParams->ctl[n].ctlclass == CC_TAB){

#if SKIN
		if (isSkinActive() && gParams->skinStyle>0){
			
			color = skinGetColor();

		} else 
#endif
		{

			color = (int)GetSysColor(COLOR_BTNFACE);

			if ( getAppTheme() && (gParams->ctl[n].theme==1  || (gParams->ctl[n].theme==-1 && gParams->gDialogTheme==1)) ){
				
				if (getWindowsVersion() <= 10 ){ //XP & Win 2003
	            
					typedef HANDLE (STDAPICALLTYPE *LPOpenThemeData) (HWND, LPCWSTR);
					typedef HRESULT (STDAPICALLTYPE *LPGetThemeColor) (HTHEME,INT,INT,INT,COLORREF*);
					typedef HRESULT (STDAPICALLTYPE *LPCloseThemeData) (HTHEME);
					static LPOpenThemeData lpOpenThemeData = NULL;
					static LPGetThemeColor lpGetThemeColor = NULL;
					static LPCloseThemeData lpCloseThemeData = NULL;
					HANDLE vsTheme;

					HMODULE hMod = LoadLibrary("uxtheme.dll");
					if (hMod){
						lpOpenThemeData = (LPOpenThemeData) GetProcAddress(hMod, "OpenThemeData");
						lpGetThemeColor = (LPGetThemeColor) GetProcAddress(hMod,"GetThemeColor");
						lpCloseThemeData = (LPCloseThemeData) GetProcAddress(hMod,"CloseThemeData");

						vsTheme = lpOpenThemeData(gParams->ctl[n].hCtl,L"TAB");
						if (vsTheme){
							COLORREF pColor = 0;
							lpGetThemeColor(vsTheme, 9, 1, 3821, &pColor); //3802 //3821
							//lpGetThemeColor(vsTheme, 9, 1, 3821, pColor); //3802 //3821
							lpCloseThemeData(vsTheme);
							color = (int)pColor;
						}

						FreeLibrary(hMod);
					}

				} else { //Vista and 7

					color = 0xffffff; //pure white tab sheet background
				}
			}
		} 
	
	}

	return color;

} /*fm_getCtlColor*/


int setCtlDefVal(int n, int defval)
{
    if (n < 0 || n >= N_CTLS || !gParams->ctl[n].inuse) {
        return false;
    }

    gParams->ctl[n].defval = defval;

    // Added by Ognen Genchev
    gParams->ctl[n].val = defval;
    gParams->ctl[n].cancelVal = defval;
    // end
    
    return true;
}

int getCtlDefVal(int n)
{
    if (n < 0 || n >= N_CTLS || !gParams->ctl[n].inuse) {
        return false;
    }

    return gParams->ctl[n].defval;
}

/**********************************************************************/
/* setCtlVal(n, val)
/*
/*  Sets the value of control n to val.
/*
/*  Returns the previous value of control n, or -1 if n is not a
/*  valid control.
/*
/**********************************************************************/
int setCtlVal(int n, int val)
{
    int originalValue = val;    //Is this really needed???
    int prevValue;

    int val2;


    //Info("setCtlVal(%d, %d)", n, val);
    if (n < 0 || n >= N_CTLS || !gParams->ctl[n].inuse) {
        return -1;
    }

    prevValue = gParams->ctl[n].val;


    //Only manipulate the controls if dialog was displayed, otherwise we get problems
    if (fmc.displayDialog){ 

#if SKIN
		if (gParams->ctl[n].ctlclass != CC_STANDARD && gParams->ctl[n].ctlclass != CC_GROUPBOX){
			if (isSkinActive() && isSkinCtl(n)){
				gParams->ctl[n].val = val;
				fmc.pre_ctl[n] = val;
				if (ctlEnabledAs(n)) skinDraw(n,0);
				return prevValue;
			}
		}
#endif
        /**********************************************************************************************/
        /*  Fix by Ognen Genchev
        /*
        /*  Condition moved out of the switch statement to prevent resetting trackbar
        /*  value to 0 after reinitialization of the plugin.
        /**********************************************************************************************/
        if(gParams->ctl[n].ctlclass == CC_TRACKBAR){
            if (prevValue != originalValue) {
                //update the actual control if it changed
                SendMessage(gParams->ctl[n].hCtl, TBM_SETPOS, TRUE /*set*/, val);
            }
        }
        // end
        switch (gParams->ctl[n].ctlclass) {
        case CC_STANDARD:
	    case CC_SLIDER:
        case CC_SCROLLBAR:
        //  Fix by Ognen Genchev
        //  Added case CC_TRACKBAR to fix the problem with infinitely writing values
        //  below min and above max when using arrow keys to change trackbar value.
        case CC_TRACKBAR:
        // end
            if (gParams->ctl[n].minval > gParams->ctl[n].maxval) {
                //range is inverted...              //NFG for scrollbar -- thumb stays at left
                                                    // but we could fake it by using -val internally...
                if (val > gParams->ctl[n].minval) val = gParams->ctl[n].minval;
                else if (val < gParams->ctl[n].maxval) val = gParams->ctl[n].maxval;
                //make sure lineSize and pageSize are negative...       //// why not do this (only) in setCtlRange()
                                                                        //// and setCtlLineSize()/setCtlPageSize()
                                                                        //// i.e., only when linesize, pagesize, 
                                                                        //// minval, or maxval is modified ?
                if (gParams->ctl[n].lineSize > 0)
                    gParams->ctl[n].lineSize = -gParams->ctl[n].lineSize;
                if (gParams->ctl[n].pageSize > 0)
                    gParams->ctl[n].pageSize = -gParams->ctl[n].pageSize;
            }
            else {
                //normal range (min <= max)
                if (val < gParams->ctl[n].minval) val = gParams->ctl[n].minval;
                else if (val > gParams->ctl[n].maxval) val = gParams->ctl[n].maxval;
                //make sure lineSize and pageSize are positive...       ///// DITTO
                if (gParams->ctl[n].lineSize < 0)
                    gParams->ctl[n].lineSize = -gParams->ctl[n].lineSize;
                if (gParams->ctl[n].pageSize < 0)
                    gParams->ctl[n].pageSize = -gParams->ctl[n].pageSize;
            }
            //if (prevValue != originalValue || prevValue != val) {
                //update the actual control if it changed

			    if ( gParams->ctl[n].thumbSize > 0){
				    int range = abs(gParams->ctl[n].maxval - gParams->ctl[n].minval);
				    val2 = val * (range - gParams->ctl[n].thumbSize-1) / range;
                } else {
				    val2 = val;
			    }
            	
#if SKIN
				if (isSkinActive() && isSkinCtl(n)){
					
					gParams->ctl[n].val = val;
				    fmc.pre_ctl[n] = val;
					if (ctlEnabledAs(n)) skinDraw(n,0); //val2

				} else 
#endif				
				{

					if (gParams->ctl[n].gamma != 100){
						val2 = gammaCtlVal(n,val2,false);
					}

					if (gParams->ctl[n].minval > gParams->ctl[n].maxval) {
						//range is inverted...
						if (gParams->ctl[n].ctlclass == CC_SLIDER)
							SendMessage(gParams->ctl[n].hCtl, TBM_SETPOS, TRUE, gParams->ctl[n].minval + gParams->ctl[n].maxval - val2);
						else
							SetScrollPos(gParams->ctl[n].hCtl, SB_CTL,
									 gParams->ctl[n].minval + gParams->ctl[n].maxval - val2,
									 TRUE /*redraw*/);	
					}
					else {
						//normal range (min <= max)
						if (gParams->ctl[n].ctlclass == CC_SLIDER)
							SendMessage(gParams->ctl[n].hCtl, TBM_SETPOS, TRUE, val2);
						else
							SetScrollPos(gParams->ctl[n].hCtl, SB_CTL,
									 val2,
									 TRUE /*redraw*/);
					}

				}


                if ( (gParams->ctl[n].ctlclass == CC_STANDARD || gParams->ctl[n].ctlclass == CC_SLIDER) && (EditBoxUpdate==0 || GetFocus() != GetDlgItem(fmc.hDlg, IDC_BUDDY1+n)) ) {
                    //update the numeric readout field if it changed
                
    #if 1
                    if (gParams->ctl[n].divisor == 1) { // Set Integer Value

                        SetDlgItemInt(fmc.hDlg, IDC_BUDDY1+n,
                                      val,
                                      TRUE /*signed*/);

                    } else { // Set Double Value
                        char szBuffer[32];
                    
                        if (gParams->ctl[n].divisor<=10)
                            sprintf(szBuffer, "%.1f", (double) val / gParams->ctl[n].divisor );
                        else if (gParams->ctl[n].divisor<=100)
                            sprintf(szBuffer, "%.2f", (double) val / gParams->ctl[n].divisor );
                        else ///// if (gParams->ctl[n].divisor<=1000)
                            sprintf(szBuffer, "%.3f", (double) val / gParams->ctl[n].divisor );
                    
                        SetDlgItemText (fmc.hDlg, IDC_BUDDY1+n,(LPCTSTR) &szBuffer ) ;
                    }
    #else
                    char szBuffer[32];

                    sprintf(szBuffer, "%.3g", (double) val / gParams->ctl[n].divisor );
                    SetDlgItemText (fmc.hDlg, IDC_BUDDY1+n, (LPCTSTR) &szBuffer ) ;
    #endif


    #if 1
                    //if the text buddy is an Edit control, we need to
                    //call UpdateWindow() if we want an immediate update