	if (edge_a > edge_b)
	{
		swapvalue 	= edge_b;
		edge_b 		= edge_a;
		edge_a 		= swapvalue;
	}

	if (value > edge_b)
	{
		value = edge_b * 2 - value;
		if(value < edge_a) value = -1;
	}
	else if (value < edge_a)
	{
		value = edge_a - value;
		if(value > edge_b) value = -1;
	}
	return value;
}



//function added by Harald Heim on Feb 6, 2002
int gray(int r, int g, int b, int rweight, int gweight, int bweight)
{
    int totalweight;
    totalweight = rweight + gweight + bweight;

    if (totalweight==0)
        return 0;
    if (r == g && g == b)
        return r;
    else
        return (rweight*r + gweight*g + bweight*b) / (totalweight);
}

//function added by Harald Heim on Feb 6, 2002
/*int swap (int & a, int & b)
{
    int c;

    c = a;
    a = b;
    b = c;

    return true;
}*/


//function added by Harald Heim on Feb 6, 2002
int rgb2iuv(int r, int g, int b, int z)
{
    if (z == 0)
        return (76*r + 150*g + 29*b)/256;
    else if (z == 1)
        return (-19*r - 37*g + 56*b)/256;
    else if (z == 2)
        return (78*r - 65*g - 13*b)/256;
    else
        return 0;
}

//function added by Harald Heim on Feb 8, 2002
int iuv2rgb(int i, int u, int v, int z)
{
    if (z == 0)
        return (39*i + 90*v)/39;
    else if (z == 1)
        return (39*i - 31*u - 46*v)/39;
    else if (z == 2)
        return (39*i + 158*u)/39;
    else
        return 0;
}

//function added by Harald Heim on Feb 8, 2002
int rgb2cmyk(int r, int g, int b, int z)
{
    int k;
    
    k=255*mode_bitMultiply-max(r,max(g,b));

    if (z == 0)
        return 255*mode_bitMultiply-r-k;
    else if (z == 1)
        return 255*mode_bitMultiply-g-k;
    else if (z == 2)
        return 255*mode_bitMultiply-b-k;
    else if (z == 3)
        return k;
    else
        return 0;
}

//function added by Harald Heim on Feb 8, 2002
int cmyk2rgb(int c, int m, int y, int k, int z)
{
    if (z == 0)
        return 255*mode_bitMultiply-c-k;
    else if (z == 1)
        return 255*mode_bitMultiply-m-k;
    else if (z == 2)
        return 255*mode_bitMultiply-y-k;
    else
        return 0;
}


//function added by Harald Heim on Feb 6, 2002
int solarize(int a, int s)
{
    if (s < 0) s=0;
    else if (s > 255) s=255;

    return abs(s-a);
}


//function added by Harald Heim on Feb 7, 2002
int tricos(int x)
{ //triangle wave similiar to cos()
    x=abs(x);	//afh 01-may-2008

    if (x%1024 < 512) 
        return 512 - ((x%512)*2);
    else 
        return ((x%512)*2) - 512;
}

//function added by Harald Heim on Feb 7, 2002
int tri(int x)
{
    return tricos(x-256);
}

//function added by Harald Heim on Feb 9, 2002
int sinbell(int x)
{
    //Make sure x is between 0 and 1024 to make sure that the output is in the same range
    x = abs(x%1025);

    return fm_sin(x-256)+512;
}


//function added by Harald Heim on Feb 7, 2002
int grad2D(int x, int y, int X, int Y,int grad, int dist, int repeat)
// x / y = current x / y value
// X / Y = maximal value  of gradient field
//grad = gradient type
//dist = 0 for linear, 1 for sine distribution
//repeat = number of repetitions, 0 for no repetition
{
    int calc, m, M, d;

    if (X==0 || Y==0)
        return 0;

    //Linear gradient formulas
    switch(grad)
    {
        case 0: //Horizontal
            calc = x*255/X;
            break;

        case 1: // Vertical
            calc = y*255/Y;
            break;

        case 2: //Diagonal
            calc = (x+y)*255/(X+Y); //or  calc = x*128/X + y*128/Y;
            break;

        case 3: //Diagonal 2
            calc = 128+(x*255/X - y*255/Y)/2;
            break;

        case 4: //Radial
            m = c2m(x-X/2,y-Y/2);
            M = c2m(X,Y)/2; 
            calc = m*255/M;
            break;

        case 5: //Ellipsoid
            //calc = c2m(scl(x,0,X,-1024,1024), scl(y,0,Y,-1024,1024))/5;
            calc = c2m(x*2048/X-1024, y*2048/Y-1024)/5;
            break;

        case 6: //Pyramid
            if (x<=X/2 && y<=Y/2)    calc= 2 * min(x*255/X,y*255/Y);
            else if (x>X/2 && y<Y/2) calc= 2 * min(255-x*255/X,y*255/Y);
            else if (x<X/2 && y>Y/2) calc= 2 * min(x*255/X,255-y*255/Y);
            else                     calc= 2 * min(255-x*255/X,255-y*255/Y);
            break;

        case 7: //Beam
            calc = c2d(x,y);
            break;

        case 8: //Angular
            d = c2d(x-X/2,y-Y/2);
            //calc = scl(d,-512,512,255,0);
            calc = 255-(d+512)/4;
            break;

        case 9: //Star
            calc = 128-((128-x*255/X)*(128-y*255/Y)/128);   
            break;

        case 10: //Quarter Pyramid 1
            calc = min(x*255/X,y*255/Y);
            break;

        case 11: //Quarter Pyramid 2
            calc = 255-max(x*255/X,y*255/Y);
            break;

        case 12: //Quarter Radial
            M = c2m(X,Y)/2; 
            calc = c2m(x,y)*128/M;
            break;
            
    }//end switch


    //Sine distribution & repetition
    if (dist > 0)
    {
        if (repeat==0)
            repeat = 2;
        else
            repeat = repeat * 4;

        //calc = scl( fm_sin(calc*repeat-256) ,-511,512,0,255);
        calc = ( fm_sin(calc*repeat-256) + 512) / 4  ;
    }

    //Linear repetitions
    else if  (dist ==0 && repeat>0)
    {
        repeat = repeat * 4;

        //calc = scl( tricos(calc*repeat+512),-511,512,0,255); 
        calc = ( tricos(calc*repeat+512) + 512) / 4; 
    }   

    return calc;
}

//function added by Harald Heim on Feb 8, 2002
int tone(int a, int h, int m, int d)
{
    if (h < -128) h=-128;
    else if (h > 128)  h=128;
    if (m < -128) m=-128;
    else if (m > 128)  m=128;
    if (d < -128) d=-128;
    else if (d > 128)  d=128;
    
    return a+(d*(256-abs(a-43))/256)+(m*(256-2*abs(a-128))/256)+(h*(256-abs(a-213))/256);
}

//function added by Harald Heim on Feb 10, 2002
int xyzcnv(int x, int y, int z, int m11, int m12, int m13,
                  int m21, int m22, int m23, int m31, int m32, int m33,
                  int d)
{
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

//function added by Harald Heim on Feb 10, 2002
int cell_initialize(int i)			//is sense reversed???
{
    if (i==0) 
        Dont_Initialize_Cells = 0; //Cells aren't initialized
    else
        Dont_Initialize_Cells = 1; //Cells are initialized
    
    return true;