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