    ofn.lpstrFilter       = lpstrFilter;
    ofn.lpstrCustomFilter = lpstrCustomFilter;
    ofn.nMaxCustFilter    = nMaxCustFilter;
    ofn.nFilterIndex      = pnFilterIndex ? *pnFilterIndex : 1;
    ofn.nMaxFile          = nMaxFile;
    ofn.nMaxFileTitle     = nMaxFileTitle;
    ofn.lpstrInitialDir   = lpstrInitialDir;
    ofn.lpstrTitle        = lpstrDialogTitle;
    ofn.nFileOffset       = 0;
    ofn.nFileExtension    = 0;
    ofn.lpstrDefExt       = lpstrDefExt;
    ofn.lCustData         = 0;
    ofn.lpfnHook          = NULL;
    ofn.lpTemplateName    = NULL;
    ofn.lpstrFile         = lpstrFile;
    ofn.lpstrFileTitle    = lpstrFileTitle;
    ofn.Flags             = flags | OFN_EXPLORER;   //force Explorer-style dialog
    // Don't allow user to set following flags...
    ofn.Flags &= ~(OFN_ENABLEHOOK | OFN_ENABLETEMPLATE | OFN_ENABLETEMPLATEHANDLE);

    ok = GetOpenFileName(&ofn);

#if 1
    if (pOflags) *pOflags = ofn.Flags;
#else
    //screen out non-return bits?
    if (pOflags) {
        *pOflags = ofn.Flags & (OFN_EXTENSIONDIFFERENT | OFN_NOREADONLYRETURN | OFN_READONLY);
    }
#endif
    if (pnFileOffset) *pnFileOffset = ofn.nFileOffset;
    if (pnFileExtension) *pnFileExtension = ofn.nFileExtension;
    if (pnFilterIndex) *pnFilterIndex = ofn.nFilterIndex;
    if (ok) {
        iError = 0;   //success
    }
    else {
        //error...
        iError = CommDlgExtendedError();
        if (iError == 0) {
            //User closed or canceled the dialog box...
            iError = -1;
        }
    }
    return iError;
} //fm_getOpenFileName

/**********************************************************************/
/* iError = getSaveFileName( flags,
/*                     lpstrFile, nMaxFile,
/*                     [&nFIleOffset], [&nFileExtension],
/*                     [lpstrFileTitle], nMaxFileTitle,
/*                     [lpstrFilter], 
/*                     [lpstrCustomFilter], nMaxCustFilter,
/*                     [&nFilterIndex],
/*                     [lpstrInitialDir],
/*                     [lpstrDialogTitle],
/*                     [lpstrDefExt],
/*                     [&oflags]
/*                   );
/*
/*  Invokes the Save As... file common dialog.
/*
/**********************************************************************/
int getSaveFileName( int flags,
                     LPSTR lpstrFile, int nMaxFile,
                     int *pnFileOffset, int *pnFileExtension,
                     LPSTR lpstrFileTitle, int nMaxFileTitle,
                     LPCSTR lpstrFilter,
                     LPSTR lpstrCustomFilter, int nMaxCustFilter,
                     int *pnFilterIndex,
                     LPCSTR lpstrInitialDir,
                     LPCSTR lpstrDialogTitle,
                     LPCSTR lpstrDefExt,
                     int *pOflags
                   )
{
    OPENFILENAME ofn;
    BOOL ok;
    int iError;

    ofn.lStructSize       = sizeof(ofn);
    ofn.hwndOwner         = fmc.doingProxy?fmc.hDlg:NULL;   // handle of owner window
    ofn.hInstance         = NULL;
    ofn.lpstrFilter       = lpstrFilter;
    ofn.lpstrCustomFilter = lpstrCustomFilter;
    ofn.nMaxCustFilter    = nMaxCustFilter;
    ofn.nFilterIndex      = pnFilterIndex ? *pnFilterIndex : 1;
    ofn.nMaxFile          = nMaxFile;
    ofn.nMaxFileTitle     = nMaxFileTitle;
    ofn.lpstrInitialDir   = lpstrInitialDir;
    ofn.lpstrTitle        = lpstrDialogTitle;
    ofn.nFileOffset       = 0;
    ofn.nFileExtension    = 0;
    ofn.lpstrDefExt       = lpstrDefExt;
    ofn.lCustData         = 0;
    ofn.lpfnHook          = NULL;
    ofn.lpTemplateName    = NULL;
    ofn.lpstrFile         = lpstrFile;
    ofn.lpstrFileTitle    = lpstrFileTitle;
    ofn.Flags             = flags | OFN_EXPLORER;   //force Explorer-style dialog
    // Don't allow user to set following flags...
    ofn.Flags &= ~(OFN_ENABLEHOOK | OFN_ENABLETEMPLATE | OFN_ENABLETEMPLATEHANDLE | OFN_ALLOWMULTISELECT);

    ok = GetSaveFileName(&ofn);

#if 1
    if (pOflags) *pOflags = ofn.Flags;
#else
    //screen out non-return bits?
    if (pOflags) {
        *pOflags = ofn.Flags & (OFN_EXTENSIONDIFFERENT | OFN_NOREADONLYRETURN | OFN_READONLY);
    }
#endif
    if (pnFileOffset) *pnFileOffset = ofn.nFileOffset;
    if (pnFileExtension) *pnFileExtension = ofn.nFileExtension;
    if (pnFilterIndex) *pnFilterIndex = ofn.nFilterIndex;
    if (ok) {
        iError = 0;   //success
    }
    else {
        //error...
        iError = CommDlgExtendedError();
        if (iError == 0) {
            //User closed or canceled the dialog box...
            iError = -1;
        }
    }
    return iError;
} //fm_getSaveFileName



