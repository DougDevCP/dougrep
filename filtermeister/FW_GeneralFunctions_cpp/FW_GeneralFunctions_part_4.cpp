        else if (x >= fmc.X) x = fmc.X - 1;
        if (y < 0) y = 0;
        else if (y >= fmc.Y) y = fmc.Y - 1;
        //(optimize this later, after it's been debugged!!!)
        return fmc.t3buf[(y*fmc.X + x)*fmc.Z + z];
#endif
    }
    else {
        // Bad channel number???
        return 0;
    }
} /*fm_t3get*/

int t3get16(const int x0, const int y0, const int z0)
// Must not modify its input parameters due to cnvX/Y in-line optimizations!!!
{
    register int x = x0, y = y0, z = z0;
#if 1
    if (!fmc.t3buf) {
        // t3buf wasn't allocated????
        //ErrorOk("No tile buffer 3!");
        return 0;
    }
#endif
    // Get channel value from (temporary) tile buffer 2.
    if (z >= 0 && z < fmc.Z)
    {   // get an rgba pixel
        // pin the coordinates (i.e., replicate edges)...
        // Should replicate at IMAGE edge, not TILE edge???
#if 0
        //Min version (tile buffer is size of output tile)
        if (x < fmc.x_start) x = fmc.x_start;
        else if (x >= fmc.x_end) x = fmc.x_end - 1;
        if (y < fmc.y_start) y = fmc.y_start;
        else if (y >= fmc.y_end) y = fmc.y_end - 1;
        //(optimize this later, after it's been debugged!!!)
        return fmc.t3buf[((y-fmc.y_start)*fmc.columns + (x-fmc.x_start))*fmc.Z + z];
#else
        //Max version (tile buffer is size of input tile)
        if (x < 0) x = 0;
        else if (x >= fmc.X) x = fmc.X - 1;
        if (y < 0) y = 0;
        else if (y >= fmc.Y) y = fmc.Y - 1;
        //(optimize this later, after it's been debugged!!!)
		return *(unsigned16 *)(((unsigned8 *)fmc.t3buf) + 2*((y*fmc.X + x)*fmc.Z + z));
#endif
    }
    else {
        // Bad channel number???
        return 0;
    }
} /*fm_t3get16*/

int t4get(const int x0, const int y0, const int z0)
// Must not modify its input parameters due to cnvX/Y in-line optimizations!!!
{
    register int x = x0, y = y0, z = z0;
#if 1
    if (!fmc.t4buf) {
        // t4buf wasn't allocated????
        //ErrorOk("No tile buffer 4!");
        return 0;
    }
#endif
    // Get channel value from (temporary) tile buffer 2.
    if (z >= 0 && z < fmc.Z)
    {   // get an rgba pixel
        // pin the coordinates (i.e., replicate edges)...
        // Should replicate at IMAGE edge, not TILE edge???
#if 0
        //Min version (tile buffer is size of output tile)
        if (x < fmc.x_start) x = fmc.x_start;
        else if (x >= fmc.x_end) x = fmc.x_end - 1;
        if (y < fmc.y_start) y = fmc.y_start;
        else if (y >= fmc.y_end) y = fmc.y_end - 1;
        //(optimize this later, after it's been debugged!!!)
        return fmc.t4buf[((y-fmc.y_start)*fmc.columns + (x-fmc.x_start))*fmc.Z + z];
#else
        //Max version (tile buffer is size of input tile)
        if (x < 0) x = 0;
        else if (x >= fmc.X) x = fmc.X - 1;
        if (y < 0) y = 0;
        else if (y >= fmc.Y) y = fmc.Y - 1;
        //(optimize this later, after it's been debugged!!!)
        return fmc.t4buf[(y*fmc.X + x)*fmc.Z + z];
#endif
    }
    else {
        // Bad channel number???
        return 0;
    }
} /*fm_t4get*/

