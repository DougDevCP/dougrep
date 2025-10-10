
		//Unhook Key Capture
		if (KeyHook != NULL) UnhookWindowsHookEx(KeyHook);



        if (gResult == noErr){

            //user hit OK

            gParams->queryForParameters = FALSE;
			
		#if 0
			//Clean up threads
			terminateThread(0);
		#endif
		#if STYLER
			deleteThreads();
		#endif

        } else {

            //user hit Cancel, exit button or some other error

			//if (gParams->gDialogEvent & 2) triggerEvent(0,FME_CANCEL,0);

			finalCleanup();
            
            // Maybe should also copy the global vars to the param block here????

            return;

        }


    }
    else
    {
        // No Dialog will be displayed
        // don't need to ask for params
        // ...but may need to adjust passed-in params; e.g.,
        //    if the filterCase has changed...

    #ifdef SCRIPTABLE
        //Write the script params to the controls
        if (ScriptParamRead){
            int n;
			for (n=0;n<N_SCRIPTITEMS;n++) {
                if (ScriptParamUsed[n]) gFmcp->pre_ctl[n] = gParams->ctl[n].val = ScriptParam[n]; //setCtlVal(n, ScriptParam[n]);
            }
        }
    #endif

    }


    //Finally apply filter to image...

    //set doingProxy FALSE again, since PromptUserForInput may have
    //set it TRUE...
    fmc.doingProxy = FALSE;

	//Perform pre-filter initializations...
    InitBeforeFilter();

    if (WarnAdvanceStateAvailable())
        StartWithAdvanceState(globals);
    else
        StartNoAdvanceState(globals);

    call_FFP(FFP_OnFilterEnd,(INT_PTR)&fmc);//OnFilterEnd(&fmc);

    // Maybe here is a good place to copy the global vars to the param block????
#ifndef DONT_SAVE_GLOBALS
    /* Save the following predeclared global and user-declared global/static variables */
    gParams->globvars = gFmcp->globvars;
#endif //DONT_SAVE_GLOBALS


} /*DoStart*/

/******************************************************************************/
/* Does all of the filtering with no advance state and proxies. */



void StartNoAdvanceState (GPtr globals)
{
    DoInitialRect(globals);

} /*StartNoAdvanceState*/
    


/*****************************************************************************/
/* Does all of the filtering. */


