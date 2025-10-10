	else if (state==1) flags |= LVGS_COLLAPSED;
	else if (state==2) flags |= LVGS_FOCUSED;
	else if (state==3) flags |= LVGS_SELECTED;*/
	
	grp.cbSize    = sizeof(LVGROUP);
    grp.mask      = LVGF_STATE;
	grp.iGroupId  = group;
	grp.state = state;
	grp.stateMask = state;
    
	return (int)ListView_SetGroupInfo(gParams->ctl[n].hCtl, group, &grp);
}

int getCtlGroupState(int n, int group)
{
	LVGROUP grp;

	if (n < 0 || n >= N_CTLS || !gParams->ctl[n].inuse || gParams->ctl[n].ctlclass != CC_LISTVIEW) {
        return FALSE;
    }

	#define LVGS_SELECTED 0x00000020

	grp.cbSize    = sizeof(LVGROUP);
    grp.mask      = LVGF_STATE;
	grp.iGroupId  = group;
	grp.stateMask = LVGS_NORMAL | LVGS_COLLAPSED | LVGS_SELECTED | LVGS_HIDDEN;
	grp.state = LVGS_NORMAL | LVGS_COLLAPSED | LVGS_SELECTED | LVGS_HIDDEN;
	
	ListView_GetGroupInfo(gParams->ctl[n].hCtl, group, &grp);

	return (int) grp.state;
}

int getCtlGroupCount(int n)
{
	if (n < 0 || n >= N_CTLS || !gParams->ctl[n].inuse || gParams->ctl[n].ctlclass != CC_LISTVIEW) {
        return FALSE;
    }

	#define LVM_GETGROUPCOUNT         (LVM_FIRST + 152)
	#define ListView_GetGroupCount(hwnd) SNDMSG((hwnd), LVM_GETGROUPCOUNT, (WPARAM)0, (LPARAM)0)

	return (int)ListView_GetGroupCount(gParams->ctl[n].hCtl);
}

int getCtlItemGroup(int n, int item)
{
	LVITEM lvi;

	if (n < 0 || n >= N_CTLS || !gParams->ctl[n].inuse || gParams->ctl[n].ctlclass != CC_LISTVIEW) {
        return FALSE;
    }

	 // Initialize LVITEM members that are common to all items.
	lvi.mask   = LVIF_GROUPID;
	lvi.iItem  = item;
	lvi.iSubItem = 0;
	
	ListView_GetItem (gParams->ctl[n].hCtl, &lvi);

	return (int) lvi.iGroupId;
}

int setCtlItemGroup(int n, int item, int group)
{
	LVITEM lvi;

	if (n < 0 || n >= N_CTLS || !gParams->ctl[n].inuse || gParams->ctl[n].ctlclass != CC_LISTVIEW) {
        return FALSE;
    }

	 // Initialize LVITEM members that are common to all items.
	lvi.mask   = LVIF_GROUPID;
	lvi.iItem  = item;
	lvi.iSubItem = 0;
	lvi.iGroupId = group;
	
	return (int) ListView_SetItem (gParams->ctl[n].hCtl, &lvi);
}

int setCtlImageList(int n, int i){

	if (n < 0 || n >= N_CTLS || !gParams->ctl[n].inuse || gParams->ctl[n].ctlclass != CC_LISTVIEW || ImageList[i]==NULL) {
        return FALSE;
    }

	ListView_SetImageList(gParams->ctl[n].hCtl,ImageList[i],LVSIL_NORMAL);

	return true;

}

int setCtlItemImage(int n, int item, int i){
	
	LVITEM lvi;

	if (n < 0 || n >= N_CTLS || !gParams->ctl[n].inuse || gParams->ctl[n].ctlclass != CC_LISTVIEW) {
        return FALSE;
    }

	lvi.mask   = LVIF_IMAGE;
	lvi.iItem  = item;
	lvi.iSubItem = 0;
	lvi.iImage = i;
	
	return (int) ListView_SetItem (gParams->ctl[n].hCtl, &lvi);

}

int getCtlItemImage(int n, int item){
	
	LVITEM lvi;

	if (n < 0 || n >= N_CTLS || !gParams->ctl[n].inuse || gParams->ctl[n].ctlclass != CC_LISTVIEW) {
        return FALSE;
    }

	lvi.mask   = LVIF_IMAGE;
	lvi.iItem  = item;
	lvi.iSubItem = 0;
	
	ListView_GetItem (gParams->ctl[n].hCtl, &lvi);
	
	return (int) lvi.iImage;

}

int labelHeight = 13;

int setCtlItemSpacing(int n, int width, int height, int applyLabelHeight){

	if (n < 0 || n >= N_CTLS || !gParams->ctl[n].inuse || !(gParams->ctl[n].ctlclass==CC_LISTVIEW || gParams->ctl[n].ctlclass==CC_TOOLBAR) ) {
        return FALSE;
    }

	if (gParams->ctl[n].ctlclass == CC_LISTVIEW){

		if (width==-1 || height==-1){
			int spacing = ListView_GetItemSpacing (gParams->ctl[n].hCtl,FALSE);
			if (width==-1) width = LOWORD(spacing);
			if (height==-1) height = HIWORD(spacing);
		}

		//Add or remove Label Height
		if (applyLabelHeight != 0){
			RECT prc;
			ListView_GetItemRect(gParams->ctl[n].hCtl,0,&prc,LVIR_LABEL);
			if (prc.bottom - prc.top > 0) labelHeight = prc.bottom - prc.top; //in case collapsed don't use
			if (applyLabelHeight>0)
				height += labelHeight;
			else 
				height -= labelHeight;
		}

		return ListView_SetIconSpacing(gParams->ctl[n].hCtl,width,height);

	} else {
		
		return (int)SendMessage(gParams->ctl[n].hCtl, TB_SETBUTTONSIZE, 0, MAKELPARAM(HDBUsToPixels(width),VDBUsToPixels(height)) );

	}
}

