    if ( (n < 0 || n >= N_CTLS || !gParams->ctl[n].inuse) ) { //&& n != CTL_PREVIEW && n != CTL_PROGRESS && n != CTL_ZOOM
        return FALSE;
    }
    
    //Get dialog size if necessary
    if (dialogWidth == -1 || dialogHeight == -1){
        RECT rcParent;
        GetClientRect(fmc.hDlg, &rcParent);
        if (dialogWidth == -1) dialogWidth = PixelsToHDBUs(rcParent.right);
	    if (dialogHeight == -1) dialogHeight = PixelsToVDBUs(rcParent.bottom);
    }


	if ( (gParams->ctl[n].inuse) && gParams->ctl[n].anchor > 0){ // || n==CTL_PREVIEW || n==CTL_PROGRESS || n==CTL_ZOOM

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

        } else {
*/
	        if (gParams->ctl[n].anchor & 16) //ANCHOR_HCENTER
                gParams->ctl[n].distWidth = gParams->ctl[n].xPos - dialogWidth/2;
            else
                gParams->ctl[n].distWidth = dialogWidth - (gParams->ctl[n].xPos + gParams->ctl[n].width);

            if (gParams->ctl[n].anchor & 32) //ANCHOR_VCENTER
	            gParams->ctl[n].distHeight = gParams->ctl[n].yPos - dialogHeight/2;
            else
                gParams->ctl[n].distHeight = dialogHeight - (gParams->ctl[n].yPos + gParams->ctl[n].height);
        //}
	}

    return true;

}



int lockCtlScaling(int lock){

    gParams->scaleLock = lock;

    return true;
}


int scaleCtls(int dialogWidth, int dialogHeight){

    int n;

    if (!fmc.doingProxy) return false;

    //Get dialog size if necessary
    if (dialogWidth == -1 || dialogHeight == -1){
        RECT rcParent;
        GetClientRect(fmc.hDlg, &rcParent);
        if (dialogWidth == -1) dialogWidth = PixelsToHDBUs(rcParent.right);
	    if (dialogHeight == -1) dialogHeight = PixelsToVDBUs(rcParent.bottom);
    }


	lockWindow(1);
	
	for (n = 0;  n < N_CTLS;  n++)
    {
		if ( (gParams->ctl[n].inuse) && gParams->ctl[n].anchor > 0){ //|| n==CTL_PREVIEW || n==CTL_PROGRESS || n==CTL_ZOOM
			
			int newx = gParams->ctl[n].xPos;
			int newy = gParams->ctl[n].yPos;
			int neww = gParams->ctl[n].width;
			int newh = gParams->ctl[n].height;

            //if (n==CTL_PREVIEW) Info ("%d, %d",gParams->ctl[n].distWidth,gParams->ctl[n].distHeight);
			
			//Scale horizontal
			if ( gParams->ctl[n].anchor & ANCHOR_HCENTER ) 
                newx  = dialogWidth/2 +  gParams->ctl[n].distWidth;
            else if ( (gParams->ctl[n].anchor & ANCHOR_LEFT) && (gParams->ctl[n].anchor & ANCHOR_RIGHT)) 
				neww = neww + ((dialogWidth - (gParams->ctl[n].xPos + neww)) - gParams->ctl[n].distWidth);
			//Move horizontal
			else if ( !(gParams->ctl[n].anchor & ANCHOR_LEFT) && (gParams->ctl[n].anchor & ANCHOR_RIGHT)) 
				newx  = newx + ((dialogWidth - (gParams->ctl[n].xPos + neww)) - gParams->ctl[n].distWidth);

			//Scale vertical
			if ( gParams->ctl[n].anchor & ANCHOR_VCENTER ) 
                newy  = dialogHeight/2 +  gParams->ctl[n].distHeight;
            if ( (gParams->ctl[n].anchor & ANCHOR_TOP) && (gParams->ctl[n].anchor & ANCHOR_BOTTOM)) 
				newh = newh + ((dialogHeight - (gParams->ctl[n].yPos + newh)) - gParams->ctl[n].distHeight);
			//Move vertical
			else if ( !(gParams->ctl[n].anchor & ANCHOR_TOP) && (gParams->ctl[n].anchor & ANCHOR_BOTTOM))
				newy  = newy + ((dialogHeight - (gParams->ctl[n].yPos + newh)) - gParams->ctl[n].distHeight);
			
			//Do not make preview smaller to avoid crash
			if (n==CTL_PREVIEW){
				if (neww<10) neww=10;
				if (newh<10) newh=10;
			}

			//if (n==CTL_PROGRESS) Info ("scale");

            /*if ( neww == gParams->ctl[n].width && newh == gParams->ctl[n].height) 
                setCtlPos(n,newx,newy,-1,-1);
            else*/

            /*if (gParams->ctl[n].ctlclass == CC_COMBOBOX)
                setCtlPos(n,newx,newy,-1,15); //less flicker when resizing
            else*/
				setCtlPos(n,newx,newy,neww,newh);
			
		}
	}

	lockWindow(0);

    return true;

}






