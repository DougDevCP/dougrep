/**********************************************************************/
/* REGION createRectRgn(xLeft, yTop, xRight, yBottom)
/*  Creates a simple rectangular region, with coordinates given in DBUs
/*  relative to the upper-left corner of the dialog box.  <left> is the
/*  x-coordinate of the left edge of the rectangle; <right> is the
/*  x-coordinate of the right edge.  <top> is the y-coordinate of the
/*  top edge of the rectangle; <bottom> is the y-coordinate of the
/*  bottom edge.  The <left> and <top> coordinates are inclusive (i.e.,
/*  pixels at these coordinates are included in the rectangle).  The
/*  <right> and <bottom> coordinates are exclusive (i.e., pixels up to
/*  but *not* including these coordinates are part of the rectangle).
/*
/**********************************************************************/
INT_PTR createRectRgn(int xLeft, int yTop, int xRight, int yBottom)
{
    Map4DBUsToPixels(&xLeft, &yTop, &xRight, &yBottom);
    return (INT_PTR)CreateRectRgn(xLeft, yTop, xRight, yBottom);
} /*fm_createRectRgn*/

/**********************************************************************/
/* REGION createRoundRectRgn(xLeft, yTop, xRight, yBottom, eWidth, eHeight)
/*  Creates a rectangular region with rounded corners.  <left>, <top>,
/*  <right>, and <bottom> are the same as for 'RECT' and describe the
/*  unrounded rectangle.  <width> and <height> specify the width and
/*  height of an ellipse; quarter sections of this ellipse are used to
/*  form the rounded corners of the rectangle.  All measurements are
/*  in DBUs.
/*
/**********************************************************************/
INT_PTR createRoundRectRgn(int xLeft, int yTop, int xRight, int yBottom,
                                 int eWidth, int eHeight)
{
    Map4DBUsToPixels(&xLeft, &yTop, &xRight, &yBottom);
    Map2DBUsToPixels(&eWidth, &eHeight);
    return (INT_PTR)CreateRoundRectRgn(xLeft, yTop, xRight, yBottom,
                                   eWidth, eHeight);
} /*fm_createRoundRectRgn*/

/**********************************************************************/
/* REGION createCircularRgn(xLeft, yTop, iDiameter)
/*  Creates a simple circular region.  <left> and <top> give the x- and
/*  y-coordinates of the upper-left corner of the bounding box for the
/*  circle.  <diameter> gives the diameter of the circle.  All measurements
/*  are in DBUs.
/*
/**********************************************************************/
INT_PTR createCircularRgn(int xLeft, int yTop, int iDiameter)
{
    int xRight = xLeft + iDiameter;
    int yBottom = yTop + iDiameter;
    Map4DBUsToPixels(&xLeft, &yTop, &xRight, &yBottom);
    return (INT_PTR)CreateEllipticRgn(xLeft, yTop, xRight, yBottom);
} /*fm_createCircularRgn*/

/**********************************************************************/
/* REGION createEllipticRgn(xLeft, yTop, xRight, yBottom)
/*  Creates an elliptical region.  <left>, <top>, <right>, and <bottom>
/*  define the coordinates of the bounding box for the ellipse.  All
/*  measurements are in DBUs.
/*
/**********************************************************************/
INT_PTR createEllipticRgn(int xLeft, int yTop, int xRight, int yBottom)
{
    Map4DBUsToPixels(&xLeft, &yTop, &xRight, &yBottom);
    return (INT_PTR)CreateEllipticRgn(xLeft, yTop, xRight, yBottom);
} /*fm_createEllipticRgn*/

/**********************************************************************/
/* REGION createPolyRgn(fillMode, nPoints, x1, y1, x2, y2, x3, y3, ...)
/*  Creates an arbitrary polygon from a list of pairs of vertices.  This
/*  list must consist of at least 3 pairs of x- and y-coordinates (in
/*  DBUs).  The total number of coordinates must be even.  The polygon is
/*  defined by drawing an imaginary line from the first vertex to the
/*  second, then to each subsequent vertex, and finally back to the
/*  original vertex.  The region is then defined by "filling" this polygon
/*  according to the fill mode.  The fill mode must be specified as either
/*  'ALTERNATE' (1) or 'WINDING' (2).
/*
/**********************************************************************/
INT_PTR createPolyRgn(int fillMode, int nPoints, int x1, int y1,
                            int x2, int y2, int x3, int y3, ...)
{
    int i;

    //Map 1st three mandatory points to pixels.
    Map4DBUsToPixels(&x1, &y1, &x2, &y2);
    Map2DBUsToPixels(&x3, &y3);

    //Map any remaining optional points.
    //Following is non-portable!!!! (Depends on __cdecl calling
    //convention to push args right-to-left as contiguous LONGs
    //on the stack.)
    for (i = 3; i < nPoints; i++) {
        Map2DBUsToPixels(&(&x1)[2*i], &(&y1)[2*i]);
    }

    return (INT_PTR)CreatePolygonRgn(
            //Following is non-portable!!!! (Depends on __cdecl calling
            //convention to push args right-to-left as contiguous LONGs
            //on the stack.)
            (CONST POINT *)&x1, // pointer to array of points 
            nPoints,    // number of points in array 
            fillMode    // polygon-filling mode 
           );
} /*fm_createPolyRgn*/  

