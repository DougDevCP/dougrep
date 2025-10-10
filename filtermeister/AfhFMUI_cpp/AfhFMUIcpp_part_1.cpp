///////////////////////////////////////////////////////////////////////////////////
//  File: AfhFMUI.c
//
//  Source file for the FilterMeisterVS Windows specific dialog handling.
//
//  Copyright (c) 1997-2017 by Alex Hunter, Harald Heim & Martijn Van der Lee
//  
//  Derived from Photoshop SDK source files which are: Copyright by Adobe Systems, Inc.
// 
//  FilterMeisterVS is free software. You can redistribute and modify it
//  under the terms of the MPL2 license. See https://www.mozilla.org/en-US/MPL/2.0/
//  Any changes to the FilterMeisterVS files in the /components/filtermeister/
//  sub folder have to be published or sent to harry@thepluginsite.com. 
//  The individual filter code itself does not need to be disclosed.
//  
///////////////////////////////////////////////////////////////////////////////////

/****************************************************************

        TO DO

  - F12 no longer toggles editor?
  - use F11 to enter advanced mode?
  - editor controls are disabled every other time FM invoked - FIXED
  - preview cursor not restored on right button up?
  - globals param is obsolete in many cases, but fix setZoom
    et al anyway
  - local imageOffset in setZoom?
  - IDC_HELP --> IDC_HELP1 - DONE
  - Should setZoom skip 13, 15?
  - Is fDragging logic needed in setZoom?
  - Should setZoom be doing SetFocus???
  - Why did HH comment out MOUSEOUT logic in WM_MOUSEMOVE?
  - Fix &rcCtl != NULL logic
  - Check ClickDrag logic
  - Add pixelDepth, simplify logic
  - Make CTL_PREVIEW more like a user control (set inuse, etc.) and
    thereby simplify logic elsewhere
  - Should welcome be displayed only first time FM installed, or
    each time upgraded/re-installed?  (E.g., record current version
    in welcome field in registry, display welcome if different?)
  - Timer events use same ctl numbers as user controls?  Maybe they
    should be considered user controls in their own right, with own
    unique ctl numbers?
  - Add IDC_ARROW etc. as manifest constants??
  - Allow setPreviewCursor to accept a resource name?
  - Locate help.htm via various paths???
  - Add file name and dirty indicator to editor dialog title
  - Need to free the RichEdit library in AfhFMeditor.c???
  - Correctly restore minimized editor dialog window on subsequent
    invocations.
  - replace hard-coded control ids with symbolic names from resource.h
  - Init kills default region with upper rounded corners around the
    default filter dialog on XP (whether or not Visual Theme are enabled)
    -need to save/restore current dialog region (if present) instead of
    setting region to null in fm_resetAllCtls?
  - add itoa, ltoa, ultoa (non-ANSI)
  - add atof, atoi, atol (ANSI)
  - implement errno?

  - allow empty list of class-specific control attributes
  - ability to specify width of buddy1 NUMEDIT in STANDARD control
  - move Font... control back to position after Save...
  - bug when standard control val < -32k or > 32k?
  - createCtl() needs to reset divisor=1
  - setCtlDivisor() needs to update NUMEDIT buddy

  - cleanup and simplify the .pre_ctl logic:
  --- make sure .pre_ctl is set whenever .val is set
  --- better yet, replace .val everywhere with fmc.pre_ctl
      (except still use .val to save .pre_ctl across invocations)

  - restrict STANDARD/SCROLLBAR range to (-32k, +32k-1) ?
  - what is range for TRACKBAR?

  - interpolate filterFilenameText before using it in AfhFMmake
    so Filename:"!T" will work?

  ========================================================
  From the documentation for SetWindowRgn:

      If the bRedraw parameter is TRUE, the system sends the 
      WM_WINDOWPOSCHANGING and WM_WINDOWPOSCHANGED messages 
      to the window. 

  Would that be a better way to force our own redraws when needed?

  Also note:

      After a successful call to SetWindowRgn, the operating 
      system owns the region specified by the region handle hRgn. 
      The operating system does not make a copy of the region. 
      Thus, you should not make any further function calls with 
      this region handle. In particular, do not close this region 
      handle. 





  ======================= B U G S ========================

  - Correctly abort if SetupFilterRecordForProxy() gets an error,
    so host app won't crash. (Kohan)

  - fm_srcp() should mask off a if Z < 4 ???

  =======

    I'm currently looking at a problem in the 1.0 beta
    which crashes PSP and Photoshop, but doesn't crash in earlier
    versions, or in the latest build from my own sources.

    To reproduce the crash:

    1. Open a new instance of FM 1.0 beta in PSP or PS.
    2. Open the editor with blank source.
    3. Enter a single blank (or some other trivial FFP program) in
        the editor.
    4. Press the OK button.
    5. When asked if you want to recompile, click YES.
    6. The host app crashes.

   - above crash no longer occurs in 0.4.21 !???,
   - whoops, yes it does!!  PS disappears altogether now (no GP message)!

   - But a syntax error in 0.4.21 now crashes PS 6.0!!!
   --- could it be related to the NT/XP tooltip bug?
   --- seems to crash in SetFocus().  Problem with FM intercepting
       exceptions?

  **** AH! -- It's a simple stack overflow in FilterDialogProc(),
       caused by szCode[] growing too large.
       - make szCode static for now
       - eventually, eliminate use of szCode altogether

/***************************************************************/


