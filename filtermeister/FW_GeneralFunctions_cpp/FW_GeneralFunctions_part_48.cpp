
int setCtlTextW(int n, WCHAR * iText){

    if (n < 0 || n >= N_CTLS || !gParams->ctl[n].inuse) {
        return FALSE;
    }

    return SetWindowTextW(gParams->ctl[n].hCtl, iText);
}



/**********************************************************************/
/* setCtlText(n, "text")
/*
/*  Sets the text label or content for control n to the given text string.
/*
/**********************************************************************/
int setCtlText(int n, char * iText)
{
    // Could check nargs and pass to setCtlTextv() if nargs > 2 ????
    RECT    wRect;

    if (n < 0 || n >= N_CTLS || !gParams->ctl[n].inuse) {
        return FALSE;
    }

    if (iText) {
        strncpy(gParams->ctl[n].label, (char *)iText, MAX_LABEL_SIZE+1);
        gParams->ctl[n].label[MAX_LABEL_SIZE] = '\0';   //ensure null-terminated.
    }
    else {
        //No text string
        gParams->ctl[n].label[0] = '\0';
    }


#if SKIN
	if (isSkinActive() && isSkinCtl(n) && gParams->ctl[n].ctlclass != CC_STANDARD){
		if (ctlEnabledAs(n)) skinDraw(n,0);
		return true;
	}
#endif

    switch (gParams->ctl[n].ctlclass) {
	
	case CC_STATICTEXT:
    case CC_CHECKBOX:
    case CC_RADIOBUTTON:
        //SetWindowText(gParams->ctl[n].hCtl, formatString(gParams->ctl[n].label));
		SetWindowText(gParams->ctl[n].hCtl, formatString((char *)iText));

        if (gParams->ctl[n].bkColor == (COLORREF) -1) {
            //It seems these controls with a transparent
            //background color do not erase the background
            //when they receive a WM_SETTEXT message
            //(maybe because we are handling WM_CTLCOLOR*
            //and setting the background to transparent??), so
            //we brute-force it... (this may be overkill??)

            if (gParams->ctl[n].enabled){
			//if (ctlEnabledAs(n)){
                int retval;
		        GetWindowRect(gParams->ctl[n].hCtl, &wRect); 
		        ScreenToClient (fmc.hDlg, (LPPOINT)&wRect);
		        ScreenToClient (fmc.hDlg, (LPPOINT)&(wRect.right));
                retval = RedrawWindow(
				        fmc.hDlg,   // handle of entire dialog window
				        &wRect, // address of structure with update rectangle
				        NULL,   // handle of update region
				        RDW_ERASE|RDW_INVALIDATE|RDW_ERASENOW|RDW_UPDATENOW // array of redraw flags
		           );
                return retval;
	        }
        }
        
        return TRUE;
        break;

    case CC_STANDARD:
	case CC_SLIDER:
        //specific to default scrollbars...
        {//scope
            //Resize the static text buddy to exactly fit the text,
            //to minimize problems with overlapping controls...
            SIZE  size = {60,14};
            RECT  r;
            HWND  hWnd = GetDlgItem(fmc.hDlg, IDC_BUDDY2+n);
            HDC   hDC = GetDC(hWnd);
            HFONT hOldFont = (HFONT)SelectObject(hDC, gParams->ctl[n].hFont);
            LPSTR p = formatString(gParams->ctl[n].label);
            //CAUTION: p points to volatile storage (which will
            //be overwritten by calls to Info(), etc.)!!
            GetTextExtentPoint32(hDC, p, (int)strlen(p), &size);
            if (gParams->ctl[n].sb.maxw2 > 0 && size.cx > gParams->ctl[n].sb.maxw2)
				gParams->ctl[n].sb.w2 = gParams->ctl[n].sb.maxw2;
			else
				gParams->ctl[n].sb.w2 = size.cx;    //text width in pixels
			if (gParams->ctl[n].sb.maxh2 > 0 && size.cy > gParams->ctl[n].sb.maxh2)
				gParams->ctl[n].sb.h2 = gParams->ctl[n].sb.maxh2;
			else
				gParams->ctl[n].sb.h2 = size.cy;    //text height in pixels
            computeBuddy2Pos(n,
                             gParams->ctl[n].xPos,
                             gParams->ctl[n].yPos,
                             gParams->ctl[n].width,
                             gParams->ctl[n].height,
                             &r);
            SetWindowPos(gParams->ctl[n].hBuddy2, NULL,
                         r.left,    //x
                         r.top,     //y
                         r.right,   //w
                         r.bottom,  //h
                         SWP_NOZORDER | SWP_NOACTIVATE | SWP_NOCOPYBITS);
            //Cleanup...
            SelectObject(hDC, hOldFont);
            ReleaseDC(hWnd, hDC);
            //Set the actual control text...
            SetWindowText(hWnd, p);

            //It seems a static text control does not erase the
            //background when it receives a WM_SETTEXT message
            //(maybe because we are handling WM_CTLCOLORSTATIC
            //and setting the background to transparent??), so
            //we brute-force it... (this may be overkill??)

			if (!ctlUpdateLocked){
				//if (gParams->ctl[n].enabled){
				if (ctlEnabledAs(n)){
					GetWindowRect(hWnd, &wRect); 
					ScreenToClient (fmc.hDlg, (LPPOINT)&wRect);
					ScreenToClient (fmc.hDlg, (LPPOINT)&(wRect.right));
					return RedrawWindow(
		#if 0
							//just updating the control window doesn't work :(
							hWnd,   // handle of control window
		#else
							fmc.hDlg,   // handle of entire dialog window
		#endif
		#if 1
							&wRect, // address of structure with update rectangle
		#else
							NULL,   // address of structure with update rectangle, NULL=>entire client area
		#endif
							NULL,   // handle of update region
							RDW_ERASE|RDW_INVALIDATE|RDW_ERASENOW|RDW_UPDATENOW     // array of redraw flags
					   );
				}
			}

        }//scope
        break;

    case CC_EDIT:
        //SetWindowText(gParams->ctl[n].hCtl, formatString(gParams->ctl[n].label));
		SetWindowText(gParams->ctl[n].hCtl, formatString((char *)iText));

        //for an Edit control, we need to call UpdateWindow() to force a
        // WM_PAINT if we want an immediate update
        if (gParams->ctl[n].enabled){
		//if (ctlEnabledAs(n)){
			return UpdateWindow(gParams->ctl[n].hCtl);
		} else
			return TRUE;
        break;

    case CC_COMBOBOX:
        SendMessage(gParams->ctl[n].hCtl, CB_RESETCONTENT, 0, 0);
		{
            //The following old code causes a problem when compiled as a 64bit plugin
			//Info("%d\n%s",*iText,iText);
            //if (*iText!=0) {
        #if 0      
                //#if _WIN64 || _MSC_VER >= 1400

                    int i;
			        char *a;
			        char copy[MAX_LABEL_SIZE+1];
                    strncpy(copy, iText, MAX_LABEL_SIZE+1);

                    /*
                    a = strtok(copy,"\n");
                    for (i=0;i>=0;i++){
				        SendMessage(gParams->ctl[n].hCtl, CB_ADDSTRING, 0, (LPARAM)a); //formatString(a)
				        a = strtok(NULL,"\n");
				        if (a==0)break;
			        }*/

                    a = strtok(copy,"\n"); 
                    while(a!=NULL){
                        SendMessage(gParams->ctl[n].hCtl, CB_ADDSTRING, 0, (LPARAM)a); //formatString(a)
                        a=strtok(NULL,"\n");
                    }
        #endif
                //#else
                    char *p, *q, q_save;
                    char* copy;
    			    
                    copy = (char *)malloc(strlen(iText)+1);
                    strcpy(copy,iText);

                    //p = gParams->ctl[n].label;
			        //p = iText; //Allow combo box text that is longer than 1023 bytes 
                    p = copy; 
			        while (*p) {
                        //for (q = p; *q != '\0' && *q != '\n'; q++)
                        //    ;
                        q = p; while(*q != '\0' && *q != '\n') q++;

                        q_save = *q;
                        *q = '\0';
                        SendMessage(gParams->ctl[n].hCtl, CB_ADDSTRING, 0, (LPARAM)p); //formatString(p)
				        *q = q_save;
                        p = q;
                        if (*p) p++;
                    } //while 

                    free (copy);
    			
                //#endif
                
            //}
			
        }
        // set the control value to set the initial selection
        setCtlVal(n, gParams->ctl[n].val);
        break;

    case CC_LISTBOX:
    case CC_LISTBAR:
        SendMessage(gParams->ctl[n].hCtl, LB_RESETCONTENT, 0, 0);
        {
            int count=0;
    #if 0            
            //#if _WIN64 || _MSC_VER >= 1400

			    ///int i;
			    char *a;
			    char copy[65535]; //MAX_LABEL_SIZE+1
			    strncpy(copy, (char *)iText, 65535); //MAX_LABEL_SIZE+1

			    /*a = strtok(copy,"\n");
			    for (i=0;i>=0;i++){
				    SendMessage(gParams->ctl[n].hCtl, LB_ADDSTRING, 0, (LPARAM)a);//formatString(a)
				    a = strtok(NULL,"\n");
				    if (a==0)break;
                    count++;
			    }*/

                a = strtok(copy,"\n"); 
                while(a!=NULL){
                    SendMessage(gParams->ctl[n].hCtl, CB_ADDSTRING, 0, (LPARAM)a); //formatString(a)
                    a=strtok(NULL,"\n");
                    count++;
                }

             //#else
    #endif    
                char *p, *q, q_save;
                char* copy;
                int chars, maxchars=0;
                SIZE sz = {0,0};
                //char str[512];
                int pxsize=0; //i,
                //POINT pt;

                copy = (char *)malloc(strlen(iText)+1);
                strcpy(copy,iText);

                //p = gParams->ctl[n].label;
			    //p = (char *)iText; //Allow list box text that is longer than 1023 bytes 
                p = copy; 
                while (*p) {
                    //for (q = p; *q != '\0' && *q != '\n'; q++)
                    //    ;
                    q = p; while(*q != '\0' && *q != '\n') q++;

                    q_save = *q;
                    *q = '\0';
                    SendMessage(gParams->ctl[n].hCtl, LB_ADDSTRING, 0, (LPARAM)p); //(LPARAM)formatString(p));

                    if (gParams->ctl[n].style & WS_HSCROLL){
                        chars = (int)strlen(p);
                        if (chars>maxchars) {
                            //TEXTMETRIC tm; 
                            maxchars = chars;
                            //GetTextExtentPoint32(GetDC(gParams->ctl[n].hCtl),(LPCTSTR)p,chars,&sz);
                            
                            //Info ("%d",GetMapMode(GetDC(gParams->ctl[n].hCtl)));
                            //pt.x = sz.cx;
                            //pt.y = sz.cy;
                            //LPtoDP(GetDC(gParams->ctl[n].hCtl),&pt,1);
                            //GetTextMetrics(GetDC(gParams->ctl[n].hCtl), &tm); 
                            //sz.cx -= tm.tmOverhang;
                        }
                    }

                    *q = q_save;
                    p = q;
                    if (*p) p++;