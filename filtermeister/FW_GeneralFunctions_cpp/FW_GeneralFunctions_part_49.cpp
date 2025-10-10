                    count++;
                } //while

                if (gParams->ctl[n].style & WS_HSCROLL && maxchars>0){

                    TEXTMETRIC tm;
                    GetTextMetrics(GetDC(gParams->ctl[n].hCtl), &tm); 
                    SendMessage(gParams->ctl[n].hCtl, LB_SETHORIZONTALEXTENT, (WPARAM)(tm.tmAveCharWidth-2) * (maxchars+2), 0);
                    
                    //SendMessage(gParams->ctl[n].hCtl, LB_SETHORIZONTALEXTENT, (WPARAM)sz.cx, 0);
                }

                free(copy);
			
            //#endif
            

            if (gParams->ctl[n].ctlclass==CC_LISTBAR && count>0 && gParams->ctl[n].height < 20) SendMessage(gParams->ctl[n].hCtl, LB_SETCOLUMNWIDTH, HDBUsToPixels(gParams->ctl[n].width/count), 0);
			
        }
        // set the control value to set the initial selection
        setCtlVal(n, gParams->ctl[n].val);
        break;
	
	case CC_LISTVIEW:
		{
			int i;
			char *a;
			char copy[MAX_LABEL_SIZE+1];
			LVITEM lvi;

			ListView_DeleteAllItems(gParams->ctl[n].hCtl);
			ListView_RemoveAllGroups(gParams->ctl[n].hCtl);

			lvi.mask   = LVIF_TEXT; // | LVIF_IMAGE|LVIF_STATE ;
			lvi.iSubItem  = 0;
			
			strncpy(copy, (char *)iText, MAX_LABEL_SIZE+1);
			if (iText[0] == 0) return false; //Delete items only

			a = strtok(copy,"\n");
			for (i=0;i>=0;i++){
			  	
				lvi.iItem  = i;
				lvi.pszText  = (LPTSTR)a;
				ListView_InsertItem (gParams->ctl[n].hCtl, &lvi);
					
				a = strtok(NULL,"\n");
				if (a==0)break;
			}

		}
		break;

	case CC_TAB:
		{
			TCITEM tabItem; 
			int i;
			char *a;
			char copy[MAX_LABEL_SIZE+1];

			TabCtrl_DeleteAllItems (gParams->ctl[n].hCtl); 
			
			strncpy(copy, (char *)iText, MAX_LABEL_SIZE+1);

			tabItem.mask = TCIF_TEXT | TCIF_IMAGE; 
			tabItem.iImage = -1; 

			a = strtok(copy,"\n");
			for (i=0;i>=0;i++){
			  	
				tabItem.pszText = a; 
				TabCtrl_InsertItem(gParams->ctl[n].hCtl, i, &tabItem);

				a = strtok(NULL,"\n");
				if (a==0)break;
			}

#if SKIN
			//if (isSkinActive()) skinDraw(n,2); //Erase Background
#endif

		}	
		break;

	case CC_TOOLBAR:
		{
			int itemNumber = 0;
			TBBUTTON * tbButtons;
			char * a;
			int i;
			char copy[MAX_LABEL_SIZE+1];
			//SIZE size;

			
			if (strcmp(iText,"")!=0){

				int n_buttons = (int)SendMessage(gParams->ctl[n].hCtl,TB_BUTTONCOUNT,0,0);

				//Delete old buttons
				for (i=0;i<n_buttons;i++){
					SendMessage(gParams->ctl[n].hCtl,TB_DELETEBUTTON,0,0);
				}

				//Count items
				strncpy(copy, (char *)iText, MAX_LABEL_SIZE+1);
				a = strtok(copy,"\n");
				for (i=0;i>=0;i++){
					itemNumber++;
					a = strtok(NULL,"\n");
					if (a==0)break;
				}
				
				//Alloc
				tbButtons = (TBBUTTON *)calloc(itemNumber*sizeof(TBBUTTON),1);

				//Add items
				strncpy(copy, (char *)iText, MAX_LABEL_SIZE+1);
				a = strtok(copy,"\n");
				for (i=0;i>=0;i++){
			  		tbButtons[i].iBitmap = I_IMAGENONE;
					tbButtons[i].idCommand = i;
					tbButtons[i].fsState = TBSTATE_ENABLED;
					tbButtons[i].fsStyle = BTNS_DROPDOWN|BTNS_AUTOSIZE;
					tbButtons[i].dwData = 0;
					tbButtons[i].iString = (INT_PTR)a;
					a = strtok(NULL,"\n");
					if (a==0)break;
				}

				SendMessage(gParams->ctl[n].hCtl, TB_BUTTONSTRUCTSIZE, (WPARAM)sizeof(TBBUTTON), 0);
				SendMessage(gParams->ctl[n].hCtl, TB_ADDBUTTONS,       (WPARAM)itemNumber, (LPARAM)tbButtons);

				//setCtlPos(n,-1,-1,-1,-1);
				//SendMessage(gParams->ctl[n].hCtl, TB_AUTOSIZE, 0, 0);
				//Get new size
				//SendMessage(gParams->ctl[n].hCtl, TB_GETMAXSIZE, 0, (LPARAM)&size);
				//gParams->ctl[n].width = size.cx;
				//gParams->ctl[n].height = size.cy;
			}

		}
		break;

    default:
        //most other controls....
        return SetWindowText(gParams->ctl[n].hCtl, formatString(gParams->ctl[n].label));
        break;
    } //switch
    return FALSE;   //failed
} /*fm_setCtlText*/