int ctlColorBox;
int ctlColorDialogPreview;
int cdLanguage = 0;
int cdInitialColor;
int cdOldcolor = 0;
//char * strColorTitle;
HWND foundWindow = 0;

#include <ColorDlg.h>
#define COLOR_PREVIEW 740


BOOL CALLBACK EnumChildProc(__in  HWND hwnd,__in  LPARAM lParam){
	char tempString[256];
	GetWindowText(hwnd,tempString,256);
	if (strcmp(tempString,(char *)lParam)==0){
		foundWindow = hwnd;
		return FALSE;
	}
	return TRUE;
}

void findChildWindow (HWND window, char * text){
	foundWindow = 0;
	EnumChildWindows(window,EnumChildProc,(LPARAM)text);
}

void cdUpdatePreview(HWND hdlg){

	if (SendMessage(GetDlgItem(hdlg,COLOR_PREVIEW),BM_GETCHECK, 0, 0)){ //Is Preview check box activated?
		BOOL fTranslated;
		int rVal = (int)GetDlgItemInt(hdlg, COLOR_RED, &fTranslated, TRUE);
		int gVal = (int)GetDlgItemInt(hdlg, COLOR_GREEN, &fTranslated, TRUE);
		int bVal = (int)GetDlgItemInt(hdlg, COLOR_BLUE, &fTranslated, TRUE);
		int color = RGB(rVal,gVal,bVal);
		
		if (fTranslated && color != cdOldcolor) {
			//setCtlColor(ctlColorBox,color);  //Avoid redraw problem on some systems
			setCtlVal(ctlColorBox,color);
			Sleep(30); //Avoid preview flicker
			doAction(CA_PREVIEW);
			cdOldcolor = RGB(rVal,gVal,bVal);
		}
	}

}

