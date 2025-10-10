    }
    else if (i == -1) {
        /* return +1 if n is even, -1 if n is odd */
#if 0
        return (n & 1) ? -1 : 1;
#elif 1
        return 1 - ((n & 1) << 1);  // best code under Win32 Release
#elif 1
        return (-(n & 1) & -2) + 1;
#endif
    }
    else {
        return 0;
    }

}//fm_ipow
#elif 1
/************** FASTER INTEGER IMPLEMENTATION **********************/
#define T int /* template type */
T ipow(T value, int n)
{
   /* Explicitly compute the first few powers, this is never going to be as 
    * efficient as simply putting x*x (for example) in the code, but it is 
    * better than needing to go through initialization and loop constructs. 
    */ 
   T z = 1; 
   T y; 
    

   // In general, the binary-exponentiation algorithm for pow(x, n)
   // requires lg2(n) + onebits(n) - 1 multiply ops to compote, if n > 0,
   // where: lg2(n) is the largest integer i such that 2**i <= n, and
   //        onebits(n) is the number of 1-bits in the binary
   //        representation of n
   // e.g.:
   //                                         binary    add-chain   linear
   //           n  lg(2) + onebits(n) - 1  =  mulops      mulops    mulops
   //        ----- -----   ---------- ---     ------    ---------   ------
   //           1     0  +      1     - 1         0         0           0
   //           2     1  +      1     - 1         1         1           1
   //           3     1  +      2     - 1         2         2           2
   //           4     2  +      1     - 1         2         2           3
   //           5     2  +      2     - 1         3         3           4
   //           6     2  +      2     - 1         3         3           5
   //           7     2  +      3     - 1         4         4           6
   //           8     3  +      1     - 1         3         3           7
   //           9     3  +      2     - 1         4         4           8
   //          10     3  +      2     - 1         4         4           9
   //          11     3  +      3     - 1         5         5          10
   //          12     3  +      2     - 1         4         4          11
   //          13     3  +      3     - 1         5         5          12 
   //          14     3  +      3     - 1         5         5          13
   //          15     3  +      4     - 1         6         5          14
   //          16     4  +      1     - 1         4         6          15
   //
   // For comparison, we show the number of multiply ops required by an optimal
   // addition/multiplcation chain and for linear calculation of the product
   // (mulops = n - 1) in the last two columns.  We see that the lowest n for
   // which the add-chain algorithm takes fewer mulops than binary exponentiation
   // is n = 15.
   // For n < = 18, we can always compute pow(x, n) with no more than 5 multiplies.
   //
   // Except for a few other special cases (such as n  power of 2), it's probably
   // not worth unrolling the loop for n > 18 in the long run...
   //
   switch( n ) 
   { 
     T register temp;
     case 0: 
       return z; 
     case 1:     // 0 mulops
       return value; 
     case 2:     // 1 mulop
       return value * value; 
     case 3:     // 2 mulops
       return value * value * value; 
     case 4:     // 2 mulops
       return temp = value * value, temp * temp;
     case 5:     // 3 mulops
       return temp = value * value, temp *= temp, temp * value;
     case 6:     // 3 mulops
       return temp = value * value * value, temp * temp;
     case 7:     // 4 mulops
       return temp = value * value * value, temp *= temp, temp * value;
     case 8:     // 3 mulops
       return temp = value * value, temp *= temp, temp * temp;
     case 9:     // 4 mulops
       return temp = value * value, temp *= temp, temp *= temp, temp * value;
     case 10:    // 4 mulops
       return temp = value * value, temp *= temp, temp *= value, temp * temp;
     case 11:    // 5 mulops
       return temp = value * value, temp *= temp, temp *= value, temp *= temp, temp * value;
     case 12:    // 4 mulops
       return temp = value * value * value, temp *= temp, temp * temp;
     case 13:    // 5 mulops
       return temp = value * value * value, temp *= temp, temp *= temp, temp * value;
     case 14:    // 5 mulops
       return temp = value * value * value, temp *= temp, temp *= value, temp * temp;
     case 15:    // 5 mulops (1st time add-chain beats binary exponentiation (6 mulops) per [Knuth]!!)
       return temp = value * value, temp *= temp, temp *= value, temp * temp * temp;
     case 16:    // 4 mulops
       return temp = value * value, temp *= temp, temp *= temp, temp * temp;
     case 17:    // 5 mulops
       return temp = value * value, temp *= temp, temp *= temp, temp *= temp, temp * value;
     case 18:    // 5 mulops
       return temp = value * value, temp *= temp, temp *= temp, temp *= value, temp * temp;
   } 
    
   if( n < 0 ) /* If n is negative then we need to compute the inverse */ 
   { 
     n = -n; 
     y = z / value; 
   } 
   else 
   { 
     y = value; 
   } 

   do 
   { 
     if( n & 0x1u ) /* 0x1u is the hexadecimal literal constant representing */ 
     {              /* an unsigned integer == 1. n & 0x1u means that the */ 
       z *= y;      /* smallest bit of n is zero, in this case the current */ 
     }              /* repeated square power of value (== y) is a component */ 
                    /* of the result. */ 
      
     y *= y;    /* Repeatedly square on each bit. */ 
     n >>= 1;   /* Iterate through all the bits. */ 
      
   }while( n ); /* If n == 0 then we are done. */ 
    
   return z; 
}//fm_ipow
#undef T
#elif 0
/************** FAIRLY FAST LIBRARY IMPLEMENTATION *****************/
// Disable "no return value" warnings
#pragma warning(disable:4035)
int ipow(int i, int n)
{

}//fm_ipow
// restore "no return value" warnings
#pragma warning (default: 4035)
#elif 1
/************** REFERENCE IMPLEMENTATION, FAIRLY SLOW **********/
int ipow(int i, int n)
{
    // Kludgy but correct reference implementation:
    return (int)pow((double)i, (double)n);
}//fm_ipow
#endif


