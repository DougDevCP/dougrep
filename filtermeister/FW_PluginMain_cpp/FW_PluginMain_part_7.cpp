        || gStuff->imageMode == plugInModeRGBColor || gStuff->imageMode == plugInModeLabColor
        || gStuff->imageMode == plugInModeHSLColor || gStuff->imageMode == plugInModeHSBColor
        || gStuff->imageMode == plugInModeRGB48 || gStuff->imageMode == plugInModeLab48)*/
        fmc.planesWithoutAlpha = 3;
    

    fmc.maxSpace = gStuff->maxSpace;
	fmc.bufferSpace = gStuff->bufferSpace;
    fmc.isFloating = gStuff->isFloating;
    fmc.haveMask = gStuff->haveMask;

    //Added by Harald Heim, Jul 5, 2002
    fmc.autoMask = gStuff->autoMask;
    
    fmc.backColor = RGBA(gStuff->backColor[0],
                            gStuff->backColor[1],
                            gStuff->backColor[2],
                            gStuff->backColor[3]);
    fmc.foreColor = RGBA(gStuff->foreColor[0],
                            gStuff->foreColor[1],
                            gStuff->foreColor[2],
                            gStuff->foreColor[3]);
    // Need to mask out garbage in unused bytes of back/foreColor...
    {   int mask = 0xffffffff;
        switch (gStuff->imageMode)
        {
        case plugInModeBitmap:
        case plugInModeGrayScale:
        case plugInModeIndexedColor:    //can this happen???
        case plugInModeGray16:          //correct???
            mask = 0x000000ff;
            break;

        case plugInModeDuotone:
        case plugInModeDuotone16:

            //mask = 0x0000ffff;          //is duotone always 2 channels???
            mask = 0x00ffffff;          //duotone can have RGB fore and background colors!
            break;

        case plugInModeRGBColor:
        case plugInModeLabColor:
        case plugInModeHSLColor:        //can this happen???
        case plugInModeHSBColor:        //can this happen???
        case plugInModeRGB48:           //correct???      
        case plugInModeLab48:   

            mask = 0x00ffffff;
            break;

        case plugInModeMultichannel:     
        case plugInModeDeepMultichannel:
            
            mask = 0x00ffffff;
            break;

        case plugInModeCMYKColor:
        case plugInModeCMYK64:

        default:
            mask = 0xffffffff;
            break;
        } /*switch*/
        fmc.backColor &= mask;
        fmc.foreColor &= mask;
    } //scope

    //scale 16-bit back/foreground values down to 8 bits...     //Is this correct ???
    fmc.bgColor = fm_RGB(gStuff->background.red >> 8,
                         gStuff->background.green >> 8,
                         gStuff->background.blue >> 8);
    fmc.fgColor = RGB(gStuff->foreground.red >> 8,
                         gStuff->foreground.green >> 8,
                         gStuff->foreground.blue >> 8);
    fmc.hostSig = gStuff->hostSig;

    
    
    fmc.platformData = (intptr_t) gStuff->platformData;
    fmc.imageHRes = (gStuff->imageHRes >> 16) + (gStuff->imageHRes & 0xffff) / 65536.0; //(int) gStuff->imageHRes; //>> 16
    fmc.imageVRes = (gStuff->imageVRes >> 16) + (gStuff->imageVRes & 0xffff) / 65536.0; //(int) gStuff->imageVRes; //>> 16


    fmc.imageMode = gStuff->imageMode;

/*
#if BIGDOCUMENT
#else
     fmc.wholeWidth = gStuff->wholeSize.h;
     fmc.wholeHeight = gStuff->wholeSize.v;
#endif
*/

    /*if (gStuff->filterCase == 0){
		if (gStuff->haveMask==0 && gStuff->isFloating==0) gStuff->filterCase = 1;
		if (gStuff->haveMask==1 && gStuff->isFloating==0) gStuff->filterCase = 2;
		if (gStuff->isFloating==1) gStuff->filterCase = 3;
	}*/
	fmc.filterCase = gStuff->filterCase;
	fmc.samplingSupport = gStuff->samplingSupport;
    fmc.zoomFactor = 0;     //not yet set
	fmc.enlargeFactor = 1;
	fmc.scrollFactor = 1;

    // Restore values in FM context record across invocations...
    fmc.need_iuv = gParams->need_iuv;
    fmc.need_d = gParams->need_d;
    fmc.need_m = gParams->need_m;
    fmc.need_tbuf = gParams->need_tbuf;
    fmc.need_t2buf = gParams->need_t2buf;
	fmc.need_t3buf = gParams->need_t3buf;
	fmc.need_t4buf = gParams->need_t4buf;
    fmc.needPremiere = gParams->needPremiere;
    fmc.needPadding = gParams->needPadding;
    fmc.tileHeight = gParams->tileHeight; //fmc.bandWidth = gParams->bandWidth;
    fmc.tileWidth = gParams->tileWidth;
#ifndef DONT_SAVE_GLOBALS
    /* Restore the following predeclared global and user-declared global/static variables */
    fmc.globvars = gParams->globvars;