#include <windows.h>
#include <windowsx.h>       // For SetDlgMsgResult()
#include <wchar.h> // Added by Ognen Genchev
#include <math.h>
#include <gdiplus.h> // Added by Ognen Genchev
#include <commdlg.h>
#include <commctrl.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <stddef.h>         /* offsetof */
#include <process.h>        // _beginthread
#include <time.h>
#include <assert.h>
#include <direct.h>
#ifndef APP
    #include "PITypes.h"
    #include "PIGeneral.h"
    #include "PIFilter.h"
    #include "PIAbout.h"
    #include "PIProperties.h"
    #include "PIUtilities.h"
	#include "PIColorSpaceSuite.h"
#endif

#include "AfhFM.h"
#include "AfhFMcontext.h"
#if SKIN
    #include "../../FW_SpecialFunctions.h"
#endif

#include "resource.h"

#include "WinUtil.h"    // Message crackers and stuff

#include "FW_FFP.h"

// Added by Ognen Genchev
// A runtime copy of DialConfig that stores GDI+ colors for drawing
struct DialRuntimeConfig {
    HWND hwnd;
    int index;
    float angle;
    BOOL dragging;

    int x, y, size;

    int dialRadius;
    int tickLength;
    int ellipseThickness;
    int tickThickness;
    int dotRadius;

    Gdiplus::Color dotColor;
    Gdiplus::Color tickColor;
    Gdiplus::Color ellipseColor;
    Gdiplus::Color backgroundColor;

    WCHAR fontName[32];
};
// end

//#include "../../FW_SpecialFunctions.h"

#define WM_MOUSEWHEEL 0x020A

BOOL UIScaling = false;

//HANDLE hDllInstance=NULL;  /* DLL instance handle */

//HWND ghTT;  //tooltip control handle
//HWND hEditorDlg = NULL;

HHOOK KeyHook = NULL;

int cancelEventPerformed=0;
int okEventPerformed=0;


//Moved to main level of AfhFMUI.c, Harald Heim. Dec 14, 2002
//Moved to Global Level, because of restoreImageData()
POINT imageOffset = {0, 0}; //Static

//Move up to top level of AfhFMUI.c, Harald Heim. January, 2003
static HCURSOR hcHand;
INT_PTR cursorResource;
#ifdef APP
	HANDLE hbicon = 0;
