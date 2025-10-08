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

    graphics.DrawString(text, -1, &font, layoutRect, &format, &textBrush);

    BitBlt(hdc, 0, 0, rc.right, rc.bottom, memDC, 0, 0, SRCCOPY);
    SelectObject(memDC, oldBmp);
    DeleteObject(memBmp);
    DeleteDC(memDC);
}

// --------------------------------------------------------------------
// Angle Update
// --------------------------------------------------------------------
static void UpdateAngleFromMouse(DialRuntimeConfig* config, int x, int y)
{
    RECT rc;
    GetClientRect(config->hwnd, &rc);
    POINT center = { (rc.right - rc.left) / 2, (rc.bottom - rc.top) / 2 };

    float dx = (float)(x - center.x);
    float dy = (float)(center.y - y); // inverted Y
    config->angle = atan2f(dy, -dx) * (180.0f / 3.14159265f);
    if (config->angle < 0) config->angle += 360.0f;
    InvalidateRect(config->hwnd, NULL, TRUE);
}

// --------------------------------------------------------------------
// Notify Parent of Change
// --------------------------------------------------------------------
// extern TParameters* gParams; // defined elsewhere

static void NotifyDialChanged(DialRuntimeConfig* config)
{
    if (!config || !config->hwnd) return;
    HWND hParent = GetParent(config->hwnd);
    if (!hParent) return;

    int idx = config->index;
    gParams->ctl[idx].val = (int)config->angle;

    SendMessage(hParent, WM_COMMAND,
        MAKEWPARAM(idx, EN_CHANGE), (LPARAM)config->hwnd);
}

// --------------------------------------------------------------------
// Custom Dial Window Proc
// --------------------------------------------------------------------
static LRESULT CALLBACK CustomDialProc(HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam)
{
    DialRuntimeConfig* config = (DialRuntimeConfig*)GetWindowLongPtr(hwnd, GWLP_USERDATA);
    switch (msg)
    {
    case WM_PAINT: {
        PAINTSTRUCT ps;
        HDC hdc = BeginPaint(hwnd, &ps);
        RECT rc;
        GetClientRect(hwnd, &rc);
        if (config) DrawCustomDial(hdc, rc, config);
        EndPaint(hwnd, &ps);
        return 0;
    }
    case WM_MOUSEWHEEL:
        if (config) {
            config->angle += GET_WHEEL_DELTA_WPARAM(wParam) > 0 ? 10 : -10;
            if (config->angle < 0) config->angle += 360;
            if (config->angle >= 360) config->angle -= 360;
            InvalidateRect(hwnd, NULL, FALSE);
            NotifyDialChanged(config);
        }
        return 0;
    case WM_LBUTTONDOWN:
        if (config) {
            config->dragging = TRUE;
            SetCapture(hwnd);
            UpdateAngleFromMouse(config, GET_X_LPARAM(lParam), GET_Y_LPARAM(lParam));
            NotifyDialChanged(config);
        }
        return 0;
    case WM_LBUTTONUP:
        if (config) {
            config->dragging = FALSE;
            ReleaseCapture();
            NotifyDialChanged(config);
        }
        return 0;
    case WM_MOUSEMOVE:
        if (config && config->dragging) {
            UpdateAngleFromMouse(config, GET_X_LPARAM(lParam), GET_Y_LPARAM(lParam));
            NotifyDialChanged(config); 
        }
        return 0;
    case WM_DESTROY:
        if (config) delete config;
        return 0;
    }
    return DefWindowProc(hwnd, msg, wParam, lParam);
}

// --------------------------------------------------------------------
// Create Dial Control (callable from createCtl())
// --------------------------------------------------------------------
HWND CreateCustomDialControl(HWND parent, HINSTANCE hInst, int x, int y, int size, HMENU id, const DialConfig* baseConfig)
{
    HWND hwnd = CreateWindow(WC_STATIC, NULL,
        WS_VISIBLE | WS_CHILD, x, y, size, size,
        parent, id, hInst, NULL);

    DialRuntimeConfig* config = new DialRuntimeConfig();
    config->hwnd = hwnd;
    config->index = baseConfig->index;
    config->angle = baseConfig->angle;
    config->dragging = baseConfig->dragging;

    config->x = baseConfig->x;
    config->y = baseConfig->y;
    config->size = baseConfig->size;

    config->dialRadius = baseConfig->dialRadius;
    config->tickLength = baseConfig->tickLength;
    config->ellipseThickness = baseConfig->ellipseThickness;
    config->tickThickness = baseConfig->tickThickness;
    config->dotRadius = baseConfig->dotRadius;

    // Convert COLORREF → GDI+ Color
    config->dotColor.SetFromCOLORREF(baseConfig->dotColor);
    config->tickColor.SetFromCOLORREF(baseConfig->tickColor);
    config->ellipseColor.SetFromCOLORREF(baseConfig->ellipseColor);
    config->backgroundColor.SetFromCOLORREF(baseConfig->backgroundColor);

    wcscpy_s(config->fontName, baseConfig->fontName);

    SetWindowLongPtr(hwnd, GWLP_USERDATA, (LONG_PTR)config);
    SetWindowLongPtr(hwnd, GWLP_WNDPROC, (LONG_PTR)CustomDialProc);

    return hwnd;
}
// end

// DrawBitmapTransparent(), CustomTrackbarWndProc() added by Ognen Genchev for customizing trackbar and slider controls thumbs and channels with bitmap
void DrawBitmapTransparent(HDC hDCDest, int nXDest, int nYDest, int nBitmapWidth, int nBitmapHeight, HBITMAP hBitmap, int nXSrc, int nYSrc, int nTransparentColor)
{
    HDC hDCSrc;
    HBITMAP hBitmapOld;
    HDC hDCMask;
    HBITMAP hBitmapMask;
    HBITMAP hBitmapMaskOld;
    HDC hDCMem;
    HBITMAP hBitmapMem;
    HBITMAP hBitmapMemOld;
    int nBkColorOld;
    int nTextColorOld;
    BITMAP bm;

    GetObject( hBitmap, sizeof( BITMAP ), &bm );

    if (!nBitmapWidth) {
        nBitmapWidth = bm.bmWidth;
    }

    if (!nBitmapHeight) {
        nBitmapHeight = bm.bmHeight;
    }

    hDCSrc = CreateCompatibleDC( hDCDest );
    hBitmapOld = (HBITMAP)SelectObject( hDCSrc, hBitmap );
    hDCMask = CreateCompatibleDC( hDCDest );
    hBitmapMask = CreateBitmap( nBitmapWidth, nBitmapHeight, 1, 1, 0 );
    hBitmapMaskOld = (HBITMAP)SelectObject( hDCMask, hBitmapMask );
    hDCMem = CreateCompatibleDC( hDCDest );
    hBitmapMem = CreateCompatibleBitmap( hDCDest, nBitmapWidth, nBitmapHeight );
    hBitmapMemOld = (HBITMAP)SelectObject( hDCMem, hBitmapMem );
    nBkColorOld = SetBkColor( hDCSrc, nTransparentColor );
    BitBlt( hDCMask, 0, 0, nBitmapWidth, nBitmapHeight, hDCSrc, nXSrc, nYSrc, SRCCOPY );
    SetBkColor( hDCSrc, nBkColorOld );
    nBkColorOld = SetBkColor( hDCDest, RGB(255,255,255) );
    nTextColorOld = SetTextColor( hDCDest, RGB(0,0,0) );
    BitBlt( hDCMem, 0, 0, nBitmapWidth, nBitmapHeight, hDCDest, nXDest, nYDest, SRCCOPY );
    BitBlt( hDCMem, 0, 0, nBitmapWidth, nBitmapHeight, hDCSrc, nXSrc, nYSrc, SRCINVERT );
    BitBlt( hDCMem, 0, 0, nBitmapWidth, nBitmapHeight, hDCMask, 0, 0, SRCAND );
    BitBlt( hDCMem, 0, 0, nBitmapWidth, nBitmapHeight, hDCSrc, nXSrc, nYSrc, SRCINVERT );
    BitBlt( hDCDest, nXDest, nYDest, nBitmapWidth, nBitmapHeight, hDCMem, 0, 0, SRCCOPY );
    SetBkColor( hDCDest, nBkColorOld );
    SetTextColor( hDCDest, nTextColorOld );
    SelectObject( hDCMem, hBitmapMemOld );
    DeleteDC( hDCMem );
    DeleteObject( hBitmapMem );
    SelectObject( hDCMask, hBitmapMaskOld );
    DeleteDC( hDCMask );
    DeleteObject( hBitmapMask );
    SelectObject( hDCSrc, hBitmapOld );
    DeleteDC( hDCSrc );
}

INT_PTR WINAPI CustomTrackbarWndProc(HWND hwnd, UINT message, WPARAM wParam, LPARAM lParam) {

    switch(message) {
        case WM_NOTIFY:
        {
            LPNMHDR lpNmhdr = (LPNMHDR)lParam;
            if (lpNmhdr->code == NM_CUSTOMDRAW)
            {
                LPNMCUSTOMDRAW lpNMCustomDraw = (LPNMCUSTOMDRAW)lParam;

                if (lpNMCustomDraw->dwDrawStage == CDDS_PREPAINT) {
                    return CDRF_NOTIFYITEMDRAW;
                }

                else if (lpNMCustomDraw->dwDrawStage == CDDS_ITEMPREPAINT)
                {
                    long nLeft = lpNMCustomDraw->rc.left;
                    long nTop = lpNMCustomDraw->rc.top;
                    long nRight = lpNMCustomDraw->rc.right;
                    long nBottom = lpNMCustomDraw->rc.bottom;
                    if (lpNMCustomDraw->dwItemSpec == TBCD_THUMB && lpNMCustomDraw->hdr.hwndFrom == gParams->ctl[0].hCtl)
                    {
                        long nWidth = nRight - nLeft;
                        long nHeight = nBottom - nTop;

                        if (nWidth - bm.bmWidth > 0)
                        {
                            nLeft += (nWidth - bm.bmWidth)/2;
                            nWidth = bm.bmWidth;
                        }

                        if (nHeight - bm.bmHeight > 0)
                        {
                            nTop += (nHeight - bm.bmHeight)/2;
                            nHeight = bm.bmHeight;
                        }

                        DrawBitmapTransparent(lpNMCustomDraw->hdc , nLeft, nTop, nWidth, nHeight, hBitmapThumb[0], 0, 0, RGB( 255, 0, 255 ));

                        return CDRF_SKIPDEFAULT;
                    }
                    if (lpNMCustomDraw->dwItemSpec == TBCD_CHANNEL && lpNMCustomDraw->hdr.hwndFrom == gParams->ctl[0].hCtl)
                    {
                        long nWidth = nRight - nLeft;
                        long nHeight = nBottom - nTop;

                        if (nWidth - bm.bmWidth > 0)
                        {
                            nLeft += (nWidth - bm.bmWidth)/2;
                            nWidth = bm.bmWidth;
                        }

                        if (nHeight - bm.bmHeight > 0)
                        {
                            nTop += (nHeight - bm.bmHeight)/2;
                            nHeight = bm.bmHeight;
                        }

                        DrawBitmapTransparent(lpNMCustomDraw->hdc , nLeft, nTop, nWidth, nHeight, hBitmapChannel[0], 0, 0, RGB( 255, 0, 255 ));

                        return CDRF_SKIPDEFAULT;
                    }
                    if (lpNMCustomDraw->dwItemSpec == TBCD_THUMB && lpNMCustomDraw->hdr.hwndFrom == gParams->ctl[1].hCtl)
                    {
                        long nWidth = nRight - nLeft;
                        long nHeight = nBottom - nTop;

                        if (nWidth - bm.bmWidth > 0)
                        {
                            nLeft += (nWidth - bm.bmWidth)/2;
                            nWidth = bm.bmWidth;
                        }

                        if (nHeight - bm.bmHeight > 0)
                        {
                            nTop += (nHeight - bm.bmHeight)/2;
                            nHeight = bm.bmHeight;
                        }

                        DrawBitmapTransparent(lpNMCustomDraw->hdc , nLeft, nTop, nWidth, nHeight, hBitmapThumb[1], 0, 0, RGB( 255, 0, 255 ));

                        return CDRF_SKIPDEFAULT;
                    }
                    if (lpNMCustomDraw->dwItemSpec == TBCD_CHANNEL && lpNMCustomDraw->hdr.hwndFrom == gParams->ctl[1].hCtl)
                    {
                        long nWidth = nRight - nLeft;
                        long nHeight = nBottom - nTop;

                        if (nWidth - bm.bmWidth > 0)
                        {
                            nLeft += (nWidth - bm.bmWidth)/2;
                            nWidth = bm.bmWidth;
                        }

                        if (nHeight - bm.bmHeight > 0)
                        {
                            nTop += (nHeight - bm.bmHeight)/2;
                            nHeight = bm.bmHeight;
                        }

                        DrawBitmapTransparent(lpNMCustomDraw->hdc , nLeft, nTop, nWidth, nHeight, hBitmapChannel[1], 0, 0, RGB( 255, 0, 255 ));

                        return CDRF_SKIPDEFAULT;
                    }
                }
            }
        }
        break;
    }
    return CallWindowProc(defWndProc, hwnd, message, wParam, lParam);
}
// end

//Declare
BOOL PerformControlAction(int n, FME e, int previous, int ctlMsg, int ctlCode,
                          BOOL fActionable, BOOL fProcessed, int overrideBusy);


#define IDC_PROXY   101


BOOL fDragging = FALSE;
BOOL fDrawing = FALSE;
BOOL fSizing = FALSE;
BOOL fScaling = FALSE;
BOOL fPaint = FALSE;
BOOL fMaximized = FALSE;
BOOL fMinimized = FALSE;
//BOOL fMoving = FALSE;
BOOL fDialogDisplayed = false;


#ifdef START_SCALEFACTOR_1
static long scaleFactor = 1;
#else
static long scaleFactor = -888; // -888 means compute optimal scale factor
#endif



/*****************************************************************************/

/* Calculates the rectangle we will want to use for the proxy for an image. */

void GetProxyItemRect (HWND hDlg, Rect *proxyItemRect)
    {

    RECT    wRect;

    GetWindowRect(GetDlgItem(hDlg, IDC_PROXY), &wRect); 
    ScreenToClient (hDlg, (LPPOINT)&wRect);
    ScreenToClient (hDlg, (LPPOINT)&(wRect.right));

    proxyItemRect->left = (int16)wRect.left;
    proxyItemRect->top  = (int16)wRect.top;
    proxyItemRect->right = (int16)wRect.right;
    proxyItemRect->bottom = (int16)wRect.bottom;
    
    }

/*****************************************************************************/



static unsigned8 *pRGBbuffer = NULL;    //remember to deallocate at exit!!!!
BufferID pRGBbufferID;
static int pRGBbufferSize = 0;

#ifndef APP

void ShowOutputBuffer( GPtr globals, HWND hDlg, BOOL fDragging)
{
    PSPixelMap pixels;
    PSPixelMask mask;
    PAINTSTRUCT  ps;
    RECT  itemBounds;
    RECT  wRect;
#if BIGDOCUMENT
	VRect iRect;
#else
	Rect  iRect;
#endif
    int proxyWidth;
    int proxyHeight;
#if 1
    RECT  rcIntersect;
#endif
    POINT   mapOrigin; 
    HDC     hDC;

	long  scaleFactor = gStuff->inputRate >> 16;

    //startClock();

    if (gResult != noErr) {
        // Got an error somewhere along the way...
        // Get out quick in case the error was returned by
        // AdvanceState(), in which case the input/output data
        // pointers are bogus and can cause severe dyspepsia.
        return;
    }

    // If AFH_DRAG_FILTERED_OUTPUT is set, turn off fDragging locally so we
    // can display the filtered output instead of the raw input...
    if (gParams->flags & AFH_DRAG_FILTERED_OUTPUT)
    {
        fDragging = FALSE;
    }

    GetWindowRect(GetDlgItem(hDlg, IDC_PROXY), &wRect);
    proxyWidth = wRect.right - wRect.left;
    proxyHeight = wRect.bottom - wRect.top;
    mapOrigin.x = 0;
    mapOrigin.y = 0;
    ClientToScreen(hDlg, &mapOrigin);

#if BIGDOCUMENT
	iRect = GetOutRect();
#else
    copyRect(&iRect,&gStuff->outRect);
#endif
#if HIGHZOOM
	//Scale up for display
	if (gFmcp->enlargeFactor != 1){
		iRect.left *= gFmcp->enlargeFactor;
		iRect.right *= gFmcp->enlargeFactor;
		iRect.top *= gFmcp->enlargeFactor;
		iRect.bottom *= gFmcp->enlargeFactor;
	}
#endif

    // Following rounded in different directions if Proxy was shifted off top or left of screen,
    // since the screen coords went negative, and on x86 architecture, integer division always truncates
    // towards zero.  This resulted in some strange off-by-one-pixel artifacts if the proxy
    // got shifted offscreen and then back onscreen.  Fix is to convert wRect to client coords
    // *before* division by 2, so the dividend will always be positive and truncate consistently
    // *down* towards zero.
    //itemBounds.left = ((wRect.right + wRect.left - gStuff->outRect.right + gStuff->outRect.left) / 2) - mapOrigin.x;
    //itemBounds.top = ((wRect.bottom + wRect.top - gStuff->outRect.bottom + gStuff->outRect.top) / 2) - mapOrigin.y;

    /*itemBounds.left = (wRect.right - mapOrigin.x + wRect.left - mapOrigin.x - (gStuff->outRect.right - gStuff->outRect.left)) / 2;
    itemBounds.top = (wRect.bottom - mapOrigin.y + wRect.top - mapOrigin.y - (gStuff->outRect.bottom - gStuff->outRect.top)) / 2;
    itemBounds.right = itemBounds.left + (gStuff->outRect.right - gStuff->outRect.left);
    itemBounds.bottom = itemBounds.top + (gStuff->outRect.bottom - gStuff->outRect.top);*/
	



    //Temporary Deactivated

    if (iRect.bottom - iRect.top <= wRect.bottom - wRect.top)
    {    
        itemBounds.top = (wRect.bottom - mapOrigin.y + wRect.top - mapOrigin.y - (iRect.bottom - iRect.top)) / 2;
        itemBounds.bottom = itemBounds.top + (iRect.bottom - iRect.top);
    } else { // in case preview is sized down
        itemBounds.top = (wRect.bottom - mapOrigin.y + wRect.top - mapOrigin.y - (wRect.bottom - wRect.top))/2;
        itemBounds.bottom = itemBounds.top + (wRect.bottom - wRect.top);
    }

    if (iRect.right - iRect.left <= wRect.right - wRect.left)
    {
        itemBounds.left = (wRect.right - mapOrigin.x + wRect.left - mapOrigin.x - (iRect.right - iRect.left)) / 2;
        itemBounds.right = itemBounds.left + (iRect.right - iRect.left);
    } else { // in case preview is sized down
        itemBounds.left = (wRect.right - mapOrigin.x + wRect.left - mapOrigin.x - (wRect.right - wRect.left))/2;
        itemBounds.right = itemBounds.left + (wRect.right - wRect.left);
    }



    hDC = BeginPaint(hDlg, &ps);

#if 1
    if (IntersectRect(&rcIntersect, &itemBounds, &ps.rcPaint)) {
#endif

    wRect = itemBounds;
    InflateRect(&wRect, 1, 1);
    //FrameRect(hDC, &wRect, GetStockObject(BLACK_BRUSH)); //Moved down


    pixels.version = 1;
    pixels.bounds.top = iRect.top;
    pixels.bounds.left = iRect.left;
    /*pixels.bounds.bottom = gStuff->outRect.bottom;
    pixels.bounds.right = gStuff->outRect.right;*/
    //Info ("%d,%d - %d,%d",itemBounds.top,itemBounds.bottom,gStuff->outRect.top,gStuff->outRect.bottom);

    if ( iRect.bottom - iRect.top <= itemBounds.bottom - itemBounds.top)
    {
        pixels.bounds.bottom = iRect.bottom;
    } else { // in case preview is sized down
        pixels.bounds.bottom = iRect.top + (itemBounds.bottom - itemBounds.top);
    }

    if ( iRect.right - iRect.left <= itemBounds.right - itemBounds.left)
    {
        pixels.bounds.right = iRect.right;
    } else { // in case preview is sized down
        pixels.bounds.right = iRect.left  + (itemBounds.right - itemBounds.left);
    }


    pixels.imageMode = gStuff->imageMode;
    pixels.rowBytes = fDragging ? gStuff->inRowBytes : gStuff->outRowBytes;
    pixels.colBytes = fDragging ? gStuff->inHiPlane - gStuff->inLoPlane + 1
                                : gStuff->outHiPlane - gStuff->outLoPlane + 1;
    pixels.planeBytes = 1;
    pixels.baseAddr = fDragging ? MyAdjustedInData : gStuff->outData;


#ifdef HIGHZOOM
	// >100% Zoom
	//if (pixels.imageMode == plugInModeRGB48 || pixels.imageMode == plugInModeLab48 || pixels.imageMode == plugInModeDeepMultichannel) 
	if (gFmcp->enlargeFactor != 1)
	{
        unsigned8 *inPtr1 = (unsigned8 *)pixels.baseAddr;
        unsigned8 *inBase = (unsigned8 *)pixels.baseAddr;
		unsigned8 *outPtr;
        int iRows = pixels.bounds.bottom - pixels.bounds.top;
		int iCols = pixels.bounds.right - pixels.bounds.left;
		
		int iR,pR;
		int maxRC;
        //int xStart= (iCols - iCols/gFmcp->enlargeFactor)/2;
		//int yStart= (iRows - iRows/gFmcp->enlargeFactor)/2;
		int xStart= 0;
		int yStart= 0;
		
        int rgb16 = pixels.imageMode == plugInModeRGB48 || pixels.imageMode == plugInModeLab48 || pixels.imageMode == plugInModeDeepMultichannel;
        int rgb8 = pixels.imageMode == plugInModeRGBColor || pixels.imageMode == plugInModeLabColor || pixels.imageMode == plugInModeMultichannel;
        int gray16 = pixels.imageMode == plugInModeGray16;
        int gray8 = pixels.imageMode == plugInModeGrayScale;
        int cmyk8 = pixels.imageMode == plugInModeCMYKColor;
        int cmyk16 = pixels.imageMode == plugInModeCMYK64;
        int r,g,b,k,a,iPos,j;
		
		
        if (pRGBbufferSize != proxyWidth*proxyHeight*(gStuff->inHiPlane+1)){ //3 //Only reallocate if size has to be changed
            if (pRGBbuffer) {
                if (pRGBbufferID != 0){
			        UnlockBuffer(pRGBbufferID);
			        FreeBuffer(pRGBbufferID); 
		        } else
			        free(pRGBbuffer);
		        pRGBbufferID = 0;
                pRGBbuffer = NULL;
				pRGBbufferSize = 0;
            }
            
            // Allocate a buffer large enough to cover the entire proxy window.
            if (AllocateBuffer( proxyWidth*proxyHeight * (gStuff->inHiPlane+1), &pRGBbufferID) == noErr){
				pRGBbuffer = (unsigned8 *)LockBuffer(pRGBbufferID, true);
			} else { //Fall back to Windows allocation
				pRGBbuffer = (unsigned8 *)calloc(proxyWidth*proxyHeight, (gStuff->inHiPlane+1)); //3
				pRGBbufferID=0;
			}


            if (!pRGBbuffer) {
#if 1
                ErrorOk("Can't allocate proxy preview buffer for >100% zoom!");
#endif
                pRGBbufferSize = 0;
                goto egress;    
            } else {

                pRGBbufferSize = proxyWidth*proxyHeight*(gStuff->inHiPlane+1); //3
            }
        }


		if (rgb16 || gray16 || cmyk16){
            pR = pixels.rowBytes / 2;
		}else{
            pR = pixels.rowBytes;
		}

		maxRC = (iRows-1)/gFmcp->enlargeFactor*pR + (iCols-1)/gFmcp->enlargeFactor*pixels.colBytes;
		
        outPtr = pRGBbuffer;
        while (iRows--) {

            iR = (-yStart + iRows/gFmcp->enlargeFactor)*pR;
    
            j = iCols;
            while (j--) {
                // Unroll 3 planes...
                iPos = maxRC - (iR  + (-xStart + j/gFmcp->enlargeFactor)*pixels.colBytes); //*pC

                if (rgb8){
                    outPtr[0] = inBase[iPos + 0];
                    outPtr[1] = inBase[iPos + 1];
                    outPtr[2] = inBase[iPos + 2];
                    if (gStuff->inHiPlane==3) {
					    outPtr[3] = inBase[iPos + 3];
				    }
                } else if (rgb16){
                    r = ((int16 *)inBase)[iPos + 0];
                    g = ((int16 *)inBase)[iPos + 1];
                    b = ((int16 *)inBase)[iPos + 2];
					outPtr[0] = (r >> 7) | (r >> 15);
                    outPtr[1] = (g >> 7) | (g >> 15);
                    outPtr[2] = (b >> 7) | (b >> 15);
                    if (gStuff->inHiPlane==3) {
					    a = ((int16 *)inBase)[iPos + 3];
					    outPtr[3] = (a >> 7) | (a >> 15);
				    }
                } else if (gray16){
                    r = ((int16 *)inBase)[iPos + 0];
                    outPtr[0] = (r >> 7) | (r >> 15);
                    if (gStuff->inHiPlane==1) {
					    a = ((int16 *)inBase)[iPos + 1];
					    outPtr[1] = (a >> 7) | (a >> 15);
				    }
                } else if (gray8){
                    outPtr[0] = inBase[iPos + 0];
                    if (gStuff->inHiPlane==1) {
					    outPtr[1] = inBase[iPos + 1];
				    }
                } else  if (cmyk8){
                    outPtr[0] = inBase[iPos + 0];
                    outPtr[1] = inBase[iPos + 1];
                    outPtr[2] = inBase[iPos + 2];
                    outPtr[3] = inBase[iPos + 3];
                    if (gStuff->inHiPlane==4) {
					    outPtr[4] = inBase[iPos + 4];
				    }
                } else if (cmyk16){
                    r = ((int16 *)inBase)[iPos + 0];
                    g = ((int16 *)inBase)[iPos + 1];
                    b = ((int16 *)inBase)[iPos + 2];
                    k = ((int16 *)inBase)[iPos + 3];
                    outPtr[0] = (r >> 7) | (r >> 15);
                    outPtr[1] = (g >> 7) | (g >> 15);
                    outPtr[2] = (b >> 7) | (b >> 15);
                    outPtr[3] = (k >> 7) | (k >> 15);
                    if (gStuff->inHiPlane==4) {
					    a = ((int16 *)inBase)[iPos + 4];
					    outPtr[4] = (a >> 7) | (a >> 15);
				    }
                }
                outPtr += (gStuff->inHiPlane+1); //3
            }//while j
            

        }//while iRows

        // Point pixel map to the RGB buffer...
        pixels.baseAddr = pRGBbuffer;
        pixels.rowBytes = iCols*(gStuff->inHiPlane+1); //3
        pixels.colBytes = (gStuff->inHiPlane+1); //3
        pixels.planeBytes = 1;

        if (pixels.imageMode == plugInModeRGB48)
            pixels.imageMode = plugInModeRGBColor;      //data is RGB
        else if (pixels.imageMode == plugInModeLab48)
            pixels.imageMode = plugInModeLabColor; //data is Lab
        else if (pixels.imageMode == plugInModeDeepMultichannel)
            pixels.imageMode = plugInModeMultichannel; //data is Multichannel
        else if (pixels.imageMode == plugInModeGray16)
            pixels.imageMode = plugInModeGrayScale;
        else if (pixels.imageMode == plugInModeCMYK64)
            pixels.imageMode = plugInModeCMYKColor;
    }
#endif




#if 1


    //Added by Harald Heim, December 6, 2002
    if (gFmcp->enlargeFactor==1 && (pixels.imageMode == plugInModeRGB48 || pixels.imageMode == plugInModeLab48 || pixels.imageMode == plugInModeDeepMultichannel)) {

        // Convert outData from 48-bit to 24-bit for display...
        unsigned8 *inPtr1 = (unsigned8 *)pixels.baseAddr;
        unsigned8 *outPtr;
        int iRows = pixels.bounds.bottom - pixels.bounds.top;
        int iCols = pixels.bounds.right - pixels.bounds.left;

        //We need to reallocate the pRGBbuffer if the preview has been resized in the meantime.
        //This was the problem that caused the crash. The buffer wasn't reallocated, so sometimes it was too small.
        //if (!pRGBbuffer) {
        if (pRGBbufferSize != proxyWidth*proxyHeight*(gStuff->inHiPlane+1)){ //3 //Only reallocate if size has to be changed
            
            if (pRGBbuffer) {
                if (pRGBbufferID != 0){
			        UnlockBuffer(pRGBbufferID);
			        FreeBuffer(pRGBbufferID); 
		        } else
			        free(pRGBbuffer);
		        pRGBbufferID = 0;
                pRGBbuffer = NULL;
				pRGBbufferSize = 0;
            }
            
            // Allocate an RGB buffer large enough to cover the entire proxy window.
            if (AllocateBuffer( proxyWidth*proxyHeight * (gStuff->inHiPlane+1), &pRGBbufferID) == noErr){
				pRGBbuffer = (unsigned8 *)LockBuffer(pRGBbufferID, true);
			} else { //Fall back to Windows allocation
				pRGBbuffer = (unsigned8 *)calloc(proxyWidth*proxyHeight, (gStuff->inHiPlane+1)); //3
				pRGBbufferID=0;
			}
            
            if (!pRGBbuffer) {
#if 1
                ErrorOk("Can't allocate proxy preview buffer for 48-bit image!");
#endif
                pRGBbufferSize = 0;
                goto egress;
            
            } else {

                pRGBbufferSize = proxyWidth*proxyHeight*(gStuff->inHiPlane+1); //3
            }
        }

        outPtr = pRGBbuffer;
        while (iRows--) {
            int16 *inPtr = (int16 *)inPtr1;
            int j = iCols;
            while (j--) {
                // Unroll 3 planes...
                int r = inPtr[0];
                int g = inPtr[1];
                int b = inPtr[2];
                outPtr[0] = (r >> 7) | (r >> 15);
                outPtr[1] = (g >> 7) | (g >> 15);
                outPtr[2] = (b >> 7) | (b >> 15);
                if (gStuff->inHiPlane==3) {
                    int a = inPtr[3];
                    outPtr[3] = (a >> 7) | (a >> 15);
                }
                inPtr += pixels.colBytes;
                outPtr += (gStuff->inHiPlane+1); //3
            }//while j
            inPtr1 += pixels.rowBytes;
        }//while iRows
        // Point pixel map to the RGB buffer...
        pixels.baseAddr = pRGBbuffer;
        pixels.rowBytes = iCols*(gStuff->inHiPlane+1); //3
        pixels.colBytes = (gStuff->inHiPlane+1); //3
        pixels.planeBytes = 1;

        if (pixels.imageMode == plugInModeRGB48)
            pixels.imageMode = plugInModeRGBColor;      //data is RGB
        else if (pixels.imageMode == plugInModeLab48)
            pixels.imageMode = plugInModeLabColor; //data is Lab
        else if (pixels.imageMode == plugInModeDeepMultichannel)
            pixels.imageMode = plugInModeMultichannel; //data is Multichannel
    }

    else if (gFmcp->enlargeFactor==1 && pixels.imageMode == plugInModeGray16) {
        // Convert outData from Gray16 to Grayscale for display...
        unsigned8 *inPtr1 = (unsigned8 *)pixels.baseAddr;
        unsigned8 *outPtr;
        int iRows = pixels.bounds.bottom - pixels.bounds.top;
        int iCols = pixels.bounds.right - pixels.bounds.left;
        
        //if (!pRGBbuffer) {
        if (pRGBbufferSize != proxyWidth*proxyHeight*(gStuff->inHiPlane+1)){ //Only reallocate if size has to be changed

            if (pRGBbuffer) {
                if (pRGBbufferID != 0){
			        UnlockBuffer(pRGBbufferID);
			        FreeBuffer(pRGBbufferID); 
		        } else
			        free(pRGBbuffer);
		        pRGBbufferID = 0;
                pRGBbuffer = NULL;
				pRGBbufferSize = 0;
            }

            // Allocate a grayscale buffer large enough to cover the entire proxy window.
            if (AllocateBuffer( proxyWidth*proxyHeight * (gStuff->inHiPlane+1), &pRGBbufferID) == noErr){
				pRGBbuffer = (unsigned8 *)LockBuffer(pRGBbufferID, true);
			} else { //Fall back to Windows allocation
				pRGBbuffer = (unsigned8 *)calloc(proxyWidth*proxyHeight, (gStuff->inHiPlane+1)); //3
				pRGBbufferID=0;
			}


            if (!pRGBbuffer) {
#if 1
                ErrorOk("Can't allocate proxy preview buffer for Gray16 image!");
#endif
                goto egress;
            
            } else {

                pRGBbufferSize = proxyWidth*proxyHeight;
            }
        }
        outPtr = pRGBbuffer;
        while (iRows--) {
            int16 *inPtr = (int16 *)inPtr1;
            int j = iCols;
            while (j--) {
                //int gray = *inPtr;
                int gray = inPtr[0];
                outPtr[0] = (gray >> 7) | (gray >> 15);
                if (gStuff->inHiPlane==1){
                    int a = inPtr[1];
                    outPtr[1] = (a >> 7) | (a >> 15);
                }
                //*outPtr++ = (gray >> 7) | (gray >> 15);
                inPtr += pixels.colBytes;
                outPtr += (gStuff->inHiPlane+1); //2;
            }//while j
            inPtr1 += pixels.rowBytes;
        }//while iRows
        // Point pixel map to the grayscale buffer...
        pixels.baseAddr = pRGBbuffer;
        pixels.rowBytes = iCols*(gStuff->inHiPlane+1);
        pixels.colBytes = (gStuff->inHiPlane+1); //1;
        pixels.planeBytes = 1;
        pixels.imageMode = plugInModeGrayScale;      //data is GrayScale
    }

    //Added by Harald Heim, December 6, 2002
    else if (gFmcp->enlargeFactor==1 && pixels.imageMode == plugInModeCMYK64) {

        // Convert outData from CMYK64 to CMYK for display...
        unsigned8 *inPtr1 = (unsigned8 *)pixels.baseAddr;
        unsigned8 *outPtr;
        int iRows = pixels.bounds.bottom - pixels.bounds.top;
        int iCols = pixels.bounds.right - pixels.bounds.left;
        
        //if (!pRGBbuffer) {
        if (pRGBbufferSize != proxyWidth*proxyHeight*(gStuff->inHiPlane+1)){ //4 //Only reallocate if size has to be changed

            if (pRGBbuffer) {
                if (pRGBbufferID != 0){
			        UnlockBuffer(pRGBbufferID);
			        FreeBuffer(pRGBbufferID); 
		        } else
			        free(pRGBbuffer);
		        pRGBbufferID = 0;
                pRGBbuffer = NULL;
				pRGBbufferSize = 0;
            }

            // Allocate a CMYK buffer large enough to cover the entire proxy window.
            if (AllocateBuffer( proxyWidth*proxyHeight * (gStuff->inHiPlane+1), &pRGBbufferID) == noErr){
				pRGBbuffer = (unsigned8 *)LockBuffer(pRGBbufferID, true);
			} else { //Fall back to Windows allocation
				pRGBbuffer = (unsigned8 *)calloc(proxyWidth*proxyHeight, (gStuff->inHiPlane+1)); //3
				pRGBbufferID=0;
			}

            if (!pRGBbuffer) {
#if 1
                ErrorOk("Can't allocate proxy preview buffer for CMYK64 image!");
#endif
                goto egress;

            } else {

                pRGBbufferSize = proxyWidth*proxyHeight*(gStuff->inHiPlane+1);//4
            }
        }
        outPtr = pRGBbuffer;
        while (iRows--) {
            int16 *inPtr = (int16 *)inPtr1;
            int j = iCols;
            while (j--) {
                // Unroll 4 planes...
                int c = inPtr[0];
                int m = inPtr[1];
                int y = inPtr[2];
                int k = inPtr[3];
                outPtr[0] = (c >> 7) | (c >> 15);
                outPtr[1] = (m >> 7) | (m >> 15);
                outPtr[2] = (y >> 7) | (y >> 15);
                outPtr[3] = (k >> 7) | (k >> 15);
                if (gStuff->inHiPlane==4) {
                    int a = inPtr[4];
                    outPtr[4] = (a >> 7) | (a >> 15);
                }
                inPtr += pixels.colBytes;
                outPtr += (gStuff->inHiPlane+1);//4;
            }//while j
            inPtr1 += pixels.rowBytes;
        }//while iRows
        // Point pixel map to the CMYK buffer...
        pixels.baseAddr = pRGBbuffer;
        pixels.rowBytes = iCols*(gStuff->inHiPlane+1);//4;
        pixels.colBytes = (gStuff->inHiPlane+1);//4;
        pixels.planeBytes = 1;
        pixels.imageMode = plugInModeCMYKColor;      //data is CMYK
    }

    
    //Added by Harald Heim, December 6, 2002
    else if (pixels.imageMode == plugInModeDuotone16) {

        // Convert outData from Duotone16 to Duotone for display...
        unsigned8 *inPtr1 = (unsigned8 *)pixels.baseAddr;
        unsigned8 *outPtr;
        int iRows = pixels.bounds.bottom - pixels.bounds.top;
        int iCols = pixels.bounds.right - pixels.bounds.left;
        
        //if (!pRGBbuffer) {
        if (pRGBbufferSize != proxyWidth*proxyHeight*(gStuff->inHiPlane+1)){ //2 //Only reallocate if size has to be changed

            if (pRGBbuffer) {
                if (pRGBbufferID != 0){
			        UnlockBuffer(pRGBbufferID);
			        FreeBuffer(pRGBbufferID); 
		        } else
			        free(pRGBbuffer);
		        pRGBbufferID = 0;
                pRGBbuffer = NULL;
				pRGBbufferSize = 0;
            }

            // Allocate a Duotone buffer large enough to cover the entire proxy window.
            if (AllocateBuffer( proxyWidth*proxyHeight * (gStuff->inHiPlane+1), &pRGBbufferID) == noErr){
				pRGBbuffer = (unsigned8 *)LockBuffer(pRGBbufferID, true);
			} else { //Fall back to Windows allocation
				pRGBbuffer = (unsigned8 *)calloc(proxyWidth*proxyHeight, (gStuff->inHiPlane+1)); //3
				pRGBbufferID=0;
			}

            if (!pRGBbuffer) {
#if 1
                ErrorOk("Can't allocate proxy preview buffer for Duotone16 image!");
#endif
                goto egress;

            } else {

                pRGBbufferSize = proxyWidth*proxyHeight*(gStuff->inHiPlane+1);//2;
            }
        }
        outPtr = pRGBbuffer;
        while (iRows--) {
            int16 *inPtr = (int16 *)inPtr1;
            int j = iCols;
            while (j--) {
                // Unroll 2 planes...
                int d = inPtr[0];
                int u = inPtr[1];
                outPtr[0] = (d >> 7) | (d >> 15);
                outPtr[1] = (u >> 7) | (u >> 15);
                if (gStuff->inHiPlane==2) {
                    int a = inPtr[3];
                    outPtr[2] = (a >> 7) | (a >> 15);
                }
                inPtr += pixels.colBytes;
                outPtr += (gStuff->inHiPlane+1); //2;
            }//while j
            inPtr1 += pixels.rowBytes;
        }//while iRows
        // Point pixel map to the Duotone buffer...
        pixels.baseAddr = pRGBbuffer;
        pixels.rowBytes = iCols*(gStuff->inHiPlane+1);//2;
        pixels.colBytes = (gStuff->inHiPlane+1);//2;
        pixels.planeBytes = 1;
        pixels.imageMode = plugInModeDuotone;      //data is Duotone
    }
    
#endif


    pixels.mat = NULL;
    pixels.masks = NULL;
    pixels.maskPhaseRow = 0;
    pixels.maskPhaseCol = 0;


	if (gStuff->isFloating || //gStuff->depth == 16 || //Temp preview fix on 16bit layer -> but no transparency chess pattern
        (gStuff->haveMask && gStuff->filterCase == filterCaseProtectedTransparencyWithSelection)) {
       
		mask.next = NULL;

		 if (gFmcp->enlargeFactor != 1){
            //This avoid crash but does not display preview correctly. 
			//We probably need to copy the MyAdjustedMaskData into pRGBbuffer on channel 4!
			mask.maskData = pRGBbuffer+gStuff->inHiPlane;
            mask.rowBytes = pixels.rowBytes;
            mask.colBytes = pixels.colBytes;
		 } else {
			mask.maskData = MyAdjustedMaskData;
			mask.rowBytes = gStuff->maskRowBytes;
			mask.colBytes = 1;
		 }
        mask.maskDescription = kSimplePSMask;
    
        pixels.masks = &mask;

    } else if ((gStuff->inLayerPlanes != 0) && (gStuff->inTransparencyMask != 0) &&
               (gStuff->filterCase != filterCaseProtectedTransparencyNoSelection) &&
               (gStuff->filterCase != filterCaseProtectedTransparencyWithSelection)) {
        // Was wrong for PS Protected Transparency cases, in which
        // case inLayerPlanes = 3 and inTransparencyMask=1, but inColumnBytes = 3,
        // not 4, and the transparency mask is NOT interleaved with the RGB data.
        // In fact, I don't know where the transparency mask data is for this case!
        
        //int multiply=1;
        //if (gStuff->depth == 16) multiply=2

	    mask.next = NULL;
        
        if (gStuff->depth == 16 || gFmcp->enlargeFactor != 1){
            
            mask.maskData = pRGBbuffer+gStuff->inHiPlane;

            mask.rowBytes = pixels.rowBytes;
            mask.colBytes = pixels.colBytes;

        } else {
            mask.maskData = fDragging ?
                        ((char *) MyAdjustedInData) + (gStuff->inHiPlane - gStuff->inLoPlane) :
                        ((char *) gStuff->outData) + (gStuff->outHiPlane - gStuff->outLoPlane);

            mask.rowBytes = fDragging ? gStuff->inRowBytes : gStuff->outRowBytes;
            mask.colBytes = fDragging ? gStuff->inHiPlane - gStuff->inLoPlane + 1
                                  : gStuff->outHiPlane - gStuff->outLoPlane + 1;
        }
        mask.maskDescription = kSimplePSMask;
    
        pixels.masks = &mask;
    }

	//if ( !fDragging) Info ("%d,%d - %d,%d     %d, %d", pixels.bounds.left, pixels.bounds.top, pixels.bounds.right, pixels.bounds.bottom,   itemBounds.left, itemBounds.top);

	//Info ("UIScaling: %d",UIScaling);
	//fmc.hostSig=='PffA' ||
	//&& fmc.hostSig!='PffA'

	if (UIScaling && (pixels.imageMode==plugInModeRGBColor || pixels.imageMode==plugInModeGrayScale) ) { //Compensate UIScaling bug

        BITMAPINFO info;
		int w = pixels.bounds.right - pixels.bounds.left;
		int h = pixels.bounds.bottom - pixels.bounds.top;
		int size;
		BYTE * buffer;
		BufferID bufferID = 0;
		BYTE * buffer2 = (BYTE *)pixels.baseAddr;
		int count=0,count2,x,y;

        //startClock();


		//Info ("Here");

		//Only width and heights with a multiple of 4 are displayed correctly 
		if (w%4!=0) w = (w/4)*4;
		if (h%4!=0) h = (h/4)*4;
		size = w*h*3;

		wRect.right -= (pixels.bounds.right - pixels.bounds.left - w);
		wRect.bottom -= (pixels.bounds.bottom - pixels.bounds.top - h);
		FrameRect(hDC, &wRect, (HBRUSH)GetStockObject(BLACK_BRUSH));


		/*SPErr error;
		PSColorSpaceSuite2 *sPSColorSpace;
		error = sSPBasic->AcquireSuite(kPSColorSpaceSuite, kPSColorSpaceSuiteVersion2, (const void**)&sPSColorSpace);
		if (!error){
			buffer2 = malloc(size);
			sPSColorSpace->ConvertToMonitorRGB(plugIncolorServicesRGBSpace,pixels.baseAddr,buffer2,size);
			sSPBasic->ReleaseSuite(kPSColorSpaceSuite,kPSColorSpaceSuiteVersion2);
		}*/
		
		if (AllocateBuffer(size, &bufferID) == noErr){
			buffer = (BYTE *)LockBuffer(bufferID, true);
		} else { //Fall back to Windows allocation
			buffer = (BYTE *)malloc(size);
		}
		
		//Swap color channels for StretchDIBits //Mirror vertically and 
		if (pixels.imageMode==plugInModeGrayScale){

			//for (y=h-1;y>=0;y--){
			for (y=0;y<h;y++){
				count2 = y*pixels.rowBytes; //w*pixels.colBytes;
			for (x=0;x<w;x++){
				if (gStuff->inHiPlane!=1 || buffer2[count2-1]==255){
					buffer[count] = buffer[count+1] = buffer[count+2] = 
						buffer2[count2];
				} else { //Add checker board
					int m = buffer2[count2-1];
					int xx = x%16;
					int yy = y%16;
					int c = (xx<8 && yy<8) || (xx>=8 && yy>=8) ? 255 : 192;
					if (m==0){
						buffer[count] = buffer[count+1] = buffer[count+2] = c;
					} else {
						buffer[count] = buffer[count+1] = buffer[count+2] = 
							((255-m)*c + m* buffer2[count2])/255;
					}
				}
				count+=3;
				count2+=pixels.colBytes;
			}}

		} else {

			//for (y=h-1;y>=0;y--){
			for (y=0;y<h;y++){
				count2 = y*pixels.rowBytes; //w*pixels.colBytes;
			for (x=0;x<w;x++){
				if (gStuff->inHiPlane!=3 || buffer2[count2-1]==255){
					buffer[count] = buffer2[count2+2];
					buffer[count+1] = buffer2[count2+1];
					buffer[count+2] = buffer2[count2];
				} else { //Add checker board
					int m = buffer2[count2-1];
					int xx = x%16;
					int yy = y%16;
					int c = (xx<8 && yy<8) || (xx>=8 && yy>=8) ? 255 : 192;
					if (m==0){
						buffer[count] = buffer[count+1] = buffer[count+2] = c;
					} else {
						buffer[count] = ((255-m)*c + m* buffer2[count2+2])/255;
						buffer[count+1] = ((255-m)*c + m* buffer2[count2+1])/255;
						buffer[count+2] = ((255-m)*c + m* buffer2[count2])/255;
					}
				}
				count+=3;
				count2+=pixels.colBytes;
			}}
		}

		ZeroMemory(&info, sizeof(BITMAPINFO));
		info.bmiHeader.biBitCount = 24;
		info.bmiHeader.biWidth = w;
		info.bmiHeader.biHeight = -h; //top down image
		info.bmiHeader.biPlanes = 1;
		info.bmiHeader.biSize = sizeof(BITMAPINFOHEADER);
		info.bmiHeader.biSizeImage = 0; //size;
		info.bmiHeader.biCompression = BI_RGB;
                
		StretchDIBits(hDC, itemBounds.left, itemBounds.top, w, h, 0, 0, w, h, buffer, &info, DIB_RGB_COLORS, SRCCOPY);

        //DebugLogVal(stopClock());

		
		if (buffer){
			if (bufferID != 0){
				UnlockBuffer(bufferID);
				FreeBuffer(bufferID); 
			} else
				free(buffer);
			bufferID = 0;
			buffer = NULL;
		}

		//if (buffer2!=pixels.baseAddr)
		//	free(buffer2);

	} else {

		FrameRect(hDC, &wRect, (HBRUSH)GetStockObject(BLACK_BRUSH));

        //startClock();

		(gStuff->displayPixels)(&pixels, &pixels.bounds, itemBounds.top, itemBounds.left, (void *)hDC);

        //DebugLogVal(stopClock());

	}


#if 1
    }//if intersects
#endif
egress:
    EndPaint(hDlg, (LPPAINTSTRUCT) &ps);

    fDrawing = false;
	
	//DRAWITEM event for preview
	if (gParams->ctl[CTL_PREVIEW].properties & CTP_DRAWITEM){            
        PerformControlAction(
                CTL_PREVIEW,             //control index
                FME_DRAWITEM,           //FM event code
                0,                      //previous value
                WM_DRAWITEM,            //message type
                0,                      //notification code
                FALSE,                  //fActionable
                TRUE,0);                  //fProcessed
    }




} /*ShowOutputBuffer*/

