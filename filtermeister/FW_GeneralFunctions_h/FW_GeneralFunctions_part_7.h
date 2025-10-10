/*
/**********************************************************************/
EXTERNC int createTitlebarRgn(void);

/**********************************************************************/
/* REGION createMenubarRgn();
/*  Is a predefined rectangular region defining the area of the dialog
/*  menu bar (if any).
/*
/**********************************************************************/
EXTERNC int createMenubarRgn(void);

/**********************************************************************/
/* REGION createClientRgn();
/*  Is a predefined rectanglar region defining the client area of the
/*  dialog box.
/*
/**********************************************************************/
EXTERNC int createClientRgn(void);

/**********************************************************************/
/* REGION createStatusbarRgn();
/*  Is a predefined rectangular region defining the area of the dialog
/*  status bar (if any).
/*
/**********************************************************************/
EXTERNC int createStatusbarRgn(void);

/**********************************************************************/
/* BOOL playSoundWave(szWaveFile);
/*  Plays the specified wave file asynchronously.  Looks first for
/*  an embedded WAVE resource with the specified name; else searches
/*  the usual paths for the specified wave file.
/*
/**********************************************************************/
EXTERNC BOOL playSoundWave(INT_PTR szWaveFile);

/**********************************************************************/
/* BOOL playSoundWaveLoop(szWaveFile);
/*  Plays the specified wave file repeatedly.  Looks first for
/*  an embedded WAVE resource with the specified name; else searches
/*  the usual paths for the specified wave file.
/*
/**********************************************************************/
BOOL playSoundWaveLoop(INT_PTR szWaveFile);

/**********************************************************************/
/* BOOL playSoundWaveSync(szWaveFile);
/*  Plays the specified wave file synchronously.  Looks first for
/*  an embedded WAVE resource with the specified name; else searches
/*  the usual paths for the specified wave file.
/*
/**********************************************************************/
EXTERNC BOOL playSoundWaveSync(INT_PTR szWaveFile);

EXTERNC int fm_sleep(int msecs);

EXTERNC int fm_RGB(int r, int g, int b) ;
    
EXTERNC int RGBA(int r, int g, int b, int a) ;
    
EXTERNC int Rval(int rgba) ;
    
EXTERNC int Gval(int rgba) ;
    
EXTERNC int Bval(int rgba) ;
    
EXTERNC int Aval(int rgba);

/**********************************************************************/
/* iError = getOpenFileName( flags,
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
/*  Invokes the Open file common dialog.
/*
/**********************************************************************/
EXTERNC int getOpenFileName( int flags,
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
                   );

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
EXTERNC int getSaveFileName( int flags,
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
                   );


EXTERNC int chooseColor2(int n, int initialColor, int ctlPreview, int language);

/**********************************************************************/
/* chooseColor(initialColor, szPrompt, ...)
/*
/*  Invokes the host app's color picker to choose a color.
/*  Returns the chosen color, or -1 if an error occurred.
/*
/**********************************************************************/
EXTERNC int chooseColor(int initialColor, const char *szPrompt, ...);

/**********************************************************************/
/* setRegRoot(int hkey)
/*  Sets the current registry root key.
/*
/**********************************************************************/
EXTERNC int setRegRoot(HKEY hkey);

/**********************************************************************/
/* getRegRoot(int *hkey)
/*  Retrieves the current registry root key.
/*
/**********************************************************************/
EXTERNC int getRegRoot(HKEY *phkey);

/**********************************************************************/
/* setRegPath(lpsz szPath[, varargs]...)
/*  Sets the current registry path, with printf-style formatting.
/*  Returns ERROR_INVALID_DATA if expanded path string is too long;
/*  else returns ERROR_SUCCESS.
/*
/**********************************************************************/
EXTERNC int setRegPath(LPCSTR szPath,...);

/**********************************************************************/
/* getRegPath(lpsz szPath, int maxPathLen)
/*  Retrieves the current (not yet interpolated) registry path.
/*  Returns ERROR_INVALID_DATA if caller's buffer is too small;
/*  else returns ERROR_SUCCESS.
/*
/**********************************************************************/
EXTERNC int getRegPath(LPSTR szPath, int maxPathLen);

/**********************************************************************/
/* putRegInt(int iValue, lpsz szValueName[, varargs]...)
/*  Stores an integer value into the registry, with printf-style formatting
/*  for the value name.
/*
/**********************************************************************/
EXTERNC int putRegInt(int iValue, LPCSTR szValueName,...);

/**********************************************************************/
/* getRegInt(int *iValue, lpsz szValueName[, varargs]...)
/*  Gets an integer value from the registry, with printf-style formatting
/*  for the value name.
/*
/**********************************************************************/
EXTERNC int getRegInt(int *iValue, LPCSTR szValueName,...);

