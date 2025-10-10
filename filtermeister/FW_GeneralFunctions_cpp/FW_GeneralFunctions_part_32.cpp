        if (file==NULL) return FALSE;

        // Determine file length...
        fseek(file, 0, SEEK_END);
        cb = ftell(file);
        rewind(file);

        if (allocArray (arraynr,cb,0,0,1))
            fread((void *)getArrayAddress(arraynr), 1, cb, file);

        fclose(file);
        return true;

    } 

    // Check for size of resource and allocate array
    if ((cb=SizeofResource((HMODULE)hDllInstance, hRes)) > 0)
    {
        allocArray (arraynr,cb,0,0,1);
    } else 
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
    }

    //Copy the ressource data into the array
    memcpy (Array[arraynr],hResPointer,cb);


    return TRUE;
}


int copyResToArrayEx (char* restype, char* resname, int arraynr, int X, int Y, int Z)
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
        if (file==NULL) return FALSE;

        // Determine file length...
        fseek(file, 0, SEEK_END);
        cb = ftell(file);
        rewind(file);

		
		if (allocArray (arraynr,X,Y,Z,1))
			fread((void *)getArrayAddress(arraynr), 1, min(X*Y*Z,cb), file);
		
        fclose(file);

		if (X*Y*Z==cb)
			return true;
		else
			return false;
    } 

    // Check for size of resource and allocate array
    if ((cb=SizeofResource((HMODULE)hDllInstance, hRes)) > 0){
        allocArray (arraynr,X,Y,Z,1);
    } else 
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
    }

    //Copy the ressource data into the array
    memcpy (Array[arraynr],hResPointer,min(X*Y*Z,cb));


    return TRUE;
}


int getScreenSize(int s, int fAbs){

	int xDiff=0,yDiff=0;
	RECT workArea;
	
	SystemParametersInfo(SPI_GETWORKAREA, 0, &workArea, 0); //Work area

	if (!fAbs){
		RECT rcClient, rcWind; 
		//Get difference between window and client area
        GetClientRect(fmc.hDlg, &rcClient); 
        GetWindowRect(fmc.hDlg, &rcWind); 
        xDiff = (rcWind.right - rcWind.left) - (rcClient.right-rcClient.left); 
        yDiff = (rcWind.bottom - rcWind.top) - (rcClient.bottom-rcClient.top); 
	}
	
	//Center on screen
	if (s==0) 
		return workArea.right - workArea.left - xDiff; 
	else 
		return workArea.bottom - workArea.top - yDiff;
}

//Maximized client area
int getDialogMaxSize(int s){

	RECT workArea;
	
	SystemParametersInfo(SPI_GETWORKAREA, 0, &workArea, 0); //Work area
	
	if (s==0) 
		return PixelsToHDBUs(workArea.right - workArea.left);
	else {
		return PixelsToVDBUs(workArea.bottom - workArea.top - GetSystemMetrics(SM_CYCAPTION)); //Subtract title bar height
	}
}



int setDialogShowState(int state)
{
    return ShowWindow (fmc.hDlg, state);
}

int setDialogShowStateEx(int state, int width, int height){

	RECT rc;
	RECT workArea;
	WINDOWPLACEMENT wndpl;

	width = HDBUsToPixels(width);
	height = VDBUsToPixels(height);

	SystemParametersInfo(SPI_GETWORKAREA, 0, &workArea, 0); //Work area
	GetWindowRect(fmc.hDlg, &rc);

	//Center on screen
	rc.left = ((workArea.right - workArea.left) - width)/2; 
	rc.top = ((workArea.bottom - workArea.top) - height)/2;
	rc.right = rc.left + width;
	rc.bottom = rc.top + height;

	wndpl.length = sizeof(WINDOWPLACEMENT);
	wndpl.flags = 0;
	wndpl.showCmd = SW_MAXIMIZE;
	wndpl.rcNormalPosition = rc;

	if (state==SW_MAXIMIZE) fMaximized = true; //Produce FME_EXITSIZE event
	if (state==SW_MINIMIZE) fMinimized = true; //Produce FME_EXITSIZE event

	return SetWindowPlacement(fmc.hDlg,&wndpl);
};

