	
	return 0;

}

int copyArray (int src, int dest)
{

    if (src<0 || src>99) return 0;	// afh 01-may2008
    if (Array[src]==NULL) return 0;
    if (ArraySize[src] == 0) return 0;
    if (dest<0 || dest>99) return 0;
    //Addded by Harald Heim, March 26, 2003
    if (src == dest) return 1;
    
#ifndef APP
	if (arrayMode == 1){
			
		//Free previous buffer
		//if (ArrayID[dest]!=0) {
		if (ArrayID[dest]!=0 || dest == PhotoPlusArray) { 
			gStuff->bufferProcs->unlockProc(ArrayID[dest]);
			gStuff->bufferProcs->freeProc(ArrayID[dest]); 
			if (dest == PhotoPlusArray) PhotoPlusArray = -1;
		} else if (Array[dest]!=0){
			free (Array[dest]);
		}
		
		ArrayID[dest] = 0;
		Array[dest] = 0;
		
		if (gStuff->bufferProcs->allocateProc((int32) ArraySize[src], &ArrayID[dest]) == noErr){
			Array[dest] = gStuff->bufferProcs->lockProc(ArrayID[dest], true);
		} else {
			Array[dest] = realloc (Array[dest], (unsigned32) ArraySize[src]); //Fall back to Windows API
		}

	}else {
#endif

		Array[dest] = realloc (Array[dest], ArraySize[src]);

#ifndef APP
	}
#endif
    

    if (Array[dest]==NULL)
    {
        ArrayID[dest]=0;
		ArraySize[dest]=0;
        ArrayX[dest] = 0;
        ArrayY[dest] = 0;
        ArrayZ[dest] = 0;
        ArrayPadding[dest]=0;
        return 0;
    }
    else
    {   
        memcpy (Array[dest], Array[src], ArraySize[src] );

        ArrayX[dest] = ArrayX[src];
        ArrayY[dest] = ArrayY[src];
        ArrayZ[dest] = ArrayZ[src];
        ArrayBytes[dest] = ArrayBytes[src];
        ArraySize[dest]=ArraySize[src];
        ArrayPadding[dest]=ArrayPadding[src];
        return 1;
    }
    return 0;

}

/*
int swapArray (int src, int dest)
{

    INT_PTR temp;
    int t;

    if (src<0 || src>99) return 0;	// afh 01-may2008
    if (dest<0 || dest>99) return 0;
    if (src == dest) return true;
    
    temp = (INT_PTR)Array[src];
    Array[src] = Array[dest];
    Array[dest] = (void *)temp;

    temp = (INT_PTR)ArrayID[src];
    ArrayID[src] = ArrayID[dest];
    ArrayID[dest] = (BufferID) temp;

	temp = ArraySize[src];
    ArraySize[src] = ArraySize[dest];
    ArraySize[dest] = temp;

    t = ArrayX[src];
    ArrayX[src] = ArrayX[dest];
    ArrayX[dest] = t;

    t = ArrayY[src];
    ArrayY[src] = ArrayY[dest];
    ArrayY[dest] = t;

    t = ArrayZ[src];
    ArrayZ[src] = ArrayZ[dest];
    ArrayZ[dest] = t;

    t = ArrayBytes[src];
    ArrayBytes[src] = ArrayBytes[dest];
    ArrayBytes[dest] = t;

    t = ArrayPadding[src];
    ArrayPadding[src] = ArrayPadding[dest];
    ArrayPadding[dest] = t;
    
    return true;
}*/



int fillArray (int nr, int val)
{
    UINT_PTR i;

    if (nr<0 || nr>99 || Array[nr]==NULL || ArraySize[nr] == 0) return 0;	// afh 01-may-2008

    if (ArrayBytes[nr]==1){
        
        if (val<0) val=0; 
        else if (val>255) val=255;

        memset( Array[nr], val, ArraySize[nr] );

    } else if (ArrayBytes[nr]==2){

        if (val<0) val=0; 
        else if (val>65535) val=65535;

        //this could be optimized...
        for (i=0;i<ArraySize[nr];i+=2){
             *(unsigned16 *)(((unsigned8 *) Array[nr]) + i)  = val;
        }

    } else if (ArrayBytes[nr]==4){
        
        //this could be optimized...
        for (i=0;i<ArraySize[nr];i+=4){
            *(unsigned32 *)(((unsigned8 *) Array[nr]) + i ) = val;
        }
    
	} else if (ArrayBytes[nr]==8){
        double dval = (double)val;
        
        for (i=0;i<ArraySize[nr];i+=8){
            //could maybe do a 64-bit int copy??-afh
            *(double *)(((unsigned8 *) Array[nr]) + i ) = dval;
        }
	}

    return true;
}

