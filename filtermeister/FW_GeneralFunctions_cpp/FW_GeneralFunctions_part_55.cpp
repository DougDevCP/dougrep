                    UpdateWindow(GetDlgItem(fmc.hDlg, IDC_BUDDY1+n));
    #endif
                } //if CC_STANDARD
            //}//if
            break;
        // Commented out by Ognen Genchev. Fixed above.
        // case CC_TRACKBAR:
        //     if (prevValue != originalValue) {
        //         //update the actual control if it changed
            
        //         SendMessage(gParams->ctl[n].hCtl, TBM_SETPOS, TRUE /*set*/, val);
        //     }
        //     break;

        case CC_CHECKBOX:
            val &= 3;
            if (prevValue != val) {
                //update the control if it changed
                if ((int)SendMessage(gParams->ctl[n].hCtl, BM_GETCHECK, 0, 0) != val) {
                    //update the control itself if it was not already
                    //automatically updated...
                    SendMessage(gParams->ctl[n].hCtl, BM_SETCHECK, val, 0);
                }
            }
            break;

        //case CC_PUSHBUTTON: //Allow pushbuttons to have more than two values
        case CC_RADIOBUTTON:
            val &= 1;
            if (prevValue != val) {
                //update the control if it changed
                HWND nextCtl;
                int next;
                HWND thisCtl = gParams->ctl[n].hCtl;
                int nIter;
                switch (gParams->ctl[n].style & BS_TYPEMASK) {
                case BS_AUTORADIOBUTTON:
                // added by Ognen Genchev
                case BS_OWNERDRAW:
                // end
                    if ((int)SendMessage(thisCtl, BM_GETCHECK, 0, 0) != val) {
                        //update the control itself if it was not already
                        //automatically updated...
                        SendMessage(thisCtl, BM_SETCHECK, val, 0);
                    }
                    //for an auto radio button, set all other radio buttons
                    //in the current group to 0 if this button is being set
                    //to non-zero.
                    nIter = 0;
                    nextCtl = GetNextDlgGroupItem(fmc.hDlg, thisCtl, FALSE /*next*/);
                    while (nextCtl != NULL && nextCtl != thisCtl) {
                        if (++nIter > N_CTLS) {
                            // can't be more than N_CTLS user controls --
                            // we must be looping, so exit fast...
                            break;
                        }
                        next = GetDlgCtrlID(nextCtl) - IDC_CTLBASE;
                        if (next >= 0 && next < N_CTLS) {
                            if (gParams->ctl[next].ctlclass == CC_RADIOBUTTON) {
                                if (gParams->ctl[next].val != 0) {
                                    //set it to zero... (don't call setCtlVal to do
                                    //this, to avoid infinite recursion!!!!)
                                    gParams->ctl[next].val = 0;
                                    fmc.pre_ctl[next] = 0; //update cached value
                                    if ((int)SendMessage(nextCtl, BM_GETCHECK, 0, 0) != 0) {
                                        //update the control itself if it was not already
                                        //automatically updated...
                                        SendMessage(nextCtl, BM_SETCHECK, 0, 0);
                                    }
                                }//if
                            }//if
                        }
                        else {
                            //not a valid user control??? Get out quick!!
                            break;
                        }//if
                        nextCtl = GetNextDlgGroupItem(fmc.hDlg, nextCtl, FALSE /*next*/);
                    }//while
                    break;
                case BS_GROUPBOX:
                    //nothing to update for this control...
                    break;
                default:
                    //unconditionally update all other controls...
                    SendMessage(thisCtl, BM_SETCHECK, val, 0);
                    break;
                }//switch button subtype
            }//if value changed
            break;

        case CC_LISTBOX:
        case CC_LISTBAR:
            if (gParams->ctl[n].style & (LBS_MULTIPLESEL | LBS_EXTENDEDSEL)) {
                //Must use LB_SELITEMRANGEEX for multiple selection list boxes...
                SendMessage(gParams->ctl[n].hCtl, LB_SELITEMRANGEEX, val, val);
            }
            else {
                //Use LB_SETCURSEL for single-selection list boxes...
                val = (int)SendMessage(gParams->ctl[n].hCtl, LB_SETCURSEL, val, 0);
            }

			//Makes listbar work like a tab control
			gParams->ctl[n].val = val;
			fmc.pre_ctl[n] = val;
			if (gParams->ctl[n].ctlclass == CC_LISTBAR){
				if (val!=-1){
					int i;
					for (i = 0; i < N_CTLS; ++i) {
						if (gParams->ctl[i].inuse && gParams->ctl[i].tabControl == n &&  gParams->ctl[i].tabSheet != -1){
							sub_enableCtl(i);
						}	
					}
				}
			}

            break;

		case CC_LISTVIEW:
			if (val<0){//Deselect
				ListView_SetItemState(gParams->ctl[n].hCtl,fmc.pre_ctl[n],0, LVIS_SELECTED|LVIS_FOCUSED);
			} else {
				ListView_SetItemState(gParams->ctl[n].hCtl,val,LVIS_SELECTED|LVIS_FOCUSED, LVIS_SELECTED|LVIS_FOCUSED);
			}
			break;

        case CC_COMBOBOX:
            val = (int)SendMessage(gParams->ctl[n].hCtl, CB_SETCURSEL, val, 0);
            break;

	    case CC_TAB:
		    if (gParams->ctl[n].ctlclass == CC_TAB) TabCtrl_SetCurSel (gParams->ctl[n].hCtl,val);
		    
		    gParams->ctl[n].val = val;		//MWVDL Uncommented
		    fmc.pre_ctl[n] = val; //update cached value
		    //triggerEvent(n,FME_CLICKED,0);

		    if (val!=-1){
			    int i;
			    for (i = 0; i < N_CTLS; ++i) {
				    if (gParams->ctl[i].inuse && gParams->ctl[i].tabControl == n &&  gParams->ctl[i].tabSheet != -1){
					    sub_enableCtl(i);	//MWVDL 2008-07-18
				    }	
			    }
		    }
			
		    break;

        case CC_OWNERDRAW:
        case CC_STATICTEXT:
        case CC_FRAME:
        case CC_RECT:
        case CC_BITMAP:
        case CC_ICON:
        case CC_METAFILE:
        case CC_GROUPBOX:
        default:
            break;
        } //switch class

    }


    //CHECK FOR IN RANGE FOR ALL CONTROLS!!!

    gParams->ctl[n].val = val;
    fmc.pre_ctl[n] = val; //update cached value

    //if (n==39) Info ("setCtlVal: ctl(%d)=%d",n,fmc.pre_ctl[n]);

    return prevValue;
} /*fm_setCtlVal*/



