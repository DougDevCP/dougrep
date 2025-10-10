    mskRectPtr->right  += rightPad;

#ifndef DONT_USE_MASK
#if BIGDOCUMENT  
    if (fmc.maskRect32.right > fmc.maskRect32.left)
    {   
		// Update maskRect for host
		SetMaskRect(fmc.maskRect32);
		// Mask was nonempty, so adjust input rectangle also
        fmc.inRect32.top    -= topPad;
        fmc.inRect32.bottom += bottomPad;
        fmc.inRect32.left   -= leftPad;
        fmc.inRect32.right  += rightPad;
    }
#else
    if (gStuff->maskRect.right > gStuff->maskRect.left)
    {   // mask is nonempty, so adjust input rectangle also
        gStuff->inRect.top    -= topPad;
        gStuff->inRect.bottom += bottomPad;
        gStuff->inRect.left   -= leftPad;
        gStuff->inRect.right  += rightPad;
    }
#endif
#endif	 

#if BIGDOCUMENT
	// Update inRect for host
	SetInRect(fmc.inRect32);
#endif

} /*addPadding*/

void subtractPadding (GPtr globals)
{
	// Remove padding from either fmc.maskRect32 or fmc.inRect32
    mskRectPtr->top    += topPad;
    mskRectPtr->bottom -= bottomPad;
    mskRectPtr->left   += leftPad;
    mskRectPtr->right  -= rightPad;

#ifndef DONT_USE_MASK
#if BIGDOCUMENT
	if (fmc.maskRect32.right > fmc.maskRect32.left)
#else
    if (gStuff->maskRect.right > gStuff->maskRect.left)
#endif
    {   	
#if BIGDOCUMENT
        // Update maskRect for host
		SetMaskRect(fmc.maskRect32);	 
		// Mask was nonempty, so adjust input rectangle also
        fmc.inRect32.top    += topPad;
        fmc.inRect32.bottom -= bottomPad;
        fmc.inRect32.left   += leftPad;
        fmc.inRect32.right  -= rightPad;   
#else	
        // mask is nonempty...
        gStuff->inRect.top    += topPad;
        gStuff->inRect.bottom -= bottomPad;
        gStuff->inRect.left   += leftPad;
        gStuff->inRect.right  -= rightPad;
#endif
        // PSP 4.12 treats maskData as read-only, so we need to make
        // our own (adjusted) copy.
        MyAdjustedMaskData = (unsigned8 *) gStuff->maskData + (topPad*gStuff->maskRowBytes + leftPad);
                                                            // note: maskColumnBytes=1
    }
#endif

		//Some hosts (e.g., PSP X3) set inColumnBytes incorrectly, so we set it correctly here
	    if (gStuff->imageMode > 9) //16-bit image data
			gStuff->inColumnBytes = 2 * ((gStuff->inHiPlane - gStuff->inLoPlane) + 1);	
		else
			gStuff->inColumnBytes = (gStuff->inHiPlane - gStuff->inLoPlane) + 1;
		
		// PSP 4.12 treats inData as read-only, so we need to make
        // our own (adjusted) copy.
		MyAdjustedInData = (unsigned8 *) gStuff->inData + (topPad*gStuff->inRowBytes + leftPad*gStuff->inColumnBytes);
#if BIGDOCUMENT
	// Update inRect for host
	SetInRect(fmc.inRect32);
#endif

} /*subtractPadding*/

/*****************************************************************************/
//
//  Stuff used to select "desirable" Proxy Preview scale factors...
//
/// 100%, 50%, 33.3%, 25%, 20%, 16.7%, 14.3%, 12.5%, 11.1%, 10%, 9.09%, 8.33%, (7.69%),
/// 7.14%, (6.67%), 6.25%, (5.88%), (5.55%), (5.26%), 5%, (4.76%),...,4%,...,3.33%,...,2.5%,
/// ...,2%,...,1.49%,...,1.33%,...,1%,...0.50%,...,0.33%,...,0.25%,...0.20%,...,0.17%,
/// ...,0.14%,...,0.12%,...,0.11%,...,0.10%,...0.05%,...0.03%,...0.02%,...,0.01%,...,
/// .005%, .002%, .001%, .0005%, .0002%, .0001%, .00005%, .00002%, .00001% <== brings 2G down to 200

