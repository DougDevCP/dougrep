                        
                        gParams->ctl[idd - IDC_BUDDY1].val = iVal;

                        /*PerformControlAction(
                                idd - IDC_BUDDY1,   //control index
                                FME_EDITCHANGED,        //FM event code
                                iVal,                      //previous value
                                wMsg,                   //message type
                                0,                      //notification code
                                FALSE,                  //fActionable
                                TRUE,0);                  //fProcessed
                        */

                        if (!fTranslated) {
                            //bad syntax ... force an illegal value so setCtlVal()      //or just set to 0???
                            //will fix it up...
                            iVal = -9845783;  //black magic
                            fTranslated = TRUE;
                        }
                        if (fTranslated) {
                        
                            //Added by Harald Heim, May 22, 2003
                            int oldvalue = gParams->ctl[idd - IDC_BUDDY1].val;

                            //set value of associated control
                            //(calling setCtlVal() may be dangerous overkill,
                            //possibly even resulting in an infinite loop???)
                            setCtlVal(idd - IDC_BUDDY1, iVal);


                        
                            //Added by Harald Heim, May 22, 2003
                            PerformControlAction(
                                idd - IDC_BUDDY1,       //control index
                                FME_CHANGED,//FME_VALUECHANGED,       //FM event code
                                oldvalue,               //previous value
                                wMsg,                   //message type
                                0,                      //notification code
                                FALSE,                  //fActionable
                                TRUE,0);                  //fProcessed                                        


                            if (!fDragging) {
                                if (EditBoxUpdate==1){
                                    //Update Preview immediately
                                    doAction(gParams->ctl[idd - IDC_BUDDY1].action);

                                } else {
                                    //Delayed update via timer
                                    iCtl2 = idd - IDC_BUDDY1;//Store control number for timer
                                    if (!SetTimer(hDlg, 1001, EditBoxUpdate, NULL)) //Update max. every 20 ms
                                        doAction(gParams->ctl[idd - IDC_BUDDY1].action);
                                }
                            }

                        }
                        changed = FALSE;

                    }

                }
                else if (iCmd == EN_SETFOCUS) {
                    changed = FALSE;
                    
                    if (gParams->ctl[idd - IDC_BUDDY1].properties & CTP_MOUSEOVER) {
                        
                        PerformControlAction(
                                idd - IDC_BUDDY1,   //control index
                                FME_SETFOCUS,			//FM event code
                                0,                      //previous value
                                wMsg,                   //message type
                                0,                      //notification code
                                FALSE,                  //fActionable
                                TRUE,0);                  //fProcessed
                    }

                }
                else if (iCmd == EN_KILLFOCUS ||
                         iCmd == EN_MAXTEXT) {
                    BOOL fTranslated;
                    int iVal;

                    if (iCmd == EN_MAXTEXT) {
                        //presumably because user hit RETURN
                        changed = TRUE;
                    }

                    if (iCmd == EN_KILLFOCUS && (gParams->ctl[idd - IDC_BUDDY1].properties & CTP_MOUSEOVER)) {
                        
                        PerformControlAction(
                                idd - IDC_BUDDY1,   //control index
                                FME_KILLFOCUS,          //FM event code
                                0,                      //previous value
                                wMsg,                   //message type
                                0,                      //notification code
                                FALSE,                  //fActionable
                                TRUE,0);                  //fProcessed
                    }

                    //update only if the NUMEDIT control actually changed!
                    //(e.g., keep track of EN_CHANGE notices).
                    //Otherwise, switching to another app when a NUMEDIT
                    //has the focus will cause an unwarranted filter run...
                    if (!changed) return TRUE; //???


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

                    //Info("iVal = %d\nfTranslated = %d", iVal, fTranslated);

                    if (!fTranslated) {
                        //bad syntax ... force an illegal value so setCtlVal()      //or just set to 0???
                        //will fix it up...
                        iVal = -9845783;  //black magic
                        fTranslated = TRUE;
                    }
                    if (fTranslated) {
                        
                        //Added by Harald Heim, May 22, 2003
                        int oldvalue = gParams->ctl[idd - IDC_BUDDY1].val;

                        //set value of associated control
                        //(calling setCtlVal() may be dangerous overkill,
                        //possibly even resulting in an infinite loop???)
                        setCtlVal(idd - IDC_BUDDY1, iVal);
                        
                        //Added by Harald Heim, May 22, 2003
                        PerformControlAction(
                            idd - IDC_BUDDY1,       //control index
                            FME_CHANGED,       		//FM event code
                            oldvalue,               //previous value
                            wMsg,                   //message type
                            0,                      //notification code
                            FALSE,                  //fActionable
                            TRUE,0);                  //fProcessed                                        


                        if (!fDragging) {
                            if (gResult == noErr) {
                                // Do Filtering operation
                                DoFilterRect (globals);
                                // Invalidate Proxy Item
                                UpdateProxy(hDlg, FALSE);
                            }
                        }
                    }
                    changed = FALSE;
#if 0
                    return SetDlgMsgResult(hDlg, wMsg, TRUE);
#else
                    return TRUE; //so EN_MAXTEXT won't ding the bell -- ineffective???
#endif
                }