int getDialogRestoreWidth(){
	WINDOWPLACEMENT wndpl;
	wndpl.length = sizeof(WINDOWPLACEMENT);
	GetWindowPlacement(fmc.hDlg, &wndpl);
	return PixelsToHDBUs((int)(wndpl.rcNormalPosition.right-wndpl.rcNormalPosition.left));
};

int getDialogRestoreHeight(){
	WINDOWPLACEMENT wndpl;
	wndpl.length = sizeof(WINDOWPLACEMENT);
	GetWindowPlacement(fmc.hDlg, &wndpl);
	return PixelsToVDBUs((int)(wndpl.rcNormalPosition.bottom-wndpl.rcNormalPosition.top));
};


//SW_HIDE             0
//SW_NORMAL           1
//#define SW_SHOWMINIMIZED    2
//#define SW_MAXIMIZE         3
int getDialogWindowState(){
	WINDOWPLACEMENT wndpl;
	wndpl.length = sizeof(WINDOWPLACEMENT);
	GetWindowPlacement(fmc.hDlg, &wndpl);
	return (int)wndpl.showCmd;
};



int setDialogSizeMax(void)
{

    RECT rect;

    if (SystemParametersInfo(SPI_GETWORKAREA,0,&rect,0)){

        RECT rcClient, rcWind; 
        int xDiff,yDiff;

        //Get difference between window and client area
        GetClientRect(fmc.hDlg, &rcClient); 
        GetWindowRect(fmc.hDlg, &rcWind); 
        xDiff = (rcWind.right - rcWind.left) - rcClient.right; 
        yDiff = (rcWind.bottom - rcWind.top) - rcClient.bottom; 

        //Does not work correctly for visual styles:
        //xDiff = 2*GetSystemMetrics(SM_CXDLGFRAME);
        //yDiff = 2*GetSystemMetrics(SM_CYDLGFRAME);
        
        setDialogPos ( true, 
            PixelsToHDBUs(rect.left), 
            PixelsToVDBUs(rect.top),
            PixelsToHDBUs(rect.right - rect.left - xDiff+1),
            PixelsToVDBUs(rect.bottom - rect.top - yDiff+1)
        );

    } else { //old unprecise method

        //int ScreenWidth = getDisplaySettings(1); //dm.dmPelsWidth
	    //int ScreenHeight = getDisplaySettings(2); //dm.dmPelsHeight

        DEVMODE dm;
        dm.dmSize = sizeof(DEVMODE);
        dm.dmDriverExtra = 0;

        if (EnumDisplaySettings(NULL,ENUM_CURRENT_SETTINGS,&dm))
        setDialogPos ( true, -1, -1,PixelsToHDBUs(dm.dmPelsWidth-4),PixelsToVDBUs(dm.dmPelsHeight-4) ); //-4
    }

    return true;
}


int setDialogSizeGrip(int state)
{
    
    if (sizeGrip != state){
        sizeGrip = state;
        if (sizeGrip == 0) refreshWindow(); //Make sure the grip vanishes
    }

    return true;

}

//Scale or do not scale preview and frame together
void unlockPreviewFrame(int state)
{
	unlockedPreviewFrame = state;
}


int PixelsToHDBUs(int h) 
{
    int result=0;
    int buX=0;
    SIZE  size;
    HDC hdc = GetDC(fmc.hDlg);
    HFONT hDlgFont = (HFONT) SendMessage(fmc.hDlg, WM_GETFONT, 0, 0L);
    HFONT hFontOld = (HFONT)SelectObject(hdc,hDlgFont);

	if (h==0) return 0;

    if (GetTextExtentPoint32(hdc,"ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrst"
                "uvwxyz",52,&size)){
        
        buX = (size.cx/26+1)/2;

        //Convert to DBU
        if (buX != 0) result = (h * 4) / buX;
    }

    SelectObject(hdc,hFontOld);
    ReleaseDC(fmc.hDlg, hdc);

    return result;
}


int PixelsToVDBUs(int v) 