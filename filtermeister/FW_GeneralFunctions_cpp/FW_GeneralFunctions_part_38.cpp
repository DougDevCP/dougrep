#if 0 //InterlockedIncrement requires Win98+ or NT 4.0+ for proper return value
        mySlot = InterlockedIncrement(&NextSlot) - 1;
#else
        mySlot = InterlockedExchangeAdd(&NextSlot, 1); //should work on all cpus and Win versions?
#endif
        if (mySlot >= NumberOfThreads) return false; //an unknown thread entered waitForSync??
    }//if

    lpThreadArray[mySlot] = syncnr;  //record our current syncnr


    // Wait until all threads are at the current syncnr or greater.

    for (i=0; i<NumberOfThreads && !timeelapsed; i++) {
        //wait until thread i reaches syncnr or greater...
        //(note that an as-yet empty slot will have a syncnr value of 0)
        while(lpThreadArray[i] < syncnr) {
            //check for timeout
            if (timeout!=INFINITE && clock()-starttime >= timeout) {
                timeelapsed=true; 
                break;
            }//if timeout
            //yield this timeslice
            Sleep(0);
        }//while
    }//for

    //Either we timed out, or all threads are at current syncnr or greater...
    return !timeelapsed;

}//waitForSync


int deleteSync(void){

    NumberOfThreads = 0;

    if (lpThreadArray!=NULL){
        free (lpThreadArray);
        lpThreadArray=NULL;
    }

    return true;
}//deleteSync

#elif 1 //#7: Let syncnr be TLS rather than a param ******************************

int NumberOfThreads=0;
int *lpThreadArray=NULL;
int NextSlot=0;


int createSync(int number){

    if (number<2) return false;
        
    NumberOfThreads = number;
    NextSlot = 0;
    lpThreadArray = calloc(number,sizeof(*lpThreadArray));
    if (!lpThreadArray) return false;

    return true;
}//createSync


int waitForSync(int unused, int timeout){

    THREAD_LOCAL static int mySlot = -1;
    THREAD_LOCAL static int syncnr = 0;
    int i;
    BOOL timeelapsed=false;
    int starttime = clock(); 
    
    if (NumberOfThreads<2 || lpThreadArray==NULL) return false; //In case Sync was not initialized correctly

    syncnr++;   //increment current syncnr level (must always be > 0)
    if(syncnr <= 0) return false;   //oops

    //first time around, assign next slot in ThreadArray to this thread...

    if (mySlot == -1) {
        //not yet assigned a slot...
#if 0 //InterlockedIncrement requires Win98+ or NT 4.0+ for proper return value
        mySlot = InterlockedIncrement(&NextSlot) - 1;
#else
        mySlot = InterlockedExchangeAdd(&NextSlot, 1); //should work on all cpus and Win versions?
#endif
        if (mySlot >= NumberOfThreads) return false; //an unknown thread entered waitForSync??
    }//if

    lpThreadArray[mySlot] = syncnr;  //record our current syncnr


    // Wait until all threads are at the current syncnr or greater.

    for (i=0; i<NumberOfThreads && !timeelapsed; i++) {
        //wait until thread i reaches syncnr or greater...
        //(note that an as-yet empty slot will have a syncnr value of 0)
        while(lpThreadArray[i] < syncnr) {
            //check for timeout
            if (timeout!=INFINITE && clock()-starttime >= timeout) {
                timeelapsed=true; 
                break;
            }//if timeout
            //yield this timeslice
            Sleep(0);
        }//while
    }//for

    //Either we timed out, or all threads are at current syncnr or greater...
    return !timeelapsed;

}//waitForSync


int deleteSync(void){

    NumberOfThreads = 0;

    if (lpThreadArray!=NULL){
        free (lpThreadArray);
        lpThreadArray=NULL;
    }

    return true;
}//deleteSync

#endif //Sync API versions ********************************************



LPCRITICAL_SECTION lpcs = NULL;

void createSection(){
	if (lpcs) deleteSection();
	lpcs = (LPCRITICAL_SECTION)malloc(sizeof(CRITICAL_SECTION));
	if (lpcs)
		InitializeCriticalSection(lpcs);
}

void enterSection(){
	if (!lpcs) return;
	EnterCriticalSection(lpcs);
}

void leaveSection(){
	if (!lpcs) return;
	LeaveCriticalSection(lpcs);
}

void deleteSection(){	
	if (!lpcs) return;
	DeleteCriticalSection(lpcs);
	free(lpcs);
	lpcs = NULL;
}



/****** EVENT API'S ******/