#endif //APP




void UpdateProxy (HWND hDlg, BOOL bEraseBackground)
{
    HWND hProxy;
    RECT imageRect;

    static long previousScaleFactor = -1; //force initial background erase. //Thread Safe????
	static long prevEnlargeFactor = -1;
#ifdef APP
    static int image = 0;
#endif

    // Question: Does this automatically take care of all
    // needed background erases?  If so, we can delete the
    // logic that does this everywhere else...
    // To test this, uncomment the following line:
    bEraseBackground = FALSE;
    if (scaleFactor > previousScaleFactor || gFmcp->enlargeFactor != prevEnlargeFactor) {
        // If scale factor has increased, proxy image will
        // be smaller, so we need to erase the background.
        // (Actually, don't need to erase background if image
        // is still clipped to preview window at the new scale
        // factor -- we can do this optimization later if
        // unneeded flashing is a problem.)
        bEraseBackground = TRUE;
    }
    previousScaleFactor = scaleFactor;
	prevEnlargeFactor = gFmcp->enlargeFactor;

#ifdef APP
    if (image != app.inputIndex) bEraseBackground = TRUE; 
    image = app.inputIndex;
#endif

    // Invalidate Proxy Item
    hProxy = GetDlgItem(hDlg, IDC_PROXY);
    GetWindowRect(hProxy, &imageRect);
    ScreenToClient (hDlg, (LPPOINT)&imageRect);
    ScreenToClient (hDlg, (LPPOINT)&(imageRect.right));
    InvalidateRect (hDlg, &imageRect, bEraseBackground);

} /*UpdateProxy*/

/*****************************************************************************/

int doAction(CTLACTION action)
{
    int iCtl; // Added by Ognen Genchev
    //Perform the specified action for this control,,,
    switch(action) {
    
    case CA_SIZE: //Invoke FME_SIZE && FME_EXITSIZE events

        gFmcp->n = 0;
        gFmcp->e = FME_SIZE;
        gFmcp->previous = 0;
        gFmcp->ctlMsg = WM_SIZE;
        gFmcp->ctlCode = 0;
        gFmcp->ctlClass = gParams->ctl[0].ctlclass;

        call_FFP(FFP_OnCtl,(INT_PTR)gFmcp); //OnCtl(gFmcp);
        
        //Make sure that the preview is displayed correctly
        GetProxyItemRect (MyHDlg, &gProxyRect); 
        SetupFilterRecordForProxy (MyGlobals, &scaleFactor, scaleFactor, &imageOffset);
        gFmcp->n = 0;
        gFmcp->e = FME_EXITSIZE;
        gFmcp->previous = 0;
        gFmcp->ctlMsg = WM_EXITSIZEMOVE;
        gFmcp->ctlCode = 0;
        gFmcp->ctlClass = gParams->ctl[0].ctlclass;

        call_FFP(FFP_OnCtl,(INT_PTR)gFmcp); //OnCtl(gFmcp);
        break;

    case CA_PREVIEW:
        // Update the preview...
        if (!fDragging) {
            if (gResult == noErr) {
                // Do Filtering operation
                DoFilterRect (MyGlobals);
                // Invalidate Proxy Item
                UpdateProxy(MyHDlg, FALSE);
            }//noErr
        }
        break;
    case CA_APPLY:
		okEventPerformed = 1;//OK Button was pressed

        /*Fix by Ognen Genchev
            Cancel button had the same behaviour as the OK button, remembering the modified value.
        Now it is fixed by storing .val in .initVal thus when pressing cancel button, the previous
        value is remembered.
            Another thing is, default initial values were not stored properly upon CANCEL execution.
            The value written in .cancelVal at plugin initialization fixes this.
      */
        for (iCtl = 0;  iCtl < N_CTLS;  iCtl++) {
            gParams->ctl[iCtl].initVal = gParams->ctl[iCtl].val;
            gParams->ctl[iCtl].cancelVal = gParams->ctl[iCtl].val;
            // end
        }

        // Apply filter to original source image...
        SendMessage(MyHDlg, WM_COMMAND, IDOK, 0);
        break;
    case CA_CANCEL:
        cancelEventPerformed = 1;//Cancel Button was pressed
        // Fix by Ognen Genchev for proper cancel event execution
        for (iCtl = 0;  iCtl < N_CTLS;  iCtl++) {
            gParams->ctl[iCtl].initVal = gParams->ctl[iCtl].cancelVal;
        }
        // end
		// Exit without doing anything...
        SendMessage(MyHDlg, WM_COMMAND, IDCANCEL, 0);
        break;
    case CA_ABOUT:
        // Display the ABOUT dialog box...
        //SendMessage(MyHDlg, WM_COMMAND, IDC_HELP2, -1);
		DoAbout(globals,2);
        break;
    case CA_RESET:
        //Info("action=RESET is not yet implemented.");
        break;
    case CA_NONE:
    default:
        // No action...
        break;
    }//switch action
    return TRUE;
} /*DoAction*/


BOOL PerformControlAction(int n, FME e, int previous, int ctlMsg, int ctlCode,
                          BOOL fActionable, BOOL fProcessed, int overrideBusy)
{
    static int busy = 0;

    if (n < 0 || n >= N_CTLS) return FALSE;

    // Prevent reentry if reentry count != 0
    if (busy && overrideBusy==0) {
#if defined(_DEBUG) && 0
        OutputDebugString("PerformControlAction is BUSY!\n");
#endif
        return FALSE;
    }
    busy++;

#if SKIN
	if (isSkinActive() && gParams->skinStyle>0){
		if (gParams->ctl[n].ctlclass == CC_PUSHBUTTON || gParams->ctl[n].ctlclass == CC_COMBOBOX || gParams->ctl[n].ctlclass == CC_STANDARD || gParams->ctl[n].ctlclass == CC_CHECKBOX){
			if ( (e==FME_MOUSEOVER)) skinDraw(n,3);
			else if (e==FME_MOUSEOUT) skinDraw(n,0);
		}
	}
#endif


#ifdef APP
    if (performAppAction(n, e, previous, ctlMsg, ctlCode, fActionable, fProcessed, overrideBusy)){
        busy--;
        return fProcessed;
    }
#endif

	{
        int res;
        gFmcp->n = n;
        gFmcp->e = e;
        //Added by Harald Heim, May 22, 2003
        gFmcp->previous = previous;

        gFmcp->ctlMsg = ctlMsg;
        gFmcp->ctlCode = ctlCode;
        gFmcp->ctlClass = gParams->ctl[n].ctlclass;

        res = call_FFP(FFP_OnCtl,(INT_PTR)gFmcp); //res = OnCtl(gFmcp);

        if (res) {
            busy--; //decrement reentry count
            return TRUE;   //fully processed, skip default action
        }
    }
    //Perform the default action for this control...
    if (fActionable) {
        doAction(gParams->ctl[n].action);
    }

    busy--; //decrement reentry count
    return fProcessed;   //processed or not
} /*PerformControlAction*/

/*****************************************************************************/

static int xFrom, xTo;
static int yFrom, yTo;

static void scaleControl (HWND hDlg, int nID)
{
    RECT r;
    HWND hCtl = GetDlgItem(hDlg, nID);

    if (hCtl) {
        GetWindowRect(hCtl, &r);                        //screen coords
        MapWindowPoints(NULL, hDlg, (LPPOINT)&r, 2);    //...to client coords
        // Undo the scaling...
        r.left   = (r.left * xTo)/xFrom;    // Order of multiply/divide is important!!
        r.right  = (r.right * xTo)/xFrom;
        r.top    = (r.top * yTo)/yFrom;
        r.bottom = (r.bottom * yTo)/yFrom;
        // Resize and reposition the window.
        SetWindowPos(hCtl, NULL,
                     r.left, r.top,
                     r.right - r.left,
                     r.bottom - r.top,
                     SWP_NOZORDER | SWP_NOACTIVATE);
    }//if
} /*scaleControl*/

static void scaleWindow (HWND hwnd)
{
    RECT r;

    GetWindowRect(hwnd, &r);    //screen coords
    SetWindowPos(hwnd, NULL,
                 0, 0,
                 (r.right - r.left)*xTo/xFrom,
                 (r.bottom - r.top)*yTo/yFrom,
                 SWP_NOMOVE | SWP_NOZORDER | SWP_NOACTIVATE);

} /*scaleWindow*/



/*****************************************************************************/

// Search PATH and FM_PATH for specified file name.
// Returns pointer to full path name if found, else
// returns pointer to original file name.
// CAUTION: Full path name is stored in a static buffer!

const char *FmSearchFile(const char *szFileName)
{
    static char pathbuffer[_MAX_PATH+1];

    /* Search for file in PATH environment variable: */
    _searchenv(szFileName, "PATH", pathbuffer);
    if (*pathbuffer != '\0') {
        //found it in PATH search list...
#if 0
        Info("Found file in PATH:\n%s", pathbuffer);
#endif
        return pathbuffer;
    }
    else {
        /* Search for file in FM_PATH environment variable: */
        _searchenv(szFileName, "FM_PATH", pathbuffer);
        if (*pathbuffer != '\0') {
            //found it in FM_PATH search list...
#if 0
        Info("Found file in FM_PATH:\n%s", pathbuffer);
#endif
            return pathbuffer;
        }
    }
    // If not found in PATH or FM_PATH, then just return
    // the file name verbatim...
#if 0
    Info("Not found in PATH or FM_PATH:\n%s", szFileName);
#endif
    return szFileName;
} /*FmSearchFile*/




LRESULT CALLBACK KeyHookProc( int code, WPARAM wParam, LPARAM lParam){
	
	if (code==HC_ACTION){
		
        if (GetForegroundWindow() == MyHDlg){ //Only run for foreground window

            //Not in combo boxes, only if Alt additionally
            if ( (wParam >= 0x41 && wParam <= 0x5A) || wParam == VK_SUBTRACT ){
                if (getCtlClass(getCtlFocus()) == CC_COMBOBOX) 
                    if (getAsyncKeyStateF(VK_MENU) > -32767)
                        return CallNextHookEx ( KeyHook, code, wParam, lParam);
            }

            //if (wParam==WM_KEYDOWN){
		    if (!(lParam & 2147483648)){ //Check if bit 31 is not set -> key down
		        
				if (!(lParam & 1073741824)){ //Do not call repeatedly when key is pressed down

					//KBDLLHOOKSTRUCT  *pKeyBoard = (KBDLLHOOKSTRUCT *)lParam;
					//triggerEvent((int)pKeyBoard->vkCode,FME_KEYDOWN,0);
					
					triggerEvent((int)wParam,FME_KEYDOWN,0);
				}

		    } else { //WM_KEYUP

			    triggerEvent((int)wParam,FME_KEYUP,0);

		    }

        }

	}
	
	return CallNextHookEx ( KeyHook, code, wParam, lParam);

}
 
/*
LRESULT CALLBACK LLKeyHookProc( int code, WPARAM wParam, LPARAM lParam){
	
	if (code==HC_ACTION){
		
        if (GetForegroundWindow() == MyHDlg){ //Only run for foreground window

            KBDLLHOOKSTRUCT *hks = (KBDLLHOOKSTRUCT *)lParam;

            //Not in combo boxes, only if Alt additionally
            if ( (wParam >= 0x41 && wParam <= 0x5A) || wParam == VK_SUBTRACT ){
                if (getCtlClass(getCtlFocus()) == CC_COMBOBOX) 
                    if (getAsyncKeyStateF(VK_MENU) > -32767)
                        return CallNextHookEx ( KeyHook, code, wParam, lParam);
            }

            if (wParam==WM_KEYDOWN || wParam==WM_SYSKEYDOWN){
		    
                Info ("keydown");
                triggerEvent(hks->vkCode,FME_KEYDOWN,0);

		    } else { //WM_KEYUP

                Info ("keyup");
			    triggerEvent(hks->vkCode,FME_KEYUP,0);

		    }

        }

	}
	
	return CallNextHookEx ( KeyHook, code, wParam, lParam);

}
*/


static OPENFILENAME ofn;
static char szFileName[_MAX_PATH + 1];
//char szTitleName[_MAX_FNAME + _MAX_EXT + 1];
FILETIME szFileTime;

// Added by Ognen Genchev
/*void Triangle (HDC hdc, POINT pt[])
{
    int iCtl;
    HPEN hPen = CreatePen(PS_SOLID, 2, gParams->ctl[0].shapeColor); //iCtl is unknown
    SelectObject (hdc, hPen);
    Polygon (hdc, pt, 3);
    SelectObject (hdc, hPen);
}*/
// end