/*
HDC PixelhDC=0;
HDC BufferhDC=0;
HBITMAP hbmBuffer=0;
HFONT fontDB=0;


int startSetPixel (int n)
{
        //RECT rcCtl;
        //GetWindowRect(gParams->ctl[n].hCtl, &rcCtl);

        if (n==-1)
            PixelhDC = GetDC(fmc.hDlg);
        else
            PixelhDC = GetDC(gParams->ctl[n].hCtl);
        
        //BufferhDC = CreateCompatibleDC(PixelhDC);
        //hbmBuffer = CreateCompatibleBitmap(BufferhDC, rcCtl.right-rcCtl.left, rcCtl.bottom-rcCtl.top);
        //SelectObject(BufferhDC, hbmBuffer);

        return true;

}

int endSetPixel (int n)
{
        //RECT rcCtl;
        //GetWindowRect(gParams->ctl[n].hCtl, &rcCtl);

        //BitBlt(PixelhDC, 0, 0, rcCtl.right-rcCtl.left, rcCtl.bottom-rcCtl.top, BufferhDC, 0, 0, SRCCOPY);
        //DeleteObject (hbmBuffer);
        //DeleteDC (BufferhDC);

        if (n==-1)
            ReleaseDC (fmc.hDlg, PixelhDC);
        else    
            ReleaseDC (gParams->ctl[n].hCtl, PixelhDC);
        
        return true;
        
}

int setPixel (int x, int y, int value)
{   
            
        if (PixelhDC==0) return false;

        return SetPixelV (PixelhDC,x,y,(COLORREF) value);
        //return SetPixelV (BufferhDC,x,y,(COLORREF) value);

}
*/



int countProcessors(void){

	SYSTEM_INFO SystemInfo;
	
	GetSystemInfo(&SystemInfo);
 
	return (int)SystemInfo.dwNumberOfProcessors;

    //return 1;
}


void doMultiThreading(LPTHREAD_START_ROUTINE ThreadFunction, int useMT, int useSync){
	
	if (useMT)  //Use multithreading
		mt.totalcpu = countProcessors();
	else 
		mt.totalcpu  = 1;

	if (mt.totalcpu > 1) { //More than one processor
        
		HANDLE * hThread = NULL;
		DWORD * dwThread = NULL;
		INT_PTR index;

		hThread = (HANDLE *)malloc(sizeof(HANDLE)*mt.totalcpu);
		dwThread = (DWORD *)malloc(sizeof(DWORD)*mt.totalcpu);

		//if (mt.totalcpu > MAXCPU) mt.totalcpu = MAXCPU; // Up to 16 processors possible for now

		if (hThread && dwThread){
			if (useSync) createSync(mt.totalcpu);
			for (index=1; index < mt.totalcpu; index++) {
				hThread[index-1] = CreateThread( NULL, 0, ThreadFunction, (LPVOID)index, 0, &dwThread[index-1] ); //&dwThread[index-1]
			} 
		}
		ThreadFunction(0);

		if (hThread && dwThread){
			//Wait for all threads to finish
			WaitForMultipleObjects( mt.totalcpu-1, hThread, TRUE, INFINITE );
			for (index=0; index<mt.totalcpu-1; index++) CloseHandle(hThread[index]); 
			free(hThread);
			free(dwThread);
			if (useSync) deleteSync();
		}
		
	} else { //Only one Processor

		ThreadFunction(0);

	}   

}


/****** THREAD APIs ******/

#if 0

INT_PTR triggerThread(int n, int event, int previous ){

    HANDLE hThread;
    DWORD dwThread;
    FMcontext *pfmc_copy;

    if ( (n < 0 || n >= N_CTLS) && event != FME_CUSTOMEVENT ) return false;

    pfmc_copy = malloc(sizeof(FMcontext));
    if (!pfmc_copy) return false;   //couldn't allocate FMC as TLS
	

    //could do this in the thread, so it can be multitasked? 
    //but then it's hard to pass other params	
#if 0
    memcpy(pfmc_copy,&fmc,sizeof(FMcontext)); //Copy values of original FMC
#else
    *pfmc_copy = fmc; //Copy values of original FMC 
#endif

    //Set new values
    pfmc_copy->n = n;
    pfmc_copy->e = event;
    pfmc_copy->previous = previous;
    pfmc_copy->ctlMsg = event;
    pfmc_copy->ctlCode = 0;

    if (event==FME_CUSTOMEVENT)
    	pfmc_copy->ctlClass = CC_UNUSED;
    else
        pfmc_copy->ctlClass = gParams->ctl[n].ctlclass;

    
    //Do call
    //RetVal = X86_call_fmf1(gParams->CB_onCtl, (int)&fmc);
    hThread = CreateThread( NULL,
                            0, 
                            (LPTHREAD_START_ROUTINE)OnCtl,
                            (LPVOID)pfmc_copy, 
                            0, 