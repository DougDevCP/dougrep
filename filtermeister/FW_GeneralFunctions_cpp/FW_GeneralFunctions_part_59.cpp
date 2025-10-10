/*
/*  Deletes control n.
/*
/**********************************************************************/
int deleteCtl(int n)
{
    if (n < 0 || n >= N_CTLS) {
        //invalid control id
        return FALSE;
    }

    if (n == CTL_PROGRESS || n == CTL_FRAME || n == CTL_ZOOM){
    
        int nIDDlgItem;

        if (n == CTL_PROGRESS) nIDDlgItem = IDC_PROGRESS1;
        else if (n == CTL_FRAME) nIDDlgItem = IDC_PROXY_GROUPBOX;
        else if (n == CTL_ZOOM) nIDDlgItem = 498; //Zoom Lable

        DestroyWindow (GetDlgItem(fmc.hDlg, nIDDlgItem));
        
        if (n == CTL_ZOOM){
            DestroyWindow (GetDlgItem(fmc.hDlg, IDC_BUTTON1));
            DestroyWindow (GetDlgItem(fmc.hDlg, IDC_BUTTON2));
        }

		gParams->ctl[n].enabled = 0;
        
        
    } else if (gParams->ctl[n].inuse) {

        //delete any tool tip
        setCtlToolTip(n, 0, 0);

        //delete any associated image resources
        setCtlImage(n, 0, 0);

        if (gParams->ctl[n].hCtl) {
            //destroy the control
            DestroyWindow(gParams->ctl[n].hCtl);
            gParams->ctl[n].hCtl = NULL;
        }
        if (gParams->ctl[n].hBuddy1) {
            //destroy the first buddy control
            DestroyWindow(gParams->ctl[n].hBuddy1);
            gParams->ctl[n].hBuddy1 = NULL;
			//gParams->ctl[n].buddy1Style = 0;
			//gParams->ctl[n].buddy1StyleEx = 0;
        }
        if (gParams->ctl[n].hBuddy2) {
            //destroy the second buddy control
            DestroyWindow(gParams->ctl[n].hBuddy2);
            gParams->ctl[n].hBuddy2 = NULL;
			//gParams->ctl[n].buddy2Style = 0;
			//gParams->ctl[n].buddy2StyleEx = 0;
        }
        gParams->ctl[n].inuse = FALSE;
    }
    return TRUE;    //success
} /*fm_deleteCtl*/



