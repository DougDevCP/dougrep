    pos = (( (y+ArrayPadding[nr])*ArrayX[nr] + (x+ArrayPadding[nr]) )*ArrayZ[nr] + z);//*ArrayBytes[nr];
    
    if (pos <0 || pos*ArrayBytes[nr] >= ArraySize[nr]) return 0;


    /*if (ArrayBytes[nr] == 1)
        ((unsigned8 *)Array[nr]) [pos] = val;
    else if (ArrayBytes[nr] == 2)
        *(unsigned16 *)(((unsigned8 *) Array[nr]) + pos)  = val;
    else if (ArrayBytes[nr] == 4)
        *(unsigned32 *)(((unsigned8 *) Array[nr]) + pos ) = val;
    else
        return 0; // or better return -1 ?
    */

    switch(ArrayBytes[nr]) {
		case 1:
	        ((unsigned8 *)Array[nr]) [pos] = val;
            break;
		case 2:
	        ((unsigned16 *)Array[nr]) [pos] = val;
            break;
		case 4:
	        ((unsigned32 *)Array[nr]) [pos] = val;	
	}
    
    return 1;

}


void fast_putArray (int nr, int x, int y, int z, int val)
{
    const UINT_PTR pos = (( (y+ArrayPadding[nr])*ArrayX[nr] + (x+ArrayPadding[nr]) )*ArrayZ[nr] + z);//*ArrayBytes[nr];

    if (pos <0 || pos*ArrayBytes[nr] >= ArraySize[nr]) return;

	switch(ArrayBytes[nr]) {
		case 1:
	        ((unsigned8 *)Array[nr]) [pos] = val;
            break;
		case 2:
	        ((unsigned16 *)Array[nr]) [pos] = val;
            break;
		case 4:
	        ((unsigned32 *)Array[nr]) [pos] = val;	
	}
}

int UNSAFE_putArray (int nr, int x, int y, int z, int val)
{
    const int pos = (( (y+ArrayPadding[nr])*ArrayX[nr] + (x+ArrayPadding[nr]) )*ArrayZ[nr] + z)*ArrayBytes[nr];

	switch(ArrayBytes[nr]) {
		case 1:
	        ((unsigned8 *)Array[nr]) [pos] = val;
			return 1;
		case 2:
	        *(unsigned16 *)(((unsigned8 *) Array[nr]) + pos)  = val;
			return 1;
		case 4:
	        *(unsigned32 *)(((unsigned8 *) Array[nr]) + pos ) = val;
			return 1;
		default:
			return 0;
	}

	return 1;
}

int fputArray (int nr, int x, int y, int z, double val)
{

    UINT_PTR pos;

    if (Array[nr]==NULL || nr<0 || nr>99 || ArrayBytes[nr] < 2) return 0;
    
    pos = (( (y+ArrayPadding[nr])*ArrayX[nr] + (x+ArrayPadding[nr]) )*ArrayZ[nr] + z)*ArrayBytes[nr];
    if (pos <0 || pos >= ArraySize[nr]) return 0;

	//if (val> 3.4E+38) val=3.4E+38; else if (val< 3.4E-38) val=3.4E-38;

#if HALFFLOAT    
	if (ArrayBytes[nr] == 2){
		UIF fval;
		fval.f = (float)val;
		*(unsigned short *)(((unsigned8 *) Array[nr]) + pos ) = floatToHalf( fval.i );
    } else 
#endif		
	if (ArrayBytes[nr] == 4)
		*(float *)(((unsigned8 *) Array[nr]) + pos ) = (float)val;
	else if (ArrayBytes[nr] == 8)
		*(double *)(((unsigned8 *) Array[nr]) + pos ) = val;
    else 
        return 0; //bad value for ArrayBytes[nr].
	
    return 1;
}

int UNSAFE_fputArray (int nr, int x, int y, int z, double val)
{
    const int pos = (( (y+ArrayPadding[nr])*ArrayX[nr] + (x+ArrayPadding[nr]) )*ArrayZ[nr] + z)*ArrayBytes[nr];
    
	switch (ArrayBytes[nr]) {
		case 4:
			*(float *)(((unsigned8 *) Array[nr]) + pos ) = (float)val;
			return 1;
		case 8:
			*(double *)(((unsigned8 *) Array[nr]) + pos ) = val;
			return 1;
#if HALFFLOAT
		case 2: {
				UIF fval;
				fval.f = (float)val;
				*(unsigned short *)(((unsigned8 *) Array[nr]) + pos ) = floatToHalf( fval.i );
			}
			return 1;
#endif
		default:
			return 0;
	}
	
    return 1;
}



