
      if (fmc.doingProxy && gStuff->haveMask){ //Simulate final result in preview
            
            int MaskValue = msk(x,y); //src(x,y,-1);
            
            if (MaskValue>0){
                if (MaskValue<65535){
                    val = (val*MaskValue/2 + src16(x,y,z)*(32768-MaskValue/2) )/32768; //32768
                }
                    
                *(unsigned16 *)(((unsigned8 *)gStuff->outData) +
                        (((x-fmc.x_start)*fmc.dstColDelta + z) << 1) +
                        (y-fmc.y_start)*fmc.dstRowDelta)
                    = val;

            } 

        } else { //regular pset

                *(unsigned16 *)(((unsigned8 *)gStuff->outData) +
                        (((x-fmc.x_start)*fmc.dstColDelta + z) << 1) +
                        (y-fmc.y_start)*fmc.dstRowDelta)
                = val;
        }

#else
*/        
            *(unsigned16 *)(((unsigned8 *)gStuff->outData) +
                        (((x-fmc.x_start)*fmc.dstColDelta + z) << 1) +
                        (y-fmc.y_start)*fmc.dstRowDelta)
                = val;
//#endif


    }
    return val; //clamped value
} /*fm_pset16*/


void fast_pset(int x, int y, int z, int val)
{     
    if (x >= fmc.x_start && x < fmc.x_end && y >= fmc.y_start && y < fmc.y_end && z >= 0 && z < fmc.dstColDelta ){
        
        if (fmc.imageMode<10){
            if ((unsigned)val > 255) val = (val <= 0) ? 0 : 255;
            ((unsigned8 *)gStuff->outData)[(x-fmc.x_start)*fmc.dstColDelta + (y-fmc.y_start)*fmc.dstRowDelta + z] = val;

        } else {
/*
            #ifndef _WIN64
                if (fmc.hostSig != 0x50535039){ //if not PSP X3
            #endif
                    if ((unsigned)val > 0x8000) val = (val <= 0) ? 0 : 0x8000;
            #ifndef _WIN64
                } else { //PSP X3
                    if ((unsigned)val > 32767) val = (val <= 0) ? 0 : 32767;
                }
            #endif
*/
            *(unsigned16 *)(((unsigned8 *)gStuff->outData) + (((x-fmc.x_start)*fmc.dstColDelta + z) << 1) + (y-fmc.y_start)*fmc.dstRowDelta) = val;
            //((unsigned16 *)gStuff->outData)[(x-fmc.x_start)*fmc.dstColDelta + (y-fmc.y_start)*fmc.dstRowDelta + z] = val;
        }
    }
}


int tset(int x, int y, int z, int val)
{
#if 1
    if (!fmc.tbuf) {
        // tbuf wasn't allocated????
        //ErrorOk("No tile buffer!");
        return 0;
    }
#endif
    // Store channel value in (temporary) tile buffer.
    // Clamp value to [0,255].
#if 0
    if (val > 255) val = 255;
    else if (val < 0) val = 0;
#elif 0 //using unsigned should be faster...
    val = ((unsigned)val <= 255) ? val : (val <= 0) ? 0 : 255;
#else //or is this faster??
    if ((unsigned)val > 255) {
        val = (val <= 0) ? 0 : 255;
    }//needs to be clamped
#endif
#if 0
    //Min version (tile buffer is size of output tile)
    if (x >= fmc.x_start && x < fmc.x_end &&
        y >= fmc.y_start && y < fmc.y_end &&
        z >= 0 && z < fmc.Z )
    {   //valid coordinate.
        //(optimize this later, after it's been debugged!!!)
        fmc.tbuf[((y-fmc.y_start)*fmc.columns + (x-fmc.x_start))*fmc.Z + z] = val;
    }
#else
    //Max version (tile buffer is size of input tile)
    if (x >= 0 && x < fmc.X &&
        y >= 0 && y < fmc.Y &&
        z >= 0 && z < fmc.Z )
    {   //valid coordinate.
        //(optimize this later, after it's been debugged!!!)
        fmc.tbuf[(y*fmc.X + x)*fmc.Z + z] = val;
    }
#endif
    return val; //clamped value
} /*fm_tset*/

int tset16(int x, int y, int z, int val)
{
#if 1
    if (!fmc.tbuf) {
        // tbuf wasn't allocated????
        //ErrorOk("No tile buffer!");
        return 0;
    }
#endif
    // Store channel value in (temporary) tile buffer.
    // Clamp value to [0,255].
    if (val > 32768) val = 32768;
    else if (val < 0) val = 0;
#if 0
    //Min version (tile buffer is size of output tile)
    if (x >= fmc.x_start && x < fmc.x_end &&
        y >= fmc.y_start && y < fmc.y_end &&
        z >= 0 && z < fmc.Z )
    {   //valid coordinate.
        //(optimize this later, after it's been debugged!!!)
        fmc.tbuf[((y-fmc.y_start)*fmc.columns + (x-fmc.x_start))*fmc.Z + z] = val;
    }
#else
    //Max version (tile buffer is size of input tile)
    if (x >= 0 && x < fmc.X &&
        y >= 0 && y < fmc.Y &&
        z >= 0 && z < fmc.Z )
    {   //valid coordinate.
        //(optimize this later, after it's been debugged!!!)
        *(unsigned16 *)(((unsigned8 *)fmc.tbuf) + 2*((y*fmc.X + x)*fmc.Z + z)) = val;
    }
#endif
    return val; //clamped value
} /*fm_tset16*/