/**********************************************************************/
/* createPredefinedCtls()
/*
/*  Creates the pre-defined user controls.
/*
/**********************************************************************/
void createPredefinedCtls(void)
{
	RECT rcCtl;

/*  
	//OK button
	createCtl(CTL_OK, CC_PUSHBUTTON, "OK", -1, -1, -1, -1, BS_DEFPUSHBUTTON, 0, 0, 3);
	setCtlAnchor (CTL_OK,ANCHOR_BOTTOM|ANCHOR_RIGHT);
    setCtlScripting (CTL_OK, false);
    
    //Cancel button
	createCtl(CTL_CANCEL, CC_PUSHBUTTON, "Cancel", -1, -1, -1, -1, 0, 0, 0, 3);
	setCtlAnchor (CTL_CANCEL,ANCHOR_BOTTOM|ANCHOR_RIGHT);
    setCtlScripting (CTL_CANCEL, false);
*/
    
    //FM Logo
    //createCtl(CTL_LOGO, CC_IMAGE, 0, -1, -1, -1, -1, SS_NOTIFY, 0, 0, 3);
    //setCtlImage(CTL_LOGO, (INT_PTR)"logo2.bmp", 'B');
    
	// setDefaultWindowTheme(gParams->ctl[CTL_PROGRESS].hCtl);
    // setDefaultWindowTheme(GetDlgItem(MyHDlg, IDC_BUTTON1));
    // setDefaultWindowTheme(gParams->ctl[CTL_ZOOM].hCtl); //Button 2
    // setDefaultWindowTheme(gParams->ctl[CTL_FRAME].hCtl);

	///// gParams->ctl[CTL_PROGRESS].inuse = true;  //breaks PROGRESS bar (scaled twice, created dynamically as well as statically, anything else?)
    //gParams->ctl[CTL_PROGRESS].enabled = 3;


	//Set coordinates for existing controls
	gParams->ctl[CTL_PREVIEW].hCtl = GetDlgItem(fmc.hDlg, 101);
	GetWindowRect( gParams->ctl[CTL_PREVIEW].hCtl, &rcCtl );
	ScreenToClient (fmc.hDlg, (LPPOINT)&rcCtl);
	ScreenToClient (fmc.hDlg, (LPPOINT)&(rcCtl.right));
	gParams->ctl[CTL_PREVIEW].width = PixelsToHDBUs(rcCtl.right - rcCtl.left);
	gParams->ctl[CTL_PREVIEW].height = PixelsToVDBUs(rcCtl.bottom - rcCtl.top);
	gParams->ctl[CTL_PREVIEW].xPos = PixelsToHDBUs(rcCtl.left);
	gParams->ctl[CTL_PREVIEW].yPos = PixelsToVDBUs(rcCtl.top);
	
	gParams->ctl[CTL_FRAME].hCtl = GetDlgItem(fmc.hDlg, IDC_PROXY_GROUPBOX);
	GetWindowRect( gParams->ctl[CTL_FRAME].hCtl, &rcCtl );
	ScreenToClient (fmc.hDlg, (LPPOINT)&rcCtl);
	ScreenToClient (fmc.hDlg, (LPPOINT)&(rcCtl.right));
	gParams->ctl[CTL_FRAME].width = PixelsToHDBUs(rcCtl.right - rcCtl.left);
	gParams->ctl[CTL_FRAME].height = PixelsToVDBUs(rcCtl.bottom - rcCtl.top);
	gParams->ctl[CTL_FRAME].xPos = PixelsToHDBUs(rcCtl.left);
	gParams->ctl[CTL_FRAME].yPos = PixelsToVDBUs(rcCtl.top);
	
	gParams->ctl[CTL_PROGRESS].hCtl = GetDlgItem(fmc.hDlg, IDC_PROGRESS1);
	GetWindowRect( gParams->ctl[CTL_PROGRESS].hCtl, &rcCtl );
	ScreenToClient (fmc.hDlg, (LPPOINT)&rcCtl);
	ScreenToClient (fmc.hDlg, (LPPOINT)&(rcCtl.right));
	gParams->ctl[CTL_PROGRESS].width = PixelsToHDBUs(rcCtl.right - rcCtl.left);
	gParams->ctl[CTL_PROGRESS].height = PixelsToVDBUs(rcCtl.bottom - rcCtl.top);
	gParams->ctl[CTL_PROGRESS].xPos = PixelsToHDBUs(rcCtl.left);
	gParams->ctl[CTL_PROGRESS].yPos = PixelsToVDBUs(rcCtl.top);

	gParams->ctl[CTL_ZOOM].hCtl = GetDlgItem(fmc.hDlg, IDC_BUTTON1);
	GetWindowRect( gParams->ctl[CTL_ZOOM].hCtl, &rcCtl );
	ScreenToClient (fmc.hDlg, (LPPOINT)&rcCtl);
	ScreenToClient (fmc.hDlg, (LPPOINT)&(rcCtl.right));
	gParams->ctl[CTL_ZOOM].width = PixelsToHDBUs(rcCtl.right - rcCtl.left);
	gParams->ctl[CTL_ZOOM].height = PixelsToVDBUs(rcCtl.bottom - rcCtl.top);
	gParams->ctl[CTL_ZOOM].xPos = PixelsToHDBUs(rcCtl.left);
	gParams->ctl[CTL_ZOOM].yPos = PixelsToVDBUs(rcCtl.top);

	//Set Anchors
	setCtlAnchor (CTL_PREVIEW,ANCHOR_LEFT|ANCHOR_RIGHT|ANCHOR_TOP|ANCHOR_BOTTOM);
	setCtlAnchor (CTL_FRAME,ANCHOR_LEFT|ANCHOR_RIGHT|ANCHOR_TOP|ANCHOR_BOTTOM);
	setCtlAnchor (CTL_PROGRESS,ANCHOR_BOTTOM);
	setCtlAnchor (CTL_ZOOM,ANCHOR_BOTTOM);

	//Set scripting
    setCtlScripting (CTL_PREVIEW, false);
	setCtlScripting (CTL_FRAME, false);
    setCtlScripting (CTL_PROGRESS, false);
    setCtlScripting (CTL_ZOOM, false);

} /*createPredefinedCtls*/