//----------------------------------------------------------------
//  bool = ffillArray(nr, dval)
//
//      Fill an Array (nr) with a f.p. value dval.
//      dval will be converted to half, float, or double if
//      ArrayBytes[nr] is 2, 4, or 8, resp.
//
//  Returns false if failed (bad value for nr, Array not
//      allocated, ArrayBytes not 2, 4, or 8, etc.).
//  Returns true if successful.
//----------------------------------------------------------------
int ffillArray (int nr, double dval)
{
    UINT_PTR i;

    if (nr<0 || nr>99 || Array[nr]==NULL || ArraySize[nr] == 0) return false;

#if HALFFLOAT
    if (ArrayBytes[nr]==2){
        //fill with half (16-bit f.p.) value
        unsigned short hval;
		UIF fval;
		fval.f = (float)dval;
        hval = floatToHalf( fval.i );

        //this could be optimized...
        for (i=0;i<ArraySize[nr];i+=2){
             *(unsigned short *)(((unsigned8 *) Array[nr]) + i) = hval;
        }

    } else 
#endif		
	if (ArrayBytes[nr]==4){
        //fill with float (32-bit f.p.) value
        float fval = (float)dval;
        //this could be optimized...
        for (i=0;i<ArraySize[nr];i+=4){
            *(float *)(((unsigned8 *) Array[nr]) + i ) = fval;
        }
    
	} else if (ArrayBytes[nr]==8){
        //fill with double (64-bit f.p.) value
        for (i=0;i<ArraySize[nr];i+=8){
            //could maybe do a 64-bit int fill instead??
            *(double *)(((unsigned8 *) Array[nr]) + i ) = dval;
        }
    } else
        return false; //bad value for ArrayBytes[nr].

    return true;

}//ffillArray




int ctlEnabled(int n)
{
//#if 0
    //// NO! Do not perpetuate this travesty!!!
    //// See note at enableCtl(). AFH 20Aug2008
    int state= (int)gParams->ctl[n].enabled & 3;

    if (state==3) return -1;
    else return state;
//#else
    return gParams->ctl[n].enabled;
//#endif
} 

#define MAX_RECURSIVE_TAB_CONTROLS 16
/*	ctlEnabledAs

	Returns the enabled state as the control is rendered on screen, not the internal state.
	Initially the "enabledAs" state depends on whether it has a Tab control assigned, but
	it could be easily changed to allow other parent controls or external criteria.
	
	Returns:
		0 (invisible and disabled)
		1 (visible but disabled)
		3 (visible and enabled)
*/ //MWVDL 2008-08-19
int sub_ctlEnabledAs(int n, int depth)
{
	//if(depth < 0) {
	//	ErrorOk("More than %d recursive tab controls", MAX_RECURSIVE_TAB_CONTROLS);
	//}
	
	if ((n < 0 || n >= N_CTLS || !gParams->ctl[n].inuse) || n==CTL_PREVIEW ){ //&& n != CTL_PROGRESS && n != CTL_FRAME && n != CTL_ZOOM) { //&& n != CTL_PREVIEW
        return 0;
    }

    {//scope
        int level = gParams->ctl[n].enabled;					// by default, level is n.enabled
        const int t = gParams->ctl[n].tabControl;
        if (t >= 0) {											// is tabControl set
	        if (t < N_CTLS											// does tabControl exist? (in range)
		     && gParams->ctl[t].inuse								// is tabControl used?
	         && (gParams->ctl[t].ctlclass == CC_TAB || gParams->ctl[t].ctlclass == CC_LISTBAR)   // is tabControl a tab control?
	         && gParams->ctl[t].val == gParams->ctl[n].tabSheet) {	// is tabControl's sheet same as n's sheet?
		        level = min(level, sub_ctlEnabledAs(t, --depth));		// lowest state of enabled, ctlEnabledAs(tab)
	        } else {
		        level = 0;											// invisible
	        }
        }
	    return level;
    }//scope
}
int ctlEnabledAs(int n)
{
	return sub_ctlEnabledAs(n, MAX_RECURSIVE_TAB_CONTROLS);
}



//function added by Harald Heim on Dec 7, 2002
int rgb2lab(int r, int g, int b, int z)
{

    double rVal, gVal, bVal;
    double xVal, yVal, zVal;
    double lVal, aVal;
    double fX, fY, fZ;

    THREAD_LOCAL static int Cr,Cg,Cb;
    THREAD_LOCAL static int Cl,Ca,Cb2;


    if (r==Cr && g==Cg && b==Cb){ //Read from Cache

        if (z==0)
            return Cl;
        else if (z==1)
            return Ca;
        else
            return Cb2;   
