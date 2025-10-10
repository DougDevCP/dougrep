                            
                                //Perform the default action for this control.
                            
                                KillTimer(hDlg, 1000); //Cancel any previous trigger.
                                iCtl2 = i;

                                if (GetKeyState(VK_SHIFT) < 0){
                                    if (!SetTimer(hDlg, 1001, 20, NULL)) //Update max. every 20 ms
                                        doAction(gParams->ctl[i].action);
                                } else if (!SetTimer(hDlg, 1000, 500, NULL)) {
                                    doAction(gParams->ctl[i].action);
                                }
                                
                            }
                        } 
                   }

                }
            }
            return 0;   //processed


        case WM_HSCROLL:
        case WM_VSCROLL:

            //handles both scrollbar and trackbar messages
            //(Note that TB_PAGEDOWN == SB_PAGEDOWN, etc.)

            { //Start scope
                int oldvalue; 

                hCtrl = (HWND) lParam;
                iCtl = (int)GetWindowLong(hCtrl, GWL_ID);
                if (iCtl < IDC_CTLBASE || iCtl >= IDC_CTLBASE + N_CTLS)
                    return FALSE;
                iCtl -= IDC_CTLBASE;
                idd = COMMANDID (wParam);
    #if 0
                if (gParams->ctl[iCtl].ctlclass == CC_TRACKBAR) {
                    Info("TB notification %d", idd);
                }
    #endif

                oldvalue = gParams->ctl[iCtl].val;

                switch (idd)
                {
                case SB_PAGEDOWN:
                    gParams->ctl[iCtl].val += gParams->ctl[iCtl].pageSize;
                    break;
                case SB_LINEDOWN:
                    gParams->ctl[iCtl].val += gParams->ctl[iCtl].lineSize;
                    break;
                case SB_PAGEUP:
                    gParams->ctl[iCtl].val -= gParams->ctl[iCtl].pageSize;
                    break;
                case SB_LINEUP:
                    gParams->ctl[iCtl].val -= gParams->ctl[iCtl].lineSize;
                    break;
                case SB_TOP:
                    gParams->ctl[iCtl].val = gParams->ctl[iCtl].minval;
                    break;
                case SB_BOTTOM:
                    gParams->ctl[iCtl].val = gParams->ctl[iCtl].maxval;
                    break;
                case SB_THUMBTRACK:
                case SB_THUMBPOSITION:
                    
					if (gParams->ctl[iCtl].ctlclass == CC_STANDARD || gParams->ctl[iCtl].ctlclass == CC_SCROLLBAR){
						
						SCROLLINFO si;
						ZeroMemory(&si, sizeof(si));
						si.cbSize = sizeof(si);
						si.fMask = SIF_TRACKPOS;

						if (GetScrollInfo(hCtrl, SB_CTL, &si) )
							gParams->ctl[iCtl].val = si.nTrackPos; //get 32bit scroll bar value
						else
							gParams->ctl[iCtl].val = (short)HIWORD(wParam); //get 16bit scroll bar value
                        
					} else { //Trackbar and Slider

						gParams->ctl[iCtl].val = (int)SendMessage(hCtrl, TBM_GETPOS, 0, 0); //get 32bit track bar value
					}
					
                    if (gParams->ctl[iCtl].ctlclass == CC_STANDARD ||
						gParams->ctl[iCtl].ctlclass == CC_SLIDER ||
                        gParams->ctl[iCtl].ctlclass == CC_SCROLLBAR)
                    {
                        if (gParams->ctl[iCtl].minval > gParams->ctl[iCtl].maxval) {
                            //range is inverted...
                            gParams->ctl[iCtl].val = gParams->ctl[iCtl].minval +
                                                     gParams->ctl[iCtl].maxval -
                                                     gParams->ctl[iCtl].val;
                        }
                    } //if
                    break;
                default:
                    return FALSE;
                } /*switch*/
                
                if (oldvalue != gParams->ctl[iCtl].val)
                {
                    
					if ( (idd== SB_THUMBPOSITION || idd == SB_THUMBTRACK) && gParams->ctl[iCtl].thumbSize > 0){

						int range = abs(gParams->ctl[iCtl].maxval - gParams->ctl[iCtl].minval);
						gParams->ctl[iCtl].val = gParams->ctl[iCtl].val * range / (range - gParams->ctl[iCtl].thumbSize-1);

					}					
					
					if ( (idd== SB_THUMBPOSITION || idd == SB_THUMBTRACK) && gParams->ctl[iCtl].gamma != 100){
						gParams->ctl[iCtl].val = gammaCtlVal(iCtl,gParams->ctl[iCtl].val,true);
                    }

					setCtlVal(iCtl, gParams->ctl[iCtl].val);
                  
                
                    //Added by Harald Heim, May 22, 2003
                    PerformControlAction(
                                iCtl,                   //control index
                                FME_CHANGED,       		//FM event code
                                oldvalue,               //previous value
                                wMsg,                   //message type
                                0,                      //notification code
                                FALSE,                  //fActionable
                                TRUE,0);                  //fProcessed                                        
                    
                    
                    /*if (gParams->ctl[iCtl].ctlclass == CC_STANDARD || gParams->ctl[iCtl].ctlclass == CC_SLIDER){
                        if (getAsyncKeyState(VK_LBUTTON) >= 0){
                            SetFocus (gParams->ctl[iCtl].hBuddy1);
                            SendMessage(gParams->ctl[iCtl].hBuddy1, EM_SETSEL, 0, -1);
                        }
                    }*/


                } //if CC_STANDARD || CC_SCROLLBAR

            }//scope


            // Perform default action for THUMBTRACK events only if TRACK property is set
            // (or sense is reversed by SHIFT key).
            if ( idd != SB_THUMBTRACK ||
                 ( (((gParams->ctl[iCtl].properties & CTP_TRACK) != 0) ^(GetKeyState(VK_SHIFT) < 0)) /*&&  fmc.renderTime < 333*/ )
               )
            {
                KillTimer(hDlg, 1001); //Cancel any previous trigger.
                iCtl2 = iCtl;
                //Perform the default action for this control.
                if (!SetTimer(hDlg, 1001, 20, NULL)){ //Update max. every 20 ms
                    doAction(gParams->ctl[iCtl].action);
                }
            }
            return TRUE;

        case WM_COMMAND:
            idd = COMMANDID (wParam); //loword
            iCmd = COMMANDCMD(wParam, lParam); //hiword

			//Key shortcuts
			/*if (iCmd==1){
				//Info ("%d",idd);
				PerformControlAction(
					idd,  //control index
					FME_KEYDOWN,  //FM event code
					0,	//previous value
					wMsg,               //message type
					iCmd,               //notification code
					FALSE,
					TRUE,0);

			}*/

			if (lParam==0 && idd>2){ //Menu items - they have to start with an index of 3!!!

				PerformControlAction(
					idd,  //control index
					FME_MENUCLICKED,  //FM event code
					0,	//previous value
					wMsg,               //message type
					iCmd,               //notification code
					FALSE,
					TRUE,0);

			}else if (idd >= IDC_BUDDY2 && idd < IDC_BUDDY2 + N_CTLS) {
                
                FME fmeCode;
                BOOL fActionable;

                if (iCmd == BN_CLICKED) {
                    fmeCode = FME_CLICKED;
                    fActionable = false;
                } else if (iCmd == STN_DBLCLK) {
                    //fmeCode = FME_DBLCLK;
					fmeCode = FME_CLICKED;
                    
					//Set Default Value
                    if (gParams->ctl[idd - IDC_BUDDY2].defval != 0xffffffff){
                        int oldvalue = gParams->ctl[idd - IDC_BUDDY2].val;
						
						if (GetFocus() != gParams->ctl[idd - IDC_BUDDY2].hBuddy1) SetFocus(gParams->ctl[idd - IDC_BUDDY2].hCtl);
                        
						if (oldvalue != gParams->ctl[idd - IDC_BUDDY2].defval){
							setCtlVal(idd - IDC_BUDDY2, gParams->ctl[idd - IDC_BUDDY2].defval);
							PerformControlAction(
								idd - IDC_BUDDY2,  //control index
								FME_CHANGED,  //FM event code
								oldvalue,	//previous value
								wMsg,               //message type
								iCmd,               //notification code
								FALSE,
								TRUE,0);
							fActionable = true;
						} else
							fActionable = false;
                    }
                }

                if (iCmd == BN_CLICKED || iCmd == STN_DBLCLK) {
                    return PerformControlAction(
                        idd - IDC_BUDDY2,   //control index
                        fmeCode,            //FM event code
                        0,                  //previous value
                        wMsg,               //message type
                        0,                  //notification code
                        fActionable,         
                        TRUE,0);
                }
            }


            if (idd >= IDC_BUDDY1 && idd < IDC_BUDDY1 + N_CTLS) {
                static BOOL changed = FALSE;
				int n = idd - IDC_BUDDY1;
                
#if SKIN
				if (isSkinActive() && isSkinCtl(n)){
					if (iCmd == BN_CLICKED) {
						int iVal,oldvalue;
						switch (gParams->ctl[n].ctlclass) {
						case CC_TAB:
							if (!gParams->ctl[n].enabled) break;
							//Info ("%d",n);
							iVal = skinGetTab(n);
							oldvalue = gParams->ctl[n].val;
							if (iVal>=0 && fmc.pre_ctl[n]!=iVal ){
								gParams->ctl[n].val = fmc.pre_ctl[n] = iVal;
								skinDraw(n,1);
							} else if (fmc.pre_ctl[n]==iVal)
								if (getAsyncKeyState(VK_SHIFT)<=-32767) resetTabCtls(n);

							return PerformControlAction(
								n,  //control index
								FME_CLICKED,            //FM event code
								oldvalue,           //previous value
								wMsg,               //message type
								iCmd,               //notification code
								TRUE,
								TRUE,0);
							break;
						}
					}
				}
#endif				
				
				// NUMEDIT buddy control for a scrollbar
                if (iCmd == EN_CHANGE) {
                    changed = TRUE;
                    
                    if (EditBoxUpdate != 0){

                    //if (gParams->ctl[idd - IDC_BUDDY1].properties & CTP_MOUSEOVER) {
                    
                        BOOL fTranslated;
                        int iVal;
#if 1
                        //Changed by Harald Heim, May 1, 2003
                        if (gParams->ctl[idd - IDC_BUDDY1].divisor == 1) { // Set Integer Value

                            iVal = (int)GetDlgItemInt(hDlg, idd, &fTranslated, TRUE /*signed*/);

                        } else 
#endif
                        { // Set Double Value
                        
                            char  szInput[32];
                            double dVal;
                            char *pInvalid;

                            fTranslated = GetDlgItemText (hDlg, idd, szInput, 32);
                            // WRONG!  strtod() is locale-dependent!!!
                            dVal = strtod( szInput, &pInvalid);     //should check errno, *pInvalid==0 ???
#if 0
                            iVal = (int) (gParams->ctl[idd - IDC_BUDDY1].divisor * dVal);   //should this round???
#else
                            iVal = (int) (gParams->ctl[idd - IDC_BUDDY1].divisor * dVal + (dVal<0.0?-0.5:0.5));
#endif
                        }