int t4get16(const int x0, const int y0, const int z0)
// Must not modify its input parameters due to cnvX/Y in-line optimizations!!!
{
    register int x = x0, y = y0, z = z0;
#if 1
    if (!fmc.t4buf) {
        // t4buf wasn't allocated????
        //ErrorOk("No tile buffer 4!");
        return 0;
    }
#endif
    // Get channel value from (temporary) tile buffer 2.
    if (z >= 0 && z < fmc.Z)
    {   // get an rgba pixel
        // pin the coordinates (i.e., replicate edges)...
        // Should replicate at IMAGE edge, not TILE edge???
#if 0
        //Min version (tile buffer is size of output tile)
        if (x < fmc.x_start) x = fmc.x_start;
        else if (x >= fmc.x_end) x = fmc.x_end - 1;
        if (y < fmc.y_start) y = fmc.y_start;
        else if (y >= fmc.y_end) y = fmc.y_end - 1;
        //(optimize this later, after it's been debugged!!!)
        return fmc.t4buf[((y-fmc.y_start)*fmc.columns + (x-fmc.x_start))*fmc.Z + z];
#else
        //Max version (tile buffer is size of input tile)
        if (x < 0) x = 0;
        else if (x >= fmc.X) x = fmc.X - 1;
        if (y < 0) y = 0;
        else if (y >= fmc.Y) y = fmc.Y - 1;
        //(optimize this later, after it's been debugged!!!)
		return *(unsigned16 *)(((unsigned8 *)fmc.t4buf) + 2*((y*fmc.X + x)*fmc.Z + z));
#endif
    }
    else {
        // Bad channel number???
        return 0;
    }
} /*fm_t4get16*/