/**********************************************************************/
/* setCtlTextv(n, "format",...)
/*
/*  Sets the text label or content for control n to the expanded
/*  printf-style format string.
/*
/**********************************************************************/
int setCtlTextv(int n, char* iFmt,...)
{
    char szBuffer[1024];
    va_list ap;

    va_start(ap, iFmt);
    _vsnprintf(szBuffer, sizeof(szBuffer), (char *)iFmt, ap);
    szBuffer[sizeof(szBuffer)-1] = 0;   //ensure NUL-terminated
    va_end(ap);
    return setCtlText(n, (char*)szBuffer);
} /*fm_setCtlTextv*/


/**********************************************************************/
/* enableToolTipBalloon(enable)
/*
/*  Enables or disables the balloon shape of all tool tips, based on
/*  the value of boolean value "enable".
/*
/**********************************************************************/
// For older versions of the SDK include files...
#ifndef TTS_BALLOON

#define TTS_NOANIMATE           0x10
#define TTS_NOFADE              0x20
#define TTS_BALLOON             0x40
#define TTS_CLOSE               0x80

//flags
#define TTF_PARSELINKS          0x1000


// ToolTip Icons (Set with TTM_SETTITLE)
#define TTI_NONE                0
#define TTI_INFO                1
#define TTI_WARNING             2
#define TTI_ERROR               3

#define TTM_SETTITLEA           (WM_USER + 32)  // wParam = TTI_*, lParam = char* szTitle
#define TTM_SETTITLEW           (WM_USER + 33)  // wParam = TTI_*, lParam = wchar* szTitle

#ifdef UNICODE
#define TTM_SETTITLE            TTM_SETTITLEW
#else
#define TTM_SETTITLE            TTM_SETTITLEA
#endif

#endif  //old include file


int enableToolTipBalloon(int enable) {
	LONG dwStyle = (LONG)GetWindowLong(ghTT, GWL_STYLE);

	int oldStyle = (dwStyle & TTS_BALLOON)? TRUE : FALSE;

	if (enable != FALSE) {
        dwStyle = WS_POPUP | TTS_ALWAYSTIP | TTS_NOPREFIX | TTS_BALLOON;

	} else {
        dwStyle = WS_POPUP | TTS_ALWAYSTIP | TTS_NOPREFIX | WS_BORDER;
	}

	SetWindowLong(ghTT, GWL_STYLE, dwStyle);

	return oldStyle;
}

