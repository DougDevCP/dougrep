    va_list ap;

    va_start(ap, lpFmt);
    _vsnprintf(szBuffer, sizeof(szBuffer), (LPCTSTR)lpFmt, ap);
    szBuffer[sizeof(szBuffer)-1] = 0;   //ensure NUL-terminated
    va_end(ap);
    return MessageBox(
        fmc.doingProxy?fmc.hDlg:NULL,   // handle of owner window
        formatString(szBuffer),      // interpolated text for message box
        
        //Added by Harald Heim, January 12, 2003
         fmc.filterTitleText,
        //"FilterMeister",              // address of title of message box  

        MB_ICONQUESTION|MB_YESNOCANCEL  // style of message box
        | MB_TASKMODAL                  // Force task modal style.
       );
} /*YesNoCancel*/




LARGE_INTEGER start;
clock_t startclock;

int startClock(){

    if (!QueryPerformanceCounter(&start)) startclock = clock();

    return true;
}

int stopClock(){

    LARGE_INTEGER stop;
    LARGE_INTEGER proc_freq;
    double diff;

    if (QueryPerformanceCounter(&stop)){
        if (QueryPerformanceFrequency(&proc_freq)){
            diff = ((stop.QuadPart - start.QuadPart) / (proc_freq.QuadPart/1000.0) );
            return (int)diff;
        }
    } else {
        int stopclock = clock();
        return (int)(stopclock-startclock);
    }

    return 0;
}


int convertColor(int16 sourceSpace, int resultSpace, FilterColor color)
{
    //0=plugIncolorServicesRGBSpace
    //1=plugIncolorServicesHSBSpace
    //2=plugIncolorServicesCMYKSpace
    //3=plugIncolorServicesLabSpace
    //4=plugIncolorServicesGraySpace
    //5=plugIncolorServicesHSLSpace
    //6=plugIncolorServicesXYZSpace
   
    int16 a;

	ColorServicesInfo csInfo;

	csInfo.selector = plugIncolorServicesConvertColor;

	csInfo.sourceSpace = sourceSpace; //CSModeToSpace((int16)sourceMode); //plugIncolorServicesRGBSpace;
    csInfo.resultSpace = resultSpace; //CSModeToSpace((int16)resultMode); //CSModeToSpace(gFilterRecord->imageMode);

	csInfo.reservedSourceSpaceInfo = NULL;
	csInfo.reservedResultSpaceInfo = NULL;
	csInfo.reserved = NULL;
	csInfo.selectorParameter.pickerPrompt = NULL;
	csInfo.infoSize = sizeof(csInfo);
	
    for (a = 0; a < 4; a++)
		csInfo.colorComponents[a] = color[a];

    if (!gStuff->colorServices(&csInfo)){
		for (a = 0; a < 4; a++)
			color[a] = (int8)csInfo.colorComponents[a];
        return true;
    }

    return false;

}



int getProperty(int property, int notsupported){

#ifndef APP
    GetPropertyProc pGPP;   //pointer to getPropertyProc
    OSErr err;
    intptr_t simpleProperty;
    Handle complexProperty;

    //Check if available
    pGPP = gStuff->getPropertyObsolete;
    if (gStuff->propertyProcs) {
        if (gStuff->propertyProcs->propertyProcsVersion >= kCurrentPropertyProcsVersion &&
            gStuff->propertyProcs->numPropertyProcs >= 1 &&
            gStuff->propertyProcs->getPropertyProc) {
            //prefer this version of getPropertyProc over the obsolete one
            pGPP = gStuff->propertyProcs->getPropertyProc;
        }
    } //propertyProcs != 0


    if (pGPP) {
        err = pGPP('8BIM', property, 0, &simpleProperty, &complexProperty);
        if (err == noErr){
            return simpleProperty;
        }
    } 
#endif

    return notsupported; //Not Supported

}