UINT_PTR CALLBACK CCHookProc(__in  HWND hdlg,__in  UINT uiMsg,__in  WPARAM wParam,__in  LPARAM lParam){

	if(uiMsg == WM_INITDIALOG){
		
		char * controlTextEN[12]={"&Basic colors:","&Custom colors:","Cancel","Color","|S&olid","Hu&e:","&Sat:","&Lum:","&Red:","&Green:","Bl&ue:","&Add to Custom Colors"};
		char * controlTextDE[12]={"&Grundfarben:","Benut&zerdefinierte Farben:","Abbruch","Farbe","|&Basis","&Farbt.:","&S�tt.:","H&ell.:","&Rot:","Gr�&n:","B&lau:","Farben hinzuf&�gen"};
		char ** controlText = &controlTextEN[0];
		int controlID[12]={-1,COLOR_CUSTOM1,IDCANCEL,COLOR_SOLID_LEFT,COLOR_SOLID_RIGHT,COLOR_HUEACCEL,COLOR_SATACCEL,COLOR_LUMACCEL,COLOR_REDACCEL,COLOR_GREENACCEL,COLOR_BLUEACCEL,COLOR_ADD};
		RECT r;
		HWND hCtl;
		int i;

		if (cdLanguage==1) controlText = &controlTextDE[0]; //Switch to German

		//Set window title text
		SetWindowText(hdlg, cdLanguage?"Bitte w�hlen Sie eine Farbe:":"Please choose a color:" ); //strColorTitle 

		//Localize
		findChildWindow(hdlg,"&Basic colors:");
		if (foundWindow==0) findChildWindow(hdlg,"&Grundfarben:");
		if (foundWindow!=0) SetWindowText(foundWindow,controlText[0]);
		findChildWindow(hdlg,"&Custom colors:");
		if (foundWindow==0) findChildWindow(hdlg,"Benut&zerdefinierte Farben:");
		if (foundWindow!=0) SetWindowText(foundWindow,controlText[1]);
		for (i=2;i<12;i++) SetWindowText(GetDlgItem(hdlg,controlID[i]),controlText[i]);

		//Hide button
		EnableWindow(GetDlgItem(hdlg,COLOR_MIX), FALSE);
		ShowWindow(GetDlgItem(hdlg,COLOR_MIX), SW_HIDE);

		//Add Preview check box
		if (ctlColorDialogPreview>0){
			GetWindowRect(GetDlgItem(hdlg,COLOR_MIX), &r); //Position where hidden button was
			ScreenToClient (hdlg, (LPPOINT)&r);

			hCtl = CreateWindowEx(  
					0,//gParams->ctl[n].styleEx,        // extended styles
					"BUTTON",                       // predefined class 
					cdLanguage?"Vorschau":"Preview",	// control text
					WS_VISIBLE | WS_CHILD | WS_TABSTOP | BS_AUTOCHECKBOX,          // styles
					r.left+3,                         // starting x position 
					r.top,                          // starting y position 
					80,//r.right,                        // control width 
					20,//r.bottom,                       // control height 
					hdlg,							// parent window 
					(HMENU)COLOR_PREVIEW,        // control ID
					(HINSTANCE)hDllInstance,
					NULL);                          // pointer not needed 

			// Set control font to same as dialog font
			if (hCtl) {
				HFONT hDlgFont;
				if ((hDlgFont = (HFONT)SendMessage(hdlg, WM_GETFONT, 0, 0L)) != NULL){
					SendMessage(hCtl, WM_SETFONT, (WPARAM)hDlgFont, 0L);  //redraw=FALSE
				}
			}
			//Activate check box ???
			if (ctl(ctlColorDialogPreview)) SendMessage(hCtl, BM_SETCHECK, TRUE, 0);
		}


		//Center dialog
		{
			RECT rc,mrc;
			GetWindowRect(hdlg, &rc);
			if (fmc.hDlg!=NULL){
				//GetWindowRect(fmc.hDlg, &mrc);
				GetWindowRect(gParams->ctl[ctlColorBox].hCtl, &mrc);
				SetWindowPos(hdlg, NULL, mrc.left + ((mrc.right-mrc.left)-(rc.right-rc.left))/2, mrc.top+((mrc.bottom-mrc.top)-(rc.bottom-rc.top))/2, 0, 0,SWP_NOSIZE|SWP_NOZORDER|SWP_NOACTIVATE|SWP_SHOWWINDOW);
			} else
				SetWindowPos(hdlg, NULL,(getDisplaySettings(1)-(rc.right-rc.left))/2, (getDisplaySettings(2)-(rc.bottom-rc.top))/2, 0, 0,SWP_NOSIZE|SWP_NOZORDER|SWP_NOACTIVATE|SWP_SHOWWINDOW);
		}

	} else if (uiMsg == WM_COMMAND){

		int idd = LOWORD(wParam);
        int iCmd = HIWORD(wParam);
		
		if (idd==COLOR_PREVIEW){ //Preview check
			int val = (int)SendMessage(GetDlgItem(hdlg,idd),BM_GETCHECK, 0, 0);
			setCtlVal(ctlColorDialogPreview, val);

			if (val){
				cdOldcolor=-1;
				cdUpdatePreview(hdlg);
			} else {
				BOOL fTranslated;
				int rVal = (int)GetDlgItemInt(hdlg, COLOR_RED, &fTranslated, TRUE);
				int gVal = (int)GetDlgItemInt(hdlg, COLOR_GREEN, &fTranslated, TRUE);
				int bVal = (int)GetDlgItemInt(hdlg, COLOR_BLUE, &fTranslated, TRUE);
				int color = RGB(rVal,gVal,bVal);
				
				if (cdInitialColor != color){ //Restore initial color
					//setCtlColor(ctlColorBox,cdInitialColor); //Avoid redraw problem on some systems
					setCtlVal(ctlColorBox,cdInitialColor);
					doAction(CA_PREVIEW);
				}
			}

		} else if (iCmd == EN_CHANGE){ //Color change
			if (idd==COLOR_RED||idd==COLOR_GREEN||idd==COLOR_BLUE){
				if (ctlColorDialogPreview>0) cdUpdatePreview(hdlg);	
			}
		}

	}

	return false;
}

int chooseColor2(int n, int initialColor, int ctlPreview, int language){ //char * title, 
	
	CHOOSECOLOR cc;
    static COLORREF acrCustClr[16];
	//char language[256];

	//Get language of color dialog
	//getLocaleInfo (LOCALE_USER_DEFAULT, LOCALE_SENGLANGUAGE, language, 255 );
	//if (strcmp (language, "German") ==0 ) cdLanguage = 1;
    