int triggerEvent(int n, int event, int previous ){

        int RetVal=0;

        //Save values
        int n2 = fmc.n;
        int e2 = fmc.e;
        int previous2 = fmc.previous;
        int ctlMsg2 = fmc.ctlMsg;
        int ctlCode2 = fmc.ctlCode;
        int ctlClass2 = fmc.ctlClass;
    

		if ( (n < 0 || n >= N_CTLS) && event != FME_CUSTOMEVENT ) return false;


        //Set new values
        fmc.n = n;
        fmc.e = event;
        fmc.previous = previous;
        fmc.ctlMsg = event;//WM_SIZE;
        fmc.ctlCode = 0;

		if (event==FME_CUSTOMEVENT)
			fmc.ctlClass = CC_UNUSED;
		else
			fmc.ctlClass = gParams->ctl[n].ctlclass;
        
		//Do call
        //RetVal = OnCtl((FMcontext * const)&fmc, n, event, previous);
		RetVal = call_FFP(FFP_OnCtl, (INT_PTR)&fmc);

        //Restore old values
        fmc.n = n2;
        fmc.e = e2;
        fmc.previous = previous2;
        fmc.ctlMsg = ctlMsg2;
        fmc.ctlCode = ctlCode2;
        fmc.ctlClass = (CTLCLASS)ctlClass2;

        return RetVal;
}


int doEvents(void) {  

        MSG msg; 
        
        while ( PeekMessage(&msg, fmc.hDlg, 0, 0, PM_REMOVE ) )  {  
            TranslateMessage(&msg);
            DispatchMessage(&msg);
            if (msg.message == WM_QUIT) break; // controlled by windows
            if (msg.message == WM_CLOSE && msg.hwnd == fmc.hDlg) break;
            //if( m_bShutDown != FALSE ) break; // controlled by program
        }

        return true;
}


#if APP

//int cachedScaleFactor;
int cachedFMC[4];

int requestRect (int inLeft, int inTop, int inRight, int inBottom, int scaleFactor)
{

	if (fmc.doingProxy){

		//Only for 100% zoom
		if (scaleFactor != 1) return false; //fmc.scaleFactor==1 || 

		//cachedScaleFactor = fmc.scaleFactor;
		//fmc.scaleFactor = scaleFactor;
		//populateFMC(IMAGE,IMAGEOUT);

		cachedFMC[0]=fmc.x_start;
		cachedFMC[1]=fmc.y_start;
		cachedFMC[2]=fmc.x_end;
		cachedFMC[3]=fmc.y_end;

		//Set values necessary for src()
		gStuff->inData = Array[IMAGE];
		MyAdjustedInData = gStuff->inData;
		fmc.x_start = 0;
		fmc.y_start = 0;
		fmc.x_end = fmc.X = ArrayX[IMAGE];
		fmc.y_end = fmc.Y = ArrayY[IMAGE];
		fmc.srcRowDelta = ArrayX[IMAGE]*ArrayZ[IMAGE]*ArrayBytes[IMAGE];
		fmc.srcColDelta = ArrayZ[IMAGE];
	}
    
	//Info ("%d x %d",fmc.X,fmc.Y);
	return true;
}

int restoreRect (void)
{

	if (fmc.doingProxy){

		//if (fmc.scaleFactor==1) return false; 

		//fmc.scaleFactor = cachedScaleFactor;
		//populateFMC(PREVIEW,PREVIEWOUT);
		//Info ("%d,%d - %d,%d",fmc.x_start,fmc.y_start,fmc.x_end,fmc.y_end);

		gStuff->inData = Array[PREVIEW];
		MyAdjustedInData = gStuff->inData;
		fmc.x_start = cachedFMC[0];
		fmc.y_start = cachedFMC[1];
		fmc.x_end = cachedFMC[2];
		fmc.y_end = cachedFMC[3];
		fmc.srcRowDelta = ArrayX[PREVIEW]*ArrayZ[PREVIEW]*ArrayBytes[PREVIEW];
		fmc.srcColDelta = ArrayZ[PREVIEW];
	}
    
	//Info ("%d x %d",fmc.X,fmc.Y);
	return true;
}

#else

	  
int restoreRect (void){

    //Only run in proxy mode
    if (!fmc.doingProxy) return false;

    //imageOffset.x += 1;
    //imageOffset.y += 1;

    GetProxyItemRect (fmc.hDlg, &gProxyRect);
    SetupFilterRecordForProxy (globals, (long *)&fmc.scaleFactor, fmc.scaleFactor, &imageOffset);


    //Restore Values
