/*
/*  CA_RESET    Resets all user controls to their initial values.
/*
/**********************************************************************/
EXTERNC int setCtlAction(int n, int a);

/**********************************************************************/
/* enableCtl(n, level)
/*
/*  Determines whether control n is visible and whether it is enabled
/*  by specifying level = 0 (invisible and disabled), 1 (visible but
/*  disabled), or 3 (visible and enabled).  Returns the previous enable
/*  level, or 0 if n is not a valid control index.
/*
/*  NOTE: Due to an early documentation error, level=-1 is considered
/*        equivalent to level=3.  However, other bits in level are
/*        reserved for future use, and should not be randomly set!
/*
/**********************************************************************/
EXTERNC void setEnableMode(int state);
EXTERNC int enableCtl(int n, int level);

/**********************************************************************/
/* enableCtl(n) support function
/*
/*  Determines whether a control should be visible, disabled or 
/*	invisible and sets it as such. Returns 0.
/*
/**********************************************************************/
EXTERNC int sub_enableCtl(int n);

/**********************************************************************/
/* deleteCtl(n)
/*
/*  Deletes control n.
/*
/**********************************************************************/
EXTERNC int deleteCtl(int n);

/**********************************************************************/
/* createPredefinedCtls()
/*
/*  Creates the pre-defined user controls.
/*
/**********************************************************************/
EXTERNC void createPredefinedCtls(void);

/**********************************************************************/
/* resetAllCtls()
/*
/*  Resets all controls (deletes them).
/*
/**********************************************************************/
EXTERNC int resetAllCtls(void);

/**********************************************************************/
/* createCtl(n, c, t, x, y, w, h, s, sx, p, e)
/*
/*  Dynamically creates a control with index n, class c, text t,
/*  coordinates (x,y), width w, height h, style s, extended style sx,
/*  properties p, and enable level e.
/*  All measurements are in DBUs.
/*  For x, y, w, and h, a value of -1 means use the default value.
/*
/**********************************************************************/
EXTERNC int createCtl(int n, int c, char * t, int x, int y, int w, int h,
                        int s, int sx, int p, int e);

/**********************************************************************/
/* initCtl(n)
/*
/*  Initializes control n to default values.
/*
/**********************************************************************/
EXTERNC int initCtl(int n);


/////// BUGNOTE:
/////// Should not use negative x, y coords as flags, since they
/////// may legitimately be negative; e.g., in a multi-monitor
/////// environment.  !!!!!!! ?????? !!!!!!

/**********************************************************************/
/*  setDialogPos(fAbs, x, y, w, h)
/*
/*  Sets the position and size of the dialog window.  If fAbs is true,
/*  x and y are absolute screen coordinates; otherwise, x and y are
/*  relative to the upper-left corner of the client area in the host
/*  application's main window.  If x and y are set to -1, the dialog
/*  window will be centered within the host client area or the working
/*  area of the entire screen, depending on whether fAbs is false or
/*  true, respectively.  Otherwise, if either x or y is negative, the
/*  position of the dialog will not be changed. If w or h is negative,
/*  the size of the dialog window will not be changed.
/*
/*  All measurements are in dialog box units (DBUs).
/*
/**********************************************************************/
EXTERNC int setDialogPos(int fAbs, int x, int y, int w, int h);


/**********************************************************************/
/* setDialogMaxMin()
/*  Set the minum and maximum size of the dialog
/*
/**********************************************************************/
EXTERNC int setDialogMinMax(int mintrackX, int mintrackY, int maxtrackX, int maxtrackY);



EXTERNC int getDialogPos(int w, int t);


/**********************************************************************/
/* getDialogWidth()
/*  Gets the width of the client area of the dialog.
/*
/**********************************************************************/
EXTERNC int getDialogWidth(void);

/**********************************************************************/
/* getDialogHeight()
/*  Gets the height of the client area of the dialog.
/*
/**********************************************************************/
EXTERNC int getDialogHeight(void);



/**********************************************************************/
/* setDialogStyle(TitleBar |...)
/*  Sets various styles of the dialog.
/*
/**********************************************************************/
EXTERNC int setDialogStyle(int flags);

/**********************************************************************/
/* clearDialogStyle(TitleBar |...)
/*  Clears various styles of the dialog.
/*
/**********************************************************************/
EXTERNC int clearDialogStyle(int flags);

// !!!! Should recalc Edit-mode dialog size when TOOLWINDOW is set/cleared
// !!!! and when dialog is resized!!!!


