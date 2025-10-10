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