        return y - (225*pb + 500*pr)/1000; 
    else if (z == 2)
        return y + 1827*pb/1000;
    else
        return 0;
}


//// Optimizations: Pluggable variants based on Z, bit depth, etc.?
////                Use aligned 1 byte/2 byte or 2 byte/1 byte accesses?
////                Factor out subscript expression if VC doesn't.

//// How about  srcRGB(), srcRGBA(), srcGrey(), srcDuo(), srcCMYK(), etc.
////         or src3(), src4(), src1(), src2(), src4(), ...
//// Ditto for  pgetRGB(), pgetRGBA(), etc.?

//Added by Harald Heim, January 2003

int srcp(int x, int y)
//int srcp(UINT_PTR x, UINT_PTR y)
{   
    if (x < 0) x = 0;
    else if (x >= fmc.X) x = fmc.X - 1;
    if (y < 0) y = 0;
    else if (y >= fmc.Y) y = fmc.Y - 1;


    if ((x == fmc.X-1 && y == fmc.Y-1) || (fmc.isTileable && x == fmc.x_end-1 && y == fmc.y_end-1)){ //Second condition to avoid problems in PSP8

        int r,g,b,a;

        UINT_PTR pos = (x-fmc.x_start)*fmc.srcColDelta + (y-fmc.y_start)*fmc.srcRowDelta;

        r = ((unsigned8 *)MyAdjustedInData) [pos];
        g = ((unsigned8 *)MyAdjustedInData) [pos + 1];
        b = ((unsigned8 *)MyAdjustedInData) [pos + 2];

        if (fmc.Z>3){
            a = ((unsigned8 *)MyAdjustedInData) [pos + 3];
        } else 
            a = 255;

        return (((a & 0xff) << 8 | (b & 0xff)) << 8 | (g & 0xff)) << 8 | (r & 0xff);
    

    } else { //if (x < fmc.X-1  || y < fmc.Y-1){

        return *(unsigned32 *)(((unsigned8 *)MyAdjustedInData) +
                               (x-fmc.x_start)*fmc.srcColDelta +
                               (y-fmc.y_start)*fmc.srcRowDelta   );
    }


}


int pgetp(int x, int y)
{   

    if (x < fmc.x_start) x = fmc.x_start;
    else if (x >= fmc.x_end) x = fmc.x_end - 1;
    if (y < fmc.y_start) y = fmc.y_start;
    else if (y >= fmc.y_end) y = fmc.y_end - 1;

    
    if ((x == fmc.X-1 && y == fmc.Y-1) || (fmc.isTileable && x == fmc.x_end-1 && y == fmc.y_end-1)){ //Second condition to avoid problems in PSP8
    
        int r,g,b,a;
        
        int pos = (x-fmc.x_start)*fmc.dstColDelta + (y-fmc.y_start)*fmc.dstRowDelta;

        r = ((unsigned8 *)gStuff->outData) [pos];
        g = ((unsigned8 *)gStuff->outData) [pos + 1];
        b = ((unsigned8 *)gStuff->outData) [pos + 2];

        if (fmc.Z>3){
            a = ((unsigned8 *)gStuff->outData) [pos + 3];
        } else 
            a = 255;

        return (((a & 0xff) << 8 | (b & 0xff)) << 8 | (g & 0xff)) << 8 | (r & 0xff);


    } else { //if (x < fmc.X  && y < fmc.Y-1){
        
        return  *(unsigned32 *)(((unsigned8 *)gStuff->outData) +
                                (x-fmc.x_start)*fmc.dstColDelta + 
                                (y-fmc.y_start)*fmc.dstRowDelta  );                 
    }


}



int tgetp(int x, int y)
{   

    //int r,g,b;

    //Added by Harald Heim, April 20, 2003
    if (x < 0) x = 0;
    else if (x >= fmc.X) x = fmc.X - 1;
    if (y < 0) y = 0;
    else if (y >= fmc.Y) y = fmc.Y - 1;


    return *(unsigned32 *) (fmc.tbuf + (y*fmc.X + x)*fmc.Z );


    /*r= fmc.tbuf[(y*fmc.X + x)*fmc.Z + 0];
    g= fmc.tbuf[(y*fmc.X + x)*fmc.Z + 1];
    b= fmc.tbuf[(y*fmc.X + x)*fmc.Z + 2];

    return ((b & 0xff) << 8 | (g & 0xff)) << 8 | (r & 0xff);*/
    

}

//// Overlaps when Z<4???

//// Use "pluggable" versions of tsetp: tsetp4 when Z >=4;
///  tsetp1, tsetp2, tsetp3 when Z==1, 2, 3, resp.

int tsetp(int x, int y, int val)
{   
    //int r,g,b;

    //r = val & 0xff;
    //g = val >> 8 & 0xff;
    //b = val >> 16 & 0xff;

    /// WHY CLAMP COORDINATES?  Shouldn't the tsetp be ignored if out of bounds????
    if (x < 0) x = 0;
    else if (x >= fmc.X) x = fmc.X - 1;
    if (y < 0) y = 0;
    else if (y >= fmc.Y) y = fmc.Y - 1;

    
    *(unsigned32 *) ( fmc.tbuf + (y*fmc.X + x)*fmc.Z ) = val;

    //fmc.tbuf[(y*fmc.X + x)*fmc.Z + 0] = r;
    //fmc.tbuf[(y*fmc.X + x)*fmc.Z + 1] = g;
    //fmc.tbuf[(y*fmc.X + x)*fmc.Z + 2] = b;

    return val;

}

