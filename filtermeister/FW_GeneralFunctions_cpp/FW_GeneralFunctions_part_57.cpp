/*  unchanged, specify a value of -1 for that parameter.
/*
/**********************************************************************/
int setCtlPos(int n, int x, int y, int w, int h)
{
    RECT r;
    UINT flags = SWP_NOZORDER | SWP_NOACTIVATE | SWP_NOCOPYBITS;

    //added by Harald Heim, Dec 24, 2002
    //if ( (n < 0 || n >= sizeof(gParams->ctl)/sizeof(gParams->ctl[0]) || !gParams->ctl[n].inuse) &&
	if ( (n < 0 || n >= N_CTLS || !gParams->ctl[n].inuse) ) {  //&& n != CTL_PREVIEW  && n != CTL_PROGRESS && n != CTL_FRAME && n != CTL_ZOOM 
        return FALSE;
    }
	

	/*if (n == CTL_PREVIEW || n == CTL_PROGRESS || n == CTL_FRAME || n == CTL_ZOOM){
		if (x < 0 || y < 0) {
			flags |= SWP_NOMOVE;
		}
		if (w < 0 || h < 0) {
			flags |= SWP_NOSIZE;
		}
	}*/


if (pixelUnit==0) {

#if 0
    if (x < 0 || y < 0) {
        flags |= SWP_NOMOVE;
    }
    else {
        gParams->ctl[n].xPos = x;
        gParams->ctl[n].yPos = y;
    }
    if (w < 0 || h < 0) {
        flags |= SWP_NOSIZE;
    }
    else {
        gParams->ctl[n].width = w;
        gParams->ctl[n].height = h;
    }
#else
    if (x >= 0) gParams->ctl[n].xPos = x;
    if (y >= 0) gParams->ctl[n].yPos = y;
    if (w >= 0) gParams->ctl[n].width = w;
    if (h >= 0) gParams->ctl[n].height = h;
#endif

    x = r.left = gParams->ctl[n].xPos;
    y = r.top = gParams->ctl[n].yPos;
    w = r.right = gParams->ctl[n].width;
    h = r.bottom = gParams->ctl[n].height;
	
    MapDialogRect(fmc.hDlg, &r);

} else { //setCtlPixelPos

    if (x<0)
        x = HDBUsToPixels(gParams->ctl[n].xPos);
    else {
        gParams->ctl[n].xPos = PixelsToHDBUs(x);
    }
    r.left = x;
    
    if (y<0)
        y = VDBUsToPixels(gParams->ctl[n].yPos);
    else {
        gParams->ctl[n].yPos = PixelsToVDBUs(y);
    }
    r.top = y;

    if (w<0)
        w = HDBUsToPixels(gParams->ctl[n].width);
    else {
        gParams->ctl[n].width = PixelsToHDBUs(w);
    }
    r.right = w;
    
    if (h<0)
        h = VDBUsToPixels(gParams->ctl[n].height);
    else {
        gParams->ctl[n].height = PixelsToVDBUs(h);
    }
    r.bottom = h;

}


    if (gParams->ctl[n].ctlclass == CC_STANDARD || gParams->ctl[n].ctlclass == CC_SLIDER || gParams->ctl[n].ctlclass == CC_LISTBAR || gParams->ctl[n].ctlclass == CC_COMBOBOX) {
        RECT r2;

        // Commented out by Ognen Genchev
        // if (gParams->ctl[n].ctlclass != CC_LISTBAR && gParams->ctl[n].ctlclass != CC_COMBOBOX){
            //reposition the numedit control
            computeBuddy1Pos(n, x, y, w, h, NULL, NULL, &r2);
            SetWindowPos(gParams->ctl[n].hBuddy1, NULL,
                            r2.left,    //x
                            r2.top,     //y
                            r2.right,   //w
                            r2.bottom,  //h
                            flags);
        // }
            // end

        //reposition the static text control
        computeBuddy2Pos(n, x, y, w, h, &r2);
        SetWindowPos(gParams->ctl[n].hBuddy2, NULL,
                        r2.left,    //x
                        r2.top,     //y
                        r2.right,   //w
                        r2.bottom,  //h
                        flags);
    } //if CC_STANDARD


    //added by Harald Heim, Dec 24, 2002
    if (n == CTL_PREVIEW) {
        
		RECT imageRect;

		if (!unlockedPreviewFrame){

			double factor = 1.0;
			getSysDPI(&factor,0);

			if (gParams->ctl[CTL_FRAME].hCtl==NULL) gParams->ctl[CTL_FRAME].hCtl = GetDlgItem(fmc.hDlg, IDC_PROXY_GROUPBOX);
			
			gParams->ctl[CTL_FRAME].xPos = PixelsToHDBUs(r.left-3);
			gParams->ctl[CTL_FRAME].yPos = PixelsToVDBUs(r.top-(int)(9*factor+0.5));
			gParams->ctl[CTL_FRAME].width = PixelsToHDBUs(r.right+7);
			gParams->ctl[CTL_FRAME].height = PixelsToVDBUs(r.bottom+4+(int)(9*factor+0.5));

			//Auto position frame around preview
			SetWindowPos(gParams->ctl[CTL_FRAME].hCtl, NULL,
								r.left-3,					//x
								r.top-(int)(9*factor+0.5),      //y
								r.right+7,   //w
								r.bottom+4+(int)(9*factor+0.5),   //h
								flags);
		}
        
        // Invalidate Proxy Item
		if (gParams->ctl[CTL_PREVIEW].hCtl==NULL) gParams->ctl[CTL_PREVIEW].hCtl = GetDlgItem(fmc.hDlg, 101);
		
		GetWindowRect(gParams->ctl[CTL_PREVIEW].hCtl, &imageRect);
        ScreenToClient (fmc.hDlg, (LPPOINT)&imageRect);
        ScreenToClient (fmc.hDlg, (LPPOINT)&(imageRect.right));
        InvalidateRect (fmc.hDlg, &imageRect, TRUE);

		//Info ("%d,%d - %d,%d",r.left,r.top,r.right,r.bottom);
		
        //Position preview
        SetWindowPos(gParams->ctl[CTL_PREVIEW].hCtl, NULL,
                            r.left,     //x
                            r.top,      //y
                            r.right,    //w
                            r.bottom,   //h
                            flags);

		//Avoid problems for preview drawing without recalculation
		//But this causes crash for images larger than 2 GB, so deactivate it now !!!
		#ifndef APP
		//	restoreRect();
		#endif

        //Update Preview
        ShowOutputBuffer(globals, MyHDlg, FALSE /*fDragging*/);

        //Make FilterMeister aware of the changed proxy preview
        GetProxyItemRect (MyHDlg, &gProxyRect);

        // TODO: Update tooltip rect if in use...

		if (!fScaling) setCtlAnchor (n, gParams->ctl[n].anchor);

        return 1;

    } else if (n == CTL_ZOOM){

        HWND Control;
        int ret;


        Control = GetDlgItem(fmc.hDlg, IDC_BUTTON2);
        ret = SetWindowPos(Control, NULL,
                        r.left,     //x
                        r.top,      //y
                        HDBUsToPixels(12),    //w
                        VDBUsToPixels(10),   //h
                        flags);
        
        Control = GetDlgItem(fmc.hDlg, 498);
        ret = SetWindowPos(Control, NULL,
                        r.left + HDBUsToPixels(12),     //x
                        r.top,      //y
                        HDBUsToPixels(30),    //w
                        VDBUsToPixels(10),   //h
                        flags);


        Control = GetDlgItem(fmc.hDlg, IDC_BUTTON1);
        ret = SetWindowPos(Control, NULL,
                        r.left + HDBUsToPixels(42),     //x
                        r.top,      //y
                        HDBUsToPixels(12),    //w
                        VDBUsToPixels(10),   //h
                        flags);

		if (!fScaling) setCtlAnchor (n, gParams->ctl[n].anchor);

        return ret;

	} else {

        HWND Control = 0;
		int ret;

#if SKIN
		if (isSkinActive() && gParams->skinStyle>0){
			if (gParams->ctl[n].ctlclass == CC_COMBOBOX) 
				r.bottom = skinGetComboHeight(); //Limit control size
			else if (gParams->ctl[n].ctlclass == CC_TAB){
				ret = SetWindowPos(gParams->ctl[n].hBuddy1, NULL,
                            r.left,     //x
                            r.top,      //y
                            r.right,    //w
                            skinGetTabButtonHeight(),   //h
                            flags);
			}
		}
#endif

        // TODO: Set .hCtl for all predefined controls...

		//Needed for second invocation !!!
		if (gParams->ctl[n].hCtl)
			Control = gParams->ctl[n].hCtl;
        else if (n == CTL_ZOOM) Control = GetDlgItem(fmc.hDlg, IDC_BUTTON1);
        else if (n == CTL_FRAME) Control = GetDlgItem(fmc.hDlg, IDC_PROXY_GROUPBOX);
        else if (n == CTL_PROGRESS) Control = GetDlgItem(fmc.hDlg, IDC_PROGRESS1);
		// //else if (gParams->ctl[n].ctlclass == CC_TOOLBAR) Control = gParams->ctl[n].hBuddy2;
        //else 
		//	Control = gParams->ctl[n].hCtl;

		//if (n == CTL_PROGRESS)
		//	Info ("%d\n%d",Control,gParams->ctl[n].hCtl);

        ret = SetWindowPos(Control, NULL,
                            r.left,     //x
                            r.top,      //y
                            r.right,    //w
                            r.bottom,   //h
                            flags);

		if (!fScaling) setCtlAnchor (n, gParams->ctl[n].anchor);
        else {
            //scaling the control.  Force it to update value
            // (needed for STANDARD and SCROLLBAR controls to
            // keep the flashing highlight in sync with the thumb;
            // probably not needed by other controls, but better
            // safe than sorry..).
            if (GetFocus() == gParams->ctl[n].hCtl && 
                (gParams->ctl[n].ctlclass == CC_SCROLLBAR || gParams->ctl[n].ctlclass == CC_STANDARD) ) 
                    setCtlVal(n, fmc.pre_ctl[n]);
        }

		//Adjust Column
		if (gParams->ctl[n].ctlclass == CC_LISTVIEW){
			LVCOLUMN lvc;
			lvc.mask = LVCF_WIDTH;
			lvc.cx = r.right-22;
			ListView_SetColumn (gParams->ctl[n].hCtl,0,&lvc);
		}

		return ret;

    }

} /*fm_setCtlPos*/

int setCtlDefPos(int n, int x, int y, int width, int height)
{
	if (x>=0) gParams->ctl[n].defXPos = x;
	if (y>=0)  gParams->ctl[n].defYPos = y;
	if (width>=0) gParams->ctl[n].defwidth = width;
    if (height>=0) gParams->ctl[n].defheight = height;
	return true;
}

int getCtlDefPos(int n, int item)
{
	if (item==0) return gParams->ctl[n].defXPos;
	if (item==1) return gParams->ctl[n].defYPos;
	if (item==2) return gParams->ctl[n].defwidth;
	if (item==3) return gParams->ctl[n].defheight;
    return -1;
}

int setCtlPixelPos(int n, int x, int y, int w, int h)