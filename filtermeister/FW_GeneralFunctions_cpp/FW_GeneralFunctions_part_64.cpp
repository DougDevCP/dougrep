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

	case CC_LISTVIEW:
		{
			HWND hCtl;
            RECT r;
			INITCOMMONCONTROLSEX icex;           // Structure for control initialization.

			icex.dwICC = ICC_LISTVIEW_CLASSES;
			InitCommonControlsEx(&icex);

            r.left = x;
            r.top = y;
            r.right = w;
            r.bottom = h;
            MapDialogRect(fmc.hDlg, &r);

            // force default styles.
            gParams->ctl[n].style |= WS_CHILD | LVS_ICON | LVS_NOCOLUMNHEADER | LVS_ALIGNTOP | LVS_AUTOARRANGE; //LVS_EDITLABELS; //LVS_REPORT
			//if (getAppTheme() && getWindowsVersion()>=11)
				gParams->ctl[n].style |= LVS_NOLABELWRAP; //Works fine under Vista/7 but not under XP or without visual styles
			
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
                WC_LISTVIEW,                    // predefined class 
                "",                            // control text
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


			ListView_SetExtendedListViewStyle(hCtl,LVS_EX_DOUBLEBUFFER|LVS_EX_FULLROWSELECT); //|LVS_EX_LABELTIP
			
			//if (getAppTheme() && getWindowsVersion()<11) ListView_SetExtendedListViewStyle(hCtl,LVS_EX_BORDERSELECT); //Permanently deactivates full row select under XP

			//Insert column
			{
				LVCOLUMN lvc;
				lvc.mask = LVCF_WIDTH; //  LVCF_FMT | LVCF_TEXT | LVCF_SUBITEM;
				//lvc.cx = HDBUsToPixels(w)+10;
				lvc.cx = r.right+9;
				//Info ("%d,%d",r.right,r.left);
				ListView_InsertColumn (hCtl,0,&lvc);
				//ListView_SetColumnWidth(hCtl,0,LVSCW_AUTOSIZE);
			}

			gParams->ctl[n].inuse = TRUE;

			// Turn off XP Visual Styles
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

            // Populate the list box from the control text string...
            gParams->ctl[n].inuse = TRUE;   //so setCtlText can work
            //setCtlText(n, (char *)gParams->ctl[n].label);


			//Subclass
			if (getAppTheme()){
				WNDPROC OldProc = (WNDPROC)SetWindowLongPtr(hCtl,GWLP_WNDPROC,(LONG_PTR)ListviewSubclassProc);
				SetWindowLongPtr(hCtl,GWLP_USERDATA,(LONG_PTR)OldProc);
			}

		}
		break;	

    case CC_EDIT:
		{
            //create a edit control
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

            // create the static control
            hCtl = CreateWindowEx(  
                gParams->ctl[n].styleEx,        // extended styles
                "EDIT",                       // predefined class 
                "",//formatString(gParams->ctl[n].label), // control text
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

	case CC_TAB:
		{
            //create a tab control
            HWND hCtl;
            /* Convert dialog units to pixels */
            RECT r;
			//TCITEM tabItem; 

			
			InitCommonControls(); 

            r.left = x;
            r.top = y;
            r.right = w;
            r.bottom = h;
            MapDialogRect(fmc.hDlg, &r);

            // force default styles.
            gParams->ctl[n].style |= WS_CHILD | WS_CLIPSIBLINGS | WS_VISIBLE;// | WS_CLIPCHILDREN;

            // create the static control
            hCtl = CreateWindowEx(  
                gParams->ctl[n].styleEx,        // extended styles
                WC_TABCONTROL,                  // predefined class 
                "",//formatString(gParams->ctl[n].label), // control text
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

			//TabCtrl_SetMinTabWidth (gParams->ctl[n].hCtl, 20);

            // Turn off XP Visual Styles
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

            setCtlText(n,t);

			//Add tab button
			//tabItem.mask = TCIF_TEXT | TCIF_IMAGE; 
			//tabItem.iImage = -1; 
			//tabItem.pszText = "Tab1"; 

			//TabCtrl_InsertItem(hCtl, 0, &tabItem);

			//Subclass
			/*if (getAppTheme()){
				WNDPROC OldProc = (WNDPROC)SetWindowLongPtr(hCtl,GWLP_WNDPROC,(LONG_PTR)TabSubclassProc);
				SetWindowLongPtr(hCtl,GWLP_USERDATA,(LONG_PTR)OldProc);
			}*/

        }
        break;

	case CC_TOOLBAR:
		{

			//create a tab control
            HWND hCtl;
            RECT r;
			//HWND hBuddy2;
			
			InitCommonControls(); 

			//Info ("%d, %d\n%d x %d",x,y,w,h);

            r.left = x;
            r.top = y;
            r.right = w;
            r.bottom = h;
            MapDialogRect(fmc.hDlg, &r);

			/*
			//Use static window for embedding the tool bar in order to position it
			hBuddy2 = CreateWindowEx(  
                0, //gParams->ctl[n].buddy2StyleEx, //0,                              // extended styles
                "STATIC",                       // predefined class 
                "",                             // control text (set later...)
                WS_VISIBLE | WS_CHILD |
                //SS_NOTIFY |                      // (So tooltips will work)
                0, //gParams->ctl[n].buddy2Style,// | alignStyle,                     // styles
                r.left,                         // starting x position 
                r.top,                          // starting y position 
                r.right,                        // button width 
                r.bottom,                       // button height 
                fmc.hDlg,                       // parent window 
                (HMENU) (INT_PTR)(IDC_BUDDY2+n),         // control ID
                hDllInstance,
                NULL);                          // pointer not needed 

            if (!hBuddy2) return FALSE;    //failed to create window.
            gParams->ctl[n].hBuddy2 = hBuddy2;
			*/

			//gParams->ctl[n].bkColor = getSysColor(COLOR_BTNFACE);


            // force default styles.
            gParams->ctl[n].style |= WS_CHILD | WS_VISIBLE | TBSTYLE_FLAT | TBSTYLE_LIST | CCS_NODIVIDER | CCS_NOPARENTALIGN | CCS_NORESIZE;// TBSTYLE_WRAPABLE | | TBSTYLE_FLAT | CCS_NOPARENTALIGN | CCS_NORESIZE;  | WS_CLIPSIBLINGS
			//if (!getAppTheme()) gParams->ctl[n].style |= TBSTYLE_FLAT; // | CCS_NOPARENTALIGN | CCS_NORESIZE;

            // create the static control
            hCtl = CreateWindowEx(  
                gParams->ctl[n].styleEx,        // extended styles
                TOOLBARCLASSNAME,                  // predefined class 
                "",//formatString(gParams->ctl[n].label), // control text
                gParams->ctl[n].style,          // styles
                r.left,                         // starting x position 
                r.top,                          // starting y position 
                r.right,                        // control width 
                r.bottom,                       // control height 
				fmc.hDlg, //gParams->ctl[n].hBuddy2, //getAppTheme()?gParams->ctl[n].hBuddy2:fmc.hDlg, //fmc.hDlg,                       // parent window 