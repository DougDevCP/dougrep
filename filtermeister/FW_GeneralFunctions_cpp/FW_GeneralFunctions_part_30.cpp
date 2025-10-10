
#ifndef LVM_ISITEMVISIBLE
	#define LVM_ISITEMVISIBLE (LVM_FIRST+182) 
	#define ListView_IsItemVisible(hwnd,index) (UINT)SNDMSG((hwnd),LVM_ISITEMVISIBLE,(WPARAM)(index),(LPARAM)0) 
#endif

int getCtlItemTop(int n)
{

	if (n < 0 || n >= N_CTLS || !gParams->ctl[n].inuse) {
        return FALSE;
    }

	if (gParams->ctl[n].ctlclass == CC_LISTBOX || gParams->ctl[n].ctlclass == CC_LISTBAR)
		return (int) SendMessage(gParams->ctl[n].hCtl, LB_GETTOPINDEX, 0, 0);
	else if (gParams->ctl[n].ctlclass == CC_COMBOBOX)
		return (int) SendMessage(gParams->ctl[n].hCtl, CB_GETTOPINDEX, 0, 0);
	else if (gParams->ctl[n].ctlclass == CC_LISTVIEW){
		int i;
		int itemcount = getCtlItemCount(n);
		for (i=0;i<itemcount;i++) if (ListView_IsItemVisible(gParams->ctl[n].hCtl,i)) break;
		if (i==itemcount-1) return 0; else return i;
		//return ListView_GetTopIndex(gParams->ctl[n].hCtl); // Does not work!!!
	} else 
		return -1;

}

int getCtlItemPos(int n, int item, int w)
{
	RECT rc;
	int val=0;

	if (n < 0 || n >= N_CTLS || !gParams->ctl[n].inuse || gParams->ctl[n].ctlclass != CC_TOOLBAR) {
        return FALSE;
    }

#if SKIN
	if (isSkinActive() && gParams->skinStyle>0){
		rc.left = item*skinGetToolItemWidth();
		rc.top = 0;
	} else
#endif
		SendMessage(gParams->ctl[n].hCtl, TB_GETRECT, item, (LPARAM)&rc);
	
	switch (w){
		case 0: val = PixelsToHDBUs(rc.left); break;
		case 1: val = PixelsToVDBUs(rc.top); break;
		case 2: val = PixelsToHDBUs(rc.right - rc.left); break;
		case 3: val = PixelsToVDBUs(rc.bottom - rc.top); break;
	}

	return val;

}

int setCtlItemTop(int n, int item)
{
	int retval;

	if (n < 0 || n >= N_CTLS || !gParams->ctl[n].inuse) {
        return FALSE;
    }

	if (gParams->ctl[n].ctlclass == CC_LISTBOX || gParams->ctl[n].ctlclass == CC_LISTBAR)
		retval = (int)SendMessage(gParams->ctl[n].hCtl, LB_SETTOPINDEX, item, 0);
	else if (gParams->ctl[n].ctlclass == CC_COMBOBOX)
		retval = (int)SendMessage(gParams->ctl[n].hCtl, CB_SETTOPINDEX, item, 0);
	else if (gParams->ctl[n].ctlclass == CC_LISTVIEW){
		return (int)ListView_EnsureVisible(gParams->ctl[n].hCtl,item,FALSE);
	/*
		RECT prc ={0,0,0,0};
		POINT pItemPosition;
		if (ListView_GetViewRect(gParams->ctl[n].hCtl,&prc)){
			if (ListView_GetItemPosition(gParams->ctl[n].hCtl,item,&pItemPosition)){
				if (ListView_Scroll(gParams->ctl[n].hCtl,0, pItemPosition.y + prc.top)){
					return true;
				}
			} else { //Less good alternative
				ListView_EnsureVisible(gParams->ctl[n].hCtl,item,FALSE);
			}
		}
	*/
		return false;
	} else
		return false;

	if (retval == LB_ERR) //|| retval == CB_ERR
		return false;
	else
		return true;

}