int setCtlDivisor(int n, int div)
{
    if (n < 0 || n >= N_CTLS || !gParams->ctl[n].inuse) {
        return -1;
    }

    if (gParams->ctl[n].ctlclass == CC_STANDARD || gParams->ctl[n].ctlclass == CC_SLIDER) {
    
        if (div >= 1  && div <= 1000){
            gParams->ctl[n].divisor = div;
            // Update the control display if necessary
            setCtlVal(n, gParams->ctl[n].val);
            return true;
        }
    }

    return false;

} /*fm_setCtlDivisor*/



int getCtlDivisor(int n)
{
    if (n < 0 || n >= N_CTLS || !gParams->ctl[n].inuse) {
        return 1;
    }

    if (gParams->ctl[n].ctlclass == CC_STANDARD || gParams->ctl[n].ctlclass == CC_SLIDER) {
		return gParams->ctl[n].divisor;
    }

    return 1;

} /*fm_getCtlDivisor*/



int setCtlGamma(int n, int gamma)
{
    if (n < 0 || n >= N_CTLS || !gParams->ctl[n].inuse) {
        return -1;
    }

    if (gParams->ctl[n].ctlclass == CC_STANDARD || gParams->ctl[n].ctlclass == CC_SLIDER) {
    
        if (gamma >= 1  && gamma <= 1000){
            gParams->ctl[n].gamma = gamma;
            // Update the control display if necessary
            setCtlVal(n, gParams->ctl[n].val);
            return true;
        }
    }

    return false;

} /*fm_setCtlGamma*/



int setCtlProperties(int n, int props)
{
    int prevProps;
    if (n != CTL_PREVIEW && (n < 0 || n >= sizeof(gParams->ctl)/sizeof(gParams->ctl[0]) ||
        !gParams->ctl[n].inuse) ) {
        return -1;
    }
    prevProps = gParams->ctl[n].properties;
    gParams->ctl[n].properties |= props;
    if (gParams->ctl[n].ctlclass == CC_COMBOBOX &&
        gParams->ctl[n].properties & CTP_EXTENDEDUI) {
        // Set extended keyboard UI
        SendMessage(gParams->ctl[n].hCtl, CB_SETEXTENDEDUI, TRUE, 0);
    }
    return prevProps;
} /*fm_setCtlProperties*/

int clearCtlProperties(int n, int props)
{
    int prevProps;
    if (n < 0 || n >= sizeof(gParams->ctl)/sizeof(gParams->ctl[0]) ||
        !gParams->ctl[n].inuse) {
        return -1;
    }
    prevProps = gParams->ctl[n].properties;
    gParams->ctl[n].properties &= ~props;
    if (gParams->ctl[n].ctlclass == CC_COMBOBOX &&
        !(gParams->ctl[n].properties & CTP_EXTENDEDUI)) {
        // Reset extended keyboard UI
        SendMessage(gParams->ctl[n].hCtl, CB_SETEXTENDEDUI, FALSE, 0);
    }
    return prevProps;
} /*fm_clearCtlProperties*/

int setCtlRange(int n, int lo, int hi)
{   /* returns TRUE if success, else FALSE */
    int retVal = 0; //assume the worst
    if (n < 0 || n >= N_CTLS || !gParams->ctl[n].inuse) {
        return retVal;
    }
    gParams->ctl[n].minval = lo;
    gParams->ctl[n].maxval = hi;

    //update the actual control range
    switch (gParams->ctl[n].ctlclass) {
    case CC_STANDARD:
    case CC_SCROLLBAR:
        if (gParams->ctl[n].minval > gParams->ctl[n].maxval) {
            //range is inverted...
            retVal= SetScrollRange(gParams->ctl[n].hCtl,
                                   SB_CTL,
                                   gParams->ctl[n].maxval,
                                   gParams->ctl[n].minval,
                                   TRUE /*redraw*/);
        }
        else {
            //normal range (min <= max)
            retVal= SetScrollRange(gParams->ctl[n].hCtl,
                                   SB_CTL,
                                   gParams->ctl[n].minval,
                                   gParams->ctl[n].maxval,
                                   TRUE /*redraw*/);
        }
        break;

	case CC_SLIDER: