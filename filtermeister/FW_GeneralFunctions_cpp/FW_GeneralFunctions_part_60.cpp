							GetWindowRect( gParams->ctl[i].hBuddy1, &rect );
							ScreenToClient (hDlg, (LPPOINT)&rect);
							ScreenToClient (hDlg, (LPPOINT)&(rect.right));
							ExcludeClipRect(hDC,rect.left,rect.top,rect.right-1,rect.bottom-1);
						}*/
					}
				}
			}
		}
		//return TRUE;
		break;
	}

	return CallWindowProc((WNDPROC)GetWindowLongPtr(hDlg,GWLP_USERDATA),hDlg,wMsg,wParam,lParam); //call the default system handler for the control
}
#endif

//Sub Classed Listview
INT_PTR WINAPI ListviewSubclassProc (HWND hDlg, UINT wMsg, WPARAM wParam, LPARAM lParam)  
{
	WNDPROC NextProc = NULL;

	switch (wMsg){

		case WM_LBUTTONDOWN:
		{
			int x = LOWORD(lParam);
            int y = HIWORD(lParam);
			#define LVHT_EX_GROUP_HEADER       0x10000000
			#define LVHT_EX_GROUP_FOOTER       0x20000000
			#define LVHT_EX_GROUP_COLLAPSE     0x40000000
			#define LVHT_EX_GROUP_BACKGROUND   0x80000000
			#define LVHT_EX_GROUP_STATEICON    0x01000000
			#define LVHT_EX_GROUP_SUBSETLINK   0x02000000
			#define LVHT_EX_GROUP              (LVHT_EX_GROUP_BACKGROUND | LVHT_EX_GROUP_COLLAPSE | LVHT_EX_GROUP_FOOTER | LVHT_EX_GROUP_HEADER | LVHT_EX_GROUP_STATEICON | LVHT_EX_GROUP_SUBSETLINK)
			#define ListView_HitTestEx(hwndLV, pinfo) \
					(int)SNDMSG((hwndLV), LVM_HITTEST, (WPARAM)-1, (LPARAM)(LV_HITTESTINFO *)(pinfo))
            	
			int i;
			int iCtl=-1;
			LVHITTESTINFO pinfo;
			RECT rcCtl;
			
			for (i=0;i<N_CTLS;i++) if (hDlg == gParams->ctl[i].hCtl) {iCtl=i; break;}
			if (iCtl==-1) break;
            
			GetWindowRect(gParams->ctl[i].hCtl, &rcCtl);
			ScreenToClient (hDlg, (LPPOINT)&rcCtl);
			ScreenToClient (hDlg, (LPPOINT)&(rcCtl.right));
	
			pinfo.pt.x = x-rcCtl.left;
			pinfo.pt.y = y-rcCtl.top;
			pinfo.flags = 0;


#ifndef _DEBUG
    __try { //Avoid 32-bit crash !!!
#endif
			if (ListView_HitTestEx(gParams->ctl[iCtl].hCtl,&pinfo)!=-1){
			//if (SendMessage(hDlg, LVM_HITTEST, -1, (LPARAM)&pinfo)){
				if (pinfo.flags & LVHT_EX_GROUP_HEADER) { //LVHT_EX_GROUP
					
					//Expand or collapse group
					if (getCtlGroupState(iCtl,pinfo.iItem)==LVGS_NORMAL)
						setCtlGroupState(iCtl,pinfo.iItem,LVGS_COLLAPSED);
					else
						setCtlGroupState(iCtl,pinfo.iItem,LVGS_NORMAL);

					//Avoid that first item in a group is automatically selected
					SendMessage (gParams->ctl[iCtl].hCtl,WM_LBUTTONDBLCLK,0,lParam);
					
					//Make sure that listview is redrawn
					enableCtl(iCtl,0);
					enableCtl(iCtl,-1);
				
					return TRUE;
				}
			}
#ifndef _DEBUG
	} __except (EXCEPTION_EXECUTE_HANDLER) {};
#endif

			break;
		}

		//Causes redraw issues so deactivate for now
		case WM_KEYDOWN:
		{
			int nextItem = -1;
			int key = (int)wParam;
			int i,j;
			int iCtl=-1;
			
			#define LVGS_FOCUSED 0x00000010
			#define LVGS_SELECTED 0x00000020
			
			if (key==VK_LEFT || key==VK_RIGHT){

				for (i=0;i<N_CTLS;i++) if (hDlg == gParams->ctl[i].hCtl) {iCtl=i; break;}
				if (iCtl==-1) break;
				
				for (j=0;j<getCtlGroupCount(iCtl);j++){

					int groupState = getCtlGroupState(iCtl,j);

					if ((groupState & LVGS_SELECTED || groupState & LVGS_FOCUSED)){ // && !(groupState & LVGS_HIDDEN)
						if (key==VK_RIGHT && (groupState & LVGS_COLLAPSED)){
							setCtlGroupState(iCtl,j,LVGS_NORMAL);
							setCtlFocus(iCtl);
							setCtlGroupState(iCtl,j,LVGS_FOCUSED);
							//Make sure that listview is redrawn
							enableCtl(iCtl,0);
							enableCtl(iCtl,-1);
							SetFocus(hDlg);
							return TRUE;
						} else if (key==VK_LEFT && !(groupState & LVGS_COLLAPSED)){
							setCtlGroupState(iCtl,j,LVGS_COLLAPSED);
							setCtlFocus(iCtl);
							setCtlGroupState(iCtl,j,LVGS_FOCUSED);
							//Make sure that listview is redrawn
							enableCtl(iCtl,0);
							enableCtl(iCtl,-1);
							SetFocus(hDlg);
							return TRUE;
						}
						break;
					}
				}


				//Jump to previous/next line with cursor left/right----
				if (key==VK_RIGHT){
					nextItem = ListView_GetNextItem(hDlg,fmc.pre_ctl[iCtl],LVNI_TORIGHT);
					if (nextItem==-1){
						nextItem = ListView_GetNextItem(hDlg,fmc.pre_ctl[iCtl],LVNI_ALL);
						if (nextItem!=-1){
							setCtlVal(iCtl,nextItem);
							return TRUE;
						}
					}
				} else if (key==VK_LEFT){
					nextItem = ListView_GetNextItem(hDlg,fmc.pre_ctl[iCtl],LVNI_TOLEFT);
					if (nextItem==-1){
						#define LVNI_PREVIOUS 0x0020
						nextItem = ListView_GetNextItem(hDlg,fmc.pre_ctl[iCtl],LVNI_PREVIOUS);
						if (nextItem!=-1){
							setCtlVal(iCtl,nextItem);
							return TRUE;
						}
					}
				}


/*
			} else if (key==VK_UP || key==VK_DOWN ){ // || key==VK_PAGEUP || key==VK_PAGEDOWN
				
				for (i=0;i<N_CTLS;i++) if (hDlg == gParams->ctl[i].hCtl) {iCtl=i; break;}

				//Only if multi select is active
				//if ( !(GetWindowLong(gParams->ctl[iCtl].hCtl, GWL_STYLE) & LVS_SINGLESEL))
				{
					int val, nextval, group, nextgroup;				
					val = ListView_GetNextItem(gParams->ctl[iCtl].hCtl,-1, LVNI_FOCUSED);
					group = getCtlItemGroup(iCtl,val);

					if (key==VK_UP) nextval = ListView_GetNextItem(gParams->ctl[iCtl].hCtl,val, LVNI_ABOVE);//val-1;
					else nextval = ListView_GetNextItem(gParams->ctl[iCtl].hCtl,val, LVNI_BELOW);//val+1;

					//Expand next group if neccesary 
					if (nextval == -1){
						if (key==VK_UP && group != 0){
							for (i=val;i>=0;i--){
								if (getCtlItemGroup(iCtl,i)==group-1) {
									nextval=i;
									break;
								}
							}
						} else if (key==VK_DOWN && group != getCtlGroupCount(iCtl)-1){
							for (i=val;i<getCtlItemCount(iCtl);i++){
								if (getCtlItemGroup(iCtl,i)==group+1) {
									nextval=i;
									break;
								}
							}
						}
					}

					if (nextval<0 || nextval >= getCtlItemCount(iCtl)) return TRUE;

					nextgroup = getCtlItemGroup(iCtl,nextval);

					//Info ("%d - %d",group,nextgroup);

					//Avoid that all items of a group are selected
					if (group != nextgroup) {
						
						//Expand next group if neccesary
						if (getCtlGroupState(iCtl,nextgroup) & LVGS_COLLAPSED) {
							setCtlGroupState(iCtl,nextgroup,LVGS_NORMAL);
							ListView_EnsureVisible(gParams->ctl[iCtl].hCtl,nextval,true);
						}

						setCtlItemState(iCtl,val,0);
						setCtlVal(iCtl,nextval);
						ListView_EnsureVisible(gParams->ctl[iCtl].hCtl,nextval,true);
						return TRUE;
					}
				}
*/
			}
			break;
		}
		
  	}//wMsg	


	NextProc = (WNDPROC)GetWindowLongPtr(hDlg,GWLP_USERDATA);
	return CallWindowProc(NextProc,hDlg,wMsg,wParam,lParam); //call the default system handler for the control (button)
}

/**********************************************************************/
/* createCtl(n, c, t, x, y, w, h, s, sx, p, e)
/*
/*  Dynamically creates a control with index n, class c, text t,
/*  coordinates (x,y), width w, height h, style s, extended style sx,
/*  properties p, and enable level e.
/*  All measurements are in DBUs.
/*  For x, y, w, and h, a value of -1 means use the default value.
/*
/**********************************************************************/
int createCtl(int n, int c, char * t, int x, int y, int w, int h,
                        int s, int sx, int p, int e)
{
    if (n < 0 || n >= N_CTLS) {
        //invalid control id
        return 0;
    }

	//s = s | WS_CLIPSIBLINGS;

    if (gParams->ctl[n].inuse) {
        // kill previous control with this id
        deleteCtl(n);
    }

    gParams->ctl[n].ctlclass = (CTLCLASS)c;
    gParams->ctl[n].hCtl = NULL;
    gParams->ctl[n].hBuddy1 = NULL;
    gParams->ctl[n].hBuddy2 = NULL;

    if (t) {
        strncpy(gParams->ctl[n].label, (char *)t, MAX_LABEL_SIZE+1);
        gParams->ctl[n].label[MAX_LABEL_SIZE] = '\0';   //ensure null-terminated.
    }
    else {
        //No text string
        gParams->ctl[n].label[0] = '\0';
    }

    // -1 means use default value for x, y, w, or h.

    // Default size depends on control class (and sometimes
    // control index, style, or properties)...
    switch (c) {
    case CC_PUSHBUTTON:
        if (w < 0) w = 34;
        if (h < 0) h = 14;
        break;
    case CC_RECT:
    case CC_FRAME:
    case CC_IMAGE:
    case CC_BITMAP:
    case CC_ICON:
    case CC_METAFILE:
    case CC_ANIMATION:
        if (w < 0) w = 32;
        if (h < 0) h = (n == CTL_LOGO) ? 43 : 32;
        break;
    case CC_GROUPBOX:
        if (w < 0) w = 100;
        if (h < 0) h = 32;
        break;
    case CC_LISTVIEW:
	case CC_LISTBOX:
    case CC_COMBOBOX:
	    if (w < 0) w = 90;
        if (h < 0) h = 32;
        break;
    case CC_LISTBAR:
        if (w < 0) w = 90;
        if (h < 0) h = 10;
        break;
    case CC_STANDARD:
    case CC_SCROLLBAR:
        if (s & SBS_VERT) {
            //vertical scrollbar
            if (w < 0) w = 8;
            if (h < 0) h = 90;
        }
        else {