				// Turn off XP Visual Styles
				//setDefaultWindowTheme(hCtl);
			}


			
            if (c != CC_STANDARD && c != CC_SLIDER) break;

            //create the NUMEDIT buddy control
            if (s & SBS_VERT || s & TBS_VERT) {
                //vertical scrollbar
                alignStyle = ES_CENTER;
            }
            else {
                //horizontal scrollbar
                alignStyle = ES_LEFT;
            }
            
			//gParams->ctl[n].buddy1StyleEx |= WS_EX_STATICEDGE; //staticedge replaces black border as default?
			
			computeBuddy1Pos(n, x, y, w, h, NULL, NULL, &r);

            hBuddy1 = CreateWindowEx(  
                gParams->ctl[n].buddy1StyleEx, //0 /*WS_EX_STATICEDGE*/,               // extended styles
                "EDIT",                         // predefined class 
                "",                            // control text
                alignStyle | WS_VISIBLE | WS_CHILD /*| WS_BORDER*/ | WS_TABSTOP
#if 1
                | ES_MULTILINE | ES_WANTRETURN  // to get EN_MAXTEXT when user hits return
#endif
                /*| ES_NUMBER*/,                // styles (ES_NUMBER doesn't allow negative numbers!)
                r.left,                         // starting x position 
                r.top,                          // starting y position 
                r.right,                        // button width 
                r.bottom,                       // button height 
                fmc.hDlg,                       // parent window 
                (HMENU) (INT_PTR)(IDC_BUDDY1+n),         // control ID
				(HINSTANCE)hDllInstance,
                NULL);                          // pointer not needed 

            if (!hBuddy1) return FALSE;    //failed to create window.
            gParams->ctl[n].hBuddy1 = hBuddy1;

			//Add border here, otherwise we cannot remove it anymore
			SetWindowLong(gParams->ctl[n].hBuddy1, GWL_STYLE,
				GetWindowLong(gParams->ctl[n].hBuddy1, GWL_STYLE) | WS_BORDER);
			gParams->ctl[n].buddy1Style = WS_BORDER;

			SetWindowPos(gParams->ctl[n].hBuddy1, NULL, 0, 0, 0, 0,
                        SWP_FRAMECHANGED|SWP_NOZORDER|SWP_NOACTIVATE|SWP_NOSIZE|SWP_NOMOVE);

            // Turn off XP Visual Styles
            //setDefaultWindowTheme(hBuddy1);

			/*
			 hBuddy3 = CreateWindowEx(  
                0,//gParams->ctl[n].buddy1StyleEx, 
                "msctls_updown32",                      // predefined class 
                "",                            // control text
                WS_VISIBLE | WS_CHILD | WS_TABSTOP | UDS_ALIGNRIGHT | UDS_ARROWKEYS,
                0,                         // starting x position 
                0,                          // starting y position 
                0,                        // button width 
                0,                       // button height 
                fmc.hDlg,                       // parent window 
                (HMENU) (IDC_BUDDY1+n),         // control ID
                hDllInstance,
                NULL);                          // pointer not needed 

			SendMessage(hBuddy3, UDM_SETBUDDY, (WPARAM)gParams->ctl[n].hBuddy1, 0);
			*/

            //create the static text (label) buddy
            if (s & SBS_VERT || s & TBS_VERT) {
                //vertical scrollbar
                alignStyle = SS_CENTER;
            }
            else {
                //horizontal scrollbar
                alignStyle = SS_RIGHT;
            }
            alignStyle |= (s & WS_BORDER);  //OR in border style from main control
            gParams->ctl[n].sb.w2 = 0;      //dummy width
            gParams->ctl[n].sb.h2 = 0;      //dummy height
			gParams->ctl[n].sb.maxw2 = 0;   //dummy width
            gParams->ctl[n].sb.maxh2 = 0;   //dummy height
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
                    SendMessage(hBuddy1, WM_SETFONT, (WPARAM)hDlgFont, 0L);  //redraw=FALSE
                    SendMessage(hBuddy2, WM_SETFONT, (WPARAM)hDlgFont, 0L);  //redraw=FALSE
                }
            }

            // Set the control text now, so the size of the static text control
            // can be computed correctly...
            if (gParams->ctl[n].label[0] != '\0') {
                gParams->ctl[n].inuse = TRUE;   //so setCtlText can work
                setCtlText(n, (char *)formatString(gParams->ctl[n].label));
            }
        }//scope
        break;