#if 0
                else {
                    //unexpected notification????
                    static int counter = 0;
                    static int wmsg_save = 0;
                    static int wparam_save = 0;
                    static int lparam_save = 0;
                    static int idd_save = 0;
                    static int cmd_save = 0;
                    wmsg_save = wMsg;
                    wparam_save = wParam;
                    lparam_save = lParam;
                    idd_save = idd;
                    cmd_save = iCmd;
                    counter++;
                    return TRUE;
                }
#endif
                return TRUE;   //???
            }

            if (idd >= IDC_CTLBASE && idd < IDC_CTLBASE + N_CTLS) {
                int iVal = 0;
				int n = idd - IDC_CTLBASE;
				int success = 0;

                //Added by Harald Heim
                int oldvalue=0;

                BOOL fProcessed = TRUE;
                BOOL fActionable = FALSE;
                FME fmeCode = FME_UNKNOWN;


#if SKIN
				//Skinned Controls
				if (isSkinActive() && isSkinCtl(n)){
					if (iCmd == BN_CLICKED) {
						fmeCode = FME_CLICKED;
						switch (gParams->ctl[n].ctlclass) {
						case CC_CHECKBOX:
							oldvalue = gParams->ctl[n].val;
							if (gParams->ctl[n].val==BST_UNCHECKED) iVal = BST_CHECKED; else iVal = BST_UNCHECKED;
							gParams->ctl[n].val = fmc.pre_ctl[n] = iVal;
							skinDraw(n,1);
							success = 1;
							break;
						case CC_PUSHBUTTON:
						case CC_COMBOBOX:
							oldvalue = gParams->ctl[n].val;
							skinDraw(n,1);
							success = 1;
							break;
						case CC_TOOLBAR:
							skinDraw(n,1);
							return TRUE;
						case CC_STANDARD:
							{
								oldvalue = gParams->ctl[n].val;
								if (!skinSetSlider(n)) return FALSE;
								
								PerformControlAction(
									n,                   //control index
									FME_CHANGED,       		//FM event code
									oldvalue,               //previous value
									wMsg,                   //message type
									0,                      //notification code
									FALSE,                  //fActionable
									TRUE,0);                  //fProcessed                                        
								
								//Update preview
								//if ( ((gParams->ctl[n].properties & CTP_TRACK) != 0) ^ (GetKeyState(VK_SHIFT) < 0) ){
									KillTimer(hDlg, 1001); //Cancel any previous trigger.
									iCtl2 = n;
									//Perform the default action for this control.
									if (!SetTimer(hDlg, 1001, 20, NULL)){ //Update max. every 20 ms
										doAction(gParams->ctl[n].action);
									}
								//}
								return TRUE; 
								break;
							}
						}//switch
					}
				}
#endif

				// It's a user control...
				if (success==0){
					switch (gParams->ctl[idd - IDC_CTLBASE].ctlclass) {
					case CC_CHECKBOX:
					case CC_RADIOBUTTON:
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
						// NOTE that BN_CLICKED == STN_CLICKED!!!
						if (iCmd == BN_CLICKED) {
							//From a button or static class control...
							static int reentry_depth = 0;
							static int reentry_count = 0;

							//check for reentry...
							if (reentry_depth++ > 0) {
								//reentered!!! Log the fact, and exit quickly...
								reentry_count++;
								reentry_depth--;
	#if 0
								return FALSE;   //do default processing anyway??
	#else
								return TRUE;    //no default processing either
	#endif
							}

							fmeCode = FME_CLICKED;

							switch (gParams->ctl[idd - IDC_CTLBASE].ctlclass) {
							case CC_CHECKBOX:
							case CC_RADIOBUTTON: