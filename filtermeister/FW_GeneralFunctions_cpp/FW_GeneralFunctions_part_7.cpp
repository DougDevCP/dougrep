    int iRes;
    __asm {
        fld x                       ; load x
#if 1
        fnstcw fmc.current_cw       ; save current cw (probably redundant!)
        fldcw fmc.CW_ROUND          ; new cw with RC=round
        fistp iRes                  ; convert to integer, rounding to nearest or even
        fldcw fmc.current_cw        ; restore current cw
#else
        fistp iRes                  ; All we need if ROUND is always the default!!!
#endif
        mov eax, iRes               ; return int result in eax
    }
}//fm_iround
// restore "no return value" warnings
#pragma warning (default: 4035)
#elif 1
/************** REFERENCE IMPLEMENTATION, FAIRLY SLOW **********/
int iround(double x)
{
    // Kludgy but correct reference implementation:
    return (int)round(x);
}//fm_iround
#endif

/////////////////////////////////////////////////////////////////////
//
// ichop(x) : truncate double x toward 0, return as 32-bit integer
//
////////////////////////////////////////////////////////////////////
#ifndef _WIN64
/************** FAIRLY FAST LIBRARY IMPLEMENTATION *****************/
// Disable "no return value" warnings
#pragma warning(disable:4035)
int ichop(double x)
{
    //code based on FM's in-line implementation...
    int iRes;
    __asm {
        fld x                       ; load x
#if 1
        fnstcw fmc.current_cw       ; save current cw (probably redundant!)
#endif
        fldcw fmc.CW_CHOP           ; new cw with RC=chop
        fistp iRes                  ; convert to integer, chopping toward 0
        fldcw fmc.current_cw        ; restore current cw
        mov eax, iRes               ; return int result in eax
    }
}//fm_ichop
// restore "no return value" warnings
#pragma warning (default: 4035)
#elif 1
/************** REFERENCE IMPLEMENTATION, FAIRLY SLOW **********/
int ichop(double x)
{
    // Kludgy but correct reference implementation:
    return (int)chop(x);
}//fm_ichop
#endif

/////////////////////////////////////////////////////////////////////
//
// ifloor(x) : round double x down toward -Infinity,
//             return as a 32-bit integer.
//
////////////////////////////////////////////////////////////////////
#ifndef _WIN64
/************** FAIRLY FAST LIBRARY IMPLEMENTATION *****************/
// Disable "no return value" warnings
#pragma warning(disable:4035)
int ifloor(double x)
{
    //code based on FM's in-line implementation...
    int iRes;
    __asm {
        fld x                       ; load x
#if 1
        fnstcw fmc.current_cw       ; save current cw (probably redundant!)
#endif
        fldcw fmc.CW_FLOOR          ; new cw with RC=floor
        fistp iRes                  ; convert to integer, rounding down toward -Infinity
        fldcw fmc.current_cw        ; restore current cw
        mov eax, iRes               ; return int result in eax
    }
}//fm_ifloor
// restore "no return value" warnings
#pragma warning (default: 4035)
#elif 1
/************** REFERENCE IMPLEMENTATION, FAIRLY SLOW **********/
int ifloor(double x)
{
    // Kludgy but correct reference implementation:
    return (int)floor(x);
}//fm_ifloor
#endif

/////////////////////////////////////////////////////////////////////
//
// iceil(x) : round double x up toward +Infinity, 
//            return as a 32-bit integer.
//
////////////////////////////////////////////////////////////////////
#ifndef _WIN64
/************** FAIRLY FAST LIBRARY IMPLEMENTATION *****************/
// Disable "no return value" warnings
#pragma warning(disable:4035)
int iceil(double x)
{
    //code based on FM's in-line implementation...
    int iRes;
    __asm {
        fld x                       ; load x
#if 1
        fnstcw fmc.current_cw       ; save current cw (probably redundant!)
#endif
        fldcw fmc.CW_CEIL           ; new cw with RC=ceil
        fistp iRes                  ; convert to integer, truncating toward +Infinity
        fldcw fmc.current_cw        ; restore current cw
        mov eax, iRes               ; return int result in eax
    }
}//fm_iceil
// restore "no return value" warnings
#pragma warning (default: 4035)
#elif 1
/************** REFERENCE IMPLEMENTATION, FAIRLY SLOW **********/
int iceil(double x)
{
    // Kludgy but correct reference implementation:
    return (int)ceil(x);
}//fm_iceil
#endif