// NOTE: Shouldn't we have a version of SetCtlToolTip with printf-style
//       formatting?  E.g., setCtlToolTipv(n, s, "format",...)

/**********************************************************************/
/* setCtlToolTip(n, "Tool Tip Text", s)
/*
/*  Sets the tool tip text for control n to the specified string, with
/*  style s (TTF_CENTERTIP, TTF_RTLREADING, TTF_TRACK, TTF_ABSOLUTE,
/*  TTF_TRANSPARENT), or deletes the tool tip for this control if the
/*  text argument is NULL or "".
/*
/**********************************************************************/
int setCtlToolTip(int n, char * iText, int s)
{
    int iss = TRUE;
    TOOLINFO ti;
    int safe_style = s & (TTF_CENTERTIP | TTF_RTLREADING | TTF_TRACK |
                          TTF_ABSOLUTE | TTF_TRANSPARENT);

    if (n < 0 || n >= N_CTLS || !gParams->ctl[n].inuse) {
        return FALSE;
    }

    if (gParams->ctl[n].tooltip[0] == '\0') {
        //no current tool tip for this control
        if (iText != 0 && ((char *)iText)[0] != '\0') {
           
			if (n==CTL_PREVIEW){
				ti.cbSize = sizeof(TOOLINFO);
                ti.uFlags = safe_style | TTF_IDISHWND | TTF_SUBCLASS;//0;                      // or TTF_SUBCLASS???
                ti.hwnd = MyHDlg;
                ti.uId = (UINT) CTL_PREVIEW;         // our id for this control
                ti.rect.left = gProxyRect.left;
                ti.rect.top = gProxyRect.top;
                ti.rect.right = gProxyRect.right;
                ti.rect.bottom = gProxyRect.bottom;
                ti.hinst = (HINSTANCE)hDllInstance;
			} else {
				//register a tooltip for this control
				ti.cbSize = sizeof(TOOLINFO);
				ti.uFlags = safe_style | TTF_IDISHWND | TTF_SUBCLASS;
				ti.hwnd = MyHDlg;
				ti.uId = (UINT_PTR) gParams->ctl[n].hCtl;
                // IIRC: leaving the rect undefined results in no tooltips under
                //       NT 4.0+ and XP.  Setting to 0 seems to fix the problem.
                ti.rect.left = ti.rect.top = ti.rect.right = ti.rect.bottom = 0;
				ti.hinst = (HINSTANCE)hDllInstance;
			}
            ti.lpszText = formatString((char *)iText);
            iss = (int)SendMessage(ghTT, TTM_ADDTOOL, 0, (LPARAM)&ti);
            if (gParams->ctl[n].ctlclass == CC_STANDARD || gParams->ctl[n].ctlclass == CC_SLIDER) {
                //set tooltips for buddies
                ti.uId = (UINT_PTR) gParams->ctl[n].hBuddy1;
                iss = (int)SendMessage(ghTT, TTM_ADDTOOL, 0, (LPARAM)&ti);
#if 1
                ti.uId = (UINT_PTR) gParams->ctl[n].hBuddy2;
                iss = (int)SendMessage(ghTT, TTM_ADDTOOL, 0, (LPARAM)&ti);
#endif
            }
        }
    }
    else {
        //already have a tool tip for this control
        if (iText != 0 && ((char *)iText)[0] != '\0') {
            //update text for this tool tip...
            ti.cbSize = sizeof(TOOLINFO);
            ti.uFlags = safe_style | TTF_IDISHWND;
            ti.hwnd = MyHDlg;
            ti.uId = (UINT_PTR) gParams->ctl[n].hCtl;
            //ti.rect = xxx;
            ti.hinst = (HINSTANCE)hDllInstance;
            ti.lpszText = formatString((char *)iText);
            iss = (int)SendMessage(ghTT, TTM_UPDATETIPTEXT, 0, (LPARAM)&ti);
            if (gParams->ctl[n].ctlclass == CC_STANDARD  || gParams->ctl[n].ctlclass == CC_SLIDER) {