INT_PTR WINAPI FilterDialogProc(HWND hDlg, UINT wMsg, WPARAM wParam, LPARAM lParam)       // Win32 Change
{

//#if defined(USE_BITMAP) || defined(FML)
    // See BDLG32 (msdn sample 4544)
    static HBITMAP hBmpDialog;  // Handle of bitmap to display as dialog background.
    static HBITMAP hBmpOld;     // Handle of old bitmap to restore.
    static BITMAP  bmInfo;      // Bitmap information structure.
    static HDC     hdcMem;      // Compatible Memory DC for dialog.
    static POINT   pt;          // Position for upper left corner of bitmap.
    static SIZE    size;        // Size (width and height) of dialog client rectangle.
//#endif
    HWND  hCtrl;
    
    //Move up to top level
    //static HCURSOR hcHand;
    
    
    int idd;    // WIN32 Change
    int iCtl;
    static int iCtl2;
    int iCmd;
    long   prevScaleFactor = scaleFactor;
    static POINT ptPrev, ptNew;

    // Added by OgnenGenchev
    // int cx, cy;
    // POINT ptt[3];
    // end

    //Moved to main level of AfhFMUI.c, Harald Heim. Dec 14, 2002
    //static POINT imageOffset = {0, 0};


#ifndef NOOPTIMZE_QUICK_TOGGLE
    static LONG fMouseMoved = 0;
#endif
    static GPtr globals = NULL;         /* needs to be static */
    static int iCurrentMouseoverCtl = -1;   //should re-init whenever controls change
                                            //e.g. after a recompile
#if 0
    static HACCEL hAccel;
#endif

    switch  (wMsg) {

        case  WM_INITDIALOG:
        {
            /* set up globals   */
            globals = (GPtr) lParam;
            gFmcp->hDlg = MyHDlg = hDlg;

            // Added by Ognen Genchev, 
            // GDI+ init once
            static ULONG_PTR gdiplusToken = 0;
            if (!gdiplusToken) {
                Gdiplus::GdiplusStartupInput gdiplusStartupInput;
                Gdiplus::GdiplusStartup(&gdiplusToken, &gdiplusStartupInput, NULL);
            }

            // Example dial
            DialConfig baseDial = {};
            baseDial.index = 0;
            baseDial.hwnd = NULL;
            baseDial.angle = 0.0f;
            baseDial.dragging = FALSE;
            baseDial.x = 20;
            baseDial.y = 20;
            baseDial.size = 100;

            baseDial.dialRadius = 30;
            baseDial.tickLength = 5;
            baseDial.ellipseThickness = 2;
            baseDial.tickThickness = 3;
            baseDial.dotRadius = 5;

            // Use COLORREF (RGB) instead of Gdiplus::Color
            baseDial.dotColor        = RGB(255, 0, 0);
            baseDial.tickColor       = RGB(50, 50, 50);
            baseDial.ellipseColor    = RGB(0, 0, 0);
            baseDial.backgroundColor = RGB(220, 220, 220);

            // DialConfig dial2 = baseDial;
            // dial2.dotColor = RGB(0, 255, 0);
            wcscpy_s(baseDial.fontName, L"Arial");
            // wcscpy_s(dial2.fontName, L"Arial");


            CreateCustomDialControl(hDlg, (HINSTANCE)hDllInstance, 284, 112, 100, (HMENU)(INT_PTR)(IDC_CTLBASE+hCtrl), &baseDial);
            // CreateCustomDialControl(hDlg, (HINSTANCE)hDllInstance, 284, 215, 100, (HMENU)(INT_PTR)(IDC_CTLBASE+hCtrl), &dial2);

            Gdiplus::GdiplusShutdown(gdiplusToken);
            // end

			#ifdef GDIPLUS
			{
				void initGraphicsPlus();
				initGraphicsPlus();
			}
			#endif

			UIScaling = isHDPIAware(); //checkUIScaling();


            // Create the Tool Tip control first! -----------------------------------------------------------------
            //-----------------------------------------------------------------------------------------------------
#if 1
            {//scope
                INITCOMMONCONTROLSEX iccex;
                BOOL status;

                iccex.dwSize = sizeof(INITCOMMONCONTROLSEX);
                iccex.dwICC = ICC_WIN95_CLASSES;  //all Win95 classes
                status = InitCommonControlsEx(&iccex);
#if 0
                Info("InitCommonControlsEx status = %x", status);
#endif
            }//end scope
#else
            // Should link dynamically, so only load comctl32.dll
            // if necessary!
            InitCommonControls();
#endif


            ghTT = CreateWindowEx(  
                0,         // extended styles -- WS_EX_TOPMOST ???
                TOOLTIPS_CLASS,   // predefined class 
                NULL,       // text 
				WS_POPUP | TTS_ALWAYSTIP | TTS_NOPREFIX | WS_BORDER,
                CW_USEDEFAULT,         // starting x position 
                CW_USEDEFAULT,         // starting y position 
                CW_USEDEFAULT,        // button width 
                CW_USEDEFAULT,        // button height 
                hDlg,       // parent window 
                NULL,
                (HINSTANCE)hDllInstance,
                NULL);      // pointer not needed 
#if 0
            Info("Tool Tip control created, ghTT = %x,\n"
                 "                          hDlg = %x,\n"
                 "                         hDllInstance = %x",
                 ghTT, hDlg, hDllInstance);
#endif

#if 1
            // Set max tooltip width...
            SendMessage(ghTT, TTM_SETMAXTIPWIDTH, 0, 300);
#endif

#if 1
            // Set tooltip background color...
            //SendMessage(ghTT, TTM_SETTIPBKCOLOR, (COLORREF)0xcccccc, 0);
            // Set tooltip text color...
            //SendMessage(ghTT, TTM_SETTIPTEXTCOLOR, RGB(255,0,0), 0);
#endif

#if 0
            // Set a title and icon for balloon tips...
            SendMessage(ghTT, TTM_SETTITLE, TTI_INFO, (LPARAM)"FilterMeister says:");
#endif

#if 1
            // Set tooltip delay times...
            {
                int iTime = 5000;   // initial show time in milliseconds
#if 0
                // TTDT_AUTOMATIC sets autopop = initial*10, reshow = initial/5...
                SendMessage(ghTT, TTM_SETDELAYTIME, TTDT_AUTOMATIC, MAKELONG(iTime, 0));
#elif 1
                // TTDT_AUTOMATIC with initial < 0 sets all values to default...
                SendMessage(ghTT, TTM_SETDELAYTIME, TTDT_AUTOMATIC, MAKELONG(-1, 0));
#else
                // Set each delay parameter indidually...
                SendMessage(ghTT, TTM_SETDELAYTIME, TTDT_INITIAL, MAKELONG(8000, 0));
                SendMessage(ghTT, TTM_SETDELAYTIME, TTDT_AUTOPOP, MAKELONG(10000, 0));
                SendMessage(ghTT, TTM_SETDELAYTIME, TTDT_RESHOW, MAKELONG(3000, 0));
#endif
            }
#endif

            // Per MSDN, we must explicitly define a ToolTip
            // control as topmost...
#if 0
            // but it screws up z-pos of common dialogs such as Load/Save!!!
            SetWindowPos(ghTT, HWND_TOPMOST, 0, 0, 0, 0,
                         SWP_NOMOVE | SWP_NOSIZE | SWP_NOACTIVATE);
#endif

            //-----------------------------------------------------------------------------------------------------
            //-----------------------------------------------------------------------------------------------------


            // Save or restore dialog styles...
#ifndef ONWINDOWSTART
            if (gParams->gInitialInvocation) {
#endif
                //On first invocation, save initial dialog styles.
                gParams->gDialogInitialStyle =
                gParams->gDialogStyle = (int)GetWindowLong(hDlg, GWL_STYLE);
                gParams->gDialogInitialStyleEx =
                gParams->gDialogStyleEx = (int)GetWindowLong(hDlg, GWL_EXSTYLE);

#ifndef ONWINDOWSTART
            }
            else {
				//On subsequent invocations, restore styles in effect
                //when dialog last exited (but turn off VISIBLE bit for now).
    #if 0
                Info("Prev style = %x\r\nCurrent style = %x",
                        gParams->gDialogStyle,
                        GetWindowLong(hDlg, GWL_STYLE));
                Info("Prev styleEx = %x\r\nCurrent styleEx = %x",
                        gParams->gDialogStyleEx,
                        GetWindowLong(hDlg, GWL_EXSTYLE));
    #endif
                SetWindowLong(hDlg, GWL_STYLE, gParams->gDialogStyle & ~WS_VISIBLE);
                SetWindowLong(hDlg, GWL_EXSTYLE, gParams->gDialogStyleEx);
            }
#endif

// #ifndef to #ifdef modified by Ognen Genchev for proper initialization.
#ifdef ONWINDOWSTART
            // Restore dialog position and size, and predefined controls...
            if (gParams->gInitialInvocation) {
#endif
// end
                //Initial invocation...
#if ONWINDOWSTART
    #ifndef PHOTOFREEBIES //if not Photofreebies
                //clearDialogStyle(WS_CAPTION);
                setDialogPos(FALSE, -1, 0, 529,332); //388, 142);
                //setCtlPos(CTL_PREVIEW,5,42,529-184,332-65);
    #endif
#else
                /*************************************************************************/
                /*  COMMENTED OUT by Ognen Genchev to fix the problem when reinitializing
                /*  the plugin in case user made modifications to the dialog positioning
                /*  and size.
                /*************************************************************************/
                // setDialogPos(FALSE, -1, 0, 485,178); //388, 142);
#endif

                //Now define the default controls:
                //???? Could just pre-fill the ctls array with all the
                //???? right parameters, and let "Initialize user controls"
                //???? do all the dirty work?  But would also have to do
                //???? that in resetAllCtls...
                // Ognen Genchev. Initialize custom control's state.
                iCtl = NULL;
                setCustomCtl(iCtl, gParams->ctl[iCtl].state);
                // end
                createPredefinedCtls();

// #ifndef to #ifdef modified by Ognen Genchev for proper initialization.
#ifdef ONWINDOWSTART
            } else {
                //On subsequent invocations, use coordinates that were
                //saved at previous exit...
            
                //if (gParams->gDialogState == 0){

					SetWindowPos(hDlg, NULL,
                             gParams->gDialogRect.left,
                             gParams->gDialogRect.top,
                             gParams->gDialogRect.right - gParams->gDialogRect.left,
                             gParams->gDialogRect.bottom - gParams->gDialogRect.top,
                             SWP_NOZORDER | SWP_NOACTIVATE);

                /*} else { //Dialog was maximized before
                
					SetWindowPos(hDlg, NULL,
                             gParams->gDialogRectMin.left,
                             gParams->gDialogRectMin.top,
                             gParams->gDialogRectMin.right - gParams->gDialogRectMin.left,
                             gParams->gDialogRectMin.bottom - gParams->gDialogRectMin.top,
                             SWP_NOZORDER | SWP_NOACTIVATE);

                    ShowWindow (hDlg, SW_MAXIMIZE); //Maximize Window

                }*/
            	
				// Restore/refresh dialog theme...
				//Info ("%d",gParams->gDialogTheme);
				//setDialogTheme((int) gParams->gDialogTheme);
				
                //Avoid that the XP theme is reactivated on second invocation
				//setDefaultWindowTheme(GetDlgItem(hDlg, IDC_PROGRESS1));
                //setDefaultWindowTheme(GetDlgItem(hDlg, IDC_BUTTON1));
                //setDefaultWindowTheme(GetDlgItem(hDlg, IDC_BUTTON2));
                //setDefaultWindowTheme(GetDlgItem(hDlg, IDC_PROXY_GROUPBOX));
            }

			// setDialogTheme((int) gParams->gDialogTheme);

			
			// Restore any dialog background image.
            //Need to make this pointer relative to gParams????
            gDialogImage = gParams->gDialogImage_save;

            // Restore any dialog clipping region.
            if (gParams->gDialogRegion != NULL) {
                SetWindowRgn(
                    hDlg,   // handle to window whose window region is to be set
                    gParams->gDialogRegion, // handle to region 
                    FALSE   // window redraw flag 
                       );
            }
#endif
// end
            hcHand = LoadCursor((HINSTANCE)hDllInstance, MAKEINTRESOURCE(IDC_CURSOR1));
			cursorResource = IDC_CURSOR1;


//#ifndef NOISECONTROL
            // Do this first, to compute scaleFactor...
            GetProxyItemRect (hDlg, &gProxyRect); 
            SetupFilterRecordForProxy (globals, &scaleFactor, scaleFactor, &imageOffset);
            // But don't run the filter until all other params are computed...
            //DoFilterRect (globals);

            // Set focus to OK button.
            SetFocus (GetDlgItem (hDlg, IDX_OK));

			#if OLDZOOMLABEL || TESTSHELL
				// Give the zoom buttons a more readable font...
				// (But then the focus rectangles overwrite the font????)
				SendDlgItemMessage(hDlg, IDC_BUTTON1, WM_SETFONT,
								   (WPARAM) GetStockObject(SYSTEM_FIXED_FONT),
								   FALSE /*redraw flag*/);
				SendDlgItemMessage(hDlg, IDC_BUTTON2, WM_SETFONT,
								   (WPARAM) GetStockObject(SYSTEM_FIXED_FONT),
								   FALSE /*redraw flag*/);

                // Enable/disable zoom buttons...
                EnableWindow(GetDlgItem(hDlg, IDC_BUTTON1), scaleFactor > 1);
                EnableWindow(GetDlgItem(hDlg, IDC_BUTTON2), scaleFactor < MAX_SCALE_FACTOR);
            #endif
//#endif

// ------------------------------------------------------------------------------------------------
// ------------------------------------------------------------------------------------------------

#ifndef APP
// #ifndef to #ifdef modified by Ognen Genchev for proper reinitialization.
#ifdef ONWINDOWSTART
// end
            // Initialize user controls
            if (!gParams->gInitialInvocation){       //does this break standalones???

                //Create Tab controls first
                for (iCtl = 0;  iCtl < N_CTLS;  iCtl++){
                    
                    if (gParams->ctl[iCtl].ctlclass == CC_TAB){
                    
                        int  val       = gParams->ctl[iCtl].val;
                        int  initVal   = gParams->ctl[iCtl].initVal;
                        int  minval    = gParams->ctl[iCtl].minval;
                        int  maxval    = gParams->ctl[iCtl].maxval;
                        int  lineSize  = gParams->ctl[iCtl].lineSize;
                        int  pageSize  = gParams->ctl[iCtl].pageSize;
					    int  thumbSize  = gParams->ctl[iCtl].thumbSize;
                        int  imageType = gParams->ctl[iCtl].imageType;
                        int  state = gParams->ctl[iCtl].state;
                        int  divisor   = gParams->ctl[iCtl].divisor;
                        int  ticFreq   = gParams->ctl[iCtl].tb.ticFreq;
                        CTLACTION action = gParams->ctl[iCtl].action;
                        COLORREF textColor = gParams->ctl[iCtl].textColor;
                        COLORREF bkColor = gParams->ctl[iCtl].bkColor;
                        COLORREF shapeColor = gParams->ctl[iCtl].shapeColor; // Added by Ognen Genchev
                        char tooltip[MAX_TOOLTIP_SIZE+1];
                        char image[_MAX_PATH+1];
					    int noeditborder=false;
					    int anchor = gParams->ctl[iCtl].anchor;
					    int tabControl = gParams->ctl[iCtl].tabControl;
					    int tabSheet = gParams->ctl[iCtl].tabSheet;
                        int scripted = gParams->ctl[iCtl].scripted;

                        strcpy(tooltip, gParams->ctl[iCtl].tooltip);
                        //strcpy(image, gParams->ctl[iCtl].image);

					    //if (!(gParams->ctl[iCtl].buddy1Style & WS_BORDER)) noeditborder=true;
					    
                        gParams->ctl[iCtl].inuse = FALSE;  //so fm_createCtl can work ???
                                                            //without trying to delete
                                                            //stale handles, etc. ???
                        createCtl(iCtl,
                                     gParams->ctl[iCtl].ctlclass,         //class
                                     gParams->ctl[iCtl].label,    //text
                                     gParams->ctl[iCtl].xPos,          //x
                                     gParams->ctl[iCtl].yPos,          //y
                                     gParams->ctl[iCtl].width,         //w
                                     gParams->ctl[iCtl].height,        //h
                                     gParams->ctl[iCtl].style,         //styles
                                     gParams->ctl[iCtl].styleEx,       //extended styles
                                     gParams->ctl[iCtl].properties,    //properties
                                     gParams->ctl[iCtl].enabled);      //visible/enabled
                    
                        //restore clobbered attributes
                        // setCtlTheme(iCtl, gParams->ctl[iCtl].theme); // Overwrites the background color if theme enabled
					    setCtlTab(iCtl, tabControl, tabSheet);		// Overwrites the background color if theme enabled
                        gParams->ctl[iCtl].initVal = initVal;
                        setCtlRange(iCtl, minval, maxval);
                        setCtlLineSize(iCtl, lineSize);
                        setCtlPageSize(iCtl, pageSize);
					    // setCtlThumbSize(iCtl, thumbSize);
                        setCtlAction(iCtl, action);
                        /******************************************************/
                        /*  MODIFICATION by Ognen Genchev
                        /*  fm_setCtlVal() receives value from initVal to solve
                        /*  the problem with writing value modification on
                        /*  cancelling opertion.
                        /*  See case CA_APPLY comment bellow
                        /******************************************************/
                        setCtlVal(iCtl, initVal);
                        // end
                        setCtlFontColor(iCtl, textColor);
                        setCtlColor(iCtl, bkColor);
                        setFrameColor(iCtl, shapeColor); // Added by Ognen Genchev
                        setCtlToolTip(iCtl, tooltip, 0);
                        setCtlImage(iCtl, image, imageType);
                        setCustomCtl(iCtl, state); // Added by Ognen Genchev
                        setCtlDivisor(iCtl, divisor);
					    setCtlAnchor(iCtl, anchor);
                        setCtlScripting(iCtl, scripted);
                    }
                }

			 

              //.. then create others...
              for (iCtl = 0;  iCtl < N_CTLS;  iCtl++)
              {
                if (gParams->ctl[iCtl].inuse && gParams->ctl[iCtl].ctlclass != CC_TAB)
                {
#if 1
                    //createCtl clobbers these, so save them for later
                    int  val       = gParams->ctl[iCtl].val;
                    int  initVal   = gParams->ctl[iCtl].initVal;
                    int  minval    = gParams->ctl[iCtl].minval;
                    int  maxval    = gParams->ctl[iCtl].maxval;
                    int  lineSize  = gParams->ctl[iCtl].lineSize;
                    int  pageSize  = gParams->ctl[iCtl].pageSize;
					int  thumbSize  = gParams->ctl[iCtl].thumbSize;
                    int  imageType = gParams->ctl[iCtl].imageType;
                    int  state = gParams->ctl[iCtl].state;
                    int  divisor   = gParams->ctl[iCtl].divisor;
                    int  ticFreq   = gParams->ctl[iCtl].tb.ticFreq;
                    CTLACTION action = gParams->ctl[iCtl].action;
                    COLORREF textColor = gParams->ctl[iCtl].textColor;
                    COLORREF bkColor = gParams->ctl[iCtl].bkColor;
                    char tooltip[MAX_TOOLTIP_SIZE+1];
                    char image[_MAX_PATH+1];
					int noeditborder=false;
					int anchor = gParams->ctl[iCtl].anchor;
					int tabControl = gParams->ctl[iCtl].tabControl;
					int tabSheet = gParams->ctl[iCtl].tabSheet;
                    int scripted = gParams->ctl[iCtl].scripted;

                    strcpy(tooltip, gParams->ctl[iCtl].tooltip);
                    strcpy(image, gParams->ctl[iCtl].image);

					if (!(gParams->ctl[iCtl].buddy1Style & WS_BORDER)) noeditborder=true;
					
                    gParams->ctl[iCtl].inuse = FALSE;  //so fm_createCtl can work ???
                                                        //without trying to delete
                                                        //stale handles, etc. ???
                    
					createCtl(iCtl,
                                 gParams->ctl[iCtl].ctlclass,         //class
                                 gParams->ctl[iCtl].label,    //text
                                 gParams->ctl[iCtl].xPos,          //x
                                 gParams->ctl[iCtl].yPos,          //y
                                 gParams->ctl[iCtl].width,         //w
                                 gParams->ctl[iCtl].height,        //h
                                 gParams->ctl[iCtl].style,         //styles
                                 gParams->ctl[iCtl].styleEx,       //extended styles
                                 gParams->ctl[iCtl].properties,    //properties
                                 gParams->ctl[iCtl].enabled);      //visible/enabled
					
                    //restore clobbered attributes
                    // setCtlTheme(iCtl, gParams->ctl[iCtl].theme); // Overwrites the background color if theme enabled
					setCtlTab(iCtl, tabControl, tabSheet);		// Overwrites the background color if theme enabled
                    gParams->ctl[iCtl].initVal = initVal;
                    setCtlRange(iCtl, minval, maxval);
                    setCtlLineSize(iCtl, lineSize);
                    setCtlPageSize(iCtl, pageSize);
					// setCtlThumbSize(iCtl, thumbSize);
                    setCtlAction(iCtl, action);
                    /******************************************************/
                    /*  MODIFICATION by Ognen Genchev
                    /*  fm_setCtlVal() receives value from initVal to solve
                    /*  the problem with writing value modification on
                    /*  cancelling opertion.
                    /*  See case CA_APPLY comment bellow
                    /******************************************************/
                    setCtlVal(iCtl, initVal);
                    // end
                    setCtlFontColor(iCtl, textColor);
                    setCtlColor(iCtl, bkColor);
                    setCtlToolTip(iCtl, tooltip, 0);
                    setCtlImage(iCtl, image, imageType);
                    setCustomCtl(iCtl, state); // Added by Ognen Genchev
                    setCtlDivisor(iCtl, divisor);
					setCtlAnchor(iCtl, anchor);
                    setCtlScripting(iCtl, scripted);

					//Set buddy styles
					if (gParams->ctl[iCtl].ctlclass == CC_STANDARD || gParams->ctl[iCtl].ctlclass == CC_SLIDER){
						setCtlBuddyStyle (iCtl,0,gParams->ctl[iCtl].buddy2Style);
						setCtlBuddyStyle (iCtl,1,gParams->ctl[iCtl].buddy1Style);
						setCtlBuddyStyleEx (iCtl,0,gParams->ctl[iCtl].buddy2StyleEx);
						setCtlBuddyStyleEx (iCtl,1,gParams->ctl[iCtl].buddy1StyleEx);
						if (noeditborder) clearCtlBuddyStyle (iCtl,1,WS_BORDER);
					}

                    switch (gParams->ctl[iCtl].ctlclass) {
						case CC_COMBOBOX:
						case CC_LISTBAR:
							setCtlLabel(iCtl,gParams->ctl[iCtl].label2);
							break;
						case CC_TRACKBAR:
						case CC_SLIDER:
							setCtlTicFreq(iCtl, ticFreq);
							break;
						default:
							break;
                    } //switch class


#else
                    hCtrl = GetDlgItem(hDlg, IDC_CTLBASE+iCtl);
                    if (gParams->ctl[iCtl].minval > gParams->ctl[iCtl].maxval) {
                        //range is inverted...
                        SetScrollRange(hCtrl, SB_CTL,
                                       gParams->ctl[iCtl].maxval,
                                       gParams->ctl[iCtl].minval,
                                       FALSE ); 
                        SetScrollPos(hCtrl, SB_CTL,
                                     gParams->ctl[iCtl].minval + gParams->ctl[iCtl].maxval -
                                        gParams->ctl[iCtl].val,
                                        TRUE ); //redraw
                    }
                    else {
                        //normal range (min <= max)
                        SetScrollRange(hCtrl, SB_CTL,
                                       gParams->ctl[iCtl].minval,
                                       gParams->ctl[iCtl].maxval,
                                       FALSE ); //no redraw
                        SetScrollPos(hCtrl, SB_CTL,
                                     gParams->ctl[iCtl].val,
                                     TRUE ); //redraw
                    }
                    SetDlgItemInt(hDlg, IDC_BUDDY1+iCtl,
                                 gParams->ctl[iCtl].val,
                                 TRUE); //signed

                    //set control text
#if 0
                    fsetCtlText(iCtl, (int)gParams->ctl[iCtl].label);
#else
                    //avoid unnecessary redrawing...
                    SetDlgItemText(hDlg, IDC_BUDDY2+iCtl, gParams->ctl[iCtl].label);
#endif

                    //set control enable state
#if 0
                    sub_enableCtl(iCtl);	//MWVDL 2008-07-18
#else
                    {
                        int level = gParams->ctl[iCtl].enabled;
                        EnableWindow(GetDlgItem(hDlg, IDC_CTLBASE+iCtl), (level & 2) != 0);
                        // enable or disable the control edit field
                        EnableWindow(GetDlgItem(hDlg, IDC_BUDDY1+iCtl), (level & 2) != 0);
                        // enable or disable the control text label (no, looks crappy)
                        ////EnableWindow(GetDlgItem(hDlg, IDC_BUDDY2+iCtl), (level & 2) != 0);
                        // show or hide the control
                        ShowWindow(GetDlgItem(hDlg, IDC_CTLBASE+iCtl), (level & 1)?SW_SHOW:SW_HIDE);
                        // show or hide the control edit field
                        ShowWindow(GetDlgItem(hDlg, IDC_BUDDY1+iCtl), (level & 1)?SW_SHOW:SW_HIDE);
                        // show or hide the control text label
                        ShowWindow(GetDlgItem(hDlg, IDC_BUDDY2+iCtl), (level & 1)?SW_SHOW:SW_HIDE);
                    }
#endif
#endif
                } 
            
                //The following code occasionally causes the preview to be sized to a small or zero size which can cause a crash in Cloak Mode
                // So better remove it and resize the preview in OnFilterStart
           /*     
                else if (iCtl == CTL_PREVIEW || iCtl == CTL_FRAME || iCtl == CTL_ZOOM || iCtl == CTL_PROGRESS) {

									
					setCtlPos (iCtl, gParams->ctl[iCtl].xPos,
						gParams->ctl[iCtl].yPos,
						gParams->ctl[iCtl].width,
                        gParams->ctl[iCtl].height);

					sub_enableCtl(iCtl); //MWVDL 2008-07-18

				}
            */

              } //for iCtl
            } //!initialInvocation

#endif //#ifndef ONWINDOWSTART
#endif //#ifndef APP

// ------------------------------------------------------------------------------------------------
// ------------------------------------------------------------------------------------------------



#if OLDZOOMLABEL || TESTSHELL
{
    //Update old zoom label
    char buff[20];
    wsprintf(buff, "%d%%", 100*gFmcp->enlargeFactor/(gStuff->inputRate>>16));
    SetDlgItemText(MyHDlg, 498, buff);
}
#endif


// ONWINDOWSTART set from #ifndef to #ifdef by Ognen Genchev, to initialize call_FFP() only once.
// It is done for proper reinitialization of the controls, for example, when canceling the operation
// after changing control's value, it will remember the last written value, where before this was not
// the case. Will this create further problems? Who knows...
#ifdef ONWINDOWSTART
    if (gParams->gInitialInvocation) {
#endif      
		call_FFP(FFP_CtlDef,(INT_PTR)gFmcp);
        #ifdef APP
            addAppButtons();
        #endif
#ifdef ONWINDOWSTART
    }
#endif
// end

#ifdef SCRIPTABLE
            //Write the script params to the controls
            if (ScriptParamRead){
                int n;
                for (n=0;n<N_SCRIPTITEMS;n++) {
                    if (ScriptParamUsed[n] && gParams->ctl[n].inuse && gParams->ctl[n].scripted){
                        setCtlVal(n, ScriptParam[n]);
                    }
                }
            }
#endif


            if (gParams->gDialogState == 0){
                //save current position and size of dialog
                GetWindowRect(hDlg, &gParams->gDialogRectMin);
            }

            // Restore/refresh dialog title...
            setDialogText(gParams->gDialogTitle);

			// setDefaultWindowTheme(GetDlgItem(hDlg, IDC_PROGRESS1));
            // setDefaultWindowTheme(GetDlgItem(hDlg, IDC_BUTTON1));
            // setDefaultWindowTheme(GetDlgItem(hDlg, IDC_BUTTON2));
            // setDefaultWindowTheme(GetDlgItem(hDlg, IDC_PROXY_GROUPBOX));


			// Start the hit-test timer running...
            KillTimer(hDlg, 2); //Cancel any previous hit-test timer.
            SetTimer(hDlg, 2, 20, NULL);   //previous Period = 250 msec.

			//LLKeyHook = SetWindowsHookEx(WH_KEYBOARD_LL,LLKeyHookProc, (HINSTANCE) GetWindowLong(hDlg, GWL_HINSTANCE),(DWORD)NULL);
			//KeyHook = SetWindowsHookEx(WH_KEYBOARD,KeyHookProc, (HINSTANCE) GetWindowLong(hDlg, GWL_HINSTANCE),(DWORD)NULL);
			
          
#if 0
            //Get Accelerators for Filter dialog
            hAccel = LoadAccelerators(hDllInstance, MAKEINTRESOURCE(IDR_ACCELERATOR_EDIT));
#endif

			/*if (gParams->gInitialInvocation) {
				//set current directory to filter install directory
				_chdir(getFilterInstallDir());
			}*/

			//previous = false if initial invocation
			//if (gParams->gDialogEvent & 1) triggerEvent(0,FME_INIT,(int)!gParams->gInitialInvocation);

#ifdef APP
            {
                HICON hIcon1; 
                hIcon1 = (HICON) LoadImage( hDllInstance, MAKEINTRESOURCE(107), IMAGE_ICON, 16,16,LR_DEFAULTSIZE); 
                SendMessage(hDlg, WM_SETICON, ICON_SMALL, (LPARAM) hIcon1 );

            }
#endif


#if ONWINDOWSTART
            call_FFP(FFP_OnWindowStart,(INT_PTR)gFmcp);

			SetupFilterRecordForProxy (globals, &scaleFactor, scaleFactor, &imageOffset);

			// Set focus to OK button.
            SetFocus (GetDlgItem (hDlg, IDX_OK));
#endif

			
			//Deativate shity press and hold pen flick
			if (getWindowsVersion()>=11) {
				int n;
				for (n=0;n<N_CTLS;n++) deactivateFlicks(n);
			}
			
            // Finally, compute the proxy image now that all params are set...
            // (But skip it if AdvanceState() returned an error.)


            if (gResult == noErr) {
                // Set a timer to trigger a delayed proxy update,
                // giving the message loop a chance to paint the dialog
                // window first...
                KillTimer(hDlg, 1); //Cancel any previous trigger.
                if (!SetTimer(hDlg, 1, 100, NULL)) {
                    // If SetTimer() fails, compute the proxy immediately.
                    if (!fDragging) {
                        DoFilterRect (globals);
                    }
                }
            } //noErr

            
			//Info ("%d,%d - %d,%d",gProxyRect.left,gProxyRect.top,gProxyRect.right,gProxyRect.bottom);

#if APP		
			//Display correct big icon in task bar
			hbicon = LoadImage( GetModuleHandle(0),MAKEINTRESOURCE(107),IMAGE_ICON,
				GetSystemMetrics(SM_CXICON),GetSystemMetrics(SM_CYICON),0);
			if (hbicon) SendMessage(hDlg, WM_SETICON, ICON_BIG, (LPARAM)hbicon);
			//HANDLE hsicon = LoadImage(GetModuleHandle(0),MAKEINTRESOURCE(107),IMAGE_ICON,
			//	GetSystemMetrics(SM_CXSMICON),GetSystemMetrics(SM_CYSMICON),0);
			//if (hsicon) SendMessage(hDlg, WM_SETICON, ICON_SMALL, (LPARAM)hsicon);
#endif
            // Added by Ognen Genchev
            // For bitmap trackbar thumb and channel
            defWndProc = (WNDPROC) SetWindowLongPtr(hDlg, GWLP_WNDPROC, (LONG_PTR)CustomTrackbarWndProc);
            // end
        }
            return  FALSE; // FALSE says we set the focus to a control.


		//case 0x02CC: //WM_TABLET_QUERYSYSTEMGESTURESTATUS:
		//	return 0x00010000; //TABLET_DISABLE_FLICKS;
		//case 0x02D1: //WM_TABLET_FLICK
		//	return 1; //FLICK_WM_HANDLED_MASK


	case WM_WINDOWPOSCHANGED:
		{
			if (!fDialogDisplayed) {
				if (IsWindowVisible(hDlg)){
					fDialogDisplayed = true;
					PerformControlAction(
                                0,						//control index
                                FME_DIALOGDISPLAYED,    //FM event code
                                0,                      //previous value
                                wMsg,                   //message type
                                0,                      //notification code
                                FALSE,                  //fActionable
                                TRUE,0);                 //fProcessed
				}
			}
		}

		return 0;


#if 0
        case WM_ACTIVATE:

            if ((int)SendMessage(GetDlgItem(hEditorDlg, IDC_AUTOLOAD), BM_GETCHECK, 0, 0) == BST_CHECKED){
                    int fActive = LOWORD(wParam);           // activation flag 
                    //fMinimized = (BOOL) HIWORD(wParam); // minimized flag 
                    HWND hwndPrevious = (HWND) lParam;       // window handle 
                    //HWND activeWindow = GetActiveWindow();

                    //if ( LOWORD(wParam) == WA_CLICKACTIVE && (hwndPrevious !=hEditorDlg) && (hwndPrevious !=hDlg)){ 
                    if ( LOWORD(wParam) > 0 && (hwndPrevious !=hEditorDlg) && (hwndPrevious !=hDlg)){ 
                        //Reload and compile  
                        FILETIME dFileTime;
                        HANDLE fHandle = CreateFile(szFileName, GENERIC_READ | GENERIC_WRITE, 
                                    FILE_SHARE_READ | FILE_SHARE_DELETE,
                                    NULL, OPEN_EXISTING, FILE_FLAG_BACKUP_SEMANTICS, NULL);
                        GetFileTime (fHandle, NULL,NULL, &dFileTime);
                        CloseHandle (fHandle);
                            
                        if (szFileTime.dwLowDateTime != dFileTime.dwLowDateTime || 
                        szFileTime.dwHighDateTime != dFileTime.dwHighDateTime){
                            openSourceFile();
                            SendMessage(hDlg, WM_COMMAND, IDC_GO, 0);
                        }
                    }
            }
    
            return 0;

#endif

        case WM_ERASEBKGND:

#if !defined(SKIN) //defined (PLUGINGALAXY) //defined(HYPERTYLE) || 

			if (gDialogImage)
            // See BDLG32 (msdn sample 4544)
            {   // Setup bitmap file for display as dialog background.
                HDC     hDC;

                // First try loading the bitmap from an embedded BITMAP resource...
                hBmpDialog = LoadBitmap((HINSTANCE)hDllInstance, gDialogImage);

                //if (hBmpDialog) Info("Loaded bitmap from resource: %s", gDialogImage);

                // If it's not embedded, try searching for it as an
                // external bitmap file...
                if (!hBmpDialog) {
                    // Load bitmap from file...
                    hBmpDialog = (HBITMAP)LoadImage((HINSTANCE)hDllInstance,
                                           FmSearchFile(gDialogImage),
                                           IMAGE_BITMAP,
                                           0, 0,
                                           LR_LOADFROMFILE);
                }
                if (!hBmpDialog) {
                    //failed

                    //For standalone version, we just quietly ignore the
                    //failure to find the image file, and fall through to
                    //paint the background with whatever color or gradient
                    //is in effect.
                    gDialogImage = NULL;
                    gParams->gDialogImage_save = NULL;//don't retry on subsequent invocations

                }
                else {
                    // Get bitmap information.
                    GetObject(hBmpDialog, sizeof(BITMAP), &bmInfo);

                    // Create a compatible memory DC using the dialog's DC.
                    hDC = GetDC(hDlg);  // release later!
                    hdcMem = CreateCompatibleDC(hDC);
                    if (!hdcMem) {
                        //failed
                        // Release the client window DC.
                        ReleaseDC(hDlg, hDC);
                        DeleteObject(hBmpDialog);
                        hBmpDialog = NULL;
                        gParams->gDialogImage_save = gDialogImage = NULL;//first, to prevent reentry
                        ErrorOk("Could not create memory DC for background image");
                        return TRUE;    //otherwise background paints too much????
                    }
                    else {
                        // Select the bitmap into the memory DC.
                        hBmpOld = (HBITMAP)SelectObject(hdcMem, hBmpDialog);
                        SetMapMode(hdcMem, GetMapMode(hDC));    //needed???

                        // Set gDialogImage to null so we don't open this file again
                        // during this invocation.
                        gDialogImage = NULL;
                    }
                    // Release the client window DC.
                    ReleaseDC(hDlg, hDC);
                }
            }
            if (hBmpDialog && gParams->gDialogImage_save)
            {   // Display a bitmap as the dialog background.
                // MUST use the DC passed in wParam, not the DC of hDlg --
                // The clipping/update regions are different!!!
                HDC hDC = (HDC) wParam; // handle of device context

                if (gParams->gDialogImageMode == DIM_STRETCHED)
                //mode=stretched
                {   // Compress/stretch the bitmap as necessary...
                    // See BDLG32 (msdn sample 4544)
                    RECT rectClient;
                    POINT pt;
                    int oldMode;

                    GetClientRect(hDlg, &rectClient);
                    size.cx = rectClient.right;     // zero-based
                    size.cy = rectClient.bottom;    // zero-based
                    pt.x = rectClient.left;
                    pt.y = rectClient.top;
                    oldMode = SetStretchBltMode(hDC, gParams->gDialogImageStretchMode);
                    StretchBlt(hDC, pt.x, pt.y, size.cx, size.cy,
                               hdcMem, 0, 0, bmInfo.bmWidth, bmInfo.bmHeight,
                               SRCCOPY);
                    SetStretchBltMode(hDC, oldMode);
                }
                else if (gParams->gDialogImageMode == DIM_TILED)
                //mode=tiled
                {   // Use multiple straight BitBlts... (a la Prosise)
                    int x, y;
                    POINT ptSize, ptOrg, ptTotalSize;
                    RECT rectClient;

                    GetClientRect(hDlg, &rectClient);
                    size.cx = rectClient.right;     // zero-based
                    size.cy = rectClient.bottom;    // zero-based
                    ptSize.x = bmInfo.bmWidth;
                    ptSize.y = bmInfo.bmHeight;
                    DPtoLP(hDC, &ptSize, 1);
                    //need to convert size.cx, size.cy to LP??
                    ptTotalSize.x = size.cx;
                    ptTotalSize.y = size.cy;
                    DPtoLP(hDC, &ptTotalSize, 1);
                    ptOrg.x = 0;
                    ptOrg.y = 0;
                    DPtoLP(hdcMem, &ptOrg, 1);
                    for (y = 0; y < ptTotalSize.y; y += ptSize.y) {
                        for (x = 0; x < ptTotalSize.x; x += ptSize.x) {
                            BitBlt(hDC, x, y, ptSize.x, ptSize.y,
                                   hdcMem, ptOrg.x, ptOrg.y, SRCCOPY);
                        }
                    }
                }
                else
                //mode=exact
                {   // Use straight BitBlt... (a la Petzold)
                    POINT   ptSize, ptOrg;
                    ptSize.x = bmInfo.bmWidth;
                    ptSize.y = bmInfo.bmHeight;
                    DPtoLP(hDC, &ptSize, 1);
                    ptOrg.x = 0;
                    ptOrg.y = 0;
                    DPtoLP(hdcMem, &ptOrg, 1);
                    BitBlt(hDC, 0, 0, ptSize.x, ptSize.y,
                           hdcMem, ptOrg.x, ptOrg.y, SRCCOPY);
                }
            }
            else
            {   // Do a gradient fill a la Prosise BitmapDemo.cpp...
                HBRUSH hBrush[64];
                int i;
                int nWidth;
                int nHeight;
                RECT rect;
                // MUST use the DC passed in wParam, not the DC of hDlg --
                // The clipping/update regions are different!!!
                HDC hDC = (HDC) wParam; // handle of device context

                //COLORREF color1 = RGB(255,0,0);
                //COLORREF color2 = RGB(0,255,0);
                //COLORREF color1 =  GetSysColor(COLOR_BTNFACE);//RGB(0xD4,0xD0,0xC8);  //yellow grey
                //COLORREF color2 =  GetSysColor(COLOR_BTNFACE);//RGB(0xD4,0xD0,0xC8);   //yellow grey

                COLORREF color1 = gParams->gDialogGradientColor1;
                COLORREF color2 = gParams->gDialogGradientColor2;
                int r1 = GetRValue(color1);
                int g1 = GetGValue(color1);
                int b1 = GetBValue(color1);
                int r2 = GetRValue(color2);
                int g2 = GetGValue(color2);
                int b2 = GetBValue(color2);

                GetClientRect(hDlg, &rect);
                nWidth = rect.right - rect.left;
                nHeight = rect.bottom - rect.top;

#ifndef NOTRACE
                Trace (globals, "WM_ERASEBKGND\n"
                                " nWidth = %d\n"
                                " nHeight = %d\n"
                                " hDC = %#x",
                                nWidth,
                                nHeight,
                                hDC);
#endif
                
                if (color1 == color2) {
                    
                    // optimize this if color1 == color2
                    hBrush[0] = CreateSolidBrush(color1);
                    FillRect(hDC, &rect, hBrush[0]);
                    
                    DeleteObject(hBrush[0]);
                    //SetTimer(hDlg, 1002, 100, NULL);
                }
                else
                {
                    // create a 64-step linear gradient
                    for (i=0; i < 64; i++)
                    {
                        hBrush[i] = CreateSolidBrush(
                            RGB( (r1*(63-i)+r2*i)/63, (g1*(63-i)+g2*i)/63, (b1*(63-i)+b2*i)/63 )
                            );
                    }

                    if (gParams->gDialogGradientDirection == 0) {
                        // vertical gradient
#if 0
                        for (i=0; i < nHeight; i++) {
                            SetRect(&rect, 0, i, nWidth, i+1);
                            FillRect(hDC, &rect, hBrush[(i*64)/nHeight]);
                        }
#else
                        for (i=0; i < 64; i++) {
                            SetRect(&rect, 0, i*nHeight/64, nWidth, (i+1)*nHeight/64);
                            FillRect(hDC, &rect, hBrush[i]);
                        }
#endif
                    }
                    else {
                        // horizontal gradient
#if 0
                        for (i=0; i < nWidth; i++) {
                            SetRect(&rect, i, 0, i+1, nHeight);
                            FillRect(hDC, &rect, hBrush[(i*64)/nWidth]);
                        }
#else
                        for (i=0; i < 64; i++) {
                            SetRect(&rect, i*nWidth/64, 0, (i+1)*nWidth/64, nHeight);
                            FillRect(hDC, &rect, hBrush[i]);
                        }
#endif
                    }

                    for (i=0; i < 64; i++)
                        DeleteObject(hBrush[i]);
                } /* color1 != color2 */
                
            }


#else

	#if 0
			//Doube Buffering - Causes slower drawing under Win7
			{
				HDC hDC = (HDC) wParam; // handle of device context
				HBRUSH hBrush;
				RECT rect;
				HDC offDC;
				HBITMAP hOldBitmap, hBitmap;
				
				offDC = CreateCompatibleDC(hDC);
				
				GetClientRect(hDlg, &rect);
				hBitmap = CreateCompatibleBitmap(hDC, rect.right-rect.left, rect.bottom-rect.top);
				hOldBitmap = SelectObject(offDC, hBitmap);
				
				hBrush = CreateSolidBrush(gParams->gDialogGradientColor1);
				SelectObject(offDC,hBrush);

				//Background color 
				FillRect(offDC, &rect, hBrush);
				DeleteObject(hBrush);

				//Apply Preview Color
				if (gParams->ctl[CTL_PREVIEW].bkColor != -1){
					//Only if image does not fill the preview
					if ( (gFmcp->x_start == 0 && gFmcp->x_end == gFmcp->X) || (gFmcp->y_start == 0 && gFmcp->y_end == gFmcp->Y) ){
						RECT rect;
						
						GetWindowRect( GetDlgItem(hDlg, 101), &rect );
						ScreenToClient (hDlg, (LPPOINT)&rect);
						ScreenToClient (hDlg, (LPPOINT)&(rect.right));

						hBrush = CreateSolidBrush(gParams->ctl[CTL_PREVIEW].bkColor);
						FillRect(offDC, &rect, hBrush);
						DeleteObject(hBrush);
					}
				}

#if SKIN
				//Erase tab background
				if (isSkinActive()) {
					int i;
					for (i = 0; i < N_CTLS; ++i) {
						if (gParams->ctl[i].ctlclass == CC_TAB){
							if (gParams->ctl[i].inuse && gParams->ctl[i].enabled & 3){
								//skinDraw(i,2);
								RECT rect;
								
								int screenX = getCtlPixelPos(i,0); 
								int screenY = getCtlPixelPos(i,1);
								int width = HDBUsToPixels(gParams->ctl[i].defwidth);
								int height = VDBUsToPixels(gParams->ctl[i].defheight);
								
								rect.left = screenX;
								rect.top = screenY;
								rect.right = screenX+width-1;
								rect.bottom = screenY+height;

								hBrush = CreateSolidBrush(skinGetColor(i));
								FillRect(offDC, &rect, hBrush);
								DeleteObject(hBrush);
							}
						}	
					}
				}
#endif

				//Output to screen	
				BitBlt(hDC, 0, 0, rect.right, rect.bottom, offDC, 0, 0, SRCCOPY);

				DeleteObject (hBitmap);
				SelectObject (offDC, hOldBitmap);
				DeleteDC (offDC);
			}

	#else


			//Clip controls to avoid flickering under Vista and 7
			if (getWindowsVersion() >= 11){ //getAppTheme() &&
				int i;
				HDC hDC = (HDC) wParam;
				RECT rect;//r,rC;
				for (i = 0; i < N_CTLS; ++i) {
					if (gParams->ctl[i].inuse && ctlEnabledAs(i)){
						if (i != CTL_FRAME && gParams->ctl[i].ctlclass != CC_IMAGE && gParams->ctl[i].ctlclass != CC_TOOLBAR && //gParams->ctl[i].ctlclass != CC_EDIT &&
							!(gParams->ctl[i].ctlclass == CC_STATICTEXT && gParams->ctl[i].tabControl<0) &&
							!(!getAppTheme() && gParams->ctl[i].ctlclass == CC_CHECKBOX) &&  //check boxes cause redraw problems with classic theme
							!(gParams->ctl[i].ctlclass == CC_GROUPBOX && (gParams->ctl[i].tabControl==-1 && gParams->ctl[i].enabled & 3))
							#if SKIN
							#ifndef SKINFLICKERFREE
								&& !(isSkinActive() && gParams->skinStyle>0 && (gParams->ctl[i].tabControl>=0 || gParams->ctl[i].ctlclass == CC_TAB))
							#endif
							#endif
						){
							GetWindowRect( gParams->ctl[i].hCtl, &rect );
							ScreenToClient (hDlg, (LPPOINT)&rect);
							ScreenToClient (hDlg, (LPPOINT)&(rect.right));
							ExcludeClipRect(hDC,rect.left,rect.top,rect.right-1,rect.bottom-1);
							/*if (gParams->ctl[i].hBuddy2 && gParams->ctl[i].buddyBkColor != -1){ //Non-transparent labels
								GetWindowRect( gParams->ctl[i].hBuddy2, &rect );
								ScreenToClient (hDlg, (LPPOINT)&rect);
								ScreenToClient (hDlg, (LPPOINT)&(rect.right));
								ExcludeClipRect(hDC,rect.left,rect.top,rect.right-1,rect.bottom-1);
							}
							if (gParams->ctl[i].hBuddy1){ //edit boxes
								GetWindowRect( gParams->ctl[i].hBuddy1, &rect );
								ScreenToClient (hDlg, (LPPOINT)&rect);
								ScreenToClient (hDlg, (LPPOINT)&(rect.right));
								ExcludeClipRect(hDC,rect.left,rect.top,rect.right-1,rect.bottom-1);
							}*/
						}
					}
				}


			#if SKINFLICKERFREE
				/*for (i = 0; i < N_CTLS; ++i) {
					if (gParams->ctl[i].inuse && gParams->ctl[i].tabControl>=0){
						if (gParams->ctl[i].ctlclass == CC_IMAGE && ctlEnabledAs(i)){
							HRGN hClipRegion=0, hCtlRegion;
							GetWindowRect( gParams->ctl[i].hCtl, &rect );
							ScreenToClient (hDlg, (LPPOINT)&rect);
							ScreenToClient (hDlg, (LPPOINT)&(rect.right));
							//Add back
							hCtlRegion = CreateRectRgn(rect.left,rect.top,rect.right-1,rect.bottom-1);
							GetClipRgn(hDC,hClipRegion);
							CombineRgn (hClipRegion,hCtlRegion,hClipRegion,RGN_XOR);
							SelectClipRgn(hDC,hClipRegion);
						}
					}
				}*/
			#endif

			}


			/*{
				static int cycle=0;
				char string[256] = "";
				cycle++;
				sprintf(string,"%d: WM_ERASE\n",cycle);
				OutputDebugString(string);
			}*/

			//Background color
			{
				HDC hDC = (HDC) wParam;
				HBRUSH hBrush;
				RECT rect;
				GetClientRect(hDlg, &rect);
				hBrush = CreateSolidBrush(gParams->gDialogGradientColor1);
                FillRect(hDC, &rect, hBrush);
                DeleteObject(hBrush);
			}

			//Apply Preview Color
			drawPreviewColor((HDC)wParam);
			/*
			if (gParams->ctl[CTL_PREVIEW].bkColor != -1){
				//Only if image does not fill the preview
				if ( (gFmcp->x_start == 0 && gFmcp->x_end == gFmcp->X) || (gFmcp->y_start == 0 && gFmcp->y_end == gFmcp->Y) ){
					HBRUSH hBrush;
					RECT rect;
					HDC hDC = (HDC) wParam;

					GetWindowRect( GetDlgItem(hDlg, 101), &rect );
					ScreenToClient (hDlg, (LPPOINT)&rect);
					ScreenToClient (hDlg, (LPPOINT)&(rect.right));
					hBrush = CreateSolidBrush(gParams->ctl[CTL_PREVIEW].bkColor);
					FillRect(hDC, &rect, hBrush);
					DeleteObject(hBrush);
				}
			}*/


#if SKIN && !defined(GDIPLUS)

			//Erase tab background
			if (isSkinActive() && gParams->skinStyle>0) {
				int i;

#ifndef SKINFLICKERFREE
				for (i = 0; i < N_CTLS; ++i) {
					if (gParams->ctl[i].ctlclass == CC_TAB){
						if (gParams->ctl[i].inuse && gParams->ctl[i].enabled & 3){
							/*HBRUSH hBrush;
							RECT rect;
							HDC hDC = (HDC) wParam;

							int screenX = getCtlPixelPos(i,0); 
							int screenY = getCtlPixelPos(i,1);
							int width = HDBUsToPixels(gParams->ctl[i].width); //defwidth
							int height = VDBUsToPixels(gParams->ctl[i].height); //defheight
							
							rect.left = screenX;
							rect.top = screenY + skinGetTabButtonHeight();
							rect.right = screenX+width-1;
							rect.bottom = screenY+height;

							hBrush = CreateSolidBrush(skinGetColor(i));
							FillRect(hDC, &rect, hBrush);
							DeleteObject(hBrush);*/

							skinSetDC((HDC)wParam);
							skinDraw(i,2);
							
						}
					}
				}


				for (i = 0; i < N_CTLS; ++i) {
					if (gParams->ctl[i].ctlclass == CC_GROUPBOX){
						if ( gParams->ctl[i].inuse && 
							 ( ctlEnabledAs(i) || (gParams->ctl[i].tabControl==-1 && gParams->ctl[i].enabled & 3)) 
						){
						//if (IsWindowVisible(gParams->ctl[i].hCtl)){
							
							/*HDC hDC = (HDC) wParam;
							HBRUSH hBrush;
							RECT rect;
							
							int screenX = getCtlPixelPos(i,0); 
							int screenY = getCtlPixelPos(i,1);
							int width = HDBUsToPixels(gParams->ctl[i].width); //defwidth
							int height = VDBUsToPixels(gParams->ctl[i].height); //defheight
							
							rect.left = screenX;
							rect.top = screenY;
							rect.right = screenX+width;
							rect.bottom = screenY+height;

							hBrush = CreateSolidBrush(0xffffff);
							FrameRect(hDC, &rect, hBrush);
							DeleteObject(hBrush);*/

							skinSetDC((HDC)wParam);
							skinDraw(i,2);

						}
					}
				}
#endif

			}

#endif


	#endif

#endif




#if 1
            return TRUE;   // No more background painting needed.
#else
            // Seems to make things worse...
            return SetDlgMsgResult(hDlg, wMsg, TRUE);   // No more background painting needed.
#endif


#ifdef FOCALBLADE

        case WM_MENUSELECT:
            {
                int n = LOWORD(wParam);   // menu item or submenu index 
                int fuFlags = HIWORD(wParam); // menu flags 
                HMENU hmenu = (HMENU) lParam;          // handle of menu clicked 
                int previous = 0;

                if (GetSubMenu(hmenu,n) != NULL) previous = 1;

                PerformControlAction(
                                    n,                      //control index
                                    FME_MENUSELECT,           //FM event code
                                    previous,                  //previous value -> is sub menu if nonzero
                                    WM_MENUSELECT,            //message type
                                    0,                      //notification code
                                    FALSE,                  //fActionable
                                    TRUE,1);                  //fProcessed

                return false;
            }
#endif

        case WM_DRAWITEM:
            {//scope
                LPDRAWITEMSTRUCT pdis = (LPDRAWITEMSTRUCT)lParam;
                if (pdis->CtlID >= IDC_CTLBASE && pdis->CtlID < IDC_CTLBASE+N_CTLS) {
                    int n = pdis->CtlID - IDC_CTLBASE;
                    
                    // Added by Ognen Genchev
                    COLORREF shapeColor = gParams->ctl[n].shapeColor;
                    // end

					//if (n==CTL_PREVIEW) Info ("%d",gParams->ctl[n].inuse,!(gParams->ctl[n].enabled != 3));
					//if (n==CTL_PROGRESS) Info ("Progress"); 

#if SKIN
					if (isSkinActive() && isSkinCtl(n)){ // && gParams->ctl[n].ctlclass != CC_TAB // && gParams->ctl[n].ctlclass != CC_GROUPBOX
						
						skinDraw(n,0);

					} else 
#endif					
                        // Added by Ognen Genchev
                        if (gParams->ctl[n].inuse && (gParams->ctl[n].properties & CTP_RECTANGLE) && !(gParams->ctl[n].enabled != 3)) {

                            // Draw rectangular frame
                            FrameRect(pdis->hDC, &pdis->rcItem, CreateSolidBrush(shapeColor));
                            
                                // Draw triangle
    /*                      cx = pdis->rcItem.right  - pdis->rcItem.left;
                            cy = pdis->rcItem.bottom - pdis->rcItem.top;

                            ptt[0].x = 3 * cx / 8 ;  ptt[0].y = 1 * cy / 8;
                            ptt[1].x = 5 * cx / 8 ;  ptt[1].y = 1 * cy / 8;
                            ptt[2].x = 4 * cx / 8 ;  ptt[2].y = 3 * cy / 8;

                            Triangle (pdis->hDC, ptt);*/

                            PerformControlAction(
                                    n,                      //control index
                                    FME_RECTANGLE,           //FM event code
                                    0,                      //previous value
                                    WM_DRAWITEM,            //message type
                                    0,                      //notification code
                                    FALSE,                  //fActionable
                                    TRUE,0);                  //fProcessed
                        }
                        // end

						if (gParams->ctl[n].inuse && (gParams->ctl[n].properties & CTP_DRAWITEM) && !(gParams->ctl[n].enabled != 3) ){
                    
                        PerformControlAction(
                                n,                      //control index
                                FME_DRAWITEM,           //FM event code
                                0,                      //previous value
                                WM_DRAWITEM,            //message type
                                0,                      //notification code
                                FALSE,                  //fActionable
                                TRUE,0);                  //fProcessed
                    }
                    
                    //It's a user-defined control
                    if (gParams->ctl[n].ctlclass != CC_IMAGE) {
                        return FALSE;       //for now
                    }
#if 1
                    //Info("WM_DRAWITEM: ctl = %d", n);
                    //InvalidateRect(pdis->hwndItem, NULL, TRUE); //Infinite loop!
#endif
                    if (gParams->ctl[n].image[0] == '\0') {
                        //no image
                        return FALSE;       //not handled
                    }
                    if (pdis->itemAction & ODA_DRAWENTIRE) {
                        //stretch a bitmap

                        if (gParams->ctl[n].im.hbm == 0) {
                            //first time through, load bitmap into a memory DC
                            //first try to load from RT_BITMAP resource...
                            gParams->ctl[n].im.hbm = (HBITMAP)LoadImage((HINSTANCE)hDllInstance,
                                                               gParams->ctl[n].image,
                                                               IMAGE_BITMAP,
                                                               0, 0,
                                                               LR_DEFAULTCOLOR);
                            if (gParams->ctl[n].im.hbm == 0) {
                                //if not in resources, load from file...
                                gParams->ctl[n].im.hbm = (HBITMAP)LoadImage((HINSTANCE)hDllInstance,
                                                                   FmSearchFile(gParams->ctl[n].image),
                                                                   IMAGE_BITMAP,
                                                                   0, 0,
                                                                   LR_LOADFROMFILE);
                            }
                            if (gParams->ctl[n].im.hbm == 0) {
                                gParams->ctl[n].im.hbm = (HBITMAP) 1; //1-shot stopper

                                return FALSE;   //failed
                            }
                            else {
                                COLORREF crTransparent;

#if SKIN
								//Invert Bitmpap?
								if (isSkinActive() && skinGetInvertBitmaps()){
									if (strstr(gParams->ctl[n].image,"_off") || strstr(gParams->ctl[n].image,"_grey") || strstr(gParams->ctl[n].image,"arrow_"))
										gParams->ctl[n].im.hbm = (HBITMAP)(INT_PTR)invertBitmap(gParams->ctl[n].im.hbm,NULL);
								}
#endif

                                // Get bitmap information.
                                GetObject(gParams->ctl[n].im.hbm, sizeof(BITMAP),
                                          &gParams->ctl[n].im.bmInfo);

                                // Create a compatible memory DC.
                                gParams->ctl[n].im.hdcMem = CreateCompatibleDC(pdis->hDC);
                                if (!gParams->ctl[n].im.hdcMem) {
                                        return FALSE;   //failed
                                }
                                SetMapMode(gParams->ctl[n].im.hdcMem, GetMapMode(pdis->hDC));

                                // Select the bitmap into the memory DC.
                                gParams->ctl[n].im.hbmOld = (HBITMAP)SelectObject(gParams->ctl[n].im.hdcMem,
                                                                         gParams->ctl[n].im.hbm);

                                // Choose the transparent color to be the
                                // color of the pixel at (0,0)
                                crTransparent = GetPixel(gParams->ctl[n].im.hdcMem, 0, 0);

                                // Set the background color of the image
                                // to the transparent color chosen above
                                SetBkColor(gParams->ctl[n].im.hdcMem, crTransparent);

                                // Create another memory DC for the AND mask
                                gParams->ctl[n].im.hdcAnd = CreateCompatibleDC(pdis->hDC);
                                if (!gParams->ctl[n].im.hdcAnd) {
                                        return FALSE;   //failed
                                }
                                SetMapMode(gParams->ctl[n].im.hdcAnd, GetMapMode(pdis->hDC));

                                // Create a monochrome bitmap
                                gParams->ctl[n].im.hbmAnd =
                                    CreateBitmap(gParams->ctl[n].im.bmInfo.bmWidth,
                                                 gParams->ctl[n].im.bmInfo.bmHeight,
                                                 1, 1, NULL);

                                // Select the monochrome bitmap into the AND DC
                                gParams->ctl[n].im.hbmOldAnd =
                                    (HBITMAP)SelectObject(gParams->ctl[n].im.hdcAnd,
                                                 gParams->ctl[n].im.hbmAnd);

                                // Copy the image into the monochrome DC, setting
                                // all background-colored pixels to 1, and all
                                // other pixels to 0
                                BitBlt(gParams->ctl[n].im.hdcAnd,
                                       0, 0,
                                       gParams->ctl[n].im.bmInfo.bmWidth,
                                       gParams->ctl[n].im.bmInfo.bmHeight,
                                       gParams->ctl[n].im.hdcMem,
                                       0, 0,
                                       SRCCOPY);

                                // Set transparent bits in the image to black
                                BitBlt(gParams->ctl[n].im.hdcMem,
                                       0, 0,
                                       gParams->ctl[n].im.bmInfo.bmWidth,
                                       gParams->ctl[n].im.bmInfo.bmHeight,
                                       gParams->ctl[n].im.hdcAnd,
                                       0, 0,
                                       0x220326); //dest<-dest AND NOT src
                            }
                        }
                        if ((DWORD_PTR)gParams->ctl[n].im.hbm > 1) {
                            int oldMode = SetStretchBltMode(pdis->hDC, COLORONCOLOR);
                            //Set foreground color to black and background color to
                            //white in the display DC to get desired monchrome
                            //conversion for the AND mask...
                            COLORREF oldTextColor = SetTextColor(pdis->hDC, RGB(0,0,0));
                            COLORREF oldBkColor = SetBkColor(pdis->hDC, RGB(255,255,255));
                            StretchBlt(pdis->hDC,
                                       pdis->rcItem.left,
                                       pdis->rcItem.top,
                                       pdis->rcItem.right - pdis->rcItem.left,
                                       pdis->rcItem.bottom - pdis->rcItem.top,
                                       gParams->ctl[n].im.hdcAnd,
                                       0, 0, 
                                       gParams->ctl[n].im.bmInfo.bmWidth, 
                                       gParams->ctl[n].im.bmInfo.bmHeight,
                                       SRCAND);
                            //Restore the display DC's previous foreground and background
                            //colors...
                            SetTextColor(pdis->hDC, oldTextColor);
                            SetBkColor(pdis->hDC, oldBkColor);
#if 1
                            StretchBlt(pdis->hDC,
                                       pdis->rcItem.left,
                                       pdis->rcItem.top,
                                       pdis->rcItem.right - pdis->rcItem.left,
                                       pdis->rcItem.bottom - pdis->rcItem.top,
                                       gParams->ctl[n].im.hdcMem,
                                       0, 0, 
                                       gParams->ctl[n].im.bmInfo.bmWidth, 
                                       gParams->ctl[n].im.bmInfo.bmHeight,
#if 1
                                       SRCPAINT);   //or SRCINVERT
#else
                                       SRCINVERT);   //or SRCINVERT
#endif
#endif
                            SetStretchBltMode(pdis->hDC, oldMode);
                        }
                    } //if ODA_DRAWENTIRE
                    if (pdis->itemAction & ODA_SELECT) {
                    }
                    if (pdis->itemAction & ODA_FOCUS) {
                    }
                    return TRUE;    //handled
                }//if user control
            }//scope
            return FALSE;   //not handled

#ifdef APP
#ifndef LIGHTROOM

        case WM_DROPFILES:
        {
            HDROP hdrop = (HDROP)wParam;
            char szNextFile [MAX_PATH];

            UINT uNumFiles = DragQueryFile ( hdrop, 0xFFFFFFFF, 0, 0);
            UINT uFile;
			
            if (uNumFiles==1){

                if ( DragQueryFile ( hdrop, 0, szNextFile, MAX_PATH ) > 0 ){
                    openImage(szNextFile,0,true,false);
                }

            } else {

                int openIT = (app.inputTotal==0);
				
                for (uFile = 0; uFile < uNumFiles; uFile++){
                    // Get the next filename from the HDROP info.
                    if ( DragQueryFile ( hdrop, uFile, szNextFile, MAX_PATH ) > 0 ){
                        addImageFile(szNextFile);
                    }
                }
                updateImageList();

                if (openIT)
                    openImage(app.strInputFiles,1,true,false);
                else {
                    SetupFilterRecordForProxy (globals, &fmc.scaleFactor, fmc.scaleFactor, &imageOffset);
                    //UpdateProxy(MyHDlg, TRUE);
                    //ShowOutputBuffer(globals, MyHDlg, FALSE /*fDragging*/);
                    doAction(CA_PREVIEW);
                }
 
            }
            

            // Free up memory.
            DragFinish ( hdrop );

            return 0; //processed

        }
#endif
#endif

        case WM_CTLCOLORSTATIC:
        case WM_CTLCOLORBTN:
        case WM_CTLCOLORLISTBOX:
        case WM_CTLCOLORSCROLLBAR:
//#if 0
            // Yes, this controls nonreadonly edit box colors;
            // Readonly edit boxes seem to use WM_CTLCOLORSTATIC instead.
        case WM_CTLCOLOREDIT:
//#endif

            {
                HDC hdc = (HDC) wParam;   // handle of display context
                HWND hwnd = (HWND) lParam; // handle of static control
                int iCtl;

                iCtl = GetDlgCtrlID(hwnd);    // ID of control

/*                
				if (// iCtl >= IDC_BUDDY2 && iCtl < IDC_BUDDY2+N_CTLS   // Scrollbar labels
                    //|| iCtl >= 600 && iCtl < 600+N_CHECKBOXES   // Checkboxes--NFG, not 600!
                    // || iCtl == IDC_SLIDER1          // trackbar -- NFG!!!
                       (iCtl >= 803 && iCtl <= 899) // Static text but NOT Group boxes
                       //(iCtl >= 800 && iCtl <= 899) // Static text and [NFG]Group boxes
                    || (iCtl >= IDC_TRACE && iCtl <= IDC_OPTBEV)
                    || (iCtl >= IDC_DISASSEMB && iCtl <= IDC_USE_CMOV)
                    || (iCtl == IDC_USE_INLINES)
                    //|| iCtl == 410                  // Slider1 label
                    || iCtl == 499                  // Host Name
                    || iCtl == 500                  // Filter Title
                    || iCtl == 501                  // Filter Case
                    || iCtl == 3987                 // Custom control above
                    || iCtl == 3988                 // Custom control above
                    || iCtl == 3989                 // Custom control above
                    // || iCtl == 3990                 // NFG for trackbar!!
#if 0
                    || iCtl == IDC_EMF
                    //|| iCtl == IDC_ICON1
#endif
#ifdef ALLOW_DRAGOUT
                    || iCtl == IDC_DRAGOUT
#endif
#ifdef USE_BITMAP
                    || iCtl == IDX_OK || iCtl == IDX_CANCEL
#ifdef FM_LIFTCOV
                    || iCtl == IDC_BUTTON1 || iCtl == IDC_BUTTON2   // Zoom buttons
#endif
#endif
                    )
                {   // We want these controls to be transparent...
                    // Note that these controls are NOT updated dynamically
                    // (which doesn't work right, since the backgrounds don't
                    // get erased properly).
                    SetBkMode(hdc, TRANSPARENT);
#if defined(xxxxx)
                    SetTextColor(hdc, RGB(  0,  0,  0));    //black text
#elif defined(FM_LIFTCOV)
                    SetTextColor(hdc, RGB(255,255,  0));    //yellow text
#else
                    SetTextColor(hdc, RGB(255,255,255));    //white text
#endif
                    //should really use SetDlgMsgResult...
                    return (INT_PTR) (HBRUSH) GetStockObject(HOLLOW_BRUSH); // create statically?
                }
*/
                if (iCtl >= IDC_CTLBASE && iCtl < IDC_CTLBASE+N_CTLS) {
                    // user control...
                    int n = iCtl - IDC_CTLBASE;
                    if (!gParams->ctl[n].inuse) return FALSE; //not processed
                    if (gParams->ctl[n].bkColor == -1) {
                        //transparent background
                        if (gParams->ctl[n].ctlclass == CC_STANDARD ||
							gParams->ctl[n].ctlclass == CC_SLIDER ||
                            gParams->ctl[n].ctlclass == CC_SCROLLBAR) {
                            //scrollbars are NFG with transparent body,
                            //so bail out to the Windows default handling
                            //in this case...
                            return FALSE;   //not processed
                        /*} else if (gParams->ctl[n].ctlclass == CC_SLIDER){
                            //SetBkMode(hdc, TRANSPARENT); 
                            //return (INT_PTR)GetStockObject(HOLLOW_BRUSH);
                            if (!getAppTheme()){
                                return false;
                            } else {
                                RECT clientRectangle = {0};
                                GetClientRect(gParams->ctl[n].hCtl, &clientRectangle);

                                {
                                    typedef HRESULT WINAPI ISDRAWTHEME(HWND hwnd, HDC hdc,const RECT *prc);
                                    ISDRAWTHEME* pISDRAWTHEME = NULL;
                                    HMODULE hMod = LoadLibrary("uxtheme.dll");
                                    if(hMod){
                                        pISDRAWTHEME = (ISDRAWTHEME*) GetProcAddress(hMod, "DrawThemeParentBackground");
                                        pISDRAWTHEME(gParams->ctl[n].hCtl, (HDC) wParam, &clientRectangle);
                                        FreeLibrary(hMod);
                                    }
                                }
                                return (LRESULT) GetStockObject(NULL_BRUSH);
                            }*/
                        }   
                        //else, go ahead and set transparent background mode
                        SetBkMode(hdc, TRANSPARENT);
                    }
                    else {
                        //solid background color
                        SetBkMode(hdc, OPAQUE);
                        SetBkColor(hdc, gParams->ctl[n].bkColor);
                    }
                    SetTextColor(hdc, gParams->ctl[n].textColor);
                    return (INT_PTR)gParams->ctl[n].hBrush;         //background brush
                

				} else if (iCtl >= IDC_BUDDY2 && iCtl < IDC_BUDDY2+N_CTLS) {
                    // static text buddy for a scrollbar...
                    int n = iCtl - IDC_BUDDY2;
                    if (!gParams->ctl[n].inuse) return FALSE; //not processed
                    
					/*if (gParams->ctl[n].ctlclass == CC_TOOLBAR){
						SetBkMode(hdc, OPAQUE);
                        SetBkColor(hdc, gParams->ctl[n].bkColor);
						SetTextColor(hdc, gParams->ctl[n].textColor);
						return (INT_PTR)gParams->ctl[n].hBrush;         //background brush
					}*/

					if (gParams->ctl[n].buddyBkColor == -1) {
						//for now, we always want static text buddies to have
						//a transparent background
						SetBkMode(hdc, TRANSPARENT);
						SetTextColor(hdc, gParams->ctl[n].textColor);
						return (INT_PTR)GetStockObject(HOLLOW_BRUSH);         //background brush
					} else {
						//solid background color
                        SetBkMode(hdc, OPAQUE);
                        SetBkColor(hdc, gParams->ctl[n].buddyBkColor);
						SetTextColor(hdc, gParams->ctl[n].textColor);
						return (INT_PTR)gParams->ctl[n].hBrush;         //background brush
					}
                

				} else if (iCtl >= IDC_BUDDY1 && iCtl < IDC_BUDDY1+N_CTLS) {
                    // static text buddy for a scrollbar...
                    int n = iCtl - IDC_BUDDY1;
                    if (!gParams->ctl[n].inuse) return FALSE; //not processed
                    
					if (gParams->ctl[n].editColor != -1) {
						//solid background color
                        SetBkMode(hdc, OPAQUE);
                        SetBkColor(hdc, gParams->ctl[n].editColor);
						SetTextColor(hdc, gParams->ctl[n].editTextColor);
						return (INT_PTR)gParams->ctl[n].hBrush;         //background brush
					} else
						return FALSE;
                }


                //else if (iCtl >= 800 && iCtl <= 802) { //Advanced mode group boxes
                //    SetBkColor(hdc, RGB(128,128,128));  //gray background
                //    SetTextColor(hdc, RGB(255,255,255));    //white text
                //    //should really use SetDlgMsgResult...
                //    return (INT_PTR) (HBRUSH) GetStockObject(GRAY_BRUSH);
                //}
                //else if (iCtl != IDC_PROXY             // Could this be the proxy update bug???
                //     //&&  iCtl != IDC_SLIDER1           // NFG otherwise
                //     //&&  iCtl != IDC_ICON1
                //     &&  iCtl != 3990                  // NFG otherwise for trackbar
                //     )
                //{
                //    //SetBkColor(hdc, RGB(255,255,255));  //white background
                //    //SetTextColor(hdc, RGB(0,0,100));    //dark blue text
                //    // //should really use SetDlgMsgResult...
                //    //return (INT_PTR) (HBRUSH) GetStockObject(WHITE_BRUSH); // create statically?
                //    return FALSE;  // Not processed.
                //}
                else return FALSE;  // Not processed.
            }
            break;

        case WM_TIMER:
            switch (wParam) {
            case 1:
                // Cancel the proxy-delay timer (make it a one-shot).
                KillTimer(hDlg, 1);

                if (gResult == noErr) {
                    //If we are dragging, the proxy will have already been
                    //updated.
                    if (!fDragging) {
                        // Do Filtering operation
                        DoFilterRect (globals);
                        // Invalidate Proxy Item (and repaint background if scale factor
                        // increased; i.e., if proxy image may have shrunk)...
                        {
                            static long originalScaleFactor = -1; //force initial update
                            UpdateProxy(hDlg, scaleFactor > originalScaleFactor);
                            originalScaleFactor = scaleFactor;
                        }
                    }//if not dragging
                }//if noErr
                break;

            case 2:
 
                if (GetForegroundWindow() == MyHDlg){ //Only make it work if FM dialog is active

                    //check for cursor in a MOUSEOVER control
                    RECT rcCtl;
                    int i;
                    int hitTest;
                    POINT pt;
                    static POINT oldpt;
                    BOOL RetVal;
                    int enableState;

                    //Added by Harald Heim, Jun 6,2002
                    //int NoInvisible;

                    GetCursorPos(&pt);

                    if (pt.x == oldpt.x && pt.y == oldpt.y) break; //Quit on same coordinates

                    for (i = 0; i < N_CTLS; ++i) {

                         //Check if control is enabled
                        if (gParams->ctl[i].tabControl>=0){
                            enableState = ctlEnabledAs (i); //Control is on a tab sheet
                        } else {
                           enableState = gParams->ctl[i].enabled;
                        }
                        
                        if ((gParams->ctl[i].inuse && ((gParams->ctl[i].properties & CTP_MOUSEMOVE) || (gParams->ctl[i].properties & CTP_MOUSEOVER)) && !(enableState != 3 && MouseOverWhenInvisible == 0)) ) {
                        //if (i==CTL_PREVIEW || (gParams->ctl[i].inuse && ((gParams->ctl[i].properties & CTP_MOUSEMOVE) || (gParams->ctl[i].properties & CTP_MOUSEOVER)) && !(enableState != 3 && MouseOverWhenInvisible == 0)) ) {
                        //if (i==CTL_PREVIEW || (gParams->ctl[i].inuse && ((gParams->ctl[i].properties & CTP_MOUSEMOVE) || (gParams->ctl[i].properties & CTP_MOUSEOVER)) && !(gParams->ctl[i].enabled != 3 && MouseOverWhenInvisible == 0)) ) {
                        //if (gParams->ctl[i].inuse && gParams->ctl[i].properties & CTP_MOUSEOVER) {
                            
                            if (i==CTL_PREVIEW) {
                                RetVal = GetWindowRect(GetDlgItem(hDlg, IDC_PROXY), &rcCtl); 
								//continue;

                            } else if (gParams->ctl[i].ctlclass == CC_TAB){ //Mouse event only for tab buttons
#if SKIN
								if (isSkinActive() && gParams->skinStyle>0){
									RetVal = GetWindowRect(gParams->ctl[i].hBuddy1, &rcCtl);
								} else 
#endif								
								{
									RECT rect;
									RetVal = GetWindowRect(gParams->ctl[i].hCtl, &rcCtl);
									TabCtrl_GetItemRect (gParams->ctl[i].hCtl, TabCtrl_GetItemCount(gParams->ctl[i].hCtl)-1, &rect);
									rcCtl.right = rcCtl.left + rect.right-1;
									rcCtl.bottom = rcCtl.top + rect.bottom-1;
								}
                            } else {
                                RetVal = GetWindowRect(gParams->ctl[i].hCtl, &rcCtl);
                            }
                            

                            //Include labels and edit box
                            if (gParams->ctl[i].ctlclass == CC_STANDARD || gParams->ctl[i].ctlclass == CC_SLIDER){
                                
                                hitTest = pt.x >= rcCtl.left && pt.x < rcCtl.right &&
                                          pt.y >= rcCtl.top  && pt.y < rcCtl.bottom;

#if SKIN
								if (!isSkinActive() || !(getAsyncKeyState(VK_LBUTTON) & 0x8000)){ //Avoid preview update problem when dragging slider 
									if (!hitTest){
										if (GetWindowRect(gParams->ctl[i].hBuddy1, &rcCtl)){
											hitTest = pt.x >= rcCtl.left && pt.x < rcCtl.right &&
													  pt.y >= rcCtl.top  && pt.y < rcCtl.bottom;
										}
										if (!hitTest) {
											if (GetWindowRect(gParams->ctl[i].hBuddy2, &rcCtl))
												hitTest = pt.x >= rcCtl.left && pt.x < rcCtl.right &&
														  pt.y >= rcCtl.top  && pt.y < rcCtl.bottom;
										}
									}
								}
#endif

                            } else { //Normal controls */
                                
                                hitTest = pt.x >= rcCtl.left && pt.x < rcCtl.right &&
                                          pt.y >= rcCtl.top  && pt.y < rcCtl.bottom;
                            }



                            //this control wants mouseover notifications
                            if (RetVal) {
                            //if (GetWindowRect(gParams->ctl[i].hCtl, &rcCtl)) {

                                if (hitTest) {
                                    //it's a hit!
                                    
                                     if (gParams->ctl[i].properties & CTP_MOUSEMOVE){
                                        //this control wants mousemove notifications

                                        PerformControlAction(
                                            i,                      //control index
                                            FME_MOUSEMOVE,          //FM event code
                                            0,                      //previous value
                                            wMsg,                   //message type
                                            0,                      //notification code
                                            FALSE,                  //fActionable
                                            TRUE,0);                  //fProcessed

                                    }
                                    
                                     if (gParams->ctl[i].properties & CTP_MOUSEOVER){ //i==CTL_PREVIEW || 
                                    
                                        if (i != iCurrentMouseoverCtl) {
                                            //MOUSEOUT to iCurrentMouseoverCtl if >= 0
                                            if (iCurrentMouseoverCtl >= 0) {
                                                PerformControlAction(
                                                    iCurrentMouseoverCtl,   //control index
                                                    FME_MOUSEOUT,           //FM event code
                                                    0,                      //previous value
                                                    wMsg,                   //message type
                                                    0,                      //notification code
                                                    FALSE,                  //fActionable
                                                    TRUE,0);                  //fProcessed

#if SKIN
												if (isSkinActive()){
													if (gParams->ctl[iCurrentMouseoverCtl].ctlclass == CC_STANDARD || gParams->ctl[iCurrentMouseoverCtl].ctlclass == CC_SLIDER){
														if (getAsyncKeyState(VK_LBUTTON) & 0x8000){ //Take swapped mouse buttons into account
															PerformControlAction(
																iCurrentMouseoverCtl,  //control index
																FME_CLICKED,            //FM event code
																-1, //previous value
																wMsg,               //message type
																0,               //notification code
																TRUE,
																TRUE,0);	
														}
													}
												}
#endif                                            
                                            }

                                           
                                            //MOUSEOVER to i
                                            iCurrentMouseoverCtl = i;
                                            PerformControlAction(
                                                iCurrentMouseoverCtl,   //control index
                                                FME_MOUSEOVER,          //FM event code
                                                0,                      //previous value
                                                wMsg,                   //message type
                                                0,                      //notification code
                                                FALSE,                  //fActionable
                                                TRUE,0);                  //fProcessed
                                        }
                                    }
                                    break;
                                }
                            }
                        }
                    } //for i
                    if (i >= N_CTLS) {  //dodgy!!!
                        //MOUSEOUT to iCurrentMouseoverCtl if >= 0
                        if (iCurrentMouseoverCtl >= 0) {
                            PerformControlAction(
                                iCurrentMouseoverCtl,   //control index
                                FME_MOUSEOUT,           //FM event code
                                0,                      //previous value
                                wMsg,                   //message type
                                0,                      //notification code
                                FALSE,                  //fActionable
                                TRUE,0);                  //fProcessed
                            
#if SKIN
							if (isSkinActive()){
								if (gParams->ctl[iCurrentMouseoverCtl].ctlclass == CC_STANDARD || gParams->ctl[iCurrentMouseoverCtl].ctlclass == CC_SLIDER){
									if (getAsyncKeyState(VK_LBUTTON) & 0x8000){ //Take swapped mouse buttons into account
										PerformControlAction(
											iCurrentMouseoverCtl,  //control index
											FME_CLICKED,            //FM event code
											-1, //previous value
											wMsg,               //message type
											0,               //notification code
											TRUE,
											TRUE,0);	
									}
								}
							}
#endif

							iCurrentMouseoverCtl = -1;

                        }						
                    }

                    oldpt = pt;
                }
                break;

            //Added by Harald Heim, Dec 13,2002
            case 3:
            case 4:
            case 5:
            case 6:
            case 7:
            case 8:
            case 9:
            case 10:
            case 11:
            case 12:

                //Trigger Timer Event
                PerformControlAction(
                    (int)wParam-3,   //(pseudo-)control index -- for setting FM global var 'n', 
                                // but 'ctlClass' will be garbage!
                    FME_TIMER,           //FM event code
                    0,                      //previous value
                    wMsg,                   //message type
                    0,                      //notification code
                    FALSE,                  //fActionable
                    TRUE,0);                  //fProcessed

                break;
             
            case 1000: //MouseWheel Timer
                KillTimer(hDlg, 1000);
                doAction(gParams->ctl[iCtl2].action);
                break;

            case 1001: //Preview Update Timer -> max. 50 times per second
                KillTimer(hDlg, 1001);
                doAction(gParams->ctl[iCtl2].action);
                break;

            //case 1002: //WM_PAINT -> Redraw Proxy
            //    KillTimer(hDlg, 1002);              
            //    break;

            default:
                return FALSE;   //not handled

            }//switch
            return TRUE;    //handled

        case WM_DESTROY:

			#if APP		
				DeleteObject(hbicon);
			#endif

			// Kill any leftover timers
            KillTimer(hDlg, 1);
            KillTimer(hDlg, 2);

            KillTimer(hDlg, 1000);
            KillTimer(hDlg, 1001);

            {   // Restore old dialog bitmap.
                if (hBmpOld && hdcMem) {
                    SelectObject(hdcMem, hBmpOld);
                }
                // Need to DeleteObject() the bitmap that was loaded.
                /*if (hBmpDialog) {
                    if (!DeleteObject(hBmpDialog)) {
                        //ErrorOk("DeleteObject #5 failed: %8.8x", hBmpDialog);
                    }
                }*/
                // Delete the memory DC.
                if (hdcMem) {
                    DeleteDC(hdcMem);
                }
            }

#define CHECK(what) (what)

            //Delete any acquired resources in user controls...
            {//scope
                int n;
                for (n = 0;  n < N_CTLS;  n++)
                {
                    if (gParams->ctl[n].inuse) {
                        //what about hCtl, hBuddy1, hBuddy2????
                        if (gParams->ctl[n].hBrush) {
                            CHECK(DeleteObject(gParams->ctl[n].hBrush));
                            gParams->ctl[n].hBrush = NULL;
                        }
#if 0
                        //NO! Fonts are shared among controls,
                        //and NT (but not Win9x) generates a check
                        //and vanishes Photoshop!
                        if (gParams->ctl[n].hFont) {
                            CHECK(DeleteObject(gParams->ctl[n].hFont));
                            gParams->ctl[n].hFont = NULL;
                        }
#endif
                        if (gParams->ctl[n].ctlclass == CC_IMAGE) {
                            //select old bitmaps back into DC's, then
                            //delete all image bitmaps and DC's.

                            if (gParams->ctl[n].im.hbmOld) {
                                SelectObject(gParams->ctl[n].im.hdcMem, gParams->ctl[n].im.hbmOld);
                                gParams->ctl[n].im.hbmOld = 0;
                            }
                            if (gParams->ctl[n].im.hbm) {
                                CHECK(DeleteObject(gParams->ctl[n].im.hbm));
                                gParams->ctl[n].im.hbm = 0;
                            }
                            if (gParams->ctl[n].im.hdcMem) {
                                CHECK(DeleteDC(gParams->ctl[n].im.hdcMem));
                                gParams->ctl[n].im.hdcMem = 0;
                            }

                            if (gParams->ctl[n].im.hbmOldAnd) {
                                SelectObject(gParams->ctl[n].im.hdcAnd, gParams->ctl[n].im.hbmOldAnd);
                                gParams->ctl[n].im.hbmOldAnd = 0;
                            }
                            if (gParams->ctl[n].im.hbmAnd) {
                                CHECK(DeleteObject(gParams->ctl[n].im.hbmAnd));
                                gParams->ctl[n].im.hbmAnd = 0;
                            }
                            if (gParams->ctl[n].im.hdcAnd) {
                                CHECK(DeleteDC(gParams->ctl[n].im.hdcAnd));
                                gParams->ctl[n].im.hdcAnd = 0;
                            }
                        }//if CC_IMAGE
                        else if (gParams->ctl[n].ctlclass == CC_METAFILE) {
                            //set no image, and retrieve previous image
                            HENHMETAFILE hPrevImage;
                            hPrevImage = (HENHMETAFILE)SendMessage(gParams->ctl[n].hCtl,
                                                                   STM_SETIMAGE,
                                                                   IMAGE_ENHMETAFILE,
                                                                   (LPARAM)NULL);
                            //delete previous image, if any
                            if (hPrevImage) {
                                CHECK(DeleteEnhMetaFile(hPrevImage));
                            }
                        }//if CC_METAFILE
                        else if (gParams->ctl[n].ctlclass == CC_BITMAP) {
                            //set no image, and retrieve previous bitmap
                            HBITMAP hPrevImage;
                            hPrevImage = (HBITMAP)SendMessage(gParams->ctl[n].hCtl,
                                                              STM_SETIMAGE,
                                                              IMAGE_BITMAP,
                                                              (LPARAM)NULL);
                            //delete previous bitmap, if any
                            if (hPrevImage) {
                                CHECK(DeleteObject(hPrevImage));
                            }
                        }//if CC_BITMAP
                    }//if inuse
                } //for n
            }//scope

			
			//Deactivate skin
			gParams->skinActive = 0;
			gParams->skinStyle = -1;
			gParams->gDialogGradientColor1 = GetSysColor(COLOR_BTNFACE);
#if SKIN
			freeSkin();//Workaround for Serif PhotoPlus
#endif
			//Info ("Exit");

			
            // Delete the proxy RGB buffer, if any.
            if (pRGBbuffer) {
#ifndef APP
                if (pRGBbufferID != 0){
			        UnlockBuffer(pRGBbufferID);
			        FreeBuffer(pRGBbufferID); 
		        } else
#endif
			        free(pRGBbuffer);
		        pRGBbufferID = 0;
                pRGBbuffer = NULL;
				pRGBbufferSize = 0;
            }

			#ifdef GDIPLUS
			{
				void exitGraphicsPlus();
				exitGraphicsPlus();
			}
			#endif

            return FALSE;   // Not processed.


        case WM_NCHITTEST:
//#if defined(USE_BITMAP) || defined(FML)
            {
                POINT   pt;
                UINT    nHitTest = HTCLIENT;

                pt.x = LOWORD(lParam);
                pt.y = HIWORD(lParam);
                
                
                //Test if Size grip
                if (sizeGrip){
                    nHitTest = (int)DefWindowProc(hDlg, wMsg, wParam, lParam);
                    if( nHitTest == HTCLIENT ) 
                    { 
                            RECT rc; 
                            GetWindowRect(hDlg, &rc ); 
                            rc.left = rc.right - GetSystemMetrics(SM_CXHSCROLL); 
                            rc.top = rc.bottom - GetSystemMetrics(SM_CYVSCROLL); 
                        
                            if( PtInRect(&rc, pt) ){ 
                                nHitTest = HTBOTTOMRIGHT;
                                return SetDlgMsgResult(hDlg, wMsg, nHitTest);
                            }
                    }
                }


                ScreenToClient(hDlg, &pt);
                if (pt.x >= gProxyRect.left && pt.x < gProxyRect.right &&
                    pt.y >= gProxyRect.top  && pt.y < gProxyRect.bottom)
                {   // It's in our proxy preview window...
                    nHitTest = HTCLIENT;
                }
//#ifdef FML
                else if (gParams->gDialogDragMode == 2)  //Drag=None
                {
                    // Call default window proc to perform hit test...
                    nHitTest = (int)DefWindowProc(hDlg, wMsg, wParam, lParam);
                    // Change title bar hit to client hit to prevent dragging...
                    if (nHitTest == HTCAPTION)
                        nHitTest = HTCLIENT;
                }
//#endif

                else if (gParams->gDialogDragMode > 2)  //Drag=Any Control
                {
                    nHitTest = (int)DefWindowProc(hDlg, wMsg, wParam, lParam);

                    if (nHitTest == HTCLIENT){
                        
                        RECT rcCtl;

                        GetWindowRect(gParams->ctl[gParams->gDialogDragMode].hCtl, &rcCtl);
                        ScreenToClient (hDlg, (LPPOINT)&rcCtl);
                        ScreenToClient (hDlg, (LPPOINT)&(rcCtl.right));

                        //if (pt.x >= rcCtl.left && pt.x < rcCtl.right &&
                        //    pt.y >= rcCtl.top  && pt.y < rcCtl.bottom) 
                        if (pt.y < rcCtl.bottom)
                        {    
                                nHitTest = HTCAPTION;
                                //fMoving = TRUE;
                        }
                    }
                    
                }  
                else
                {
                    

#ifndef NO_TITLE_BAR
                    // Call default window proc to perform hit test...
                    nHitTest = (int)DefWindowProc(hDlg, wMsg, wParam, lParam);
                    // Change client hit to title bar hit to force dragging...
                    if (nHitTest == HTCLIENT){

#endif
//#ifdef FML
                        if (gParams->gDialogDragMode == 1){   //Drag=Background
//#endif
                              if (getAsyncKeyState(VK_LBUTTON) & 0x8000){ //Take swapped mouse buttons into account
                                    nHitTest = HTCAPTION;
                                    //fMoving = TRUE;
                              }

                        }
                    }

                }
                return SetDlgMsgResult(hDlg, wMsg, nHitTest);
            }
//#endif //USE_BITMAP||FML
            return FALSE;   // Not processed -- pass to DefWindowProc().

        case WM_SIZE:
                      
            if (fPaint) //make sure that it isn't executed before the dialog appears, otherwise Memory Error
            {
                int fwSizeType = (int)wParam;      // resizing flag 
                int nWidth = LOWORD(lParam);  // width of client area 
                int nHeight = HIWORD(lParam); // height of client area 

				int dialogWidth = PixelsToHDBUs(nWidth);
				int dialogHeight = PixelsToVDBUs(nHeight);

				if (fwSizeType == SIZE_MAXIMIZED){
                    gParams->gDialogState = 2;
				} else {
                    gParams->gDialogState = 0;
                    //save current position and size of dialog
                    GetWindowRect(hDlg, &gParams->gDialogRectMin);
                }

                //if (!fMoving) setDialogStyleEx( 0x02000000L);

				fScaling = true;
                if(!gParams->scaleLock) {
                    //Info ("Scaling Controls");
					scaleCtls(dialogWidth, dialogHeight);
                }
                fScaling = false;

				//Repaint windows
				{
					/*int i;
					for (i=0;i<N_CTLS;i++){
						if (gParams->ctl[i].inuse && gParams->ctl[i].enabled){
							//InvalidateRect(gParams->ctl[i].hCtl, NULL, FALSE);
							//InvalidateRect(gParams->ctl[i].hCtl, NULL, TRUE);
							refreshCtl(i);
						}
					}*/
				}


                fSizing=true;

                PerformControlAction(
                            fwSizeType,             //control index
                            FME_SIZE,               //FM event code
                            fwSizeType,             //previous value
                            wMsg,                   //message type
                            0,                      //notification code
                            FALSE,                  //fActionable
                            TRUE,0);                  //fProcessed

				
                //When maximizing or restoring window there is no WM_EXITSIZEMOVE event, so we have to do this now
                //Because there is no way to distinguish a restore, we have to use fMaximized
                if (fwSizeType == SIZE_MAXIMIZED || fMaximized || fwSizeType == SIZE_MINIMIZED || fMinimized){ // || fwSizeType == SIZE_RESTORED
                    
                    if (fwSizeType == SIZE_MAXIMIZED) fMaximized = TRUE;
                    else fMaximized = FALSE;

					if (fwSizeType == SIZE_MINIMIZED) fMinimized = TRUE;
                    else fMinimized = FALSE;

                    //Make sure that the preview is displayed correctly
                    GetProxyItemRect (hDlg, &gProxyRect); 
                    SetupFilterRecordForProxy (globals, &scaleFactor, scaleFactor, &imageOffset);

                    PerformControlAction(
                                0,                      //control index
                                FME_EXITSIZE,           //FM event code
                                0,                      //previous value
                                WM_EXITSIZEMOVE,        //message type
                                0,                      //notification code
                                FALSE,                  //fActionable
                                TRUE,0);                  //fProcessed

					
#ifndef PLUGINGALAXY3
					RedrawWindow(MyHDlg,NULL,NULL,RDW_ERASE|RDW_INVALIDATE|RDW_ERASENOW|RDW_UPDATENOW);
                    DoFilterRect (MyGlobals);
					UpdateProxy(MyHDlg, FALSE);
#endif

					fSizing = FALSE;
                }         
				
            }    

            return FALSE;

#if 0
        case WM_ENTERSIZEMOVE:
            //double buffering
            if (!fMoving) setDialogStyleEx( 0x02000000L); //WS_EX_COMPOSITED
            fMoving = FALSE;
            /*if (fPaint) { //make sure that it isn't executed before the dialog appears, otherwise Memory Error

                PerformControlAction(
                            0,                      //control index
                            FME_ENTERSIZE,               //FM event code
                            0,                      //previous value
                            wMsg,                   //message type
                            0,                      //notification code
                            FALSE,                  //fActionable
                            TRUE,0);                  //fProcessed
            }*/

            return FALSE;

        /*case WM_MOVE:
            //double buffering
            //clearDialogStyleEx( 0x02000000L); //WS_EX_COMPOSITED
            fMoving = FALSE;
            return FALSE;*/
#endif
                    
        case WM_EXITSIZEMOVE:
            //double buffering
            //clearDialogStyleEx( 0x02000000L); //WS_EX_COMPOSITED
            //fMoving = FALSE;

            if (fSizing){

				fSizing = FALSE;
                
                //Make sure that the preview is displayed correctly
                GetProxyItemRect (hDlg, &gProxyRect); 
                SetupFilterRecordForProxy (globals, &scaleFactor, scaleFactor, &imageOffset);
                PerformControlAction(
                            0,                      //control index
                            FME_EXITSIZE,           //FM event code
                            0,                      //previous value
                            wMsg,                   //message type
                            0,                      //notification code
                            FALSE,                  //fActionable
                            TRUE,0);                  //fProcessed
                 
                //To avoid artifacts by size grip
                /*if (sizeGrip){
                    RedrawWindow(
                        hDlg,   // handle of entire dialog window
                        NULL,   // address of structure with update rectangle, NULL=>entire client area
                        NULL,   // handle of update region
                        RDW_ERASE|RDW_INVALIDATE|RDW_ERASENOW|RDW_UPDATENOW     // array of redraw flags
                    );
                }*/
           
				
				RedrawWindow(MyHDlg,NULL,NULL,RDW_ERASE|RDW_INVALIDATE|RDW_ERASENOW|RDW_UPDATENOW);
                DoFilterRect (MyGlobals);
				UpdateProxy(MyHDlg, FALSE);
				
            }

#if SKIN
			//Draw frame around preview
			if (isSkinActive() && gParams->skinStyle>0
				#if STYLER
					&& ctl(N_FXCTLS+21)==0
				#endif 
			){
				skinDrawPreviewFrame(CTL_PREVIEW);
			}
#endif

			//Draw Resize handle
			if (sizeGrip && getDialogWindowState()!=SW_MAXIMIZE) drawSizeGrip();


			return FALSE;

        case WM_GETMINMAXINFO:
            {
				LPMINMAXINFO lpmmi = (LPMINMAXINFO) lParam; // address of structure 
                if (MaxMinInfo.ptMinTrackSize.x >0 && MaxMinInfo.ptMinTrackSize.y >0)
                    lpmmi->ptMinTrackSize = (&MaxMinInfo)->ptMinTrackSize;
                if (MaxMinInfo.ptMaxTrackSize.x >0 && MaxMinInfo.ptMaxTrackSize.y >0)
                    lpmmi->ptMaxTrackSize = (&MaxMinInfo)->ptMaxTrackSize;


				if (getWindowsVersion()>=11) {
					static int oldWinState = SW_NORMAL;
					static int restoreWidth;
					static int restoreHeight;

					//Reduce flicker when maximizing window by scaling the controls before maximizing
					if (fDialogDisplayed && getDialogWindowState()==SW_MAXIMIZE){
						restoreWidth = getDialogWidth();
						restoreHeight = getDialogHeight();
						scaleCtls(getDialogMaxSize(0),getDialogMaxSize(1));
						updateAnchors(getDialogMaxSize(0),getDialogMaxSize(1));
						refreshWindow();
						oldWinState = SW_MAXIMIZE;
					//Reduce flicker when restoring window
					} else if (oldWinState==SW_MAXIMIZE){
						scaleCtls(restoreWidth,restoreHeight);
						updateAnchors(restoreWidth,restoreHeight);
						//refreshWindow();
						oldWinState = SW_NORMAL;
					}
				}


            }
            return FALSE;

        case WM_PAINT:
            
			fPaint = true;
			
			/*{
				static int cycle=0;
				char string[256] = "";
				cycle++;
				sprintf(string,"%d: WM_PAINT\n",cycle);
				OutputDebugString(string);
			}*/

			//if (!fSizing || getAsyncKeyState(VK_LBUTTON)>=0){ //!fSizing || 
				ShowOutputBuffer(globals, hDlg, fDragging);     // Draw Proxy Item
			/*} else {
				PAINTSTRUCT  ps;
				BeginPaint(hDlg,&ps);
				EndPaint(hDlg,&ps);
			}*/


			//Draw frame around preview
			if (getAsyncKeyState(VK_LBUTTON)>=0){
#if SKIN
				if (isSkinActive() && gParams->skinStyle>0
					#if STYLER
						&& ctl(N_FXCTLS+21)==0
					#endif 
				){
					skinDrawPreviewFrame(CTL_PREVIEW);
				}
#endif
				//Draw Resize handle
				if (sizeGrip && getDialogWindowState()!=SW_MAXIMIZE) drawSizeGrip();
			}


            return TRUE;    // SDK samples had FALSE, but that causes update
                            // problems when full-dragging a window across
                            // the proxy... does the "fix" introduce any new
                            // problems? 
            break;

/*
        case WM_NCPAINT:
            {
                //HRGN hrgn = (HRGN) wParam;   // handle of update region 
                RECT rect;
                RECT rcCtl;
                HDC hdc;
                int i;

                GetRgnBox((HRGN) wParam, &rect);

                //Info ("%d,%d - %d,%d",rect.left,rect.top,rect.right,rect.bottom);
                
                for (i = 0; i < N_CTLS; ++i) {
                    if (gParams->ctl[i].inuse  && gParams->ctl[i].enabled == 3) {
                        if (GetWindowRect(gParams->ctl[i].hCtl, &rcCtl)) {
                            ScreenToClient (hDlg, (LPPOINT)&rcCtl);
                            ScreenToClient (hDlg, (LPPOINT)&(rcCtl.right));
                            if (rect.left == rcCtl.left && rect.right == rcCtl.right && rect.top == rcCtl.top  && rect.bottom == rcCtl.bottom) {

                                if (gParams->ctl[i].style & WS_BORDER){
                                    
                                      hdc = GetWindowDC(hDlg);
                                      DrawEdge (hdc, &rect, EDGE_ETCHED, 0);
                                      ReleaseDC(hDlg, hdc);
                                }

                            }
                        }
                    }
                }
                
                //hdc = GetDCEx(hDlg, (HRGN)wParam, DCX_WINDOW|DCX_INTERSECTRGN);
                

                return 0;
            }
*/
        case WM_MOUSEMOVE:
            {
                int x = (SHORT) LOWORD(lParam);
                int y = (SHORT) HIWORD(lParam);

#if 1
                {
                    // relay this mouse message to the tooltip control...
                    MSG msg; 

                    msg.lParam = lParam; 
                    msg.wParam = wParam; 
                    msg.message = wMsg; 
                    msg.hwnd = hDlg;
#if 1   //not needed???
                    msg.time = 0;
                    msg.pt.x = 40;
                    msg.pt.y = 40;
#endif
                    SendMessage(ghTT, TTM_RELAYEVENT, 0, (LPARAM) &msg); 
                }
#endif
                if (fDragging && !fDrawing)
                {
                    POINT tempOffset;

                    fDrawing = true; //Make sure that dragging the preview works swiftly even on 4K screens

                    SetCursor(LoadCursor(NULL, IDC_SIZEALL));
                    ptNew.x = x;
                    ptNew.y = y;
#ifndef NOOPTIMZE_QUICK_TOGGLE
                    //Only update if image can be dragged in preview
                #ifndef APP
                    if ( gFmcp->x_start != 0 || gFmcp->y_start != 0 || gFmcp->x_end != gFmcp->X || gFmcp->y_end != gFmcp->Y ){
                #endif
                        fMouseMoved |= (ptNew.x - ptPrev.x) | (ptNew.y - ptPrev.y);
                #ifndef APP
                    }
                #endif
#endif
                    //Only update if image can be dragged in preview
                #ifndef APP
                    if (gFmcp->x_start != 0 || gFmcp->y_start != 0 || gFmcp->x_end != gFmcp->X || gFmcp->y_end != gFmcp->Y ){
                #endif
                        imageOffset.x -= (ptNew.x - ptPrev.x) * scaleFactor * gFmcp->scrollFactor;// / gFmcp->enlargeFactor;
                        imageOffset.y -= (ptNew.y - ptPrev.y) * scaleFactor * gFmcp->scrollFactor;// / gFmcp->enlargeFactor;
                #ifndef APP                
                    }
                #endif

					ptPrev = ptNew;
					GetProxyItemRect (hDlg, &gProxyRect);
                    tempOffset = imageOffset; // don't modify imageOffset while dragging
                    
                    //startClock();
                    if ( ((gParams->flags & AFH_ZERO_OUTPUT_DRAG) == AFH_ZERO_OUTPUT_DRAG) && gFmcp->x_start == 0 && gFmcp->y_start == 0 && gFmcp->x_end == gFmcp->X && gFmcp->y_end == gFmcp->Y ){
                        
                         //Temporarily deactivate Zero drag to avoid redraw problem
                         gParams->flags &= ~AFH_ZERO_OUTPUT_DRAG;
                         SetupFilterRecordForProxy (globals, &scaleFactor, scaleFactor, &tempOffset);
                         //Activate Zero drag again
                         gParams->flags |= AFH_ZERO_OUTPUT_DRAG;

                    }else {
                        SetupFilterRecordForProxy (globals, &scaleFactor, scaleFactor, &tempOffset);                        
                    }
                    //DebugLogVal(stopClock());

                    
                    if (gResult == noErr) {
                        // Run the filter only if want to drag the filtered output
                        // rather than the raw input...
                        if (gParams->flags & AFH_DRAG_FILTERED_OUTPUT)
                        {
                            DoFilterRect (globals);
                        }
                        UpdateProxy(hDlg, FALSE);
                    }//noErr
                    SetCursor(LoadCursor(NULL, IDC_SIZEALL)); //again, in case WAIT cursor appeared

                    //MOUSEOUT to iCurrentMouseoverCtl if >= 0
                    if (iCurrentMouseoverCtl >= 0) {
                        PerformControlAction(
                            iCurrentMouseoverCtl,   //control index
                            FME_MOUSEOUT,           //FM event code
                            0,                      //previous value
                            wMsg,                   //message type
                            0,                      //notification code
                            FALSE,                  //fActionable
                            TRUE,0);                  //fProcessed
                        iCurrentMouseoverCtl = -1;
                    }

                    return TRUE; // ??
                }

                //Proxy Box
                else if (x >= gProxyRect.left && x < gProxyRect.right &&
                         y >= gProxyRect.top && y < gProxyRect.bottom)
                {
                    
                    //if (gParams->ctl[CTL_PREVIEW].inuse){

						//Info ("%d, %d",GetCursor(),hcHand);
                        SetCursor(hcHand);
                
                        /*   // Why did Harald comment this out????

                             //Found the reason for commenting this out! This code sends a mouseout 
                             //to the preview itself which causes some problems in my FFP code. 
                             //iCurrentMouseoverCtl must be set to the preview control 
                             //at this point already.

                        //MOUSEOUT to iCurrentMouseoverCtl if >= 0
                        if (iCurrentMouseoverCtl >= 0) {
                            PerformControlAction(
                                iCurrentMouseoverCtl,   //control index
                                FME_MOUSEOUT,           //FM event code
                                0,                      //previous value
                                wMsg,                   //message type
                                0,                      //notification code
                                FALSE,                  //fActionable
                                TRUE,0);                  //fProcessed
                            iCurrentMouseoverCtl = -1;
                        }
                        */
                    
                        //Added by Harald Heim, Dec 13, 2002
                        //Send MouseMove for Proxy
                    
                        
                        if (gParams->ctl[CTL_PREVIEW].properties & CTP_MOUSEMOVE){
                            PerformControlAction(
                                CTL_PREVIEW,   //control index
                                FME_MOUSEMOVE,           //FM event code
                                0,                      //previous value
                                wMsg,                   //message type
                                0,                      //notification code
                                FALSE,                  //fActionable
                                TRUE,0);                  //fProcessed
                        }
                    
                    //}


#if 1
                    return TRUE; //??
#else
                    return FALSE; //so underlying controls can see mouse and pass to tooltips???
#endif
                }
                else
                {
                    //check for hit in a MOUSEOVER control
                    RECT rcCtl;
                    int i;
                    int D = 0;
                    int hitTest;
                    int enableState;

                    for (i = 0; i < N_CTLS; ++i) {
                        
                        //Check if control is enabled
                        if (gParams->ctl[i].tabControl>=0){
                            enableState = ctlEnabledAs (i); //Control is on a tab sheet
                        } else {
                           enableState = gParams->ctl[i].enabled;
                        }


                        if (gParams->ctl[i].inuse && ((gParams->ctl[i].properties & CTP_MOUSEMOVE) || (gParams->ctl[i].properties & CTP_MOUSEOVER)) && enableState == 3) {
                        //if (gParams->ctl[i].inuse && (gParams->ctl[i].properties & CTP_MOUSEOVER) && !(gParams->ctl[i].enabled != 3 && MouseOverWhenInvisible == 0)) {
                        //if (gParams->ctl[i].inuse && gParams->ctl[i].enabled == 3 && gParams->ctl[i].properties & CTP_MOUSEOVER) {
                        //if (gParams->ctl[i].inuse && gParams->ctl[i].properties & CTP_MOUSEOVER) {

                            //this control wants mouseover notifications
                            if (GetWindowRect(gParams->ctl[i].hCtl, &rcCtl)) {
                                ScreenToClient (hDlg, (LPPOINT)&rcCtl);
                                ScreenToClient (hDlg, (LPPOINT)&(rcCtl.right));

                                //Mouse event only for tab buttons
                                if (gParams->ctl[i].ctlclass == CC_TAB){
#if SKIN
                                    if (isSkinActive() && gParams->skinStyle>0){
										GetWindowRect(gParams->ctl[i].hBuddy1, &rcCtl);
									} else 
#endif									
									{
										RECT rect;
										TabCtrl_GetItemRect (gParams->ctl[i].hCtl, TabCtrl_GetItemCount(gParams->ctl[i].hCtl)-1, &rect);
										rcCtl.right = rcCtl.left + rect.right-1;
										rcCtl.bottom = rcCtl.top + rect.bottom-1;
									}
                                }                                    
                                            
                                hitTest = x >= rcCtl.left-D && x < rcCtl.right+D &&
                                          y >= rcCtl.top-D  && y < rcCtl.bottom+D;
                                

                                if (hitTest) {
                                    //it's a hit!
                                    
                                    if (gParams->ctl[i].properties & CTP_MOUSEMOVE){
                                        //this control wants mousemove notifications

                                        PerformControlAction(
                                            i,                      //control index
                                            FME_MOUSEMOVE,          //FM event code
                                            0,                      //previous value
                                            wMsg,                   //message type
                                            0,                      //notification code
                                            FALSE,                  //fActionable
                                            TRUE,0);                  //fProcessed

                                    } 
                                    
                                    if (gParams->ctl[i].properties & CTP_MOUSEOVER){
                                    
                                        if (i != iCurrentMouseoverCtl) {
                                            //MOUSEOUT to iCurrentMouseoverCtl if >= 0
                                            if (iCurrentMouseoverCtl >= 0) {
                                                PerformControlAction(
                                                    iCurrentMouseoverCtl,   //control index
                                                    FME_MOUSEOUT,           //FM event code
                                                    0,                      //previous value
                                                    wMsg,                   //message type
                                                    0,                      //notification code
                                                    FALSE,                  //fActionable
                                                    TRUE,0);                  //fProcessed
                                            }
                                                
                                            //MOUSEOVER to i
                                            iCurrentMouseoverCtl = i;
                                            PerformControlAction(
                                                iCurrentMouseoverCtl,   //control index
                                                FME_MOUSEOVER,          //FM event code
                                                0,                      //previous value
                                                wMsg,                   //message type
                                                0,                      //notification code
                                                FALSE,                  //fActionable
                                                TRUE,0);                  //fProcessed
                                        }
                                        
                                    }
                                    
                                    break;

                                }
                            }
                            
                        }
                    } //for i

                    if (i >= N_CTLS) {  //dodgy!!!
                        //MOUSEOUT to iCurrentMouseoverCtl if >= 0
                        if (iCurrentMouseoverCtl >= 0) {
                            PerformControlAction(
                                iCurrentMouseoverCtl,   //control index
                                FME_MOUSEOUT,           //FM event code
                                0,                      //previous value
                                wMsg,                   //message type
                                0,                      //notification code
                                FALSE,                  //fActionable
                                TRUE,0);                  //fProcessed
                            
#if SKIN
							if (isSkinActive()){
								if (gParams->ctl[iCurrentMouseoverCtl].ctlclass == CC_STANDARD || gParams->ctl[iCurrentMouseoverCtl].ctlclass == CC_SLIDER){
									if (getAsyncKeyState(VK_LBUTTON) & 0x8000){ //Take swapped mouse buttons into account
										PerformControlAction(
											iCurrentMouseoverCtl,  //control index
											FME_CLICKED,            //FM event code
											-1, //previous value
											wMsg,               //message type
											0,               //notification code
											TRUE,
											TRUE,0);	
									}
								}
							}
#endif

							iCurrentMouseoverCtl = -1;

                        }
                    }
                }
            }
            return FALSE;
            break;

			case WM_NOTIFY:
            {
				iCmd = COMMANDCMD(wParam, lParam);

				hCtrl = ((LPNMHDR)lParam)->hwndFrom;
                iCtl = (int)GetWindowLong(hCtrl, GWL_ID);
                if (iCtl < IDC_CTLBASE || iCtl >= IDC_CTLBASE + N_CTLS)
                    return FALSE;
                iCtl -= IDC_CTLBASE;

                static int selchange = false;
                
                switch (((LPNMHDR)lParam)->code)
				{
					case TCN_SELCHANGE:
					{ 
						int val = TabCtrl_GetCurSel(hCtrl);
						int oldvalue; //i, 
                        
						if (val!=-1) {
							
							selchange = true;
							oldvalue = gParams->ctl[iCtl].val;
							setCtlVal (iCtl, val);

							//already done in setCtlVal()
							/*
							//lockCtl(iCtl);
                           	for (i = 0; i < N_CTLS; ++i) {
								if (gParams->ctl[i].inuse && gParams->ctl[i].tabControl == iCtl &&  gParams->ctl[i].tabSheet != -1){
									sub_enableCtl(i);	//MWVDL 2008-07-18
								}	
							}
							//unlockCtl(iCtl); 
							*/
							
                            return PerformControlAction(
								iCtl,  //control index
								FME_CLICKED,            //FM event code
								oldvalue, //previous value
								wMsg,               //message type
								iCmd,               //notification code
								FALSE,
								TRUE,0);

						}

					} 
					
					case NM_CLICK:
					{

						int val;
						int oldvalue;

						if (gParams->ctl[iCtl].ctlclass==CC_TAB){
							//int i, previewUpdate = false;
							//int subtab;
							//val = TabCtrl_GetCurSel(hCtrl);
						
							//Shift click on tab sheet button
							if (getAsyncKeyState(VK_SHIFT)<=-32767){
								if (!selchange){ //Tab is already active

									resetTabCtls(iCtl);
/*
									for (i = 0; i < N_CTLS; ++i) {

										//Get sub tab
										if (gParams->ctl[i].tabControl > 0) subtab = gParams->ctl[gParams->ctl[i].tabControl].tabControl; else subtab = -1;

										if (gParams->ctl[i].inuse && gParams->ctl[i].defval != -1 && 
											((gParams->ctl[i].enabled == 3 && gParams->ctl[i].tabControl == iCtl &&  gParams->ctl[i].tabSheet == val) || subtab == iCtl)
										){
										
											oldvalue = gParams->ctl[i].val;
											setCtlVal(i,gParams->ctl[i].defval);
											if (gParams->ctl[i].ctlclass == CC_OWNERDRAW) setCtlColor(i,gParams->ctl[i].defval);
											
											if (gParams->ctl[i].ctlclass == CC_CHECKBOX || gParams->ctl[i].ctlclass == CC_COMBOBOX){
												PerformControlAction(
													i,  //control index
													FME_CLICKED,            //FM event code
													oldvalue, //previous value
													wMsg,               //message type
													iCmd,               //notification code
													FALSE,
													TRUE,0);
											}

											if (gParams->ctl[i].action == CA_PREVIEW) previewUpdate = true;
										}
									
									}

									PerformControlAction(
										iCtl,  //control index
										FME_TABRESET,            //FM event code
										0,		//previous value
										wMsg,               //message type
										iCmd,               //notification code
										FALSE,
										TRUE,0);
									
									if (previewUpdate) doAction(CA_PREVIEW);
*/

								}
							}

							selchange = false;

						} else if (getCtlClass(iCtl)==CC_LISTVIEW){

							LPNMITEMACTIVATE lpnmitem = (LPNMITEMACTIVATE) lParam;
							val =  lpnmitem->iItem;

							oldvalue = gParams->ctl[iCtl].val;
							setCtlVal (iCtl, val);
							doEvents(); //Avoid selection delay

							return PerformControlAction(
								iCtl,  //control index
								FME_CLICKED,            //FM event code
								oldvalue, //previous value
								wMsg,               //message type
								iCmd,               //notification code
								TRUE,
								TRUE,0);	
							
						}

						break;
					}

					case NM_RCLICK:

						if (getCtlClass(iCtl)==CC_LISTVIEW){

							LPNMITEMACTIVATE lpnmitem = (LPNMITEMACTIVATE) lParam;
							int val =  lpnmitem->iItem;
							int oldvalue;

							oldvalue = gParams->ctl[iCtl].val;
							setCtlVal (iCtl, val);
							doEvents(); //Avoid selection delay

							return PerformControlAction(
								iCtl,  //control index
								FME_CLICKED,            //FM event code
								oldvalue, //previous value
								wMsg,               //message type
								iCmd,               //notification code
								TRUE,
								TRUE,0);	

							/*if (gParams->ctl[iCtl].inuse && (gParams->ctl[iCtl].properties & CTP_CONTEXTMENU) && !(gParams->ctl[iCtl].enabled != 3) ){

								PerformControlAction(
                                    iCtl,   //control index
                                    FME_CONTEXTMENU,        //FM event code
                                    0,                      //previous value
                                    wMsg,                   //message type
                                    0,                      //notification code
                                    FALSE,                  //fActionable
                                    TRUE,0);                  //fProcessed
							}*/

						}
					
						break;

					case TBN_DROPDOWN:
						{
							LPNMTOOLBAR lpnmtb = (LPNMTOOLBAR) lParam;

							gParams->ctl[iCtl].val = lpnmtb->iItem;
							fmc.pre_ctl[iCtl] = lpnmtb->iItem;

							return PerformControlAction(
								iCtl,				//control index
								FME_CONTEXTMENU,	//FM event code
								0,					//previous value
								wMsg,               //message type
								iCmd,               //notification code
								FALSE,
								TRUE,0);

						}
						break;

					case TBN_HOTITEMCHANGE:
						{
							LPNMTBHOTITEM lpnmhi = (LPNMTBHOTITEM) lParam;
							//TBBUTTON tbButton;
							//int n_buttons = SendMessage(gParams->ctl[iCtl].hCtl, TB_BUTTONCOUNT, 0, 0); 
							//int i, active=0;

							if (gParams->ctl[iCtl].properties & CTP_MOUSEOVER){

								PerformControlAction(
                                    iCtl,					//control index
                                    FME_MOUSEOVER,          //FM event code
                                    lpnmhi->idNew,          //previous value
                                    wMsg,                   //message type
                                    0,                      //notification code
                                    FALSE,                  //fActionable
                                    TRUE,0);                  //fProcessed

							}

							//if (lpnmhi->idNew!=0) Info ("%d",lpnmhi->idNew);

							/*
							if ( gParams->ctl[iCtl].data !=-1){
								gParams->ctl[iCtl].data = -1;
								return PerformControlAction(
									iCtl,  //control index
									FME_CONTEXTMENU,  //FM event code
									lpnmhi->idNew,     //previous value
									wMsg,               //message type
									iCmd,               //notification code
									FALSE,
									TRUE,0);
							}*/

							/*
							gParams->ctl[iCtl].val = lpnmhi->idNew;

							return PerformControlAction(
								iCtl,  //control index
								FME_CONTEXTMENU,  //FM event code
								lpnmhi->idNew,     //previous value
								wMsg,               //message type
								iCmd,               //notification code
								FALSE,
								TRUE,0);*/

							//SendMessage(gParams->ctl[iCtl].hCtl,TB_SETHOTITEM,-1,0);
							//SendMessage(gParams->ctl[iCtl].hCtl,TB_SETHOTITEM,lpnmhi->idNew,0);

							/*
							for (i=0;i<n_buttons;i++){
								SendMessage(gParams->ctl[iCtl].hCtl,TB_GETBUTTON, i, (LPARAM)&tbButton);
								if (tbButton.fsState == TBSTATE_MARKED) {active=true; break;}
							}

							if (lpnmhi->idNew>0) Info ("Highlighted: %d\nOld: %d\nactive: %d",lpnmhi->idNew,lpnmhi->idOld,active);
							*/

						}
						break;

//#if 0
					case LVN_ITEMCHANGED:
					{
						int oldvalue = gParams->ctl[iCtl].val;
						int val = ListView_GetNextItem(gParams->ctl[iCtl].hCtl,-1, LVNI_FOCUSED); //LVNI_SELECTED
						
/*
						#define LVHT_EX_GROUP_HEADER       0x10000000
						#define LVHT_EX_GROUP_FOOTER       0x20000000
						#define LVHT_EX_GROUP_COLLAPSE     0x40000000
						#define LVHT_EX_GROUP_BACKGROUND   0x80000000
						#define LVHT_EX_GROUP_STATEICON    0x01000000
						#define LVHT_EX_GROUP_SUBSETLINK   0x02000000
						#define LVHT_EX_GROUP              (LVHT_EX_GROUP_BACKGROUND | LVHT_EX_GROUP_COLLAPSE | LVHT_EX_GROUP_FOOTER | LVHT_EX_GROUP_HEADER | LVHT_EX_GROUP_STATEICON | LVHT_EX_GROUP_SUBSETLINK)
						#define ListView_HitTestEx(hwndLV, pinfo) \
								(int)SNDMSG((hwndLV), LVM_HITTEST, (WPARAM)-1, (LPARAM)(LV_HITTESTINFO *)(pinfo))
	                        
						LVHITTESTINFO pinfo;
						POINT coords;
						RECT rcCtl;
						POINT pt;

						GetWindowRect(gParams->ctl[iCtl].hCtl, &rcCtl);
						GetCursorPos(&pt);
						pinfo.pt.x = pt.x-rcCtl.left;
						pinfo.pt.y = pt.y-rcCtl.top;
						pinfo.flags = LVHT_EX_GROUP;
						ListView_HitTestEx(gParams->ctl[iCtl].hCtl,&pinfo);
						if (pinfo.iGroup!=0)
							Info ("Item: %d\nGroup: %d",pinfo.iItem,pinfo.iGroup);
						
						//SetWindowLong(gParams->ctl[124].hCtl, GWL_STYLE, GetWindowLong(gParams->ctl[124].hCtl, GWL_STYLE) & ~WS_VSCROLL); 
						//ShowScrollBar(gParams->ctl[124].hCtl,SB_VERT,TRUE);
*/

						
						if (val != oldvalue && val != -1){
							//Only if key is used, otherwise it may be triggered twice
							if (getAsyncKeyState(VK_LBUTTON)>=0 && getAsyncKeyState(VK_RBUTTON)>=0){

								SetFocus (gParams->ctl[iCtl].hCtl); //Make sure that focus is on list view
								setCtlVal (iCtl, val);	
								doEvents(); //Avoid selection delay

								return PerformControlAction(
									iCtl,  //control index
									FME_CLICKED,            //FM event code
									oldvalue, //previous value
									wMsg,               //message type
									iCmd,               //notification code
									TRUE,
									TRUE,0);
							}
						}

					}
					break;
//#endif
				}
            
				return TRUE;
            }
			
			case WM_LBUTTONDOWN:
            {
                int x = (SHORT) LOWORD(lParam);
                int y = (SHORT) HIWORD(lParam);

#if 1
                {
                    // relay this mouse message to the tooltip control...
                    MSG msg; 

                    msg.lParam = lParam; 
                    msg.wParam = wParam; 
                    msg.message = wMsg; 
                    msg.hwnd = hDlg; 
                    SendMessage(ghTT, TTM_RELAYEVENT, 0, (LPARAM) &msg); 
                }
#endif

				if (x >= gProxyRect.left && x < gProxyRect.right &&
                    y >= gProxyRect.top && y < gProxyRect.bottom)
                {
                    
                    //if (gParams->ctl[CTL_PREVIEW].inuse){
                        //Merged by Harald Heim, May 20, 2003
                        if (ClickDrag == 0)
                        {
                    
                            //Allow Shift click with hand tool
						#if defined(COLORWASHER) || defined(FOCALBLADE) || defined(NOISECONTROL)

							if (getAsyncKeyState(VK_SHIFT)<0){

								PerformControlAction(
									CTL_PREVIEW,   //control index
									FME_LEFTCLICKED_DOWN,   //FM event code
									0,                      //previous value
									wMsg,                   //message type
									0,                      //notification code
									FALSE,                  //fActionable
									TRUE,0);                  //fProcessed

							} else {

						#endif


								SetCapture(hDlg);
								SetCursor(LoadCursor(NULL, IDC_SIZEALL));
								ptPrev.x = x;
								ptPrev.y = y;
		#ifndef NOOPTIMZE_QUICK_TOGGLE
								fMouseMoved = 0;
		#endif
								fDragging = TRUE;
                                fDrawing = false;

								if (gParams->ctl[CTL_PREVIEW].properties & CTP_PREVIEWDRAG){
									PerformControlAction(
										CTL_PREVIEW,   //control index
										FME_PREVIEWDRAG,        //FM event code
										0,                      //previous value
										wMsg,                   //message type
										0,                      //notification code
										FALSE,                  //fActionable
										TRUE,0);                  //fProcessed
								}


						#if defined(COLORWASHER) || defined (FOCALBLADE) || defined(NOISECONTROL)
							}
						#endif

						
                        
                            gParams->flags |= AFH_DRAGGING_PROXY;
                            UpdateProxy(hDlg, FALSE);  // To display raw input.
                            return TRUE; //??
                        }
                        else
                        {
                            //send FME_LEFTCLICKED_DOWN event for Proxy
                            SetCursor(hcHand);
                            PerformControlAction(
                                CTL_PREVIEW,   //control index
                                FME_LEFTCLICKED_DOWN,   //FM event code
                                0,                      //previous value
                                wMsg,                   //message type
                                0,                      //notification code
                                FALSE,                  //fActionable
                                TRUE,0);                  //fProcessed
                        }
                    //}

                }
/*
				else 
				{
                    RECT rcCtl;
                    int i;
                    int D = 0;

					#define LVHT_EX_GROUP_HEADER       0x10000000
					#define LVHT_EX_GROUP_FOOTER       0x20000000
					#define LVHT_EX_GROUP_COLLAPSE     0x40000000
					#define LVHT_EX_GROUP_BACKGROUND   0x80000000
					#define LVHT_EX_GROUP_STATEICON    0x01000000
					#define LVHT_EX_GROUP_SUBSETLINK   0x02000000
					#define LVHT_EX_GROUP              (LVHT_EX_GROUP_BACKGROUND | LVHT_EX_GROUP_COLLAPSE | LVHT_EX_GROUP_FOOTER | LVHT_EX_GROUP_HEADER | LVHT_EX_GROUP_STATEICON | LVHT_EX_GROUP_SUBSETLINK)
					#define ListView_HitTestEx(hwndLV, pinfo) \
							(int)SNDMSG((hwndLV), LVM_HITTEST, (WPARAM)-1, (LPARAM)(LV_HITTESTINFO *)(pinfo))
                    
					for (i = 0; i < N_CTLS; ++i) {
                        if (gParams->ctl[i].inuse  && gParams->ctl[i].enabled == 3 && gParams->ctl[i].ctlclass == CC_LISTVIEW){
                            
							LVHITTESTINFO pinfo;
							POINT coords = {x,y};
							pinfo.pt = coords;
							pinfo.flags = LVHT_EX_GROUP;
							ListView_HitTestEx(gParams->ctl[i].hCtl,&pinfo);
							Info ("Item: %d\nGroup: %d",pinfo.iItem,pinfo.iGroup);
                        }
                    } //for i
                }
*/
                
                /*else
                {
                    //check for hit
                    RECT rcCtl;
                    int i;
                    int D = 0;
                    for (i = 0; i < N_CTLS; ++i) {
 
                        if (gParams->ctl[i].inuse  && gParams->ctl[i].enabled == 3) { //&& gParams->ctl[i].ctlclass == CC_OWNERDRAW
                        
                            //this control needs notifications
                            if (GetWindowRect(gParams->ctl[i].hCtl, &rcCtl)) {
                                ScreenToClient (hDlg, (LPPOINT)&rcCtl);
                                ScreenToClient (hDlg, (LPPOINT)&(rcCtl.right));
                                if (x >= rcCtl.left-D && x < rcCtl.right+D &&
                                    y >= rcCtl.top-D  && y < rcCtl.bottom+D) {
                                    //it's a hit!
                                    
                                    //if (gParams->ctl[i].properties & CTP_MOUSEMOVE){
                                        //this control wants mousemove notifications

                                        PerformControlAction(
                                            i,                      //control index
                                            FME_LEFTCLICKED_DOWN,   //FM event code
                                            0,                      //previous value
                                            wMsg,                   //message type
                                            0,                      //notification code
                                            FALSE,                  //fActionable
                                            TRUE,0);                  //fProcessed

                                    //} 
                                    
                                    break;

                                }
                            }
                            
                        }
                    } //for i
                }*/


            }

            return FALSE;
            break;

        case WM_LBUTTONUP:
#if 1
            {
                // relay this mouse message to the tooltip control...
                MSG msg; 

                msg.lParam = lParam; 
                msg.wParam = wParam; 
                msg.message = wMsg; 
                msg.hwnd = hDlg; 
                SendMessage(ghTT, TTM_RELAYEVENT, 0, (LPARAM) &msg); 
            }
#endif

            {
                int x = (SHORT) LOWORD(lParam);
                int y = (SHORT) HIWORD(lParam);

                //if (gParams->ctl[CTL_PREVIEW].inuse){
                    if (fDragging)
                    {
            
                        if (x >= gProxyRect.left && x < gProxyRect.right &&
                            y >= gProxyRect.top && y < gProxyRect.bottom)
                        {
                            SetCursor(hcHand);
                        }
                        else
                        {
                            //SetCursor(LoadCursor(NULL, IDC_ARROW));  //???
                        }
                        ReleaseCapture();
						fDragging = FALSE;
                        gParams->flags &= ~AFH_DRAGGING_PROXY;
                        ptNew.x = x;
                        ptNew.y = y;
        #ifndef NOOPTIMZE_QUICK_TOGGLE
                        //Only update if image can be dragged in preview
                     #ifndef APP
                        if (gFmcp->x_start != 0 || gFmcp->y_start != 0 || gFmcp->x_end != gFmcp->X || gFmcp->y_end != gFmcp->Y ){
                    #endif
                            fMouseMoved |= (ptNew.x - ptPrev.x) | (ptNew.y - ptPrev.y);
                    #ifndef APP
                        }
                    #endif
        #endif
                        //Only update if image can be dragged in preview
                    #ifndef APP
                        if (gFmcp->x_start != 0 || gFmcp->y_start != 0 || gFmcp->x_end != gFmcp->X || gFmcp->y_end != gFmcp->Y ){
                    #endif
                            imageOffset.x -= (ptNew.x - ptPrev.x) * scaleFactor * gFmcp->scrollFactor;// / gFmcp->enlargeFactor;
                            imageOffset.y -= (ptNew.y - ptPrev.y) * scaleFactor * gFmcp->scrollFactor;// / gFmcp->enlargeFactor;
                    #ifndef APP
                        }
                    #endif
                        GetProxyItemRect (hDlg, &gProxyRect); 


                        if ( ((gParams->flags & AFH_ZERO_OUTPUT_DRAG) == AFH_ZERO_OUTPUT_DRAG) && gFmcp->x_start == 0 && gFmcp->y_start == 0 && gFmcp->x_end == gFmcp->X && gFmcp->y_end == gFmcp->Y ){
                        
                             //Temporarily deactivate Zero drag to avoid redraw problem
                             gParams->flags &= ~AFH_ZERO_OUTPUT_DRAG;
                             SetupFilterRecordForProxy (globals, &scaleFactor, scaleFactor, &imageOffset);
                             //Activate Zero drag again
                             gParams->flags |= AFH_ZERO_OUTPUT_DRAG;

                        }else {
                            SetupFilterRecordForProxy (globals, &scaleFactor, scaleFactor, &imageOffset);
                        }


                        if (gResult == noErr) {
                            // Do filter and update proxy...
        #ifndef NOOPTIMZE_QUICK_TOGGLE
                            //if mouse didn't move since LBUTTONDOWN, no need to run filter...
                            //gives a quick before/after preview toggle

                            if (fMouseMoved) {
                                
                                //Setup x_start etc.
#if BIGDOCUMENT
								VRect scaledFilterRect;
                                gFmcp->filterRect32 = GetFilterRect(); //needed??
								scaledFilterRect = gFmcp->filterRect32;
                                scaleRect32(&scaledFilterRect, 1, gFmcp->scaleFactor);
								gFmcp->inRect32 = GetInRect();	//needeed??
                                gFmcp->x_start = gFmcp->inRect32.left - scaledFilterRect.left;
                                gFmcp->y_start = gFmcp->inRect32.top - scaledFilterRect.top;
#else
								Rect scaledFilterRect;
                                copyRect (&scaledFilterRect, &gStuff->filterRect);
                                scaleRect (&scaledFilterRect, 1, gFmcp->scaleFactor);
                                gFmcp->x_start = gStuff->inRect.left - scaledFilterRect.left;
                                gFmcp->y_start = gStuff->inRect.top - scaledFilterRect.top;
#endif

                                gFmcp->x_end = gFmcp->x_start + gFmcp->columns;
                                gFmcp->y_end = gFmcp->y_start + gFmcp->rows;
                            
                                if (gParams->ctl[CTL_PREVIEW].properties & CTP_PREVIEWDRAG){
                                    PerformControlAction(
                                        CTL_PREVIEW,   //control index
                                        FME_PREVIEWDRAG,        //FM event code
                                        1,                      //previous value
                                        wMsg,                   //message type
                                        0,                      //notification code
                                        FALSE,                  //fActionable
                                        TRUE,0);                  //fProcessed
                                }
    
                                DoFilterRect (globals);
                                fMouseMoved = 0;
                            }
        #else
                                DoFilterRect (globals);
        #endif
                        
                            UpdateProxy(hDlg, FALSE);
                        }//noErr
                        return TRUE;
                    }

                    //Info ("WM_LBUTTONUP\nfDragging: %d\nx: %d, y: %d\n%d,%d - %d,%d",fDragging,x,y,gProxyRect.left,gProxyRect.top,gProxyRect.right,gProxyRect.bottom);


                    //Added by Harald Heim, Dec 13, 2002
                    if (x >= gProxyRect.left && x < gProxyRect.right &&
                            y >= gProxyRect.top && y < gProxyRect.bottom)
                        {                           
                            //Info ("Should send FME_LEFTCLICKED_UP now");
                            
                            //send FME_LEFTCLICKED_UP event for Proxy
                            //SetCursor(hcHand);
                            PerformControlAction(
                                CTL_PREVIEW,   //control index
                                FME_LEFTCLICKED_UP,           //FM event code
                                0,                      //previous value
                                wMsg,                   //message type
                                0,                      //notification code
                                FALSE,                  //fActionable
                                TRUE,                   //fProcessed
                                1);                     //override busy logic

                    }

                //}//gParams->ctl[CTL_PREVIEW].inuse
            }

            return FALSE;
            break;


        case WM_CONTEXTMENU:

            {
                int x = (SHORT) LOWORD(lParam);
                int y = (SHORT) HIWORD(lParam);

                RECT rcCtl;
                int i;

                for (i = 0; i < N_CTLS; ++i) {
                    
                    if (gParams->ctl[i].inuse && (gParams->ctl[i].properties & CTP_CONTEXTMENU) && !(gParams->ctl[i].enabled != 3) ){
                    
                        if (GetWindowRect(gParams->ctl[i].hCtl, &rcCtl)) {
                            
                            if (x >= rcCtl.left && x < rcCtl.right &&
                                y >= rcCtl.top  && y < rcCtl.bottom) {
                                //it's a hit!
                                
								if (gParams->ctl[i].ctlclass==CC_LISTVIEW){
										
									if (!IsWindowVisible(gParams->ctl[i].hCtl)) break;

									PerformControlAction(
										i,  //control index
										FME_CLICKED,            //FM event code
										0,		//previous value
										wMsg,               //message type
										0,               //notification code
										TRUE,
										TRUE,0);
								}

								PerformControlAction(
                                    i,   //control index
                                    FME_CONTEXTMENU,        //FM event code
                                    0,                      //previous value
                                    wMsg,                   //message type
                                    0,                      //notification code
                                    FALSE,                  //fActionable
                                    TRUE,0);                  //fProcessed

                                break;
                            }
                        }
                    }
                } //for i

                return false;
            }
            

#if 1
        case WM_RBUTTONDOWN:
            
            //Added by Harald Heim, Aug 9, 2002
            {
                // relay this mouse message to the tooltip control...
                MSG msg; 

                msg.lParam = lParam; 
                msg.wParam = wParam; 
                msg.message = wMsg; 
                msg.hwnd = hDlg; 
                SendMessage(ghTT, TTM_RELAYEVENT, 0, (LPARAM) &msg); 
            }

            {
                int x = (SHORT) LOWORD(lParam);
                int y = (SHORT) HIWORD(lParam);


                if (x >= gProxyRect.left && x < gProxyRect.right &&
                    y >= gProxyRect.top && y < gProxyRect.bottom) // && gParams->ctl[CTL_PREVIEW].inuse
                {

                    //Merged by Harald Heim, May 20, 2003
                    if (ClickDrag == 1)
                    {
                        SetCapture(hDlg);
                        SetCursor(LoadCursor(NULL, IDC_SIZEALL));
                        ptPrev.x = x;
                        ptPrev.y = y;
#ifndef NOOPTIMZE_QUICK_TOGGLE
                        fMouseMoved = 0;
#endif
                        fDragging = TRUE;
                        fDrawing = false;

                        if (gParams->ctl[CTL_PREVIEW].properties & CTP_PREVIEWDRAG){
                            PerformControlAction(
                                CTL_PREVIEW,   //control index
                                FME_PREVIEWDRAG,        //FM event code
                                0,                      //previous value
                                wMsg,                   //message type
                                0,                      //notification code
                                FALSE,                  //fActionable
                                TRUE,0);                  //fProcessed
                        }
                        
                        gParams->flags |= AFH_DRAGGING_PROXY;
                        UpdateProxy(hDlg, FALSE);  // To display raw input.
                        return TRUE; //??
                    } 
                    else 
                    {
                        //send FME_RIGHTCLICKED_DOWN event for Proxy
                        //SetCursor(hcHand);
                        PerformControlAction(
                            CTL_PREVIEW,   //control index
                            FME_RIGHTCLICKED_DOWN,           //FM event code
                            0,                      //previous value
                            wMsg,                   //message type
                            0,                      //notification code
                            FALSE,                  //fActionable
                            TRUE,0);                  //fProcessed
                      
                    }

                } else {

                    //Right mouse button on dialog background
                    PerformControlAction(
                        CTL_BACKGROUND,   //control index
                        FME_CONTEXTMENU,           //FM event code
                        0,                      //previous value
                        wMsg,                   //message type
                        0,                      //notification code
                        FALSE,                  //fActionable
                        TRUE,0);                  //fProcessed

                }   
            }

            return FALSE;
            break;


        case WM_RBUTTONUP:

            //Added by Harald Heim, Dec 13, 2002
            {
                // relay this mouse message to the tooltip control...
                MSG msg; 

                msg.lParam = lParam; 
                msg.wParam = wParam; 
                msg.message = wMsg; 
                msg.hwnd = hDlg; 
                SendMessage(ghTT, TTM_RELAYEVENT, 0, (LPARAM) &msg); 
            }

            {
                int x = (SHORT) LOWORD(lParam);
                int y = (SHORT) HIWORD(lParam);

                //if (gParams->ctl[CTL_PREVIEW].inuse){
                    if (fDragging)
                    {
                        if (x >= gProxyRect.left && x < gProxyRect.right &&
                            y >= gProxyRect.top && y < gProxyRect.bottom)
                        {
                            SetCursor(hcHand);
                        }
                        else
                        {
                            //SetCursor(LoadCursor(NULL, IDC_ARROW));  //???
                        }
                        ReleaseCapture();
                        fDragging = FALSE;
                        gParams->flags &= ~AFH_DRAGGING_PROXY;
                        ptNew.x = x;
                        ptNew.y = y;
        #ifndef NOOPTIMZE_QUICK_TOGGLE
                        //Only update if image can be dragged in preview
                    #ifndef APP
                        if (gFmcp->x_start != 0 || gFmcp->y_start != 0 || gFmcp->x_end != gFmcp->X || gFmcp->y_end != gFmcp->Y){
                    #endif
                            fMouseMoved |= (ptNew.x - ptPrev.x) | (ptNew.y - ptPrev.y);
                    #ifndef APP
                        }
                    #endif
        #endif
                        //Only update if image can be dragged in preview
                    #ifndef APP
                        if (gFmcp->x_start != 0 || gFmcp->y_start != 0 || gFmcp->x_end != gFmcp->X || gFmcp->y_end != gFmcp->Y ){
                    #endif
                            imageOffset.x -= (ptNew.x - ptPrev.x) * scaleFactor * gFmcp->scrollFactor;// / gFmcp->enlargeFactor;
                            imageOffset.y -= (ptNew.y - ptPrev.y) * scaleFactor * gFmcp->scrollFactor;// / gFmcp->enlargeFactor;
                    #ifndef APP    
                        }
                    #endif
                        GetProxyItemRect (hDlg, &gProxyRect);


                        if ( ((gParams->flags & AFH_ZERO_OUTPUT_DRAG) == AFH_ZERO_OUTPUT_DRAG) && gFmcp->x_start == 0 && gFmcp->y_start == 0 && gFmcp->x_end == gFmcp->X && gFmcp->y_end == gFmcp->Y ){
                        
                             //Temporarily deactivate Zero drag to avoid redraw problem
                             gParams->flags &= ~AFH_ZERO_OUTPUT_DRAG;
                             SetupFilterRecordForProxy (globals, &scaleFactor, scaleFactor, &imageOffset);
                             //Activate Zero drag again
                             gParams->flags |= AFH_ZERO_OUTPUT_DRAG;

                        }else {
                            SetupFilterRecordForProxy (globals, &scaleFactor, scaleFactor, &imageOffset);
                        }


                        if (gResult == noErr) {
                            // Do filter and update proxy...
        #ifndef NOOPTIMZE_QUICK_TOGGLE
                            //if mouse didn't move since LBUTTONDOWN, no need to run filter...
                            //gives a quick before/after preview toggle

                            if (fMouseMoved) {
                            
                                //Setup x_start etc.
#if BIGDOCUMENT
								VRect scaledFilterRect;
								gFmcp->filterRect32 = GetFilterRect();	//needed??
								scaledFilterRect = gFmcp->filterRect32;
                                scaleRect32(&scaledFilterRect, 1, gFmcp->scaleFactor);
								gFmcp->inRect32 = GetInRect();
                                gFmcp->x_start = gFmcp->inRect32.left - scaledFilterRect.left;
                                gFmcp->y_start = gFmcp->inRect32.top - scaledFilterRect.top;
#else
								Rect scaledFilterRect;
                                copyRect (&scaledFilterRect, &gStuff->filterRect);
                                scaleRect (&scaledFilterRect, 1, gFmcp->scaleFactor);
                                gFmcp->x_start = gStuff->inRect.left - scaledFilterRect.left;
                                gFmcp->y_start = gStuff->inRect.top - scaledFilterRect.top;
#endif
                                gFmcp->x_end = gFmcp->x_start + gFmcp->columns;
                                gFmcp->y_end = gFmcp->y_start + gFmcp->rows;
                                
                                if (gParams->ctl[CTL_PREVIEW].properties & CTP_PREVIEWDRAG){
                                    PerformControlAction(
                                        CTL_PREVIEW,   //control index
                                        FME_PREVIEWDRAG,        //FM event code
                                        1,                      //previous value
                                        wMsg,                   //message type
                                        0,                      //notification code
                                        FALSE,                  //fActionable
                                        TRUE,0);                  //fProcessed
                                }
                          

                                DoFilterRect (globals);
                                fMouseMoved = 0;
                            }
        #else
                            DoFilterRect (globals);
        #endif
                            UpdateProxy(hDlg, FALSE);
                        }//noErr
                        return TRUE;
                    }



                    if (x >= gProxyRect.left && x < gProxyRect.right &&
                        y >= gProxyRect.top && y < gProxyRect.bottom)
                    {
                        //send FME_RIGHTCLICKED_UP event for Proxy
                        //SetCursor(hcHand);
                        PerformControlAction(
                            CTL_PREVIEW,   //control index
                            FME_RIGHTCLICKED_UP,           //FM event code
                            0,                      //previous value
                            wMsg,                   //message type
                            0,                      //notification code
                            FALSE,                  //fActionable
                            TRUE,1);                  //fProcessed

                    }

                //}//gParams->ctl[CTL_PREVIEW].inuse
            }

            return FALSE;
            break;
            //End of Added by Harald Heim, Dec 13, 2002



        case WM_MBUTTONDOWN:
        case WM_MBUTTONUP:
            {
                // relay this mouse message to the tooltip control...
                MSG msg; 

                msg.lParam = lParam; 
                msg.wParam = wParam; 
                msg.message = wMsg; 
                msg.hwnd = hDlg; 
                SendMessage(ghTT, TTM_RELAYEVENT, 0, (LPARAM) &msg); 
            }
            return FALSE;
            break;
#endif

#if 0
        // Doesn't work -- Dlg eats all key codes???
        case WM_KEYDOWN:          
			

            

			if (wParam == VK_F12) {
                // Toggle the editor.
                SendMessage(hDlg, WM_COMMAND, IDC_EDIT, 0);
                return 0;   //processed
            }
            
            

            return 1;   //not processed
            break;
#endif

        case WM_MOUSEWHEEL:
            {
                int zDelta = (short) HIWORD(wParam);
                int oldvalue;
                int i;
                HWND focusHWND = GetFocus();
                
                for (i = 0; i < N_CTLS; ++i) {

                    if (gParams->ctl[i].inuse && !(gParams->ctl[i].enabled != 3) ){
                    
                        if (gParams->ctl[i].hCtl == focusHWND) {

                            if (gParams->ctl[i].ctlclass == CC_STANDARD ||
                                gParams->ctl[i].ctlclass == CC_SCROLLBAR ||
                                gParams->ctl[i].ctlclass == CC_TRACKBAR ||
								gParams->ctl[i].ctlclass == CC_SLIDER){
                    
                                oldvalue = gParams->ctl[i].val;
        
                                if (zDelta<0)
                                    gParams->ctl[i].val += gParams->ctl[i].pageSize;
                                else
                                    gParams->ctl[i].val -= gParams->ctl[i].pageSize;

    
                                setCtlVal(i, gParams->ctl[i].val);

                                PerformControlAction(
                                                i,                   //control index
                                                FME_CHANGED,			//FM event code
                                                oldvalue,               //previous value
                                                wMsg,                   //message type
                                                0,                      //notification code
                                                FALSE,                  //fActionable
                                                TRUE,0);                  //fProcessed  
                            
                                //Perform the default action for this control.
                            
                                KillTimer(hDlg, 1000); //Cancel any previous trigger.
                                iCtl2 = i;

                                if (GetKeyState(VK_SHIFT) < 0){
                                    if (!SetTimer(hDlg, 1001, 20, NULL)) //Update max. every 20 ms
                                        doAction(gParams->ctl[i].action);
                                } else if (!SetTimer(hDlg, 1000, 500, NULL)) {
                                    doAction(gParams->ctl[i].action);
                                }
                                
                            }
                        } 
                   }

                }
            }
            return 0;   //processed


        case WM_HSCROLL:
        case WM_VSCROLL:

            //handles both scrollbar and trackbar messages
            //(Note that TB_PAGEDOWN == SB_PAGEDOWN, etc.)

            { //Start scope
                int oldvalue; 

                hCtrl = (HWND) lParam;
                iCtl = (int)GetWindowLong(hCtrl, GWL_ID);
                if (iCtl < IDC_CTLBASE || iCtl >= IDC_CTLBASE + N_CTLS)
                    return FALSE;
                iCtl -= IDC_CTLBASE;
                idd = COMMANDID (wParam);
    #if 0
                if (gParams->ctl[iCtl].ctlclass == CC_TRACKBAR) {
                    Info("TB notification %d", idd);
                }
    #endif

                oldvalue = gParams->ctl[iCtl].val;

                switch (idd)
                {
                case SB_PAGEDOWN:
                    gParams->ctl[iCtl].val += gParams->ctl[iCtl].pageSize;
                    break;
                case SB_LINEDOWN:
                    gParams->ctl[iCtl].val += gParams->ctl[iCtl].lineSize;
                    break;
                case SB_PAGEUP:
                    gParams->ctl[iCtl].val -= gParams->ctl[iCtl].pageSize;
                    break;
                case SB_LINEUP:
                    gParams->ctl[iCtl].val -= gParams->ctl[iCtl].lineSize;
                    break;
                case SB_TOP:
                    gParams->ctl[iCtl].val = gParams->ctl[iCtl].minval;
                    break;
                case SB_BOTTOM:
                    gParams->ctl[iCtl].val = gParams->ctl[iCtl].maxval;
                    break;
                case SB_THUMBTRACK:
                case SB_THUMBPOSITION:
                    
					if (gParams->ctl[iCtl].ctlclass == CC_STANDARD || gParams->ctl[iCtl].ctlclass == CC_SCROLLBAR){
						
						SCROLLINFO si;
						ZeroMemory(&si, sizeof(si));
						si.cbSize = sizeof(si);
						si.fMask = SIF_TRACKPOS;

						if (GetScrollInfo(hCtrl, SB_CTL, &si) )
							gParams->ctl[iCtl].val = si.nTrackPos; //get 32bit scroll bar value
						else
							gParams->ctl[iCtl].val = (short)HIWORD(wParam); //get 16bit scroll bar value
                        
					} else { //Trackbar and Slider

						gParams->ctl[iCtl].val = (int)SendMessage(hCtrl, TBM_GETPOS, 0, 0); //get 32bit track bar value
					}
					
                    if (gParams->ctl[iCtl].ctlclass == CC_STANDARD ||
						gParams->ctl[iCtl].ctlclass == CC_SLIDER ||
                        gParams->ctl[iCtl].ctlclass == CC_SCROLLBAR)
                    {
                        if (gParams->ctl[iCtl].minval > gParams->ctl[iCtl].maxval) {
                            //range is inverted...
                            gParams->ctl[iCtl].val = gParams->ctl[iCtl].minval +
                                                     gParams->ctl[iCtl].maxval -
                                                     gParams->ctl[iCtl].val;
                        }
                    } //if
                    break;
                default:
                    return FALSE;
                } /*switch*/
                
                if (oldvalue != gParams->ctl[iCtl].val)
                {
                    
					if ( (idd== SB_THUMBPOSITION || idd == SB_THUMBTRACK) && gParams->ctl[iCtl].thumbSize > 0){

						int range = abs(gParams->ctl[iCtl].maxval - gParams->ctl[iCtl].minval);
						gParams->ctl[iCtl].val = gParams->ctl[iCtl].val * range / (range - gParams->ctl[iCtl].thumbSize-1);

					}					
					
					if ( (idd== SB_THUMBPOSITION || idd == SB_THUMBTRACK) && gParams->ctl[iCtl].gamma != 100){
						gParams->ctl[iCtl].val = gammaCtlVal(iCtl,gParams->ctl[iCtl].val,true);
                    }

					setCtlVal(iCtl, gParams->ctl[iCtl].val);
                  
                
                    //Added by Harald Heim, May 22, 2003
                    PerformControlAction(
                                iCtl,                   //control index
                                FME_CHANGED,       		//FM event code
                                oldvalue,               //previous value
                                wMsg,                   //message type
                                0,                      //notification code
                                FALSE,                  //fActionable
                                TRUE,0);                  //fProcessed                                        
                    
                    
                    /*if (gParams->ctl[iCtl].ctlclass == CC_STANDARD || gParams->ctl[iCtl].ctlclass == CC_SLIDER){
                        if (getAsyncKeyState(VK_LBUTTON) >= 0){
                            SetFocus (gParams->ctl[iCtl].hBuddy1);
                            SendMessage(gParams->ctl[iCtl].hBuddy1, EM_SETSEL, 0, -1);
                        }
                    }*/


                } //if CC_STANDARD || CC_SCROLLBAR

            }//scope


            // Perform default action for THUMBTRACK events only if TRACK property is set
            // (or sense is reversed by SHIFT key).
            if ( idd != SB_THUMBTRACK ||
                 ( (((gParams->ctl[iCtl].properties & CTP_TRACK) != 0) ^(GetKeyState(VK_SHIFT) < 0)) /*&&  fmc.renderTime < 333*/ )
               )
            {
                KillTimer(hDlg, 1001); //Cancel any previous trigger.
                iCtl2 = iCtl;
                //Perform the default action for this control.
                if (!SetTimer(hDlg, 1001, 20, NULL)){ //Update max. every 20 ms
                    doAction(gParams->ctl[iCtl].action);
                }
            }
            return TRUE;

        case WM_COMMAND:
            idd = COMMANDID (wParam); //loword
            iCmd = COMMANDCMD(wParam, lParam); //hiword

			//Key shortcuts
			/*if (iCmd==1){
				//Info ("%d",idd);
				PerformControlAction(
					idd,  //control index
					FME_KEYDOWN,  //FM event code
					0,	//previous value
					wMsg,               //message type
					iCmd,               //notification code
					FALSE,
					TRUE,0);

			}*/

			if (lParam==0 && idd>2){ //Menu items - they have to start with an index of 3!!!

				PerformControlAction(
					idd,  //control index
					FME_MENUCLICKED,  //FM event code
					0,	//previous value
					wMsg,               //message type
					iCmd,               //notification code
					FALSE,
					TRUE,0);

			}else if (idd >= IDC_BUDDY2 && idd < IDC_BUDDY2 + N_CTLS) {
                
                FME fmeCode;
                BOOL fActionable;

                if (iCmd == BN_CLICKED) {
                    fmeCode = FME_CLICKED;
                    fActionable = false;
                } else if (iCmd == STN_DBLCLK) {
                    //fmeCode = FME_DBLCLK;
					fmeCode = FME_CLICKED;
                    
					//Set Default Value
                    if (gParams->ctl[idd - IDC_BUDDY2].defval != 0xffffffff){
                        int oldvalue = gParams->ctl[idd - IDC_BUDDY2].val;
						
						if (GetFocus() != gParams->ctl[idd - IDC_BUDDY2].hBuddy1) SetFocus(gParams->ctl[idd - IDC_BUDDY2].hCtl);
                        
						if (oldvalue != gParams->ctl[idd - IDC_BUDDY2].defval){
							setCtlVal(idd - IDC_BUDDY2, gParams->ctl[idd - IDC_BUDDY2].defval);
							PerformControlAction(
								idd - IDC_BUDDY2,  //control index
								FME_CHANGED,  //FM event code
								oldvalue,	//previous value
								wMsg,               //message type
								iCmd,               //notification code
								FALSE,
								TRUE,0);
							fActionable = true;
						} else
							fActionable = false;
                    }
                }

                if (iCmd == BN_CLICKED || iCmd == STN_DBLCLK) {
                    return PerformControlAction(
                        idd - IDC_BUDDY2,   //control index
                        fmeCode,            //FM event code
                        0,                  //previous value
                        wMsg,               //message type
                        0,                  //notification code
                        fActionable,         
                        TRUE,0);
                }
            }


            if (idd >= IDC_BUDDY1 && idd < IDC_BUDDY1 + N_CTLS) {
                static BOOL changed = FALSE;
				int n = idd - IDC_BUDDY1;
                
#if SKIN
				if (isSkinActive() && isSkinCtl(n)){
					if (iCmd == BN_CLICKED) {
						int iVal,oldvalue;
						switch (gParams->ctl[n].ctlclass) {
						case CC_TAB:
							if (!gParams->ctl[n].enabled) break;
							//Info ("%d",n);
							iVal = skinGetTab(n);
							oldvalue = gParams->ctl[n].val;
							if (iVal>=0 && fmc.pre_ctl[n]!=iVal ){
								gParams->ctl[n].val = fmc.pre_ctl[n] = iVal;
								skinDraw(n,1);
							} else if (fmc.pre_ctl[n]==iVal)
								if (getAsyncKeyState(VK_SHIFT)<=-32767) resetTabCtls(n);

							return PerformControlAction(
								n,  //control index
								FME_CLICKED,            //FM event code
								oldvalue,           //previous value
								wMsg,               //message type
								iCmd,               //notification code
								TRUE,
								TRUE,0);
							break;
						}
					}
				}
#endif				
				
				// NUMEDIT buddy control for a scrollbar
                if (iCmd == EN_CHANGE) {
                    changed = TRUE;
                    
                    if (EditBoxUpdate != 0){

                    //if (gParams->ctl[idd - IDC_BUDDY1].properties & CTP_MOUSEOVER) {
                    
                        BOOL fTranslated;
                        int iVal;
#if 1
                        //Changed by Harald Heim, May 1, 2003
                        if (gParams->ctl[idd - IDC_BUDDY1].divisor == 1) { // Set Integer Value

                            iVal = (int)GetDlgItemInt(hDlg, idd, &fTranslated, TRUE /*signed*/);

                        } else 
#endif
                        { // Set Double Value
                        
                            char  szInput[32];
                            double dVal;
                            char *pInvalid;

                            fTranslated = GetDlgItemText (hDlg, idd, szInput, 32);
                            // WRONG!  strtod() is locale-dependent!!!
                            dVal = strtod( szInput, &pInvalid);     //should check errno, *pInvalid==0 ???
#if 0
                            iVal = (int) (gParams->ctl[idd - IDC_BUDDY1].divisor * dVal);   //should this round???
#else
                            iVal = (int) (gParams->ctl[idd - IDC_BUDDY1].divisor * dVal + (dVal<0.0?-0.5:0.5));
#endif
                        }
                        
                        gParams->ctl[idd - IDC_BUDDY1].val = iVal;

                        /*PerformControlAction(
                                idd - IDC_BUDDY1,   //control index
                                FME_EDITCHANGED,        //FM event code
                                iVal,                      //previous value
                                wMsg,                   //message type
                                0,                      //notification code
                                FALSE,                  //fActionable
                                TRUE,0);                  //fProcessed
                        */

                        if (!fTranslated) {
                            //bad syntax ... force an illegal value so setCtlVal()      //or just set to 0???
                            //will fix it up...
                            iVal = -9845783;  //black magic
                            fTranslated = TRUE;
                        }
                        if (fTranslated) {
                        
                            //Added by Harald Heim, May 22, 2003
                            int oldvalue = gParams->ctl[idd - IDC_BUDDY1].val;

                            //set value of associated control
                            //(calling setCtlVal() may be dangerous overkill,
                            //possibly even resulting in an infinite loop???)
                            setCtlVal(idd - IDC_BUDDY1, iVal);


                        
                            //Added by Harald Heim, May 22, 2003
                            PerformControlAction(
                                idd - IDC_BUDDY1,       //control index
                                FME_CHANGED,//FME_VALUECHANGED,       //FM event code
                                oldvalue,               //previous value
                                wMsg,                   //message type
                                0,                      //notification code
                                FALSE,                  //fActionable
                                TRUE,0);                  //fProcessed                                        


                            if (!fDragging) {
                                if (EditBoxUpdate==1){
                                    //Update Preview immediately
                                    doAction(gParams->ctl[idd - IDC_BUDDY1].action);

                                } else {
                                    //Delayed update via timer
                                    iCtl2 = idd - IDC_BUDDY1;//Store control number for timer
                                    if (!SetTimer(hDlg, 1001, EditBoxUpdate, NULL)) //Update max. every 20 ms
                                        doAction(gParams->ctl[idd - IDC_BUDDY1].action);
                                }
                            }

                        }
                        changed = FALSE;

                    }

                }
                else if (iCmd == EN_SETFOCUS) {
                    changed = FALSE;
                    
                    if (gParams->ctl[idd - IDC_BUDDY1].properties & CTP_MOUSEOVER) {
                        
                        PerformControlAction(
                                idd - IDC_BUDDY1,   //control index
                                FME_SETFOCUS,			//FM event code
                                0,                      //previous value
                                wMsg,                   //message type
                                0,                      //notification code
                                FALSE,                  //fActionable
                                TRUE,0);                  //fProcessed
                    }

                }
                else if (iCmd == EN_KILLFOCUS ||
                         iCmd == EN_MAXTEXT) {
                    BOOL fTranslated;
                    int iVal;

                    if (iCmd == EN_MAXTEXT) {
                        //presumably because user hit RETURN
                        changed = TRUE;
                    }

                    if (iCmd == EN_KILLFOCUS && (gParams->ctl[idd - IDC_BUDDY1].properties & CTP_MOUSEOVER)) {
                        
                        PerformControlAction(
                                idd - IDC_BUDDY1,   //control index
                                FME_KILLFOCUS,          //FM event code
                                0,                      //previous value
                                wMsg,                   //message type
                                0,                      //notification code
                                FALSE,                  //fActionable
                                TRUE,0);                  //fProcessed
                    }

                    //update only if the NUMEDIT control actually changed!
                    //(e.g., keep track of EN_CHANGE notices).
                    //Otherwise, switching to another app when a NUMEDIT
                    //has the focus will cause an unwarranted filter run...
                    if (!changed) return TRUE; //???


#if 1
                    //Changed by Harald Heim, May 1, 2003
                    if (gParams->ctl[idd - IDC_BUDDY1].divisor == 1) { // Set Integer Value

                        iVal = (int)GetDlgItemInt(hDlg, idd, &fTranslated, TRUE /*signed*/);

                    } else 
#endif
                    { // Set Double Value
                        
                        char  szInput[32];
                        double dVal;
                        char *pInvalid;

                        fTranslated = GetDlgItemText (hDlg, idd, szInput, 32);
                        // WRONG!  strtod() is locale-dependent!!!
                        dVal = strtod( szInput, &pInvalid);     //should check errno, *pInvalid==0 ???
#if 0
                        iVal = (int) (gParams->ctl[idd - IDC_BUDDY1].divisor * dVal);   //should this round???
#else
                        iVal = (int) (gParams->ctl[idd - IDC_BUDDY1].divisor * dVal + (dVal<0.0?-0.5:0.5));
#endif
                    }

                    //Info("iVal = %d\nfTranslated = %d", iVal, fTranslated);

                    if (!fTranslated) {
                        //bad syntax ... force an illegal value so setCtlVal()      //or just set to 0???
                        //will fix it up...
                        iVal = -9845783;  //black magic
                        fTranslated = TRUE;
                    }
                    if (fTranslated) {
                        
                        //Added by Harald Heim, May 22, 2003
                        int oldvalue = gParams->ctl[idd - IDC_BUDDY1].val;

                        //set value of associated control
                        //(calling setCtlVal() may be dangerous overkill,
                        //possibly even resulting in an infinite loop???)
                        setCtlVal(idd - IDC_BUDDY1, iVal);
                        
                        //Added by Harald Heim, May 22, 2003
                        PerformControlAction(
                            idd - IDC_BUDDY1,       //control index
                            FME_CHANGED,       		//FM event code
                            oldvalue,               //previous value
                            wMsg,                   //message type
                            0,                      //notification code
                            FALSE,                  //fActionable
                            TRUE,0);                  //fProcessed                                        


                        if (!fDragging) {
                            if (gResult == noErr) {
                                // Do Filtering operation
                                DoFilterRect (globals);
                                // Invalidate Proxy Item
                                UpdateProxy(hDlg, FALSE);
                            }
                        }
                    }
                    changed = FALSE;
#if 0
                    return SetDlgMsgResult(hDlg, wMsg, TRUE);
#else
                    return TRUE; //so EN_MAXTEXT won't ding the bell -- ineffective???
#endif
                }
#if 0
                else {
                    //unexpected notification????
                    static int counter = 0;
                    static int wmsg_save = 0;
                    static int wparam_save = 0;
                    static int lparam_save = 0;
                    static int idd_save = 0;
                    static int cmd_save = 0;
                    wmsg_save = wMsg;
                    wparam_save = wParam;
                    lparam_save = lParam;
                    idd_save = idd;
                    cmd_save = iCmd;
                    counter++;
                    return TRUE;
                }
#endif
                return TRUE;   //???
            }

            if (idd >= IDC_CTLBASE && idd < IDC_CTLBASE + N_CTLS) {
                int iVal = 0;
				int n = idd - IDC_CTLBASE;
				int success = 0;

                //Added by Harald Heim
                int oldvalue=0;

                BOOL fProcessed = TRUE;
                BOOL fActionable = FALSE;
                FME fmeCode = FME_UNKNOWN;


#if SKIN
				//Skinned Controls
				if (isSkinActive() && isSkinCtl(n)){
					if (iCmd == BN_CLICKED) {
						fmeCode = FME_CLICKED;
						switch (gParams->ctl[n].ctlclass) {
						case CC_CHECKBOX:
							oldvalue = gParams->ctl[n].val;
							if (gParams->ctl[n].val==BST_UNCHECKED) iVal = BST_CHECKED; else iVal = BST_UNCHECKED;
							gParams->ctl[n].val = fmc.pre_ctl[n] = iVal;
							skinDraw(n,1);
							success = 1;
							break;
						case CC_PUSHBUTTON:
						case CC_COMBOBOX:
							oldvalue = gParams->ctl[n].val;
							skinDraw(n,1);
							success = 1;
							break;
						case CC_TOOLBAR:
							skinDraw(n,1);
							return TRUE;
						case CC_STANDARD:
							{
								oldvalue = gParams->ctl[n].val;
								if (!skinSetSlider(n)) return FALSE;
								
								PerformControlAction(
									n,                   //control index
									FME_CHANGED,       		//FM event code
									oldvalue,               //previous value
									wMsg,                   //message type
									0,                      //notification code
									FALSE,                  //fActionable
									TRUE,0);                  //fProcessed                                        
								
								//Update preview
								//if ( ((gParams->ctl[n].properties & CTP_TRACK) != 0) ^ (GetKeyState(VK_SHIFT) < 0) ){
									KillTimer(hDlg, 1001); //Cancel any previous trigger.
									iCtl2 = n;
									//Perform the default action for this control.
									if (!SetTimer(hDlg, 1001, 20, NULL)){ //Update max. every 20 ms
										doAction(gParams->ctl[n].action);
									}
								//}
								return TRUE; 
								break;
							}
						}//switch
					}
				}
#endif

				// It's a user control...
				if (success==0){
					switch (gParams->ctl[idd - IDC_CTLBASE].ctlclass) {
					case CC_CHECKBOX:
					case CC_RADIOBUTTON:
					case CC_PUSHBUTTON:
					case CC_OWNERDRAW:
					case CC_STATICTEXT:
					case CC_FRAME:
					case CC_RECT:
					case CC_IMAGE:
					case CC_BITMAP:
					case CC_ICON:
					case CC_METAFILE:
					case CC_GROUPBOX:
						// NOTE that BN_CLICKED == STN_CLICKED!!!
						if (iCmd == BN_CLICKED) {
							//From a button or static class control...
							static int reentry_depth = 0;
							static int reentry_count = 0;

							//check for reentry...
							if (reentry_depth++ > 0) {
								//reentered!!! Log the fact, and exit quickly...
								reentry_count++;
								reentry_depth--;
	#if 0
								return FALSE;   //do default processing anyway??
	#else
								return TRUE;    //no default processing either
	#endif
							}

							fmeCode = FME_CLICKED;

							switch (gParams->ctl[idd - IDC_CTLBASE].ctlclass) {
							case CC_CHECKBOX:
							case CC_RADIOBUTTON:
								#ifdef _WIN64
									iVal = (int)SendMessage(gParams->ctl[idd - IDC_CTLBASE].hCtl, BM_GETCHECK, 0, 0); //(HWND)(UINT_PTR)wParam
								#else
									iVal = SendMessage(COMMANDWND(wParam), BM_GETCHECK, 0, 0);
								#endif
								switch (gParams->ctl[idd - IDC_CTLBASE].style & BS_TYPEMASK) {
								case BS_CHECKBOX:
								case BS_AUTOCHECKBOX:
								case BS_3STATE:
								case BS_AUTO3STATE:
								case BS_AUTORADIOBUTTON:
                                //added by Ognen Genchev
                                case BS_OWNERDRAW:
                                // end
									//Added by Harald Heim, May 22, 2003
									oldvalue = gParams->ctl[idd - IDC_CTLBASE].val;
	                        
									setCtlVal(idd - IDC_CTLBASE, iVal);
									break;
								case BS_RADIOBUTTON:
									//setCtlVal causes reentry????
									//Need to update pre_ctl[] and ctl[].val????
									break;
								}//switch button subtype
								break;
							case CC_PUSHBUTTON:
							case CC_OWNERDRAW:
							case CC_STATICTEXT:
							case CC_FRAME:
							case CC_RECT:
							case CC_IMAGE:
							case CC_BITMAP:
							case CC_ICON:
							case CC_METAFILE:
							case CC_GROUPBOX:
								//these classes don't maintain their own value...
								//iVal = 1;
								//setCtlVal(idd - IDC_CTLBASE, iVal);
								break;
							default:
								break;
							}//switch class

							 reentry_depth--; //release reentry lock

						}//BN_CLICKED
						else {
							switch (gParams->ctl[idd - IDC_CTLBASE].ctlclass) {
							case CC_CHECKBOX:
							case CC_RADIOBUTTON:
							case CC_PUSHBUTTON:
							case CC_GROUPBOX:
							case CC_OWNERDRAW:
								//Button-style controls...
								if (iCmd == BN_DBLCLK) {
									fProcessed = FALSE;     //not processed
									fActionable = FALSE;    //for now
									fmeCode = FME_DBLCLK;
								}
								/*else if (iCmd == BN_PUSHED) {
									fProcessed = FALSE;     //not processed
									fActionable = FALSE;    //for now
									fmeCode = FME_LEFTCLICKED_DOWN;
								}
								else if (iCmd == BN_UNPUSHED) {
									fProcessed = FALSE;     //not processed
									fActionable = FALSE;    //for now
									fmeCode = FME_LEFTCLICKED_UP;
								}*/
								else {
									fProcessed = FALSE;     //not processed
									fActionable = FALSE;    //for now
									fmeCode = FME_UNKNOWN;
								}
								break;
							case CC_STATICTEXT:
							case CC_FRAME:
							case CC_RECT:
							case CC_IMAGE:
							case CC_BITMAP:
							case CC_ICON:
							case CC_METAFILE:
								//Static-style controls...
								if (iCmd == STN_DBLCLK) {
									fProcessed = FALSE;     //not processed
									fActionable = FALSE;    //for now
									fmeCode = FME_DBLCLK;
								}
								else {
									fProcessed = FALSE;     //not processed
									fActionable = FALSE;    //for now
									fmeCode = FME_UNKNOWN;
								}
								break;
							}//switch
						}//if !BN_CLICKED
						break;

					case CC_LISTBOX:
					case CC_LISTBAR:
						if (iCmd == LBN_SELCHANGE) {
	                        
							int n = idd - IDC_CTLBASE;

							//Added by Harald Heim, May 22, 2003
							oldvalue = gParams->ctl[idd - IDC_CTLBASE].val;
	                        
							#ifdef _WIN64
								iVal = (int)SendMessage(gParams->ctl[n].hCtl, LB_GETCURSEL, 0, 0);
							#else
								iVal = SendMessage(COMMANDWND(wParam), LB_GETCURSEL, 0, 0);
							#endif
							setCtlVal(n, iVal);
	                        
							if (gParams->ctl[n].ctlclass == CC_LISTBAR && gParams->ctl[n].mateControl != -1){
								char text[256];
								getCtlItemText(n,ctl(n),&text[0]);
								setCtlVal(gParams->ctl[n].mateControl,atoi(&text[0]));
							}

							//Makes listbar work like a tab control
							if (gParams->ctl[n].ctlclass == CC_LISTBAR){
								int i;
								for (i = 0; i < N_CTLS; ++i) {
									if (gParams->ctl[i].inuse && gParams->ctl[i].tabControl == n &&  gParams->ctl[i].tabSheet != -1){
										sub_enableCtl(i);
									}	
								}
							}
							
							fmeCode = FME_CLICKED;  //for now

						}
						else if (iCmd == LBN_DBLCLK) {
							//Requires LBS_NOTIFY
							fProcessed = FALSE;     //not processed
							fActionable = FALSE;    //for now
							fmeCode = FME_DBLCLK;
						}
						else {
							fProcessed = FALSE;   //not processed
						}
						break;

					case CC_COMBOBOX:
	#if defined(_DEBUG) && 0
						{   //debug combobox events
							char sz[256];
							static int iSeq = 0;
							iSeq++;
							sprintf(sz, "CB event %.4d: Ctl = %d, Cmd = %d (%s)\n",
								iSeq,
								idd - IDC_CTLBASE,
								iCmd,
								iCmd==CBN_ERRSPACE      ?"CBN_ERRSPACE":
								iCmd==CBN_SELCHANGE     ?"CBN_SELCHANGE":
								iCmd==CBN_DBLCLK        ?"CBN_DBLCLK":
								iCmd==CBN_SETFOCUS      ?"CBN_SETFOCUS":
								iCmd==CBN_KILLFOCUS     ?"CBN_KILLFOCUS":
								iCmd==CBN_EDITCHANGE    ?"CBN_EDITCHANGE":
								iCmd==CBN_EDITUPDATE    ?"CBN_EDITUPDATE":
								iCmd==CBN_DROPDOWN      ?"CBN_DROPDOWN":
								iCmd==CBN_CLOSEUP       ?"CBN_CLOSEUP":
								iCmd==CBN_SELENDOK      ?"CBN_SELENDOK":
								iCmd==CBN_SELENDCANCEL  ?"CBN_SELENDCANCEL":
								"???"
								);
							OutputDebugString(sz);
						}
	#endif
						if (iCmd == CBN_SELENDOK /*||
							iCmd == CBN_SELCHANGE*/) {
	                        
							oldvalue = gParams->ctl[idd - IDC_CTLBASE].val;
	                        
							#ifdef _WIN64
								iVal = (int)SendMessage(gParams->ctl[idd - IDC_CTLBASE].hCtl, CB_GETCURSEL, 0, 0);
							#else
								iVal = SendMessage(COMMANDWND(wParam), CB_GETCURSEL, 0, 0);
							#endif
							setCtlVal(idd - IDC_CTLBASE, iVal);
							fmeCode = FME_CLICKED;  //for now
						}
						else if (iCmd == CBN_DBLCLK) {
							//Can only occur for CBS_SIMPLE...
							fProcessed = FALSE;     //not processed
							fActionable = FALSE;    //for now
							fmeCode = FME_DBLCLK;
						}
						else if (iCmd == CBN_DROPDOWN) {
							fProcessed = FALSE;     //not processed
							fActionable = FALSE;    //for now
							fmeCode = FME_COMBO_DROPDOWN;
						}
						else if (iCmd == CBN_CLOSEUP) {
							fProcessed = FALSE;     //not processed
							fActionable = FALSE;    //for now
							fmeCode = FME_COMBO_CLOSEUP;
						}
						else {
							fProcessed = FALSE;   //not processed
						}
						break;

					case CC_EDIT:				
						
						if (iCmd == EN_CHANGE) {
							
							GetDlgItemText(hDlg, idd, gParams->ctl[idd - IDC_CTLBASE].label,1024);
							
							oldvalue = (int)strlen(gParams->ctl[idd - IDC_CTLBASE].label);
							fmeCode = FME_CHANGED;//FME_VALUECHANGED;
						
						} else if (iCmd == EN_SETFOCUS) {

							//fmeCode = FME_CLICKED;
							fmeCode = FME_SETFOCUS;//FME_SETEDITFOCUS;
							
						} else if (iCmd == EN_KILLFOCUS) {

							fmeCode = FME_KILLFOCUS;//FME_KILLEDITFOCUS;
						
						} else if (iCmd == EN_MAXTEXT){

							fmeCode = FME_CHANGED;//FME_VALUECHANGED;
						}

						fProcessed = FALSE;     //not processed
						fActionable = FALSE;    //for now

						break;

					default:
						fProcessed = FALSE;   //not processed
					}//switch class

				}//if (success==0)


#ifdef SCRIPTABLE
                //Set script values before CTL_OK event, so that the user can still modify them
                if (idd - IDC_CTLBASE == CTL_OK && fmeCode == FME_CLICKED){
                        int i;
                        for (i=0;i<N_SCRIPTITEMS;i++) {
                           ScriptParam[i] = gFmcp->pre_ctl[i]; //gParams->ctl[i].val
                           ScriptParamUsed[i] = gParams->ctl[i].inuse && gParams->ctl[i].scripted;
                        }
                }
#endif


                fActionable = fProcessed;   //for now
                return PerformControlAction(
                    idd - IDC_CTLBASE,  //control index
                    fmeCode,            //FM event code
                    oldvalue,           //previous value
                    wMsg,               //message type
                    iCmd,               //notification code
                    fActionable,
                    fProcessed,0);
            }//user control



            switch  (idd) {

#if OLDZOOMLABEL || TESTSHELL

              case IDC_BUTTON2: // -
				#ifdef HIGHZOOM 
					if (scaleFactor==1 && gFmcp->enlargeFactor>1)
						gFmcp->enlargeFactor -= 2;
					else
				#endif
						scaleFactor += 2;
                    // Fall through...
              case IDC_BUTTON1: // +
				#ifdef HIGHZOOM   
					if (scaleFactor==1 && gFmcp->enlargeFactor<16)
						gFmcp->enlargeFactor++;
					else
				#endif
						scaleFactor--;

                    if (GetKeyState(VK_SHIFT) < 0) {
                        //SHIFT modifies meaning of +/-
                        if (scaleFactor < prevScaleFactor) {
                            // Shift + means zoom 100 %
                            scaleFactor = 1;
							gFmcp->enlargeFactor = 1;
							
                        }
                        else {
							#ifdef HIGHZOOM 
							if (scaleFactor == 1 && gFmcp->enlargeFactor == 15){
								gFmcp->enlargeFactor = 1;
							} else if (scaleFactor == 1){
								gFmcp->enlargeFactor = 16;
							} else {
							#endif
								// Shift - means optimal zoom
								scaleFactor = -888; //magic # for SetupFilterRecordForProxy.
								gFmcp->enlargeFactor = 1;
							#ifdef HIGHZOOM 
							}
							#endif
                        }
                    }
                    else if (GetKeyState(VK_CONTROL) < 0) {
                        //CONTROL modifies meaning of +/-
                        if (scaleFactor < prevScaleFactor) {
                            // Ctrl + means zoom 100 %
                            scaleFactor = 1;
							gFmcp->enlargeFactor = 1;
                        }
                        else {
                            #ifdef HIGHZOOM 
							if (scaleFactor == 1 && gFmcp->enlargeFactor == 15){
								gFmcp->enlargeFactor = 1;
							} else if (scaleFactor == 1){
								gFmcp->enlargeFactor = 16;
							} else {
							#endif
								// Ctrl + means zoom 6 %
								scaleFactor = MAX_SCALE_FACTOR;
								gFmcp->enlargeFactor = 1;
							#ifdef HIGHZOOM
							}
							#endif
                        }
                    }
                    else if (scaleFactor == 13 || scaleFactor == 15)
                    {
                        // Skip scalefactor 13 or 15...
                        if (scaleFactor > prevScaleFactor)
                            scaleFactor++;  //zooming out
                        else
                            scaleFactor--;  //zooming in
                    }
                    GetProxyItemRect (hDlg, &gProxyRect); 
                    //Should really do this in WM_TIMER, once final scale factor is set???
                    SetupFilterRecordForProxy (globals, &scaleFactor, prevScaleFactor, &imageOffset);
					#ifdef HIGHZOOM
						EnableWindow(GetDlgItem(hDlg, IDC_BUTTON1), scaleFactor > 1 || gFmcp->enlargeFactor < 16 );
					#else
						EnableWindow(GetDlgItem(hDlg, IDC_BUTTON1), scaleFactor > 1);
					#endif
                    EnableWindow(GetDlgItem(hDlg, IDC_BUTTON2), scaleFactor < MAX_SCALE_FACTOR);
                    // Should switch input focus to other button when a button
                    // gets disabled ... otherwise, can't TAB way from disabled
                    // button using keyboard only.
                    #ifdef HIGHZOOM
						if (scaleFactor == 1 && gFmcp->enlargeFactor==16) SetFocus(GetDlgItem(hDlg, IDC_BUTTON2));
					#else
						if (scaleFactor == 1) SetFocus(GetDlgItem(hDlg, IDC_BUTTON2));
					#endif
                    if (scaleFactor == MAX_SCALE_FACTOR) SetFocus(GetDlgItem(hDlg, IDC_BUTTON1));
                              
                    #if OLDZOOMLABEL || TESTSHELL
                    {
                        //Update old zoom label
                        char buff[20];
                        wsprintf(buff, "%d%%", 100*gFmcp->enlargeFactor/(gStuff->inputRate>>16));
                        SetDlgItemText(MyHDlg, 498, buff);
                    }
                    #endif

                    if (gResult == noErr) {
                        // Set a timer to trigger a delayed proxy update,
                        // giving the user a chance to click several times
                        // on the zoom button first...
                        KillTimer(hDlg, 1); //Cancel any previous trigger.
                        if (!SetTimer(hDlg, 1, 500, NULL)) {
                            // If SetTimer() fails, update the proxy immediately.
                            if (!fDragging) {
                                // Do Filtering operation
                                DoFilterRect (globals);
                                // Invalidate Proxy Item (and repaint background if scale factor
                                // increased; i.e., if proxy image may have shrunk)...
                                // (To eliminate MORE flashing, only repaint BG if shrunk, and
                                // view width or height is less than Proxy width or height.)
                                UpdateProxy(hDlg, scaleFactor > prevScaleFactor);
                            }
                        }

                    }//noErr
                
                    break;

#endif //#if OLDZOOMLABEL || TESTSHELL

              case  IDOK:
                    // Added by Ognen Genchev. Fixes not remembering last values after hitting enter key.
                    for (iCtl = 0;  iCtl < N_CTLS;  iCtl++) {
                        gParams->ctl[iCtl].initVal = gParams->ctl[iCtl].val;
                        gParams->ctl[iCtl].cancelVal = gParams->ctl[iCtl].val;
                        ScriptParam[iCtl] = gFmcp->pre_ctl[iCtl];
                        ScriptParamUsed[iCtl] = gParams->ctl[iCtl].inuse && gParams->ctl[iCtl].scripted;
                        // end
                    }
// Added by Ognen Genchev. OKdone jump has the role to skip part of the IDCANCEL code in order to prevent
// the applying operation to conflict with the canceling operation. By adding this jump, now OK button and
// Enter key have individual function from ESC key, titlebar button x and cancel button.
goto OKdone;
              case  IDCANCEL:
				  	//Trigger cancel event on ESC key or x titlebar button, but not on OK or Cancel button
					if (idd == IDCANCEL && cancelEventPerformed==0){
                        // Added by Ognen Genchev. Loop through controls to remember last values after canceling operation is performed.
                        for (iCtl = 0;  iCtl < N_CTLS;  iCtl++) {
                            gParams->ctl[iCtl].initVal = gParams->ctl[iCtl].cancelVal;
                        }
                        // end
						triggerEvent (CTL_CANCEL,FME_CLICKED,0);
						//cancelEventPerformed=1; //Do not set it to 1
					} 
/*                    else if (idd == IDOK && okEventPerformed==0){ //Trigger OK on Enter key, but not on OK or Cancel button
                        for (iCtl = 0;  iCtl < N_CTLS;  iCtl++) {
                            gParams->ctl[iCtl].initVal = gParams->ctl[iCtl].val;
                            gParams->ctl[iCtl].cancelVal = gParams->ctl[iCtl].val;
                            ScriptParam[iCtl] = gFmcp->pre_ctl[iCtl];
                            ScriptParamUsed[iCtl] = gParams->ctl[iCtl].inuse && gParams->ctl[iCtl].scripted;
                            // end
                        }
						triggerEvent (CTL_OK,FME_CLICKED,0);
					}*/
OKdone:
                    if (fDragging)
                    {
                        ReleaseCapture();
                        //SetCursor(LoadCursor(NULL, IDC_ARROW));  //???
                        fDragging = FALSE;
                        gParams->flags &= ~AFH_DRAGGING_PROXY;
                    }

                    // Preserve values in FM context record across invocations...
                    gParams->need_iuv = gFmcp->need_iuv;
                    gParams->need_d = gFmcp->need_d;
                    gParams->need_m = gFmcp->need_m;
                    gParams->need_tbuf = gFmcp->need_tbuf;
                    gParams->need_t2buf = gFmcp->need_t2buf;
					gParams->need_t3buf = gFmcp->need_t3buf;
					gParams->need_t4buf = gFmcp->need_t4buf;
                    gParams->needPremiere = gFmcp->needPremiere;
                    gParams->needPadding = gFmcp->needPadding;
                    gParams->tileHeight = gFmcp->tileHeight;//gParams->bandWidth = gFmcp->bandWidth;
                    gParams->tileWidth = gFmcp->tileWidth;
#ifndef DONT_SAVE_GLOBALS
                    /* Save the following predeclared global variables */
                    //// NO!!! Should be done AFTER the filter has run on the host image!!!!!
                    gParams->globvars = gFmcp->globvars;
#endif //DONT_SAVE_GLOBALS
                    // !!!Too bad C doesn't allow direct array assignments, e.g.:
                    ///// gParams->filterAuthorText = gFmcp->filterAuthorText;
                    memcpy(gParams->filterAuthorText, gFmcp->filterAuthorText, sizeof(gParams->filterAuthorText));
                    memcpy(gParams->filterCategoryText, gFmcp->filterCategoryText, sizeof(gParams->filterCategoryText));
                    memcpy(gParams->filterTitleText, gFmcp->filterTitleText, sizeof(gParams->filterTitleText));
                    memcpy(gParams->filterFilenameText, gFmcp->filterFilenameText, sizeof(gParams->filterFilenameText));
                    memcpy(gParams->filterCopyrightText, gFmcp->filterCopyrightText, sizeof(gParams->filterCopyrightText));
                    memcpy(gParams->filterDescriptionText, gFmcp->filterDescriptionText, sizeof(gParams->filterDescriptionText));
                    memcpy(gParams->filterOrganizationText, gFmcp->filterOrganizationText, sizeof(gParams->filterOrganizationText));
                    memcpy(gParams->filterURLText, gFmcp->filterURLText, sizeof(gParams->filterURLText));
                    memcpy(gParams->filterVersionText, gFmcp->filterVersionText, sizeof(gParams->filterVersionText));
                    memcpy(gParams->filterUniqueID, gFmcp->filterUniqueID, sizeof(gParams->filterUniqueID));

                    //save current position and size of dialog
                    GetWindowRect(hDlg, &gParams->gDialogRect);
                    
                    if (gParams->gDialogState == 0){
                        //save current position and size of dialog
                        GetWindowRect(hDlg, &gParams->gDialogRectMin);
                    }

                    //save current dialog styles
                    gParams->gDialogStyle = (int)GetWindowLong(hDlg, GWL_STYLE);
                    gParams->gDialogStyleEx = (int)GetWindowLong(hDlg, GWL_EXSTYLE);
                    gParams->gInitialInvocation = FALSE;

                    //save current dialog clipping region, if any
                    // create a dummy region first...
                    gParams->gDialogRegion = CreateRectRgn(0, 0, 0, 0);
                    if (GetWindowRgn(hDlg, gParams->gDialogRegion) == ERROR) {
                        //no clipping region
                        DeleteObject(gParams->gDialogRegion);
                        gParams->gDialogRegion = NULL;
                    }


                    gFmcp->hDlg = NULL;    //So msgBox et al. will use global handle.
                    MyHDlg = NULL;          //ditto
					
                    #if CREATEDIALOGPARAM
						//if CreateDialogParam was used to create the window
						if (idd == IDCANCEL) gResult = 1;
						PostQuitMessage(0);
						#if STANDALONE
							EndDialog(hDlg, idd); //Avoids problem with File > Exit
						#endif
					#else
						//if DialogBoxParam was used to create the window
						EndDialog(hDlg, idd);
					#endif

					fDialogDisplayed = 0; //Reset in case the plugin is not removed from memory by host

                    break;

              default:
                    return FALSE;   // not handled
            }
            break;

        default:
            return  FALSE;  // not handled
    }

    return  TRUE;   // handled; do not call defProc
} /*FilterDialogProc*/

