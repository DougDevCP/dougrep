///////////////////////////////////////////////////////////////////////////////////
//  File: FW_GeneralFunctions.h
//
//  Header file of FilterMeisterVS which provides the dialog and image processing
//  functions which are directly called from the image processing code
//
//  Copyright (c) 1997-2017 by Alex Hunter, Harald Heim & Martijn Van der Lee
// 
//  FilterMeisterVS is free software. You can redistribute and modify it
//  under the terms of the MPL2 license. See https://www.mozilla.org/en-US/MPL/2.0/
//  Any changes to the FilterMeisterVS files in the /components/filtermeister/
//  sub folder have to be published or sent to harry@thepluginsite.com. 
//  The individual filter code itself does not need to be disclosed.
//  
///////////////////////////////////////////////////////////////////////////////////


#ifdef __cplusplus
#define EXTERNC extern "C"
#else
#define EXTERNC
#endif


/*****************************************************************************/
//Define necessary 64bit types
#ifndef _WIN64
    #if _MSC_VER < 1200
        typedef int INT_PTR, *PINT_PTR;
        typedef unsigned int UINT_PTR, *PUINT_PTR;
    #endif

    typedef long LONG_PTR, *PLONG_PTR;
    typedef unsigned long ULONG_PTR, *PULONG_PTR;

    #define __int3264   __int32

    typedef ULONG_PTR SIZE_T, *PSIZE_T;
    typedef LONG_PTR SSIZE_T, *PSSIZE_T;
    typedef ULONG_PTR DWORD_PTR, *PDWORD_PTR;

    typedef int intptr_t;
#endif



// IS FOLLOWING THREAD-SAFE??
extern int arrayMode;
extern void *Array[100]; //Initialized in DoStart
#ifndef APP
    extern BufferID ArrayID[100]; //for BufferProc //Initialized in DoStart
    extern BufferID bufferID;
#endif
extern UINT_PTR ArraySize[100];
extern int ArrayBytes[100], ArrayX[100], ArrayY[100], ArrayZ[100]; //Initialized in DoStart
//Added by Harald Heim, Oct 12, 2003
extern int ArrayPadding[100]; //Initialized in DoStart

extern HIMAGELIST ImageList[10]; 

#define N_FONTS 32
extern HFONT Font[N_FONTS];


    /* Module-local global constants. */

static const double twopi = 6.28318530717958647693;
static const double twobypi = 0.63661977236758134308;


    /* Module-local global variables. */

static int gNoMask;
static int gBevWidth;
//static int minRow, maxRow, minCol, maxCol;
static int X0, Y0;

static int Dont_Initialize_Cells = 0;




// ---------------------- functions------------------------------------------


/**********************************************************************/
/* Map4DBUsToPixels(&x1, &y1, &x2, &y2)
/*
/*  Maps 4 coordinates from Dialog Box Units to pixels.
/*  x1 and x2 are mapped as horizontal coordinates;
/*  y1 and y2 are mapped as vertical coordinates.
/*  (Horizontal and vertical mappings are generally identical,
/*  but not always!)
/**********************************************************************/
EXTERNC void Map4DBUsToPixels(int *x1, int *y1, int *x2, int *y2);

/**********************************************************************/
/* Map2DBUsToPixels(&x1, &y1)
/*
/*  Same as Map4DBUsToPixels(), but maps only 2 coordinates instead of 4.
/*  x1 is mapped as a horizontal coordinate;
/*  y1 is mapped as a vertical coordinate.
/**********************************************************************/
EXTERNC void Map2DBUsToPixels(int *x1, int *y1);

/**********************************************************************/
/* HDBUsToPixels(h)
/*
/*  Returns the number of pixels comprising h horizontal DBUs.
/*
/**********************************************************************/
EXTERNC int HDBUsToPixels(int h);

/**********************************************************************/
/* VDBUsToPixels(v)
/*
/*  Returns the number of pixels comprising v vertical DBUs.
/*
/**********************************************************************/
EXTERNC int VDBUsToPixels(int v);


//----------------------------------------------------------------
// Utility routines for simulating Filter Factory

EXTERNC int pset(int x, int y, int z, int val);
EXTERNC int pset16(int x, int y, int z, int val);

EXTERNC void fast_pset(int x, int y, int z, int val);

EXTERNC int tset(int x, int y, int z, int val);

EXTERNC int tset16(int x, int y, int z, int val);

EXTERNC int t2set(int x, int y, int z, int val);

EXTERNC int t2set16(int x, int y, int z, int val);

EXTERNC int t3set(int x, int y, int z, int val);

EXTERNC int t3set16(int x, int y, int z, int val);

EXTERNC int t4set(int x, int y, int z, int val);

EXTERNC int t4set16(int x, int y, int z, int val);

