LPSETWINDOWTHEME lpSetWindowTheme;
HINSTANCE hinstUXTHEMEDLL;

HRESULT setWindowTheme(HWND hwnd, LPCWSTR pszSubAppName, LPCWSTR pszSubIdList)
{
    HRESULT hres = -1;     //assume error
//#if 0                                                   // temporarily disable
    BOOL hres2 = 0;
    hinstUXTHEMEDLL = LoadLibrary("uxtheme.dll");
    if (hinstUXTHEMEDLL) {
        lpSetWindowTheme = (LPSETWINDOWTHEME) GetProcAddress(hinstUXTHEMEDLL, "SetWindowTheme");
        if (lpSetWindowTheme) {
            hres = lpSetWindowTheme( hwnd, pszSubAppName, pszSubIdList);
            //if (hres) Info("hres = %8.8x", hres);
            hres2 = FreeLibrary(hinstUXTHEMEDLL);
            //if (!hres2) Info("hres2 = %8.8x", hres2);
        }
        else {
            //Info("lpSetWindowTheme = %8.8x", lpSetWindowTheme);
        }
    }
    else {
        //Info("hinstUXTHEMEDLL = %8.8x", hinstUXTHEMEDLL);
    }
//#endif

    return hres;
} // setWindowTheme*/


//Internal Theme Variable -> theme off by default -> replaced by gParams->gDialogTheme
//int DefaultTheme=0;

typedef HANDLE HTHEME;
typedef HTHEME (STDAPICALLTYPE *LPOPENTHEMEDATA) (HWND, LPCWSTR);
LPOPENTHEMEDATA lpOpenThemeData;
typedef HRESULT (STDAPICALLTYPE *LPDRAWTHEMEBACK) (HTHEME, HDC, INT, INT, RECT*, RECT*);
LPDRAWTHEMEBACK lpDrawThemeBackground;
typedef HRESULT (STDAPICALLTYPE *LPCLOSETHEMEDATA) (HTHEME);
LPCLOSETHEMEDATA lpCloseThemeData;

/*int drawThemePart(int n, RECT * rc, LPCWSTR topic, int part, int state){

	HDC  hDC;
	HTHEME Theme;

	if (n >= 0 && n<N_CTLS)
		hDC = GetDC(gParams->ctl[n].hCtl);
	else
		hDC = GetDC(fmc.hDlg);

    hinstUXTHEMEDLL = LoadLibrary("uxtheme.dll");
    if (hinstUXTHEMEDLL) {
        lpOpenThemeData = (LPOPENTHEMEDATA) GetProcAddress(hinstUXTHEMEDLL, "OpenThemeData");
		lpDrawThemeBackground = (LPDRAWTHEMEBACK) GetProcAddress(hinstUXTHEMEDLL, "DrawThemeBackground");
		lpCloseThemeData = (LPCLOSETHEMEDATA) GetProcAddress(hinstUXTHEMEDLL, "CloseThemeData");

        Theme = lpOpenThemeData(fmc.hDlg,topic);
		lpDrawThemeBackground(Theme, hDC, part, state, rc, NULL);
		lpCloseThemeData(Theme);

		FreeLibrary(hinstUXTHEMEDLL);
		return true;
    }

	return false;
}*/


/**********************************************************************/
/* setDefaultWindowTheme(hwnd)
/*
/*  Sets the Visual Theme for a given window to the default visual
/*  theme, which for now means to turn OFF any XP Visual Styles.
/*
/**********************************************************************/
/*HRESULT setDefaultWindowTheme(HWND hwnd)
{
    // Turn off XP Visual Styles for now.
    if (gParams->gDialogTheme==0) //DefaultTheme
		return setWindowTheme(hwnd, L"", L""); //disable theme
	else
		return setWindowTheme(hwnd, NULL, NULL); //enable theme

} // setDefaultWindowTheme


/**********************************************************************/
/* setCtlTheme(n, pszSubAppName, pszSubIdList)
/*
/*  Sets the Visual Theme for control n to the specified theme.
/*
/**********************************************************************/
/*int setCtlTheme(int n, int pszSubAppName, int pszSubIdList) //Removed fm_ infront of setCtlTheme
{
    if (n < 0 || n >= N_CTLS || !gParams->ctl[n].inuse) {
        return -1;
    }

    // todo: convert ascii strings to unicode

    // todo: setWindowTheme for buddies...

    return setWindowTheme(gParams->ctl[n].hCtl, L"", L"" );  //for now
}
*/

