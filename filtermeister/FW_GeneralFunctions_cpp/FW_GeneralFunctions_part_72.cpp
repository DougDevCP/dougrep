
int retrieveFilename(char * path, char * filename)
{
	char * pch;

	if (strlen(path)==0) return false;
					        
	pch=strrchr(path,0x5C);

	if (pch != NULL){
		strcpy (filename, pch+1);
	} else 
		strcpy (filename, path);

	return true;
}

int retrieveFilenameNoExt(char * path, char * filename)
{
	char * pch;

	if (strlen(path)==0) return false;
					        
	pch=strrchr(path,0x5C);

	if (pch != NULL){
		strcpy (filename, pch+1);
	} else 
		strcpy (filename, path);

	pch=strrchr(filename,0x2E);
	
	if (pch == NULL) return false;
	
	memset(filename+(pch-filename),0,1);

	return true;
}

int retrieveFolder(char * path, char * folder)
{
	char * pch;
	
	if (strlen(path)==0) return false;

	pch=strrchr(path,0x5C);

	if (pch == NULL) return false;

	if (pch == path+strlen(path)-1){
		strcpy (folder, path);
	} else {
		memcpy(folder,path,pch-path+1);
		memset(folder+(pch-path)+1,0,1);
	}

	if (strlen(folder)>0 && folder[strlen(folder)-1] != 0x5C) strcat(folder,"\\");
		
	return true;
}



int fileExists(char *fileName){    
	DWORD	fileAttr;    
	
	fileAttr = GetFileAttributes(fileName);    
	
	//return (fileAttr != INVALID_FILE_ATTRIBUTES &&  !(fileAttr & FILE_ATTRIBUTE_DIRECTORY));

	if (fileAttr == 0xFFFFFFFF) return false;
	return true;

}

int shellExecuteAndWait(char * lpVerb, char * lpFile){

	BOOL retval=0;
	SHELLEXECUTEINFO ExecInfo;

	ExecInfo.cbSize = sizeof(SHELLEXECUTEINFO);
	ExecInfo.fMask = SEE_MASK_NOCLOSEPROCESS; //NULL;
	ExecInfo.hwnd = fmc.doingProxy?fmc.hDlg:NULL;
	ExecInfo.lpVerb = (LPCTSTR)lpVerb;
	ExecInfo.lpFile = (LPCTSTR)lpFile;
	ExecInfo.lpParameters = NULL;
    ExecInfo.lpDirectory = NULL;
    ExecInfo.nShow = SW_SHOWNORMAL;
    ExecInfo.hInstApp = NULL;

	retval = ShellExecuteEx(&ExecInfo);

	//Wait
	if (ExecInfo.hProcess){
		WaitForSingleObject(ExecInfo.hProcess,INFINITE);
		CloseHandle(ExecInfo.hProcess);
	}

	return retval;
	

	/*
	STARTUPINFO si;
    PROCESS_INFORMATION pi;
	char * strTemp = calloc(512,1);

	sprintf(strTemp,"explorer.exe %s",lpFile);

    ZeroMemory( &si, sizeof(si) );
    si.cb = sizeof(si);
    ZeroMemory( &pi, sizeof(pi) );

    // Start the child process. 
    if(CreateProcess( NULL,   // No module name (use command line)
        (LPSTR)strTemp,        // Command line
        NULL,           // Process handle not inheritable
        NULL,           // Thread handle not inheritable
        FALSE,          // Set handle inheritance to FALSE
        0,              // No creation flags
        NULL,           // Use parent's environment block
        NULL,           // Use parent's starting directory 
        &si,            // Pointer to STARTUPINFO structure
        &pi )           // Pointer to PROCESS_INFORMATION structure
    ){
        // Wait until child process exits.
		WaitForSingleObject( pi.hProcess, INFINITE );

		// Close process and thread handles. 
		CloseHandle( pi.hProcess );
		CloseHandle( pi.hThread );

        return true;
    }*/

    return false;

}



/*
HACCEL hAcceleratorTable = 0;
LPACCEL acceleratorArray = 0;
int acceleratorCount = 0;

int addKeyShortcut (int n, int key, int fVirt){

	hAcceleratorTable = NULL;
	if (hAcceleratorTable) DestroyAcceleratorTable(hAcceleratorTable);

	acceleratorCount++;
	acceleratorArray = realloc(acceleratorArray,acceleratorCount*sizeof(ACCEL));
	
	acceleratorArray[acceleratorCount-1].cmd = n;
	acceleratorArray[acceleratorCount-1].key = key;
	acceleratorArray[acceleratorCount-1].fVirt = fVirt;

	hAcceleratorTable = CreateAcceleratorTable(acceleratorArray,acceleratorCount);

	//Info ("acc: %d",hAcceleratorTable);

	return (int)hAcceleratorTable;
}*/

