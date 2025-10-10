    
    if ( (n < 0 || n >= N_CTLS || !gParams->ctl[n].inuse) ) {
        return FALSE;
    }
    
    gParams->ctl[n].styleEx &= ~flags;

	SetWindowLong(gParams->ctl[n].hCtl, GWL_EXSTYLE,
        GetWindowLong(gParams->ctl[n].hCtl, GWL_EXSTYLE) & ~flags);
    //use SWP_FRAMECHANGED to force nonclient area update...
    return SetWindowPos(gParams->ctl[n].hCtl, NULL,
                        0, 0, 0, 0,
                        SWP_FRAMECHANGED|SWP_NOZORDER|SWP_NOACTIVATE|SWP_NOSIZE|SWP_NOMOVE);
} /*fm_clearCtlStyleEx*/



int setCtlBuddyStyle(int n, int buddy, int flags)
{
    HWND hBuddy;

    if ( (n < 0 || n >= N_CTLS || !gParams->ctl[n].inuse) ) {
        return FALSE;
    }

	if (buddy==1){
		hBuddy = gParams->ctl[n].hBuddy1;
		if (hBuddy==NULL) return false;
        // FIX by Ognen Genchev
        // the OR condition is removed so it can directly assign flags to the structure member
        gParams->ctl[n].buddy1Style = flags;
        // end
    } else {
        hBuddy = gParams->ctl[n].hBuddy2;
        if (hBuddy==NULL) return false;
        // FIX by Ognen Genchev
        // the OR condition is removed so it can directly assign flags to the structure member
        gParams->ctl[n].buddy2Style = flags;
        // end
    }
	

	SetWindowLong(hBuddy, GWL_STYLE,
        GetWindowLong(hBuddy, GWL_STYLE) | flags);
    //use SWP_FRAMECHANGED to force nonclient area update...
    return SetWindowPos(hBuddy, NULL,
                        0, 0, 0, 0,
                        SWP_FRAMECHANGED|SWP_NOZORDER|SWP_NOACTIVATE|SWP_NOSIZE|SWP_NOMOVE);
} /*fm_setCtlBuddyStyle*/

int clearCtlBuddyStyle(int n, int buddy, int flags)
{
    HWND hBuddy;

    if ( (n < 0 || n >= N_CTLS || !gParams->ctl[n].inuse) ) {
        return FALSE;
    }

	if (buddy==1){
		hBuddy = gParams->ctl[n].hBuddy1;
		if (hBuddy==NULL) return false;
		gParams->ctl[n].buddy1Style &= ~flags;
	} else {
		hBuddy = gParams->ctl[n].hBuddy2;
		if (hBuddy==NULL) return false;
		gParams->ctl[n].buddy2Style &= ~flags;
	}
	

	SetWindowLong(hBuddy, GWL_STYLE,
        GetWindowLong(hBuddy, GWL_STYLE) & ~flags);
    //use SWP_FRAMECHANGED to force nonclient area update...
    return SetWindowPos(hBuddy, NULL,
                        0, 0, 0, 0,
                        SWP_FRAMECHANGED|SWP_NOZORDER|SWP_NOACTIVATE|SWP_NOSIZE|SWP_NOMOVE);
} /*fm_clearBuddyCtlStyle*/


int setCtlBuddyStyleEx(int n, int buddy, int flags)
{
    HWND hBuddy;

    if ( (n < 0 || n >= N_CTLS || !gParams->ctl[n].inuse) ) {
        return FALSE;
    }

	if (buddy==1){
		hBuddy = gParams->ctl[n].hBuddy1;
		if (hBuddy==NULL) return false;
        // FIX by Ognen Genchev
        // the OR condition is removed so it can directly assign flags to the structure member
        gParams->ctl[n].buddy1StyleEx = flags;
        // end
    } else {
        hBuddy = gParams->ctl[n].hBuddy2;
        if (hBuddy==NULL) return false;
        // FIX by Ognen Genchev
        // the OR condition is removed so it can directly assign flags to the structure member
        gParams->ctl[n].buddy2StyleEx = flags;
        // end
    }
	

	SetWindowLong(hBuddy, GWL_EXSTYLE,
        GetWindowLong(hBuddy, GWL_EXSTYLE) | flags);
    //use SWP_FRAMECHANGED to force nonclient area update...
    return SetWindowPos(hBuddy, NULL,
                        0, 0, 0, 0,
                        SWP_FRAMECHANGED|SWP_NOZORDER|SWP_NOACTIVATE|SWP_NOSIZE|SWP_NOMOVE);
} /*fm_setCtlBuddyStyleEx*/