/**********************************************************************/
/* putRegString(lpsz szString, lpsz szValueName[, vargargs]...)
/*  Stores a string value into the registry, with printf-style formatting
/*  for the value name.
/*
/**********************************************************************/
EXTERNC int putRegString(LPCSTR szString, LPCSTR szValueName,...);

/**********************************************************************/
/* getRegString(lpsz szString, int iMaxlen, lpsz szValueName[, varargs]...)
/*  Gets a string value from the registry, with printf-style formatting
/*  for the value name.
/*
/**********************************************************************/
EXTERNC int getRegString(LPSTR szString, int iMaxlen, LPCSTR szValueName,...);

/**********************************************************************/
/* putRegData(void *data, int dataLen, lpsz szValueName[, vargargs]...)
/*  Stores a set of binary data into the registry, with printf-style
/*  formatting for the value name.
/*
/**********************************************************************/
EXTERNC int putRegData(const void *pData, int dataLen, LPCSTR szValueName,...);

/**********************************************************************/
/* getRegData(void *data, int dataLen, lpsz szValueName[, vargargs]...)
/*  Gets a set of binary data from the registry, with printf-style
/*  formatting for the value name.
/*
/**********************************************************************/
EXTERNC int getRegData(void *pData, int dataLen, LPCSTR szValueName,...);

/**********************************************************************/
/* enumRegSubKey(int index, lpsz szSubKey, int maxSubKeyLen)
/*  Gets name of n-th subkey under current registry key.
/*
/**********************************************************************/
EXTERNC int enumRegSubKey(int index, LPSTR szSubKey, int maxSubKeyLen);

/**********************************************************************/
/* enumRegValue(int index, lpsz szValueName, int maxValueNameLen,
/*              int *iType, int *cbData)
/*  Gets name, type, and data size of n-th value under current registry
/*  key.
/*
/**********************************************************************/
EXTERNC int enumRegValue(int index, LPSTR szValueName, int maxValueNameLen,
                    int *iType, int *cbData);

/**********************************************************************/
/* deleteRegSubKey(lpsz szSubKey[, varargs]...)
/*  Deletes specified subkey (and all its values) below the current
/*  key.  The subkey name is specified with printf-style formatting.
/*  The specified subkey must not have sub-subkeys (at least on NT;
/*  Win95 may allow deletion of an entire key subtree?).
/*
/**********************************************************************/
EXTERNC int deleteRegSubKey(LPCSTR szSubKey,...);

/**********************************************************************/
/* deleteRegValue(lpsz szValueName[, vargargs]...)
/*  Deletes specified value under the current key.  The value name is
/*  specified with printf-style formatting.
/*
/**********************************************************************/
EXTERNC int deleteRegValue(LPCSTR szValueName,...);


EXTERNC int shellExec(char* szVerb, char* szFilename, char* szParams, char* szDefDir);

EXTERNC int msgBox(UINT uType, LPCTSTR lpCaption, LPCTSTR lpFmt,...);


EXTERNC int fm_printf(char* lpFmt,...);

EXTERNC int Info(char* lpFmt,...);

EXTERNC int Warn(char* lpFmt,...);

EXTERNC int Error(char* lpFmt,...);

EXTERNC int ErrorOk(char* lpFmt,...);

EXTERNC int YesNo(char* lpFmt,...);

EXTERNC int YesNoCancel(char* lpFmt,...);



#ifdef FMDIB
    EXTERNC int startSetPixel (int n);
	EXTERNC int startSetPixelSS (int n, int factor);
    EXTERNC int endSetPixel (int n);
	EXTERNC int startSetPixelDC(HDC dc);
	EXTERNC int endSetPixelDC(HDC dc);
    EXTERNC int setPixel (int x, int y, UINT color);
	EXTERNC UINT getPixel (int x, int y);
    EXTERNC int setFill(int n, UINT color);
	EXTERNC int setPenWidth(int width);
	EXTERNC int setPenStyle(int e);
	EXTERNC int setLine(int start_x, int start_y, int end_x, int end_y, UINT color);
	EXTERNC int setLineAA(int start_x, int start_y, int end_x, int end_y, UINT color);
	EXTERNC int setRectFill(int left, int top, int right, int bottom, UINT color);
	EXTERNC int setRectFrame(int left, int top, int right, int bottom, UINT color);
	EXTERNC int setRectGradient(int left, int top, int right, int bottom, UINT color_TL, UINT color_BR, int horizontal);
	EXTERNC int setRadialGradient(int x0, int y0, int x1, int y1, int r, COLORREF c0, COLORREF c1, int nPart);
	EXTERNC int setEllipse(int left, int top, int right, int bottom, UINT color);
	EXTERNC int setEllipseFill(int left, int top, int right, int bottom, UINT color);
    EXTERNC HFONT setFont(int size, int angle, int bold, int italic, char * fontname);
	EXTERNC int setText(int x, int y, UINT color, int alignment, char * text);
	EXTERNC int setTextv(int x, int y, UINT color, int alignment, char * lpFmt, ...);