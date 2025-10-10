

/**********************************************************************/
/* setCtlText(n, "text")
/*
/*  Sets the text label or content for control n to the given text string.
/*
/**********************************************************************/

EXTERNC int setCtlText(int n, char* iText);

/**********************************************************************/
/* setCtlTextv(n, "format",...)
/*
/*  Sets the text label or content for control n to the expanded
/*  printf-style format string.
/*
/**********************************************************************/
EXTERNC int setCtlTextv(int n, char* iFmt,...);


/**********************************************************************/
/* enableToolTipBalloon(enable)
/*
/*  Enables or disables the balloon shape of all tool tips, based on
/*  the value of boolean value "enable".
/*
/**********************************************************************/

EXTERNC int enableToolTipBalloon(int enable);

// NOTE: Shouldn't we have a version of SetCtlToolTip with printf-style
//       formatting?  E.g., setCtlToolTipv(n, s, "format",...)

/**********************************************************************/
/* setCtlToolTip(n, "Tool Tip Text", s)
/*
/*  Sets the tool tip text for control n to the specified string, with
/*  style s (TTF_CENTERTIP, TTF_RTLREADING, TTF_TRACK, TTF_ABSOLUTE,
/*  TTF_TRANSPARENT), or deletes the tool tip for this control if the
/*  text argument is NULL or "".
/*
/**********************************************************************/
EXTERNC int setCtlToolTip(int n, char * iText, int s);


EXTERNC int setToolTipDelay(int m, int iTime);



EXTERNC char * getCtlImage(int n);


/**********************************************************************/
/* setCtlImage(n, "Image Name", 'X')
/*
/*  Sets the image (if any) associated with control n to the specified
/*  filename or embedded resource name, with image type 'X', where
/*  'X' is one of the following character constants:
/*
/*      'B' -   bitmap file (.bmp)
/*      'W' -   Windows (old-style) metafile (.wmf)
/*      'E' -   enhanced metafile (.emf)
/*      'I' -   icon file (.ico)
/*      'C' -   cursor file (.cur)
/*      'J' -   JPEG file (.jpg)
/*      'G' -   GIF file (.gif)
/*      'M' -   MIG (mouse-ivo graphics) file (.mig)
/*       0  -   unspecified file type
/*
/**********************************************************************/
EXTERNC int setCtlImage(int n, char * iName, int iType);

/**********************************************************************/
/* setCtlTicFreq(n, m)
/*
/*  Sets the frequency with which tick marks are displayed for slider
/*  control n. For example, if the frequency is set to 2, a tick mark
/*  is displayed for every other increment in the slider�s range. The
/*  default setting for the frequency is 1 (that is, every increment 
/*  in the range is associated with a tick mark).
/*
/**********************************************************************/
EXTERNC int setCtlTicFreq(int n, int m);

/**********************************************************************/
/* setWindowTheme(hwnd, pszSubAppName, pszSubIdList)
/*
/*  Sets the Visual Theme for a given window to the specified theme.
/*
/*  Calls the following XP API function (if it exists):
/*
/*  THEMEAPI SetWindowTheme(HWND hwnd, LPCWSTR pszSubAppName, 
/*    LPCWSTR pszSubIdList);
/*
/*  N.B.  The 2nd and 3rd args must be UNICODE strings!
/**********************************************************************/

//Internal Theme Variable -> theme off by default -> replaced by gParams->gDialogTheme
//int DefaultTheme=0;

// EXTERNC int drawThemePart(int n, RECT * rc, LPCWSTR topic, int part, int state);


/**********************************************************************/
/* setDefaultWindowTheme(hwnd)
/*
/*  Sets the Visual Theme for a given window to the default visual
/*  theme, which for now means to turn OFF any XP Visual Styles.
/*
/**********************************************************************/
// EXTERNC HRESULT setDefaultWindowTheme(HWND hwnd);


/**********************************************************************/
/* setCtlTheme(n, pszSubAppName, pszSubIdList)
/*
/*  Sets the Visual Theme for control n to the specified theme.
/*
/**********************************************************************/
//int setCtlTheme(int n, int pszSubAppName, int pszSubIdList) //Removed  infront of setCtlTheme



//New Styles Functions


EXTERNC int getAppTheme (void);

// EXTERNC int setDialogTheme (int state);

// EXTERNC int setCtlTheme(int n, int state);

//int setDialogEvent (int state);
//int clearDialogEvent (int state);



EXTERNC int createFont(int i, int size, int bold, int italic, char * fontname);

EXTERNC int deleteFont (int i);

EXTERNC int setCtlFont(int n, int i);