/**********************************************************************/
/* REGION createDialogRgn();
/*  Is a predefined rectangular region consisting of the default dialog
/*  box itself.
/*
/**********************************************************************/
int createDialogRgn(void)
{
    // Should emit a warning or error !!!!
    return 0;   //for now...
} /*fm_createDialogRgn*/

/**********************************************************************/
/* REGION createTitlebarRgn();
/*  Is a predefined rectangular region defining the area of the dialog
/*  title bar (if present).
/*
/**********************************************************************/
int createTitlebarRgn(void)
{
    // Should emit a warning or error !!!!
    return 0;   //for now...
} /*fm_createTitlebarRgn*/

/**********************************************************************/
/* REGION createMenubarRgn();
/*  Is a predefined rectangular region defining the area of the dialog
/*  menu bar (if any).
/*
/**********************************************************************/
int createMenubarRgn(void)
{
    // Should emit a warning or error !!!!
    return 0;   //for now...
} /*fm_createMenubarRgn*/

/**********************************************************************/
/* REGION createClientRgn();
/*  Is a predefined rectanglar region defining the client area of the
/*  dialog box.
/*
/**********************************************************************/
int createClientRgn(void)
{
    // Should emit a warning or error !!!!
    return 0;   //for now...
} /*fm_createClientRgn*/

/**********************************************************************/
/* REGION createStatusbarRgn();
/*  Is a predefined rectangular region defining the area of the dialog
/*  status bar (if any).
/*
/**********************************************************************/
int createStatusbarRgn(void)
{
    // Should emit a warning or error !!!!
    return 0;   //for now...
} /*fm_createStatusbarRgn*/

/**********************************************************************/
/* BOOL playSoundWave(szWaveFile);
/*  Plays the specified wave file asynchronously.  Looks first for
/*  an embedded WAVE resource with the specified name; else searches
/*  the usual paths for the specified wave file.
/*
/**********************************************************************/
BOOL playSoundWave(INT_PTR szWaveFile)
{
    if (PlaySound((const char *)szWaveFile, (HMODULE)hDllInstance,
                  SND_ASYNC | SND_RESOURCE | SND_NODEFAULT)) {
        //played from a WAVE resource
        return TRUE;
    }
    else if (szWaveFile != 0) {
        //try to locate the original wave file...
        return PlaySound(FmSearchFile((const char *)szWaveFile), NULL,
                         SND_ASYNC | SND_FILENAME);
    }
    return TRUE;
} /*fm_playSoundWave*/

/**********************************************************************/
/* BOOL playSoundWaveLoop(szWaveFile);
/*  Plays the specified wave file repeatedly.  Looks first for
/*  an embedded WAVE resource with the specified name; else searches
/*  the usual paths for the specified wave file.
/*
/**********************************************************************/
BOOL playSoundWaveLoop(INT_PTR szWaveFile)
{
    if (PlaySound((const char *)szWaveFile, (HMODULE)hDllInstance,
        SND_LOOP | SND_ASYNC | SND_RESOURCE | SND_NODEFAULT)) {
        //played from a WAVE resource
        return TRUE;
    }
    else if (szWaveFile != 0) {
        //try to locate the original wave file...
        return PlaySound(FmSearchFile((const char *)szWaveFile), NULL,
                         SND_LOOP | SND_ASYNC | SND_FILENAME);
    }
    return TRUE;
} /*fm_playSoundWaveLoop*/

/**********************************************************************/
/* BOOL playSoundWaveSync(szWaveFile);
/*  Plays the specified wave file synchronously.  Looks first for
/*  an embedded WAVE resource with the specified name; else searches
/*  the usual paths for the specified wave file.
/*
/**********************************************************************/
BOOL playSoundWaveSync(INT_PTR szWaveFile)
{
    if (PlaySound((const char *)szWaveFile, (HMODULE)hDllInstance,
                  SND_SYNC | SND_RESOURCE | SND_NODEFAULT)) {
        //played from a WAVE resource
        return TRUE;
    }
    else if (szWaveFile != 0) {
        //try to locate the original wave file...
        return PlaySound(FmSearchFile((const char *)szWaveFile), NULL,
                         SND_SYNC | SND_FILENAME);
    }
    return TRUE;
} /*fm_playSoundWaveSync*/

int fm_sleep(int msecs)
{
    // interface to the __stdcall Sleep service
    Sleep(msecs);
    return 0;
} /*fm_sleep*/

int fm_RGB(int r, int g, int b) {
    return ((b & 0xff) << 8 | (g & 0xff)) << 8 | (r & 0xff);
} /*fm_RGB*/

int RGBA(int r, int g, int b, int a) {
    return (((a & 0xff) << 8 | (b & 0xff)) << 8 | (g & 0xff)) << 8 | (r & 0xff);
} /*fm_RGBA*/

int Rval(int rgba) {
    return GetRValue(rgba);
} /*fm_Rval*/

int Gval(int rgba) {
    return GetGValue(rgba);
} /*fm_Gval*/

int Bval(int rgba) {
    return GetBValue(rgba);
} /*fm_Bval*/

int Aval(int rgba) {
    return (BYTE)(rgba >> 24);
} /*fm_Aval*/

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
int getOpenFileName( int flags,
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