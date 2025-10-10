
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
    if (dataLen > REG_DATA_LIMIT) {
        //limit amount of data user is allowed to store in registry
        return ERROR_INVALID_DATA;
    }
#endif //REG_DATA_LIMIT
    hkey = RCK(gParams->gRegRoot, formatString(gParams->gRegPath));
    return RSB(hkey, formatString(szName), pData, dataLen);
} /*fm_putRegData*/

/**********************************************************************/
/* getRegData(void *data, int dataLen, lpsz szValueName[, vargargs]...)
/*  Gets a set of binary data from the registry, with printf-style
/*  formatting for the value name.
/*
/**********************************************************************/
int getRegData(void *pData, int dataLen, LPCSTR szValueName,...)
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
    return RQB(hkey, formatString(szName), pData, dataLen);
} /*fm_getRegData*/

/**********************************************************************/
/* enumRegSubKey(int index, lpsz szSubKey, int maxSubKeyLen)
/*  Gets name of n-th subkey under current registry key.
/*
/**********************************************************************/
int enumRegSubKey(int index, LPSTR szSubKey, int maxSubKeyLen)
{
    HKEY hkey;

    hkey = ROK(gParams->gRegRoot, formatString(gParams->gRegPath));
    return REK(hkey, index, szSubKey, maxSubKeyLen);
} /*fm_enumRegSubKey*/

/**********************************************************************/
/* enumRegValue(int index, lpsz szValueName, int maxValueNameLen,
/*              int *iType, int *cbData)
/*  Gets name, type, and data size of n-th value under current registry
/*  key.
/*
/**********************************************************************/
int enumRegValue(int index, LPSTR szValueName, int maxValueNameLen,
                    int *iType, int *cbData)
{
    HKEY hkey;

    hkey = ROK(gParams->gRegRoot, formatString(gParams->gRegPath));
    return REV(hkey, index, szValueName, (LPDWORD)&maxValueNameLen, (LPDWORD)iType, NULL, (LPDWORD)cbData);
} /*fm_enumRegValue*/

/**********************************************************************/
/* deleteRegSubKey(lpsz szSubKey[, varargs]...)
/*  Deletes specified subkey (and all its values) below the current
/*  key.  The subkey name is specified with printf-style formatting.
/*  The specified subkey must not have sub-subkeys (at least on NT;
/*  Win95 may allow deletion of an entire key subtree?).
/*
/**********************************************************************/
int deleteRegSubKey(LPCSTR szSubKey,...)
{
    va_list ap;
    char szName[1024];
    int cb;
    HKEY hkey;

    va_start(ap, szSubKey);
    cb = _vsnprintf(szName, sizeof(szName), szSubKey, ap);
    szName[sizeof(szName)-1] = 0;   //ensure NUL-terminated
    va_end(ap);
    if (cb < 0) {
        //oops... expanded string is too long
        return ERROR_INVALID_DATA;
    }
    hkey = RCK(gParams->gRegRoot, formatString(gParams->gRegPath));
    return RDK(hkey, formatString(szName));
} /*fm_deleteRegSubKey*/

/**********************************************************************/
/* deleteRegValue(lpsz szValueName[, vargargs]...)
/*  Deletes specified value under the current key.  The value name is
/*  specified with printf-style formatting.
/*
/**********************************************************************/
int deleteRegValue(LPCSTR szValueName,...)
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
    return RDV(hkey, formatString(szName));
} /*fm_deleteRegValue*/



int shellExec(char* szVerb, char* szFilename, char* szParams, char* szDefDir)
{
    HINSTANCE ret = ShellExecute(fmc.hDlg, (char *)szVerb,
                                              (char *)szFilename,
                                              (char *)szParams,
                                              (char *)szDefDir,
                                              SW_SHOWNORMAL);
    if ((INT_PTR)ret <= 32) {
#ifdef VERBOSE
        ErrorOk("shellExec failed: %d", ret);
#endif
        return 0; //ret;   //failed
    }
    return -1;    //success
} /*fm_shellExec*/

int msgBox(UINT uType, LPCTSTR lpCaption, LPCTSTR lpFmt,...)
{
    char szBuffer[1024];
    va_list ap;

    va_start(ap, lpFmt); //(LPCTSTR)
    _vsnprintf(szBuffer, sizeof(szBuffer), lpFmt, ap);
    szBuffer[sizeof(szBuffer)-1] = 0;   //ensure NUL-terminated
    va_end(ap);
    return MessageBox(
        fmc.doingProxy?fmc.hDlg:NULL,   // handle of owner window
        formatString(szBuffer),      // interpolated text for message box
        lpCaption,                      // address of title of message box  
        uType                           // style of message box
        | MB_TASKMODAL                  // Force task modal style.
       );
} /*msgBox*/

