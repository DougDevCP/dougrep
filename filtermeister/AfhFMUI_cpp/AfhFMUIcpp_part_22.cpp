								#ifdef _WIN64
									iVal = (int)SendMessage(gParams->ctl[idd - IDC_CTLBASE].hCtl, BM_GETCHECK, 0, 0); //(HWND)(UINT_PTR)wParam
								#else
									iVal = SendMessage(COMMANDWND(wParam), BM_GETCHECK, 0, 0);
								#endif
								switch (gParams->ctl[idd - IDC_CTLBASE].style & BS_TYPEMASK) {
								case BS_CHECKBOX:
								case BS_AUTOCHECKBOX:
								case BS_3STATE:
								case BS_AUTO3STATE:
								case BS_AUTORADIOBUTTON:
                                //added by Ognen Genchev
                                case BS_OWNERDRAW:
                                // end
									//Added by Harald Heim, May 22, 2003
									oldvalue = gParams->ctl[idd - IDC_CTLBASE].val;
	                        
									setCtlVal(idd - IDC_CTLBASE, iVal);
									break;
								case BS_RADIOBUTTON:
									//setCtlVal causes reentry????
									//Need to update pre_ctl[] and ctl[].val????
									break;
								}//switch button subtype
								break;
							case CC_PUSHBUTTON:
							case CC_OWNERDRAW:
							case CC_STATICTEXT:
							case CC_FRAME:
							case CC_RECT:
							case CC_IMAGE:
							case CC_BITMAP:
							case CC_ICON:
							case CC_METAFILE:
							case CC_GROUPBOX:
								//these classes don't maintain their own value...
								//iVal = 1;
								//setCtlVal(idd - IDC_CTLBASE, iVal);
								break;
							default:
								break;
							}//switch class

							 reentry_depth--; //release reentry lock

						}//BN_CLICKED
						else {
							switch (gParams->ctl[idd - IDC_CTLBASE].ctlclass) {
							case CC_CHECKBOX:
							case CC_RADIOBUTTON:
							case CC_PUSHBUTTON:
							case CC_GROUPBOX:
							case CC_OWNERDRAW:
								//Button-style controls...
								if (iCmd == BN_DBLCLK) {
									fProcessed = FALSE;     //not processed
									fActionable = FALSE;    //for now
									fmeCode = FME_DBLCLK;
								}
								/*else if (iCmd == BN_PUSHED) {
									fProcessed = FALSE;     //not processed
									fActionable = FALSE;    //for now
									fmeCode = FME_LEFTCLICKED_DOWN;
								}
								else if (iCmd == BN_UNPUSHED) {
									fProcessed = FALSE;     //not processed
									fActionable = FALSE;    //for now
									fmeCode = FME_LEFTCLICKED_UP;
								}*/
								else {
									fProcessed = FALSE;     //not processed
									fActionable = FALSE;    //for now
									fmeCode = FME_UNKNOWN;
								}
								break;
							case CC_STATICTEXT:
							case CC_FRAME:
							case CC_RECT:
							case CC_IMAGE:
							case CC_BITMAP:
							case CC_ICON:
							case CC_METAFILE:
								//Static-style controls...
								if (iCmd == STN_DBLCLK) {
									fProcessed = FALSE;     //not processed
									fActionable = FALSE;    //for now
									fmeCode = FME_DBLCLK;
								}
								else {
									fProcessed = FALSE;     //not processed
									fActionable = FALSE;    //for now
									fmeCode = FME_UNKNOWN;
								}
								break;
							}//switch
						}//if !BN_CLICKED
						break;

					case CC_LISTBOX:
					case CC_LISTBAR:
						if (iCmd == LBN_SELCHANGE) {
	                        
							int n = idd - IDC_CTLBASE;

							//Added by Harald Heim, May 22, 2003
							oldvalue = gParams->ctl[idd - IDC_CTLBASE].val;
	                        
							#ifdef _WIN64
								iVal = (int)SendMessage(gParams->ctl[n].hCtl, LB_GETCURSEL, 0, 0);
							#else
								iVal = SendMessage(COMMANDWND(wParam), LB_GETCURSEL, 0, 0);
							#endif
							setCtlVal(n, iVal);
	                        
							if (gParams->ctl[n].ctlclass == CC_LISTBAR && gParams->ctl[n].mateControl != -1){
								char text[256];
								getCtlItemText(n,ctl(n),&text[0]);
								setCtlVal(gParams->ctl[n].mateControl,atoi(&text[0]));
							}

							//Makes listbar work like a tab control
							if (gParams->ctl[n].ctlclass == CC_LISTBAR){
								int i;
								for (i = 0; i < N_CTLS; ++i) {
									if (gParams->ctl[i].inuse && gParams->ctl[i].tabControl == n &&  gParams->ctl[i].tabSheet != -1){
										sub_enableCtl(i);
									}	
								}
							}
							
							fmeCode = FME_CLICKED;  //for now

						}
						else if (iCmd == LBN_DBLCLK) {
							//Requires LBS_NOTIFY
							fProcessed = FALSE;     //not processed
							fActionable = FALSE;    //for now
							fmeCode = FME_DBLCLK;
						}
						else {
							fProcessed = FALSE;   //not processed
						}
						break;

					case CC_COMBOBOX:
	#if defined(_DEBUG) && 0
						{   //debug combobox events
							char sz[256];
							static int iSeq = 0;
							iSeq++;
							sprintf(sz, "CB event %.4d: Ctl = %d, Cmd = %d (%s)\n",
								iSeq,
								idd - IDC_CTLBASE,
								iCmd,
								iCmd==CBN_ERRSPACE      ?"CBN_ERRSPACE":
								iCmd==CBN_SELCHANGE     ?"CBN_SELCHANGE":
								iCmd==CBN_DBLCLK        ?"CBN_DBLCLK":
								iCmd==CBN_SETFOCUS      ?"CBN_SETFOCUS":
								iCmd==CBN_KILLFOCUS     ?"CBN_KILLFOCUS":
								iCmd==CBN_EDITCHANGE    ?"CBN_EDITCHANGE":
								iCmd==CBN_EDITUPDATE    ?"CBN_EDITUPDATE":
								iCmd==CBN_DROPDOWN      ?"CBN_DROPDOWN":
								iCmd==CBN_CLOSEUP       ?"CBN_CLOSEUP":
								iCmd==CBN_SELENDOK      ?"CBN_SELENDOK":
								iCmd==CBN_SELENDCANCEL  ?"CBN_SELENDCANCEL":
								"???"
								);
							OutputDebugString(sz);
						}
	#endif
						if (iCmd == CBN_SELENDOK /*||
							iCmd == CBN_SELCHANGE*/) {
	                        
							oldvalue = gParams->ctl[idd - IDC_CTLBASE].val;
	                        
							#ifdef _WIN64
								iVal = (int)SendMessage(gParams->ctl[idd - IDC_CTLBASE].hCtl, CB_GETCURSEL, 0, 0);
							#else
								iVal = SendMessage(COMMANDWND(wParam), CB_GETCURSEL, 0, 0);
							#endif
							setCtlVal(idd - IDC_CTLBASE, iVal);
							fmeCode = FME_CLICKED;  //for now
						}
						else if (iCmd == CBN_DBLCLK) {
							//Can only occur for CBS_SIMPLE...
							fProcessed = FALSE;     //not processed
							fActionable = FALSE;    //for now
							fmeCode = FME_DBLCLK;
						}
						else if (iCmd == CBN_DROPDOWN) {
							fProcessed = FALSE;     //not processed
							fActionable = FALSE;    //for now
							fmeCode = FME_COMBO_DROPDOWN;
						}
						else if (iCmd == CBN_CLOSEUP) {
							fProcessed = FALSE;     //not processed
							fActionable = FALSE;    //for now
							fmeCode = FME_COMBO_CLOSEUP;
						}
						else {
							fProcessed = FALSE;   //not processed
						}
						break;

					case CC_EDIT:				
						
						if (iCmd == EN_CHANGE) {
							
							GetDlgItemText(hDlg, idd, gParams->ctl[idd - IDC_CTLBASE].label,1024);
							
							oldvalue = (int)strlen(gParams->ctl[idd - IDC_CTLBASE].label);
							fmeCode = FME_CHANGED;//FME_VALUECHANGED;
						
						} else if (iCmd == EN_SETFOCUS) {

							//fmeCode = FME_CLICKED;
							fmeCode = FME_SETFOCUS;//FME_SETEDITFOCUS;
							
						} else if (iCmd == EN_KILLFOCUS) {

							fmeCode = FME_KILLFOCUS;//FME_KILLEDITFOCUS;
						
						} else if (iCmd == EN_MAXTEXT){

							fmeCode = FME_CHANGED;//FME_VALUECHANGED;
						}

						fProcessed = FALSE;     //not processed
						fActionable = FALSE;    //for now

						break;

					default:
						fProcessed = FALSE;   //not processed
					}//switch class

				}//if (success==0)