//////////////////////////// powi(x, n) //////////////////////////////////
//
// powi(x, n) : calculate double x to the power integer n, 
//              returns a double.
// we assume: powi(x,0) = 1.0, even when x==0.0.
//
////////////////////////////////////////////////////////////////////
#if 1
/************** NAIVE INTEGER IMPLEMENTATION **********************/
double powi(double x, int n)
{
    if (n < 0) {
        if (x == 0.0) {
            /* Domain error: return +infinity */
            errno = EDOM;
            return HUGE_VAL;  ///////////// NO +INFINITY !!!!
        }
        else if (x == 1.0) { //speedy out?
            return 1.0;
        }
        else if (x == -1.0) { //speedy out?
            /* return +1 if n is even, -1 if n is odd */
            return (n & 1) ? -1.0 : 1.0;
        }
        /* compute powi(1/x, -n) */
        x = 1.0/x;
        n = -n;
    }
    assert(n >= 0);
    if (n > 0) {
        double res = x;
        while (--n > 0) res *= x;
        return res;
    }
    assert(n == 0);
    /* Assume powi(0.0,0) = 1.0, not indefinite */
    if (x == 0.0) errno = EDOM; // but flag an EDOM error for powi(0.0,0)
    return 1.0;

}//fm_powi
#elif 1
/************** FASTER INTEGER IMPLEMENTATION **********************/
#define T double /* template type */
T powi(T value, int n)
{
   /* Explicitly compute the first few powers, this is never going to be as 
    * efficient as simply putting x*x (for example) in the code, but it is 
    * better than needing to go through initialization and loop constructs. 
    */ 
   T z = 1; 
   T y; 
    

   // In general, the binary-exponentiation algorithm for pow(x, n)
   // requires lg2(n) + onebits(n) - 1 multiply ops to compute, if n > 0,
   // where: lg2(n) is the largest integer i such that 2**i <= n, and
   //        onebits(n) is the number of 1-bits in the binary
   //        representation of n
   // e.g.:
   //                                         binary    add-chain   linear
   //           n  lg(2) + onebits(n) - 1  =  mulops      mulops    mulops
   //        ----- -----   ---------- ---     ------    ---------   ------
   //           1     0  +      1     - 1         0         0           0
   //           2     1  +      1     - 1         1         1           1
   //           3     1  +      2     - 1         2         2           2
   //           4     2  +      1     - 1         2         2           3
   //           5     2  +      2     - 1         3         3           4
   //           6     2  +      2     - 1         3         3           5
   //           7     2  +      3     - 1         4         4           6
   //           8     3  +      1     - 1         3         3           7
   //           9     3  +      2     - 1         4         4           8
   //          10     3  +      2     - 1         4         4           9
   //          11     3  +      3     - 1         5         5          10
   //          12     3  +      2     - 1         4         4          11
   //          13     3  +      3     - 1         5         5          12 
   //          14     3  +      3     - 1         5         5          13
   //          15     3  +      4     - 1         6         5          14
   //          16     4  +      1     - 1         4         6          15
   //
   // For comparison, we show the number of multiply ops required by an optimal
   // addition/multiplcation chain and for linear calculation of the product
   // (mulops = n - 1) in the last two columns.  We see that the lowest n for
   // which the add-chain algorithm takes fewer mulops than binary exponentiation
   // is n = 15.
   // For n < = 18, we can always compute pow(x, n) with no more than 5 multiplies.
   //
   // Except for a few other special cases (such as n = power of 2), it's probably
   // not worth unrolling the loop for n > 18 in the long run...
   //
   switch( n ) 
   { 
     T register temp;
     case 0: 
       return z; 
     case 1:     // 0 mulops
       return value; 
     case 2:     // 1 mulop
       return value * value; 
     case 3:     // 2 mulops
       return value * value * value; 
     case 4:     // 2 mulops
       return temp = value * value, temp * temp;
     case 5:     // 3 mulops
       return temp = value * value, temp *= temp, temp * value;
     case 6:     // 3 mulops
       return temp = value * value * value, temp * temp;
     case 7:     // 4 mulops
       return temp = value * value * value, temp *= temp, temp * value;
     case 8:     // 3 mulops
       return temp = value * value, temp *= temp, temp * temp;
     case 9:     // 4 mulops
       return temp = value * value, temp *= temp, temp *= temp, temp * value;
     case 10:    // 4 mulops
       return temp = value * value, temp *= temp, temp *= value, temp * temp;
     case 11:    // 5 mulops
       return temp = value * value, temp *= temp, temp *= value, temp *= temp, temp * value;
     case 12:    // 4 mulops
       return temp = value * value * value, temp *= temp, temp * temp;
     case 13:    // 5 mulops
       return temp = value * value * value, temp *= temp, temp *= temp, temp * value;
     case 14:    // 5 mulops
       return temp = value * value * value, temp *= temp, temp *= value, temp * temp;
     case 15:    // 5 mulops (1st time add-chain beats binary exponentiation (6 mulops) per [Knuth]!!)
       return temp = value * value, temp *= temp, temp *= value, temp * temp * temp;
     case 16:    // 4 mulops
       return temp = value * value, temp *= temp, temp *= temp, temp * temp;
     case 17:    // 5 mulops
       return temp = value * value, temp *= temp, temp *= temp, temp *= temp, temp * value;
     case 18:    // 5 mulops
       return temp = value * value, temp *= temp, temp *= temp, temp *= value, temp * temp;
   } 
    
   if( n < 0 ) /* If n is negative then we need to compute the inverse */ 
   { 
     n = -n; 
     y = z / value; 
   } 
   else 
   { 
     y = value; 
   } 

   do 
   { 
     if( n & 0x1u ) /* 0x1u is the hexadecimal literal constant representing */ 
     {              /* an unsigned integer == 1. n & 0x1u means that the */ 