int clearCtlBuddyStyleEx(int n, int buddy, int flags)
{
    HWND hBuddy;

    if ( (n < 0 || n >= N_CTLS || !gParams->ctl[n].inuse) ) {
        return FALSE;
    }

	if (buddy==1){
		hBuddy = gParams->ctl[n].hBuddy1;
		if (hBuddy==NULL) return false;
		gParams->ctl[n].buddy1StyleEx &= ~flags;
	} else {
		hBuddy = gParams->ctl[n].hBuddy2;
		if (hBuddy==NULL) return false;
		gParams->ctl[n].buddy2StyleEx &= ~flags;
	}
	

	SetWindowLong(hBuddy, GWL_EXSTYLE,
        GetWindowLong(hBuddy, GWL_EXSTYLE) & ~flags);
    //use SWP_FRAMECHANGED to force nonclient area update...
    return SetWindowPos(hBuddy, NULL,
                        0, 0, 0, 0,
                        SWP_FRAMECHANGED|SWP_NOZORDER|SWP_NOACTIVATE|SWP_NOSIZE|SWP_NOMOVE);
} /*fm_clearCtlBuddyStyleEx*/


int setCtlBuddyFontColor(int n, int color)
{
    if (n < 0 || n >= N_CTLS || !gParams->ctl[n].inuse) {
        return -1;
    }

	gParams->ctl[n].textColor = color;
	//Kludge to force a window update...
	//return setCtlText(n, (char *)gParams->ctl[n].label);
	return true;
}

int setCtlBuddyColor(int n, int color)
{
    if (n < 0 || n >= N_CTLS || !gParams->ctl[n].inuse) {
        return -1;
    }

	gParams->ctl[n].buddyBkColor = color;
	//Kludge to force a window update...
	//return setCtlText(n, (char *)gParams->ctl[n].label);
	return true;
}

int setCtlBuddyMaxSize(int n, int maxwidth, int maxheight)
{
    if (n < 0 || n >= N_CTLS || !gParams->ctl[n].inuse) {
        return -1;
    }

	gParams->ctl[n].sb.maxw2 = HDBUsToPixels(maxwidth);
	gParams->ctl[n].sb.maxh2 = VDBUsToPixels(maxheight);

	return true;
}

int setCtlEditColor(int n, int color)
{
    if (n < 0 || n >= N_CTLS || !gParams->ctl[n].inuse) {
        return -1;
    }

	gParams->ctl[n].editColor = color;
	//Kludge to force a window update...
	//return setCtlText(n, (char *)gParams->ctl[n].label);
	return true;
}

int setCtlEditTextColor(int n, int color)
{
    if (n < 0 || n >= N_CTLS || !gParams->ctl[n].inuse) {
        return -1;
    }

	gParams->ctl[n].editTextColor = color;
	//Kludge to force a window update...
	//return setCtlText(n, (char *)gParams->ctl[n].label);
	return true;
}


int getSysColor(int n)
{	
	return (int)GetSysColor(n);
}

int getSysMem(int n)
{	
	int val;

	//Win 95, 98, ME, NT
//	if (getWindowsVersion() <= 4){
	
		MEMORYSTATUS statex;

	  	
	  	statex.dwLength = sizeof (statex);
	  	GlobalMemoryStatus (&statex);
	  	
		switch (n){
			case 0:
				val = statex.dwMemoryLoad; // percent of memory in use 	
				break;
			case 1:
				val = (int)statex.dwTotalPhys / 1024;     // bytes of physical memory 
				break; 
			case 2:
				val = (int)statex.dwAvailPhys / 1024;     // free physical memory bytes 	
				break; 
			case 3:
				val = (int)statex.dwTotalPageFile / 1024; // bytes of paging file 
				break; 
			case 4:
				val = (int)statex.dwAvailPageFile / 1024; // free bytes of paging file 
				break; 
			case 5:
				val = (int)statex.dwTotalVirtual / 1024;  // user bytes of address space 
				break; 
			case 6:
				val = (int)statex.dwAvailVirtual / 1024;  // free user bytes 
				break; 
			
		}
	
	//Win 2000, XP, Vista	
/*	} else {
		
		MEMORYSTATUSEX statex;


  		statex.dwLength = sizeof (statex);
  		GlobalMemoryStatusEx (&statex);

		switch (n){
			case 0:
				val = statex.dwMemoryLoad; // percent of memory in use 	
				break;
			case 1:
				val = (int)(statex.ullTotalPhys / 1024);     // bytes of physical memory 
				break; 
			case 2:
				val = (int)(statex.ullAvailPhys / 1024);     // free physical memory bytes 	
				break; 
			case 3:
				val = (int)(statex.ullTotalPageFile / 1024); // bytes of paging file 
				break; 
			case 4:
				val = (int)(statex.ullAvailPageFile / 1024); // free bytes of paging file 
				break; 
			case 5:
				val = (int)(statex.ullTotalVirtual / 1024);  // user bytes of address space 
				break; 
			case 6:
				val = (int)(statex.ullAvailVirtual / 1024);  // free user bytes 
				break; 
			
		}
	}*/

	return val;
}
 
 
int checkArrayAlloc(int nr){

	int val;

	if (nr<0 || nr>99) val=0;
	else if (Array[nr]==NULL) val=0;
	else if (ArrayID[nr]!=0) val=2;
	else val=1;
	
	return val;
}


INT_PTR getArrayAddress(int nr){

	if (nr>=0 && nr<=99 && Array[nr] != NULL)
		return (INT_PTR)Array[nr];
	else