int setCtlExtStyle(int n, int exstyle){

	if (n < 0 || n >= N_CTLS || !gParams->ctl[n].inuse || gParams->ctl[n].ctlclass != CC_LISTVIEW) {
        return FALSE;
    }

	/*if (exstyle = LVS_EX_HIDELABELS){
		if (gParams->ctl[n].style & LVS_REPORT) return false;
		setCtlItemSpacing(n,-1,-1,-1); //Remove label height
	}*/

	return ListView_SetExtendedListViewStyle(gParams->ctl[n].hCtl,exstyle);
}

int clearCtlExtStyle(int n, int exstyle){

	int flags;

	if (n < 0 || n >= N_CTLS || !gParams->ctl[n].inuse || gParams->ctl[n].ctlclass != CC_LISTVIEW) {
        return FALSE;
    }

	flags = ListView_GetExtendedListViewStyle(gParams->ctl[n].hCtl);
	flags &= ~exstyle;

	/*if (exstyle = LVS_EX_HIDELABELS){
		setCtlItemSpacing(n,-1,-1,1); //Add label height
	}*/

	return ListView_SetExtendedListViewStyle(gParams->ctl[n].hCtl,flags);
}


int setCtlGroupView(int n, int on)
{
	if (n < 0 || n >= N_CTLS || !gParams->ctl[n].inuse || gParams->ctl[n].ctlclass != CC_LISTVIEW) {
        return FALSE;
    }

	return (int) ListView_EnableGroupView(gParams->ctl[n].hCtl, (BOOL)on);
}





int fillDir(int n, int attr, int wildcard)
{
	if (n < 0 || n >= N_CTLS || !gParams->ctl[n].inuse) {
        return FALSE;
    }

	if (gParams->ctl[n].ctlclass == CC_LISTBOX || gParams->ctl[n].ctlclass == CC_LISTBAR)
		return (int) SendMessage(gParams->ctl[n].hCtl, LB_DIR, attr, wildcard);
	else if (gParams->ctl[n].ctlclass == CC_COMBOBOX)
		return (int) SendMessage(gParams->ctl[n].hCtl, CB_DIR, attr, wildcard);
	else 
		return -1;

}



INT_PTR getResAddress(INT_PTR restype, INT_PTR resname){

	HRSRC hRes;         // handle/ptr. to res. info.
    int cb;             // Size of resource (bytes)
    HRSRC hResLoad;     // handle to loaded resource  
    char *hResPointer;  // pointer to resource data 
    
    
    // Locate the resource. 
    //hRes = FindResource(hDllInstance,MAKEINTRESOURCE(resname),(LPCTSTR)restype);
	hRes = FindResource((HMODULE)hDllInstance,(LPCTSTR)resname,(LPCTSTR)restype);
    if (hRes == NULL) { 
        return FALSE;
    } 

    // Check for size of resource
    if ((cb=SizeofResource((HMODULE)hDllInstance, hRes)) <= 0)
		return FALSE;

	// Load the resource into global memory.
    hResLoad = (HRSRC)LoadResource((HMODULE)hDllInstance, hRes);
    if (hResLoad == NULL) { 
        return FALSE;
    } 

    // Lock the resource into global memory.
    hResPointer = (char *)LockResource(hResLoad);
    if (hResPointer == NULL) {
        return FALSE;
    } else 
		return (INT_PTR)hResPointer;

}

int getResSize(char* restype, char* resname){

	HRSRC hRes;         // handle/ptr. to res. info.
    int cb;             // Size of resource (bytes)
    
    // Locate the resource.
    //hRes = FindResource(hDllInstance,MAKEINTRESOURCE(resname),(LPCTSTR)restype);
	hRes = FindResource((HMODULE)hDllInstance,(LPCTSTR)resname,(LPCTSTR)restype);
    if (hRes == NULL) { 
        return FALSE;
    } 

	// Check for size of resource and allocate array
    if ((cb=SizeofResource((HMODULE)hDllInstance, hRes)) > 0)
		return (int)cb;
	else 
		return 0;

}

int copyResToArray (char* restype, char* resname, int arraynr)
{
    HRSRC hRes;         // handle/ptr. to res. info.
    int cb;             // Size of resource (bytes)
    HRSRC hResLoad;     // handle to loaded resource  
    char *hResPointer;  // pointer to resource data 
    
    
    // Locate the resource. 
    //hRes = FindResource(hDllInstance,(LPCTSTR)resname,(LPCTSTR)restype);
    //hRes = FindResource(hDllInstance,MAKEINTRESOURCE(resname),(LPCTSTR)restype);
	hRes = FindResource((HMODULE)hDllInstance,(LPCTSTR)resname,(LPCTSTR)restype);
    if (hRes == NULL) { 
        
        //Instead try to find a file with the same name as resname
        FILE *file;
        int cb;
        
        file = fopen( (LPCTSTR)resname, "rb" );