/*****************************************************************************/


void setScaleFactor(int n){
    
    scaleFactor = n;

}



//function added by Harald Heim on Apr 2, 2002
int sub_setZoom (HWND hDlg, int n, int m)
{
    
    //static GPtr globals = NULL;         /* needs to be static */    // EH????
    long   prevScaleFactor = scaleFactor;
	static int prevEnlargeFactor;
	int retval = false;
    
    scaleFactor = n;
	gFmcp->enlargeFactor = m;

    if (scaleFactor != prevScaleFactor || gFmcp->enlargeFactor != prevEnlargeFactor){ // 
    
        GetProxyItemRect (hDlg, &gProxyRect); 
        SetupFilterRecordForProxy (globals, &scaleFactor, prevScaleFactor, &imageOffset);
		#ifdef HIGHZOOM
			EnableWindow(GetDlgItem(hDlg, IDC_BUTTON1), scaleFactor > 1 || gFmcp->enlargeFactor < 16 );
		#else
			EnableWindow(GetDlgItem(hDlg, IDC_BUTTON1), scaleFactor > 1);
		#endif
        EnableWindow(GetDlgItem(hDlg, IDC_BUTTON2), scaleFactor < 16);
        #ifdef HIGHZOOM
			if (scaleFactor == 1 && gFmcp->enlargeFactor==16) SetFocus(GetDlgItem(hDlg, IDC_BUTTON2));
		#else
			if (scaleFactor == 1) SetFocus(GetDlgItem(hDlg, IDC_BUTTON2));
		#endif
        if (scaleFactor == MAX_SCALE_FACTOR) SetFocus(GetDlgItem(hDlg, IDC_BUTTON1));

        #if OLDZOOMLABEL || TESTSHELL
        {   //Update old zoom label
            char buff[20];
            wsprintf(buff, "%d%%", 100*gFmcp->enlargeFactor/(gStuff->inputRate>>16));
            SetDlgItemText(MyHDlg, 498, buff);
        }
        #endif

        if (!fDragging) {
            DoFilterRect (MyGlobals);
            UpdateProxy(hDlg, scaleFactor > prevScaleFactor || gFmcp->enlargeFactor != prevEnlargeFactor);
        }
        
		retval = true;
    }

	prevEnlargeFactor = gFmcp->enlargeFactor;

    return retval;
}



