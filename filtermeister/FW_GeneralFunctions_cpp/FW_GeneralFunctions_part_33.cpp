{
    int result=0;
    int buY=0;
    TEXTMETRIC tm;
    HDC hdc = GetDC(fmc.hDlg);
    HFONT hDlgFont = (HFONT) SendMessage(fmc.hDlg, WM_GETFONT, 0, 0L);
    HFONT hFontOld = (HFONT)SelectObject(hdc,hDlgFont);

	if (v==0) return 0;

    if (GetTextMetrics(hdc,&tm)){
        
        buY = (WORD)tm.tmHeight;

        //Convert to DBU
        if (buY != 0) result =  (v * 8) / buY;          
    }

    SelectObject(hdc,hFontOld);
    ReleaseDC(fmc.hDlg, hdc);

    return result;
}


int setCtlMate(int n, int s){

    if (n < 0 || n >= N_CTLS) return FALSE;
    if (s < 0 || s >= N_CTLS) return FALSE;

    gParams->ctl[n].mateControl = s;

    return true;
}

int getCtlMate(int n){

    if (n < 0 || n >= N_CTLS) return FALSE;

    return gParams->ctl[n].mateControl;
}



int setCtlTab(int n, int t, int s)
{
	if (n < 0 || n >= N_CTLS || !gParams->ctl[n].inuse) {
        return FALSE; //not a valid control
    }
    
    if (t == -1) {
        // disassociate this control from any Tab control
	    gParams->ctl[n].tabControl = t;
	    gParams->ctl[n].tabSheet = s;
        //setCtlColor(n, -1); //set background color back to transparent???
        sub_enableCtl(n); //automatically enable or disable the control
#if PLUGINGALAXY
		if (gParams->ctl[n].ctlclass == CC_STATICTEXT || gParams->ctl[n].ctlclass == CC_CHECKBOX)
			setCtlColor(n,-1);
#endif

	#if SKIN
		if (isSkinActive() && gParams->skinStyle>1){
			if (gParams->ctl[n].ctlclass == CC_STATICTEXT){
				setCtlColor(n, -1);
			}
		}
	#endif

        return true;
    }

    if (t < 0 || t >= N_CTLS || !gParams->ctl[t].inuse || (gParams->ctl[t].ctlclass != CC_TAB && gParams->ctl[t].ctlclass != CC_LISTBAR)) {
        return FALSE; // t is not a valid Tab control
    }
	
	gParams->ctl[n].tabControl = t;
	gParams->ctl[n].tabSheet = s;

#if SKIN
	if (!(isSkinActive() && gParams->skinStyle>0))
#endif	
	{

		//Change background color to tab color if theme is active
		if (gParams->ctl[n].ctlclass == CC_CHECKBOX || gParams->ctl[n].ctlclass == CC_RADIOBUTTON || 
		   (gParams->ctl[n].ctlclass == CC_OWNERDRAW && (gParams->ctl[n].properties & CTP_DRAWITEM)) ) 
		   {
			   if ( getAppTheme() && (gParams->ctl[t].theme==1  || (gParams->ctl[t].theme==-1 && gParams->gDialogTheme==1)) ){
				   setCtlColor(n,getCtlColor(t));
			   }
		}
		
		//Make Groupbox text back like tab sheet
		if (gParams->ctl[n].ctlclass == CC_GROUPBOX){
			//Info ("%d: %d,%d,%d",n,Rval(getCtlColor(gParams->ctl[n].tabControl)),Gval(getCtlColor(gParams->ctl[n].tabControl)),Bval(getCtlColor(gParams->ctl[n].tabControl)) );
			//if (getAppTheme() && (gParams->ctl[t].theme==1  || (gParams->ctl[t].theme==-1 && gParams->gDialogTheme==1)) ) 
			if (gParams->ctl[t].ctlclass == CC_LISTBAR)
				setCtlColor(n, getCtlColor(gParams->ctl[t].tabControl));
			else
				setCtlColor(n, getCtlColor(t)); //gParams->ctl[n].tabControl
		} else if (gParams->ctl[n].ctlclass == CC_SLIDER ||gParams->ctl[n].ctlclass == CC_TRACKBAR){
			setCtlColor(n, getCtlColor(t));
		}
	}

#if SKIN
	if (isSkinActive() && gParams->skinStyle>1){
		if (gParams->ctl[n].ctlclass == CC_STATICTEXT){
			setCtlColor(n, skinGetColor());
		}
	}
#endif

	//Info ("%d,%d\n%d, %d",t,s,gParams->ctl[t].val,gParams->ctl[t].enabled);

	//Automatically enable or disable control
	sub_enableCtl (n);	//MWVDL 2008-06-17

	return true;

}