int setCtlClass(int iCtl, int iClass, int absy, int height, int thumbSize){

    int  val       = gParams->ctl[iCtl].val;
    int  initVal   = gParams->ctl[iCtl].initVal;
    int  minval    = gParams->ctl[iCtl].minval;
    int  maxval    = gParams->ctl[iCtl].maxval;
    int  lineSize  = gParams->ctl[iCtl].lineSize;
    int  pageSize  = gParams->ctl[iCtl].pageSize;
	//int  thumbSize  = gParams->ctl[iCtl].thumbSize;
    int  imageType = gParams->ctl[iCtl].imageType;
    int  divisor   = gParams->ctl[iCtl].divisor;
    int  ticFreq   = gParams->ctl[iCtl].tb.ticFreq;
    CTLACTION action = gParams->ctl[iCtl].action;
    COLORREF textColor = gParams->ctl[iCtl].textColor;
    COLORREF bkColor = gParams->ctl[iCtl].bkColor;
    char tooltip[MAX_TOOLTIP_SIZE+1];
    char image[_MAX_PATH+1];
	int noeditborder=false;
	int anchor = gParams->ctl[iCtl].anchor;
	int tabControl = gParams->ctl[iCtl].tabControl;
	int tabSheet = gParams->ctl[iCtl].tabSheet;
    int scripted = gParams->ctl[iCtl].scripted;
    
    strcpy(tooltip, gParams->ctl[iCtl].tooltip);
    strcpy(image, gParams->ctl[iCtl].image);

	if (!(gParams->ctl[iCtl].buddy1Style & WS_BORDER)) noeditborder=true;
	

    deleteCtl(iCtl);

    gParams->ctl[iCtl].ctlclass = (CTLCLASS)iClass;
    if (height>-1) gParams->ctl[iCtl].height = height;
    //gParams->ctl[iCtl].thumbSize = thumbSize;
    if (absy!=0) gParams->ctl[iCtl].yPos += absy;
    
    createCtl(iCtl,
                 gParams->ctl[iCtl].ctlclass,         //class
                 gParams->ctl[iCtl].label,    //text
                 gParams->ctl[iCtl].xPos,          //x
                 gParams->ctl[iCtl].yPos,          //y
                 gParams->ctl[iCtl].width,         //w
                 gParams->ctl[iCtl].height,        //h
                 0,//gParams->ctl[iCtl].style,         //styles
                 0,//gParams->ctl[iCtl].styleEx,       //extended styles
                 0,//gParams->ctl[iCtl].properties,    //properties
                 gParams->ctl[iCtl].enabled);      //visible/enabled
    
    setCtlRange(iCtl, minval, maxval);
    // setCtlTheme(iCtl, gParams->ctl[iCtl].theme); // Overwrites the background color if theme enabled
	setCtlTab(iCtl, tabControl, tabSheet);		// Overwrites the background color if theme enabled
    gParams->ctl[iCtl].initVal = initVal;
    setCtlLineSize(iCtl, lineSize);
    setCtlPageSize(iCtl, pageSize);
	//setCtlThumbSize(iCtl, thumbSize);
    setCtlAction(iCtl, action);
    setCtlVal(iCtl, initVal);
    setCtlFontColor(iCtl, textColor);
    //setCtlColor(iCtl, bkColor);
    setCtlToolTip(iCtl, tooltip, 0);
    setCtlImage(iCtl, image, imageType);
    setCtlDivisor(iCtl, divisor);
	setCtlAnchor(iCtl, anchor);
    setCtlScripting(iCtl, scripted);
    setCtlTab(iCtl,tabControl,tabSheet);

	//Set buddy styles
	if (gParams->ctl[iCtl].ctlclass == CC_STANDARD || gParams->ctl[iCtl].ctlclass == CC_SLIDER){
		setCtlBuddyStyle (iCtl,0,gParams->ctl[iCtl].buddy2Style);
		setCtlBuddyStyle (iCtl,1,gParams->ctl[iCtl].buddy1Style);
		setCtlBuddyStyleEx (iCtl,0,gParams->ctl[iCtl].buddy2StyleEx);
		setCtlBuddyStyleEx (iCtl,1,gParams->ctl[iCtl].buddy1StyleEx);
		if (noeditborder) clearCtlBuddyStyle (iCtl,1,WS_BORDER);
	}

    switch (gParams->ctl[iCtl].ctlclass) {
    case CC_COMBOBOX:
	case CC_LISTBAR:
        setCtlLabel(iCtl,gParams->ctl[iCtl].label2);
        break;
    case CC_TRACKBAR:
	case CC_SLIDER:
        setCtlTicFreq(iCtl, ticFreq);
        break;
    default:
        break;
    } //switch class


    return true;

}

int isValidFilename (char * filename){

    if (filename == NULL) return false;

    // '\', '/', ':', '*', '?', '"', '<', '>', '|'
    if (strchr(filename,'\\') != NULL) return false;
    if (strchr(filename,'/') != NULL) return false;
    if (strchr(filename,':') != NULL) return false;
    if (strchr(filename,'*') != NULL) return false;
    if (strchr(filename,'?') != NULL) return false;
    if (strchr(filename,'"') != NULL) return false;
    if (strchr(filename,'<') != NULL) return false;
    if (strchr(filename,'>') != NULL) return false;
    if (strchr(filename,'|') != NULL) return false;

    return true;
}

int convertToFilename (char * filename){

    int i=0; //j,length;
    char letter;

    if (filename == NULL) return false;

    while (i<(int)strlen(filename)){   
        letter = filename[i];
        if (letter == '\\' || letter == '/' || letter == ':' || letter == '*' || letter == '?' || letter == '"' || letter == '<' || letter == '>' || letter == '|'){
            memmove(filename+i,filename+i+1,strlen(filename)-i);
            //length = strlen(filename)-i;
            //for(j=0;j<length;j++) *(filename+i+j) = *(filename+i+1+j);
        }else
            i++;
    }
    
    return true;
}

int stripEndSpaces(char * string){

    int i; 
    int length = (int)strlen(string);

    for (i=length-1; i>=0; i--){
        if (string[i] != 32){
            if (i!=length-1) *(string+i+1) = 0;
            break;
        }
    }

    return true;
}

int keepFirstWord(char * string){

    int i; 
    int length = (int)strlen(string);

    for (i=0; i<length; i++){
        if (string[i] == 32){
            *(string+i) = 0;
            break;
        }
    }

    return true;
}




int toLowerCase(char * str) 
{   
    int i;

    for (i=0;i<(int)strlen(str);i++){        
        if (str[i] >= 65 && str[i] <= 90) str[i] += 32; 
        //if (str[i] >= 97 && str[i] <= 122) str[i] -= 32; //ToUpper
    }

    return true;
}