int getCtlItemText(int n, int item, char * str)
{
	int retval;

	if (n < 0 || n >= N_CTLS || !gParams->ctl[n].inuse) {
        return FALSE;
    }

	if (gParams->ctl[n].ctlclass == CC_LISTBOX || gParams->ctl[n].ctlclass == CC_LISTBAR)
		retval = (int) SendMessage(gParams->ctl[n].hCtl, LB_GETTEXT, item, (LPARAM)str);
	
	else if (gParams->ctl[n].ctlclass == CC_LISTVIEW){
		ListView_GetItemText(gParams->ctl[n].hCtl, item, 0, (LPTSTR)str, 255);
		retval = LB_OKAY;

	} else if (gParams->ctl[n].ctlclass == CC_COMBOBOX)
#if SKIN
		if (isSkinActive() && gParams->skinStyle>0)
			retval = (int) getCtlItemString(n,item,str);
		else
#endif
			retval = (int) SendMessage(gParams->ctl[n].hCtl, CB_GETLBTEXT, item, (LPARAM)str);

	else if (gParams->ctl[n].ctlclass == CC_TAB){
		TCITEM tabItem; 
		
		tabItem.mask = TCIF_TEXT;
		tabItem.pszText = (char *)str;
		tabItem.cchTextMax = 256;

		if (TabCtrl_GetItem(gParams->ctl[n].hCtl, item, &tabItem)){
			retval = LB_OKAY;
		} else
			retval = LB_ERR;
	}

	if (retval == LB_ERR) //|| retval == CB_ERR
		return false;
	else
		return true;

}
 
int setCtlItemText(int n, int item, char * str)
{
	int retval = -1;
	
	if (n < 0 || n >= N_CTLS || !gParams->ctl[n].inuse) {
        return FALSE;
    }
	
    if (gParams->ctl[n].ctlclass == CC_LISTBOX || gParams->ctl[n].ctlclass == CC_LISTBAR)
        retval = (int) SendMessage(gParams->ctl[n].hCtl, LB_INSERTSTRING, item, (LPARAM)str);
    
	else if (gParams->ctl[n].ctlclass == CC_LISTVIEW){

		LVITEM lvi;

		 // Initialize LVITEM members that are common to all items.
		lvi.mask   = LVIF_TEXT; // | LVIF_IMAGE|LVIF_STATE ;
		lvi.iItem  = item;
		lvi.iSubItem  = 0;
		lvi.pszText  = (LPTSTR)str; //NULL; //LPSTR_TEXTCALLBACK; // Sends an LVN_GETDISPINFO message.
		lvi.cchTextMax = lstrlen(str);
		
		retval = (int) ListView_InsertItem (gParams->ctl[n].hCtl, &lvi);
		
	} else if (gParams->ctl[n].ctlclass == CC_COMBOBOX)
		retval = (int) SendMessage(gParams->ctl[n].hCtl, CB_INSERTSTRING, item, (LPARAM)str);
	
	else if (gParams->ctl[n].ctlclass == CC_TAB){

		int tabCount = TabCtrl_GetItemCount(gParams->ctl[n].hCtl);
		TCITEM tabItem; 
		
		tabItem.mask = TCIF_TEXT | TCIF_IMAGE; 
		tabItem.iImage = -1; 
		tabItem.pszText = (char *)str; 
		
		if (item<0) item = tabCount+1;
		
		if (item>tabCount)
			retval = (int)TabCtrl_InsertItem(gParams->ctl[n].hCtl, item, &tabItem);
		else
			retval = (int)TabCtrl_SetItem(gParams->ctl[n].hCtl, item, &tabItem);


	}
	
	//Write to fmc label
	if (gParams->ctl[n].ctlclass == CC_COMBOBOX){
		if (strlen(gParams->ctl[n].label)+strlen(str) < sizeof(gParams->ctl[n].label)-1) { //1022
			strcat(gParams->ctl[n].label,str);
			strcat(gParams->ctl[n].label,"\n");
		}
	}

	return retval;

}

