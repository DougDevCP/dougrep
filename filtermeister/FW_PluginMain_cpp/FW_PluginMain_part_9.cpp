        else
            fmc.pre_tan[i] = (int)(tan( (twopi/1024.0)*(double)i ) * 1024.0 + 0.0);   // round?? can be neg!!
    }
#endif

    // Initialize constants

    // NDP control word constants, used for various rounding modes
    *(int *)&fmc.CW_ROUND = 0x133F;  //IC=affine, RC=00(round), PC=11(64bits), all 6 exceptions masked
    *(int *)&fmc.CW_FLOOR = 0x173F;  //IC=affine, RC=01(floor), PC=11(64bits), all 6 exceptions masked
    *(int *)&fmc.CW_CEIL  = 0x1B3F;  //IC=affine, RC=10(ceil),  PC=11(64bits), all 6 exceptions masked
    *(int *)&fmc.CW_CHOP  = 0x1F3F;  //IC=affine, RC=11(chop),  PC=11(64bits), all 6 exceptions masked

    // Save current control word (not a constant, but presumably 027F for MSVC).
    //fmc.current_cw = fstcw();

    // The following "magic" constant has been carefully tuned for
    // use in rounding down when computing an integer square root,
    // over most (if not all) the domain of sqr().  See the code
    // for OP_SQR in the FM compiler module.
    fmc.magic5 = 0.49999999999999;

    // Following approximation to 512.0/Pi is used to compute d...
    fmc.d512byPi = 256.0 * 0.63661977236758134308;

    // Following 32-bit floating-point constant 1.0F is used by
    // the in-line code generated for powi().  We specifically
    // want a 32-bit float, not a 64-bit double, because (at least
    // on the 387), FDIVR_F 1.0F is faster than FDIVR_D 1.0.
    fmc.float1p0 = 1.0F;

    // Initialize built-in strings...
    fmc.filterAuthorText[0] = 0;
    fmc.filterCategoryText[0] = 0;
    fmc.filterTitleText[0] = 0;
    fmc.filterFilenameText[0] = 0;
    fmc.filterCopyrightText[0] = 0;
    fmc.filterDescriptionText[0] = 0;
    fmc.filterOrganizationText[0] = 0;
    fmc.filterURLText[0] = 0;
    fmc.filterVersionText[0] = 0;
    fmc.filterCaseText[0] = 0;
    fmc.filterImageModeText[0] = 0;
    fmc.filterHostText[0] = 0;
    fmc.filterInstallDir[0] = 0;
    fmc.filterUniqueID[0] = 0;

#ifndef DONT_SAVE_GLOBALS
    //Zero out the predefined globals (harmless, but probably unnecessary) */
    memset(&fmc.globvars, 0, sizeof(fmc.globvars));
