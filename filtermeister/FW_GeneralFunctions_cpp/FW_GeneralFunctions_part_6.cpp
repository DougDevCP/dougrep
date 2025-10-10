int t4getr(int d, int m, int z)
{
    return fmc.t4get(r2x(d,m) + (fmc.X/2),
                    r2y(d,m) + (fmc.Y/2),
                    z);
}

int t4setr(int d, int m, int z, int val)
{
    return fmc.t4set(r2x(d,m) + (fmc.X/2),
                    r2y(d,m) + (fmc.Y/2),
                    z, val);
}


int ft3getr(double d, double m, int z)
{
    return fmc.t3get((int)fm_fr2x(d,m) + (fmc.X/2),
                 (int)fm_fr2y(d,m) + (fmc.Y/2),
                    z);
}
int ft4getr(double d, double m, int z)
{
    return fmc.t4get((int)fm_fr2x(d,m) + (fmc.X/2),
                 (int)fm_fr2y(d,m) + (fmc.Y/2),
                    z);
}



int ctl(int i)
{
   /* Value of control i */ 
   if (i >= 0 && i < N_CTLS)
      return fmc.pre_ctl[i];
   return 0;
}


int scl(int a, int il, int ih, int ol, int oh)
{
    //Does this work correctly when il > ih ????
#if 0
    //should really be this...
    if (ih - il + 1 != 0)
        return (a - il)*(oh - ol + 1)/(ih - il + 1) + ol;
    else
        return 0;   //or ol???
#else
    //but Filter Factory implements it this way...
    if (ih != il)
        return (a - il)*(oh - ol)/(ih - il) + ol;
    else
        return 0;   //per Jens and Filter Factory
#endif
}

double fscl(double a, double il, double ih, double ol, double oh)
{
    if (ih != il)
        return (a - il)*(oh - ol)/(ih - il) + ol;
    else
        return 0;   //per Jens and Filter Factory
}


int val(int i, int a, int b)
{
    //return scl(ctl(i), 0, 255, a, b);
#if 0
    //should really be this...
    return ctl(i)*(b - a + 1)/256 + a;
#elif 0
    //or, better yet, this...
    return (fmc.ctl[i].val - fmc.ctl[i].lo) * (b - a + 1)/
           (fmc.ctl[i].hi - fmc.ctl[i].lo + 1)
           + a;
#elif 0
    //but Filter Factory implements it this way...
    return ctl(i)*(b - a)/255 + a;
#else
    //final compromise:
    assert(IS_POWER_OF_TWO(N_CTLS));
    i &= N_CTLS-1;  //safe index
    if (gParams->ctl[i].maxval == gParams->ctl[i].minval)
        return a;   //why not?
    return (gParams->ctl[i].val - gParams->ctl[i].minval) * (b - a)/
           (gParams->ctl[i].maxval - gParams->ctl[i].minval)
           + a;
#endif
}

int map(int i, int n)
{   
#if 0
    // Following is per Jens, but not what Adobe doc says!!!
    int x, y;
    x = ctl(i*2);
    y = ctl(i*2 + 1);
    return n*(y-x)/255 + x;
#else
    // Okay, here's the way it really works (and what Adobe
    // says it should be!)
    int H, L;
    H = ctl(i*2);
    L = ctl(i*2+1);
    //return (n < L) ? 0 : (n > H) ? 255 : (H == L) ? 255 : (n-L)*255/(H-L);
    //not correct when H < L
    return (n < L) ? 0 : (n > H) ? 255 : (H == L) ? 255 : (n-L)*255/(H-L);
    //following is sometimes off by 1 when ctl(0) < ctl(1); ie. rounding towards 0 vs -inf.
    //max(0,min(255,(ctl(1)==ctl(0)?((x-ctl(1))>=0?255:0):(x-ctl(1))*255/(ctl(0)-ctl(1)))))/2+64
#endif
}


/*
int fm_min(int a, int b)
{
    return (a <= b) ? a : b;
}

int fm_max(int a, int b)
{
    return (a >= b) ? a : b;
}
*/

/***********************************************/
/****** FM-implemented f.p. math routines ******/
/***********************************************/

/////////////////////////////////////////////////////////////////////
//
// fmax(x1,x2,...) : return maximum of x1, x2, ...
//
/////////////////////////////////////////////////////////////////////
#ifdef _WIN64   // 2 operands only
double fmax(double a, double b)
{
    return (a >= b) ? a : b;
}//fm_max
#else   // variable number of operands (2 or more)
double fmax(double a, double b, ...)
{
    double fmax;
    int nargs;
    _asm mov nargs, ecx;
    fmax = (a >= b) ? a : b;
    if (nargs > 2) {
        va_list ap;
        va_start(ap, b);
        while (--nargs > 1) {
            double z = va_arg(ap, double);
            if (z > fmax) {
                fmax = z;
            }
        }
        va_end(ap);
    }
    return fmax;
}//fm_max
#endif