//function added by Harald Heim on Dec 14, 2002
int sub_scrollPreview (HWND hDlg, int mode, int offsetX, int offsetY, int update)
{

    //GPtr globals = NULL;        // EH???
    POINT tempOffset;

    //gParams->flags |= AFH_DRAGGING_PROXY;
    //UpdateProxy(hDlg, FALSE);  // To display raw input.


    if (mode==0){
        imageOffset.x += offsetX * scaleFactor;
        imageOffset.y += offsetY * scaleFactor;
    } else if (mode==1){
        imageOffset.x = offsetX * scaleFactor;
        imageOffset.y = offsetY * scaleFactor;
    }

    GetProxyItemRect (hDlg, &gProxyRect);
    tempOffset = imageOffset;
	
    SetupFilterRecordForProxy (globals, &scaleFactor, scaleFactor, &tempOffset ); //&tempOffset
    /*if ( ((gParams->flags & AFH_ZERO_OUTPUT_DRAG) == AFH_ZERO_OUTPUT_DRAG) && gFmcp->x_start == 0 && gFmcp->y_start == 0 && gFmcp->x_end == gFmcp->X && gFmcp->y_end == gFmcp->Y ){
        
         //Temporarily deactivate Zero drag to avoid redraw problem
         gParams->flags &= ~AFH_ZERO_OUTPUT_DRAG;
         SetupFilterRecordForProxy (globals, &scaleFactor, scaleFactor, &tempOffset);
         //Activate Zero drag again
         gParams->flags |= AFH_ZERO_OUTPUT_DRAG;

    }else {
        SetupFilterRecordForProxy (globals, &scaleFactor, scaleFactor, &tempOffset);
    }*/


    //GetProxyItemRect (hDlg, &gProxyRect);
    //gParams->flags &= ~AFH_DRAGGING_PROXY;
    
    if (update && gResult == noErr) {
        // Run the filter only if want to drag the filtered output
        // rather than the raw input...
        if (gParams->flags & AFH_DRAG_FILTERED_OUTPUT)
        {
            DoFilterRect (globals);
        }
        UpdateProxy(hDlg, FALSE);
    }//noErr
    

    return 1;
}



