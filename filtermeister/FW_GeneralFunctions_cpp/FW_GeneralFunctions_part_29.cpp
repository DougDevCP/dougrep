    if (last<0 || last> N_CELLS-1) return 0;
    if (first > last || first == last) return 0;

    //qsort( (void*) &fmc.cell[first] , (size_t) (last - first), sizeof(int), (int(*)(const void*, const void*)) comp );
    qsort( (void*) &fmc.cell[first] , (size_t) (last - first + 1), sizeof(int),  comp );

    return 1;

}
*/




//Added by Harald Heim, May 17, 2003
int quickSort(int first, int last)
{

    int temp;

    if (first<0 || first> N_CELLS-1 || last<0 || last> N_CELLS-1 || first > last || first == last) return 0;

    
    if (first < last){

        int pivot = fmc.cell[first];
        int i = first;
        int j = last;
        
        while (i < j){

            while (fmc.cell[i] <= pivot && i < last){
                    i += 1;
            }

            while (fmc.cell[j] >= pivot && j > first){
                j -= 1;
            }
                    
            if (i < j){
                temp = fmc.cell[i];
                fmc.cell[i] = fmc.cell[j];
                fmc.cell[j] = temp;
            }

        }

        //temp = fmc.cell[first];
        fmc.cell[first] = fmc.cell[j];
        fmc.cell[j] = pivot; //temp;

        quickSort(first, j-1);
        quickSort(j +1, last);
    }

    return 1;

}


//Added by Harald Heim, May 17, 2003
int quickFill(int x, int y, int z, int buffer, int radius, int xstart, int ystart, int xend, int yend)
{

    int i,j;
    int count=0;
    int n = radius*2 + 1;
    int yCoord=0,xCoord=0,value=0;

    if (n*n > N_CELLS) return 0; //not enough put/get cells;


    for (i = 0; i < n; i++) {
    for (j = 0; j < n; j++) {

        xCoord = x - radius + i;
        yCoord = y - radius + j;

        if (buffer<10){ //Needs to be done otherwise Memory Access Error -> why???
            if (xCoord  < 0) xCoord  = 0;
            else if (xCoord  >= fmc.X) xCoord  = fmc.X - 1;
            if (yCoord < 0) yCoord = 0;
            else if (yCoord >= fmc.Y) yCoord = fmc.Y - 1;
        }

        if (xCoord < xstart-fmc.needPadding) xCoord = xstart-fmc.needPadding;
        else if (xCoord >= xend+fmc.needPadding) xCoord = xend - 1+fmc.needPadding;
        if (yCoord < ystart-fmc.needPadding) yCoord = ystart-fmc.needPadding;
        else if (yCoord >= yend+fmc.needPadding) yCoord = yend - 1+fmc.needPadding;
            

        switch (buffer){
            case 0: //input buffer
                if (gStuff->imageMode < 10){
                    value = ((unsigned8 *)MyAdjustedInData)
                        [(xCoord -fmc.x_start)*fmc.srcColDelta + (yCoord - fmc.y_start)*fmc.srcRowDelta + z];
                } else {
                    value =  *(unsigned16 *)(((unsigned8 *)MyAdjustedInData) +
                                   (((xCoord-fmc.x_start)*fmc.srcColDelta + z) << 1) +
                                   (yCoord-fmc.y_start)*fmc.srcRowDelta);
                }
                break;
            case 1: //temp buffer 1
                value = fmc.tbuf[( yCoord *fmc.X + xCoord )*fmc.Z + z];
                break;
            case 2: //temp buffer 2
                value = fmc.t2buf[( yCoord *fmc.X + xCoord )*fmc.Z + z];
                break;
            case 3: //output buffer
                value = ((unsigned8 *)gStuff->outData)
                    [(xCoord -fmc.x_start)*fmc.dstColDelta + (yCoord -fmc.y_start)*fmc.dstRowDelta + z];
            case 10:
            case 11:
            case 12:
            case 13:
            case 14:
            case 15:
            case 16:
            case 17:
            case 18:
            case 19:
                if (ArrayBytes[buffer-10] == 1)
                    value = ((unsigned8 *)Array[buffer-10]) [ (( (yCoord+fmc.needPadding) *ArrayX[buffer-10] + (xCoord+fmc.needPadding) )*ArrayZ[buffer-10] + z) ];
                else if (ArrayBytes[buffer-10] == 2)
                    value =  *(unsigned16 *)(((unsigned8 *)Array[buffer-10]) + (( (yCoord+fmc.needPadding) *ArrayX[buffer-10] + (xCoord+fmc.needPadding) )*ArrayZ[buffer-10] + z)*ArrayBytes[buffer-10] );
                else if (ArrayBytes[buffer-10] == 4)
                    value =  *(unsigned32 *)(((unsigned8 *)Array[buffer-10]) + (( (yCoord+fmc.needPadding) *ArrayX[buffer-10] + (xCoord+fmc.needPadding) )*ArrayZ[buffer-10] + z)*ArrayBytes[buffer-10] );
        }//switch

        fmc.cell[count] = value;
        count++;
    }}


    return 1;

}



/*
 *  This Quickselect routine is based on the algorithm described in
 *  "Numerical recipes in C", Second Edition,
 *  Cambridge University Press, 1992, Section 8.5, ISBN 0-521-43108-5
 *  This code by Nicolas Devillard - 1998. Public domain.
 */ 