#endif //DONT_SAVE_GLOBALS


    // Precompute the gamma table for gamma = 1.0...
    //setGamma(1.0);

    // Initialize vectors to FM built-in routines...


    // Standard FF built-ins.
    if (gStuff->imageMode == plugInModeRGB48 ||
        gStuff->imageMode == plugInModeGray16 ||
        gStuff->imageMode == plugInModeLab48 || 
        gStuff->imageMode == plugInModeCMYK64 ||
        gStuff->imageMode == plugInModeDeepMultichannel ||
        gStuff->imageMode == plugInModeDuotone16
        ) {
       
        // pixel depth = 16
        fmc.src = src16;
        fmc.pset = pset16;
        fmc.pget = pget16;
        //fmc.pgetr = pgetr16;

		fmc.tget = tget16;
	    fmc.t2get = t2get16;
		fmc.t3get = t3get16;
		fmc.t4get = t4get16;
		fmc.tset = tset16;
		fmc.t2set = t2set16;
		fmc.t3set = t3set16;
		fmc.t4set = t4set16;
    }
    else {
        
        // pixel depth = 8
        fmc.src = src;
        fmc.pset = pset;
        fmc.pget = pget;
        //fmc.pgetr = pgetr;

		fmc.tget = tget;
	    fmc.t2get = t2get;
		fmc.t3get = t3get;
		fmc.t4get = t4get;
		fmc.tset = tset;
		fmc.t2set = t2set;
		fmc.t3set = t3set;
		fmc.t4set = t4set;
    }



    //Following are not necessarily static, but we initialize
    //them here anyway for good hygiene...
    //These also get reinitialized later at the start of each tile.
    fmc.dmax = fmc.D = 1024;
    fmc.xmin = fmc.ymin = fmc.zmin = fmc.pmin = fmc.mmin = fmc.dmin = 0;
    fmc.rmax = fmc.gmax = fmc.bmax = fmc.amax = fmc.cmax = 255;	//should change with bit-depth??
    fmc.imax = fmc.umax = fmc.vmax = 255;
    fmc.rmin = fmc.gmin = fmc.bmin = fmc.amin = fmc.cmin = 0;
    fmc.imin = fmc.umin = fmc.vmin = 0;
    fmc.R = fmc.G = fmc.B = fmc.A = fmc.C = 255;    //conflict between FF and FF+???
    fmc.I = fmc.U = fmc.V = 255;
    fmc.t = fmc.tmin = 0;
    fmc.total = fmc.tmax = 1;

    //Initialize some other variables for good hygiene.
    fmc.n = 0;
    fmc.ctlMsg = 0;
    fmc.ctlCode = 0;
    fmc.ctlClass = CC_UNUSED;
    fmc.dividend = 0;
    fmc.divisor = 1;

    fmc.doingProxy = FALSE;
    fmc.doingScripting = 0;

    //initialize random number generator 
    srand(1);

    //initialize anonymous cells
    if (Dont_Initialize_Cells == 0)
    {
        for (i = 0; i < N_CELLS; i++) {
            fmc.cell[i] = 0;
        }
    }

    // Cache current control values in fmc.pre_ctl[].
    for (i=0; i < N_CTLS; i++) {
        fmc.pre_ctl[i] = gParams->ctl[i].val;
    }

} /*InitContextRecord*/

/*****************************************************************************/
/* Filter the area. */


//Commented out by Harald Heim, Jun 4, 2002
/*
#define maxBlurRadius 3

#if 0
#define mskVal(row,rowOff,col,colOff) \
            fmc.mskPtr[(rowOff)*fmc.mskRowDelta+(colOff)*fmc.mskColDelta]
#elif 0
#define mskVal(row,rowOff,col,colOff) \
            fmc.mskData[((row)+(rowOff))*fmc.mskRowDelta+((col)+(colOff))*fmc.mskColDelta]
#elif 1
static unsigned8 gMskVal(int row, int rowOff, int col, int colOff)
{
    // alert if out of bounds, for debugging...
#ifdef CHECK_GMSKVAL
#ifndef NOTRACE
    if ( row + rowOff <            -gBevWidth-maxBlurRadius   ||
         row + rowOff >    fmc.rows+gBevWidth+maxBlurRadius-1 ||
         col + colOff <            -gBevWidth-maxBlurRadius   ||
         col + colOff > fmc.columns+gBevWidth+maxBlurRadius-1 )
    {
        Trace (globals, "Bad maskData subscripts:\n %d, %d, %d, %d",
               row, rowOff, col, colOff);
    }
#endif
#endif

    if ((gNoMask || !(gParams->flags & AFH_USE_HOST_PADDING)) &&
        (row+rowOff < minRow || row+rowOff > maxRow ||
         col+colOff < minCol || col+colOff > maxCol) )
        return 0;   // Simulate the padding...
    else if (gNoMask)
        return 255; // Entire filterRect is inside the mask.
    else
        return fmc.mskPtr[rowOff*fmc.mskRowDelta + colOff*fmc.mskColDelta];
} //gMskVal
#define mskVal(row,rowOff,col,colOff) \
    gMskVal(row,rowOff,col,colOff)
#endif
*/


