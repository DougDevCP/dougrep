
    } else if (e == 0xff - (127 - 15)) {
		if (m == 0){
			// F is an infinity; convert f to a half
			// infinity with the same sign as f.
			return s | 0x7c00;

		} else {
			// F is a NAN; we produce a half NAN that preserves
			// the sign bit and the 10 leftmost bits of the
			// significand of f, with one exception: If the 10
			// leftmost bits are all zero, the NAN would turn 
			// into an infinity, so we have to set at least one
			// bit in the significand.
			
			m >>= 13;
			return s | 0x7c00 | m | (m == 0);
		}

    } else {

		// E is greater than zero.  F is a normalized float.
		// We try to convert f to a normalized half.
		
		// Round to nearest, round "0.5" up
		
		if (m &  0x00001000) {

			m += 0x00002000;

			if (m & 0x00800000) {
				m =  0;		// overflow in significand,
				e += 1;		// adjust exponent
			}
		}

		// Handle exponent overflow

		if (e > 30) {
			// overflow ();	// Cause a hardware floating point overflow;
			return s | 0x7c00;	// if this returns, the half becomes an
		}   			// infinity with the same sign as f.

		// Assemble the half from s, e and m.
		return s | (e << 10) | (m >> 13);
    }
} 

__inline unsigned int halfToFloat (unsigned short y)
{

    int s = (y >> 15) & 0x00000001;
    int e = (y >> 10) & 0x0000001f;
    int m =  y        & 0x000003ff;

    if (e == 0) {
		if (m == 0) {
			// Plus or minus zero
			return s << 31;
		} else {
			// Denormalized number -- renormalize it
			while (!(m & 0x00000400)){
				m <<= 1;
				e -=  1;
			}
			e += 1;
			m &= ~0x00000400;
		}
    } else if (e == 31) {
		if (m == 0){
			// Positive or negative infinity
			return (s << 31) | 0x7f800000;
		} else {
			// Nan -- preserve sign and significand bits
			return (s << 31) | 0x7f800000 | (m << 13);
		}
    }

    // Normalized number
    e = e + (127 - 15);
    m = m << 13;

    // Assemble s, e and m.
    return (s << 31) | (e << 23) | m;
}

#endif