#endif //DONT_SAVE_GLOBALS
    memcpy(fmc.filterAuthorText, gParams->filterAuthorText, sizeof(fmc.filterAuthorText));
    memcpy(fmc.filterCategoryText, gParams->filterCategoryText, sizeof(fmc.filterCategoryText));
    memcpy(fmc.filterTitleText, gParams->filterTitleText, sizeof(fmc.filterTitleText));
    memcpy(fmc.filterFilenameText, gParams->filterFilenameText, sizeof(fmc.filterFilenameText));
    memcpy(fmc.filterCopyrightText, gParams->filterCopyrightText, sizeof(fmc.filterCopyrightText));
    memcpy(fmc.filterDescriptionText, gParams->filterDescriptionText, sizeof(fmc.filterDescriptionText));
    memcpy(fmc.filterOrganizationText, gParams->filterOrganizationText, sizeof(fmc.filterOrganizationText));
    memcpy(fmc.filterURLText, gParams->filterURLText, sizeof(fmc.filterURLText));
    memcpy(fmc.filterVersionText, gParams->filterVersionText, sizeof(fmc.filterVersionText));
    memcpy(fmc.filterUniqueID, gParams->filterUniqueID, sizeof(fmc.filterUniqueID));
    
    // Get filter installation directory...
    getFilterInstallDir();

    //fmc.DESIGNTIME=false;

    // Check for valid mode...
#if 1
    ///// Temp kludge!!!!  Allow all modes for now...
    gResult = noErr;
#else
    gResult = filterBadMode;    // PSP 4.0 doesn't recognize this!!
#endif

    switch (gStuff->imageMode)
    {
    case plugInModeBitmap:
        strcpy(fmc.filterImageModeText, "Bitmap");
        if (gParams->flags & AFH_SUPPORT_BITMAP)
            gResult = noErr;
        break;

    case plugInModeGrayScale:
        strcpy(fmc.filterImageModeText, "Grayscale");
        if (gParams->flags & AFH_SUPPORT_GRAY)
            gResult = noErr;
        break;

    case plugInModeDuotone:
        strcpy(fmc.filterImageModeText, "Duotone");
        if (gParams->flags & AFH_SUPPORT_DUOTONE)
            gResult = noErr;
        break;

    case plugInModeIndexedColor:
        strcpy(fmc.filterImageModeText, "Indexed Color");
        if (gParams->flags & AFH_SUPPORT_INDEXED)
            gResult = noErr;
        break;

    case plugInModeRGBColor:
        strcpy(fmc.filterImageModeText, "RGB Color");
        if (gParams->flags & AFH_SUPPORT_RGB)
            gResult = noErr;
        break;

    case plugInModeCMYKColor:
        strcpy(fmc.filterImageModeText, "CMYK Color");
        if (gParams->flags & AFH_SUPPORT_CMYK)
            gResult = noErr;
        break;

    case plugInModeLabColor:
        strcpy(fmc.filterImageModeText, "Lab Color");
        if (gParams->flags & AFH_SUPPORT_LAB)
            gResult = noErr;
        break;

    case plugInModeMultichannel:
        strcpy(fmc.filterImageModeText, "Multichannel");
        if (gParams->flags & AFH_SUPPORT_MULTI)
            gResult = noErr;
        break;

    case plugInModeHSLColor:
        strcpy(fmc.filterImageModeText, "HSL Color");
        break;

    case plugInModeHSBColor:
        strcpy(fmc.filterImageModeText, "HSB Color");
        break;

    case plugInModeGray16:
        strcpy(fmc.filterImageModeText, "16-bit Grayscale");
        break;

    case plugInModeRGB48:
        strcpy(fmc.filterImageModeText, "48-bit RGB Color");
        break;

//Added by Harald Heim, December 6, 2002

    case plugInModeLab48:
        strcpy(fmc.filterImageModeText, "48-bit Lab Color");
        break;
    
    case plugInModeCMYK64:
        strcpy(fmc.filterImageModeText, "64-bit CMYK Color");
        break;

    case plugInModeDeepMultichannel:
        strcpy(fmc.filterImageModeText, "Deep Multichannel");
        break;

    case plugInModeDuotone16:
        strcpy(fmc.filterImageModeText, "16-bit Duotone");
        break;


    
    default:
        strcpy(fmc.filterImageModeText, "Unknown Image Mode");
        break;
    } /*switch*/


    if (gResult != noErr)
    {
        if (gResult == filterBadMode && gStuff->hostSig != '8BIM')
        {   // PSP and others don't recognize the filterBadMode error,
            // so put out our own message box...
            ShowAlert(16501);
            gResult = 1;    // indicates error message already displayed.
        }
        return;
    }

    
	//set default to allocation from the host
	set_array_mode(0);

    //Initialize Arrays
    for (i=0; i<N_ARRAYS; i++){
                Array[i] = 0;
				ArrayID[i] = 0;
                ArrayPadding[i]=0;
				ArraySize[i]=0;
				ArrayBytes[i]=0;
				ArrayX[i]=0;
				ArrayY[i]=0; 
				ArrayZ[i]=0;
    }

    //Initialize Font Array
    for (i=0; i<N_FONTS; i++){
        Font[i] = 0;		
    }



#ifdef SCRIPTABLE

    //Initialize Script Array
	for (i=0; i<N_SCRIPTITEMS; i++){
        ScriptParam[i] = 0;
        ScriptParamUsed[i] = false;
    }

	ReadScriptParameters(&displayDialog); //err = ReadScriptParameters(&displayDialog);
    fmc.displayDialog = displayDialog;
    fmc.doingScripting = ScriptParamRead;

#endif

	mtAbortFlag = false; //For aborting multiple threads


	if (fmc.displayDialog)
    {
        //Display Dialog
		PlatformData *platform = (PlatformData *)((FilterRecordPtr) gStuff)->platformData;
        
        /*if (CheckProxiesSupported (gStuff->displayPixels) && WarnAdvanceStateAvailable())
		    PromptUserForInput (globals);   // Show the UI
        else
            PromptUserForInputNoProxy (globals);*/

		CheckProxiesSupported (gStuff->displayPixels);
        PromptUserForInput (globals);   // Show the UI
		