int t2set(int x, int y, int z, int val)
{
#if 1
    if (!fmc.t2buf) {
        // t2buf wasn't allocated????
        //ErrorOk("No tile buffer 2!");
        return 0;
    }
#endif
    // Store channel value in (temporary) tile buffer 2.
    // Clamp value to [0,255].
#if 0
    if (val > 255) val = 255;
    else if (val < 0) val = 0;
#elif 0 //using unsigned should be faster...
    val = ((unsigned)val <= 255) ? val : (val <= 0) ? 0 : 255;
#else //or is this faster??
    if ((unsigned)val > 255) {
        val = (val <= 0) ? 0 : 255;
    }//needs to be clamped
#endif
#if 0
    //Min version (tile buffer is size of output tile)
    if (x >= fmc.x_start && x < fmc.x_end &&
        y >= fmc.y_start && y < fmc.y_end &&
        z >= 0 && z < fmc.Z )
    {   //valid coordinate.
        //(optimize this later, after it's been debugged!!!)
        fmc.t2buf[((y-fmc.y_start)*fmc.columns + (x-fmc.x_start))*fmc.Z + z] = val;
    }
#else
    //Max version (tile buffer is size of input tile)
    if (x >= 0 && x < fmc.X &&
        y >= 0 && y < fmc.Y &&
        z >= 0 && z < fmc.Z )
    {   //valid coordinate.
        //(optimize this later, after it's been debugged!!!)
        fmc.t2buf[(y*fmc.X + x)*fmc.Z + z] = val;
    }
#endif
    return val; //clamped value
} /*fm_t2set*/

int t2set16(int x, int y, int z, int val)
{
#if 1
    if (!fmc.t2buf) {
        // t2buf wasn't allocated????
        //ErrorOk("No tile buffer 2!");
        return 0;
    }
#endif
    // Store channel value in (temporary) tile buffer 2.
    // Clamp value to [0,255].
    if (val > 32768) val = 32768;
    else if (val < 0) val = 0;
#if 0
    //Min version (tile buffer is size of output tile)
    if (x >= fmc.x_start && x < fmc.x_end &&
        y >= fmc.y_start && y < fmc.y_end &&
        z >= 0 && z < fmc.Z )
    {   //valid coordinate.
        //(optimize this later, after it's been debugged!!!)
        fmc.t2buf[((y-fmc.y_start)*fmc.columns + (x-fmc.x_start))*fmc.Z + z] = val;
    }
#else
    //Max version (tile buffer is size of input tile)
    if (x >= 0 && x < fmc.X &&
        y >= 0 && y < fmc.Y &&
        z >= 0 && z < fmc.Z )
    {   //valid coordinate.
        //(optimize this later, after it's been debugged!!!)
        *(unsigned16 *)(((unsigned8 *)fmc.t2buf) + 2*((y*fmc.X + x)*fmc.Z + z)) = val;
    }
#endif
    return val; //clamped value
} /*fm_t2set16*/

int t3set(int x, int y, int z, int val)
{
#if 1
    if (!fmc.t3buf) {
        // t3buf wasn't allocated????
        //ErrorOk("No tile buffer 3!");
        return 0;
    }
#endif
    // Store channel value in (temporary) tile buffer 2.
    // Clamp value to [0,255].
    if (val > 255) val = 255;
    else if (val < 0) val = 0;
#if 0
    //Min version (tile buffer is size of output tile)
    if (x >= fmc.x_start && x < fmc.x_end &&
        y >= fmc.y_start && y < fmc.y_end &&
        z >= 0 && z < fmc.Z )
    {   //valid coordinate.
        //(optimize this later, after it's been debugged!!!)
        fmc.t3buf[((y-fmc.y_start)*fmc.columns + (x-fmc.x_start))*fmc.Z + z] = val;
    }
#else
    //Max version (tile buffer is size of input tile)
    if (x >= 0 && x < fmc.X &&
        y >= 0 && y < fmc.Y &&
        z >= 0 && z < fmc.Z )
    {   //valid coordinate.
        //(optimize this later, after it's been debugged!!!)
        fmc.t3buf[(y*fmc.X + x)*fmc.Z + z] = val;
    }
#endif
    return val; //clamped value
} /*fm_t3set*/

int t3set16(int x, int y, int z, int val)
{
#if 1
    if (!fmc.t3buf) {
        // t3buf wasn't allocated????
        //ErrorOk("No tile buffer 3!");
        return 0;
    }
#endif
    // Store channel value in (temporary) tile buffer 2.
    // Clamp value to [0,255].
    if (val > 32768) val = 32768;
    else if (val < 0) val = 0;
#if 0
    //Min version (tile buffer is size of output tile)
    if (x >= fmc.x_start && x < fmc.x_end &&
        y >= fmc.y_start && y < fmc.y_end &&
        z >= 0 && z < fmc.Z )
    {   //valid coordinate.
        //(optimize this later, after it's been debugged!!!)
        fmc.t3buf[((y-fmc.y_start)*fmc.columns + (x-fmc.x_start))*fmc.Z + z] = val;
    }
#else
    //Max version (tile buffer is size of input tile)
    if (x >= 0 && x < fmc.X &&
        y >= 0 && y < fmc.Y &&
        z >= 0 && z < fmc.Z )
    {   //valid coordinate.
        //(optimize this later, after it's been debugged!!!)
        *(unsigned16 *)(((unsigned8 *)fmc.t3buf) + 2*((y*fmc.X + x)*fmc.Z + z)) = val;
    }
#endif
    return val; //clamped value
} /*fm_t3set16*/

int t4set(int x, int y, int z, int val)
{
#if 1
    if (!fmc.t4buf) {
        // t4buf wasn't allocated????
        //ErrorOk("No tile buffer 4!");
        return 0;
    }