/**********************************************************************/
/* resetAllCtls()
/*
/*  Resets all controls (deletes them).
/*
/**********************************************************************/
int resetAllCtls(void)
{
    int i;

    //Delete any previously defined controls.
    for (i=0; i < N_CTLS; i++) {
        if (gParams->ctl[i].inuse) {
            deleteCtl(i);
        }
    }

    //Now define the default controls:
    createPredefinedCtls();

    //This is also a good place (for now) to reinitialize
    //some dialog attributes...

    //set clipping region to NULL.
    SetWindowRgn(fmc.hDlg, NULL, TRUE /*redraw*/);
    gParams->gDialogRegion = NULL;

    //set other dialog parameters back to defaults
#if 0
    gParams->gDialogGradientColor1 = RGB(0,0,0);        //black
    gParams->gDialogGradientColor2 = RGB(0,0,0);        //black
#else
    //Added by Harald Heim, March 9, 2003
    gParams->gDialogGradientColor1 = GetSysColor(COLOR_BTNFACE); //RGB(0xD4,0xD0,0xC8);   //yellow grey
    gParams->gDialogGradientColor2 = GetSysColor(COLOR_BTNFACE); //RGB(0xD4,0xD0,0xC8);   //yellow grey
    //gParams->gDialogGradientColor1 = RGB(0x5F,0x9E,0xA0);   //X11.cadetblue
    //gParams->gDialogGradientColor2 = RGB(0x5F,0x9E,0xA0);   //X11.cadetblue
#endif
    gParams->gDialogGradientDirection = 0;              //vertical
    gParams->gDialogImage_save = gDialogImage = NULL;   //no background image
    gParams->gDialogImageFileName[0] = '\0';
    gParams->gDialogImageMode = DIM_TILED;              //TILED
    gParams->gDialogImageStretchMode = COLORONCOLOR;
    gParams->gDialogDragMode = 0;                       //drag by titlebar
#if 0
    gParams->gDialogTitle[0] = '\0';                    //default dialog title
#elif 1
    //default dialog title is "filter title (image mode, filter case)"
    strcpy(gParams->gDialogTitle, "!t"); // (!M, !f)
#else
    //default dialog title is "filter title (filter case) [image mode]"
    strcpy(gParams->gDialogTitle, "!t (!f) [!M]");
#endif

	gParams->gDialogTheme = 0; //Default Dialog Theme


    //initialize registry access...
    //setRegRoot(HKEY_CURRENT_USER);
    //setRegPath("Software\\!O\\!C\\!t");

    // Initialize built-in strings...
    gParams->filterAuthorText[0] = 0;
    //gParams->filterCategoryText[0] = 0;       //NO!! Clobbers val set at compile time
    //gParams->filterTitleText[0] = 0;          //NO!! Clobbers val set at compile time
    gParams->filterFilenameText[0] = 0;
    gParams->filterCopyrightText[0] = 0;
    gParams->filterDescriptionText[0] = 0;
    gParams->filterOrganizationText[0] = 0;
    gParams->filterURLText[0] = 0;
    gParams->filterVersionText[0] = 0;
    //gParams->filterAboutText[0] = 0;          //NO!! Clobbers val set at compile time
    //fmc.filterUniqueID[0] = 0;

    // Initialize built-in strings...
    fmc.filterAuthorText[0] = 0;
    //fmc.filterCategoryText[0] = 0;            //NO!! Clobbers val set at compile time
    //fmc.filterTitleText[0] = 0;               //NO!! Clobbers val set at compile time
    fmc.filterFilenameText[0] = 0;
    fmc.filterCopyrightText[0] = 0;
    fmc.filterDescriptionText[0] = 0;
    fmc.filterOrganizationText[0] = 0;
    fmc.filterURLText[0] = 0;
    fmc.filterVersionText[0] = 0;
    //fmc.filterUniqueID[0] = 0;

#ifndef DONT_SAVE_GLOBALS
    //Zero out the predefined globals (but not filterCategoryText, etc.!!!) */
    memset(&fmc.globvars, 0, sizeof(fmc.globvars));
#endif //DONT_SAVE_GLOBALS

    //Restore initial dialog styles...
    SetWindowLong(fmc.hDlg, GWL_STYLE, gParams->gDialogInitialStyle | WS_VISIBLE);
    SetWindowLong(fmc.hDlg, GWL_EXSTYLE, gParams->gDialogInitialStyleEx);

#if 1
    //turn this on only if needed to force update of dialog styles
    SetWindowPos(fmc.hDlg, NULL,
                 0, 0, 0, 0,
                 SWP_FRAMECHANGED|SWP_NOZORDER|SWP_NOACTIVATE|SWP_NOSIZE|SWP_NOMOVE);
#endif

#if 1
    //turn this on only if necessary to force update of dialog background and fonts
    return RedrawWindow(
            fmc.hDlg,   // handle of window
            NULL,   // address of structure with update rectangle, NULL=>entire client area
            NULL,   // handle of update region
            RDW_ERASE|RDW_INVALIDATE|RDW_ERASENOW|RDW_UPDATENOW     // array of redraw flags
       );
#else
    return TRUE;
#endif
} /*fm_resetAllCtls*/



