INT_PTR lockHost (INT_PTR bufferid)
{

    return (INT_PTR)LockBuffer((BufferID)bufferid,true);
}


int freeHost (INT_PTR bufferid)
{
    UnlockBuffer((BufferID)bufferid);
	FreeBuffer((BufferID)bufferid);

    return true;
}

#endif


//function added by Harald Heim on Apr 9, 2002
int sub_allocArray (int nr, int X, int Y, int Z, int bytes, int padding)
{

    //BufferProcs* bufferProcs = gStuff->bufferProcs;
    int size;

    if (nr<0 || nr>99) return 0; //Array 100 isn't allocatable

    if (bytes==1 || bytes==2 || bytes==4 || bytes==8) ArrayBytes[nr]=bytes;
    else return 0;

    if (X <= 0) return 0;
    ArrayX[nr]=X;
    if (Y <= 0) ArrayY[nr]=1; else ArrayY[nr]=Y;
    if (Z <= 0) ArrayZ[nr]=1; else ArrayZ[nr]=Z;

    ArrayX[nr] += 2*padding;
    ArrayY[nr] += 2*padding;    // ACK!!?? Could easily triple the size of a 1-dim array, or worse!!?? Harry:_ But who would use padding for a 1-dim array???
    ArrayPadding[nr] = padding;

    size = ArrayX[nr] * ArrayY[nr] * ArrayZ[nr] * ArrayBytes[nr];
    if (size == 0) return 0;

    //Avoid reallocation of the same size
    if (ArraySize[nr] == size && Array[nr]!=NULL) return 1;
	
	//Not necessary
	//Free array if it already exists and has a different size
	//if (ArraySize[nr] != size && Array[nr]!=NULL) freeArray(nr);

    //Info ("%d", bufferProcs->spaceProc() );


	if (arrayMode == 1){
			
		//Free previous buffer
#ifndef APP
        if (ArrayID[nr]!=0 || nr == PhotoPlusArray) { 
			UnlockBuffer(ArrayID[nr]); //gStuff->bufferProcs->unlockProc(ArrayID[nr]);
			FreeBuffer(ArrayID[nr]); //gStuff->bufferProcs->freeProc(ArrayID[nr]);
			if (nr == PhotoPlusArray) PhotoPlusArray = -1;
		} else if (Array[nr]!=NULL){

			free (Array[nr]);
		}
#else
        if (Array[nr]!=NULL) free (Array[nr]);
#endif
		
		ArrayID[nr] = 0;
		Array[nr] = 0;

#ifndef APP
		if (AllocateBuffer(size, &ArrayID[nr]) == noErr){ //if (gStuff->bufferProcs->allocateProc(size, &ArrayID[nr]) == noErr){
			Array[nr] = LockBuffer(ArrayID[nr], true);//Array[nr] = gStuff->bufferProcs->lockProc(ArrayID[nr], true);
			if (ArrayID[nr]==0) PhotoPlusArray=nr;
		} else {
#endif
			Array[nr] = realloc (Array[nr], (unsigned32) size); //Fall back to Windows API
                                                                //(realloc could be malloc, since Array[nr]==0 here?-AFH)
            ArrayID[nr] = 0; //for safety?-afh
#ifndef APP
		}
#endif

	}else {
        //but what if ArrayID[nr] != 0 ?? Need to free it??-AFH	
#ifndef APP
        if (ArrayID[nr]!=0) { 
			UnlockBuffer(ArrayID[nr]); 
			FreeBuffer(ArrayID[nr]);
            ArrayID[nr] = 0;
		} 
#endif
		Array[nr] = realloc (Array[nr], (unsigned32) size);
	}
    

    if (Array[nr]==NULL)
    {
        ArrayID[nr]=0;
		ArraySize[nr]=0;
        ArrayBytes[nr]=0;
        ArrayX[nr]=0;
        ArrayY[nr]=0;
        ArrayZ[nr]=0;
        ArrayPadding[nr]=0;

        ErrorOk ("Unable to allocate enough memory.");
        fm_abort();

        return 0;
    }
    else
    {   
        ArraySize[nr]=size;
        return 1;
    }

}


//function added by Harald Heim on Apr 9, 2002
int allocArray (int nr, int X, int Y, int Z, int bytes)
{
    
    return sub_allocArray (nr, X, Y, Z, bytes, 0);
    
}

//function added by Harald Heim on Oct 12, 2003
int allocArrayPad (int nr, int X, int Y, int Z, int bytes, int padding)
{

    return sub_allocArray (nr, X, Y, Z, bytes, padding);

}