int setCtlItemState(int n, int item, int state){

	LVITEM lvi;

	if (n < 0 || n >= N_CTLS || !gParams->ctl[n].inuse || gParams->ctl[n].ctlclass != CC_LISTVIEW) {
        return FALSE;
    }

	lvi.mask   = LVIF_STATE;
	lvi.iItem  = item;
	lvi.iSubItem = 0;
	if (state==0){
		lvi.state = 0;
		lvi.stateMask = LVIS_CUT|LVIS_DROPHILITED|LVIS_FOCUSED|LVIS_SELECTED;
	} else {
		lvi.state = state;
		lvi.stateMask = state;
	}
	
	return (int) ListView_SetItem (gParams->ctl[n].hCtl, &lvi);
}

int deleteCtlItem(int n, int item)
{

	if (n < 0 || n >= N_CTLS || !gParams->ctl[n].inuse) {
        return FALSE;
    }

	if (gParams->ctl[n].ctlclass == CC_LISTBOX || gParams->ctl[n].ctlclass == CC_LISTBAR)
		return (int) SendMessage(gParams->ctl[n].hCtl, LB_DELETESTRING, item, 0);
	
	else if (gParams->ctl[n].ctlclass == CC_COMBOBOX)
		return (int) SendMessage(gParams->ctl[n].hCtl, CB_DELETESTRING, item, 0);
	
	else if (gParams->ctl[n].ctlclass == CC_TAB)
		return (int)TabCtrl_DeleteItem(gParams->ctl[n].hCtl, item);

	return false;

}

int deleteCtlItems(int n)
{
	if (n < 0 || n >= N_CTLS || !gParams->ctl[n].inuse) {
        return FALSE;
    }
	
	if (gParams->ctl[n].ctlclass == CC_TAB){
		return (int)TabCtrl_DeleteAllItems (gParams->ctl[n].hCtl);

	} else if (gParams->ctl[n].ctlclass == CC_LISTBOX || gParams->ctl[n].ctlclass == CC_LISTBAR || gParams->ctl[n].ctlclass == CC_COMBOBOX) {
		return setCtlText(n,"");
	}
	
	return false;
}


#define LVGS_COLLAPSIBLE 0x00000008
#define LVGS_NOHEADER 0x00000004

int setCtlGroupText(int n, int group, char * str) //int startitem,
{
	LVGROUP grp;
	wchar_t wstr[512];
	
	//At least Win XP required
	//if (getWindowsVersion()<9) return FALSE; 

	if (n < 0 || n >= N_CTLS || !gParams->ctl[n].inuse || gParams->ctl[n].ctlclass != CC_LISTVIEW) {
        return FALSE;
    }

	mbstowcs(wstr,str,strlen(str)+1);
   
	#define LVGF_TITLEIMAGE 0x00001000 
	#define LVGF_EXTENDEDIMAGE 0x00002000 
	
	//ZeroMemory(&grp,sizeof(LVGROUP));
    grp.cbSize    = sizeof(LVGROUP);
    grp.mask      = LVGF_HEADER | LVGF_GROUPID  | LVGF_STATE;// | LVGF_TITLEIMAGE;// | LVGF_ALIGN;
    grp.pszHeader = (LPWSTR)wstr;
	grp.iGroupId  = group;
	grp.stateMask = 0;//LVGS_COLLAPSIBLE; //
	grp.state = 0;//LVGS_COLLAPSIBLE; //
	//grp.uAlign = LVGA_HEADER_CENTER;
	//grp.iTitleImage = 0;

    return (int) ListView_InsertGroup(gParams->ctl[n].hCtl, -1, &grp); //startitem
}

int setCtlGroupState(int n, int group, int state){

	//int flags=0;
	LVGROUP grp;

	if (n < 0 || n >= N_CTLS || !gParams->ctl[n].inuse || gParams->ctl[n].ctlclass != CC_LISTVIEW) {
        return FALSE;
    }

	//#define LVGS_FOCUSED 0x00000010
	//#define LVGS_SELECTED 0x00000020
	/*if (state==-1) flags |= LVGS_NORMAL ;
	else if (state==0) flags |= LVGS_HIDDEN; 