int shiftTabSheet(int t, int shiftval){

    int i;

    for (i=0;i<N_CTLS;i++){
        if (gParams->ctl[i].tabControl == t){
            gParams->ctl[i].tabSheet += shiftval;
        }
    }

    return true;
}


int getCtlTab(int n, int t){

    if (n < 0 || n >= N_CTLS) return -1;

    if (t==0) 
        return gParams->ctl[n].tabControl;
    else 
        return gParams->ctl[n].tabSheet;

}

int setCtlDefTab(int n, int t, int s)
{
	gParams->ctl[n].defTabControl = t;
    gParams->ctl[n].defTabSheet = s;
	return true;
}

int getCtlDefTab(int n, int m)
{
	if (m==0)return gParams->ctl[n].defTabControl;
    else return gParams->ctl[n].defTabSheet;
}


int setCtlAnchor(int n, int flags)
{
	int dialogWidth,dialogHeight;
	RECT rcParent;
	
	if ( (n < 0 || n >= N_CTLS || !gParams->ctl[n].inuse)) { //&& n != CTL_PREVIEW && n != CTL_PROGRESS && n != CTL_ZOOM
        return FALSE;
    }

	if (flags == -1) //Use already assigned anchor
        flags = gParams->ctl[n].anchor;
    else
        gParams->ctl[n].anchor = flags;
    
    if (flags==0) return true;


	GetClientRect(fmc.hDlg, &rcParent);
	dialogWidth = PixelsToHDBUs(rcParent.right);
	dialogHeight = PixelsToVDBUs(rcParent.bottom);


/*
	if (n == CTL_ZOOM){ //n == CTL_PREVIEW || n == CTL_PROGRESS || 

        RECT rcCtl;
    
        //if (n == CTL_PREVIEW)
		//	GetWindowRect( GetDlgItem(fmc.hDlg, 101), &rcCtl );
		//else if (n == CTL_PROGRESS)
		//	GetWindowRect( GetDlgItem(fmc.hDlg, IDC_PROGRESS1), &rcCtl );
		//else if (n == CTL_ZOOM)
			GetWindowRect( GetDlgItem(fmc.hDlg, IDC_BUTTON2), &rcCtl );
		
		ScreenToClient (fmc.hDlg, (LPPOINT)&rcCtl);
		ScreenToClient (fmc.hDlg, (LPPOINT)&(rcCtl.right));

		if (flags & 16) //ANCHOR_HCENTER
            gParams->ctl[n].distWidth = PixelsToHDBUs(rcCtl.left) - dialogWidth/2;
        else
            gParams->ctl[n].distWidth = dialogWidth - PixelsToHDBUs(rcCtl.right);

		if (flags & 32) //ANCHOR_VCENTER
		    gParams->ctl[n].distHeight = PixelsToVDBUs(rcCtl.top) - dialogHeight/2;
        else
            gParams->ctl[n].distHeight = dialogHeight - PixelsToVDBUs(rcCtl.bottom);

		gParams->ctl[n].width = PixelsToHDBUs(rcCtl.right - rcCtl.left);
		gParams->ctl[n].height = PixelsToVDBUs(rcCtl.bottom - rcCtl.top);
		gParams->ctl[n].xPos = PixelsToHDBUs(rcCtl.left);
		gParams->ctl[n].yPos = PixelsToVDBUs(rcCtl.top);

	} else {*/

		if (flags & ANCHOR_HCENTER)
            gParams->ctl[n].distWidth = gParams->ctl[n].xPos - dialogWidth/2;
        else
            gParams->ctl[n].distWidth = dialogWidth - (gParams->ctl[n].xPos + gParams->ctl[n].width);

        if (flags & ANCHOR_VCENTER)
		    gParams->ctl[n].distHeight = gParams->ctl[n].yPos - dialogHeight/2;
        else
            gParams->ctl[n].distHeight = dialogHeight - (gParams->ctl[n].yPos + gParams->ctl[n].height);
	//}

	
	return true;
}