#endif

// Added by Ognen Genchev
// --------------------------------------------------------------------
// Drawing
// --------------------------------------------------------------------
static void DrawCustomDial(HDC hdc, RECT rc, DialRuntimeConfig* config)
{
    HDC memDC = CreateCompatibleDC(hdc);
    HBITMAP memBmp = CreateCompatibleBitmap(hdc, rc.right, rc.bottom);
    HBITMAP oldBmp = (HBITMAP)SelectObject(memDC, memBmp);

    HBRUSH bgBrush = CreateSolidBrush(RGB(
        config->backgroundColor.GetRed(),
        config->backgroundColor.GetGreen(),
        config->backgroundColor.GetBlue()));
    FillRect(memDC, &rc, bgBrush);
    DeleteObject(bgBrush);

    Gdiplus::Graphics graphics(memDC);
    graphics.SetSmoothingMode(Gdiplus::SmoothingModeAntiAlias);

    Gdiplus::Pen circlePen(config->ellipseColor, (Gdiplus::REAL)config->ellipseThickness);
    Gdiplus::Pen tickPen(config->tickColor, (Gdiplus::REAL)config->tickThickness);
    Gdiplus::SolidBrush dotBrush(config->dotColor);

    int width = rc.right - rc.left;
    int height = rc.bottom - rc.top;
    int centerX = width / 2;
    int centerY = height / 2;

    int dialSize = config->dialRadius * 2;
    graphics.DrawEllipse(&circlePen,
        centerX - config->dialRadius,
        centerY - config->dialRadius,
        dialSize, dialSize);

    // tick marks
    Gdiplus::Point ticks[4][2] = {
        { Gdiplus::Point(centerX, centerY - config->dialRadius), Gdiplus::Point(centerX, centerY - config->dialRadius - config->tickLength) },
        { Gdiplus::Point(centerX + config->dialRadius, centerY), Gdiplus::Point(centerX + config->dialRadius + config->tickLength, centerY) },
        { Gdiplus::Point(centerX, centerY + config->dialRadius), Gdiplus::Point(centerX, centerY + config->dialRadius + config->tickLength) },
        { Gdiplus::Point(centerX - config->dialRadius, centerY), Gdiplus::Point(centerX - config->dialRadius - config->tickLength, centerY) }
    };
    for (int i = 0; i < 4; ++i)
        graphics.DrawLine(&tickPen, ticks[i][0], ticks[i][1]);

    float rad = config->angle * (3.14159265f / 180.0f);
    float dotX = centerX - config->dialRadius * cosf(rad);
    float dotY = centerY - config->dialRadius * sinf(rad);
    graphics.FillEllipse(&dotBrush,
        Gdiplus::RectF(dotX - config->dotRadius,
                       dotY - config->dotRadius,
                       (Gdiplus::REAL)(config->dotRadius * 2),
                       (Gdiplus::REAL)(config->dotRadius * 2)));

    // draw angle text
    WCHAR text[16];
    swprintf_s(text, 16, L"%.0f\x00B0", config->angle);

    Gdiplus::FontFamily fontFamily(config->fontName);
    Gdiplus::Font font(&fontFamily, (Gdiplus::REAL)(config->dialRadius / 2), Gdiplus::FontStyleRegular, Gdiplus::UnitPixel);
    Gdiplus::SolidBrush textBrush(Gdiplus::Color(255, 0, 0, 0));
    Gdiplus::StringFormat format;
    format.SetAlignment(Gdiplus::StringAlignmentCenter);
    format.SetLineAlignment(Gdiplus::StringAlignmentCenter);

    Gdiplus::RectF layoutRect(
        (Gdiplus::REAL)(centerX - config->dialRadius),
        (Gdiplus::REAL)(centerY - config->dialRadius),
        (Gdiplus::REAL)dialSize,
        (Gdiplus::REAL)dialSize);
