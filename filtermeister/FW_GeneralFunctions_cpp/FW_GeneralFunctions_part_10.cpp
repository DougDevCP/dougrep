    //benchmark = 6.1
    return (int)( (256.0 * 0.63661977236758134308) * atan2((double)y, (double)x) );
#endif
#endif
}

int c2m(int x, int y)
{
    //FF benchmark = 11.50 / 6.25 -- uses precomputed table for x/y!!!
#if 0
    //benchmark = 15.10
    //advantage: doesn't easily overflow -- IF convert x, y to double first!!!
    return (int)( sqrt((double)(x*x + y*y)) );
#elif 1
    //benchmark = 12.62 / 8.47
    return isqrt(x*x + y*y);
#elif 0
    //benchmark = 12.34 / 8.25
    register unsigned int v = x*x + y*y;
    register unsigned int t = 1L<<30, r = 0, s;

#   define STEP(k) s = t + r; r>>= 1; if (s <= v) { v -= s; r |= t;}

    STEP(15);  t >>= 2;
    STEP(14);  t >>= 2;
    STEP(13);  t >>= 2;
    STEP(12);  t >>= 2;
    STEP(11);  t >>= 2;
    STEP(10);  t >>= 2;
    STEP(9);   t >>= 2;
    STEP(8);   t >>= 2;
    STEP(7);   t >>= 2;
    STEP(6);   t >>= 2;
    STEP(5);   t >>= 2;
    STEP(4);   t >>= 2;
    STEP(3);   t >>= 2;
    STEP(2);   t >>= 2;
    STEP(1);   t >>= 2;
    STEP(0);

#   undef STEP

    return r;
#else
    //benchmark = 15.81 for 2 N-R's, 18.71 for 3 N-R's.
    int d = abs(x) + abs(y);
    int h;
    if (d == 0) return 0;
    h = x*x + y*y;      //easily overflows!
    d = (h/d + d) >> 1; //1st N-R iteration
    d = (h/d + d) >> 1; //2nd N-R iteration
    //d = (h/d + d) >> 1; //3rd N-R iteration
    return d;
#endif
}

double fm_fc2d(double x, double y)
{
    return 256.0 * 0.63661977236758134308 * atan2(y, x);
}

double fm_fc2m(double x, double y)
{
    return sqrt(x*x + y*y);
}

int get(int i)
{
    return fmc.cell[i & (N_CELLS-1)];
}

int put(int v, int i)
{
    return fmc.cell[i & (N_CELLS-1)] = v;
}

int cnv(int m11, int m12, int m13,
                  int m21, int m22, int m23,
                  int m31, int m32, int m33,
                  int d)
{
    int x = fmc.x;
    int y = fmc.y;
    int z = fmc.z;

    if (d == 0)
    {   // default divisor to sum of weights...
        d = m11+m12+m13+m21+m22+m23+m31+m32+m33;    // ???
        if (d == 0)
            return 255; // give up if divisor still 0
    }
    return (
        //could check for safe coordinates in toto and
        //then use unsafe src() to speed up...
        //or set up pointer to src(x-1,y-1,z), then
        //bump it through all 9 points without
        //recomputing index expression each time...
            m11*fmc.src(x-1, y-1, z) +
            m12*fmc.src(x  , y-1, z) +
            m13*fmc.src(x+1, y-1, z) +
            m21*fmc.src(x-1, y  , z) +
            m22*fmc.src(x  , y  , z) +
            m23*fmc.src(x+1, y  , z) +
            m31*fmc.src(x-1, y+1, z) +
            m32*fmc.src(x  , y+1, z) +
            m33*fmc.src(x+1, y+1, z)
           ) / d;
}

/********* FF+ built-in functions ***********/


INT_PTR pointer_to_buffer(int a, int x, int y, int z)
{
    if (a==0) //Input buffer
    {
        return (INT_PTR)MyAdjustedInData +(x-fmc.x_start)*fmc.srcColDelta + (y-fmc.y_start)*fmc.srcRowDelta + z;
    }
    else if (a==1) //Temp1 buffer
    {
        return (INT_PTR)fmc.tbuf + ((y-fmc.y_start)*fmc.columns + (x-fmc.x_start))*fmc.Z + z;
    }
    else if (a==2) //Temp2 buffer
    {
        return (INT_PTR)fmc.t2buf + ((y-fmc.y_start)*fmc.columns + (x-fmc.x_start))*fmc.Z + z;
    }
    else if (a==3) //Output buffer
    {
        return (INT_PTR)gStuff->outData + (x-fmc.x_start)*fmc.dstColDelta + (y-fmc.y_start)*fmc.dstRowDelta + z;
    }
    else if (a==4) //Temp3 buffer
    {
        return (INT_PTR)fmc.t3buf + ((y-fmc.y_start)*fmc.columns + (x-fmc.x_start))*fmc.Z + z;
    }
    else if (a==5) //Temp4 buffer
    {
        return (INT_PTR)fmc.t4buf + ((y-fmc.y_start)*fmc.columns + (x-fmc.x_start))*fmc.Z + z;
	}

	return false;

}


