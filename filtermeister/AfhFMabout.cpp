///////////////////////////////////////////////////////////////////////////////////
//  File: AfhFMabout.c
//
//  Source file for the FilterMeisterVS "About" dialog box.
//
//  Copyright (c) 1997-2017 by Alex Hunter, Harald Heim & Martijn Van der Lee
//	   
//  FilterMeisterVS is free software. You can redistribute and modify it
//  under the terms of the MPL2 license. See https://www.mozilla.org/en-US/MPL/2.0/
//  Any changes to the FilterMeisterVS files in the /components/filtermeister/
//  sub folder have to be published or sent to harry@thepluginsite.com. 
//  The individual filter code itself does not need to be disclosed.
//  
///////////////////////////////////////////////////////////////////////////////////

#include "PIAbout.h"

#include "AfhFM.h"
#include "AfhFMcontext.h"
#include "resource.h"
#include "WinUtil.h"    // Message crackers and stuff

#include "FW_FFP.h"


/*****************************************************************************/

INT_PTR WINAPI AboutDialogProc(HWND hDlg, UINT wMsg, WPARAM wParam, LPARAM lParam)
{
    static HBITMAP hBmpDialog;  // Handle of bitmap to display as dialog background.
    static HBITMAP hBmpOld;     // Handle of old bitmap to restore.
    static BITMAP  bmInfo;      // Bitmap information structure.
    static HDC     hdcMem;      // Compatible Memory DC for dialog.
    static POINT   pt;          // Position for upper left corner of bitmap.
    static SIZE    size;        // Size (width and height) of dialog client rectangle.
    static int     AboutImageMode = DIM_STRETCHED;  //for now
    static int     AboutImageStretchMode = COLORONCOLOR; //for now
    int idd;
    int iCmd;

    switch  (wMsg) {

        case  WM_INITDIALOG:
            {//scope
                // center the dialog on screen (if no parent);
                // else center in client area of parent.
                UINT flags = SWP_NOZORDER | SWP_NOACTIVATE | SWP_NOSIZE;
                HWND hParent;
                int  nHeight;
                int  nWidth;
                RECT rcDialog;
                RECT rcParent;
                int  x, y;
                int  xScreen;
                int  yScreen;

                GetWindowRect(hDlg, &rcDialog);
                nWidth  = rcDialog.right  - rcDialog.left;
                nHeight = rcDialog.bottom - rcDialog.top;

                if  (!(hParent = GetParent(hDlg)))
                    hParent = GetDesktopWindow();

                GetClientRect(hParent, &rcParent);
                ClientToScreen(hParent, (LPPOINT)&rcParent.left);  // point(left,  top)
                ClientToScreen(hParent, (LPPOINT)&rcParent.right); // point(right, bottom)

                x = max(rcParent.right - rcParent.left - nWidth, 0) / 2
                    + rcParent.left;
                //Don't let it go off right edge of screen
                xScreen = GetSystemMetrics(SM_CXSCREEN);
                if  (x + nWidth > xScreen)
                    x = max(0, xScreen - nWidth);
                //Don't let it go off left edge of screen
                x = max(x, 0);

	            y = max(rcParent.bottom - rcParent.top - nHeight, 0) / 2
                    + rcParent.top;
                //Don't let it go off top edge of screen
                y = max(y, 0);
                //Don't let it go off bottom edge of screen
                yScreen = GetSystemMetrics(SM_CYSCREEN);
                if  (y + nHeight > yScreen)
                    y = max(0, yScreen - nHeight);

                SetWindowPos(hDlg, NULL,
                             x, y, nWidth, nHeight,
                             flags);
            }//scope

            // See BDLG32 (msdn sample 4544)
            {   // Setup bitmap for display as dialog background.
                RECT    rectClient;
                HDC     hDC;

                // Load bitmap resource -- remember to call DeleteObject when done.
                hBmpDialog = LoadBitmap((HINSTANCE)hDllInstance, MAKEINTRESOURCE(IDB_BITMAP4));
                if (!hBmpDialog)
                    ;   //failed

                // Get bitmap information.
                GetObject(hBmpDialog, sizeof(BITMAP), &bmInfo);

                GetClientRect(hDlg, &rectClient);
                size.cx = rectClient.right;     // zero-based
                size.cy = rectClient.bottom;    // zero-based
                pt.x = rectClient.left;
                pt.y = rectClient.top;

                // Create a compatible memory DC using the dialog's DC.
                hDC = GetDC(hDlg);  // release later!
                hdcMem = CreateCompatibleDC(hDC);
                if (!hdcMem)
                        ;   //failed

                // Select the bitmap into the memory DC.
                hBmpOld = (HBITMAP)SelectObject(hdcMem, hBmpDialog);
                SetMapMode(hdcMem, GetMapMode(hDC));    //needed???

                // Release the client window DC.
                ReleaseDC(hDlg, hDC);
            }
            {//scope

				SetDlgItemText(hDlg, IDC_ABOUT_TEXT, AboutMessage());

            }//scope
            return  TRUE; // TRUE says we did not set the focus to a control.

        case WM_ERASEBKGND:
            if (hBmpDialog)
            {   // Display a bitmap as the dialog background.
                // MUST use the DC passed in wParam, not the DC of hDlg --
                // The clipping/update regions are different!!!
                HDC hDC = (HDC) wParam; // handle of device context

                if (AboutImageMode == DIM_STRETCHED)
                //mode=stretched
                {   // Compress/stretch the bitmap as necessary...
                    // See BDLG32 (msdn sample 4544)
                    int oldMode;

                    oldMode = SetStretchBltMode(hDC, AboutImageStretchMode);
                    StretchBlt(hDC, pt.x, pt.y, size.cx, size.cy,
                               hdcMem, 0, 0, bmInfo.bmWidth, bmInfo.bmHeight,
                               SRCCOPY);
                    SetStretchBltMode(hDC, oldMode);
                }
                else if (AboutImageMode == DIM_TILED)
                //mode=tiled
                {   // Use multiple straight BitBlts... (a la Prosise)
                    int x, y;
                    POINT ptSize, ptOrg, ptTotalSize;

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
            } //if hBmpDialog
            else {
                //Bitmap not available... let DefProc paint the background.
                return FALSE;
            }
            return TRUE;   // No more background painting needed.

        case WM_CTLCOLORSTATIC:
        case WM_CTLCOLORBTN:
            {
                HDC hdc = (HDC) wParam;   // handle of display context
                HWND hwnd = (HWND) lParam; // handle of static control
                int iCtl;

                iCtl = GetDlgCtrlID(hwnd);    // ID of control

                //Info("iCtl = %d", iCtl);
 
                if (iCtl == LOWORD(IDC_STATIC) || iCtl == IDOK || iCtl == IDC_ABOUT_TEXT)
                {   // We want these controls to be transparent...
                    // Note that these controls are NOT updated dynamically
                    // (which doesn't work right, since the backgrounds don't
                    // get erased properly).
                    
                    SetBkMode(hdc, TRANSPARENT);
                    SetTextColor(hdc, RGB(  0,  0,  0));    //black text
                    //should really use SetDlgMsgResult...
                    return (INT_PTR) (HBRUSH) GetStockObject(HOLLOW_BRUSH); // create statically?
                }
                else return FALSE;  // Not processed.
            }
            break;

        case WM_DESTROY:
            {   // Restore old bitmap.
                if (hBmpOld) {
                    SelectObject(hdcMem, hBmpOld);
                }
                // Need to DeleteObject() the bitmap that was loaded.
                if (hBmpDialog) {
                    DeleteObject(hBmpDialog);
                }
                // Delete the memory DC.
                DeleteDC(hdcMem);
            }
            return FALSE;   // call defproc

        case WM_COMMAND:
            idd = COMMANDID (wParam);
            iCmd = COMMANDCMD(wParam, lParam);
            switch  (idd) {
            case LOWORD(IDC_STATIC):
            case IDC_ABOUT_TEXT:
            case IDOK:
            case IDCANCEL:
                break;          // fall through to EndDialog()
            default:
                return FALSE;   // this idd not handled
            } //switch idd
            //fall through...
        case WM_LBUTTONDOWN:
        case WM_RBUTTONDOWN:
            EndDialog(hDlg, idd);
            return TRUE;    // do not call defProc

        default:
            return  FALSE;  // wMsg not handled
    } //switch wMsg

    return  TRUE;   // handled; do not call defProc

} /*AboutDialogProc*/

/*****************************************************************************/

void DoAbout (GPtr globals, short selector)
{
#if 0
    // Old style ABout -- uses simple Message Box.
    AboutRecordPtr aboutPtr;
    char message[256];
    char caption[60];
    PlatformData *platform;

    aboutPtr = (AboutRecordPtr) gStuff;
    platform = (PlatformData *) (aboutPtr->platformData);

    LoadString(hDllInstance, 1, message, sizeof(message));
    LoadString(hDllInstance, 2, caption, sizeof(caption));
    MessageBox((HWND) (platform->hwnd),message,caption,MB_APPLMODAL|MB_ICONINFORMATION|MB_OK);
#else
    // New fancy About box...
    int    nResult;
    PlatformData *platform;
    HWND hOwner;

    // PlatformData is in different places, depending on selector...
    if (selector == filterSelectorAbout) {
        platform = (PlatformData *)((AboutRecordPtr) gStuff)->platformData;
        hOwner = (HWND)platform->hwnd;
    }
    else {
        // Platform data is not set for other selectors???
        // So just set owner = our filter dialog.
#if 0
        platform = ((FilterRecordPtr) gStuff)->platformData;
#else
        hOwner = MyHDlg;
#endif
    }

    nResult = (int)DialogBoxParam((HINSTANCE)hDllInstance,
                       (LPSTR)"ABOUT",
                       (HWND)hOwner,
                       (DLGPROC)AboutDialogProc,
                       (LPARAM)selector);
#endif
} /*DoAbout*/

/*****************************************************************************/
