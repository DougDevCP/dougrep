     *      #define callLibraryDoubleFast   ((dfn1v)callLibraryFast)
	 */

EXTERNC int callLib(int fn, ...);


EXTERNC int __cdecl getCpuReg(int nr);

EXTERNC void __cdecl finit(void);

EXTERNC int __cdecl fstsw(void);

EXTERNC int __cdecl fstcw(void);

EXTERNC void __cdecl fldcw(int cw);


/****************** End DLL Access Routines *************************/




EXTERNC int setPreviewDragMode (int mode);

EXTERNC int linearInterpolate (int v1,int v2, double x);

EXTERNC int cosineInterpolate (int v1,int v2, double x);

EXTERNC int cubicInterpolate (int v0,int v1,int v2,int v3,double x);

EXTERNC int hermiteInterpolate (int T1,int P1,int P2,int T2, double s);

EXTERNC int getImageTitle (char * n);

EXTERNC int getEXIFSize ();
int getEXIFData (int * buffer);

EXTERNC int destroyMenu(INT_PTR hMenu);

EXTERNC int insertMenuItem (INT_PTR hMenu, int uItem, char* itemName, int fState, INT_PTR subMenu);
EXTERNC int insertMenuItemEx (INT_PTR hMenu, int uItem, char* itemName, int fState, int bullet, INT_PTR subMenu);

EXTERNC int trackPopupMenu (INT_PTR hMenu, int type, int x, int y, int style);

EXTERNC int setMenu (INT_PTR hMenu);

EXTERNC int getCtlFocus (void);

EXTERNC int setCtlFocus (int n);

EXTERNC int checkCtlFocus (int n);

EXTERNC int checkDialogFocus (void);


EXTERNC INT_PTR findFirstFile (LPCTSTR lpFileName, LPSTR foundItem, int *dwFileAttributes);

EXTERNC int findNextFile (INT_PTR hFindFile, LPSTR foundItem, int *dwFileAttributes);

EXTERNC BOOL findClose(INT_PTR hFindFile);



EXTERNC int getLocaleInfo(LCID Locale,
                            LCTYPE LCType,
                            LPSTR  lpLCData,
                            int    cchData);

EXTERNC int getSpecialFolder (int val, char* str);

EXTERNC int setCtlStyle(int n, int flags);

EXTERNC int clearCtlStyle(int n, int flags);

EXTERNC int setCtlStyleEx(int n, int flags);

EXTERNC int clearCtlStyleEx(int n, int flags);

EXTERNC int setCtlBuddyStyle(int n, int buddy, int flags);

EXTERNC int clearCtlBuddyStyle(int n, int buddy, int flags);

EXTERNC int setCtlBuddyStyleEx(int n, int buddy, int flags);

EXTERNC int clearCtlBuddyStyleEx(int n, int buddy, int flags);

EXTERNC int setCtlBuddyFontColor(int n, int color);
EXTERNC int setCtlBuddyColor(int n, int color);
EXTERNC int setCtlBuddyMaxSize(int n, int maxwidth, int maxheight);

EXTERNC int setCtlEditColor(int n, int color);
EXTERNC int setCtlEditTextColor(int n, int color);

EXTERNC int getSysColor(int n);

EXTERNC int getSysMem(int n);

EXTERNC int checkArrayAlloc(int nr);

EXTERNC INT_PTR getArrayAddress(int nr);

EXTERNC INT_PTR getBufferAddress(int nr);


EXTERNC int setCtlScripting (int n, int state);

EXTERNC int checkScriptVal(int n);

EXTERNC int enableScriptVal(int n, int state);

EXTERNC int getScriptVal(int n);

EXTERNC int setScriptVal(int n, int val);

EXTERNC int cnvX(int k, int off, int d,
                   INT_PTR pGetf, int x, int y, int z);

EXTERNC int cnvY(int k, int off, int d,
                   INT_PTR pGetf, int x, int y, int z);
                   
EXTERNC int fm_abort(void);

EXTERNC int testAbort(void);

EXTERNC int updatePreview(int n);

EXTERNC int updateProgress(int n, int d);


/**********************************************************************/
/* getFilterInstallDir()
/*
/*  Returns a string containing the full path of the directory from
/*  which this filter was loaded (which is presumably the installation
/*  directory).
/*
/*  As a side effect, this function also copies the path name into
/*  the predefined string filterInstallDir[256].
/*
/**********************************************************************/
EXTERNC char *getFilterInstallDir(void);

/**********************************************************************/
/* appendEllipsis(s)
/*
/*  Appends an ellipsis (...) to a string.
/*
/*  Caution: The string returned by appendEllipsis() resides in static
/*           storage and will be overwritten by the next call to
/*           appendEllipsis().  If you need the appended string to
/*           persist, copy it to your own storage.
/*
/**********************************************************************/
EXTERNC char *appendEllipsis(const char *s);

/**********************************************************************/
/* stripEllipsis(s)
/*
/*  Strips a trailing ellipsis (if any) from a string.
/*
/*  Caution: The string returned by stripEllipsis() resides in static
/*           storage and will be overwritten by the next call to
/*           stripEllipsis().  If you need the stripped string to
/*           persist, copy it to your own storage.
/*
/**********************************************************************/
EXTERNC char *stripEllipsis(const char *s);