#if 0
static int desirableScaleFactor[] = {
	1,		// 100%
	2,		// 50%
	3,		// 33.3%
	4,		// 25%
	5,		// 20%
	6,		// 16.7%
	7,		// 14.3%
	8,		// 12.5%
	9,		// 11.1%
	10,		// 10%
	11,		// 9.09%
	12,		// 8.33%
///	13,		//			7.69%	*skip*
	14,		// 7.14%
///	15,		//			6.67%	*skip*
	16,		// 6.25%
///	17,		//			5.88%	*skip*
///	18,		//			5.55%	*skip*
///	19,		//			5.26%	*skip*
	20,		// 5%
	25,		// 4%
	30,		// 3.33%
///	33,		//			3.03%	*skip*
	40,		// 2.5%
	50,		// 2%
	57,		// 1.75%
///	60,		//			1.67%	*skip*
	67,		// 1.49%
///	70,		//			1.43%	*skip*
	75,		// 1.33%
	80,		// 1.25%
///	90,		//			1.11%	*skip*
	91,		// 1.1%
	100,	// 1%
///	111,	//			0.901%	*skip*
	125,	// 0.8%
///	133,	//			0.752	*skip*
///	143,	//			0.699%	*skip*
///	167,	//			0.599	*skip*
	200,	// 0.5%
	250,	// 0.4%
///	300,	//			0.333%	*skip*
	333,	// 0.3%
	400,	// 0.25%
	500,	// 0.2%
///	600,	//			0.167%	*skip*
	625,	// 0.16%
	667,	// 0.15%
///	700,	//			0.143%	*skip*
	800,	// 0.125%
///	900,	//			0.111%	*skip*
	909,	// 0.11%
	1000,	// 0.1%
	1111,	// 0.09%
	1250,	// 0.08%
///	1333,	//			0.075	*skip*
	1429,	// 0.07%
	1667,	// 0.06%
	2000,	// 0.05%
	2500,	// 0.04%
///	3000,	//			0.0333%	*skip*
	3333,	// 0.03%
	5000,	// 0.02%
	10000,	// 0.01%
	11111,	// 0.009
	12500,	// 0.008%
///	13333,	//			0.0075%	*skip*
	14286,	// 0.007%
	16667,	// 0.006%
	20000,	// 0.005%
	25000,	// 0.004%
	33333,	// 0.003%
	50000,	// 0.002%
	100000,	// 0.001%
	111111,	// 0.0009%
	125000,	// 0.0008%
///	133333,	//			0.00075%	*skip*
	142857,	// 0.0007%
	166667, // 0.0006%
	200000,	// 0.0005%
	250000,	// 0.0004%
	333333,	// 0.0003%
	500000,	// 0.0002%
	1000000,// 0.0001%
	1111111,// 0.00009%
	1250000,// 0.00008%
///	1333333,//			0.000075%	*skip*
	1428571,// 0.00007%
	1666667,// 0.00006%
	2000000,// 0.00005%
	2500000,// 0.00004%
	3333333,// 0.00003%
	5000000,// 0.00002%
	10000000,// 0.00001%	//scales 2Gpx to 200px, so probably max reduction ever needed!
	INT_MAX	// ~infinity (stopper)
}; /**desirableScaleFactor*/
#else
//Using a roughly logarithmic series, with a step of 10**0.1 ~= 1.25 (1/0.8),
//which descends in one decade something like:
//100, 80, 64, 50, 40, 32, 25, 20, 16, 12.5, 10
static int desirableScaleFactor[] = {
	1,		// 100%
	//		// 80%		//no can do
	//		// 64%		//no can do
	2,		// 50%
	//		// 40%		//no can do
	3,		// 33.3%
	4,		// 25%
	5,		// 20%
	6,		// 16.7%
	8,		// 12.5%

	10,		// 10%
	12,		// 8.33%
	16,		// 6.25%
	20,		// 5%
	25,		// 4%
	30,		// 3.33%	//or 33 //3.03%??
	40,		// 2.5%
	50,		// 2%
	60,		// 1.67%
	80,		// 1.25%

	100,	// 1%
	125,	// 0.8%
	160,	// 0.625%	// or 156,	// 0.641%
	200,	// 0.5%
	250,	// 0.4%
	300,	// 0.333%
	400,	// 0.25%
	500,	// 0.2%
	625,	// 0.16%
	800,	// 0.125%

	1000,	// 0.1%
	1250,	// 0.08%
	1562,	// 0.064%
	2000,	// 0.05%
	2500,	// 0.04%
	3000,	// 0.0333%	//or 3125, // 0.032
	4000,	// 0.025%
	5000,	// 0.02%
	6250,	// 0.016%
	8000,	// 0.0125%

	10000,	// 0.01%
	12500,	// 0.008%
	15625,	// 0.0064%
	20000,	// 0.005%
	25000,	// 0.004%
	30000,	// 0.00333%
#if 0	///////////////// OOPS. For now scaleFactor can't exceed 30,000 ( or 32767).
		/////////////////       This limit is imposed because inputRate and MaskRate
		/////////////////		Are fixed point 16.16 numbers, and scaleFactor must
		/////////////////		fit in the high (signed) 16 bits!  This is a PS limitation,
		/////////////////		even in CS5.1 X64.  So if scaleFactor is <= 30000 and
		/////////////////		we want to scale an image to fit entirely within a 300 x 300 px
		/////////////////		frame, then no image dimension should exceeed 300px * 30000 =
		/////////////////		9,000,000 px, or approxmately 10Mpx.  Of course, this is well
		/////////////////		within the (published) CS5 limit of 300,000 x 300,000 px.
	40000,	// 0.0025%
	50000,	// 0.002%
	62500,	// 0.0016%
	80000,	// 0.00125%

	100000,	// 0.001%
	125000,	// 0.0008%
	156250,	// 0.00064%
	200000,	// 0.0005%
	250000,	// 0.0004%
	300000,	// 0.000333%
	400000,	// 0.00025%
	500000,	// 0.0002%
	625000,	// 0.00016%
	800000,	// 0.00125%

	1000000,// 0.0001%
	1250000,// 0.00008%
	1562500,// 0.000064%
	2000000,// 0.00005%
	2500000,// 0.00004%
	3000000,// 0.0000333%
	4000000,// 0.000025%
	5000000,// 0.00002%
	6250000,// 0.000016%
	8000000,// 0.000125%
	10000000,// 0.00001%	//scales 2Gpx to 200px, so probably max reduction ever needed!
#endif
	INT_MAX	// ~infinity (stopper)
}; /**desirableScaleFactor*/
#endif

/* Round up to a desirable scale factor */
int roundUpScaleFactor(int sf) {
	int i;
	for (i = 0; sf > desirableScaleFactor[i]; ++i) {
	}
	assert (sf <= desirableScaleFactor[i]);
	return desirableScaleFactor[i];
} /*roundUpScaleFactor*/

/* Round down to a desirable scale factor */
int roundDownScaleFactor(int sf) {