/*
    case 33333333: //CC_TRACKBAR:
        {
            //create a trackbar control
            //(and a NUMEDIT buddy.???)
            HWND hCtl;
            // Convert dialog units to pixels
            RECT r;

            // Should link dynamically, so only load comctl32.dll
            // if necessary!
            InitCommonControls();

            r.left = x;
            r.top = y;
            r.right = w;
            r.bottom = h;
            MapDialogRect(fmc.hDlg, &r);

            //Default tick frequency is 1.
            gParams->ctl[n].tb.ticFreq = 1;

            // force default styles (note TBS_HORZ==0 and TBS_BOTTOM==0).
            gParams->ctl[n].style |= WS_VISIBLE | WS_CHILD | WS_TABSTOP;

            // Transparent backgrounds don't work well for trackbars,
            // so change the default to opaque light gray.
            DeleteObject(gParams->ctl[n].hBrush); //superfluous for stock object
            gParams->ctl[n].hBrush = CreateSolidBrush(GetSysColor(COLOR_BTNFACE));//GetStockObject(LTGRAY_BRUSH);
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
                (HMENU) (INT_PTR)(IDC_CTLBASE+n),        // control ID
                hDllInstance,
                NULL);                          // pointer not needed 

            if (!hCtl) return FALSE;    //failed to create window.
            gParams->ctl[n].hCtl = hCtl;

            // Turn off XP Visual Styles
            //setDefaultWindowTheme(hCtl);
			gParams->ctl[n].inuse = TRUE;
			setCtlTheme(n, (int)gParams->ctl[n].theme);


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

#if 0
            // Set control font to same as dialog font
            {
                HFONT hDlgFont;
                if ((hDlgFont = (HFONT)SendMessage(fmc.hDlg, WM_GETFONT, 0, 0L)) != NULL)
                {
                    gParams->ctl[n].hFont = hDlgFont;
                    SendMessage(hCtl, WM_SETFONT, (WPARAM)hDlgFont, 0L);  //redraw=FALSE
                }
            }
#endif
        }
        break;
*/

    case CC_PUSHBUTTON:
    case CC_CHECKBOX:
    case CC_RADIOBUTTON:
    case CC_GROUPBOX:
    case CC_OWNERDRAW:
	    {
            //create a button control
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
            if (c != CC_RADIOBUTTON && c != CC_GROUPBOX) {
                //only first radio button in group should have TABSTOP.
                gParams->ctl[n].style |= WS_TABSTOP;
            }
#if 0
            if (c == CC_RADIOBUTTON && (gParams->ctl[n].style & WS_GROUP)) {
                //set TABSTOP on first radio button in GROUP???
                gParams->ctl[n].style |= WS_TABSTOP;
            }
#endif
#if 0
            if (c == CC_GROUPBOX) {
                //groupbox should always have GROUP style???
                gParams->ctl[n].style |= WS_GROUP;
            }
#endif
            gParams->ctl[n].maxval = 2;

			
            // create the button control
            hCtl = CreateWindowEx(  
                gParams->ctl[n].styleEx,        // extended styles
                "BUTTON",                       // predefined class 
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

    case CC_IMAGE:
        gParams->ctl[n].im.hdcMem = 0;
        gParams->ctl[n].im.hdcAnd = 0;
        gParams->ctl[n].im.hbm = 0;
        gParams->ctl[n].im.hbmOld = 0;