/**********************************************************************/
/* formatString(s)
/*
/*  Formats a string by interpolating the following special codes:
/*
/*       !A = Author (filterAuthorText)
/*       !a = About (filterAboutText)
/*       !C = Category (filterCategoryText)
/*       !c = Copyright (filterCopyrightText)
/*       !D = Description (filterDescriptionText)
/*   **  !d = Document title (from propTitle (titl)) -> documentTitleText
/*   XX  !d = current date (oops, used above)
/*       !F = Filename (filterFilenameText)
/*       !f = Filter case (filterCaseText)
/*       !H = Host (filterHostText)
/*       !h = Image height (imageHeight)
/*   **  !I = filterInstallDir
/*   **  !i = filterImageTitle (same as documentTitleText ??)
/*       !M = Image Mode (filterImageModeText)
/*       !m = Image Mode (numeric, from imageMode)
/*       !O = Organization (filterOrganizationText)
/*   **  !S = Serial string (hostSerialString)
/*   **  !s = Binary serial number (hostSerialNumber)
/*       !T = Title (filterTitleText)
/*       !t = Title with any trailing ellipsis stripped
/*   XX  !t = current time (oops, used above)
/*       !U = URL (filterURLText)
/*       !V = Version (filterVersionText)
/*   **  !v = FilterMeister Version string???
/*       !w = Image width (imageWidth)
/*   **  !Y - current year (e.g. 2003) -- for use in Copyright
/*       !z = Proxy zoom factor (zoomFactor)
/*       !! = !
/*
/* ** - Not yet implemented.
/*
/*  Also translates many of the latest HTML entity strings to
/*  their corresponding ISO codes (or sometimes fakes the
/*  translation by using low-valued ASCII codes or a combination
/*  of ASCII characters).  An HTML entity string has the general
/*  format '&entity;'.  It must begin with the ampersand character (&)
/*  and end with a semicolon (;). The string 'entity' between the
/*  & and the ; must be a sequence of two or more ASCII lowercase
/*  or uppercase letters (the string is case sensitive).
/*
/*  HTML numeric character references (both decimal and hexadecimal)
/*  are also recognized and translated.  The decimal format is
/*  '&#D;', where D is a decimal number which refers to the ISO 10646 
/*  decimal character number D.  The hexadecimal syntax is '&#xH;' or 
/*  '&#XH;,, where H is a hexadecimal number which refers to the ISO 
/*  10646 hexadecimal character number H.  Hexadecimal numbers in 
/*  numeric character references are case-insensitive. 
/*
/*  Here are some examples of numeric character references:
/*
/*	&#229;		(in decimal) represents the letter "a" with a small circle 
/*				above it (used, for example, in Norwegian). 
/*	&#xE5;		(in hexadecimal) represents the same character. 
/*	&#Xe5;		(in hexadecimal) represents the same character as well. 
/*	&#1048;		(in decimal) represents the Cyrillic capital letter "I". 
/*	&#x6C34;	(in hexadecimal) represents the Chinese character for water. 
/*
/*  As examples of HTML character entity references, FM will translate the 
/*  following HTML entities as indicated:
/*
/*      Entity  Translation Meaning
/*      ------  ----------- ------------------------------------
/*		&quot;	    "       double quote
/*		&amp;	    &       ampersand
/*		&apos;	    '       apostrophe
/*		&lt;	    <       less than
/*		&gt;	    >       greater than
/*		&nbsp;		        non-breaking space
/*		&iexcl;		�       inverted exclamation mark
/*      &iquest;    �       inverted question mark
/*		&cent;		�       cent sign
/*		&pound;		�       pound sign
/*		&copy;		�       copyright
/*      &reg;       �       registered trademark
/*		&trade;     tm      trademark
/*		&yen;		�       Yen sign
/*      &euro;      �       Euro sign
/*      
/*  For a complete list of all HTML entities currently translated by 
/*  FM, see the code below.
/*
/*  Note that, in order to represent a bare ampersand (&), you can
/*  code the entity string '&amp;'.  Since this is rather cumbersome,
/*  FM will also recognize and translate two successive ampersands
/*  (&&) into a single ampersand (&) in the formatted string.
/*  Furthermore, any occurrence of an ampersand (&) that does not
/*  initiate a recognized HTML entity will also be translated as
/*  a bare ampersand (&).
/*
/*  For example, the following strings:
/*
/*      "This is an ampersand:'&amp;'; &amp; so is this: &amp;."
/*      "This is an ampersand:'&&'; && so is this: &&."
/*      "This is an ampersand:'&'; & so is this: &."
/*
/*  will all be formatted as:
/*
/*      "This is an ampersand:'&'; & so is this: &."
/*
/*  For possible future implementation:
/*
/*       !<n> = value of ctl(<n>) where <n> is a decimal number
/*              (e.g., for updating a static text control?)
/*
/*  Note: The formatting is nonrecursive, to prevent inadvertent
/*        infinite loops.
/*
/*  Caution: The string returned by formatString() has a maximum
/*           length of 1024 chars (not including the terminating
/*           NUL), and will be truncated if necessary.  It resides 
/*           in (thread-local) static storage and will be overwritten
/*           by the next call to formatString() in the current thread.
/*           If you need the formatted string to persist, copy it to 
/*           your own storage.
/*
/**********************************************************************/

EXTERNC char *formatString(const char *s);


EXTERNC char *getCtlText(int n);


EXTERNC void lockCtlUpdate (int state);


EXTERNC int setCtlLabel(int n, char * iText);