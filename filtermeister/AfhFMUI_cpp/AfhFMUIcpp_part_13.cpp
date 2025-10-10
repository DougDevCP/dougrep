
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