int fm_printf(char* lpFmt,...)
{
    char szBuffer[1024];
    va_list ap;
    int nchars;

    va_start(ap, lpFmt);
    nchars = _vsnprintf(szBuffer, sizeof(szBuffer), (LPCTSTR)lpFmt, ap);
    szBuffer[sizeof(szBuffer)-1] = 0;   //ensure NUL-terminated
    va_end(ap);
    MessageBox(
        fmc.doingProxy?fmc.hDlg:NULL,   // handle of owner window
        formatString(szBuffer),      // interpolated text for message box
        
        //Added by Harald Heim, January 2003
        "stdout",           // address of title of message box  
        //"FilterMeister stdout",           // address of title of message box  

        0                               // style of message box
        | MB_TASKMODAL                  // Force task modal style.
       );
    return nchars;
} /*fm_printf*/

int Info(char* lpFmt,...)
{
    char szBuffer[1024];
    va_list ap;

    va_start(ap, lpFmt);
    _vsnprintf(szBuffer, sizeof(szBuffer), (LPCTSTR)lpFmt, ap);
    szBuffer[sizeof(szBuffer)-1] = 0;   //ensure NUL-terminated
    va_end(ap);
    return MessageBox(
        fmc.doingProxy?fmc.hDlg:NULL,   // handle of owner window
        formatString(szBuffer),      // interpolated text for message box
        
        //Added by Harald Heim, January 12, 2003
         fmc.filterTitleText,
        //"FilterMeister Info",         // address of title of message box  

        MB_ICONINFORMATION              // style of message box
        | MB_TASKMODAL                  // Force task modal style.
        //| MB_TOPMOST                    // Force topmost window.
        //| MB_SETFOREGROUND              // Force foreground window.
       );
} /*Info*/

int Warn(char* lpFmt,...)
{
    char szBuffer[1024];   
	va_list ap;

	va_start(ap, lpFmt);
    _vsnprintf(szBuffer, sizeof(szBuffer), (LPCTSTR)lpFmt, ap);
    szBuffer[sizeof(szBuffer)-1] = 0;   //ensure NUL-terminated
    va_end(ap);
    return MessageBox(
        fmc.doingProxy?fmc.hDlg:NULL,   // handle of owner window
        formatString(szBuffer),      // interpolated text for message box
        
        //Added by Harald Heim, January 12, 2003
         "Warning",
        //"FilterMeister Warning",      // address of title of message box  
        MB_ICONWARNING|MB_OKCANCEL      // style of message box
        | MB_TASKMODAL                  // Force task modal style.
       );
} /*Warn*/

int Error(char* lpFmt,...)
{
    char szBuffer[1024];
    va_list ap;

    va_start(ap, lpFmt);
    _vsnprintf(szBuffer, sizeof(szBuffer), (LPCTSTR)lpFmt, ap);
    szBuffer[sizeof(szBuffer)-1] = 0;   //ensure NUL-terminated
    va_end(ap);
    return MessageBox(
        fmc.doingProxy?fmc.hDlg:NULL,   // handle of owner window
        formatString(szBuffer),      // interpolated text for message box
        
        //Added by Harald Heim, January 12, 2003        
        "Error",
        //"FilterMeister Error",            // address of title of message box  
        
        MB_ICONERROR|MB_ABORTRETRYIGNORE // style of message box
        | MB_TASKMODAL                  // Force task modal style.
       );
} /*Error*/

int ErrorOk(char* lpFmt,...)
{
    char szBuffer[1024];
    va_list ap;

    va_start(ap, lpFmt);
    _vsnprintf(szBuffer, sizeof(szBuffer), (LPCTSTR)lpFmt, ap);
    szBuffer[sizeof(szBuffer)-1] = 0;   //ensure NUL-terminated
    va_end(ap);
    return MessageBox(
        fmc.doingProxy?fmc.hDlg:NULL,   // handle of owner window
        formatString(szBuffer),      // interpolated text for message box
        
        //Added by Harald Heim, January 12, 2003
        "Error",
        //"FilterMeister Error",            // address of title of message box  

        MB_ICONERROR|MB_OK              // style of message box
        | MB_TASKMODAL                  // Force task modal style.
       );
} /*ErrorOk*/

int YesNo(char* lpFmt,...)
{
    char szBuffer[1024];
    va_list ap;

    va_start(ap, lpFmt);
    _vsnprintf(szBuffer, sizeof(szBuffer), (LPCTSTR)lpFmt, ap);
    szBuffer[sizeof(szBuffer)-1] = 0;   //ensure NUL-terminated
    va_end(ap);
    return MessageBox(
        fmc.doingProxy?fmc.hDlg:NULL,   // handle of owner window
        formatString(szBuffer),      // interpolated text for message box
        
        //Added by Harald Heim, January 12, 2003
         fmc.filterTitleText,
        //"FilterMeister",              // address of title of message box  

        MB_ICONQUESTION|MB_YESNO        // style of message box
        | MB_TASKMODAL                  // Force task modal style.
       );
} /*YesNo*/

int YesNoCancel(char* lpFmt,...)
{
    char szBuffer[1024];