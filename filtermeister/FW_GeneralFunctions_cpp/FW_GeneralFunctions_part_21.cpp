#endif
   

    // Compute min/max values for column and row indices that still lie
    // within the (scaled) filterRect (used to simulate padding of accesses
    // outside the bounds of filterRect).
    /*minCol = scaledFilterRect.left   - gStuff->outRect.left;
    maxCol = scaledFilterRect.right  - gStuff->outRect.left - 1;
    minRow = scaledFilterRect.top    - gStuff->outRect.top;
    maxRow = scaledFilterRect.bottom - gStuff->outRect.top - 1;*/

    { //scope
    //const int bevWidth = (ctl(7)*2+fmc.scaleFactor)/(fmc.scaleFactor*2);  // round to nearest
    const int bw = bevWidth;
    const int bwnz = bw ? bw : 1;  // Non-zero, so safe as a divisor...
    const int bw2 = bw*bw;
    const int bw2nz = bwnz*bwnz;   // Non-zero, so safe as a divisor...

    /* set local globals */
    gBevWidth = bevWidth;




        for (i=-maxBlurRadius; i < fmc.rows + maxBlurRadius; i++)
        {
            for (j=-maxBlurRadius; j < fmc.columns + maxBlurRadius; j++)
            {
                DIST(i,j).x = bw + 1;
                DIST(i,j).y = bw + 1;
            } // for j

        } //for i



        /*
        //MEthod1: TOO SLOW!!!
        // pass 1: find distance (vector) from nearest unmasked point...
        if (!(gParams->flags & AFH_OPTIMIZE_BEVEL))
        {   // method 1: O(n^4) brute force
            for (i=-maxBlurRadius; i < fmc.rows + maxBlurRadius; i++)
            {
                fmc.mskPtr =
                    fmc.mskData + (i * fmc.mskRowDelta) - (maxBlurRadius * fmc.mskColDelta);

                for (j=-maxBlurRadius; j < fmc.columns + maxBlurRadius; j++)
                {
                    if (mskVal(i,0,j,0))
                    {   // inside the mask
                        int d = DIST(i,j).x*DIST(i,j).x + DIST(i,j).y*DIST(i,j).y;

                        for (i0 = -bevWidth; i0 <= bevWidth; i0++)
                        {
                            for (j0 = -bevWidth; j0 <= bevWidth; j0++)
                            {
                                int a0 = mskVal(i,i0,j,j0);
                                int d0 = i0*i0 + j0*j0;
                                if (a0 == 0 && d0 <= bw2)
                                {   // found a point outside the mask
                                    if (d0 < d)
                                    {   // closer than previous...
                                        DIST(i,j).x = j0;
                                        DIST(i,j).y = i0;
                                        d = i0*i0 + j0*j0;
                                    }
                                }
                            } // for j0
                        } // for i0
                    } //if

                    fmc.mskPtr += fmc.mskColDelta;

                } // for j
            } // for i 
        } //bevel method 1
        */
        //else
        {


            // Method 2: O(n^2+) router simulation.
            // (j,i) = center of router bit.
            // For each (j,i) on the outside edge of the mask,
            // sweep out the path of the router blade, recording
            // any new minima to points hit inside the mask.
            // By optimizing the sweep, we can keep the whole
            // thing down to slightly worse than O(n^2).
            int pad = bevWidth + maxBlurRadius;

            for (i=-pad; i < fmc.rows + pad; i++)
            {
                fmc.mskPtr = fmc.mskData + (i * fmc.mskRowDelta) - (pad * fmc.mskColDelta);

                for (j=-pad; j < fmc.columns + pad; j++)
                {
                    if (mskVal(i,0,j,0) == 0)
                    {   // outside the mask...
                        // is it on the edge of the mask?
                        int i1 = -bevWidth;
                        int i2 =  bevWidth;
                        int i3 = -bevWidth;
                        int i4 =  bevWidth;
    #if 0
                        if (mskVal(i, 1, j, 0) ||
                            mskVal(i,-1, j, 0) ||
                            mskVal(i, 0, j, 1) ||
                          //mskVal(i, 1, j, 1) ||
                          //mskVal(i, 1, j,-1) ||
                          //mskVal(i,-1, j, 1) ||
                          //mskVal(i,-1, j,-1) ||
                            mskVal(i, 0, j,-1) )
    #else
                        if (i < -maxBlurRadius)
                            i1 = -i - maxBlurRadius;
                        else if (mskVal(i,-1,j,0))
                            i1 = -bevWidth;  // can tighten this...
                        else
                            i1 = 0;
                        if (i + i1 < -maxBlurRadius)    ////// redundant????????
                            i1 = -i - maxBlurRadius;

                        if (i > fmc.rows + maxBlurRadius - 1)
                            i2 = -i + fmc.rows + maxBlurRadius - 1;
                        else if (mskVal(i,1,j,0))
                            i2 = bevWidth;  // can tighten this...
                        else
                            i2 = 0;
                        if (i + i2 > fmc.rows + maxBlurRadius -1)
                            i2 = -i + fmc.rows + maxBlurRadius - 1;

                        if (j < -maxBlurRadius)
                            i3 = -j - maxBlurRadius;
                        else if (mskVal(i,0,j,-1))
                            i3 = -bevWidth;  // can tighten this...
                        else
                            i3 = 0;
                        if (j + i3 < -maxBlurRadius)
                            i3 = -j - maxBlurRadius;

                        if (j > fmc.columns + maxBlurRadius - 1)
                            i4 = -j + fmc.columns + maxBlurRadius - 1;
                        else if (mskVal(i,0,j,1))
                            i4 = bevWidth;  // can tighten this...
                        else
                            i4 = 0;
                        if (j + i4 > fmc.columns + maxBlurRadius -1)
                            i4 = -j + fmc.columns + maxBlurRadius - 1;

                        //i1 = mskVal(i,-1, j, 0) ? -bevWidth : 0;
                        //i2 = mskVal(i, 1, j, 0) ?  bevWidth : 0;
                        //i3 = mskVal(i, 0, j,-1) ? -bevWidth : 0;
                        //i4 = mskVal(i, 0, j, 1) ?  bevWidth : 0;
    #endif
                        {   // skip altogether if (i1 >= i2) && (i3 >= i4) ???
                            for (i0 = i1; i0 <= i2; i0++)
                            {
                                for (j0 = i3; j0 <= i4; j0++)
                                {
                                    int a0 = mskVal(i,i0,j,j0);
                                    int d0 = i0*i0 + j0*j0;
                                    if (a0 != 0 && d0 <= bw2)
                                    {   // found a point inside the mask
                                        int d = DIST(i+i0,j+j0).x*DIST(i+i0,j+j0).x + 
                                                DIST(i+i0,j+j0).y*DIST(i+i0,j+j0).y;
                                        if (d0 < d)
                                        {   // closer than previous...
                                            DIST(i+i0,j+j0).x = -j0;
                                            DIST(i+i0,j+j0).y = -i0;
                                        }
                                    }
                                } /* for j0 */
                            } /* for i0 */
                        } /*if on edge*/
                    } /*if*/

                    fmc.mskPtr += fmc.mskColDelta;

                } /* for j */
            } /* for i */
        } /*bevel method 2*/

    }//scope

    return 1;

}

