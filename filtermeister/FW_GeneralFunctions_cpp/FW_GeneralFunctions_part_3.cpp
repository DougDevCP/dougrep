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
        fmc.t4buf[((y-fmc.y_start)*fmc.columns + (x-fmc.x_start))*fmc.Z + z] = val;
    }
#else
    //Max version (tile buffer is size of input tile)
    if (x >= 0 && x < fmc.X &&
        y >= 0 && y < fmc.Y &&
        z >= 0 && z < fmc.Z )
    {   //valid coordinate.
        //(optimize this later, after it's been debugged!!!)
        fmc.t4buf[(y*fmc.X + x)*fmc.Z + z] = val;
    }
#endif
    return val; //clamped value
} /*fm_t4set*/

int t4set16(int x, int y, int z, int val)
{
#if 1
    if (!fmc.t4buf) {
        // t4buf wasn't allocated????
        //ErrorOk("No tile buffer 4!");
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
        fmc.t4buf[((y-fmc.y_start)*fmc.columns + (x-fmc.x_start))*fmc.Z + z] = val;
    }
#else
    //Max version (tile buffer is size of input tile)
    if (x >= 0 && x < fmc.X &&
        y >= 0 && y < fmc.Y &&
        z >= 0 && z < fmc.Z )
    {   //valid coordinate.
        //(optimize this later, after it's been debugged!!!)
        *(unsigned16 *)(((unsigned8 *)fmc.t4buf) + 2*((y*fmc.X + x)*fmc.Z + z)) = val;
    }
#endif
    return val; //clamped value
} /*fm_t4set16*/

int pget(const int x0, const int y0, const int z0)
// Must not modify its input parameters due to cnvX/Y in-line optimizations!!!
{
    register int x = x0, y = y0, z = z0;
    if (z >= 0 && z < fmc.dstColDelta)
    {   // get an rgba pixel
        // pin the coordinates (i.e., replicate edges)...
        // Should replicate at IMAGE edge, not TILE edge???
        if (x < fmc.x_start) x = fmc.x_start;
        else if (x >= fmc.x_end) x = fmc.x_end - 1;
        if (y < fmc.y_start) y = fmc.y_start;
        else if (y >= fmc.y_end) y = fmc.y_end - 1;
        //(optimize this later, after it's been debugged!!!)
        return ((unsigned8 *)gStuff->outData)
            [(x-fmc.x_start)*fmc.dstColDelta + (y-fmc.y_start)*fmc.dstRowDelta + z];
    }
    else {
        // Bad channel number???
        return 0;
    }
} /*fm_pget*/


//Added by Harald Heim Feb 9, 2002
int pget16(const int x0, const int y0, const int z0)
// Must not modify its input parameters due to cnvX/Y in-line optimizations!!!
{
    register int x = x0, y = y0, z = z0;

    if (z >= 0 && z < fmc.dstColDelta)
    {   // get an rgba pixel
        // pin the coordinates (i.e., replicate edges)...
        // Should replicate at IMAGE edge, not TILE edge???
        if (x < fmc.x_start) x = fmc.x_start;
        else if (x >= fmc.x_end) x = fmc.x_end - 1;
        if (y < fmc.y_start) y = fmc.y_start;
        else if (y >= fmc.y_end) y = fmc.y_end - 1;
        //(optimize this later, after it's been debugged!!!)
        return *(unsigned16 *)  ( ((unsigned8 *)gStuff->outData) +
            (((x-fmc.x_start)*fmc.dstColDelta + z)<<1 ) + (y-fmc.y_start)*fmc.dstRowDelta  );
    }
    else {
        // Bad channel number???
        return 0;
    }
} /*fm_pget16*/