////////////////////////////////////////
int copyInputToOutput();

void DoFilterRect (GPtr localGlobals)
{
    static int reentry_depth = 0;
    static int reentry_count = 0;
    //int startclock;


    if (gResult != noErr) {
        // Got an error somewhere along the way...
        // Get out quick in case the error was returned by
        // AdvanceState(), in which case the input/output data
        // pointers are bogus and can cause severe dyspepsia.
        return;
    }

    //check for reentry...
    if (reentry_depth++ > 0) {
        //reentered!!! Log the fact, and exit quickly...
        reentry_count++;
        reentry_depth--;
        return; //get out quick!!!
    }

    globals = localGlobals;

    if (!gStuff->outData) {
        // I think I've caught all cases where zero-dragging
        // can cause a GP in the filter code because outData
        // is set to 0 while dragging.  This is just a last-
        // minute sanity check to try to avoid  GP-ing on the
        // end user!  (Record any failures for later analysis.)
        static int unexpected_null_outData_aborts = 0;
        unexpected_null_outData_aborts++;
#if 1
        Info("Unexpected NULL outData, count = %d", unexpected_null_outData_aborts);
#endif
        reentry_depth--;
        return; //get out quick!!!
    }

    if (fmc.doingProxy) {
        //proxy is never tiled, so we can do this here...
        InitBeforeFilter();
    }

    { //scope
#if BIGDOCUMENT
	VRect scaledFilterRect;  
#else
    Rect scaledFilterRect;
#endif
    OSErr err = noErr;

    fmc.scaleFactor = gStuff->inputRate >> 16;
    fmc.srcPtr = (unsigned8 *) MyAdjustedInData;
    fmc.dstPtr = (unsigned8 *) gStuff->outData;
    fmc.mskData = (unsigned8 *) MyAdjustedMaskData;
#if BIGDOCUMENT
	fmc.outRect32 = GetOutRect();	//needed??
    fmc.rows = fmc.outRect32.bottom - fmc.outRect32.top;
    fmc.columns = fmc.outRect32.right - fmc.outRect32.left;		   
#else
    fmc.rows = gStuff->outRect.bottom - gStuff->outRect.top;
    fmc.columns = gStuff->outRect.right - gStuff->outRect.left;
#endif
    fmc.srcRowDelta = gStuff->inRowBytes;
    fmc.srcColDelta = gStuff->inHiPlane - gStuff->inLoPlane + 1;
    fmc.inPlanes = fmc.srcColDelta;
    fmc.dstRowDelta = gStuff->outRowBytes;
    fmc.dstColDelta = gStuff->planes;
    fmc.mskRowDelta = gStuff->maskRowBytes;
    fmc.mskColDelta = 1;

    { //new scope

#ifndef DONT_USE_MASK
    if (gStuff->haveMask)
    {   // mask stuff already set up for selection mask...
        gNoMask = FALSE;
    }
    else
#endif
    if (fmc.inPlanes >= 4)
    {   // Channel 3 is probably an alpha channel, so
        // use the alpha channel (layer transparency mask)
        // instead of the selection mask
        fmc.mskData = (unsigned8 *) MyAdjustedInData + 3;
        fmc.mskRowDelta = fmc.srcRowDelta;
        fmc.mskColDelta = fmc.srcColDelta;
        gNoMask = FALSE;
    }
    else
    {   // No selection mask and no transparency channel, so
        // we'll just fake a mask that includes the entire
        // filterRect (which will give us a nice rectangular
        // bevel).
        gNoMask = TRUE;
    }

#if BIGDOCUMENT
	fmc.filterRect32 = GetFilterRect();	//needed?
	scaledFilterRect = fmc.filterRect32;
    scaleRect32(&scaledFilterRect, 1, fmc.scaleFactor);
    X0 = scaledFilterRect.left;
    Y0 = scaledFilterRect.top;