int getArray (int nr, int x, int y, int z)
{

    int pos;

    if (Array[nr]==NULL || nr<0 || nr>99 || z<0 || z >= ArrayZ[nr] || ArrayBytes[nr] == 8) return 0;

    // No check on x and y within bounds???? -- allows wrapping of bounds???
    // Harry: Isn't that done by the edgeMode stuff???
        

#ifndef HYPERTYLE
//#if !defined(HYPERTYLE) && !defined(PLUGINGALAXY)
    if (x < 0-ArrayPadding[nr]) x = 0- ArrayPadding[nr];
    else if (x >= ArrayX[nr]-ArrayPadding[nr]) x = ArrayX[nr] - 1-ArrayPadding[nr];
    if (y < 0-ArrayPadding[nr]) y = 0-ArrayPadding[nr];
    else if (y >= ArrayY[nr]-ArrayPadding[nr]) y = ArrayY[nr] - 1-ArrayPadding[nr];
#else
    switch (edgeMode)
        {
            case 0:
                // Replicate pixels at edge
                if (x < 0-ArrayPadding[nr]) x = 0- ArrayPadding[nr];
                else if (x >= ArrayX[nr]-ArrayPadding[nr]) x = ArrayX[nr] - 1-ArrayPadding[nr];
                if (y < 0-ArrayPadding[nr]) y = 0-ArrayPadding[nr];
                else if (y >= ArrayY[nr]-ArrayPadding[nr]) y = ArrayY[nr] - 1-ArrayPadding[nr];
                break;

            case 1:
                // Black (zero) padding
                if (x < 0-ArrayPadding[nr]|| y < 0-ArrayPadding[nr]|| x >= ArrayX[nr]-ArrayPadding[nr]|| y >= ArrayY[nr]-ArrayPadding[nr])
                    return 0;
				//break;			//needed, or is fall-thru intentional here??
        
            case 2:
                // Wrap around
                if (x < 0) x = ArrayX[nr]-ArrayPadding[nr] + x;   /// doesn't work for x < -2X ?? use MOD ???
                else if (x >= ArrayX[nr]-ArrayPadding[nr]) x = x - ArrayX[nr]-ArrayPadding[nr];
                if (y < 0) y = ArrayY[nr]-ArrayPadding[nr]  + y;
                else if (y >= ArrayY[nr]-ArrayPadding[nr]) y = y - ArrayY[nr]-ArrayPadding[nr];
                break;
            
            case 3: 
                // Two possible reflection modes, one of period 2N, other of period 2N-2 (as here)?
                if (x < 0) x = - x;
                else if (x >= ArrayX[nr]-ArrayPadding[nr]) x = (ArrayX[nr] - 1-ArrayPadding[nr]) - (x - (ArrayX[nr] - 1-ArrayPadding[nr]));       //optimize?
                if (y < 0) y = - y;
                else if (y >= ArrayY[nr]-ArrayPadding[nr]) y = (ArrayY[nr] - 1-ArrayPadding[nr]) - (y - (ArrayY[nr] - 1-ArrayPadding[nr]));
				break;
        }//end switch
#endif

    ///pos = y*ArrayX[nr]*ArrayZ[nr]*ArrayBytes[nr] + x*ArrayZ[nr]*ArrayBytes[nr] + z*ArrayBytes[nr];
    pos = (( (y+ArrayPadding[nr])*ArrayX[nr] + (x+ArrayPadding[nr]) )*ArrayZ[nr] + z);//*ArrayBytes[nr];

    //if (pos <0 || pos >= ArraySize[nr]) return 0; // not necessary anymore because of edge mode **WRONG** unless mod used!!!


    /*if (ArrayBytes[nr] == 1)
        return ((unsigned8 *)Array[nr]) [pos];
    else if (ArrayBytes[nr] == 2)
        return  *(unsigned16 *)(((unsigned8 *)Array[nr]) + pos);
    else if (ArrayBytes[nr] == 4)
        return  *(unsigned32 *)(((unsigned8 *)Array[nr]) + pos);*/
    
    switch(ArrayBytes[nr]) {
		case 1:
	        return ((unsigned8 *)Array[nr]) [pos];
		case 2:
            return  ((unsigned16 *)Array[nr]) [pos];
	        //return  *(unsigned16 *)(((unsigned8 *)Array[nr]) + pos);
		case 4:
            return  ((unsigned32 *)Array[nr]) [pos];
	        //return  *(unsigned32 *)(((unsigned8 *)Array[nr]) + pos);
	}

    return 0; // or better return -1 to indicate something really wrong???

}

int fast_getArray (int nr, int x, int y, int z)
{
    int pos;

    if (x < 0) x = 0; else if (x >= ArrayX[nr]) x = ArrayX[nr] - 1;
    if (y < 0) y = 0; else if (y >= ArrayY[nr]) y = ArrayY[nr] - 1;

    pos = (( y*ArrayX[nr] + x )*ArrayZ[nr] + z);//*ArrayBytes[nr];
    
	switch(ArrayBytes[nr]) {
		case 1:
	        return ((unsigned8 *)Array[nr]) [pos];
		case 2:
            return  ((unsigned16 *)Array[nr]) [pos];
	        //return  *(unsigned16 *)(((unsigned8 *)Array[nr]) + pos);
		case 4:
            return  ((unsigned32 *)Array[nr]) [pos];
	        //return  *(unsigned32 *)(((unsigned8 *)Array[nr]) + pos);
	}

	return 0;
}

//int UNSAFE_get_pos[100];

