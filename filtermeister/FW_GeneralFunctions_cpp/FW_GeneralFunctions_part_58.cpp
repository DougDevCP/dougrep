{
    int retval;

    pixelUnit=1;
    retval = setCtlPos(n,x,y,w,h);
    pixelUnit=0;

    return retval;

} /*fm_setCtlPixelPos*/


int getCtlPixelPos(int n, int w)
{
    RECT rcCtl;

	if ( n < 0 || n >= N_CTLS || !gParams->ctl[n].inuse ) {
        return FALSE;
    }

    GetWindowRect( gParams->ctl[n].hCtl, &rcCtl );
	ScreenToClient (fmc.hDlg, (LPPOINT)&rcCtl);
	ScreenToClient (fmc.hDlg, (LPPOINT)&(rcCtl.right));

	switch (w){
		case 0: return rcCtl.left;
		case 1: return rcCtl.top;
		case 2: return (rcCtl.right - rcCtl.left);
		case 3: return (rcCtl.bottom - rcCtl.top);
	}

	return 0;
	
} /*fm_setCtlPixelPos*/

// setCtlEditPos() by Ognen Genchev.
int setCtlEditPos(int n, int x, int y)
{
    if (gParams->ctl[n].ctlclass == CC_STANDARD || gParams->ctl[n].ctlclass == CC_SLIDER) {
        RECT r1,r2;
        UINT flags = SWP_NOZORDER | SWP_NOACTIVATE | SWP_NOCOPYBITS;

        gParams->ctl[n].editXpos = r1.left = x; //If these two lines are not used -> Crash under 98 and ME //eh?? garbage for MapDialogRect??-afh
        gParams->ctl[n].editYpos = r1.top = y;
        r2.right = 0;
        r2.bottom = 0;
        
        MapDialogRect(fmc.hDlg, &r2);

        //reposition the numedit control
        computeBuddy1Pos(n, gParams->ctl[n].xPos, gParams->ctl[n].yPos, gParams->ctl[n].width, gParams->ctl[n].height, x, y, &r2);

        //Assign new width and height
        // r2.left = r1.left;
        // r2.top = r1.top;

        SetWindowPos(gParams->ctl[n].hBuddy1, NULL,
                        r2.left,    //x
                        r2.top,     //y
                        r2.right,   //w
                        r2.bottom,  //h
                        flags);

        return true;

    } //if CC_STANDARD

    return false;
} /*fm_setCtlEditPos*/
// end

int setCtlEditSize(int n, int w, int h)
{

    if (gParams->ctl[n].ctlclass == CC_STANDARD || gParams->ctl[n].ctlclass == CC_SLIDER) {
        RECT r1,r2;
        UINT flags = SWP_NOZORDER | SWP_NOACTIVATE | SWP_NOCOPYBITS;

        r1.left = 0; //If these two lines are not used -> Crash under 98 and ME
        r1.top = 0;
        gParams->ctl[n].editwidth =  r1.right = w;
        gParams->ctl[n].editheight = r1.bottom = h;
        
        MapDialogRect(fmc.hDlg, &r1);

        //reposition the numedit control
        computeBuddy1Pos(n, gParams->ctl[n].xPos, gParams->ctl[n].yPos, gParams->ctl[n].width, gParams->ctl[n].height, NULL, NULL, &r2);

        //Assign new width and height
        r2.right = r1.right;
        r2.bottom = r1.bottom;

        SetWindowPos(gParams->ctl[n].hBuddy1, NULL,
                        r2.left,    //x
                        r2.top,     //y
                        r2.right,   //w
                        r2.bottom,  //h
                        flags);

        return true;

    } //if CC_STANDARD

    return false;

} /*fm_setCtlEditBox*/



int setEditBoxUpdate(int ms)
{   

    EditBoxUpdate = ms;

    return true;
}



