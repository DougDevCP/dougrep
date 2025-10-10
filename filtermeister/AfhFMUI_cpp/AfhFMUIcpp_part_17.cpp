										iCtl,  //control index
										FME_TABRESET,            //FM event code
										0,		//previous value
										wMsg,               //message type
										iCmd,               //notification code
										FALSE,
										TRUE,0);
									
									if (previewUpdate) doAction(CA_PREVIEW);
*/

								}
							}

							selchange = false;

						} else if (getCtlClass(iCtl)==CC_LISTVIEW){

							LPNMITEMACTIVATE lpnmitem = (LPNMITEMACTIVATE) lParam;
							val =  lpnmitem->iItem;

							oldvalue = gParams->ctl[iCtl].val;
							setCtlVal (iCtl, val);
							doEvents(); //Avoid selection delay

							return PerformControlAction(
								iCtl,  //control index
								FME_CLICKED,            //FM event code
								oldvalue, //previous value
								wMsg,               //message type
								iCmd,               //notification code
								TRUE,
								TRUE,0);	
							
						}

						break;
					}

					case NM_RCLICK:

						if (getCtlClass(iCtl)==CC_LISTVIEW){

							LPNMITEMACTIVATE lpnmitem = (LPNMITEMACTIVATE) lParam;
							int val =  lpnmitem->iItem;
							int oldvalue;

							oldvalue = gParams->ctl[iCtl].val;
							setCtlVal (iCtl, val);
							doEvents(); //Avoid selection delay

							return PerformControlAction(
								iCtl,  //control index
								FME_CLICKED,            //FM event code
								oldvalue, //previous value
								wMsg,               //message type
								iCmd,               //notification code
								TRUE,
								TRUE,0);	

							/*if (gParams->ctl[iCtl].inuse && (gParams->ctl[iCtl].properties & CTP_CONTEXTMENU) && !(gParams->ctl[iCtl].enabled != 3) ){

								PerformControlAction(
                                    iCtl,   //control index
                                    FME_CONTEXTMENU,        //FM event code
                                    0,                      //previous value
                                    wMsg,                   //message type
                                    0,                      //notification code
                                    FALSE,                  //fActionable
                                    TRUE,0);                  //fProcessed
							}*/

						}
					
						break;

					case TBN_DROPDOWN:
						{
							LPNMTOOLBAR lpnmtb = (LPNMTOOLBAR) lParam;

							gParams->ctl[iCtl].val = lpnmtb->iItem;
							fmc.pre_ctl[iCtl] = lpnmtb->iItem;

							return PerformControlAction(
								iCtl,				//control index
								FME_CONTEXTMENU,	//FM event code
								0,					//previous value
								wMsg,               //message type
								iCmd,               //notification code
								FALSE,
								TRUE,0);

						}
						break;

					case TBN_HOTITEMCHANGE:
						{
							LPNMTBHOTITEM lpnmhi = (LPNMTBHOTITEM) lParam;
							//TBBUTTON tbButton;
							//int n_buttons = SendMessage(gParams->ctl[iCtl].hCtl, TB_BUTTONCOUNT, 0, 0); 
							//int i, active=0;

							if (gParams->ctl[iCtl].properties & CTP_MOUSEOVER){

								PerformControlAction(
                                    iCtl,					//control index
                                    FME_MOUSEOVER,          //FM event code
                                    lpnmhi->idNew,          //previous value
                                    wMsg,                   //message type
                                    0,                      //notification code
                                    FALSE,                  //fActionable
                                    TRUE,0);                  //fProcessed

							}

							//if (lpnmhi->idNew!=0) Info ("%d",lpnmhi->idNew);

							/*
							if ( gParams->ctl[iCtl].data !=-1){
								gParams->ctl[iCtl].data = -1;
								return PerformControlAction(
									iCtl,  //control index
									FME_CONTEXTMENU,  //FM event code
									lpnmhi->idNew,     //previous value
									wMsg,               //message type
									iCmd,               //notification code
									FALSE,
									TRUE,0);
							}*/

							/*
							gParams->ctl[iCtl].val = lpnmhi->idNew;

							return PerformControlAction(
								iCtl,  //control index
								FME_CONTEXTMENU,  //FM event code
								lpnmhi->idNew,     //previous value
								wMsg,               //message type
								iCmd,               //notification code
								FALSE,
								TRUE,0);*/

							//SendMessage(gParams->ctl[iCtl].hCtl,TB_SETHOTITEM,-1,0);
							//SendMessage(gParams->ctl[iCtl].hCtl,TB_SETHOTITEM,lpnmhi->idNew,0);

							/*
							for (i=0;i<n_buttons;i++){
								SendMessage(gParams->ctl[iCtl].hCtl,TB_GETBUTTON, i, (LPARAM)&tbButton);
								if (tbButton.fsState == TBSTATE_MARKED) {active=true; break;}
							}

							if (lpnmhi->idNew>0) Info ("Highlighted: %d\nOld: %d\nactive: %d",lpnmhi->idNew,lpnmhi->idOld,active);
							*/

						}
						break;

