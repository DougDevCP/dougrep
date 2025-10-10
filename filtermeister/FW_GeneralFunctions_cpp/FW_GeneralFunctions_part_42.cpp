{

    MENUITEMINFO mii;

    if (uItem<1){ //Not smaller than 1
        //uItem=1;
		return false;
	}

    mii.cbSize = sizeof(MENUITEMINFO);

    
    mii.fMask = MIIM_STATE | MIIM_ID | MIIM_TYPE;
    if (subMenu != 0){ 
            mii.fMask = mii.fMask | MIIM_SUBMENU; 
            mii.hSubMenu = (HMENU) subMenu;
    }

    if (strcmp ((char *)itemName, "---") == 0)
        mii.fType = MFT_SEPARATOR;
    else 
        mii.fType = MFT_STRING;

    if (bullet){
        mii.hbmpChecked = NULL;
        mii.fType |= MFT_RADIOCHECK;
    }

    mii.fState = fState;
    mii.wID = uItem;
    mii.dwTypeData = (LPTSTR)itemName;
    mii.cch = (UINT)strlen ((char *)itemName);


    InsertMenuItem ( (HMENU)hMenu, uItem-1, MF_BYPOSITION, &mii);


    return true;
}


int trackPopupMenu (INT_PTR hMenu, int type, int x, int y, int style)
{
    RECT r;
    POINT pt;


	if (type==1){ //Display at cursor coordinates
        
        GetCursorPos(&pt);
        x = pt.x;
        y = pt.y;
        
    } else { //Use provided coordinates

		RECT rcClient, rcWind; 
    
		//Vista & 7 etc.
		if (getWindowsVersion()>=11) {
			x += 3;
			y += 2;
		}

		//Convert DBUs to Pixel
        x = HDBUsToPixels(x);
        y = VDBUsToPixels(y);

		//Try to calculate the title bar height
		if (GetWindowLong(fmc.hDlg, GWL_STYLE) & WS_CAPTION){
			GetClientRect(fmc.hDlg, &rcClient); 
			GetWindowRect(fmc.hDlg, &rcWind); 
			y += ((rcWind.bottom - rcWind.top) - (rcClient.bottom - rcClient.top)) - ((rcWind.right - rcWind.left) - (rcClient.right - rcClient.left)); 
			y += 1;
		}
    
        //Map to Screen coordinates
        GetWindowRect(fmc.hDlg, &r);
        x += r.left;
        y += r.top;
    }

    if (style==0) 
        style = TPM_TOPALIGN | TPM_LEFTALIGN | TPM_NONOTIFY | TPM_RETURNCMD | TPM_LEFTBUTTON;
    else
        style |= TPM_RETURNCMD;

    return TrackPopupMenuEx((HMENU)hMenu, (UINT)style, x, y, fmc.hDlg, NULL);

}


int setMenu (INT_PTR hMenu){


    return SetMenu (fmc.hDlg, (HMENU)hMenu);

}



int getCtlFocus (void)
{
    HWND focWindow = GetFocus();
    int focCtl = -1, n;

    if (focWindow == NULL) return -1;

    for (n = 0; n < N_CTLS; n++) {
        if (gParams->ctl[n].hCtl == focWindow){
            focCtl = n;
            break;
        }
    }

	if (n==N_CTLS) return -1;

	return focCtl;
}


int setCtlFocus (int n)
{
    HWND prevWindow;

    if ( (n < 0 || n >= N_CTLS || !gParams->ctl[n].inuse) && n!= CTL_PREVIEW) {
        return FALSE;
    }

    prevWindow = SetFocus (gParams->ctl[n].hCtl);

    if (prevWindow == NULL) return false;

	return true;
}

int checkCtlFocus (int n)
{
    
	if ( (n < 0 || n >= N_CTLS || !gParams->ctl[n].inuse) && n!= CTL_PREVIEW) {
        return FALSE;
    }

    if (GetFocus() == gParams->ctl[n].hCtl)
        return true;
    else
        return false;

}

int checkDialogFocus (void)
{
    
	if (GetForegroundWindow() == MyHDlg)
        return true;
    else
        return false;

}


    //// WARNING!! findFirstFile() may modify the lpFileName string!!! /////