//function added by Harald Heim on January, 2003
int sub_setPreviewCursor (INT_PTR res)
{
    HCURSOR hcHand_old = hcHand;

	// How about loading a named resource????

	//HCURSOR
	if (res >= 32700){
		//Info ("cursor set: %d", res);
		hcHand = (HCURSOR)(INT_PTR)res;
    //Standard Windows Cursors
	} else if (res >= 32512)
        hcHand = LoadCursor(NULL,  MAKEINTRESOURCE(res));
    //Load from Resource
    else
        hcHand = LoadCursor((HINSTANCE)hDllInstance, MAKEINTRESOURCE(res));

    if (hcHand != NULL){
            DestroyCursor(hcHand_old);
            cursorResource = res;
			SetCursor(hcHand);
            return true;
    }

    return false;
}

INT_PTR sub_getPreviewCursor (void)
{

	return cursorResource;

}


/*****
Similarly, by default the TAB key moves the input focus to the next control in the tabbing sequence. 
However, it might be useful for the user to be able to enter TAB characters in an edit control. By 
default, multiple-line edit controls accept CTRL+TAB to enter TAB characters in an edit control.
The following sections describe four methods that can be used to implement alternative behavior for 
the ENTER and TAB keys. It should be noted that some users may object if the ENTER and TAB keys no 
longer act in the default fashion.

Use ES_WANTRETURN in Windows version 3.1

Windows version 3.1 supplies a new edit control style, ES_WANTRETURN, which causes an ENTER key to 
advance to a new line and not be passed to the dialog manager. ES_WANTRETURN does not affect TAB keys.

Subclass the edit control

An edit control subclass procedure could respond to the WM_GETDLGCODE message with DLGC_WANTALLKEYS 
to receive all keyboard input. This causes the edit control to be sent all key input, including ENTER 
and TAB keys. The following subclass procedure could be used:
  
LONG FAR PASCAL AllKeyFunc(HWND hWnd, WORD Message, WORD wParam,
   LONG lParam)
{
   //
   // This subclass function makes pressing an ENTER key
   // create a new line when the focus is in an edit control.
   //

   if ( Message == WM_GETDLGCODE )
      return DLGC_WANTALLKEYS;

   return CallWindowProc(lpfnOldClassProc, hWnd, Message, wParam,
      lParam);
}

  ****/