EXTERNC int pget(const int x0, const int y0, const int z0);

//Added by Harald Heim Feb 9, 2002
EXTERNC int pget16(const int x0, const int y0, const int z0);

EXTERNC int tget(const int x0, const int y0, const int z0);

EXTERNC int tget16(const int x0, const int y0, const int z0);

EXTERNC int t2get(const int x0, const int y0, const int z0);

EXTERNC int t2get16(const int x0, const int y0, const int z0);

EXTERNC int t3get(const int x0, const int y0, const int z0);

EXTERNC int t3get16(const int x0, const int y0, const int z0);

EXTERNC int t4get(const int x0, const int y0, const int z0);

EXTERNC int t4get16(const int x0, const int y0, const int z0);

EXTERNC int src(const int x0, const int y0, const int z0);
EXTERNC int src16(const int x0, const int y0, const int z0);

EXTERNC int fast_src(int x, int y, int z);

EXTERNC int fast_src3 (int x, int y, int z);
EXTERNC int fast_pget3 (int x, int y, int z);
EXTERNC int fast_t3get3 (int x, int y, int z);
EXTERNC int fast_t4get3 (int x, int y, int z);
EXTERNC int fast_frad3(double d, double m, int z);
EXTERNC int fast_fpgetr3(double d, double m, int z);
EXTERNC int fast_ft3getr3(double d, double m, int z);
EXTERNC int fast_ft4getr3(double d, double m, int z);

EXTERNC int r2x(int d, int m);
EXTERNC int r2y(int d, int m);

EXTERNC double fm_fr2x(double d, double m);
EXTERNC double fm_fr2y(double d, double m);

EXTERNC int rad(int d, int m, int z);
EXTERNC int pgetr(int d, int m, int z);

EXTERNC int frad(double d, double m, int z);
EXTERNC int fpgetr(double d, double m, int z);

EXTERNC int psetr(int d, int m, int z, int val);

EXTERNC int tgetr(int d, int m, int z);

EXTERNC int tsetr(int d, int m, int z, int val);

EXTERNC int t2getr(int d, int m, int z);

EXTERNC int t2setr(int d, int m, int z, int val);

EXTERNC int t3getr(int d, int m, int z);

EXTERNC int t3setr(int d, int m, int z, int val);

EXTERNC int t4getr(int d, int m, int z);

EXTERNC int t4setr(int d, int m, int z, int val);

EXTERNC int ft3getr(double d, double m, int z);
EXTERNC int ft4getr(double d, double m, int z);

EXTERNC int ctl(int i);

EXTERNC int scl(int a, int il, int ih, int ol, int oh);
EXTERNC double fscl(double a, double il, double ih, double ol, double oh);

EXTERNC int val(int i, int a, int b);

EXTERNC int map(int i, int n);

EXTERNC int fm_min(int a, int b);

EXTERNC int fm_max(int a, int b);

EXTERNC int iround(double x);

/////////////////////////////////////////////////////////////////////
//
// ichop(x) : truncate double x toward 0, return as 32-bit integer
//
////////////////////////////////////////////////////////////////////

EXTERNC int ichop(double x);

/////////////////////////////////////////////////////////////////////
//
// ifloor(x) : round double x down toward -Infinity,
//             return as a 32-bit integer.
//
////////////////////////////////////////////////////////////////////

EXTERNC int ifloor(double x);

/////////////////////////////////////////////////////////////////////
//
// iceil(x) : round double x up toward +Infinity, 
//            return as a 32-bit integer.
//
////////////////////////////////////////////////////////////////////

EXTERNC int iceil(double x);


//////////////////////////// ipow(i, n) ///////////////////////////////
//
// ipow(i, n) : calculate integer i to the power integer n, 
//              returns a 32-bit integer.
// we assume: ipow(i,0) = 1, even when i==0.
//
////////////////////////////////////////////////////////////////////

EXTERNC int ipow(int i, int n);

//////////////////////////// powi(x, n) //////////////////////////////////
//
// powi(x, n) : calculate double x to the power integer n, 
//              returns a double.
// we assume: powi(x,0) = 1.0, even when x==0.0.
//
////////////////////////////////////////////////////////////////////

EXTERNC double powi(double x, int n);

/******************************************************/
/****** end of FM-implemented f.p. math routines ******/
/******************************************************/


EXTERNC int fm_abs(int a);

EXTERNC int add(int a, int b, int c);

EXTERNC int sub(int a, int b, int c);

EXTERNC int dif(int a, int b);
EXTERNC double fdif(double a, double b);

EXTERNC int rnd(int a, int b);

EXTERNC int rst(int seed);

EXTERNC int mix(int a, int b, int n, int d);

EXTERNC int mix1(int a, int b, int n, int d);

EXTERNC int mix2(int a, int b, int n, int d);