//function added by Harald Heim on Feb 5, 2002
int phaseshift(int a, int b) 
    //a=Color Value, b=Shift Value from 0 to 512
{   
    if (b < 0) b=0;
    else if (b > 512) b=512;

    if (b==0)
        return a;
    else
        //return 255 - scl( fm_sin( scl(a,0,255,256-b*4,768+b*4) ) ,-512,511,0,255 );
        return 255 - ( fm_sin( scl(a,0,255,256-b*4,768+b*4) ) +512 ) / 4;

}


//This function is NOT THREAD_SAFE!!!!
int blend(int a, int b, int z, int m, int r) 
    //a=Bottom Color Value, b=Top Color Value, m=Blend Mode, r = blending ratio from 0 (bottom) to 255 (top)
{   
    //THREAD_LOCAL 
    static int noise, col;
    int calc, scaled;
	

    if (a < 0) a=0;	//afh 01-may-2008
    else if (a > mode_maxVal) a=mode_maxVal;	//afh 01-may-2008
    if (b < 0) b=0;
    else if (b > mode_maxVal) b=mode_maxVal;
    if (r < 0) r=0;
    else if (r > mode_maxVal) r=mode_maxVal;

    if (r==255 && m<4)
        return b;
    else if (r==0)
        return a;
    else
    {
        
        switch (m)
        {
            case 0://Normal
                if (r==255)
					return b;
				else
					return ((255-r)*a + r*b)/255;
					//return mix(a,b,255-r,255);

            case 1: //Dissolve
                if (z==0) noise=rnd(0,255);
                return r < noise ? a : b;
				//return r < rnd(0,255) ? a : b;

            case 2: //Threshold
                if (z==0) col = a;
                return col < (255-r)*mode_bitMultiply ? a : b;

            case 3: //Threshold 2
                if (z==0) col = b;
                return col < (255-r)*mode_bitMultiply ? a : b;

            case 4: //Multiply
                calc = a*b/mode_maxVal;
                break;

            case 5: //Screen
                calc = mode_maxVal-((mode_maxVal-a)*(mode_maxVal-b))/mode_maxVal;
                break;

            case 6: //Overlay
                calc = a<mode_midVal ? (2*a*b)/mode_maxVal : mode_maxVal-(2*(mode_maxVal-a)*(mode_maxVal-b))/mode_maxVal;
                break;

            case 7: //Soft Light
                //scaled = scl(a,0,maxVal,64,192);
                scaled = a/2 + mode_quarVal;
                calc =  a<mode_midVal ? (2*b*scaled)/mode_maxVal : mode_maxVal-2*(mode_maxVal-scaled)*(mode_maxVal-b)/mode_maxVal;
                break;
            
            case 8: //Hard Light -> Same as overlay!
                calc = b<mode_midVal ? (2*a*b)/mode_maxVal : mode_maxVal-(2*(mode_maxVal-a)*(mode_maxVal-b))/mode_maxVal;
                break;

            case 9://Dodge
                //calc = maxVal*a/max(1,maxVal-b);
                calc = b >= mode_maxVal? mode_maxVal : min(a * mode_maxVal / (mode_maxVal - b), mode_maxVal);
                //calc = (a * 256 / (256 - b)) - 1; //Faster Alternative
                break;

            case 10: //Burn
                //calc = maxVal-(maxVal*(maxVal-a)/(b+1));
                calc = b <= 0? 0 : max(mode_maxVal - ((mode_maxVal - a) * mode_maxVal / b), 0);
                break;

            case 11://Darken
                calc = min(a,b);
                break;

            case 12: //Lighten
                calc = max(a,b);
                break;

            case 13://Exclusion
                calc = mode_maxVal-(((mode_maxVal-a)*(mode_maxVal-b)/mode_maxVal)+(a*b/mode_maxVal));
                break;

            case 14://Difference
                calc = dif(a,b);
                break;

            case 15://Negative Difference 1
                calc = mode_maxVal-dif(a,mode_maxVal-b);
                break;

            case 16://Negative Difference 2
                calc = dif(mode_maxVal-a,b);
                break;

            case 17://Subtract
                calc = a-b;
                break;

            case 18://Add
                calc = a+b;
                break;

            case 19://Expose
                calc = a+a-b;
				break;

			/*case 20://Soft Light 2
                //calc = b > mode_midVal ? 
				//		mode_maxVal - (mode_maxVal-a) * (mode_maxVal-(b-mode_midVal)) / mode_maxVal :
				//	    ((2*b-mode_maxVal) * (a-a*a/mode_maxVal))/ mode_maxVal  + a;
				{
					double value;
					double cb = (double)a / mode_maxVal;
					double cs = (double)b / mode_maxVal;
					if (cs <= .5) {
						value = cb - (1-2*cs) * cb * (1-cb);
					} else {
						value = cb <= .25 ? ((16*cb-12)*cb+4)*cb : sqrt(cb);
						value = cb + (2*cs-1) * (value-cb);
					}
					calc = (int)(value * mode_maxVal);
				}*/

        }//end switch


        if (r==255)
			return calc;
		else
			return ((255-r)*a + r*calc)/255;
			//return mix(a, calc, 255-r,255);