//#if 0
					case LVN_ITEMCHANGED:
					{
						int oldvalue = gParams->ctl[iCtl].val;
						int val = ListView_GetNextItem(gParams->ctl[iCtl].hCtl,-1, LVNI_FOCUSED); //LVNI_SELECTED
						
/*
						#define LVHT_EX_GROUP_HEADER       0x10000000
						#define LVHT_EX_GROUP_FOOTER       0x20000000
						#define LVHT_EX_GROUP_COLLAPSE     0x40000000
						#define LVHT_EX_GROUP_BACKGROUND   0x80000000
						#define LVHT_EX_GROUP_STATEICON    0x01000000
						#define LVHT_EX_GROUP_SUBSETLINK   0x02000000
						#define LVHT_EX_GROUP              (LVHT_EX_GROUP_BACKGROUND | LVHT_EX_GROUP_COLLAPSE | LVHT_EX_GROUP_FOOTER | LVHT_EX_GROUP_HEADER | LVHT_EX_GROUP_STATEICON | LVHT_EX_GROUP_SUBSETLINK)
						#define ListView_HitTestEx(hwndLV, pinfo) \
								(int)SNDMSG((hwndLV), LVM_HITTEST, (WPARAM)-1, (LPARAM)(LV_HITTESTINFO *)(pinfo))
	                        
						LVHITTESTINFO pinfo;
						POINT coords;
						RECT rcCtl;
						POINT pt;

						GetWindowRect(gParams->ctl[iCtl].hCtl, &rcCtl);
						GetCursorPos(&pt);
						pinfo.pt.x = pt.x-rcCtl.left;
						pinfo.pt.y = pt.y-rcCtl.top;
						pinfo.flags = LVHT_EX_GROUP;
						ListView_HitTestEx(gParams->ctl[iCtl].hCtl,&pinfo);
						if (pinfo.iGroup!=0)
							Info ("Item: %d\nGroup: %d",pinfo.iItem,pinfo.iGroup);
						
						//SetWindowLong(gParams->ctl[124].hCtl, GWL_STYLE, GetWindowLong(gParams->ctl[124].hCtl, GWL_STYLE) & ~WS_VSCROLL); 
						//ShowScrollBar(gParams->ctl[124].hCtl,SB_VERT,TRUE);
*/

						
						if (val != oldvalue && val != -1){
							//Only if key is used, otherwise it may be triggered twice
							if (getAsyncKeyState(VK_LBUTTON)>=0 && getAsyncKeyState(VK_RBUTTON)>=0){

								SetFocus (gParams->ctl[iCtl].hCtl); //Make sure that focus is on list view
								setCtlVal (iCtl, val);	
								doEvents(); //Avoid selection delay

								return PerformControlAction(
									iCtl,  //control index
									FME_CLICKED,            //FM event code
									oldvalue, //previous value
									wMsg,               //message type
									iCmd,               //notification code
									TRUE,
									TRUE,0);
							}
						}

					}
					break;
//#endif
				}
            
				return TRUE;
            }
			
			case WM_LBUTTONDOWN:
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
                    SendMessage(ghTT, TTM_RELAYEVENT, 0, (LPARAM) &msg); 
                }
#endif

				if (x >= gProxyRect.left && x < gProxyRect.right &&
                    y >= gProxyRect.top && y < gProxyRect.bottom)
                {
                    
                    //if (gParams->ctl[CTL_PREVIEW].inuse){
                        //Merged by Harald Heim, May 20, 2003
                        if (ClickDrag == 0)
                        {
                    
                            //Allow Shift click with hand tool
						#if defined(COLORWASHER) || defined(FOCALBLADE) || defined(NOISECONTROL)

							if (getAsyncKeyState(VK_SHIFT)<0){

								PerformControlAction(
									CTL_PREVIEW,   //control index
									FME_LEFTCLICKED_DOWN,   //FM event code
									0,                      //previous value
									wMsg,                   //message type
									0,                      //notification code
									FALSE,                  //fActionable
									TRUE,0);                  //fProcessed

							} else {

						#endif


								SetCapture(hDlg);
								SetCursor(LoadCursor(NULL, IDC_SIZEALL));
								ptPrev.x = x;
								ptPrev.y = y;
		#ifndef NOOPTIMZE_QUICK_TOGGLE
								fMouseMoved = 0;
		#endif
								fDragging = TRUE;
                                fDrawing = false;

								if (gParams->ctl[CTL_PREVIEW].properties & CTP_PREVIEWDRAG){
									PerformControlAction(
										CTL_PREVIEW,   //control index
										FME_PREVIEWDRAG,        //FM event code
										0,                      //previous value
										wMsg,                   //message type
										0,                      //notification code
										FALSE,                  //fActionable
										TRUE,0);                  //fProcessed
								}


						#if defined(COLORWASHER) || defined (FOCALBLADE) || defined(NOISECONTROL)
							}
						#endif

						
                        
                            gParams->flags |= AFH_DRAGGING_PROXY;
                            UpdateProxy(hDlg, FALSE);  // To display raw input.
                            return TRUE; //??
                        }
                        else
                        {