//////////////////////////// ipow(i, n) ///////////////////////////////
//
// ipow(i, n) : calculate integer i to the power integer n, 
//              returns a 32-bit integer.
// we assume: ipow(i,0) = 1, even when i==0.
//
////////////////////////////////////////////////////////////////////
#if 0
/************** NAIVE INTEGER IMPLEMENTATION **********************/
int ipow(int i, int n)
{
    if (n > 0) {
#if 0
        int iRes = i;
        while (--n) 
        {// no overflow check
            iRes *= i;
        }
#elif 0
        int iRes = i;
        while (--n) 
        {// overflow check
            iRes *= i;
            __asm jo oflo1;
        }
#elif 0
        int iRes = i;
        char register oflo = 0;
        while (--n && !oflo) 
        {// overflow check
            iRes *= i;
            __asm  seto oflo;
        }
        if (oflo) goto oflo1;
#else
        int iRes;
        __asm { //overflow check in asm
             mov     ecx, n
	         mov	 edx, i
  		     dec	 ecx
  		     mov	 eax, edx
  		     je	     DONE
LP01:
  	         imul	 eax, edx
             jo      oflo1
  		     dec	 ecx
  		     jne	 LP01
DONE:
             mov     iRes, eax
        }//asm
#endif
        return iRes;
    }
    else if (n == 0) {
        /* Assume ipow(0,0) = 1, not indefinite */
        if (i == 0) errno = EDOM; // but flag an EDOM error for ipow(0,0)
        return 1;
    }
    /* n < 0 */
    else if (i == 0) {
        /* Domain error: return approximation to +infinity */
        errno = EDOM;
        return INT_MAX; //anything better??
    }
    else if (i == 1) {
        return 1;
    }
    else if (i == -1) {
        /* return +1 if n is even, -1 if n is odd */
#if 0
        return (n & 1) ? -1 : 1;
#elif 1
        return 1 - ((n & 1) << 1);  // best code under Release
#elif 1
        return (-(n & 1) & -2) + 1;
#endif
    }
    else {
        return 0;
    }

oflo1:  /* Overflow => Range error: return approximation to +infinity */
        errno = ERANGE;
        return INT_MAX; //anything better??

}//fm_ipow
#elif 1
/************** LESS NAIVE INTEGER IMPLEMENTATION **********************/
// This one uses the binary exponentiation algorithm.
int ipow(int i, int n)
{
    int iRes = 1;

    /// But first, unroll the loop for the first 18 or so cases...

    if (n > 0) {  //how about --n, start with iRes = i (elim 1 mult, and also eliminate final i*=i if n>>1 is 0???
#ifdef _WIN64
        do { // no overflow checking
            if (n & 1) {
                iRes *= i;  /* If n is odd, multiply in an i. */
            } 
            i *= i;         /* Repeatedly square on each bit. */ 
            n >>= 1;        /* Iterate through all the bits. */ 
        } while (n);        /* If n == 0 then we are done. */ 
#elif 1
        do { // overflow check
            if (n & 1) {
                iRes *= i;  /* If n is odd, multiply in an i. */
                __asm jo oflo1;
            } 
            i *= i;         /* Repeatedly square on each bit. */ 
            __asm jo oflo2; ///// oflo2: oflow iff n >> 1 != 0...
            n >>= 1;        /* Iterate through all the bits. */ 
        } while (n);        /* If n == 0 then we are done. */ 
#elif 1
        char register oflo = 0;
        char register oflo2 = 0;
        do { // overflow check
            if (n & 1) {
                iRes *= i;  /* If n is odd, multiply in an i. */
                __asm  seto oflo;
            } 
            i *= i;         /* Repeatedly square on each bit. */ 
            __asm  seto oflo2;
            n >>= 1;        /* Iterate through all the bits. */ 
        } while (n && !oflo && !oflo2);  /* If n == 0 then we are done. */ 
        if (oflo || n) goto oflo1;
#else
        __asm { //overflow check in asm
            ////// WRONG //////
             mov     ecx, n
	         mov	 edx, i
  		     dec	 ecx
  		     mov	 eax, edx
  		     je	     DONE
LP01:
  	         imul	 eax, edx
             jo      oflo1
  		     dec	 ecx
  		     jne	 LP01
DONE:
             mov     iRes, eax
        }//asm
#endif
        return iRes;

oflo2:  /* Overflow here only if n >> 1 != 0 */
        if ((n >> 1) == 0) return iRes;
        // fall through to overflow case...
oflo1:  /* Overflow => Range error: return approximation to +infinity */
        errno = ERANGE;
        return INT_MAX; //anything better??

    }
    else if (n == 0) {
        /* Assume ipow(0,0) = 1, not indefinite */
        if (i == 0) errno = EDOM; // but flag an EDOM error for ipow(0,0)
        return 1;
    }
    /* n < 0 */
    else if (i == 0) {
        /* Domain error: return approximation to +infinity */
        errno = EDOM;
        return INT_MAX; //anything better??
    }
    else if (i == 1) {
        return 1;