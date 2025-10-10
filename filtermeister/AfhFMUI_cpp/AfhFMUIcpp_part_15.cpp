
					
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