int freeSBD(void)
{
    // Release the dist array...
    UnlockBuffer(bufferID);
    FreeBuffer(bufferID);
    bufferID = 0;
    return 1;
}

int getSBD(int j, int i)
{
    int valx,valy;
    if (j < fmc.x_start || j > fmc.x_end-1) return 0;
    if (i < fmc.y_start || i > fmc.y_end-1) return 0;
    i=i-fmc.y_start;
    j=j-fmc.x_start;
    if (i<0) i=0;
    else if (i> fmc.rows-1) i=fmc.rows-1;
    if (j<0) j=0;
    else if (j> fmc.columns-1) j=fmc.columns-1;
    valx=DIST(i,j).x;
    valy=DIST(i,j).y;

    return valx*valx + valy*valy;

}

int getSBDangle(int j, int i, int a10)
{
    int valx,valy;
    double phi = -((double) a10 / 100.0) * 6.28318530717958647693 / 360.0;

    if (j < fmc.x_start || j > fmc.x_end-1) return 0;
    if (i < fmc.y_start || i > fmc.y_end-1) return 0;
    i=i-fmc.y_start;
    j=j-fmc.x_start;
    if (i<0) i=0;
    else if (i> fmc.rows-1) i=fmc.rows-1;
    if (j<0) j=0;
    else if (j> fmc.columns-1) j=fmc.columns-1;
    valx=DIST(i,j).x;
    valy=DIST(i,j).y;

    //return (int)( (256.0 * 0.63661977236758134308) *  (atan2((double)valy, (double)valx) )   );
    return (int)( 512.0 *  cos (atan2((double)valy, (double)valx) - phi)   );

}


int getSBDX(int j, int i)
{
    if (j < fmc.x_start || j > fmc.x_end-1) return 0;
    if (i < fmc.y_start || i > fmc.y_end-1) return 0;
    i=i-fmc.y_start;
    j=j-fmc.x_start;
    if (i<0) i=0;
    else if (i> fmc.rows-1) i=fmc.rows-1;
    if (j<0) j=0;
    else if (j> fmc.columns-1) j=fmc.columns-1;
    return DIST(i,j).x;
}

int getSBDY(int j, int i)
{
    if (j < fmc.x_start || j > fmc.x_end-1) return 0;
    if (i < fmc.y_start || i > fmc.y_end-1) return 0;
    i=i-fmc.y_start;
    j=j-fmc.x_start;
    if (i<0) i=0;
    else if (i> fmc.rows-1) i=fmc.rows-1;
    if (j<0) j=0;
    else if (j> fmc.columns-1) j=fmc.columns-1;
    return DIST(i,j).y;
}

#endif



int getCtlPos (int n, int w){

    int val=0;
    
    //RECT rcCtl;
    //GetWindowRect( gParams->ctl[n].hCtl, &rcCtl) 
    //ScreenToClient (fmc.hDlg, (LPPOINT)&rcCtl);
    //PixelsToVDBUs 

	if (n == CTL_PREVIEW){ //|| getCtlClass(n)==CC_COMBOBOX){

        RECT rcCtl;
		HWND hwnd;

		if (n == CTL_PREVIEW) hwnd = GetDlgItem(fmc.hDlg, 101);
		else gParams->ctl[n].hCtl;
    
        GetWindowRect( hwnd, &rcCtl );
		ScreenToClient (fmc.hDlg, (LPPOINT)&rcCtl);
		ScreenToClient (fmc.hDlg, (LPPOINT)&(rcCtl.right));

		switch (w){
			case 0: val = PixelsToHDBUs(rcCtl.left); break;
			case 1: val = PixelsToVDBUs(rcCtl.top); break;
			case 2: val = PixelsToHDBUs(rcCtl.right - rcCtl.left); break;
			case 3: val = PixelsToVDBUs(rcCtl.bottom - rcCtl.top); break;
		}
	
	} else {

		switch (w){
			case 0: val = gParams->ctl[n].xPos; break;
			case 1: val = gParams->ctl[n].yPos; break;