int updateAnchors(int dialogWidth, int dialogHeight){

    int n;

    if (!fmc.doingProxy) return false;
    
    
    //Get dialog size if necessary
    if (dialogWidth == -1 || dialogHeight == -1){
        RECT rcParent;
        GetClientRect(fmc.hDlg, &rcParent);
        if (dialogWidth == -1) dialogWidth = PixelsToHDBUs(rcParent.right);
	    if (dialogHeight == -1) dialogHeight = PixelsToVDBUs(rcParent.bottom);
    }


    for (n = 0;  n < N_CTLS;  n++)
    {
		if ( (gParams->ctl[n].inuse) && gParams->ctl[n].anchor > 0){ //|| n==CTL_PREVIEW || n==CTL_PROGRESS || n==CTL_ZOOM

		/*
	        if (n == CTL_ZOOM){ //n == CTL_PREVIEW || n == CTL_PROGRESS || 
		        RECT rcCtl;
    
                //if (n == CTL_PREVIEW)
			    //    GetWindowRect( GetDlgItem(fmc.hDlg, 101), &rcCtl );
		        //else if (n == CTL_PROGRESS)
			    //    GetWindowRect( GetDlgItem(fmc.hDlg, IDC_PROGRESS1), &rcCtl );
		        //else if (n == CTL_ZOOM)
			        GetWindowRect( GetDlgItem(fmc.hDlg, IDC_BUTTON2), &rcCtl );
		        
		        ScreenToClient (fmc.hDlg, (LPPOINT)&rcCtl);
		        ScreenToClient (fmc.hDlg, (LPPOINT)&(rcCtl.right));

		        if (gParams->ctl[n].anchor & 16) //ANCHOR_HCENTER
                    gParams->ctl[n].distWidth = PixelsToHDBUs(rcCtl.left) - dialogWidth/2;
                else
                    gParams->ctl[n].distWidth = dialogWidth - PixelsToHDBUs(rcCtl.right);

		        if (gParams->ctl[n].anchor & 32) //ANCHOR_VCENTER
		            gParams->ctl[n].distHeight = PixelsToVDBUs(rcCtl.top) - dialogHeight/2;
                else
                    gParams->ctl[n].distHeight = dialogHeight - PixelsToVDBUs(rcCtl.bottom);

	        } else { */
		
		        if (gParams->ctl[n].anchor & ANCHOR_HCENTER)
                    gParams->ctl[n].distWidth = gParams->ctl[n].xPos - dialogWidth/2;
                else
                    gParams->ctl[n].distWidth = dialogWidth - (gParams->ctl[n].xPos + gParams->ctl[n].width);

                if (gParams->ctl[n].anchor & ANCHOR_VCENTER)
		            gParams->ctl[n].distHeight = gParams->ctl[n].yPos - dialogHeight/2;
                else
                    gParams->ctl[n].distHeight = dialogHeight - (gParams->ctl[n].yPos + gParams->ctl[n].height);
	        //}
		}
	}

    return true;

}

int updateAnchor(int n, int dialogWidth, int dialogHeight){

    if (!fmc.doingProxy) return false;
    