/**********************************************************************/
/* setDialogStyleEx(ToolWindow |...)
/*  Sets various extended styles of the dialog.
/*
/**********************************************************************/
EXTERNC int setDialogStyleEx(int flags);

/**********************************************************************/
/* clearDialogStyleEx(ToolWindow |...)
/*  Clears various extended styles of the dialog.
/*
/**********************************************************************/
EXTERNC int clearDialogStyleEx(int flags);

/**********************************************************************/
/* setDialogText("title")
/*  Sets the caption in the title bar.
/*
/**********************************************************************/
EXTERNC int setDialogText(char * title);

/**********************************************************************/
/* setDialogTextv("format", ...)
/*  Sets the caption in the title bar with printf-style formatting.
/*
/**********************************************************************/
EXTERNC int setDialogTextv(char * iFmt,...);

/**********************************************************************/
/* setDialogGradient(color1,color2,direction)
/*  Fills the background of the client area of the dialog box with
/*  a vertical (direction=0) or horizontal (direction=1) linear gradient.
/*
/**********************************************************************/
EXTERNC int setDialogGradient(int color1, int color2, int direction);

/**********************************************************************/
/* setDialogColor(color)
/*  Sets the background color for the client area of the dialog box.
/*
/**********************************************************************/
EXTERNC int setDialogColor(int color);

EXTERNC int getDialogColor();

/**********************************************************************/
/* setDialogImage(filename)
/*  Applies an image from the specified file as the background image
/*  for the client area of the dialog box, tiling or stretching it to
/*  fit if requested by setDialogImageMode.
/*
/**********************************************************************/
EXTERNC int setDialogImage(char * filename);

/**********************************************************************/
/* setDialogImageMode({EXACT==0|TILED==1|STRETCHED==2}, <stretch_mode>)
/*  Specifies whether the background image is to the tiled or
/*  stretched to fill the entire dialog, or used exactly as is.
/*  If the image is to be stretched, then the stretch mode is
/*  also specified.
/*
/**********************************************************************/
EXTERNC int setDialogImageMode(int mode, int stretchMode);

/**********************************************************************/
/* setDialogDragMode({TitleBar==0|Background==1|None==2})
/*  Determines whether the filter dialog box can be dragged by the
/*  title bar only (0), by dragging the title bar or anywhere on the
/*  dialog background (1), or not at all (2).  The default mode is 0.
/**********************************************************************/
EXTERNC int setDialogDragMode(int mode);

/**********************************************************************/
/* setDialogRegion(<region_expression>)
/*  Sets the clipping region (outline) of the filter dialog.
/*
/**********************************************************************/
EXTERNC int setDialogRegion(INT_PTR rgn);

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
EXTERNC INT_PTR createRectRgn(int xLeft, int yTop, int xRight, int yBottom);

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
EXTERNC INT_PTR createRoundRectRgn(int xLeft, int yTop, int xRight, int yBottom,
                                 int eWidth, int eHeight);

/**********************************************************************/
/* REGION createCircularRgn(xLeft, yTop, iDiameter)
/*  Creates a simple circular region.  <left> and <top> give the x- and
/*  y-coordinates of the upper-left corner of the bounding box for the
/*  circle.  <diameter> gives the diameter of the circle.  All measurements
/*  are in DBUs.
/*
/**********************************************************************/
EXTERNC INT_PTR createCircularRgn(int xLeft, int yTop, int iDiameter);

/**********************************************************************/
/* REGION createEllipticRgn(xLeft, yTop, xRight, yBottom)
/*  Creates an elliptical region.  <left>, <top>, <right>, and <bottom>
/*  define the coordinates of the bounding box for the ellipse.  All
/*  measurements are in DBUs.
/*
/**********************************************************************/
EXTERNC INT_PTR createEllipticRgn(int xLeft, int yTop, int xRight, int yBottom);

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
EXTERNC INT_PTR createPolyRgn(int fillMode, int nPoints, int x1, int y1,
                            int x2, int y2, int x3, int y3, ...);

/**********************************************************************/
/* REGION createDialogRgn();
/*  Is a predefined rectangular region consisting of the default dialog
/*  box itself.
/*
/**********************************************************************/
EXTERNC int createDialogRgn(void);

/**********************************************************************/
/* REGION createTitlebarRgn();
/*  Is a predefined rectangular region defining the area of the dialog
/*  title bar (if present).