        gParams->ctl[n].im.hbmOldAnd = 0;
        gParams->ctl[n].style |= SS_OWNERDRAW;  //force ownerdraw style
        //fall through...
    case CC_STATICTEXT:
    case CC_FRAME:
    case CC_RECT:
    case CC_BITMAP:
        //gParams->ctl[n].style |= SS_BITMAP;  //force style
    case CC_ICON:
        //gParams->ctl[n].style |= SS_ICON;  //force style
    case CC_METAFILE:
        //gParams->ctl[n].style |= SS_ENHMETAFILE;  //force style
        {
            //create a static control
            HWND hCtl;
            /* Convert dialog units to pixels */
            RECT r;

            r.left = x;
            r.top = y;
            r.right = w;
            r.bottom = h;
            MapDialogRect(fmc.hDlg, &r);

            // force default styles.
            gParams->ctl[n].style |= WS_VISIBLE | WS_CHILD;

            // create the static control
            hCtl = CreateWindowEx(  
                gParams->ctl[n].styleEx,        // extended styles
                "STATIC",                       // predefined class 
                formatString(gParams->ctl[n].label),  // control text
                gParams->ctl[n].style,          // styles
                r.left,                         // starting x position 
                r.top,                          // starting y position 
                r.right,                        // control width 
                r.bottom,                       // control height 
                fmc.hDlg,                       // parent window 
                (HMENU) (INT_PTR)(IDC_CTLBASE+n),        // control ID
				(HINSTANCE)hDllInstance,
                NULL);                          // pointer not needed 

            if (!hCtl) return FALSE;    //failed to create window.
            gParams->ctl[n].hCtl = hCtl;

            // Turn off XP Visual Styles
            //setDefaultWindowTheme(hCtl);
			gParams->ctl[n].inuse = TRUE;
			// setCtlTheme(n, (int)gParams->ctl[n].theme);

            // Set control font to same as dialog font
            {
                HFONT hDlgFont;
                if ((hDlgFont = (HFONT)SendMessage(fmc.hDlg, WM_GETFONT, 0, 0L)) != NULL)
                {
                    gParams->ctl[n].hFont = hDlgFont;
                    SendMessage(hCtl, WM_SETFONT, (WPARAM)hDlgFont, 0L);  //redraw=FALSE
                }
            }
        }
        break;

    case CC_COMBOBOX:
        {
            //create a combo box control
            HWND hCtl;
            /* Convert dialog units to pixels */
            RECT r;

            r.left = x;
            r.top = y;
            r.right = w;
            r.bottom = h;
            MapDialogRect(fmc.hDlg, &r);

            // force default styles.
            gParams->ctl[n].style |= WS_VISIBLE | WS_CHILD | WS_TABSTOP;

            // Combo box-specific parameters
            gParams->ctl[n].val = -1;       //no item selected
            gParams->ctl[n].initVal = -1;   //no item selected
            gParams->ctl[n].minval = -1;
            gParams->ctl[n].maxval = 0xFFFF;    //max item index

            // create the combo box control
            hCtl = CreateWindowEx(  
                gParams->ctl[n].styleEx,        // extended styles
                "COMBOBOX",                     // predefined class 
                "",                             // control text
                gParams->ctl[n].style,          // styles
                r.left,                         // starting x position 
                r.top,                          // starting y position 
                r.right,                        // control width 
                r.bottom,                       // control height 
                fmc.hDlg,                       // parent window 
                (HMENU) (INT_PTR)(IDC_CTLBASE+n),        // control ID
				(HINSTANCE)hDllInstance,
                NULL);                          // pointer not needed 

            if (!hCtl) return FALSE;    //failed to create window.
            gParams->ctl[n].hCtl = hCtl;

            // Turn off XP Visual Styles
            //setDefaultWindowTheme(hCtl);
			gParams->ctl[n].inuse = TRUE;
			// setCtlTheme(n, (int)gParams->ctl[n].theme);


            if (gParams->ctl[n].properties & CTP_EXTENDEDUI) {
                // Set extended keyboard UI
                SendMessage(hCtl, CB_SETEXTENDEDUI, TRUE, 0);
            }

            // Set control font to same as dialog font
            {
                HFONT hDlgFont;
                if ((hDlgFont = (HFONT)SendMessage(fmc.hDlg, WM_GETFONT, 0, 0L)) != NULL)
                {
                    gParams->ctl[n].hFont = hDlgFont;
                    SendMessage(hCtl, WM_SETFONT, (WPARAM)hDlgFont, 0L);  //redraw=FALSE
                }
            }

            // Populate the list box from the control text string...
            gParams->ctl[n].inuse = TRUE;   //so setCtlText can work
            setCtlText(n, (char *)gParams->ctl[n].label);


			//Label
			{
                HWND hBuddy2;
                int alignStyle;
            
                alignStyle = SS_RIGHT;
                alignStyle |= (s & WS_BORDER);  //OR in border style from main control
                gParams->ctl[n].sb.w2 = 0;      //dummy width
                gParams->ctl[n].sb.h2 = 0;      //dummy height
				gParams->ctl[n].sb.maxw2 = 0;      //dummy width
                gParams->ctl[n].sb.maxh2 = 0;      //dummy height
                computeBuddy2Pos(n, x, y, w, h, &r);

                hBuddy2 = CreateWindowEx(  
                    gParams->ctl[n].buddy2StyleEx, //0,                              // extended styles
                    "STATIC",                       // predefined class 
                    "",                             // control text (set later...)
                    WS_VISIBLE | WS_CHILD |
                    SS_NOTIFY |                     // (So tooltips will work)
                    gParams->ctl[n].buddy2Style | alignStyle,                     // styles
                    r.left,                         // starting x position 
                    r.top,                          // starting y position 
                    r.right,                        // button width 
                    r.bottom,                       // button height 
                    fmc.hDlg,                       // parent window 
                    (HMENU) (INT_PTR)(IDC_BUDDY2+n),         // control ID
					(HINSTANCE)hDllInstance,
                    NULL);                          // pointer not needed 

                if (!hBuddy2) return FALSE;    //failed to create window.
                gParams->ctl[n].hBuddy2 = hBuddy2;

                // Turn off XP Visual Styles
                //setDefaultWindowTheme(hBuddy2);
			    gParams->ctl[n].inuse = TRUE;
			    // setCtlTheme(n, (int)gParams->ctl[n].theme);
			    
                // Set control font to same as dialog font
                {
                    HFONT hDlgFont;
                    if ((hDlgFont = (HFONT)SendMessage(fmc.hDlg, WM_GETFONT, 0, 0L)) != NULL)
                    {
                        gParams->ctl[n].hFont = hDlgFont;
                        //SendMessage(hBuddy1, WM_SETFONT, (WPARAM)hDlgFont, 0L);  //redraw=FALSE
                        SendMessage(hBuddy2, WM_SETFONT, (WPARAM)hDlgFont, 0L);  //redraw=FALSE
                    }
                }

                // Set the control text now, so the size of the static text control
                // can be computed correctly...
                /*if (gParams->ctl[n].label2[0] != '\0') {
                    gParams->ctl[n].inuse = TRUE;   //so setCtlText can work
                    setCtlLabel(n, (char *)formatString(gParams->ctl[n].label2));
                }*/
            }

        }
        break;

    case CC_LISTBOX:
    case CC_LISTBAR:
        {
            //create a list box control
            HWND hCtl;
            /* Convert dialog units to pixels */
            RECT r;

            r.left = x;
            r.top = y;
            r.right = w;
            r.bottom = h;
            MapDialogRect(fmc.hDlg, &r);

            // force default styles.
            gParams->ctl[n].style |= LBS_NOTIFY | WS_VISIBLE | WS_CHILD | WS_TABSTOP;
            if (c==CC_LISTBAR) {
                gParams->ctl[n].style |= LBS_MULTICOLUMN; //| WS_BORDER; 
                //gParams->ctl[n].styleEx |= WS_EX_CLIENTEDGE;
            }

            // List box-specific parameters
            gParams->ctl[n].val = -1;       //no item selected
            gParams->ctl[n].initVal = -1;   //no item selected
            gParams->ctl[n].minval = -1;
            gParams->ctl[n].maxval = 0xFFFF;    //max item index

			DeleteObject(gParams->ctl[n].hBrush); //superfluous for stock object
			gParams->ctl[n].hBrush = CreateSolidBrush(GetSysColor(COLOR_BTNFACE));
			gParams->ctl[n].bkColor = GetSysColor(COLOR_BTNFACE);
			
            // create the list box control
            hCtl = CreateWindowEx(  
                gParams->ctl[n].styleEx,        // extended styles
                "LISTBOX",                      // predefined class 
                "",                             // control text
                gParams->ctl[n].style,          // styles
                r.left,                         // starting x position 
                r.top,                          // starting y position 
                r.right,                        // control width 
                r.bottom,                       // control height 
                fmc.hDlg,                       // parent window 
                (HMENU) (INT_PTR)(IDC_CTLBASE+n),        // control ID
				(HINSTANCE)hDllInstance,
                NULL);                          // pointer not needed 

            if (!hCtl) return FALSE;    //failed to create window.
            gParams->ctl[n].hCtl = hCtl;

            // Turn off XP Visual Styles
            //setDefaultWindowTheme(hCtl);
			gParams->ctl[n].inuse = TRUE;
			// setCtlTheme(n, (int)gParams->ctl[n].theme);

#if 0
            {//scope
                RECT rc;
                GetWindowRect(hCtl, &rc);
                Info("Set to (%d,%d,%d,%d)\n"
                     "Adjusted to (%d,%d,%d,%d)",
                     r.left, r.top, r.right, r.bottom,
                     rc.left, rc.top, rc.right, rc.bottom);
            }//scope
#endif

            // Set control font to same as dialog font
            {
                HFONT hDlgFont;
                if ((hDlgFont = (HFONT)SendMessage(fmc.hDlg, WM_GETFONT, 0, 0L)) != NULL)
                {
                    gParams->ctl[n].hFont = hDlgFont;
                    SendMessage(hCtl, WM_SETFONT, (WPARAM)hDlgFont, 0L);  //redraw=FALSE
                }
            }

            // Populate the list box from the control text string...
            gParams->ctl[n].inuse = TRUE;   //so setCtlText can work
            setCtlText(n, (char *)gParams->ctl[n].label);

            
           //Label
           if (c==CC_LISTBAR) {

                HWND hBuddy2;
                int alignStyle;
            
                alignStyle = SS_RIGHT;
                alignStyle |= (s & WS_BORDER);  //OR in border style from main control
                gParams->ctl[n].sb.w2 = 0;      //dummy width
                gParams->ctl[n].sb.h2 = 0;      //dummy height
                computeBuddy2Pos(n, x, y, w, h, &r);

                hBuddy2 = CreateWindowEx(  
                    gParams->ctl[n].buddy2StyleEx, //0,                              // extended styles
                    "STATIC",                       // predefined class 
                    "",                             // control text (set later...)
                    WS_VISIBLE | WS_CHILD |
                    SS_NOTIFY |                     // (So tooltips will work)
                    gParams->ctl[n].buddy2Style | alignStyle,                     // styles
                    r.left,                         // starting x position 
                    r.top,                          // starting y position 
                    r.right,                        // button width 
                    r.bottom,                       // button height 
                    fmc.hDlg,                       // parent window 
                    (HMENU) (INT_PTR)(IDC_BUDDY2+n),         // control ID
					(HINSTANCE)hDllInstance,
                    NULL);                          // pointer not needed 

                if (!hBuddy2) return FALSE;    //failed to create window.
                gParams->ctl[n].hBuddy2 = hBuddy2;

                // Turn off XP Visual Styles
                //setDefaultWindowTheme(hBuddy2);