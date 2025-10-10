}

//function added by Harald Heim on Feb 13, 2002
//int bEllipse(int x, int y, int centerx, int centery, int radiusx, int radiusy)
//{
//  return c2m(scl(x,0,fmc.X,-radiusx*fmc.Y/fmc.X,radiusx*fmc.Y/fmc.X) -centerx , scl(y,fmc.Y,0,-radiusy,radiusy) -centery ) >= fmc.Y/2 ? 1: 0;
//}



//function added by Harald Heim on Feb 13, 2002
int bRect(int x, int y, int centerx, int centery, int radiusx, int radiusy)
{
    return  x >= centerx - radiusx  && x <= centerx + radiusx && y >= centery-radiusy && y <= centery+radiusy  ? 1: 0;
    
}

//function added by Harald Heim on Feb 13, 2002
int bRect2(int x, int y, int topx, int topy, int bottomx, int bottomy)
{
    return  x >= topx && x <= bottomx && y >= topy && y <= bottomy  ? 1: 0;
    
}

//function added by Harald Heim on Feb 13, 2002
int bTriangle(int x, int y, int centerx, int centery, int radius)
{
    int calc = y * fmc.X / fmc.Y / 2;
    int topy = centery - radius;

    return  x - centerx + calc >= topy && x - centerx - calc <= -topy && y - centery < radius  ? 1 : 0;
    
}



//function added by Harald Heim on Apr 6, 2002
int refreshWindow (void)
{
    ////return UpdateWindow(fmc.hDlg);
    //InvalidateRect(gParams->ctl[n].hCtl, NULL, TRUE );
    //return UpdateWindow(gParams->ctl[n].hCtl);

    //InvalidateRect(fmc.hDlg, NULL, TRUE );
    //return UpdateWindow(fmc.hDlg);

    return RedrawWindow(
            fmc.hDlg,   // handle of entire dialog window
            NULL,   // address of structure with update rectangle, NULL=>entire client area
            NULL,   // handle of update region
            RDW_ERASE|RDW_INVALIDATE|RDW_ERASENOW|RDW_UPDATENOW     // array of redraw flags
       );
	
}

int refreshWindowNoErase (void)
{
    return RedrawWindow(
            fmc.hDlg,   // handle of entire dialog window
            NULL,   // address of structure with update rectangle, NULL=>entire client area
            NULL,   // handle of update region
            RDW_INVALIDATE     // array of redraw flags
       );

}

//function added by Harald Heim on Apr 6, 2002
int refreshRgn (INT_PTR Rgn)
{

    if (Rgn == 0) return 0;

    return RedrawWindow(
            fmc.hDlg,   // handle of entire dialog window
            NULL,   // address of structure with update rectangle, NULL=>entire client area
            (HRGN)Rgn,  // handle of update region
            RDW_ERASE|RDW_INVALIDATE|RDW_ERASENOW|RDW_UPDATENOW     // array of redraw flags
       );

}


int setCtlOrder (int n, int order){

    HWND Control;
    INT_PTR InsertAfter = (INT_PTR)order;
    
    if (n<0 || n > N_CTLS) return false;
    
    //HWND_TOP        0
    //HWND_BOTTOM     1
    //HWND_TOPMOST    -1
    //HWND_NOTOPMOST  -2
    if (order < -2 || order > 1) return false;

    //if (n == CTL_PREVIEW) Control = GetDlgItem(fmc.hDlg, 101);
    //else if (n == CTL_FRAME) Control = GetDlgItem(fmc.hDlg, IDC_PROXY_GROUPBOX);
    //else if (n == CTL_PROGRESS) Control = GetDlgItem(fmc.hDlg, IDC_PROGRESS1);
    //else 
	Control = gParams->ctl[n].hCtl;
    
    if (Control==NULL) return false;

    SetWindowPos(Control, (HWND)InsertAfter, 0, 0, 0, 0, SWP_FRAMECHANGED|SWP_NOACTIVATE|SWP_NOSIZE|SWP_NOMOVE);
    InvalidateRect(gParams->ctl[n].hCtl, NULL, TRUE );
	UpdateWindow(gParams->ctl[n].hCtl);
    
    return true;
}