int tget(const int x0, const int y0, const int z0)
// Must not modify its input parameters due to cnvX/Y in-line optimizations!!!
{
    register int x = x0, y = y0, z = z0;
#if 1
    if (!fmc.tbuf) {
        // tbuf wasn't allocated????
        //ErrorOk("No tile buffer!");
        return 0;
    }
#endif
    // Get channel value from (temporary) tile buffer.
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
        return fmc.tbuf[((y-fmc.y_start)*fmc.columns + (x-fmc.x_start))*fmc.Z + z];
#else
        //Max version (tile buffer is size of input tile)
        if (x < 0) x = 0;
        else if (x >= fmc.X) x = fmc.X - 1;
        if (y < 0) y = 0;
        else if (y >= fmc.Y) y = fmc.Y - 1;
        //(optimize this later, after it's been debugged!!!)
        return fmc.tbuf[(y*fmc.X + x)*fmc.Z + z];
#endif
    }
    else {
        // Bad channel number???
        return 0;
    }
} /*fm_tget*/


int tget16(const int x0, const int y0, const int z0)
// Must not modify its input parameters due to cnvX/Y in-line optimizations!!!
{
    register int x = x0, y = y0, z = z0;
#if 1
    if (!fmc.tbuf) {
        // tbuf wasn't allocated????
        //ErrorOk("No tile buffer!");
        return 0;
    }
#endif
    // Get channel value from (temporary) tile buffer.
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
        return fmc.tbuf[((y-fmc.y_start)*fmc.columns + (x-fmc.x_start))*fmc.Z + z];
#else
        //Max version (tile buffer is size of input tile)
        if (x < 0) x = 0;
        else if (x >= fmc.X) x = fmc.X - 1;
        if (y < 0) y = 0;
        else if (y >= fmc.Y) y = fmc.Y - 1;
        //(optimize this later, after it's been debugged!!!)
        return *(unsigned16 *)(((unsigned8 *)fmc.tbuf) + 2*((y*fmc.X + x)*fmc.Z + z));

#endif
    }
    else {
        // Bad channel number???
        return 0;
    }
} /*fm_tget16*/


int t2get(const int x0, const int y0, const int z0)
// Must not modify its input parameters due to cnvX/Y in-line optimizations!!!
{
    register int x = x0, y = y0, z = z0;
#if 1
    if (!fmc.t2buf) {
        // t2buf wasn't allocated????
        //ErrorOk("No tile buffer 2!");
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
        return fmc.t2buf[((y-fmc.y_start)*fmc.columns + (x-fmc.x_start))*fmc.Z + z];
#else
        //Max version (tile buffer is size of input tile)
        if (x < 0) x = 0;
        else if (x >= fmc.X) x = fmc.X - 1;
        if (y < 0) y = 0;
        else if (y >= fmc.Y) y = fmc.Y - 1;
        //(optimize this later, after it's been debugged!!!)
        return fmc.t2buf[(y*fmc.X + x)*fmc.Z + z];
#endif
    }
    else {
        // Bad channel number???
        return 0;
    }
} /*fm_t2get*/

int t2get16(const int x0, const int y0, const int z0)
// Must not modify its input parameters due to cnvX/Y in-line optimizations!!!
{
    register int x = x0, y = y0, z = z0;
#if 1
    if (!fmc.t2buf) {
        // t2buf wasn't allocated????
        //ErrorOk("No tile buffer 2!");
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
        return fmc.t2buf[((y-fmc.y_start)*fmc.columns + (x-fmc.x_start))*fmc.Z + z];
#else
        //Max version (tile buffer is size of input tile)
        if (x < 0) x = 0;
        else if (x >= fmc.X) x = fmc.X - 1;
        if (y < 0) y = 0;
        else if (y >= fmc.Y) y = fmc.Y - 1;
        //(optimize this later, after it's been debugged!!!)
		return *(unsigned16 *)(((unsigned8 *)fmc.t2buf) + 2*((y*fmc.X + x)*fmc.Z + z));
#endif
    }
    else {
        // Bad channel number???
        return 0;
    }
} /*fm_t2get16*/

int t3get(const int x0, const int y0, const int z0)
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