/*
__inline unsigned short float2le (double val)
{
  double v;
  int e;

  v = val;
  if (v < 1e-32) {
    return 0;
  }
  else {
    v = frexp(v,&e) * 256.0/v;
	return (unsigned char)(val * v)<<8 | (unsigned char)(e + 128);
  }
} 

__inline double le2float(unsigned short le)
{
  double f;
  unsigned char le1 = le >> 8;
  unsigned char le2 = le - (le1 << 8);

  if (le1) {   //nonzero pixel
    f = ldexp(1.0,le2-(int)(128+8));
	return le1 * f;
  }
  else
    return 0;
} 


int fputArrayLE (int nr, int x, int y, int z, double val)
{
    int pos;

    if (Array[nr]==NULL || nr<0 || nr>99 || ArrayBytes[nr] < 2) return 0;
    
    pos = (( (y+ArrayPadding[nr])*ArrayX[nr] + (x+ArrayPadding[nr]) )*ArrayZ[nr] + z)*ArrayBytes[nr];
    if (pos <0 || pos >= ArraySize[nr]) return 0;

	*(unsigned short *)(((unsigned8 *) Array[nr]) + pos ) = float2le (val);
	
    return 1;

}

double fgetArrayLE (int nr, int x, int y, int z)
{
    int pos;

    if (Array[nr]==NULL || nr<0 || nr>99 || z<0 || z >= ArrayZ[nr] || ArrayBytes[nr] < 2) return 0;

    pos = (( (y+ArrayPadding[nr])*ArrayX[nr] + (x+ArrayPadding[nr]) )*ArrayZ[nr] + z)*ArrayBytes[nr];
    if (pos <0 || pos >= ArraySize[nr]) return 0;

	return le2float ( *(unsigned short *)(((unsigned8 *)Array[nr]) + pos) );

}
*/



int putArrayString (int nr, int x, LPSTR string)
{

    UINT_PTR pos;

    if (nr<0 || nr>99 || Array[nr]==NULL) return false;
    
    pos = (x+ArrayPadding[nr])*ArrayY[nr];
    if (pos <0 || pos + (int)strlen(string) >= ArraySize[nr]) return false; //strlen+1 for NUL byte???-Alex

    strcpy ((char *)((unsigned8 *)Array[nr] + pos), string);

    return true;

}


char dummystr[] = "Not Available"; //Return this string to avoid memory access violations

char* getArrayString (int nr, int x)//, LPSTR string)
{

    UINT_PTR pos;

    if (nr<0 || nr>99 || Array[nr]==NULL) return (char*)(unsigned8 *)dummystr;
    
    pos = (x+ArrayPadding[nr])*ArrayY[nr];
    if (pos <0 || pos >= ArraySize[nr]) return (char*)(unsigned8 *)dummystr;

    //strcpy (string, (char *)((unsigned8 *)Array[nr] + pos));

    return (char*)((unsigned8 *)Array[nr] + pos);//true;

}




//function added by Harald Heim on Jun 4, 2002
unsigned8 gMskVal(int row, int rowOff, int col, int colOff)
{

    //This code produces a transparent-blank preview and final image when applying plugin to a layer
    /*if ((gNoMask || !(gParams->flags & AFH_USE_HOST_PADDING)) &&
        (row+rowOff < minRow || row+rowOff > maxRow ||
         col+colOff < minCol || col+colOff > maxCol) )
        return 0;   // Simulate the padding... 
    else */

    if (gNoMask)
        return 255; // Entire filterRect is inside the mask.
    else
        return fmc.mskPtr[rowOff*fmc.mskRowDelta + colOff*fmc.mskColDelta];
} /*gMskVal*/



#define maxBlurRadius 3
#define mskVal(row,rowOff,col,colOff) \
    gMskVal(row,rowOff,col,colOff)


// "Distance to mask" array of vectors.
#define DIST(i,j) dist[(i)*(fmc.columns+2*maxBlurRadius) + (j)]


#ifndef APP

//function added by Harald Heim on Jun 4, 2002
int calcSBD(int bevWidth)
{
    int i,j,i0,j0; 

#if BIGDOCUMENT
	VRect scaledFilterRect;
#else
    Rect scaledFilterRect;
#endif
    OSErr err = noErr;

    int32 distSize = (fmc.rows + 2*maxBlurRadius)*(fmc.columns + 2*maxBlurRadius)*sizeof(struct Vect_s);

    //Info("distSize: %d",distSize);

    //free buffer if necessary
    if (bufferID != 0)
    {
        UnlockBuffer(bufferID);
        FreeBuffer(bufferID);
        bufferID = 0;
    }

    // Allocate the dist array...
    err = AllocateBuffer(distSize, &bufferID);

    if (err != noErr) {
        // oops.  what now???
        gResult = err;
        return 0;
    }
    dist = (struct Vect_s *)LockBuffer(bufferID, FALSE);
    // Offset to account for non-0 lower bounds...
    dist += maxBlurRadius*(fmc.columns + 2*maxBlurRadius + 1);


#if BIGDOCUMENT
		scaledFilterRect = GetFilterRect();
		scaleRect32 (&scaledFilterRect, 1, fmc.scaleFactor);
#else
        copyRect (&scaledFilterRect, &gStuff->filterRect);
		scaleRect (&scaledFilterRect, 1, fmc.scaleFactor);