int UNSAFE_getArray (int nr, int x, int y, int z)
{
    const int pos = (( (y+ArrayPadding[nr])*ArrayX[nr] + (x+ArrayPadding[nr]) )*ArrayZ[nr] + z)*ArrayBytes[nr];
    //UNSAFE_get_pos[nr] = (( (y+ArrayPadding[nr])*ArrayX[nr] + (x+ArrayPadding[nr]) )*ArrayZ[nr] + z)*ArrayBytes[nr];

	switch(ArrayBytes[nr]) {
		case 1:
	        return ((unsigned8 *)Array[nr]) [pos];
		case 2:
	        return  *(unsigned16 *)(((unsigned8 *)Array[nr]) + pos);
		case 4:
	        return  *(unsigned32 *)(((unsigned8 *)Array[nr]) + pos);
	}

	return 0;
}

/*
int UNSAFE_getArrayNext8 (int nr)
{  
    return ((unsigned8 *)Array[nr]) [UNSAFE_get_pos[nr]++];
}
int UNSAFE_getArrayNext16 (int nr)
{
    UNSAFE_get_pos[nr] += 2;
    return  *(unsigned16 *)(((unsigned8 *)Array[nr]) + UNSAFE_get_pos[nr]);
}
int UNSAFE_getArrayNext32 (int nr)
{  
    UNSAFE_get_pos[nr] += 4;
    return  *(unsigned32 *)(((unsigned8 *)Array[nr]) + UNSAFE_get_pos[nr]);
}
*/


double fgetArray (int nr, int x, int y, int z)
{

    UINT_PTR pos;

    if (Array[nr]==NULL || nr<0 || nr>99 || z<0 || z >= ArrayZ[nr] || ArrayBytes[nr] < 2) return 0.0;

    // No edge effects as for getArray above???


    pos = (( (y+ArrayPadding[nr])*ArrayX[nr] + (x+ArrayPadding[nr]) )*ArrayZ[nr] + z)*ArrayBytes[nr];
    if (pos <0 || pos >= ArraySize[nr]) return 0.0;

#if HALFFLOAT
	if (ArrayBytes[nr] == 2){
		UIF fval;
		fval.i = halfToFloat( *(unsigned short *)(((unsigned8 *)Array[nr]) + pos) );
		return (double)fval.f;

    } else 
#endif		
	if (ArrayBytes[nr] == 4)
		return *(float *)(((unsigned8 *)Array[nr]) + pos);
    else if (ArrayBytes[nr] == 8){
        return *(double *)(((unsigned8 *)Array[nr]) + pos);
    } else
        return 0.0; // or return some other value such as Indefinite Nan???
}


double UNSAFE_fgetArray (int nr, int x, int y, int z)
{
    const int pos = (((y + ArrayPadding[nr]) * ArrayX[nr] + (x + ArrayPadding[nr])) * ArrayZ[nr] + z) * ArrayBytes[nr];

	switch(ArrayBytes[nr]) {
		case 4:
			return *(float *)(((unsigned8 *)Array[nr]) + pos);
		case 8:
			return *(double *)(((unsigned8 *)Array[nr]) + pos);
#if HALFFLOAT
		case 2: {
			UIF fval;
			fval.i = halfToFloat( *(unsigned short *)(((unsigned8 *)Array[nr]) + pos) );
			return (double)fval.f;
		}
#endif
	}

    return 0.0;
}


//function added by Harald Heim on Apr 9, 2002
int putArray (int nr, int x, int y, int z, int val)
{

    UINT_PTR pos;

    if (Array[nr]==NULL || nr<0 || nr>99 || ArrayBytes[nr] == 8) return 0;
    //if ( x<0 || y<0 || z<0 || x >= ArrayX[nr] || y >= ArrayY[nr] || z >= ArrayZ[nr]  ) return 0;
    
    // clamp val to correct range...
    if (ArrayBytes[nr]==1){
        if (val < 0) val = 0;
        else if (val > 255 ) val = 255;
    } else if (ArrayBytes[nr]==2){
        if (val < 0) val = 0;
        else if (val > 65535 ) val = 65535;
    }

    //pos = y*ArrayX[nr]*ArrayZ[nr]*ArrayBytes[nr] + x*ArrayZ[nr]*ArrayBytes[nr] + z*ArrayBytes[nr];