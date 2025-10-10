    
    /* None of the globals requires initialization. */
    
}

//-------------------------------------------------------------------------------
//
//  ValidateParameters
//
//  Initialize parameters to default values.
//
//  Inputs:
//      GPtr globals        Pointer to global structure.
//
//  Outputs:
//
//-------------------------------------------------------------------------------

void ValidateParameters (GPtr globals)
{
    int i;

    // Should add a version # to gParams, and ignore old params if version #
    // mismatch!!!

    if (!gStuff->parameters)
    {

        gStuff->parameters = NewHandle ((long) sizeof (TParameters));

        if (!gStuff->parameters)
        {
            gResult = memFullErr;
            return;
        }


#if 0
        // Note that NewHandle zeroes the allocated memory.  Otherwise,         /// DOES IT ZERO????? yes, sets GMEM_ZEROINIT
        // we should zero out the gParams here to make sure the ptrs in
        // the CB code buffer structures are set to NULL, to avoid
        // problems when calling X86_emit_free_resources() on an
        // otherwise uninitialized CB.
        memset(gParams, 0, sizeof(*gParams));
#endif

        // Better query for parameters, since they didn't exist!!
        gParams->queryForParameters = TRUE;

        gParams->version_major = 1;
        gParams->version_minor = 0;

        gParams->flags     = AFH_USE_ADVANCESTATE|AFH_FLUSH_ADVANCESTATE|
                             AFH_INIT_OPTIMAL_ZOOM|AFH_OPTIMIZE_BEVEL|
                             AFH_ZERO_OUTPUT_DRAG| 
                             AFH_USE_INLINES|
                             AFH_SUPPORT_RGB; //|AFH_QUICK_TOGGLE ; /*|AFH_SUPPORT_GRAY| AFH_PROXY_BIG_GULP; */ 

        gParams->tileHeight = 100; //gParams->bandWidth = 100;    // Slow FM down for now...
        gParams->tileWidth = 0;

        for (i = 0;  i < N_CTLS;  i++)
        {   //default control settings
            //gParams->ctl[i].id = 0;
            gParams->ctl[i].initVal = 0;
            gParams->ctl[i].val = 0;
            gParams->ctl[i].defval = 0xffffffff;	//or -1 for X64???
            gParams->ctl[i].minval = 0;
            gParams->ctl[i].maxval = 255;
            gParams->ctl[i].lineSize = 1;
            gParams->ctl[i].pageSize = 10;
			gParams->ctl[i].thumbSize = 0;
            gParams->ctl[i].properties = 0;
            gParams->ctl[i].action = CA_NONE;
            gParams->ctl[i].label[0] = '\0';
            gParams->ctl[i].enabled = 3; //enabled and visible
            gParams->ctl[i].inuse = FALSE;
            gParams->ctl[i].tooltip[0] = '\0';
            gParams->ctl[i].image[0] = '\0';
            gParams->ctl[i].imageType = 0;
            
            gParams->ctl[i].editwidth = 24;
            gParams->ctl[i].editheight = 10;
			gParams->ctl[i].editXpos = 90; // Added by Ognen Genchev
			gParams->ctl[i].editYpos = 0; // Added by Ognen Genchev

			gParams->ctl[i].tabControl = -1;
            gParams->ctl[i].tabSheet = -1;
            gParams->ctl[i].mateControl = -1;
			gParams->ctl[i].anchor = 0;
			gParams->ctl[i].distWidth = 0;
			gParams->ctl[i].distHeight = 0;
			
            gParams->ctl[i].divisor = 1;
            gParams->ctl[i].gamma = 100;
			gParams->ctl[i].theme = -1;

            gParams->ctl[i].scripted = -1;
			gParams->ctl[i].data = -1;
        }

		//Preview is always used
        gParams->ctl[CTL_PREVIEW].inuse = TRUE;
        gParams->ctl[CTL_PREVIEW].properties = CTP_MOUSEMOVE | CTP_MOUSEOVER | CTP_PREVIEWDRAG; //Activate the preview events by default for now until the problem with Photoshop is solved
		gParams->ctl[CTL_PREVIEW].bkColor = -1; //No preview color
		gParams->ctl[CTL_PREVIEW].xPos = -1;
		gParams->ctl[CTL_PREVIEW].yPos = -1;
		gParams->ctl[CTL_PREVIEW].width = -1;
		gParams->ctl[CTL_PREVIEW].height = -1;
		gParams->ctl[CTL_PREVIEW].enabled = 3;
		
		gParams->ctl[CTL_FRAME].inuse = TRUE;
		gParams->ctl[CTL_FRAME].xPos = -1;
		gParams->ctl[CTL_FRAME].yPos = -1;
		gParams->ctl[CTL_FRAME].width = -1;
		gParams->ctl[CTL_FRAME].height = -1;
		gParams->ctl[CTL_FRAME].enabled = 3;
		
		gParams->ctl[CTL_PROGRESS].inuse = TRUE;
		gParams->ctl[CTL_PROGRESS].xPos = -1;
		gParams->ctl[CTL_PROGRESS].yPos = -1;
		gParams->ctl[CTL_PROGRESS].width = -1;
		gParams->ctl[CTL_PROGRESS].height = -1;
		gParams->ctl[CTL_PROGRESS].enabled = 3;
		gParams->ctl[CTL_PROGRESS].ctlclass = CC_PROGRESSBAR;
		gParams->ctl[CTL_PROGRESS].style = 0;
		gParams->ctl[CTL_PROGRESS].styleEx = 0;
		
		gParams->ctl[CTL_ZOOM].inuse = TRUE;
		gParams->ctl[CTL_ZOOM].xPos = -1;
		gParams->ctl[CTL_ZOOM].yPos = -1;
		gParams->ctl[CTL_ZOOM].width = -1;
		gParams->ctl[CTL_ZOOM].height = -1;
		gParams->ctl[CTL_ZOOM].enabled = 3;

	
        //Default dialog settings
		gParams->skinActive = 0;
		gParams->skinStyle = -1;
        gParams->gInitialInvocation = TRUE;
        gParams->gDialogRegion = NULL;
#if 0
        gParams->gDialogGradientColor1 = RGB(0,0,0);        //black
        gParams->gDialogGradientColor2 = RGB(0,0,0);        //black
#else
        //Changed by Harald Heim, March 9, 2003
        gParams->gDialogGradientColor1 = GetSysColor(COLOR_BTNFACE);//RGB(0xD4,0xD0,0xC8);   //yellow grey
        gParams->gDialogGradientColor2 = GetSysColor(COLOR_BTNFACE);//RGB(0xD4,0xD0,0xC8);   //yellow grey
        //gParams->gDialogGradientColor1 = RGB(0x5F,0x9E,0xA0);   //X11.cadetblue
        //gParams->gDialogGradientColor2 = RGB(0x5F,0x9E,0xA0);   //X11.cadetblue
#endif
        gParams->gDialogGradientDirection = 0;              //vertical
        gParams->gDialogImage_save = gDialogImage = NULL;   //no background image
        gParams->gDialogImageFileName[0] = '\0';
        gParams->gDialogImageMode = DIM_TILED;              //TILED
        gParams->gDialogImageStretchMode = COLORONCOLOR;
        gParams->gDialogDragMode = 0;                       //drag by titlebar
#if 0
        gParams->gDialogTitle[0] = '\0';                    //default dialog title
#elif 1
        //default dialog title is "filter title (image mode, filter case)"
        strcpy(gParams->gDialogTitle, "!t"); // (!M, !f)
#else
        //default dialog title is "filter title (filter case) [image mode]"
        strcpy(gParams->gDialogTitle, "!t (!f) [!M]");
#endif

		gParams->gDialogTheme = 0; //Default Dialog Theme

		//gParams->gDialogEvent = 0; //Default value: no event


        //initialize registry access...
        //setRegRoot(HKEY_CURRENT_USER);
        //setRegPath("Software\\!O\\!C\\!t");

        // Initialize built-in strings...
        gParams->filterAuthorText[0] = 0;
        gParams->filterCategoryText[0] = 0;
        gParams->filterTitleText[0] = 0;
        gParams->filterFilenameText[0] = 0;
        gParams->filterCopyrightText[0] = 0;
        gParams->filterDescriptionText[0] = 0;
        gParams->filterOrganizationText[0] = 0;
        gParams->filterURLText[0] = 0;
        gParams->filterVersionText[0] = 0;
        gParams->filterAboutText[0] = 0;    //temporary
        gParams->filterUniqueID[0] = 0;

        //gParams->renderTime = 0;

    } //!gStuff->parameters
} // end ValidateParameters