int src(const int x0, const int y0, const int z0)
// Must not modify its input parameters due to cnvX/Y in-line optimizations!!!
{   ///// THIS VERSION ONLY WORKS WHEN FILTERING ENTIRE RECT AT ONCE!!!
    ///// OOPS- That's NFG for the proxy if it's clipped -- so in addPadding,
    /////     we set the pad amount astronomically high to always grab the
    /////     entire rect (assuming DONT_USE_MASK is set).
    register int x = x0, y = y0, z = z0;
    
    if (z >= 0 && z < fmc.planesWithoutAlpha) //3 //gStuff->planes //gStuff->inHiPlane
    {   // get an rgb pixel

        // pin the coordinates (i.e., replicate edges)...
        //Added by Harald Heim, Feb 13, 02

    //#ifndef HYPERTYLE
	#if !defined(HYPERTYLE) && !defined(PLUGINGALAXY)
        if (x < 0) x = 0; else if (x >= fmc.X) x = fmc.X - 1;
        if (y < 0) y = 0; else if (y >= fmc.Y) y = fmc.Y - 1;
    #else
        switch (edgeMode)
        {
            case 0:
                if (x < 0) x = 0; else if (x >= fmc.X) x = fmc.X - 1;
                if (y < 0) y = 0; else if (y >= fmc.Y) y = fmc.Y - 1;
                break;

            case 1:
                if (x < 0 || y < 0 || x >= fmc.X || y >= fmc.Y)
                    return 0;
				break;			//was missing!!
        
            case 2:
                //if (x < 0) x = (fmc.X - 1) + x;				/// or use MOD ???
                //else if (x >= fmc.X) x = (x -(fmc.X-1)) % fmc.X;
                //if (y < 0) y = (fmc.Y - 1) + y;
                //else if (y >= fmc.Y) y = (y -(fmc.Y-1)) % fmc.Y;
				if (x < 0) x = (fmc.X-1) + x%(fmc.X-1);
                else if (x >= fmc.X) x = x % (fmc.X-1);
                if (y < 0) y = (fmc.Y-1) + y%(fmc.Y-1);
                else if (y >= fmc.Y) y = y % (fmc.Y-1);
                break;
            
            case 3: //two reflection modes, one of period 2N, other of period 2N-2 (as here)?
                //if (x < 0) x = - x;
                //else if (x >= fmc.X) x = (fmc.X - 1) - (x - (fmc.X - 1));       //optimize?
                //if (y < 0) y = - y;
                //else if (y >= fmc.Y) y = (fmc.Y - 1) - (y - (fmc.Y - 1));
				if (x < 0) x = - x%(fmc.X-1);
                else if (x >= fmc.X) x = (fmc.X - 1) - x%(fmc.X-1);
				if (y < 0) y = - y%(fmc.Y-1);
                else if (y >= fmc.Y) y = (fmc.Y - 1) - y%(fmc.Y-1);
                break;
        }//end switch
    #endif

        return ((unsigned8 *)MyAdjustedInData)
            [(x-fmc.x_start)*fmc.srcColDelta + (y-fmc.y_start)*fmc.srcRowDelta + z];
        // note that srcPlaneDelta is always 1
    }

    else if (z == 3 || z < fmc.planes) //==3
    {   // get a mask pixel
#if 1
        // pin the coordinates (i.e., replicate edges)...
        //Added by Harald Heim, Jul 2, 02

    //#ifndef HYPERTYLE
	#if !defined(HYPERTYLE) && !defined(PLUGINGALAXY)
        if (x < 0) x = 0; else if (x >= fmc.X) x = fmc.X - 1;
        if (y < 0) y = 0; else if (y >= fmc.Y) y = fmc.Y - 1;
    #else
        switch (edgeMode)
        {
            case 0:
                if (x < 0) x = 0; else if (x >= fmc.X) x = fmc.X - 1;
                if (y < 0) y = 0; else if (y >= fmc.Y) y = fmc.Y - 1;
                break;

            case 1:
                if (x < 0 || y < 0 || x >= fmc.X || y >= fmc.Y)
                    return 0;
				break;			//was missing!!
        
            case 2:
                //if (x < 0) x = (fmc.X - 1) + x;				/// or use MOD ???
                //else if (x >= fmc.X) x = (x -(fmc.X-1)) % fmc.X;
                //if (y < 0) y = (fmc.Y - 1) + y;
                //else if (y >= fmc.Y) y = (y -(fmc.Y-1)) % fmc.Y;
				if (x < 0) x = (fmc.X-1) + x%(fmc.X-1);
                else if (x >= fmc.X) x = x % (fmc.X-1);
                if (y < 0) y = (fmc.Y-1) + y%(fmc.Y-1);
                else if (y >= fmc.Y) y = y % (fmc.Y-1);
                break;
            
            case 3: //two reflection modes, one of period 2N, other of period 2N-2 (as here)?
                //if (x < 0) x = - x;
                //else if (x >= fmc.X) x = (fmc.X - 1) - (x - (fmc.X - 1));       //optimize?
                //if (y < 0) y = - y;
                //else if (y >= fmc.Y) y = (fmc.Y - 1) - (y - (fmc.Y - 1));
				if (x < 0) x = - x%(fmc.X-1);
                else if (x >= fmc.X) x = (fmc.X - 1) -x%(fmc.X-1);
				if (y < 0) y = - y%(fmc.Y-1);
                else if (y >= fmc.Y) y = (fmc.Y - 1) -y%(fmc.Y-1);
                break;
        }//end switch
    #endif
#else
        // alpha outside image/selection is assumed 0...
        if (x < 0 || x >= fmc.X || y < 0 || y >= fmc.Y)
        {   // pin the coordinates (return 0 if outside, NOT replicate edges!!!)
            return 0;
        }
        else
#endif

        //Causes problems when reading the alpha of greyscale images, so z==3 was added
        if (gNoMask && z==3)
        {   // entire filter rect is presumed selected
            return 255;
        }

//Commented out by Harald Heim, Feb 09, 2002
/*
#ifndef DONT_USE_MASK
        else if (gStuff->haveMask)
        {   // get value from mask data
            return ((unsigned8 *)MyAdjustedMaskData)
                [(x-fmc.x_start)*fmc.mskColDelta + (y-fmc.y_start)*fmc.mskRowDelta];
        }
#endif
*/
        //else
        //{   // get value from alpha channel
            return ((unsigned8 *)MyAdjustedInData)
                [(x-fmc.x_start)*fmc.srcColDelta + (y-fmc.y_start)*fmc.srcRowDelta + z]; //3 //gStuff->planes
        //}
    }

    else
    {   // bad channel number??
        return 0;   // ???
    }
} /*fm_src*/

int src16(const int x0, const int y0, const int z0)
// Must not modify its input parameters due to cnvX/Y in-line optimizations!!!
{   // 16-bit version.
    ///// THIS VERSION ONLY WORKS WHEN FILTERING ENTIRE RECT AT ONCE!!!
    ///// OOPS- That's NFG for the proxy if it's clipped -- so in addPadding,
    /////     we set the pad amount astronomically high to always grab the
    /////     entire rect (assuming DONT_USE_MASK is set).
    register int x = x0, y = y0, z = z0;

    if (z >= 0 && z < fmc.planesWithoutAlpha) //3 //gStuff->planes
    {   // get an rgba... pixel
        // pin the coordinates (i.e., replicate edges)...
        
    //#ifndef HYPERTYLE
	#if !defined(HYPERTYLE) && !defined(PLUGINGALAXY)
        if (x < 0) x = 0; else if (x >= fmc.X) x = fmc.X - 1;
        if (y < 0) y = 0; else if (y >= fmc.Y) y = fmc.Y - 1;
    #else
		switch (edgeMode)
        {
            case 0:
                if (x < 0) x = 0; else if (x >= fmc.X) x = fmc.X - 1;