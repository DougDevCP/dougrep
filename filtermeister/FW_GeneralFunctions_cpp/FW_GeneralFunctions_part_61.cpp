            //horizontal scrollbar
            if (w < 0) w = 90;
            if (h < 0) h = 8;
        }
        break;
    case CC_TRACKBAR:
	case CC_SLIDER:
        if (s & TBS_VERT) {
            //vertical trackbar
            if (w < 0) w = 10;
            if (h < 0) h = 90;
        }
        else {
            //horizontal trackbar
            if (w < 0) w = 90;
            if (h < 0) h = 10;
        }
        break;
    case CC_PREVIEW:
        if (w < 0) w = 228;
        if (h < 0) h = 154;
        break;
    case CC_TOOLBAR:
		if (w < 0) w = 150;
        if (h < 0) h = 13;
		break;
	case CC_TAB:
		if (w < 0) w = 120;
        if (h < 0) h = 60;
		break;
	case CC_SPINNER:
    case CC_UPDOWN:
    case CC_CHECKBOX:
    case CC_RADIOBUTTON:
    case CC_OWNERDRAW:
	case CC_PROGRESSBAR:
    case CC_EDIT:
    case CC_STATICTEXT:
    case CC_TOOLTIP:
    default:
        if (w < 0) w = 90;
        if (h < 0) h = 8;
        break;
    }//switch class

    // Default position depends on whether the control is
    // a predefined control...
    switch (n) {
	//Changed by Harald Heim, March 9, 2003
    case CTL_OK:
        if (x < 0) x = 408;//306;
        if (y < 0) y = 160;//126;
        break;
    case CTL_CANCEL:
        if (x < 0) x = 446;//269;
        if (y < 0) y = 160;//126;
        break;

    case CTL_PREVIEW:
        if (x < 0) x = 5;
        if (y < 0) y = 5;
        break;
	case CTL_PROGRESS:
    case CTL_HOST:
    default:
        //Changed by Harald Heim, March 9, 2003
        if (x < 0) x = 350 - (n/10 % 10)*5;
        if (y < 0) y = 10*(n % 10) + 5;
        break;
    }//switch n

    // Default action depends on control class (and sometimes
    // control index)...
    switch (c) {
    case CC_RADIOBUTTON:
    case CC_GROUPBOX:
    case CC_LISTVIEW:
	case CC_LISTBOX:
    case CC_COMBOBOX:
	case CC_EDIT:
	case CC_TAB:
	case CC_TOOLBAR:
        // Default action for these is NONE
        gParams->ctl[n].action = CA_NONE;
        break;
    case CC_PUSHBUTTON:
    case CC_STANDARD:
	case CC_SLIDER:
    case CC_SCROLLBAR:
    case CC_TRACKBAR:
    case CC_SPINNER:
    case CC_UPDOWN:
    case CC_LISTBAR:
    case CC_CHECKBOX:
    case CC_OWNERDRAW:
	case CC_PROGRESSBAR:
    case CC_STATICTEXT:
    case CC_RECT:
    case CC_FRAME:
    case CC_IMAGE:
    case CC_BITMAP:
    case CC_ICON:
    case CC_METAFILE:
    case CC_TOOLTIP:
    case CC_PREVIEW:
    case CC_ANIMATION:
    default:
        //default action for most controls is PREVIEW
        gParams->ctl[n].action = CA_PREVIEW;
        break;
    }//switch class

    //Default actions for predefined controls are different...
    switch (n) {
    case CTL_OK:
        gParams->ctl[n].action = CA_APPLY;
        break;
    case CTL_CANCEL:
        gParams->ctl[n].action = CA_CANCEL;
        break;

    case CTL_PREVIEW:
    case CTL_PROGRESS:
    case CTL_HOST:
    default:
        break;
    }//switch n


    gParams->ctl[n].xPos = x;
    gParams->ctl[n].yPos = y;
    gParams->ctl[n].width = w;
    gParams->ctl[n].height = h;
    gParams->ctl[n].style = s;
    gParams->ctl[n].styleEx = sx;
    gParams->ctl[n].properties = p;
    gParams->ctl[n].hBrush = (HBRUSH)GetStockObject(HOLLOW_BRUSH);
#if 0
    gParams->ctl[n].bkColor = GetSysColor(COLOR_BTNFACE);
#else
    if (c == CC_GROUPBOX){ //Avoid that text of a group box is stricked through
        gParams->ctl[n].bkColor = GetSysColor(COLOR_BTNFACE);
    } else
        gParams->ctl[n].bkColor = (COLORREF) -1;    //transparent

	gParams->ctl[n].buddyBkColor = (COLORREF) -1;    //transparent
	gParams->ctl[n].editColor = (COLORREF) -1;    //do not apply
	gParams->ctl[n].editTextColor = (COLORREF) -1;    //do not apply
#endif


#if 0
    gParams->ctl[n].textColor = GetSysColor(COLOR_BTNTEXT);
#else
    //Changed by Harald Heim, March 22, 2003
    //gParams->ctl[n].textColor = RGB(255,255,255);  //fontcolor=white
    gParams->ctl[n].textColor = GetSysColor(COLOR_WINDOWTEXT);