/*
typedef HRESULT (STDAPICALLTYPE *LPENABLETHEMEDIALOGTEXTURE) (HWND, DWORD);

HRESULT setCtlTabTheme(int n, int val)
{
    HRESULT hres;
    DWORD flags;
    LPENABLETHEMEDIALOGTEXTURE lpEnableThemeDialogTexture;

    if (val==0) flags = ETDT_DISABLE; else flags = ETDT_ENABLETAB;

    hinstUXTHEMEDLL = LoadLibrary("uxtheme.dll");
    if (hinstUXTHEMEDLL) {
        lpEnableThemeDialogTexture = (LPENABLETHEMEDIALOGTEXTURE) GetProcAddress(hinstUXTHEMEDLL, "EnableThemeDialogTexture");
        if (lpSetWindowTheme) {
            hres = lpEnableThemeDialogTexture( gParams->ctl[n].hCtl, flags);
            Info ("setCtlTabTheme");
            FreeLibrary(hinstUXTHEMEDLL);
        }
        else {
            return false;
        }
    }
    else {
        return false;
    }

    return true;
}
*/



//New Styles Functions

typedef BOOL (STDAPICALLTYPE *LPISTHEMEACTIVE) (VOID);
LPISTHEMEACTIVE lpIsThemeActive;

typedef struct _DllVersionInfo {
    DWORD cbSize;
    DWORD dwMajorVersion;
    DWORD dwMinorVersion;
    DWORD dwBuildNumber;
    DWORD dwPlatformID;
} DLLVERSIONINFO;

//
// Following code is cribbed from:
//
//      http://www.codeproject.com/KB/tips/DetectTheme.aspx
//
// q.v. for possible errors and updates!
//
int getAppTheme (void){

    int ret = 0;
    OSVERSIONINFO ovi = {0};
    ovi.dwOSVersionInfoSize = sizeof ovi;
    GetVersionEx(&ovi);
    
    
    if (appTheme!=-1) return appTheme;


	if(ovi.dwMajorVersion>=5) // && ovi.dwMinorVersion==1)
    {
        //Windows XP or higher detected
        typedef BOOL WINAPI ISAPPTHEMED(void);
        typedef BOOL WINAPI ISTHEMEACTIVE(void);
        ISAPPTHEMED* pISAPPTHEMED = NULL;
        ISTHEMEACTIVE* pISTHEMEACTIVE = NULL;
        HMODULE hMod = LoadLibrary("uxtheme.dll");
        if(hMod)
        {
            pISAPPTHEMED = (ISAPPTHEMED*) GetProcAddress(hMod, "IsAppThemed");				
            pISTHEMEACTIVE = (ISTHEMEACTIVE*) GetProcAddress(hMod,"IsThemeActive");

            if(pISAPPTHEMED && pISTHEMEACTIVE)
            {
                if(pISAPPTHEMED() && pISTHEMEACTIVE())                
                {                
                    typedef HRESULT CALLBACK DLLGETVERSION(DLLVERSIONINFO*);
                    DLLGETVERSION* pDLLGETVERSION = NULL;

                    HMODULE hModComCtl = LoadLibrary("comctl32.dll");
                    if(hModComCtl)
                    {
                        pDLLGETVERSION = (DLLGETVERSION*) GetProcAddress(hModComCtl,"DllGetVersion");
                        if(pDLLGETVERSION)
                        {
                            DLLVERSIONINFO dvi = {0};
                            dvi.cbSize = sizeof dvi;
                            if(pDLLGETVERSION(&dvi) == NOERROR )
                            {
                                //ret = dvi.dwMajorVersion >= 6;
								if (dvi.dwMajorVersion >= 6) 
									ret = 1;	
                            }
                        }
                        FreeLibrary(hModComCtl);                    
                    }
                }
            }
            FreeLibrary(hMod);
        }
    }    
    
    appTheme = ret;

    return ret;

}