#ifdef SCRIPTABLE
                //Set script values before CTL_OK event, so that the user can still modify them
                if (idd - IDC_CTLBASE == CTL_OK && fmeCode == FME_CLICKED){
                        int i;
                        for (i=0;i<N_SCRIPTITEMS;i++) {
                           ScriptParam[i] = gFmcp->pre_ctl[i]; //gParams->ctl[i].val
                           ScriptParamUsed[i] = gParams->ctl[i].inuse && gParams->ctl[i].scripted;
                        }
                }
#endif


                fActionable = fProcessed;   //for now
                return PerformControlAction(
                    idd - IDC_CTLBASE,  //control index
                    fmeCode,            //FM event code
                    oldvalue,           //previous value
                    wMsg,               //message type
                    iCmd,               //notification code
                    fActionable,
                    fProcessed,0);
            }//user control



            switch  (idd) {

#if OLDZOOMLABEL || TESTSHELL

              case IDC_BUTTON2: // -
				#ifdef HIGHZOOM 
					if (scaleFactor==1 && gFmcp->enlargeFactor>1)
						gFmcp->enlargeFactor -= 2;
					else
				#endif
						scaleFactor += 2;
                    // Fall through...
              case IDC_BUTTON1: // +
				#ifdef HIGHZOOM   
					if (scaleFactor==1 && gFmcp->enlargeFactor<16)
						gFmcp->enlargeFactor++;
					else
				#endif
						scaleFactor--;

                    if (GetKeyState(VK_SHIFT) < 0) {
                        //SHIFT modifies meaning of +/-
                        if (scaleFactor < prevScaleFactor) {
                            // Shift + means zoom 100 %
                            scaleFactor = 1;
							gFmcp->enlargeFactor = 1;
							
                        }
                        else {
							#ifdef HIGHZOOM 
							if (scaleFactor == 1 && gFmcp->enlargeFactor == 15){
								gFmcp->enlargeFactor = 1;
							} else if (scaleFactor == 1){
								gFmcp->enlargeFactor = 16;
							} else {
							#endif