//Sub Classed Tab
#if 0
INT_PTR WINAPI TabSubclassProc (HWND hDlg, UINT wMsg, WPARAM wParam, LPARAM lParam)  
{
	int i, iCtl;

	switch (wMsg){

		//case WM_PAINT:
			//return TRUE;

		case WM_ERASEBKGND:
			//break;
			//return TRUE;
		//Clip controls to avoid flickering
		for (i=0;i<N_CTLS;i++) if (hDlg == gParams->ctl[i].hCtl) {iCtl=i; break;}
		if (iCtl==-1) break;
		{
			HDC hDC = (HDC) wParam;
			RECT rect;//r,rC;
			for (i = 0; i < N_CTLS; ++i) {
				if (gParams->ctl[i].inuse && gParams->ctl[i].enabled & 3 && gParams->ctl[i].ctlclass != CC_IMAGE){
					if (gParams->ctl[i].tabControl == iCtl && gParams->ctl[i].tabSheet == gParams->ctl[iCtl].val){ // ctlEnabledAs(i)
						GetWindowRect( gParams->ctl[i].hCtl, &rect );
						ScreenToClient (hDlg, (LPPOINT)&rect);
						ScreenToClient (hDlg, (LPPOINT)&(rect.right));
						ExcludeClipRect(hDC,rect.left,rect.top,rect.right-1,rect.bottom-1);
						/*if (gParams->ctl[i].hBuddy2 && gParams->ctl[i].buddyBkColor != -1){ //Non-transparent labels
							GetWindowRect( gParams->ctl[i].hBuddy2, &rect );
							ScreenToClient (hDlg, (LPPOINT)&rect);
							ScreenToClient (hDlg, (LPPOINT)&(rect.right));
							ExcludeClipRect(hDC,rect.left,rect.top,rect.right-1,rect.bottom-1);
						}
						if (gParams->ctl[i].hBuddy1){ //edit boxes