// int setDialogTheme (int state){

// 	if (state==0){
// 		//DefaultTheme=0;
// 		gParams->gDialogTheme=0;
// 		return setWindowTheme(fmc.hDlg, L"", L""); //disable theme
// 	} else {
		
// 		//These two lines make sure that the title bar does not looses its theme at the second invocation
// 	    SetWindowRgn(fmc.hDlg, NULL, TRUE /*redraw*/);
// 		gParams->gDialogRegion = NULL;

// 		//DefaultTheme=1;
// 		gParams->gDialogTheme=1;
// 		return setWindowTheme(fmc.hDlg, NULL, NULL); //enable theme
// 	}
// }

// Added by Ognen Genchev
int setCustomCtl(int n, int iName)
{
    if (n < 0 || n >= N_CTLS || !gParams->ctl[n].inuse) {
        return false;
    }

    hBitmapThumb[0] = (HBITMAP)LoadImage((HINSTANCE)hDllInstance, "c:\\Temp\\thumb.bmp", IMAGE_BITMAP, 0, 0, LR_LOADFROMFILE);
    hBitmapChannel[0] = (HBITMAP)LoadImage((HINSTANCE)hDllInstance, "c:\\Temp\\channel.bmp", IMAGE_BITMAP, 0, 0, LR_LOADFROMFILE);
    GetObject( hBitmapThumb[0], sizeof( BITMAP ), &bm );
    GetObject( hBitmapChannel[0], sizeof( BITMAP ), &bm );

    hBitmapThumb[1] = (HBITMAP)LoadImage((HINSTANCE)hDllInstance, "c:\\Temp\\thumb1.bmp", IMAGE_BITMAP, 0, 0, LR_LOADFROMFILE);
    hBitmapChannel[1] = (HBITMAP)LoadImage((HINSTANCE)hDllInstance, "c:\\Temp\\channel1.bmp", IMAGE_BITMAP, 0, 0, LR_LOADFROMFILE);
    GetObject( hBitmapThumb[1], sizeof( BITMAP ), &bm );
    GetObject( hBitmapChannel[1], sizeof( BITMAP ), &bm );

    return TRUE;
} // setCustomCtl()

// setFrameColor() added by Ognen Genchev
int setFrameColor(int n, int color)
{
    HWND hCtl = gParams->ctl[n].hCtl;

    if (n < 0 || n >= N_CTLS || !gParams->ctl[n].inuse) {
        return false;
    }
    // shapeColor converted to HGDIOBJ
    DeleteObject((HGDIOBJ)gParams->ctl[n].shapeColor);
    gParams->ctl[n].shapeColor = (COLORREF)color;

    InvalidateRect(hCtl, NULL, TRUE /*erase background*/);
    return UpdateWindow(hCtl);
}// setFrameColor()

/*int setCtlTheme(int n, int state)
{
    int nIDDlgItem;
	LPCWSTR pszSubAppName; 
	LPCWSTR pszSubIdList;
	int RetVal=0;

	//if (n < 0 || n >= N_CTLS) { //|| !gParams->ctl[n].inuse
	if ( (n < 0 || n >= N_CTLS || !gParams->ctl[n].inuse)) { //  && n != CTL_PROGRESS && n != CTL_FRAME && n != CTL_ZOOM
	    return -1;
    }

    // todo: setWindowTheme for buddies...

	if (n == CTL_PROGRESS) nIDDlgItem = IDC_PROGRESS1;
    else if (n == CTL_FRAME) nIDDlgItem = IDC_PROXY_GROUPBOX;
    else if (n == CTL_ZOOM) nIDDlgItem = 498; //Zoom Lable
    else  nIDDlgItem = IDC_CTLBASE+n;


	if (state==-1){ //DefaultTheme
		if (gParams->gDialogTheme==0) {
			pszSubAppName = L""; //disable theme
			pszSubIdList = L"";
		} else if (gParams->gDialogTheme==1){