int getCtlRange(int n, int w)
{   
    
    if (n==CTL_PROGRESS){
        
        if (fmc.doingProxy) {
            if (w==0)
                return (int)SendMessage(GetDlgItem(MyHDlg, IDC_PROGRESS1), PBM_GETRANGE, TRUE, 0);
            else
                return (int)SendMessage(GetDlgItem(MyHDlg, IDC_PROGRESS1), PBM_GETRANGE, FALSE, 0);
            }

    } else if ( n >= 0 && n < N_CTLS && gParams->ctl[n].inuse){
        
        if (w==0)
            return gParams->ctl[n].minval;
        else
            return gParams->ctl[n].maxval;
    }

    return 0;            

}



/**********************************************************************/
/* setCtlAction(n, a)
/*
/*  Sets the default action for control n to a, where a is CA_NONE,
/*  CA_CANCEL, CA_APPLY, CA_PREVIEW, CA_EDIT, CA_ABOUT, or CA_RESET.
/*  Returns the previous default action for the control (or 0 if the
/*  control index is invalid).
/*
/*  The default action is the action that will be taken if the default
/*  OnCtl handler is invoked for control n.  The meaning of each action
/*  is:
/*
/*  CA_NONE     No action.  This is the default action for radio buttons.
/*
/*  CA_CANCEL   FM exits, leaving the original source image unaltered.
/*              This is the default action for the Cancel button,
/*
/*  CA_APPLY    The filter is applied to the original source image,
/*              and FM exits.  This is the default action for the OK
/*              button.
/*
/*  CA_PREVIEW  The filter is applied to the proxy image, and all previews
/*              are updated.  This is the default action for most user
/*              controls, including the STANDARD, SCROLLBAR, TRACKBAR,
/*              PUSHBUTTON, and CHECKBOX controls.
/*
/*  CA_EDIT     FM enters or leaves Edit Mode.  This is the default
/*              action for the Edit control.
/*
/*  CA_ABOUT    FM displays the ABOUT dialog box.
/*
/*  CA_RESET    Resets all user controls to their initial values.
/*
/**********************************************************************/
int setCtlAction(int n, int a)
{
    int prev_action;

    if (n < 0 || n >= N_CTLS || !gParams->ctl[n].inuse) {
        return 0;
    }

    prev_action = gParams->ctl[n].action;
    gParams->ctl[n].action = (CTLACTION)a;
    return prev_action;
} /*fm_setCtlAction*/

/**********************************************************************/
/* enableCtl(n, level)
/*
/*  Determines whether control n is visible and whether it is enabled
/*  by specifying level = 0 (invisible and disabled), 1 (visible but
/*  disabled), or 3 (visible and enabled).  Returns the previous enable
/*  level, or 0 if n is not a valid control index.
/*
/*  NOTE: Due to an early documentation error, level=-1 is considered
/*        equivalent to level=3.  However, other bits in level are
/*        reserved for future use, and should not be randomly set!
/*
/**********************************************************************/
int EnableMode = 0;

void setEnableMode(int state){
	//bit 0: Ignore tab state
	//bit 1: Do not deactivate tab controls
	EnableMode = state;
	//Info ("%d\nbit 0:%d\nbit1: %d",EnableMode,EnableMode & 1,!(EnableMode & 2));
}

int enableCtl(int n, int level)
{
    int prev_level;

    if (n < 0 || n >= N_CTLS) return 0;   //avoid array indexing error

    if (level == -1) level = 3; // see documentation note above

    prev_level = gParams->ctl[n].enabled;
    gParams->ctl[n].enabled = level & 3;    //clear reserved bits for now

    sub_enableCtl(n);

    return prev_level;
} /*enableCtl*/

