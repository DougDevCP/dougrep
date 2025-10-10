int updateProgress(int n, int d) {

	static int lastn = -1;
	static int lastd = -1;

	//if (fmc.doingProxy) return 0;

	if (d <= 0) d = 1;

	//No unnecessary updates
	if (n==lastn && d==lastd) return gResult;
	lastn = n;
	lastd = d;


#ifndef APP
    if (fmc.doingProxy) {
#endif

    #ifdef APP

		if (app.doProgress){

            int n1 = n;
            int d1 = d;

            clock_t timer = clock()-app.progressStartTime;
            //char timeDisplay[64];

            //if (app.progressTotal>0)
            {
                int min = ((app.progressCurrent-1)*app.progressTaskTotal+(app.progressTaskCurrent-1)) *100 / (app.progressTotal*app.progressTaskTotal);
                int max = ((app.progressCurrent-1)*app.progressTaskTotal+app.progressTaskCurrent) *100 / (app.progressTotal*app.progressTaskTotal);

                n1 = min + (n*(max-min))/d ;
                d1 = 100;
            }

            if (!app.progressDisplayed) {
                if (timer>1000) {

					if (app.hProgress==NULL) displayProgressWindow(1);

					if (app.hProgress!=NULL){
						HWND hLabel1 = GetDlgItem(app.hProgress, 1001);
	                    
						ShowWindow(app.hProgress, SW_SHOW);
						app.progressDisplayed = true;
						SendMessage(GetDlgItem(app.hProgress, IDC_PROGRESS1), PBM_SETPOS, (n1*100)/d1, 0);
						
						//Update Label
						InvalidateRect(hLabel1, NULL, TRUE );
						UpdateWindow(hLabel1);
					}
                }
           } else {
               if (app.hProgress!=NULL) SendMessage(GetDlgItem(app.hProgress, IDC_PROGRESS1), PBM_SETPOS, (n1*100)/d1, 0);
				   
               //Also use main window progress bar
               if (!fmc.doingProxy && app.progressDisplayed) 
				   //SendMessage(GetDlgItem(MyHDlg, IDC_PROGRESS1), PBM_SETPOS, (n*100)/d, 0);
				   setProgressMain((n*100)/d);
           }
            
           /*if (app.progressDisplayed && n>0) {
                HWND hLabel2 = GetDlgItem(app.hProgress, 1006); 
                //Update Time
                sprintf (timeDisplay,"Time: %.1f of %d seconds",timer/1000.0, (int)((timer*d/n)/1000.0) );
                SetWindowText(hLabel2, timeDisplay);
           }*/

		} else
    #endif
			//SendMessage(GetDlgItem(MyHDlg, IDC_PROGRESS1), PBM_SETPOS, (n*100)/d, 0);
			setProgressMain((n*100)/d);


			gParams->ctl[CTL_PROGRESS].val = (n*100)/d; //update cached values for reading progress bar value with ctl()
			fmc.pre_ctl[CTL_PROGRESS] = (n*100)/d; //update cached values for reading progress bar value with ctl()
			return testAbort();

#ifndef APP
    }
    else {
        PIUpdateProgress(n, d);
        if (TestAbort ())
        {
            gResult = userCanceledErr;
            //longjmp()
        }
    }
#endif

    return gResult;

//#else

//    return true;

//#endif

} //fm_updateProgress




int updatePreview(int n) {
    //n is ignored for now ,,, it will be the id of the preview
    //control to be updated.
    if (fmc.doingProxy) {
        //update progress bar for proxy....
        // thread is tied up computing the filter, so
        // WM_PAINT doesn't get processed in message loop.
        UpdateProxy(MyHDlg, FALSE);
        // So update the proxy immediately!
        ShowOutputBuffer(globals, MyHDlg, FALSE /*fDragging*/);
        return 1;  //doingProxy
    }
    return 0;   //not doingProxy, so no update
} //fm_updatePreview

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
char *getFilterInstallDir(void)
{
    int i;
    DWORD ret = GetModuleFileName((HMODULE)hDllInstance,
                                  fmc.filterInstallDir,
                                  sizeof(fmc.filterInstallDir));
    // Locate the final '\', if any...
    for (i = ret - 1; i >= 0 && fmc.filterInstallDir[i] != '\\'; --i)
        ;
    // Truncate string after final '\' (or set to null string if none).
    fmc.filterInstallDir[i+1] = '\0';
    return fmc.filterInstallDir;
} /*fm_getFilterInstallDir*/

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
char *appendEllipsis(const char *s)
{
    THREAD_LOCAL static char f[512];        //Warning: static!!!!

    strncpy(f, s, sizeof(f)-4); //leave room for ellipsis
    f[sizeof(f)-4] = '\0';      //ensure f is null-terminated.
    return strcat(f, "...");
} /*fm_appendEllipsis*/

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
char *stripEllipsis(const char *s)
{
    THREAD_LOCAL static char f[512];        //Warning: static!!!! NOT THREAD-SAFE!!
    int n = (int)strlen(s);
    if (n >= 3 && strcmp(&s[n-3], "...") == 0) {
        // strip ellipsis from end of s
        n -= 3;
    }
    n = min(n, sizeof(f)-1);    //truncate to fit in f if necessary
    strncpy(f, s, n);           //copy n chars to f
    f[n] = '\0';                //add terminating null
    return f;
} /*fm_stripEllipsis*/

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