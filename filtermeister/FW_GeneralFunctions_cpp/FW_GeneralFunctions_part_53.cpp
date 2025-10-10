			pszSubAppName = NULL; //enable theme
			pszSubIdList = NULL;
		}
		gParams->ctl[n].theme = -1;
	} else if (state==0){
		pszSubAppName = L""; //disable theme
		pszSubIdList = L"";
		gParams->ctl[n].theme = 0;
	} else { //Explorer theme for listview
		pszSubAppName = L"explorer";
		//pszSubAppName = NULL; //enable theme
		pszSubIdList = NULL;
		gParams->ctl[n].theme = 1;
	}


	if (n==CTL_ZOOM){
		setWindowTheme(GetDlgItem(fmc.hDlg, IDC_BUTTON1), pszSubAppName, pszSubIdList);
		setWindowTheme(GetDlgItem(fmc.hDlg, IDC_BUTTON2), pszSubAppName, pszSubIdList);
	}
	if (gParams->ctl[n].ctlclass == CC_STANDARD || gParams->ctl[n].ctlclass == CC_SLIDER) {
		setWindowTheme(GetDlgItem(fmc.hDlg, IDC_BUDDY1+n), pszSubAppName, pszSubIdList);
		setWindowTheme(GetDlgItem(fmc.hDlg, IDC_BUDDY2+n), pszSubAppName, pszSubIdList);
	}
	
	RetVal = setWindowTheme(GetDlgItem(fmc.hDlg, nIDDlgItem), pszSubAppName, pszSubIdList); 


	//Avoid black check box text under XP
	if (gParams->ctl[n].ctlclass == CC_CHECKBOX) {
		if ( getAppTheme() && (state==1  || (state==-1 && gParams->gDialogTheme==1)) && getCtlColor(n)==-1 )
			//setCtlColor(n,GetSysColor(COLOR_BTNFACE));
            setCtlColor(n,gParams->gDialogGradientColor1);
		else if ( (state==0  || (state==-1 && gParams->gDialogTheme==0)) && getCtlColor(n)==(int)GetSysColor(COLOR_BTNFACE) ) //Switch back to transparent
			setCtlColor(n,-1);
    }

	return RetVal;
	
}*/


#if 0

int setDialogEvent (int state){

	/*
	0 = None
	1 = Init event
	2 = Cancel event
	4 = Keydown event
	*/

	if (state<0) state=0; else if (state>7) state=7;

	if (state & 4) {
		if (KeyHook != NULL) UnhookWindowsHookEx(KeyHook); //Avoid Multiple Hooks

        /*
            #if _WIN64
                KeyHook = SetWindowsHookEx(WH_KEYBOARD_LL,LLKeyHookProc, (HINSTANCE) GetWindowLongPtr(fmc.hDlg, GWLP_HINSTANCE), GetCurrentThreadId());//(DWORD)NULL); //WH_KEYBOARD
            #else
                KeyHook = SetWindowsHookEx(WH_KEYBOARD_LL,LLKeyHookProc, (HINSTANCE) GetWindowLong(fmc.hDlg, GWL_HINSTANCE), GetCurrentThreadId());//(DWORD)NULL); //WH_KEYBOARD
            #endif
        */
        
		#if _WIN64 // || _MSC_VER >= 1400
            KeyHook = SetWindowsHookEx(WH_KEYBOARD,KeyHookProc, (HINSTANCE) GetWindowLongPtr(fmc.hDlg, GWLP_HINSTANCE), GetCurrentThreadId());//(DWORD)NULL); //WH_KEYBOARD
        #else
            KeyHook = SetWindowsHookEx(WH_KEYBOARD,KeyHookProc, (HINSTANCE) GetWindowLong(fmc.hDlg, GWL_HINSTANCE), GetCurrentThreadId());//(DWORD)NULL); //WH_KEYBOARD
        #endif
	}

    gParams->gDialogEvent |= state;
	
	return true;
}

int clearDialogEvent (int state){

	/*
	0 = None
	1 = Init event
	2 = Cancel event
	4 = Keydown event
	*/

	if (state<0) state=0; else if (state>7) state=7;

	if (state & 4) {
		if (KeyHook != NULL) UnhookWindowsHookEx(KeyHook);
	}

	gParams->gDialogEvent &= ~state;
	
	return true;
}

#endif



int createFont(int i, int size, int bold, int italic, char * fontname)
{
    HFONT hfont;
    
    if (i < 0 || i >= N_FONTS) {
        return false;
    }

    if (Font[i] != 0) DeleteObject(Font[i]);

    if (fontname==NULL || fontname[0]==0 || size==0) {
        /*NONCLIENTMETRICS ncMetrics;
        ncMetrics.cbSize = sizeof(NONCLIENTMETRICS);
        if (!SystemParametersInfo (SPI_GETNONCLIENTMETRICS,sizeof(NONCLIENTMETRICS),&ncMetrics,0)) return false;
        if (fontname==NULL) fontname = ncMetrics.lfMessageFont.lfFaceName;
        Info ("%s",fontname);
        if (size==0) {
            size = ncMetrics.lfMessageFont.lfHeight;
        }*/

        LOGFONT lgFont; 
        GetObject(GetStockObject(DEFAULT_GUI_FONT), sizeof(LOGFONT), &lgFont);
        if (fontname==NULL || fontname[0]==0) fontname = lgFont.lfFaceName;
        if (size==0) size = lgFont.lfHeight;
		//Info ("%s",fontname);
    }


	hfont = CreateFont(size, 0, 0, 0, (bold? 700 : 400), italic,
						false, false,
						DEFAULT_CHARSET/*ANSI_CHARSET*/, OUT_DEFAULT_PRECIS,
						CLIP_DEFAULT_PRECIS, ANTIALIASED_QUALITY, DEFAULT_PITCH,
						(LPCTSTR)fontname);

    if (hfont){
        Font[i] = hfont;
        return true;
    }

    return false;
}

