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