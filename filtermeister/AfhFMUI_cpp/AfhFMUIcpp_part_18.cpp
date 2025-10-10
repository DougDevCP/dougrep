                            //send FME_LEFTCLICKED_DOWN event for Proxy
                            SetCursor(hcHand);
                            PerformControlAction(
                                CTL_PREVIEW,   //control index
                                FME_LEFTCLICKED_DOWN,   //FM event code
                                0,                      //previous value
                                wMsg,                   //message type
                                0,                      //notification code
                                FALSE,                  //fActionable
                                TRUE,0);                  //fProcessed
                        }
                    //}

                }
/*
				else 
				{
                    RECT rcCtl;
                    int i;
                    int D = 0;

					#define LVHT_EX_GROUP_HEADER       0x10000000
					#define LVHT_EX_GROUP_FOOTER       0x20000000
					#define LVHT_EX_GROUP_COLLAPSE     0x40000000
					#define LVHT_EX_GROUP_BACKGROUND   0x80000000
					#define LVHT_EX_GROUP_STATEICON    0x01000000
					#define LVHT_EX_GROUP_SUBSETLINK   0x02000000
					#define LVHT_EX_GROUP              (LVHT_EX_GROUP_BACKGROUND | LVHT_EX_GROUP_COLLAPSE | LVHT_EX_GROUP_FOOTER | LVHT_EX_GROUP_HEADER | LVHT_EX_GROUP_STATEICON | LVHT_EX_GROUP_SUBSETLINK)
					#define ListView_HitTestEx(hwndLV, pinfo) \
							(int)SNDMSG((hwndLV), LVM_HITTEST, (WPARAM)-1, (LPARAM)(LV_HITTESTINFO *)(pinfo))
                    
					for (i = 0; i < N_CTLS; ++i) {
                        if (gParams->ctl[i].inuse  && gParams->ctl[i].enabled == 3 && gParams->ctl[i].ctlclass == CC_LISTVIEW){
                            
							LVHITTESTINFO pinfo;
							POINT coords = {x,y};
							pinfo.pt = coords;
							pinfo.flags = LVHT_EX_GROUP;
							ListView_HitTestEx(gParams->ctl[i].hCtl,&pinfo);
							Info ("Item: %d\nGroup: %d",pinfo.iItem,pinfo.iGroup);
                        }
                    } //for i
                }
*/
                
                /*else
                {
                    //check for hit
                    RECT rcCtl;
                    int i;
                    int D = 0;
                    for (i = 0; i < N_CTLS; ++i) {
 
                        if (gParams->ctl[i].inuse  && gParams->ctl[i].enabled == 3) { //&& gParams->ctl[i].ctlclass == CC_OWNERDRAW
                        
                            //this control needs notifications
                            if (GetWindowRect(gParams->ctl[i].hCtl, &rcCtl)) {
                                ScreenToClient (hDlg, (LPPOINT)&rcCtl);
                                ScreenToClient (hDlg, (LPPOINT)&(rcCtl.right));
                                if (x >= rcCtl.left-D && x < rcCtl.right+D &&
                                    y >= rcCtl.top-D  && y < rcCtl.bottom+D) {
                                    //it's a hit!
                                    
                                    //if (gParams->ctl[i].properties & CTP_MOUSEMOVE){
                                        //this control wants mousemove notifications

                                        PerformControlAction(
                                            i,                      //control index
                                            FME_LEFTCLICKED_DOWN,   //FM event code
                                            0,                      //previous value
                                            wMsg,                   //message type
                                            0,                      //notification code
                                            FALSE,                  //fActionable
                                            TRUE,0);                  //fProcessed

                                    //} 
                                    
                                    break;

                                }
                            }
                            
                        }
                    } //for i
                }*/


            }

            return FALSE;
            break;

        case WM_LBUTTONUP:
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

            {
                int x = (SHORT) LOWORD(lParam);
                int y = (SHORT) HIWORD(lParam);

                //if (gParams->ctl[CTL_PREVIEW].inuse){
                    if (fDragging)
                    {
            
                        if (x >= gProxyRect.left && x < gProxyRect.right &&
                            y >= gProxyRect.top && y < gProxyRect.bottom)
                        {
                            SetCursor(hcHand);
                        }
                        else
                        {
                            //SetCursor(LoadCursor(NULL, IDC_ARROW));  //???
                        }
                        ReleaseCapture();
						fDragging = FALSE;
                        gParams->flags &= ~AFH_DRAGGING_PROXY;
                        ptNew.x = x;
                        ptNew.y = y;
        #ifndef NOOPTIMZE_QUICK_TOGGLE
                        //Only update if image can be dragged in preview
                     #ifndef APP
                        if (gFmcp->x_start != 0 || gFmcp->y_start != 0 || gFmcp->x_end != gFmcp->X || gFmcp->y_end != gFmcp->Y ){
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
                        GetProxyItemRect (hDlg, &gProxyRect); 


                        if ( ((gParams->flags & AFH_ZERO_OUTPUT_DRAG) == AFH_ZERO_OUTPUT_DRAG) && gFmcp->x_start == 0 && gFmcp->y_start == 0 && gFmcp->x_end == gFmcp->X && gFmcp->y_end == gFmcp->Y ){
                        
                             //Temporarily deactivate Zero drag to avoid redraw problem
                             gParams->flags &= ~AFH_ZERO_OUTPUT_DRAG;
                             SetupFilterRecordForProxy (globals, &scaleFactor, scaleFactor, &imageOffset);
                             //Activate Zero drag again
                             gParams->flags |= AFH_ZERO_OUTPUT_DRAG;

                        }else {
                            SetupFilterRecordForProxy (globals, &scaleFactor, scaleFactor, &imageOffset);
                        }


                        if (gResult == noErr) {
                            // Do filter and update proxy...
        #ifndef NOOPTIMZE_QUICK_TOGGLE
                            //if mouse didn't move since LBUTTONDOWN, no need to run filter...
                            //gives a quick before/after preview toggle

                            if (fMouseMoved) {
                                
                                //Setup x_start etc.
#if BIGDOCUMENT
								VRect scaledFilterRect;
                                gFmcp->filterRect32 = GetFilterRect(); //needed??
								scaledFilterRect = gFmcp->filterRect32;
                                scaleRect32(&scaledFilterRect, 1, gFmcp->scaleFactor);
								gFmcp->inRect32 = GetInRect();	//needeed??
                                gFmcp->x_start = gFmcp->inRect32.left - scaledFilterRect.left;
                                gFmcp->y_start = gFmcp->inRect32.top - scaledFilterRect.top;
#else
								Rect scaledFilterRect;
                                copyRect (&scaledFilterRect, &gStuff->filterRect);
                                scaleRect (&scaledFilterRect, 1, gFmcp->scaleFactor);
                                gFmcp->x_start = gStuff->inRect.left - scaledFilterRect.left;
                                gFmcp->y_start = gStuff->inRect.top - scaledFilterRect.top;
#endif

                                gFmcp->x_end = gFmcp->x_start + gFmcp->columns;
                                gFmcp->y_end = gFmcp->y_start + gFmcp->rows;
                            
                                if (gParams->ctl[CTL_PREVIEW].properties & CTP_PREVIEWDRAG){
                                    PerformControlAction(
                                        CTL_PREVIEW,   //control index
                                        FME_PREVIEWDRAG,        //FM event code
                                        1,                      //previous value
                                        wMsg,                   //message type
                                        0,                      //notification code
                                        FALSE,                  //fActionable
                                        TRUE,0);                  //fProcessed
                                }
    
                                DoFilterRect (globals);
                                fMouseMoved = 0;
                            }
        #else
                                DoFilterRect (globals);
        #endif
                        
                            UpdateProxy(hDlg, FALSE);
                        }//noErr
                        return TRUE;
                    }

                    //Info ("WM_LBUTTONUP\nfDragging: %d\nx: %d, y: %d\n%d,%d - %d,%d",fDragging,x,y,gProxyRect.left,gProxyRect.top,gProxyRect.right,gProxyRect.bottom);


                    //Added by Harald Heim, Dec 13, 2002
                    if (x >= gProxyRect.left && x < gProxyRect.right &&
                            y >= gProxyRect.top && y < gProxyRect.bottom)
                        {                           
                            //Info ("Should send FME_LEFTCLICKED_UP now");
                            
                            //send FME_LEFTCLICKED_UP event for Proxy
                            //SetCursor(hcHand);
                            PerformControlAction(
                                CTL_PREVIEW,   //control index
                                FME_LEFTCLICKED_UP,           //FM event code
                                0,                      //previous value
                                wMsg,                   //message type
                                0,                      //notification code
                                FALSE,                  //fActionable
                                TRUE,                   //fProcessed
                                1);                     //override busy logic

                    }

                //}//gParams->ctl[CTL_PREVIEW].inuse
            }

            return FALSE;
            break;


        case WM_CONTEXTMENU:

            {
                int x = (SHORT) LOWORD(lParam);
                int y = (SHORT) HIWORD(lParam);

                RECT rcCtl;
                int i;

                for (i = 0; i < N_CTLS; ++i) {
                    
                    if (gParams->ctl[i].inuse && (gParams->ctl[i].properties & CTP_CONTEXTMENU) && !(gParams->ctl[i].enabled != 3) ){
                    
                        if (GetWindowRect(gParams->ctl[i].hCtl, &rcCtl)) {
                            
                            if (x >= rcCtl.left && x < rcCtl.right &&
                                y >= rcCtl.top  && y < rcCtl.bottom) {
                                //it's a hit!
                                
								if (gParams->ctl[i].ctlclass==CC_LISTVIEW){
										
									if (!IsWindowVisible(gParams->ctl[i].hCtl)) break;

									PerformControlAction(
										i,  //control index
										FME_CLICKED,            //FM event code
										0,		//previous value
										wMsg,               //message type
										0,               //notification code
										TRUE,
										TRUE,0);
								}

								PerformControlAction(
                                    i,   //control index
                                    FME_CONTEXTMENU,        //FM event code
                                    0,                      //previous value
                                    wMsg,                   //message type
                                    0,                      //notification code
                                    FALSE,                  //fActionable
                                    TRUE,0);                  //fProcessed

                                break;
                            }
                        }
                    }
                } //for i

                return false;
            }
            

#if 1
        case WM_RBUTTONDOWN:
            
            //Added by Harald Heim, Aug 9, 2002