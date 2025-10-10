                            0,                      //notification code
                            FALSE,                  //fActionable
                            TRUE,0);                  //fProcessed
                        iCurrentMouseoverCtl = -1;
                    }

                    return TRUE; // ??
                }

                //Proxy Box
                else if (x >= gProxyRect.left && x < gProxyRect.right &&
                         y >= gProxyRect.top && y < gProxyRect.bottom)
                {
                    
                    //if (gParams->ctl[CTL_PREVIEW].inuse){

						//Info ("%d, %d",GetCursor(),hcHand);
                        SetCursor(hcHand);
                
                        /*   // Why did Harald comment this out????

                             //Found the reason for commenting this out! This code sends a mouseout 
                             //to the preview itself which causes some problems in my FFP code. 
                             //iCurrentMouseoverCtl must be set to the preview control 
                             //at this point already.

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
                            iCurrentMouseoverCtl = -1;
                        }
                        */
                    
                        //Added by Harald Heim, Dec 13, 2002
                        //Send MouseMove for Proxy
                    
                        
                        if (gParams->ctl[CTL_PREVIEW].properties & CTP_MOUSEMOVE){
                            PerformControlAction(
                                CTL_PREVIEW,   //control index
                                FME_MOUSEMOVE,           //FM event code
                                0,                      //previous value
                                wMsg,                   //message type
                                0,                      //notification code
                                FALSE,                  //fActionable
                                TRUE,0);                  //fProcessed
                        }
                    
                    //}


#if 1
                    return TRUE; //??
#else
                    return FALSE; //so underlying controls can see mouse and pass to tooltips???
#endif
                }
                else
                {
                    //check for hit in a MOUSEOVER control
                    RECT rcCtl;
                    int i;
                    int D = 0;
                    int hitTest;
                    int enableState;

                    for (i = 0; i < N_CTLS; ++i) {
                        
                        //Check if control is enabled
                        if (gParams->ctl[i].tabControl>=0){
                            enableState = ctlEnabledAs (i); //Control is on a tab sheet
                        } else {
                           enableState = gParams->ctl[i].enabled;
                        }


                        if (gParams->ctl[i].inuse && ((gParams->ctl[i].properties & CTP_MOUSEMOVE) || (gParams->ctl[i].properties & CTP_MOUSEOVER)) && enableState == 3) {
                        //if (gParams->ctl[i].inuse && (gParams->ctl[i].properties & CTP_MOUSEOVER) && !(gParams->ctl[i].enabled != 3 && MouseOverWhenInvisible == 0)) {
                        //if (gParams->ctl[i].inuse && gParams->ctl[i].enabled == 3 && gParams->ctl[i].properties & CTP_MOUSEOVER) {
                        //if (gParams->ctl[i].inuse && gParams->ctl[i].properties & CTP_MOUSEOVER) {

                            //this control wants mouseover notifications
                            if (GetWindowRect(gParams->ctl[i].hCtl, &rcCtl)) {
                                ScreenToClient (hDlg, (LPPOINT)&rcCtl);
                                ScreenToClient (hDlg, (LPPOINT)&(rcCtl.right));

                                //Mouse event only for tab buttons
                                if (gParams->ctl[i].ctlclass == CC_TAB){
#if SKIN
                                    if (isSkinActive() && gParams->skinStyle>0){
										GetWindowRect(gParams->ctl[i].hBuddy1, &rcCtl);
									} else 
#endif									
									{
										RECT rect;
										TabCtrl_GetItemRect (gParams->ctl[i].hCtl, TabCtrl_GetItemCount(gParams->ctl[i].hCtl)-1, &rect);
										rcCtl.right = rcCtl.left + rect.right-1;
										rcCtl.bottom = rcCtl.top + rect.bottom-1;
									}
                                }                                    
                                            
                                hitTest = x >= rcCtl.left-D && x < rcCtl.right+D &&
                                          y >= rcCtl.top-D  && y < rcCtl.bottom+D;
                                

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
                                    
                                    if (gParams->ctl[i].properties & CTP_MOUSEOVER){
                                    
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
                }
            }
            return FALSE;
            break;

			case WM_NOTIFY:
            {
				iCmd = COMMANDCMD(wParam, lParam);

				hCtrl = ((LPNMHDR)lParam)->hwndFrom;
                iCtl = (int)GetWindowLong(hCtrl, GWL_ID);
                if (iCtl < IDC_CTLBASE || iCtl >= IDC_CTLBASE + N_CTLS)
                    return FALSE;
                iCtl -= IDC_CTLBASE;

                static int selchange = false;
                
                switch (((LPNMHDR)lParam)->code)
				{
					case TCN_SELCHANGE:
					{ 
						int val = TabCtrl_GetCurSel(hCtrl);
						int oldvalue; //i, 
                        
						if (val!=-1) {
							
							selchange = true;
							oldvalue = gParams->ctl[iCtl].val;
							setCtlVal (iCtl, val);

							//already done in setCtlVal()
							/*
							//lockCtl(iCtl);
                           	for (i = 0; i < N_CTLS; ++i) {
								if (gParams->ctl[i].inuse && gParams->ctl[i].tabControl == iCtl &&  gParams->ctl[i].tabSheet != -1){
									sub_enableCtl(i);	//MWVDL 2008-07-18
								}	
							}
							//unlockCtl(iCtl); 
							*/
							
                            return PerformControlAction(
								iCtl,  //control index
								FME_CLICKED,            //FM event code
								oldvalue, //previous value
								wMsg,               //message type
								iCmd,               //notification code
								FALSE,
								TRUE,0);

						}

					} 
					
					case NM_CLICK:
					{

						int val;
						int oldvalue;

						if (gParams->ctl[iCtl].ctlclass==CC_TAB){
							//int i, previewUpdate = false;
							//int subtab;
							//val = TabCtrl_GetCurSel(hCtrl);
						
							//Shift click on tab sheet button
							if (getAsyncKeyState(VK_SHIFT)<=-32767){
								if (!selchange){ //Tab is already active

									resetTabCtls(iCtl);
/*
									for (i = 0; i < N_CTLS; ++i) {

										//Get sub tab
										if (gParams->ctl[i].tabControl > 0) subtab = gParams->ctl[gParams->ctl[i].tabControl].tabControl; else subtab = -1;

										if (gParams->ctl[i].inuse && gParams->ctl[i].defval != -1 && 
											((gParams->ctl[i].enabled == 3 && gParams->ctl[i].tabControl == iCtl &&  gParams->ctl[i].tabSheet == val) || subtab == iCtl)
										){
										
											oldvalue = gParams->ctl[i].val;
											setCtlVal(i,gParams->ctl[i].defval);
											if (gParams->ctl[i].ctlclass == CC_OWNERDRAW) setCtlColor(i,gParams->ctl[i].defval);
											
											if (gParams->ctl[i].ctlclass == CC_CHECKBOX || gParams->ctl[i].ctlclass == CC_COMBOBOX){
												PerformControlAction(
													i,  //control index
													FME_CLICKED,            //FM event code
													oldvalue, //previous value
													wMsg,               //message type
													iCmd,               //notification code
													FALSE,
													TRUE,0);
											}

											if (gParams->ctl[i].action == CA_PREVIEW) previewUpdate = true;
										}
									
									}

									PerformControlAction(