int deleteFont (int i){

     if (i < 0 || i >= N_FONTS) {
        return false;
    }

     if (Font[i] != 0) {
        DeleteObject(Font[i]);
        Font[i] = 0;
        return true;
    }

    return false;
}


int setCtlFont(int n, int i)
{

    if (n < 0 || n >= N_CTLS || !gParams->ctl[n].inuse) {
        return false;
    }

	gParams->ctl[n].hFont = 0;

    if (i==-1){
		HFONT hfont = (HFONT)GetStockObject(DEFAULT_GUI_FONT);
		gParams->ctl[n].hFont = hfont;
	} else if (i >= 0 && i < N_FONTS && Font[i]!=0) {
		gParams->ctl[n].hFont = Font[i];
	}

	if (gParams->ctl[n].hFont){
        if (gParams->ctl[n].ctlclass ==CC_STANDARD || gParams->ctl[n].ctlclass ==CC_SLIDER){
            //SendMessage(gParams->ctl[n].hBuddy1, WM_SETFONT, (WPARAM)gParams->ctl[n].hFont, TRUE);
            SendMessage(gParams->ctl[n].hBuddy2, WM_SETFONT, (WPARAM)gParams->ctl[n].hFont, FALSE);
        } else if (gParams->ctl[n].ctlclass ==CC_COMBOBOX){
			SendMessage(gParams->ctl[n].hBuddy2, WM_SETFONT, (WPARAM)gParams->ctl[n].hFont, FALSE);
		} else {
            SendMessage(gParams->ctl[n].hCtl, WM_SETFONT, (WPARAM)gParams->ctl[n].hFont, FALSE);

			if (gParams->ctl[n].ctlclass ==CC_LISTBAR){
				setCtlPos(n, -1, -1, -1, gParams->ctl[n].height); //Avoid collapsing
			} else if (gParams->ctl[n].ctlclass ==CC_LISTVIEW || gParams->ctl[n].ctlclass ==CC_LISTBOX){
				enableCtl(n,0);
				enableCtl(n,-1);
			} else {
				//Kludge to force a window update...
				setCtlText(n, (char *)gParams->ctl[n].label); 
			}
		}

		return true;
    }
	
    return false;
}


/**********************************************************************/
/* setCtlFontColor(n, color)
/*
/*  Sets the text color for control n to the specified RGB-triple value.
/*
/**********************************************************************/
int setCtlFontColor(int n, int color)
{
    if (n < 0 || n >= N_CTLS || !gParams->ctl[n].inuse) {
        return -1;
    }

	if (gParams->ctl[n].ctlclass == CC_LISTVIEW) {
		ListView_SetTextColor(gParams->ctl[n].hCtl,color);
		return TRUE;
	} else {
		gParams->ctl[n].textColor = color;
		//Kludge to force a window update...
		return setCtlText(n, (char *)gParams->ctl[n].label);
	}

} /*fm_setCtlFontColor*/


int getCtlFontColor(int n)
{
    if (n < 0 || n >= N_CTLS || !gParams->ctl[n].inuse) {
        return -1;
    }
    
    return gParams->ctl[n].textColor;
}

/**********************************************************************/
/* setCtlColor(n, color)
/*
/*  Sets the background color of control n to the specified RGB-triple.
/*  (This actually has no effect for many control styles; the effect is
/*  specific to each control style, and will be specified under the
/*  description for each style.)
/*
/*        Color = -1 means transparent.
/*
/**********************************************************************/
int setCtlColor(int n, int color)
{
	HWND hCtl = gParams->ctl[n].hCtl;

	if (n == CTL_PREVIEW){
		hCtl = fmc.hDlg;//GetDlgItem(fmc.hDlg, 101);
		if (color >= 0) gParams->ctl[n].bkColor = color;
		if (!ctlUpdateLocked){
			InvalidateRect(fmc.hDlg, NULL, TRUE /*erase background*/);
			return UpdateWindow(fmc.hDlg);
		} else 
			return true;
	}
	

	if (n < 0 || n >= N_CTLS || !gParams->ctl[n].inuse) {
        return -1;
    }

	
	gParams->ctl[n].bkColor = color;
	DeleteObject(gParams->ctl[n].hBrush);
	gParams->ctl[n].hBrush = (HBRUSH)((color == -1) ?
								GetStockObject(HOLLOW_BRUSH) : 
								CreateSolidBrush(color));


    // Force a window update...
#if 1
    if (gParams->ctl[n].ctlclass == CC_TRACKBAR || gParams->ctl[n].ctlclass == CC_SLIDER) {
        // Do a dummy SETRANGEMAX with the REDRAW flag set...
        SendMessage(hCtl, TBM_SETRANGEMAX, TRUE /*redraw*/,
                    gParams->ctl[n].maxval);
        return TRUE;

    } else if (gParams->ctl[n].ctlclass == CC_LISTVIEW) {
		ListView_SetBkColor(hCtl,color);
		ListView_SetTextBkColor(hCtl,color);
		return TRUE;

	} else {
        //following doesn't work for trackbars
        //but is OK for checkbox, radiobutton, statictext, groupbox, etc.
        InvalidateRect(hCtl, NULL, TRUE /*erase background*/);
        return UpdateWindow(hCtl);
    }
#elif 0
    return TRUE; //NFG for checkbox, radiobutton, statictext, etc.
#elif 0
    // This doesn't work for anything...
    return (int)SendMessage(hCtl, WM_ERASEBKGND, (WPARAM)GetDC(hCtl), 0);
#else
    //Bring out the big guns...