int getCtlItemString(int n, int item, char * string){

	char *p, *q, q_save;
    char* copy;
	int count = 0;
    
    copy = (char *)malloc(strlen(gParams->ctl[n].label)+1);
    strcpy(copy,gParams->ctl[n].label);

    p = copy; 
    while (*p) {
        q = p; while(*q != '\0' && *q != '\n') q++;

        q_save = *q;
        *q = '\0';

		if (count == item) {
			if (string) strcpy(string,p);
			break;
		}
		count++;

        *q = q_save;
        p = q;
        if (*p) p++;
    } //while 
	
    free (copy);

	return count;
}


int gammaCtlVal(int iCtl, int val, int invert){

	int gamma = gParams->ctl[iCtl].gamma;
	double r, absRange;

	if (gamma == 100) return val; //Not necessary

	r = gamma/100.0;
	if (!invert) r = 1.0/r;  //Inverse Gamma?


    if (gParams->ctl[iCtl].minval >= 0){
        absRange = (double) abs (gParams->ctl[iCtl].maxval - gParams->ctl[iCtl].minval) - 1.0;
        return gParams->ctl[iCtl].minval + (int)(pow( (val - gParams->ctl[iCtl].minval) / absRange, r) * absRange + 0.5);
    } else {
        if (val >= 0){
            absRange = (double) gParams->ctl[iCtl].maxval - 1.0;
            return (int)(pow( val / absRange, r) * absRange + 0.5);
        } else {
            absRange = (double) abs (gParams->ctl[iCtl].minval) - 1.0;
            return -(int)(pow( abs(val) / absRange, r) * absRange + 0.5);
        }
    }
}

void drawPreviewColor(HDC hDC){

	if (gParams->ctl[CTL_PREVIEW].bkColor != -1){

		if (hDC ==(HDC)-1) hDC = GetDC(fmc.hDlg);

		//Only if image does not fill the preview
		if ( (gFmcp->x_start == 0 && gFmcp->x_end == gFmcp->X) || (gFmcp->y_start == 0 && gFmcp->y_end == gFmcp->Y) ){
			HBRUSH hBrush;
			RECT rect;
			//HDC hDC = (HDC) wParam;

			GetWindowRect( GetDlgItem(fmc.hDlg, 101), &rect );
			ScreenToClient (fmc.hDlg, (LPPOINT)&rect);
			ScreenToClient (fmc.hDlg, (LPPOINT)&(rect.right));
			hBrush = CreateSolidBrush(gParams->ctl[CTL_PREVIEW].bkColor);
			FillRect(hDC, &rect, hBrush);
			DeleteObject(hBrush);
		}
	}

}

void drawSizeGrip(){

	RECT rc;
	HDC  hDC;

	GetClientRect(fmc.hDlg, &rc ); 
	rc.left = rc.right - GetSystemMetrics(SM_CXHSCROLL); 
	rc.top = rc.bottom - GetSystemMetrics(SM_CYVSCROLL);

#if SKIN
	//Delete previous grip
	/*if (!isSkinActive() || gParams->skinStyle==0){
		static RECT rcOld;
		if (rcOld.right - rcOld.left > 0 && !(EqualRect(&rcOld, &rc)==1) ) {
			HBRUSH hBr;
			hBr = GetSysColorBrush(COLOR_BTNFACE);
			FillRect (hDC, &rcOld, hBr);
			DeleteObject (hBr);
		}
		rcOld = rc;
	}*/
#endif

	/*if (getAppTheme()) drawThemePart(-1,&rc,L"SCROLLBAR",10,1);*/
#if SKIN		
	else if (isSkinActive() && gParams->skinStyle>0){ //Simulate handle
		int i; 
		int hicolor = GetSysColor(COLOR_BTNHILIGHT);
		int locolor = GetSysColor(COLOR_BTNSHADOW);
		startSetPixel(-1);
			for (i=0;i<3;i++){
				setPenWidth(2);
				setLine(rc.left+6+i*4,rc.bottom,rc.right,rc.top+6+i*4,locolor);
				setLine(rc.left+8+i*4,rc.bottom,rc.right,rc.top+8+i*4,hicolor);
				setPenWidth(1);
			}
		endSetPixel(-1);
	} 
#endif	
	// else {
		hDC = GetDC(fmc.hDlg);
		DrawFrameControl(hDC, &rc, DFC_SCROLL, DFCS_SCROLLSIZEGRIP );
		ReleaseDC(fmc.hDlg, hDC);
	// }
	
}

void resetTabCtls(int iCtl){

	int i,oldvalue;
	int previewUpdate = false;
	int val = fmc.pre_ctl[iCtl];
	int subtab;

	for (i = 0; i < N_CTLS; ++i) {