#define ELEM_SWAP(a,b) { register int t=(a);(a)=(b);(b)=t; }


int quickMedian(int low, int high)//(elem_type arr[], int n) 
{
    int median = (low + high) / 2;
    int middle, ll, hh;

    if (low<0 || low> N_CELLS-1 || high<0 || high> N_CELLS-1 || low > high || low == high) return 0;


    for (;;) {
        if (high <= low) /* One element only */
            return fmc.cell[median] ;

        if (high == low + 1) {  /* Two elements only */
            if (fmc.cell[low] > fmc.cell[high])
                ELEM_SWAP(fmc.cell[low], fmc.cell[high]) ;
            return fmc.cell[median] ;
        }

        /* Find median of low, middle and high items; swap into position low */
        middle = (low + high) / 2;
        if (fmc.cell[middle] > fmc.cell[high])    ELEM_SWAP(fmc.cell[middle], fmc.cell[high]) ;
        if (fmc.cell[low] > fmc.cell[high])       ELEM_SWAP(fmc.cell[low], fmc.cell[high]) ;
        if (fmc.cell[middle] > fmc.cell[low])     ELEM_SWAP(fmc.cell[middle], fmc.cell[low]) ;

        /* Swap low item (now in position middle) into position (low+1) */
        ELEM_SWAP(fmc.cell[middle], fmc.cell[low+1]) ;

        /* Nibble from each end towards middle, swapping items when stuck */
        ll = low + 1;
        hh = high;
        for (;;) {
            do ll++; while (fmc.cell[low] > fmc.cell[ll]) ;
            do hh--; while (fmc.cell[hh]  > fmc.cell[low]) ;

            if (hh < ll)
            break;

            ELEM_SWAP(fmc.cell[ll], fmc.cell[hh]) ;
        }

        /* Swap middle item (in position low) back into correct position */
        ELEM_SWAP(fmc.cell[low], fmc.cell[hh]) ;

        /* Re-set active partition */
        if (hh <= median)
            low = ll;
        if (hh >= median)
            high = hh - 1;
    }

}
#undef ELEM_SWAP



char * getCtlText(int n)
{
	static char dummystr[20];
	strcpy(dummystr, "Not Available"); //Return this string to avoid memory access violations

    if (n < 0 || n >= N_CTLS || !gParams->ctl[n].inuse) {        
			return (char *)(unsigned8 *)dummystr;
    }

	return (char *) (unsigned8 *) gParams->ctl[n].label;

} /*fm_getCtlText*/



int getCtlClass(int n)
{

	if (n == CTL_PREVIEW)

		return CC_PREVIEW;
	
	else if (n == CTL_PROGRESS)

		return CC_PROGRESSBAR;

	else if (n == CTL_ZOOM)

		return CC_ZOOM;

	else if ( n >= 0 && n < N_CTLS && gParams->ctl[n].inuse){
        
        return gParams->ctl[n].ctlclass;
    }

    return 0;  

} /*fm_getCtlClass*/


int getCtlView(int n){

	int styles;

	if (n < 0 || n >= N_CTLS || !gParams->ctl[n].inuse || gParams->ctl[n].ctlclass != CC_LISTVIEW) {
        return FALSE;
    }

	styles = (int)GetWindowLong(gParams->ctl[n].hCtl, GWL_EXSTYLE);

	if (styles & LVS_ICON) return LVS_ICON;
	else if (styles & LVS_REPORT) return LVS_REPORT;
	else if (styles &  LVS_SMALLICON ) return  LVS_SMALLICON ;
	else if (styles & LVS_LIST) return LVS_LIST;
	else return (int) ListView_GetView(gParams->ctl[n].hCtl); //Only work under XP and higher
	
	return -1; //Unknown
}


int getComboOpen(int n){

	if (n < 0 || n >= N_CTLS || !gParams->ctl[n].inuse || gParams->ctl[n].ctlclass != CC_COMBOBOX) {
        return FALSE;
    }

	return (int)SendMessage(gParams->ctl[n].hCtl,CB_GETDROPPEDSTATE,0,0);

}


int getCtlItemCount(int n)
{

	if (n < 0 || n >= N_CTLS || !gParams->ctl[n].inuse) {
        return FALSE;
    }

	if (gParams->ctl[n].ctlclass == CC_LISTBOX || gParams->ctl[n].ctlclass == CC_LISTBAR)
		return (int) SendMessage(gParams->ctl[n].hCtl, LB_GETCOUNT, 0, 0);
	else if (gParams->ctl[n].ctlclass == CC_COMBOBOX)
#if SKIN
		if (isSkinActive() && gParams->skinStyle>0)
			return (int) getCtlItemString(n,-1,NULL);
		else
#endif
			return (int) SendMessage(gParams->ctl[n].hCtl, CB_GETCOUNT, 0, 0);
	else if (gParams->ctl[n].ctlclass == CC_TAB)
		return TabCtrl_GetItemCount(gParams->ctl[n].hCtl);
	else if (gParams->ctl[n].ctlclass == CC_LISTVIEW)
		return ListView_GetItemCount(gParams->ctl[n].hCtl);
	else 
		return -1;

}