/****
FOLLOWING SHOWS HOW TO SUBCLASS:

Here is how to make an edit control read-only in Windows version 3.0:
  
// Declare the subclass function.
long FAR PASCAL SubEditWndProc(HWND hWnd, unsigned wMessage,
                WORD wParam, LONG lParam);

//
// Create an edit control.
//
hWndEdit=CreateWindow("EDIT",
                  "Edit Control Line Numbers",
                  WS_OVERLAPPEDWINDOW,
                  35, 35, 400, 150,
                  NULL, NULL, hInstance, NULL);
//
// Subclass the edit control.
//
static FARPROC     lpSubClassProc;

lpSubClassProc=MakeProcInstance((FARPROC)SubEditWndProc,hInstance);
     SetWindowLong(hWndEdit, GWL_WNDPROC, (LONG)lpSubClassProc);
.
.
.

//
// Read-only subclass function for an edit control.
//
long FAR PASCAL SubEditWndProc(HWND hWnd, unsigned wMessage, WORD
   wParam, LONG lParam)
{
   switch ( wMessage ) {
      case WM_CUT:
      case WM_KEYDOWN:
      case WM_PASTE:
      case WM_CHAR:
         // Do nothing here.
      break;

      default:
         CallWindowProc (lpPrevWndFunc, hWnd, wMessage, wParam,
                         lParam );
   }
}

****/