//function added by Harald Heim on Apr 6, 2002
int refreshCtl (int n, ... )
{
	//Optional argument
	int nonClient = false;
	va_list argptr;
	va_start(argptr, n); //Start after variable n
    nonClient = va_arg(argptr, long);
    va_end(argptr);


    if (gParams->ctl[n].hCtl==NULL && n != CTL_PREVIEW) return false;

    
    if (gParams->ctl[n].ctlclass == CC_TAB){
		int i;

		InvalidateRect(gParams->ctl[n].hCtl, NULL, TRUE );
		UpdateWindow(gParams->ctl[n].hCtl);
		if (nonClient) SendMessage(gParams->ctl[n].hCtl, WM_NCPAINT, 1, 0);

		for (i = 0; i < N_CTLS; ++i) {
			if (gParams->ctl[i].inuse && gParams->ctl[i].tabControl == n &&  gParams->ctl[i].tabSheet == gParams->ctl[n].val){
				InvalidateRect(gParams->ctl[i].hCtl, NULL, TRUE );
				UpdateWindow(gParams->ctl[i].hCtl);
				if (nonClient) SendMessage(gParams->ctl[i].hCtl, WM_NCPAINT, 1, 0);
				if (gParams->ctl[i].ctlclass == CC_STANDARD || gParams->ctl[i].ctlclass == CC_SLIDER){
					InvalidateRect(gParams->ctl[i].hBuddy1, NULL, TRUE );
					UpdateWindow(gParams->ctl[i].hBuddy1);
					SetWindowPos(gParams->ctl[i].hBuddy1, NULL, 0, 0, 0, 0,
                        SWP_FRAMECHANGED|SWP_NOZORDER|SWP_NOACTIVATE|SWP_NOSIZE|SWP_NOMOVE);
					if (nonClient) SendMessage(gParams->ctl[n].hBuddy1, WM_NCPAINT, 1, 0);
					
					InvalidateRect(gParams->ctl[i].hBuddy2, NULL, TRUE );
					UpdateWindow(gParams->ctl[i].hBuddy2);
					if (nonClient) SendMessage(gParams->ctl[n].hBuddy2, WM_NCPAINT, 1, 0);
				}
			}	
		}

	/*} else if (gParams->ctl[n].ctlclass == CC_BITMAP){

		setCtlPos(n, gParams->ctl[n].xPos+1, -1, -1, -1);
		setCtlPos(n, gParams->ctl[n].xPos-1, -1, -1, -1);
		InvalidateRect(gParams->ctl[n].hCtl, NULL, TRUE );
		UpdateWindow(gParams->ctl[n].hCtl);
    */
	} else if (n == CTL_PREVIEW){

		ShowOutputBuffer(globals, fmc.hDlg, fDragging);

	} else {
		////return UpdateWindow(fmc.hDlg);
        HWND Control;

        if (gParams->ctl[n].ctlclass == CC_STANDARD || gParams->ctl[n].ctlclass == CC_SLIDER){
			InvalidateRect(gParams->ctl[n].hBuddy1, NULL, TRUE );
			UpdateWindow(gParams->ctl[n].hBuddy1);
			SetWindowPos(gParams->ctl[n].hBuddy1, NULL, 0, 0, 0, 0, SWP_FRAMECHANGED|SWP_NOZORDER|SWP_NOACTIVATE|SWP_NOSIZE|SWP_NOMOVE);
			if (nonClient) SendMessage(gParams->ctl[n].hBuddy1, WM_NCPAINT, 1, 0);
			InvalidateRect(gParams->ctl[n].hBuddy2, NULL, TRUE );
			UpdateWindow(gParams->ctl[n].hBuddy2);
			if (nonClient) SendMessage(gParams->ctl[n].hBuddy2, WM_NCPAINT, 1, 0);
		}
		
        //if (n == CTL_PROGRESS) Control = GetDlgItem(fmc.hDlg, IDC_PROGRESS1);
        //else 
		Control = gParams->ctl[n].hCtl;

        InvalidateRect(Control, NULL, TRUE );
		UpdateWindow(Control);
		if (nonClient) SendMessage(Control, WM_NCPAINT, 1, 0);
		
	}

	return true;

}

int refreshItems(int n){

	if (n<0 || n > N_CTLS || gParams->ctl[n].ctlclass != CC_LISTVIEW) return false;

	return (int) ListView_RedrawItems(gParams->ctl[n].hCtl,0,getCtlItemCount(n));
}

int lockCtl (int n)
{
    return (int)SendMessage(gParams->ctl[n].hCtl,WM_SETREDRAW,false,0);//LockWindowUpdate(gParams->ctl[n].hCtl);
}

int unlockCtl (int n)
{
    return (int)SendMessage(gParams->ctl[n].hCtl,WM_SETREDRAW,true,0);//LockWindowUpdate(NULL);
}


//function added by Harald Heim on Apr 6, 2002
int lockWindow (int c)
{
    if (c==0)
        return LockWindowUpdate(NULL);
	else
        return LockWindowUpdate(fmc.hDlg);
}


//function added by Harald Heim on Apr 2, 2002
int setZoom (int n, int m)
{
    if (!fmc.doingProxy) return 0;
    
    if (n<1 && n!=-888) n=1;
    else if (n>MAX_SCALE_FACTOR) n=MAX_SCALE_FACTOR;

    //function added by Harald Heim on Jun 13, 2002
    fmc.scaleFactor = n;
    
    return sub_setZoom(fmc.hDlg, n, m); //,1);
}

int setZoomEx (int n, int m)
{
#ifdef HIGHZOOM

    if (fmc.doingProxy==FALSE) return 0;
    
    if (n<1 && n!=-888) n=1;
    else if (n>16) n=16;

	if (n>1) m=1; //No enlargement for if < 100%

	if (m<1) m=1;
    else if (m>16) m=16;

    //function added by Harald Heim on Jun 13, 2002
    fmc.scaleFactor = n;
	//fmc.enlargeFactor = m; //is set in AfhFMUI
    
    return sub_setZoom(fmc.hDlg, n, m);
#else

	return 0;

#endif
}



int set_array_mode(int mode)
{

	if (mode == 1){	
#ifndef APP
		if (gStuff->bufferProcs && 
			gStuff->bufferProcs->allocateProc &&
			gStuff->bufferProcs->lockProc &&
			gStuff->bufferProcs->unlockProc &&
			gStuff->bufferProcs->freeProc){
				arrayMode = 1;
				return true;
		} else 
#endif
        {
			arrayMode = 0;
			return false;
		}
	} else {
		arrayMode = 0;
		return true;
	}
	
	return false;
}


#ifndef APP

INT_PTR allocHost (int size)
{
    BufferID bufferid;

    if (AllocateBuffer(size, &bufferid) == noErr){   
        return (INT_PTR)bufferid;
    } else {
        return (INT_PTR)NULL;
    }
}
