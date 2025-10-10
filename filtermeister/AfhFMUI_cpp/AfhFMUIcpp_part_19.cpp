            {
                // relay this mouse message to the tooltip control...
                MSG msg; 

                msg.lParam = lParam; 
                msg.wParam = wParam; 
                msg.message = wMsg; 
                msg.hwnd = hDlg; 
                SendMessage(ghTT, TTM_RELAYEVENT, 0, (LPARAM) &msg); 
            }

            {
                int x = (SHORT) LOWORD(lParam);
                int y = (SHORT) HIWORD(lParam);


                if (x >= gProxyRect.left && x < gProxyRect.right &&
                    y >= gProxyRect.top && y < gProxyRect.bottom) // && gParams->ctl[CTL_PREVIEW].inuse
                {

                    //Merged by Harald Heim, May 20, 2003
                    if (ClickDrag == 1)
                    {
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
                        
                        gParams->flags |= AFH_DRAGGING_PROXY;
                        UpdateProxy(hDlg, FALSE);  // To display raw input.
                        return TRUE; //??
                    } 
                    else 
                    {
                        //send FME_RIGHTCLICKED_DOWN event for Proxy
                        //SetCursor(hcHand);
                        PerformControlAction(
                            CTL_PREVIEW,   //control index
                            FME_RIGHTCLICKED_DOWN,           //FM event code
                            0,                      //previous value
                            wMsg,                   //message type
                            0,                      //notification code
                            FALSE,                  //fActionable
                            TRUE,0);                  //fProcessed
                      
                    }

                } else {

                    //Right mouse button on dialog background
                    PerformControlAction(
                        CTL_BACKGROUND,   //control index
                        FME_CONTEXTMENU,           //FM event code
                        0,                      //previous value
                        wMsg,                   //message type
                        0,                      //notification code
                        FALSE,                  //fActionable
                        TRUE,0);                  //fProcessed

                }   
            }

            return FALSE;
            break;


        case WM_RBUTTONUP:

            //Added by Harald Heim, Dec 13, 2002
            {
                // relay this mouse message to the tooltip control...
                MSG msg; 

                msg.lParam = lParam; 
                msg.wParam = wParam; 
                msg.message = wMsg; 
                msg.hwnd = hDlg; 
                SendMessage(ghTT, TTM_RELAYEVENT, 0, (LPARAM) &msg); 
            }

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
                        if (gFmcp->x_start != 0 || gFmcp->y_start != 0 || gFmcp->x_end != gFmcp->X || gFmcp->y_end != gFmcp->Y){
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
								gFmcp->filterRect32 = GetFilterRect();	//needed??
								scaledFilterRect = gFmcp->filterRect32;
                                scaleRect32(&scaledFilterRect, 1, gFmcp->scaleFactor);
								gFmcp->inRect32 = GetInRect();
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



                    if (x >= gProxyRect.left && x < gProxyRect.right &&
                        y >= gProxyRect.top && y < gProxyRect.bottom)
                    {
                        //send FME_RIGHTCLICKED_UP event for Proxy
                        //SetCursor(hcHand);
                        PerformControlAction(
                            CTL_PREVIEW,   //control index
                            FME_RIGHTCLICKED_UP,           //FM event code
                            0,                      //previous value
                            wMsg,                   //message type
                            0,                      //notification code
                            FALSE,                  //fActionable
                            TRUE,1);                  //fProcessed

                    }

                //}//gParams->ctl[CTL_PREVIEW].inuse
            }

            return FALSE;
            break;
            //End of Added by Harald Heim, Dec 13, 2002



        case WM_MBUTTONDOWN:
        case WM_MBUTTONUP:
            {
                // relay this mouse message to the tooltip control...
                MSG msg; 

                msg.lParam = lParam; 
                msg.wParam = wParam; 
                msg.message = wMsg; 
                msg.hwnd = hDlg; 
                SendMessage(ghTT, TTM_RELAYEVENT, 0, (LPARAM) &msg); 
            }
            return FALSE;
            break;
#endif

#if 0
        // Doesn't work -- Dlg eats all key codes???
        case WM_KEYDOWN:          
			

            

			if (wParam == VK_F12) {
                // Toggle the editor.
                SendMessage(hDlg, WM_COMMAND, IDC_EDIT, 0);
                return 0;   //processed
            }
            
            

            return 1;   //not processed
            break;
#endif

        case WM_MOUSEWHEEL:
            {
                int zDelta = (short) HIWORD(wParam);
                int oldvalue;
                int i;
                HWND focusHWND = GetFocus();
                
                for (i = 0; i < N_CTLS; ++i) {

                    if (gParams->ctl[i].inuse && !(gParams->ctl[i].enabled != 3) ){
                    
                        if (gParams->ctl[i].hCtl == focusHWND) {

                            if (gParams->ctl[i].ctlclass == CC_STANDARD ||
                                gParams->ctl[i].ctlclass == CC_SCROLLBAR ||
                                gParams->ctl[i].ctlclass == CC_TRACKBAR ||
								gParams->ctl[i].ctlclass == CC_SLIDER){
                    
                                oldvalue = gParams->ctl[i].val;
        
                                if (zDelta<0)
                                    gParams->ctl[i].val += gParams->ctl[i].pageSize;
                                else
                                    gParams->ctl[i].val -= gParams->ctl[i].pageSize;

    
                                setCtlVal(i, gParams->ctl[i].val);

                                PerformControlAction(
                                                i,                   //control index
                                                FME_CHANGED,			//FM event code
                                                oldvalue,               //previous value
                                                wMsg,                   //message type
                                                0,                      //notification code
                                                FALSE,                  //fActionable
                                                TRUE,0);                  //fProcessed  