/*****************************************************************************/
void PromptUserForInput (GPtr globals)
{

#if APP
	HWND hParent = NULL;
#else
	PlatformData *platform = (PlatformData *)((FilterRecordPtr) gStuff)->platformData;
	HWND hParent = (HWND)platform->hwnd;
#endif

    /* Query the user for parameters. */

#if CREATEDIALOGPARAM
	
	HWND hDlg;
	MSG msg;
	int retval;

	hDlg = CreateDialogParam(
					   (HINSTANCE)hDllInstance,
					   (LPSTR)"FILTERPARAM",
					   hParent,
					   (DLGPROC)FilterDialogProc,
					   (LPARAM)globals);


	//ShowWindow(hDlg, SW_HIDE);
	ShowWindow(hDlg, SW_SHOW);

	//Message Loop - process keyboard input
	while (GetMessage(&msg, NULL, 0, 0) 
#ifndef APP
		&& (fmc.hostSig!='PSPI' || (fmc.hostSig=='PSPI' && fmc.hDlg))
#endif
	){ //&& fmc.hDlg = PSP 6 Fix
		//if (hAcceleratorTable) if (TranslateAccelerator(hDlg, hAcceleratorTable, &msg)) { Info("translated"); continue;}
		if (msg.message == WM_KEYDOWN || msg.message == WM_SYSKEYDOWN) {
			//if (msg.wParam != VK_SHIFT && msg.wParam != VK_CONTROL){ 
				
#if SKIN
				if (!isSkinActive())
#endif				
				{
					if ( (msg.wParam == '0' || msg.wParam == VK_NUMPAD0) && getAsyncKeyStateF(VK_CONTROL)>=0){
						int iCtl = getCtlFocus();
						if (iCtl!=-1){
							if (gParams->ctl[iCtl].ctlclass == CC_STANDARD){
								int oldvalue = gParams->ctl[iCtl].val;
								setCtlVal(iCtl,0);
								triggerEvent(iCtl,FME_CHANGED,oldvalue);
								doAction(gParams->ctl[iCtl].action);
								continue; //No further processing
							}
						}
					}

				} 
#if SKIN
				else { //active skin

					if (msg.wParam == VK_LEFT || msg.wParam == VK_RIGHT || msg.wParam == VK_UP || msg.wParam == VK_DOWN || 
						msg.wParam == VK_NEXT|| msg.wParam == VK_PRIOR || 
						//msg.wParam == '0' || msg.wParam == VK_NUMPAD0 || 
						((msg.wParam == '0' || msg.wParam == VK_NUMPAD0) && getAsyncKeyStateF(VK_CONTROL)>=0) ||
						msg.wParam == VK_HOME|| msg.wParam == VK_END
					){ 
						//POINT pt;
						//HWND hCtl;
						//Info ("%d",getCtlFocus());
						int iCtl = getCtlFocus();
						//GetCursorPos(&pt);
						//hCtl = WindowFromPoint(pt); //int n = getCtlFocus(); //hCtl = ChildWindowFromPointEx(hDlg,pt,CWP_SKIPDISABLED|CWP_SKIPINVISIBLE|CWP_SKIPTRANSPARENT);
						//if (hCtl!=NULL){
							//int i,iCtl=-1;
							//for (i=0;i<N_CTLS;i++) if (hCtl == gParams->ctl[i].hCtl) {iCtl=i; break;}
							if (iCtl!=-1){
								if (gParams->ctl[iCtl].ctlclass == CC_STANDARD){
									int oldvalue = gParams->ctl[iCtl].val;
									if (msg.wParam==VK_LEFT||msg.wParam==VK_UP) setCtlVal(iCtl,ctl(iCtl)-gParams->ctl[iCtl].lineSize);
									else if (msg.wParam==VK_RIGHT||msg.wParam==VK_DOWN) setCtlVal(iCtl,ctl(iCtl)+gParams->ctl[iCtl].lineSize);
									else if (msg.wParam==VK_NEXT) setCtlVal(iCtl,ctl(iCtl)+gParams->ctl[iCtl].pageSize);
									else if (msg.wParam==VK_PRIOR) setCtlVal(iCtl,ctl(iCtl)-gParams->ctl[iCtl].pageSize);
									else if (msg.wParam==VK_HOME) setCtlVal(iCtl,gParams->ctl[iCtl].minval );
									else if (msg.wParam==VK_END) setCtlVal(iCtl,gParams->ctl[iCtl].maxval);
									else if (msg.wParam==0x30 || msg.wParam==VK_NUMPAD0) setCtlVal(iCtl,0); //VK_0
									triggerEvent(iCtl,FME_CHANGED,oldvalue);
									doAction(gParams->ctl[iCtl].action);
									continue; //No further processing
								} else if (gParams->ctl[iCtl].ctlclass == CC_COMBOBOX){
									int oldvalue = gParams->ctl[iCtl].val;
									if (msg.wParam==VK_UP) setCtlVal(iCtl,max(0,ctl(iCtl)-1));
									else if (msg.wParam==VK_DOWN) setCtlVal(iCtl,min(getCtlItemString(iCtl,-1,NULL)-1,ctl(iCtl)+1));
									else if (msg.wParam==VK_HOME) setCtlVal(iCtl,0 );
									else if (msg.wParam==VK_END) setCtlVal(iCtl,getCtlItemString(iCtl,-1,NULL)-1);
									triggerEvent(iCtl,FME_CLICKED,oldvalue);
									doAction(gParams->ctl[iCtl].action);
									continue; //No further processing
								}
							}
						//}
					}
				} 
#endif

				//Do FME_KEYDOWN event
				retval = PerformControlAction(
						(int)msg.wParam,  //key code //control index
						FME_KEYDOWN,  //FM event code
						(int)msg.lParam,	//Other info //previous value
						msg.message,        //message type
						1,               //notification code
						FALSE, //no action
						FALSE, //return false if key was not processed by plugin
						0);

				if (retval) continue; //No further processing
			//}
		
		} else if (msg.message == WM_KEYUP || msg.message == WM_SYSKEYUP) {
		
			if (msg.wParam == VK_SHIFT || msg.wParam == VK_CONTROL){ 

				//Do FME_KEYDOWN event
				retval = PerformControlAction(
						(int)msg.wParam,  //key code //control index
						FME_KEYUP,  //FM event code
						(int)msg.lParam,	//Other info //previous value
						msg.message,        //message type
						1,               //notification code
						FALSE, //no action
						FALSE, //return false if key was not processed by plugin
						0);

				if (retval) continue; //No further processing

			}
		
		}

		if(!IsDialogMessage(hDlg, &msg)){
			TranslateMessage(&msg);
			DispatchMessage(&msg);
		}

	}


	if (hDlg) {
		DestroyWindow(hDlg);
		hDlg = 0;
	}
	
#else
	
	INT_PTR nResult;
	nResult = DialogBoxParam(
		               hDllInstance,
                       (LPSTR)"FILTERPARAM",
                       hParent, //platform,//(HWND)platform->hwnd,
                       (DLGPROC)FilterDialogProc,
                       (LPARAM)globals);

    if  (nResult == IDCANCEL || nResult == -1)
        gResult = 1; // don't continue

#endif

}   

/*
void PromptUserForInputNoProxy (GPtr globals)
{

    int    nResult;
    PlatformData *platform;

    platform = ((FilterRecordPtr) gStuff)->platformData;

    // Query the user for parameters
    nResult = DialogBoxParam(
		               hDllInstance,
                       (LPSTR)"FILTERPARAM",
                       (HWND)platform->hwnd,
                       (DLGPROC)FilterDialogProc,
                       (LPARAM)globals);
}   
*/

/****************************************************************************/
/* Example for ShowAlert() function which takes a string ID as parameter    */
/* and displays a message box                                               */
/****************************************************************************/

short ShowAlert (short stringID)
{
    char szMessage[256];
    char szTitle[128];

    LoadString((HINSTANCE)hDllInstance, stringID, szMessage, sizeof szMessage);
    LoadString((HINSTANCE)hDllInstance, 2, szTitle, sizeof szTitle);
    return  MessageBox(MyHDlg, szMessage, szTitle, MB_OK | MB_ICONHAND | MB_TASKMODAL);

}

/*****************************************************************************/

#ifndef NOTRACE
void Trace (GPtr globals, const char *szFormat, ...)
{
    char szBuffer[1024];
    char *pArgs;
    int nResult=0; //Added to fix compiler error
//#if 0
    if ((gParams->flags & (AFH_TRACE|AFH_DRAGGING_PROXY)) == AFH_TRACE)
//#endif
    {   // Trace if Trace flag set but not while dragging proxy...

    if  (nResult == IDCANCEL || nResult == -1)
        gResult = 1; // don't continue

        pArgs = (char *) &szFormat + sizeof(szFormat);
        wvsprintf(szBuffer, szFormat, pArgs);
        MessageBox(MyHDlg, szBuffer, "Filter trace", MB_OK | MB_TASKMODAL);
    }
} /*Trace*/
#endif



#ifndef APP

/*****************************************************************************/

/* Initialization and termination code for window's dlls. */
// Every 32-Bit DLL has an entry point DLLInit (It is actually called DllMain()...)

#ifndef __cplusplus

#ifdef _MT 
    //Needed for static linking of the C Runtime
    BOOL WINAPI _CRT_INIT(HANDLE, DWORD, LPVOID);
#endif

BOOL APIENTRY DLLInit(HANDLE hInstance, DWORD fdwReason, LPVOID lpReserved)
{

    if (fdwReason == DLL_PROCESS_ATTACH) 
        hDllInstance = hInstance;
    
#ifdef _MT    
    //Needed for static linking of the C Runtime
    if (fdwReason == DLL_PROCESS_ATTACH || fdwReason == DLL_THREAD_ATTACH || fdwReason == DLL_PROCESS_DETACH || fdwReason == DLL_THREAD_DETACH){
        if (!_CRT_INIT(hInstance, fdwReason, lpReserved)) return(FALSE);
    }
#endif

    return TRUE;   // Indicate that the DLL was initialized successfully.
}

#endif

/*****************************************************************************/

#endif