/**********************************************************************/
/* setCtlFontColor(n, color)
/*
/*  Sets the text color for control n to the specified RGB-triple value.
/*
/**********************************************************************/
EXTERNC int setCtlFontColor(int n, int color);

EXTERNC int getCtlFontColor(int n);

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
EXTERNC int setCtlColor(int n, int color);


EXTERNC int setCtlSysColor(int n, int con);

EXTERNC int setCtlFontSysColor(int n, int con);


/**********************************************************************/
/* getCtlColor(n)
/*
/*  Returns the current background color of control n as an RGB-triple,
/*  or -1 if n is not a valid control.
/*
/**********************************************************************/
EXTERNC int getCtlColor(int n);


EXTERNC int setCtlDefVal(int n, int defval);
EXTERNC int getCtlDefVal(int n);

/**********************************************************************/
/* setCtlVal(n, val)
/*
/*  Sets the value of control n to val.
/*
/*  Returns the previous value of control n, or -1 if n is not a
/*  valid control.
/*
/**********************************************************************/
EXTERNC int setCtlVal(int n, int val);


EXTERNC int setCtlDivisor(int n, int div);

EXTERNC int getCtlDivisor(int n);

EXTERNC int setCtlGamma(int n, int gamma);

EXTERNC int setCtlProperties(int n, int props);

EXTERNC int clearCtlProperties(int n, int props);

EXTERNC int setCtlRange(int n, int lo, int hi);

/**********************************************************************/
/* setCtlLineSize(n, a)
/*
/*  Sets the small step ("line size") value for control n to a, and
/*  returns the previous small step value.
/*
/**********************************************************************/
EXTERNC int setCtlLineSize(int n, int a);

/**********************************************************************/
/* setCtlPageSize(n, b)
/*
/*  Sets the large step ("page size") value for control n to b, and
/*  returns the previous large step value.
/*
/**********************************************************************/
EXTERNC int setCtlPageSize(int n, int b);

EXTERNC int setCtlThumbSize(int n, int a);


/*************************************************************/
//
//  A couple of local helper functions...
//
/*************************************************************/

EXTERNC void computeBuddy1Pos(int n, int x, int y, int w, int h, int xx, int yy, RECT *pr);

EXTERNC void computeBuddy2Pos(int n, int x, int y, int w, int h, RECT *pr);



/**********************************************************************/
/* setCtlPos(n, x, y, w, h)
/*
/*  Sets the position of the upper lefthand corner of control n to 
/*  coordinates (x,y) within the client area of the dialog box; sets 
/*  the width of the control to w and the height to h.  All measurements 
/*  are in dialog box units (DBUs).  To leave a particular parameter
/*  unchanged, specify a value of -1 for that parameter.
/*
/**********************************************************************/
EXTERNC int setCtlPos(int n, int x, int y, int w, int h);

EXTERNC int getCtlDefPos(int n, int item);
EXTERNC int setCtlDefPos(int n, int x, int y, int width, int height);

EXTERNC int setCtlPixelPos(int n, int x, int y, int w, int h);
EXTERNC int getCtlPixelPos(int n, int w);

EXTERNC int setCtlEditSize(int n, int w, int h);
EXTERNC int setEditBoxUpdate(int ms);

EXTERNC int getCtlRange(int n, int w);



/**********************************************************************/
/* setCtlAction(n, a)
/*
/*  Sets the default action for control n to a, where a is CA_NONE,
/*  CA_CANCEL, CA_APPLY, CA_PREVIEW, CA_EDIT, CA_ABOUT, or CA_RESET.
/*  Returns the previous default action for the control (or 0 if the
/*  control index is invalid).
/*
/*  The default action is the action that will be taken if the default
/*  OnCtl handler is invoked for control n.  The meaning of each action
/*  is:
/*
/*  CA_NONE     No action.  This is the default action for radio buttons.
/*
/*  CA_CANCEL   FM exits, leaving the original source image unaltered.
/*              This is the default action for the Cancel button,
/*
/*  CA_APPLY    The filter is applied to the original source image,
/*              and FM exits.  This is the default action for the OK
/*              button.
/*
/*  CA_PREVIEW  The filter is applied to the proxy image, and all previews
/*              are updated.  This is the default action for most user
/*              controls, including the STANDARD, SCROLLBAR, TRACKBAR,
/*              PUSHBUTTON, and CHECKBOX controls.
/*
/*  CA_EDIT     FM enters or leaves Edit Mode.  This is the default
/*              action for the Edit control.
/*
/*  CA_ABOUT    FM displays the ABOUT dialog box.