/////////////////////////////////////////////////////////////////////
//
// fmin(x1,x2,...) : return minimum of x1, x2, ...
//
/////////////////////////////////////////////////////////////////////
#ifdef _WIN64   // 2 operands only
double fmin(double a, double b)
{
    return (a <= b) ? a : b;
}//fm_min
#else   // variable number of operands (2 or more)
double fmin(double a, double b, ...)
{
    double fmin;
    int nargs;
    _asm mov nargs, ecx;
    fmin = (a <= b) ? a : b;
    if (nargs > 2) {
        va_list ap;
        va_start(ap, b);
        while (--nargs > 1) {
            double z = va_arg(ap, double);
            if (z < fmin) {
                fmin = z;
            }
        }
        va_end(ap);
    }
    return fmin;
}//fm_min
#endif

/////////////////////////////////////////////////////////////////////
//
// round(x) : round x to nearest or even
//
/////////////////////////////////////////////////////////////////////
#ifndef _WIN64
/************** FAIRLY FAST LIBRARY IMPLEMENTATION *****************/
// Disable "no return value" warnings
#pragma warning(disable:4035)
double round(double x)
{
    //code based on FM's in-line implementation...
    __asm {
        fld x                       ; load x
#if 1
        fnstcw fmc.current_cw       ; save current cw (probably redundant!)
        fldcw fmc.CW_ROUND          ; new cw with RC=round
        frndint                     ; round to nearest or even
        fldcw fmc.current_cw        ; restore current cw
#else
        frndint                     ; All we need if ROUND is always the default!!!
#endif
    }
}//fm_round
// restore "no return value" warnings
#pragma warning (default: 4035)
#elif 1
/************** FIRST REFERENCE IMPLEMENTATION, VERY SLOW **********/
double round(double x)
{
    // Kludgy but correct reference implementation,
    // with an early-out speedup:
    double round_near_or_up = floor(x + 0.5L);
    double round_near_or_down = ceil(x - 0.5L);
#if 1 //early out for speed?
    if (round_near_or_up == round_near_or_down) {
        return round_near_or_up;
    }
#endif
    if ( floor(round_near_or_up*0.5L)*2.0 == round_near_or_up ) {
        return round_near_or_up;    // round_near_or_up is even
    }
    else {
        assert ( floor(round_near_or_down*0.5L)*2.0 == round_near_or_down );
        return round_near_or_down;  // round_near_or_down is even
    }
}//fm_round
#else
/************** 2nd REFERENCE IMPLEMENTATION, EVEN SLOWER **********/
double round(double x)
{
    // Kludgy but correct reference implementation:
    double round_near_or_up = floor(x + 0.5L);
    if ( floor(round_near_or_up*0.5L)*2.0 == round_near_or_up )
        return round_near_or_up;    // round_near_or_up is even
    else
        return ceil(x - 0.5L);  // round_near_or_down must be even
}//fm_round
#endif

/////////////////////////////////////////////////////////////////////
//
// chop(x) : truncate x toward 0.0
//
////////////////////////////////////////////////////////////////////
#ifndef _WIN64
/************** FAIRLY FAST LIBRARY IMPLEMENTATION *****************/
// Disable "no return value" warnings
#pragma warning(disable:4035)
double chop(double x)
{
    //code based on FM's in-line implementation...
    __asm {
        fld x                       ; load x
#if 1
        fnstcw fmc.current_cw       ; save current cw (probably redundant!)
#endif
        fldcw fmc.CW_CHOP           ; new cw with RC=chop
        frndint                     ; chop toward 0.0
        fldcw fmc.current_cw        ; restore current cw
    }
}//fm_chop
// restore "no return value" warnings
#pragma warning (default: 4035)
#elif 1
/************** REFERENCE IMPLEMENTATION, FAIRLY SLOW **********/
double chop(double x)
{
    // Kludgy but correct reference implementation:
    return (x >= 0.0) ? floor(x) : -floor(-x);
}//fm_chop
#endif

/////////////////////////////////////////////////////////////////////
//
// iround(x) : round double x to nearest or even, return as 32-bit integer
//
////////////////////////////////////////////////////////////////////
#ifndef _WIN64
/************** FAIRLY FAST LIBRARY IMPLEMENTATION *****************/
// Disable "no return value" warnings
#pragma warning(disable:4035)
int iround(double x)
{
    //code based on FM's in-line implementation...