#endif
    gParams->ctl[n].hFont = (HFONT)GetStockObject(DEFAULT_GUI_FONT);
    gParams->ctl[n].fontSize = 8;

    gParams->ctl[n].enabled = e;

    gParams->ctl[n].val = 0;
    gParams->ctl[n].initVal = 0;
    gParams->ctl[n].minval = 0;
    gParams->ctl[n].maxval = 255;
    gParams->ctl[n].lineSize = 1;
    gParams->ctl[n].pageSize = 10;
	gParams->ctl[n].thumbSize = 0;
    gParams->ctl[n].divisor = 1;
    gParams->ctl[n].tooltip[0] = '\0';
    gParams->ctl[n].image[0] = '\0';
    gParams->ctl[n].imageType = 0;

    switch (c) {
    case CC_STANDARD:
    case CC_SCROLLBAR:
	case CC_TRACKBAR:
	case CC_SLIDER:
        {                   ///////  BUG!!!!  Buddy2 (static text) should be created first
                            ///////           for correct tab order!!
            //create a scrollbar with a static text buddy
            //and a NUMEDIT buddy.
            HWND hCtl, hBuddy1, hBuddy2;//, hBuddy3;
            int alignStyle;
            /* Convert dialog units to pixels */
            RECT r;

			r.left = x;
			r.top = y;
			r.right = w;
			r.bottom = h;
			MapDialogRect(fmc.hDlg, &r);


			if (c==CC_TRACKBAR || c==CC_SLIDER){
				
				// Should link dynamically, so only load comctl32.dll
				// if necessary!
				InitCommonControls();

				//r.left = x;
				//r.top = y;
				//r.right = w;
				//r.bottom = h;
				//MapDialogRect(fmc.hDlg, &r);

				//Default tick frequency is 1.
				gParams->ctl[n].tb.ticFreq = 1;

				// force default styles (note TBS_HORZ==0 and TBS_BOTTOM==0).
				gParams->ctl[n].style |= WS_VISIBLE | WS_CHILD | WS_TABSTOP | TBS_NOTICKS | TBS_BOTH;// | TBS_FIXEDLENGTH; //| 0x1000L; //0x1000L = TBS_TRANSPARENTBKGND

				// Transparent backgrounds don't work well for trackbars,
				// so change the default to opaque light gray.
				DeleteObject(gParams->ctl[n].hBrush); //superfluous for stock object
				gParams->ctl[n].hBrush = CreateSolidBrush(GetSysColor(COLOR_BTNFACE));//GetStockObject(LTGRAY_BRUSH); //GetStockObject(HOLLOW_BRUSH);
				gParams->ctl[n].bkColor = GetSysColor(COLOR_BTNFACE);

				// create the trackbar control
				hCtl = CreateWindowEx(  
					gParams->ctl[n].styleEx,        // extended styles
					TRACKBAR_CLASS,                 // predefined class 
					"",                             // control text
					gParams->ctl[n].style,          // styles
					r.left,                         // starting x position 
					r.top,                          // starting y position 
					r.right,                        // control width 
					r.bottom,                       // control height 
					fmc.hDlg,                       // parent window 
					(HMENU)(INT_PTR)(IDC_CTLBASE+n),        // control ID
					(HINSTANCE)hDllInstance,
					NULL);                          // pointer not needed 

                //SetClassLong(hCtl, GCL_HBRBACKGROUND, (LONG)GetStockObject(HOLLOW_BRUSH));

				if (!hCtl) return FALSE;    //failed to create window.
				gParams->ctl[n].hCtl = hCtl;

                //gParams->ctl[n].thumbSize = 8;
                //SendMessage (gParams->ctl[n].hCtl, TBM_SETTHUMBLENGTH, (WPARAM) VDBUsToPixels(8), 0);
                
				// Turn off XP Visual Styles
				//setDefaultWindowTheme(hCtl);
				gParams->ctl[n].inuse = TRUE;
				// setCtlTheme(n, (int)gParams->ctl[n].theme);
                

				if (gParams->ctl[n].style & TBS_TOOLTIPS) {
					int side;
					//set side for automatic tooltips
					if (gParams->ctl[n].properties & CTP_BOTTOM) {
						side = TBTS_BOTTOM;
					}
					else if (gParams->ctl[n].properties & CTP_RIGHT) {
						side = TBTS_RIGHT;
					}
					else if (gParams->ctl[n].properties & CTP_LEFT) {
						side = TBTS_LEFT;
					}
					else {
						//default is TOP
						side = TBTS_TOP;
					}
					SendMessage(gParams->ctl[n].hCtl, TBM_SETTIPSIDE, side, 0);
				}


			} else {

				//r.left = x;
				//r.top = y;
				//r.right = w;
				//r.bottom = h;
				//MapDialogRect(fmc.hDlg, &r);

				// force default styles (note SBS_HORZ==0).
				gParams->ctl[n].style |= WS_VISIBLE | WS_CHILD | WS_TABSTOP;

				// create the scrollbar control
				hCtl = CreateWindowEx(  
					gParams->ctl[n].styleEx,        // extended styles
					"SCROLLBAR",                    // predefined class 
					"",                             // control text
					gParams->ctl[n].style           // styles
					& ~WS_BORDER,                   // ...except WS_BORDER (see below)
					r.left,                         // starting x position 
					r.top,                          // starting y position 
					r.right,                        // control width 
					r.bottom,                       // control height 
					fmc.hDlg,                       // parent window 
					(HMENU)(INT_PTR)(IDC_CTLBASE+n),        // control ID
					(HINSTANCE)hDllInstance,
					NULL);                          // pointer not needed 

				if (!hCtl) return FALSE;    //failed to create window.
				gParams->ctl[n].hCtl = hCtl;
