       z *= y;      /* smallest bit of n is zero, in this case the current */ 
     }              /* repeated square power of value (== y) is a component */ 
                    /* of the result. */ 
      
     y *= y;    /* Repeatedly square on each bit. */ 
     n >>= 1;   /* Iterate through all the bits. */ 
      
   }while( n ); /* If n == 0 then we are done. */ 
    
   return z; 
}//fm_powi
#undef T
#elif 0
/************** FAIRLY FAST LIBRARY IMPLEMENTATION *****************/
// Disable "no return value" warnings
#pragma warning(disable:4035)
double powi(double x, int n)
{

}//fm_powi
// restore "no return value" warnings
#pragma warning (default: 4035)
#elif 1
/************** REFERENCE IMPLEMENTATION, FAIRLY SLOW **********/
double powi(double x, int n)
{
    // Kludgy but correct reference implementation:
    return (int)pow((double)i, (double)n);
}//fm_powi
#endif

/******************************************************/
/****** end of FM-implemented f.p. math routines ******/
/******************************************************/

/*
int fm_abs(int a)
{
    return (a >= 0) ? a : -a;
}
*/

int add(int a, int b, int c)
{
    return (a+b < c) ? a+b : c;
}

int sub(int a, int b, int c)
{
    return (a-b > c) ? a-b : c; // or abs(a-b) ???
}

int dif(int a, int b)
{
    return (a-b >= 0) ? a-b : b-a;
}

double fdif(double a, double b)
{
    return (a-b >= 0) ? a-b : b-a;
}

int rnd(int a, int b)
{
    // which is more uniformly distributed,
    // the high bits or the low bits of rand()?
#if 0
    return rand()*(b-a+1)/RAND_MAX + a;
#elif 1
    return a <= b ? rand() % (b-a+1) + a : rand() % (a-b+1) + b;
#else
    // Per Jens:  Wrong, returns [a,b) instead of [a,b].
    return a == b ? a : rand() % (b-a) + a;
#endif
}

int rst(int seed)
{
    // (Re-)seed the pseudorandom number generator.
#if 1
    //for more FF-like behavior...
    seed += rand();
#endif
    srand(seed);
    return seed;
} //fm_rnd

int mix(int a, int b, int n, int d)
{
    // FF compatible version...
    return (d != 0) ? a*n/d + b*(d-n)/d : 0;
}

int mix1(int a, int b, int n, int d)
{
#if 0
    return (d != 0) ? (a*n + b*(d-n))/d : 0;
#else
    return (d != 0) ? b - (b - a)*n/d : 0;
#endif
}

int mix2(int a, int b, int n, int d)
{
    //return (d != 0) ? b - ((b - a)*n*2 + d)/(2*d) : 0;
    return (d == 0) ? 0 :
    (b - a)*n >= 0 ? b - ((b - a)*n*2 + d)/(2*d) :
                     b + ((a - b)*n*2 + d)/(2*d) ;
}

//FF benchmark for sqr = 8.0

int fm_sqr(int x)
{
    //benchmark = 5.2
    //in-line fsqrt (OP_SQR) = 4.3
    return (x > 0) ? (int)(sqrt((double)x) + 0.0) : 0;  // trunc or round???
}

#if 0
//from Michael:
int isqrt(unsigned int x)
{
    //benchmark = 5.6
    register unsigned int r, nr, m;

    r = 0;
#if 1
    m = 0x40000000;
#else
    m = 0x4000;
#endif
    do 
    {
        nr = r + m;
        if (nr <= x) 
        {
            x -= nr;
            r = nr + m;
        }
        r >>= 1;
        m >>= 2;
        } 
    while (m != 0);

    return (int)r;
}

#else
/***********************************************************************/
/* from: http://www.research.microsoft.com/~hollasch/cgindex/math/introot.html
/* Integer Square Root
/* 
/* Ben Discoe, comp.graphics, 6 Feb 92 
/*
/***********************************************************************/
unsigned int isqrt(unsigned int v)
{
    //benchmark = 3.7 (3.4 when called from OP_SQR)
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
} /*isqrt*/
#endif

/******************** isqrt_fast: FASTCALL version *********************/
/* from: http://www.research.microsoft.com/~hollasch/cgindex/math/introot.html
/* Integer Square Root
/* 
/* Ben Discoe, comp.graphics, 6 Feb 92 
/*
/***********************************************************************/
unsigned int __fastcall isqrt_fast(unsigned int v)  //fastcall passes 'v' in ecx
{
    //benchmark = 3.7 (3.4 when called from OP_SQR)
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
} /*isqrt_fast*/


int fm_sin(int x)
{
#ifdef TRIG_ON_FLY
    //compute sine on-the-fly...
    return (int)(sin( (twopi/1024.0)*(double)x ) * 512.0 + 0.0);     // round?? can be neg!!
#else
    //use precomputed lookup table...
    return fmc.pre_sin[x & 1023];
#endif
}

int fm_cos(int x)
{
#ifdef TRIG_ON_FLY
    //compute cosine on-the-fly...
    return (int)(cos( (twopi/1024.0)*(double)x ) * 512.0 + 0.0);     // round?? can be neg!!
#else
    //use precomputed lookup table...
    return fmc.pre_sin[(x+256) & 1023];
#endif
}

int fm_tan(int x)
{
#ifdef TRIG_ON_FLY
    //compute tangent on-the-fly...
    if ( (x & 511) == 256)
        return (int)(tan( (twopi/1024.0)*(-255.5) ) * 1024.0 + 0.0);    // per Jens???
    else
        return (int)(tan( (twopi/1024.0)*(double)x ) * 1024.0 + 0.0);   // round?? can be neg!!
#else
    //use precomputed lookup table...
    return fmc.pre_tan[x & 511];
#endif
}


double fm_fsin(double x)
{
    return sin( (twopi/1024.0)*x ) * 512.0;
}
double fm_fcos(double x)
{
    return cos( (twopi/1024.0)*x ) * 512.0;
}
double fm_ftan(double x)
{
    if ( ((int)x & 511) == 256)
        return tan( (twopi/1024.0)*(-255.5) ) * 1024.0;    // per Jens???
    else
        return tan( (twopi/1024.0)*x ) * 1024.0;   // round?? can be neg!!
}




int c2d(int x, int y)
{
    //FF benchmark = 5.8
#if 0
    //benchmark = 7.1
    // per Jens (Gimp)...
    int d;

    if (y != 0)
        d = (int)(256.0 * twobypi * atan( (double)x/(double)abs(y) ) + 0.0) - 256;   // round????
    else if (x < 0)
        d = 512;
    else
        d = 0;
    return (y <= 0) ? d : -d;   // Jens had y < 0
#else
    //following is more straightforward!!!
#if 0
    // VSC doesn't fold this!!!
    return (int)( (256.0 * twobypi) * atan2((double)y, (double)x) );
#else