/**********************************************************************/
/* enableCtl(n) support function
/*
/*  Determines whether a control should be visible, disabled or 
/*	invisible and sets it as such. Returns 0.
/*
/**********************************************************************/
int sub_enableCtl(int n)
{
    int nIDDlgItem;
	int level;

    if ( (n < 0 || n >= N_CTLS || !gParams->ctl[n].inuse) || n == CTL_PREVIEW ) { // && n != CTL_PROGRESS && n != CTL_FRAME && n != CTL_ZOOM
        return 0;
    }

	if (n == CTL_PROGRESS) nIDDlgItem = IDC_PROGRESS1;
    else if (n == CTL_FRAME) nIDDlgItem = IDC_PROXY_GROUPBOX;
    else if (n == CTL_ZOOM) nIDDlgItem = 498; //Zoom Label
	//else if (n == CTL_PREVIEW) nIDDlgItem = 101;
    else  nIDDlgItem = IDC_CTLBASE+n;

	//MWVDL 2008-07-18    
    if (EnableMode & 1)
		level = gParams->ctl[n].enabled;
	else
		level = ctlEnabledAs(n);
		

	EnableWindow(GetDlgItem(fmc.hDlg, nIDDlgItem), (level & 2) != 0);
	ShowWindow(GetDlgItem(fmc.hDlg, nIDDlgItem), (level & 1)?SW_SHOW:SW_HIDE);

	if (n == CTL_ZOOM){
		EnableWindow(GetDlgItem(fmc.hDlg, IDC_BUTTON1), (level & 2) != 0);
		ShowWindow(GetDlgItem(fmc.hDlg, IDC_BUTTON1), (level & 1)?SW_SHOW:SW_HIDE);
		EnableWindow(GetDlgItem(fmc.hDlg, IDC_BUTTON2), (level & 2) != 0);
		ShowWindow(GetDlgItem(fmc.hDlg, IDC_BUTTON2), (level & 1)?SW_SHOW:SW_HIDE);
	} else if (gParams->ctl[n].ctlclass == CC_STANDARD || gParams->ctl[n].ctlclass == CC_SLIDER) {
		EnableWindow(GetDlgItem(fmc.hDlg, IDC_BUDDY1+n), (level & 2) != 0);
		EnableWindow(GetDlgItem(fmc.hDlg, IDC_BUDDY2+n), (level & 2) != 0);
		ShowWindow(GetDlgItem(fmc.hDlg, IDC_BUDDY1+n), (level & 1)?SW_SHOW:SW_HIDE);
		ShowWindow(GetDlgItem(fmc.hDlg, IDC_BUDDY2+n), (level & 1)?SW_SHOW:SW_HIDE);
		//SendMessage(GetDlgItem(fmc.hDlg, IDC_BUDDY1+n), WM_NCPAINT, 1, 0); //Draw edit box frame
		//RedrawWindow(GetDlgItem(fmc.hDlg, IDC_BUDDY1+n),NULL,NULL,RDW_FRAME|RDW_INVALIDATE|RDW_ERASENOW|RDW_UPDATENOW);
		//InvalidateRect(GetDlgItem(fmc.hDlg, IDC_BUDDY1+n), NULL, TRUE);
		//RedrawWindow(GetDlgItem(fmc.hDlg, IDC_BUDDY1+n), NULL, NULL, RDW_FRAME | RDW_UPDATENOW | RDW_NOCHILDREN);
	} else if (gParams->ctl[n].ctlclass == CC_LISTBAR || gParams->ctl[n].ctlclass == CC_COMBOBOX){
		EnableWindow(GetDlgItem(fmc.hDlg, IDC_BUDDY2+n), (level & 2) != 0);
		ShowWindow(GetDlgItem(fmc.hDlg, IDC_BUDDY2+n), (level & 1)?SW_SHOW:SW_HIDE);
	}

	if (!(EnableMode & 2)){
		if (gParams->ctl[n].ctlclass == CC_TAB || gParams->ctl[n].ctlclass == CC_LISTBAR){		//MWVDL 2008-08-17: Without the level-check, disabling of controls on the tab should be possible.
			int i;
			for (i = 0; i < N_CTLS; ++i) {
				if (gParams->ctl[i].inuse								// Only process children of this tab.
				 && gParams->ctl[i].tabControl == n
				 && gParams->ctl[i].tabSheet == gParams->ctl[n].val) {
					sub_enableCtl(i);	//MWVDL 2008-06-17
				}
			}
		}
	}
    
	return 0;

} /*enableCtl*/

/**********************************************************************/
/* deleteCtl(n)