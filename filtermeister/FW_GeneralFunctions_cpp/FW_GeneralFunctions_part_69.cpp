    // Initialize CHOOSECOLOR 
    ZeroMemory(&cc, sizeof(cc));
    cc.lStructSize = sizeof(cc);
    cc.hwndOwner = fmc.hDlg;
    cc.lpCustColors = (LPDWORD) acrCustClr;
    cc.rgbResult = (DWORD) initialColor;
    cc.Flags = CC_FULLOPEN | CC_RGBINIT | CC_ENABLEHOOK;
	cc.lpfnHook = CCHookProc;

	//Set stuff for hook
	ctlColorBox = n;
	ctlColorDialogPreview = ctlPreview;
	//strColorTitle = title;
	cdLanguage = language;
	cdInitialColor = initialColor;
	
    if (ChooseColor(&cc))
        return cc.rgbResult;
	else {  //Canceled
		//Restore color
		//if (getCtlColor(n) != initialColor){
		if (ctl(n) != initialColor){
			//setCtlColor(n,initialColor);  //Avoid redraw problem on some systems
			setCtlVal(n,initialColor);
			doAction(CA_PREVIEW);
		}
		return -1;
	}

} //fm_chooseColor




/**********************************************************************/
/* chooseColor(initialColor, szPrompt, ...)
/*
/*  Invokes the host app's color picker to choose a color.
/*  Returns the chosen color, or -1 if an error occurred.
/*
/**********************************************************************/
int chooseColor(int initialColor, const char *szPrompt, ...)
{

#ifndef APP

	if (gStuff->colorServices) {

		ColorServicesInfo csi;
		OSErr err;
		Str255 pickerPrompt;
		char szBuffer[1024];
		va_list ap;

		/* Interpret and interpolate the prompt string */
		va_start(ap, szPrompt);
		_vsnprintf(szBuffer, sizeof(szBuffer), szPrompt, ap);
		szBuffer[sizeof(szBuffer)-1] = 0;   //ensure NUL-terminated
		va_end(ap);
		strncpy(szBuffer, formatString(szBuffer), sizeof(szBuffer));
		szBuffer[sizeof(szBuffer)-1] = 0;   //ensure NUL-terminated
		// convert to Pascal (byte-counted) string, 255 chars max
		strncpy((char *)&pickerPrompt[1], szBuffer, 255);
		pickerPrompt[0] = min(strlen(szBuffer), 255);

		csi.infoSize = sizeof(csi);
		csi.selector = plugIncolorServicesChooseColor;
		csi.sourceSpace = plugIncolorServicesRGBSpace;
		csi.resultSpace = plugIncolorServicesRGBSpace;
		csi.reservedSourceSpaceInfo = NULL;
		csi.reservedResultSpaceInfo = NULL;
		csi.reserved = NULL;
		csi.colorComponents[0] = Rval(initialColor);
		csi.colorComponents[1] = Gval(initialColor);
		csi.colorComponents[2] = Bval(initialColor);
		csi.colorComponents[3] = 0;
		csi.selectorParameter.pickerPrompt = &pickerPrompt;

        //call the host's color picker
        err = gStuff->colorServices(&csi);
        if (err == noErr) {
            return fm_RGB(csi.colorComponents[0], csi.colorComponents[1], csi.colorComponents[2]);
        } else {
			if (err == userCanceledErr) return -1;  //error occurred
        }
    }

#endif //APP

	//Display out own color dialog if host does not do it
    {
        //No host color service. Call the Windows native color picker.

        //Added by Harald Heim, January 2002
        CHOOSECOLOR cc;                 // common dialog box structure 
        static COLORREF acrCustClr[16]; // array of custom colors  NOT THREAD-SAFE!! (Should it be?? or just MT lock?)
        //HWND hwnd ;                      // owner window
        //static DWORD rgbCurrent;        // initial color selection

        // Initialize CHOOSECOLOR 
        ZeroMemory(&cc, sizeof(cc));
        cc.lStructSize = sizeof(cc);
        cc.hwndOwner = fmc.hDlg;
        cc.lpCustColors = (LPDWORD) acrCustClr;
        cc.rgbResult = (DWORD) initialColor;
        cc.Flags = CC_FULLOPEN | CC_RGBINIT;
		
        if (ChooseColor(&cc))
            return cc.rgbResult;
        else 
            return -1;

    }
} //fm_chooseColor


/**********************************************************************/
/* setRegRoot(int hkey)
/*  Sets the current registry root key.
/*
/**********************************************************************/
int setRegRoot(HKEY hkey)
{
    gParams->gRegRoot = hkey;
    return ERROR_SUCCESS;
} /*fm_setRegRoot*/

/**********************************************************************/
/* getRegRoot(int *hkey)
/*  Retrieves the current registry root key.
/*
/**********************************************************************/
int getRegRoot(HKEY *phkey)
{
    *phkey = gParams->gRegRoot;
    return ERROR_SUCCESS;
} /*fm_getRegRoot*/

/**********************************************************************/
/* setRegPath(lpsz szPath[, varargs]...)
/*  Sets the current registry path, with printf-style formatting.
/*  Returns ERROR_INVALID_DATA if expanded path string is too long;
/*  else returns ERROR_SUCCESS.
/*
/**********************************************************************/
int setRegPath(LPCSTR szPath,...)
{
    va_list ap;
    int cb;

    va_start(ap, szPath);
    cb = _vsnprintf(gParams->gRegPath, sizeof(gParams->gRegPath), szPath, ap);
    gParams->gRegPath[sizeof(gParams->gRegPath)-1] = 0;   //ensure NUL-terminated
    va_end(ap);
    return (cb < 0) ? ERROR_INVALID_DATA : ERROR_SUCCESS;
} /*fm_setRegPath*/

/**********************************************************************/
/* getRegPath(lpsz szPath, int maxPathLen)
/*  Retrieves the current (not yet interpolated) registry path.
/*  Returns ERROR_INVALID_DATA if caller's buffer is too small;
/*  else returns ERROR_SUCCESS.
/*
/**********************************************************************/
int getRegPath(LPSTR szPath, int maxPathLen)
{
    strncpy(szPath, gParams->gRegPath, maxPathLen);
    if (szPath[maxPathLen-1]) {
        //caller's buffer was too short...
        szPath[maxPathLen-1] = 0;   //force NUL-terminated string;
        return ERROR_INVALID_DATA;  //return error status
    }
    return ERROR_SUCCESS;
} /*fm_getRegPath*/

/**********************************************************************/
/* putRegInt(int iValue, lpsz szValueName[, varargs]...)
/*  Stores an integer value into the registry, with printf-style formatting
/*  for the value name.
/*
/**********************************************************************/
int putRegInt(int iValue, LPCSTR szValueName,...)
{
    va_list ap;
    char szName[1024];
    int cb;
    HKEY hkey;

    va_start(ap, szValueName);
    cb = _vsnprintf(szName, sizeof(szName), szValueName, ap);
    szName[sizeof(szName)-1] = 0;   //ensure NUL-terminated
    va_end(ap);
    if (cb < 0) {
        //oops... expanded string is too long
        return ERROR_INVALID_DATA;
    }
    hkey = RCK(gParams->gRegRoot, formatString(gParams->gRegPath));
    return RSD(hkey, formatString(szName), (DWORD)iValue);
} /*fm_putRegInt*/

/**********************************************************************/
/* getRegInt(int *iValue, lpsz szValueName[, varargs]...)
/*  Gets an integer value from the registry, with printf-style formatting
/*  for the value name.
/*
/**********************************************************************/
int getRegInt(int *iValue, LPCSTR szValueName,...)
{
    va_list ap;
    char szName[1024];
    int cb;
    HKEY hkey;

    va_start(ap, szValueName);
    cb = _vsnprintf(szName, sizeof(szName), szValueName, ap);
    szName[sizeof(szName)-1] = 0;   //ensure NUL-terminated
    va_end(ap);
    if (cb < 0) {
        //oops... expanded string is too long
        return ERROR_INVALID_DATA;
    }
    hkey = ROK(gParams->gRegRoot, formatString(gParams->gRegPath));
    if (hkey!=NULL)
        return RQD(hkey, formatString(szName), (DWORD *)iValue);
    else
        return ERROR_INVALID_ACCESS;
} /*fm_getRegInt*/

/**********************************************************************/
/* putRegString(lpsz szString, lpsz szValueName[, vargargs]...)
/*  Stores a string value into the registry, with printf-style formatting
/*  for the value name.
/*
/**********************************************************************/
int putRegString(LPCSTR szString, LPCSTR szValueName,...)
{
    va_list ap;
    char szName[1024];
    int cb;
    HKEY hkey;

    va_start(ap, szValueName);
    cb = _vsnprintf(szName, sizeof(szName), szValueName, ap);
    szName[sizeof(szName)-1] = 0;   //ensure NUL-terminated
    va_end(ap);
    if (cb < 0) {
        //oops... expanded string is too long
        return ERROR_INVALID_DATA;
    }
#ifdef REG_DATA_LIMIT
    //check for data value too long...
    if (strlen(szString) > REG_DATA_LIMIT) {
        //limit amount of data user is allowed to store in registry
        return ERROR_INVALID_DATA;
    }
#endif //REG_DATA_LIMIT
    hkey = RCK(gParams->gRegRoot, formatString(gParams->gRegPath));
    return RSS(hkey, formatString(szName), szString);
} /*fm_putRegString*/

/**********************************************************************/
/* getRegString(lpsz szString, int iMaxlen, lpsz szValueName[, varargs]...)
/*  Gets a string value from the registry, with printf-style formatting
/*  for the value name.
/*
/**********************************************************************/
int getRegString(LPSTR szString, int iMaxlen, LPCSTR szValueName,...)
{
    va_list ap;
    char szName[1024];
    int cb;
    HKEY hkey;

    va_start(ap, szValueName);
    cb = _vsnprintf(szName, sizeof(szName), szValueName, ap);
    szName[sizeof(szName)-1] = 0;   //ensure NUL-terminated
    va_end(ap);
    if (cb < 0) {
        //oops... expanded string is too long
        return ERROR_INVALID_DATA;
    }
    hkey = ROK(gParams->gRegRoot, formatString(gParams->gRegPath));
    if (hkey!=NULL)
        return RQS(hkey, formatString(szName), szString, iMaxlen);
    else
        return ERROR_INVALID_ACCESS;
} /*fm_getRegString*/

/**********************************************************************/
/* putRegData(void *data, int dataLen, lpsz szValueName[, vargargs]...)
/*  Stores a set of binary data into the registry, with printf-style
/*  formatting for the value name.
/*
/**********************************************************************/
int putRegData(const void *pData, int dataLen, LPCSTR szValueName,...)
{
    va_list ap;
    char szName[1024];
    int cb;
    HKEY hkey;