INT_PTR findFirstFile (LPCTSTR lpFileName, LPSTR foundItem, int *dwFileAttributes)
{
    HANDLE hFindFile;
    WIN32_FIND_DATA FindFileData = {0};
    
    //Check if no file pattern and add one
    if (strcmp(lpFileName+strlen(lpFileName)-1,"\\")==0)
        strncat ((char *)lpFileName, "*", 1);//strncat ((const char *)lpFileName, "*", 1);

    hFindFile = FindFirstFile(lpFileName, &FindFileData);

    if (hFindFile != INVALID_HANDLE_VALUE){
        strcpy (foundItem, FindFileData.cFileName);//Pass back item
        *dwFileAttributes = (int) FindFileData.dwFileAttributes;
    }

    return (INT_PTR)hFindFile;

}


int findNextFile (INT_PTR hFindFile, LPSTR foundItem, int *dwFileAttributes)
{
    BOOL val;
    WIN32_FIND_DATA FindFileData = {0};
 
    val = FindNextFile ((HANDLE) hFindFile, &FindFileData);

    if (val!=0){
        strcpy (foundItem, FindFileData.cFileName);//Pass back item
        *dwFileAttributes = (int) FindFileData.dwFileAttributes;
    }

    return (int) val;

}

BOOL findClose(INT_PTR hFindFile)
{
    // cdecl -> strdcall thunk
    return FindClose((HANDLE)hFindFile);
}


int getLocaleInfo(LCID Locale,
                            LCTYPE LCType,
                            LPSTR  lpLCData,
                            int    cchData)
{
    // cdecl -> stdcall thunk
    return GetLocaleInfo(Locale, LCType, lpLCData, cchData);
}



int getSpecialFolder (int val, char * str)
{

	LPITEMIDLIST ppidl; //LPITEMIDLIST 

	if (SHGetSpecialFolderLocation (fmc.hDlg, val, &ppidl) == NOERROR) {
			if (SHGetPathFromIDList(ppidl, (char *)str))
				return true;
			else
				return false;
	} else {

		return false;
	}

}; 


int setCtlStyle(int n, int flags)
{
    
    if ( (n < 0 || n >= N_CTLS || !gParams->ctl[n].inuse) ) {
        return FALSE;
    }
    
    gParams->ctl[n].style |= flags;

	SetWindowLong(gParams->ctl[n].hCtl, GWL_STYLE,
        GetWindowLong(gParams->ctl[n].hCtl, GWL_STYLE) | flags);
    //use SWP_FRAMECHANGED to force nonclient area update...
    return SetWindowPos(gParams->ctl[n].hCtl, NULL,
                        0, 0, 0, 0,
                        SWP_FRAMECHANGED|SWP_NOZORDER|SWP_NOACTIVATE|SWP_NOSIZE|SWP_NOMOVE);
} /*fm_setCtlStyle*/

int clearCtlStyle(int n, int flags)
{

    if ( (n < 0 || n >= N_CTLS || !gParams->ctl[n].inuse) ) {
        return FALSE;
    }

    gParams->ctl[n].style &= ~flags;
	
	SetWindowLong(gParams->ctl[n].hCtl, GWL_STYLE,
        GetWindowLong(gParams->ctl[n].hCtl, GWL_STYLE) & ~flags);
    //use SWP_FRAMECHANGED to force nonclient area update...
    return SetWindowPos(gParams->ctl[n].hCtl, NULL,
                        0, 0, 0, 0,
                        SWP_FRAMECHANGED|SWP_NOZORDER|SWP_NOACTIVATE|SWP_NOSIZE|SWP_NOMOVE);
} /*fm_clearCtlStyle*/

int setCtlStyleEx(int n, int flags)
{
    
    HWND hCtl;

    if ( (n < 0 || n >= N_CTLS || !gParams->ctl[n].inuse) ) { //&& n!= CTL_PREVIEW
        return FALSE;
    }
    
    gParams->ctl[n].styleEx |= flags;

    //if (n == CTL_PREVIEW)
    //    hCtl = GetDlgItem(fmc.hDlg, 101);
    //else
        hCtl = gParams->ctl[n].hCtl;
	
	SetWindowLong(hCtl, GWL_EXSTYLE,
        GetWindowLong(hCtl, GWL_EXSTYLE) | flags);

	if (gParams->ctl[n].ctlclass == CC_LISTBAR){
		setCtlPos(n, -1, -1, -1, gParams->ctl[n].height); //Avoid collapsing
	}

    //use SWP_FRAMECHANGED to force nonclient area update...
    return SetWindowPos(hCtl, NULL,
                        0, 0, 0, 0,
                        SWP_FRAMECHANGED|SWP_NOZORDER|SWP_NOACTIVATE|SWP_NOSIZE|SWP_NOMOVE);
} /*fm_setCtlStyleEx*/

int clearCtlStyleEx(int n, int flags)
{