/*****************************************************************************/

/* Asks the user for the plug-in filter module's parameters. Note that
   the image size information is not yet defined at this point. Also, do
   not assume that the calling program will call this routine every time the
   filter is run (it may save the data held by the parameters handle in
   a macro file). */

void DoParameters (GPtr globals)
{
    ValidateParameters (globals);

    if (gResult != noErr) return;

    gParams->queryForParameters = TRUE; //redundant!!!

} /*DoParameters*/

/*****************************************************************************/
static int topPad, bottomPad, leftPad, rightPad;	//THREAD-SAFE??
#if BIGDOCUMENT
	static VRect *mskRectPtr;
#else
    static Rect *mskRectPtr;
#endif

void addPadding (GPtr globals)
{
#if 0   //OBSOLETE???
#ifdef STATIC_FILTER
    int padding = 20000;  // try to grab the entire rect for now
#else
    int padding = 103;  // upper bound for now... (100 pixel bevel + 3 pixel radius max blur)
#endif
#endif

#if BIGDOCUMENT	 
	VRect scaledFilterRect;
#else
    Rect scaledFilterRect;
#endif
    long  scaleFactor;
    int padding;

    if (fmc.isTileable)
        padding = fmc.needPadding;//0;        // for now, assume no padding needed
    else
        padding = 20000;    // try to grab the entire rect

#if BIGDOCUMENT
	// Grab maskRect and inRect from host (necessary??)
	fmc.maskRect32 = GetMaskRect();
	fmc.inRect32 = GetInRect();

  #ifndef DONT_USE_MASK
    if (fmc.maskRect32.right > fmc.maskRect32.left)
        {   // mask is nonempty
        mskRectPtr = &fmc.maskRect32;
        }
        else
  #endif
        {   // using alpha (layer transparency mask) as mask
        mskRectPtr = &fmc.inRect32;
        }
#else
  #ifndef DONT_USE_MASK
        if (gStuff->maskRect.right > gStuff->maskRect.left)
        {   // mask is nonempty
            mskRectPtr = &gStuff->maskRect;
        }
        else
  #endif
        {   // using alpha (layer transparency mask) as mask
            mskRectPtr = &gStuff->inRect;
        }
    //}
#endif


    if (gParams->flags & AFH_USE_HOST_PADDING)
    {
        topPad = bottomPad = leftPad = rightPad = padding;
    }
    else
    {
		// Get filterRect from host and scale it down
        scaleFactor = gStuff->inputRate >> 16;      

#if BIGDOCUMENT
		scaledFilterRect = GetFilterRect();
		scaleRect32 (&scaledFilterRect, 1, scaleFactor);
#else
        copyRect (&scaledFilterRect, &gStuff->filterRect);
        scaleRect (&scaledFilterRect, 1, scaleFactor);
#endif
		// Calculate padding at each edge
        topPad    = min(padding, mskRectPtr->top - scaledFilterRect.top);
        bottomPad = min(padding, scaledFilterRect.bottom - mskRectPtr->bottom);
        leftPad   = min(padding, mskRectPtr->left - scaledFilterRect.left);
        rightPad  = min(padding, scaledFilterRect.right - mskRectPtr->right);
    }
	// Add padding to either fmc.maskRect32 or fmc.inRect32
    mskRectPtr->top    -= topPad;
    mskRectPtr->bottom += bottomPad;
    mskRectPtr->left   -= leftPad;