void StartWithAdvanceState (GPtr globals)
{
    
    //int total;
    int row, rowDelta = gParams->tileHeight; //bandWidth;
    int col, colDelta = gParams->tileWidth;
    //int top,bottom,left,right;

    // Do the actual filtering operation on the original image
#if BIGDOCUMENT      
	fmc.filterRect32 = GetFilterRect();
    //total = fmc.filterRect32.bottom - fmc.filterRect32.top;  
#else
    //total = gStuff->filterRect.bottom - gStuff->filterRect.top;
#endif	 

    // If the algorithm isn't tileable, grab the entire image...
    //if (!fmc.isTileable) rowDelta = total;


    gStuff->inLoPlane = gStuff->outLoPlane = 0;
    gStuff->inHiPlane = gStuff->outHiPlane = gStuff->planes - 1;

  #if BIGDOCUMENT
	  // Set left and right edges of inRect and outRect to the full width of the filterRect.
	   fmc.inRect32 = fmc.outRect32 = fmc.filterRect32;
	   SetInRect(fmc.inRect32);
	   SetOutRect(fmc.outRect32);
																										 
       if (!fmc.isTileable || rowDelta==0) rowDelta = fmc.filterRect32.bottom - fmc.filterRect32.top;
       if (!fmc.isTileable || colDelta==0) colDelta = fmc.filterRect32.right - fmc.filterRect32.left;
	   
       PISetRect32(&fmc.maskRect32, 0, 0, 0, 0);
	   SetMaskRect(fmc.maskRect32);
#else
        gStuff->filterRect.top = gStuff->filterRect.top;
        gStuff->filterRect.bottom = gStuff->filterRect.bottom;
        gStuff->filterRect.left = gStuff->filterRect.left;
        gStuff->filterRect.right = gStuff->filterRect.right;
        if (!fmc.isTileable || rowDelta==0) rowDelta = gStuff->filterRect.bottom - gStuff->filterRect.top;
        if (!fmc.isTileable || colDelta==0) colDelta = gStuff->filterRect.right - gStuff->filterRect.left;
        gStuff->inRect.left = gStuff->outRect.left = gStuff->filterRect.left;
        gStuff->inRect.right = gStuff->outRect.right = gStuff->filterRect.right;
        PISetRect (&gStuff->maskRect, 0, 0, 0, 0);
#endif
    //}



    gStuff->inputRate = long2fixed (1L);
    gStuff->maskRate = long2fixed (1L);
    
    if (gParams->flags & AFH_USE_HOST_PADDING)
    {
        gStuff->inputPadding = 0;   // 0=black.
        gStuff->outputPadding = 0;  // 0=black.
        gStuff->maskPadding = 0;    // 0=black (non-selected).
    }
    else
    {
        gStuff->inputPadding = plugInDoesNotWantPadding;
        gStuff->outputPadding = plugInDoesNotWantPadding;
        gStuff->maskPadding = plugInDoesNotWantPadding;
    }
    
#if BIGDOCUMENT      
	for (row = fmc.filterRect32.top; row < fmc.filterRect32.bottom; row += rowDelta)
    //for (col = fmc.filterRect32.left; col < fmc.filterRect32.right; col += colDelta)
#else    
    for (row = gStuff->filterRect.top; row < gStuff->filterRect.bottom; row += rowDelta)
    for (col = gStuff->filterRect.left; col < gStuff->filterRect.right; col += colDelta)
#endif
    {
        
        if (TestAbort ())
        {
            gResult = userCanceledErr;
            goto done;
        }
        
#if BIGDOCUMENT      
        fmc.inRect32.top = fmc.outRect32.top = row;
        if (rowDelta > fmc.filterRect32.bottom - row)
            rowDelta = fmc.filterRect32.bottom - row;
        fmc.inRect32.bottom = fmc.outRect32.bottom = row + rowDelta;
		SetInRect(fmc.inRect32);
		SetOutRect(fmc.outRect32); 
#else
        gStuff->inRect.top = gStuff->outRect.top = row;
        //if (rowDelta > gStuff->filterRect.bottom - row)
        //    rowDelta = gStuff->filterRect.bottom - row;
        //gStuff->inRect.bottom = gStuff->outRect.bottom = row + rowDelta;
        if (rowDelta > gStuff->filterRect.bottom - row)
            gStuff->inRect.bottom = gStuff->outRect.bottom = gStuff->filterRect.bottom;
        else
            gStuff->inRect.bottom = gStuff->outRect.bottom = row + rowDelta;

        gStuff->inRect.left = gStuff->outRect.left = col;
        if (colDelta > gStuff->filterRect.right - col)
            gStuff->inRect.right = gStuff->outRect.right= gStuff->filterRect.right;
        else
            gStuff->inRect.right = gStuff->outRect.right= col + colDelta;
#endif

        
#ifndef DONT_USE_MASK
        if (gStuff->haveMask){
#if BIGDOCUMENT
			fmc.maskRect32 = fmc.inRect32;
			SetMaskRect(fmc.maskRect32);
#else
                gStuff->maskRect = gStuff->inRect;
#endif
        }
#endif

        addPadding(globals);


        gResult = AdvanceState ();
        

        subtractPadding(globals);

        if (gResult != noErr)
        {
#if 0
            ShowAlert (16500); //debug PSP 4.12
#endif
            goto done;
        }
        
        DoFilterRect (globals);
        
        if (gResult != noErr)
        {
            goto done;
        }
        
    } //for
    
done:
        
#if BIGDOCUMENT
    // Zero the request Rectangles to signal a stop
    PISetRect32(&fmc.inRect32, 0, 0, 0, 0);
    PISetRect32(&fmc.outRect32, 0, 0, 0, 0);
    PISetRect32(&fmc.maskRect32, 0, 0, 0, 0);
	SetInRect(fmc.inRect32);
	SetOutRect(fmc.outRect32);
	SetMaskRect(fmc.maskRect32);
#else
    PISetRect (&gStuff->inRect, 0, 0, 0, 0);
    PISetRect (&gStuff->outRect, 0, 0, 0, 0);
    PISetRect (&gStuff->maskRect, 0, 0, 0, 0);
#endif

    if ((gParams->flags & AFH_FLUSH_ADVANCESTATE) && (gResult == noErr))
    {

        // Call AdvanceState() one more time to flush out PSP 4.12...
        gResult = AdvanceState ();
    
    } //if
} /*StartWithAdvanceState*/



/*****************************************************************************/
void InitContextRecord(void)
{
    // Initialize static portion of the FM context record.
    // (Needs to be done only once per DLL invocation.)
    int i;

    // Following private C-RTL routine converts float on top
    // of FPU stack to long int in EAX, but is not declared
    // in any public header file (it's only referenced by
    // compiler-generated code).
#ifndef _WIN64
    extern long _ftol(void);
#endif

    // Initialize the info in the "frozen" section of the FM
    // context record.  This section should never be altered.
    // Any additions to the FM context record must *follow*
    // the frozen section.

    // initialize fmc.undefined with a signaling NaN.
    *(__int64 *)&fmc.undefined = 0x7ff7ffffffffffff; /* largest positive double SNaN */
    fmc.FMC_major_version = FMC_MAJOR_VERSION;
    fmc.FMC_minor_version = FMC_MINOR_VERSION;
    fmc.FMC_struct_size = sizeof(fmc);

#ifndef TRIG_ON_FLY
    // Precompute the trig tables...
    for (i=0; i < 1024; i++)
        fmc.pre_sin[i] = (int)(sin( (twopi/1024.0)*(double)i ) * 512.0 + 0.0); // round??

    for (i=0; i < 512; i++) {
        if ( i == 256)
            fmc.pre_tan[i] = (int)(tan( (twopi/1024.0)*(-255.5) ) * 1024.0 + 0.0);    // per Jens???