int t2getp(int x, int y)
{   

    //int r,g,b;

    //Added by Harald Heim, April 20, 2003
    if (x < 0) x = 0;
    else if (x >= fmc.X) x = fmc.X - 1;
    if (y < 0) y = 0;
    else if (y >= fmc.Y) y = fmc.Y - 1;


    return *(unsigned32 *) (fmc.t2buf + (y*fmc.X + x)*fmc.Z );


    /*r= fmc.t2buf[(y*fmc.X + x)*fmc.Z + 0];
    g= fmc.t2buf[(y*fmc.X + x)*fmc.Z + 1];
    b= fmc.t2buf[(y*fmc.X + x)*fmc.Z + 2];

    return ((b & 0xff) << 8 | (g & 0xff)) << 8 | (r & 0xff);*/
    

}

int t3getp(int x, int y)
{   

    //int r,g,b;

    //Added by Harald Heim, April 20, 2003
    if (x < 0) x = 0;
    else if (x >= fmc.X) x = fmc.X - 1;
    if (y < 0) y = 0;
    else if (y >= fmc.Y) y = fmc.Y - 1;


    return *(unsigned32 *) (fmc.t3buf + (y*fmc.X + x)*fmc.Z );


    /*r= fmc.t3buf[(y*fmc.X + x)*fmc.Z + 0];
    g= fmc.t3buf[(y*fmc.X + x)*fmc.Z + 1];
    b= fmc.t3buf[(y*fmc.X + x)*fmc.Z + 2];

    return ((b & 0xff) << 8 | (g & 0xff)) << 8 | (r & 0xff);*/
    

}

int t4getp(int x, int y)
{   

    //int r,g,b;

    //Added by Harald Heim, April 20, 2003
    if (x < 0) x = 0;
    else if (x >= fmc.X) x = fmc.X - 1;
    if (y < 0) y = 0;
    else if (y >= fmc.Y) y = fmc.Y - 1;


    return *(unsigned32 *) (fmc.t4buf + (y*fmc.X + x)*fmc.Z );


    /*r= fmc.t4buf[(y*fmc.X + x)*fmc.Z + 0];
    g= fmc.t4buf[(y*fmc.X + x)*fmc.Z + 1];
    b= fmc.t4buf[(y*fmc.X + x)*fmc.Z + 2];

    return ((b & 0xff) << 8 | (g & 0xff)) << 8 | (r & 0xff);*/
    

}

//// Use "pluggable" versions of psetp: psetp4 when Z >=4;
///  psetp1, psetp2, psetp3 when Z==1, 2, 3, resp.

int t2setp(int x, int y, int val)
{   
    //int r,g,b;

    //r = val & 0xff;
    //g = val >> 8 & 0xff;
    //b = val >> 16 & 0xff;

    /// WHY CLAMP COORDINATES?  Shouldn't the setp be ignored if out of bounds????
    if (x < 0) x = 0;
    else if (x >= fmc.X) x = fmc.X - 1;
    if (y < 0) y = 0;
    else if (y >= fmc.Y) y = fmc.Y - 1;

    
    *(unsigned32 *) ( fmc.t2buf + (y*fmc.X + x)*fmc.Z ) = val;

    //fmc.tbuf[(y*fmc.X + x)*fmc.Z + 0] = r;
    //fmc.tbuf[(y*fmc.X + x)*fmc.Z + 1] = g;
    //fmc.tbuf[(y*fmc.X + x)*fmc.Z + 2] = b;

    return val;

}

int t3setp(int x, int y, int val)
{   
    //int r,g,b;

    //r = val & 0xff;
    //g = val >> 8 & 0xff;
    //b = val >> 16 & 0xff;
    if (x < 0) x = 0;
    else if (x >= fmc.X) x = fmc.X - 1;
    if (y < 0) y = 0;
    else if (y >= fmc.Y) y = fmc.Y - 1;

    
    *(unsigned32 *) ( fmc.t3buf + (y*fmc.X + x)*fmc.Z ) = val;

    //fmc.t3buf[(y*fmc.X + x)*fmc.Z + 0] = r;
    //fmc.t3buf[(y*fmc.X + x)*fmc.Z + 1] = g;
    //fmc.t3buf[(y*fmc.X + x)*fmc.Z + 2] = b;

    return val;

}

int t4setp(int x, int y, int val)
{   
    //int r,g,b;

    //r = val & 0xff;
    //g = val >> 8 & 0xff;
    //b = val >> 16 & 0xff;
    if (x < 0) x = 0;
    else if (x >= fmc.X) x = fmc.X - 1;
    if (y < 0) y = 0;
    else if (y >= fmc.Y) y = fmc.Y - 1;

    
    *(unsigned32 *) ( fmc.t4buf + (y*fmc.X + x)*fmc.Z ) = val;

    //fmc.t4buf[(y*fmc.X + x)*fmc.Z + 0] = r;
    //fmc.t4buf[(y*fmc.X + x)*fmc.Z + 1] = g;
    //fmc.t4buf[(y*fmc.X + x)*fmc.Z + 2] = b;

    return val;

}

int psetp(int x, int y, int val)
{   

    int r,g,b,a, pos;