int freeArray (int nr)
{

    if (nr>=0 && nr<=99 && Array[nr] != NULL)
    {

/*#ifdef IMGSOURCE
        if (nr==10){
            GlobalFree (Array[nr]);
        } else 
#endif*/
#ifndef APP
        if (ArrayID[nr]!=0 || nr == PhotoPlusArray){ //was allocated with Buffer Proc
			UnlockBuffer(ArrayID[nr]);//gStuff->bufferProcs->unlockProc(ArrayID[nr]);
			FreeBuffer(ArrayID[nr]);//gStuff->bufferProcs->freeProc(ArrayID[nr]); 
			if (nr == PhotoPlusArray) PhotoPlusArray = -1;
        } else {
#endif
			free (Array[nr]);       //check result for safety??
#ifndef APP
        }
#endif
        
        Array[nr] = NULL;
		ArrayID[nr]=0;
        ArraySize[nr]=0;
        ArrayBytes[nr]=0;
        ArrayX[nr]=0;
        ArrayY[nr]=0;
        ArrayZ[nr]=0;
        ArrayPadding[nr]=0;
		
        return 1;
    }

    return 0;

}

void swapArray(int nr, int nr2)
{
	void * pTemp;
	BufferID bTemp;
	INT_PTR ptrTemp;
	int temp;

	pTemp = Array[nr];
	Array[nr] = Array[nr2];
	Array[nr2] = pTemp;

	bTemp = ArrayID[nr];
	ArrayID[nr] = ArrayID[nr2];
	ArrayID[nr2] = bTemp;

	ptrTemp=ArraySize[nr];
	ArraySize[nr]=ArraySize[nr2];
	ArraySize[nr2]=ptrTemp;

    temp=ArrayBytes[nr];
	ArrayBytes[nr]=ArrayBytes[nr2];
	ArrayBytes[nr2]=temp;

	temp=ArrayX[nr];
	ArrayX[nr]=ArrayX[nr2];
	ArrayX[nr2]=temp;

	temp=ArrayY[nr];
	ArrayY[nr]=ArrayY[nr2];
	ArrayY[nr2]=temp;

	temp=ArrayZ[nr];
	ArrayZ[nr]=ArrayZ[nr2];
	ArrayZ[nr2]=temp;

	temp=ArrayPadding[nr];
	ArrayPadding[nr]=ArrayPadding[nr2];
	ArrayPadding[nr2]=temp;
}


#if HALFFLOAT

///////////////////////////////////////////////////////////////////////////
//
// Copyright (c) 2002, Industrial Light & Magic, a division of Lucas
// Digital Ltd. LLC
// 
// All rights reserved.
// 
// Redistribution and use in source and binary forms, with or without
// modification, are permitted provided that the following conditions are
// met:
// *       Redistributions of source code must retain the above copyright
// notice, this list of conditions and the following disclaimer.
// *       Redistributions in binary form must reproduce the above
// copyright notice, this list of conditions and the following disclaimer
// in the documentation and/or other materials provided with the
// distribution.
// *       Neither the name of Industrial Light & Magic nor the names of
// its contributors may be used to endorse or promote products derived
// from this software without specific prior written permission. 
// 
// THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS
// "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT
// LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR
// A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT
// OWNER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL,
// SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT
// LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE,
// DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY
// THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
// (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE
// OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
//
///////////////////////////////////////////////////////////////////////////


__inline unsigned short floatToHalf (unsigned int i)
{
    // Our floating point number, f, is represented by the bit
    // pattern in integer i.  Disassemble that bit pattern into
    // the sign, s, the exponent, e, and the significand, m.
    // Shift s into the position where it will go in in the
    // resulting half number.
    // Adjust e, accounting for the different exponent bias
    // of float and half (127 versus 15).

    register int s =  (i >> 16) & 0x00008000;
    register int e = ((i >> 23) & 0x000000ff) - (127 - 15);
    register int m =   i        & 0x007fffff;

    // Now reassemble s, e and m into a half:
    if (e <= 0){
		if (e < -10){
			// E is less than -10.  The absolute value of f is
			// less than HALF_MIN (f may be a small normalized
			// float, a denormalized float or a zero).
			//
			// We convert f to a half zero.

			return 0;
		}

		// E is between -10 and 0.  F is a normalized float,
		// whose magnitude is less than HALF_NRM_MIN.
		//
		// We convert f to a denormalized half. 

		m = (m | 0x00800000) >> (1 - e);

		// Round to nearest, round "0.5" up.
		//
		// Rounding may cause the significand to overflow and make
		// our number normalized.  Because of the way a half's bits
		// are laid out, we don't have to treat this case separately;
		// the code below will handle it correctly.

		if (m &  0x00001000)
			m += 0x00002000;

		// Assemble the half from s, e (zero) and m.

		return s | (m >> 13);