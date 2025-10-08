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



EXTERNC int fm_sqr(int x);

/***********************************************************************/
/* from: http://www.research.microsoft.com/~hollasch/cgindex/math/introot.html
/* Integer Square Root
/* 
/* Ben Discoe (rodent@netcom.COM), comp.graphics, 6 Feb 92 
/*
/***********************************************************************/
EXTERNC unsigned int isqrt(unsigned int v);




EXTERNC int fm_sin(int x);
EXTERNC int fm_cos(int x);

EXTERNC double fm_fcos(double x);
EXTERNC double fm_fsin(double x);
EXTERNC double fm_ftan(double x);

EXTERNC int fm_tan(int x);

EXTERNC int c2d(int x, int y);
EXTERNC int c2m(int x, int y);

EXTERNC double fm_fc2d(double x, double y);
EXTERNC double fm_fc2m(double x, double y);

EXTERNC int get(int i);

EXTERNC int put(int v, int i);

EXTERNC int cnv(int m11, int m12, int m13,
                  int m21, int m22, int m23,
                  int m31, int m32, int m33,
                  int d);


/********* FF+ built-in functions ***********/


EXTERNC INT_PTR pointer_to_buffer(int a, int x, int y, int z);


EXTERNC int phaseshift(int a, int b) ;

EXTERNC int blend(int a, int b, int z, int m, int r) ;

EXTERNC int contrast(int a, int b) ;

EXTERNC int posterize(int a, int b) ;
    
EXTERNC int saturation(int r, int g, int b, int z, int sat) ;

EXTERNC int msk(int x, int y);



//------
// routines
//------
// int egw(int edge_a, int edge_b, int value)
//		Arguments
//			edge_a
//				edge_a edge value
//			edge_b
//				edge_b edge value
//			value
//				value that will be edge wrapped.
//		Return
//			The edge-wrapped value will be returned.
//		Description
//			This function returns the value untouched if it lies between edge_a and edge_b.
//			If it lies outside edge_a and edge_b, the value will be edge wrapped to lie between
//			edge_a and edge_b as if the image is continuous.
//		Examples:
//			Input value of 21 will be returned as 11 if edge_a = 10 and edge_b = 20
//			Input value of 21 will be returned as 12 if edge_a = 10 and edge_b = 19
//			Input value of  8 will be returned as 18 if edge_a = 10 and edge_b = 20
//			Input value of  5 will be returned as 14 if edge_a = 10 and edge_b = 19.
//------
EXTERNC int egw(int edge_a, int edge_b, int value);


//------
// int egm(int edge_a, int edge_b, int value)
//
//		Arguments
//			edge_a
//				edge_a edge value
//			edge_b
//				edge_b edge value
//			value
//				value that will be edge mirrored.
//		Return
//			edge-mirrored value will be returned
//			-1 if after mirroring one time the computed value is outside the range.
//		Description
//			This function returns the value untouched if it lies between edge_a and edge_b.
//			If it lies outside edge_a and edge_b, the value will be edge mirrored to lie between
//			edge_a and edge_b as if the image is reflected back at each edge.
//		Examples:
//			Input value of 21 will be returned as 19 if edge_a = 10 and edge_b = 20
//			Input value of 21 will be returned as 18 if edge_a = 10 and edge_b = 19.
//			Input value of  8 will be returned as 12 if edge_a = 10 and edge_b = 20
//			Input value of  5 will be returned as 15 if edge_a = 10 and edge_b = 19.
//------
EXTERNC int egm(int edge_a, int edge_b, int value);



//function added by Harald Heim on Feb 6, 2002
EXTERNC int gray(int r, int g, int b, int rweight, int gweight, int bweight);



//function added by Harald Heim on Feb 6, 2002
EXTERNC int rgb2iuv(int r, int g, int b, int z);

EXTERNC int iuv2rgb(int i, int u, int v, int z);

EXTERNC int rgb2cmyk(int r, int g, int b, int z);

EXTERNC int cmyk2rgb(int c, int m, int y, int k, int z);

EXTERNC int solarize(int a, int s);

EXTERNC int tricos(int x);

EXTERNC int tri(int x);

EXTERNC int sinbell(int x);

EXTERNC int grad2D(int x, int y, int X, int Y,int grad, int dist, int repeat);

EXTERNC int tone(int a, int h, int m, int d);

EXTERNC int xyzcnv(int x, int y, int z, int m11, int m12, int m13,
                  int m21, int m22, int m23, int m31, int m32, int m33,
                  int d);


EXTERNC int cell_initialize(int i);			//is sense reversed???


EXTERNC int set_edge_mode(int mode);

EXTERNC int set_bitdepth_mode(int mode);

EXTERNC int get_bitdepth_mode();

/// Use pluggable variants of psetp() instead of testing a flag at run-time!!!

EXTERNC int set_psetp_mode(int mode);

EXTERNC int iget(double x, double y, int z, int buffer, int mode);
EXTERNC int igetArrayEx(int nr, double x, double y, int z, int mode);

EXTERNC int bCircle(int x, int y, int centerx, int centery, int radius);
 

EXTERNC int bRect(int x, int y, int centerx, int centery, int radiusx, int radiusy);

EXTERNC int bRect2(int x, int y, int topx, int topy, int bottomx, int bottomy);

EXTERNC int bTriangle(int x, int y, int centerx, int centery, int radius);

EXTERNC int refreshWindow (void);
EXTERNC int refreshWindowNoErase (void);

EXTERNC int refreshRgn (INT_PTR Rgn);

EXTERNC int setCtlOrder (int n, int order);

EXTERNC int refreshCtl (int n, ... );

EXTERNC int refreshItems(int n);

EXTERNC int lockCtl (int n);
EXTERNC int unlockCtl (int n);

EXTERNC int lockWindow (int c);

EXTERNC int setZoom (int n, int m);

EXTERNC int setZoomEx (int n, int m);



EXTERNC int set_array_mode(int mode);

EXTERNC int freeHost (INT_PTR bufferid);

EXTERNC int allocArray (int nr, int X, int Y, int Z, int bytes);

EXTERNC int allocArrayPad (int nr, int X, int Y, int Z, int bytes, int padding);

EXTERNC int freeArray (int nr);

EXTERNC void swapArray(int nr, int nr2);

EXTERNC int getArray (int nr, int x, int y, int z);

EXTERNC int fast_getArray (int nr, int x, int y, int z);

EXTERNC int UNSAFE_getArray (int nr, int x, int y, int z);

EXTERNC int UNSAFE_getArrayNext8 (int nr);

EXTERNC int UNSAFE_getArrayNext16 (int nr);

EXTERNC int UNSAFE_getArrayNext32 (int nr);

EXTERNC double fgetArray (int nr, int x, int y, int z);

EXTERNC int putArray (int nr, int x, int y, int z, int val);

EXTERNC void fast_putArray (int nr, int x, int y, int z, int val);

EXTERNC int UNSAFE_putArray (int nr, int x, int y, int z, int val);

EXTERNC int fputArray (int nr, int x, int y, int z, double val);

EXTERNC int UNSAFE_fputArray (int nr, int x, int y, int z, double val);

EXTERNC int fputArrayLE (int nr, int x, int y, int z, double val);

EXTERNC int putArrayString (int nr, int x, LPSTR string);

EXTERNC char* getArrayString (int nr, int x);

EXTERNC unsigned8 gMskVal(int row, int rowOff, int col, int colOff);

EXTERNC int calcSBD(int bevWidth);

EXTERNC int freeSBD(void);

EXTERNC int getSBD(int j, int i);

EXTERNC int getSBDangle(int j, int i, int a10);

EXTERNC int getSBDX(int j, int i);

EXTERNC int getSBDY(int j, int i);



EXTERNC int getCtlPos (int n, int w);

EXTERNC int getCtlCoord (int n, int w);

EXTERNC int getPreviewCoordX (void);

EXTERNC int getPreviewCoordY (void);

EXTERNC int getAsyncKeyState (int t);

EXTERNC int getAsyncKeyStateF (int t);

EXTERNC int getAsyncKeyStateFC (int t);

EXTERNC int mouseOverWhenInvisible (int t);


EXTERNC int arrayExists (int nr);

EXTERNC int getArrayDim (int nr, int dim);

EXTERNC int copyArray (int src, int dest);

EXTERNC int fillArray (int nr, int val);

EXTERNC int ffillArray (int nr, double dval);



EXTERNC int ctlEnabled(int n);

EXTERNC int sub_ctlEnabledAs(int n, int depth);

EXTERNC int ctlEnabledAs(int n);

EXTERNC int rgb2lab(int r, int g, int b, int z);

EXTERNC int lab2rgb(int l, int a, int b, int z);


EXTERNC int setClickDrag(int b);

EXTERNC int setTimerEvent(int nr, int t, int state);

EXTERNC int getDisplaySettings(int s);

EXTERNC int scrollPreview(int mode, int ox, int oy, int update);

EXTERNC int rgb2hsl(int r, int g, int b, int z);
EXTERNC int hsl2rgb(int h, int s, int l, int z);

EXTERNC void rgb2hsl_fast8(int * r, int * g, int * b);
EXTERNC void hsl2rgb_fast8(int * h, int * s, int * l);
EXTERNC void rgb2hsl_fast16(int * r, int * g, int * b);
EXTERNC void hsl2rgb_fast16(int * h, int * s, int * l);

EXTERNC int rgb2hsl_safe(int r, int g, int b, int z);
EXTERNC int hsl2rgb_safe(int h, int s, int l, int z);

EXTERNC int rgb2ycbcr(int r, int g, int b, int z);

EXTERNC int ycbcr2rgb(int y, int cb, int cr, int z);

EXTERNC int rgb2ypbpr(int r, int g, int b, int z);

EXTERNC int ypbpr2rgb(int y, int pb, int pr, int z);


//// Optimizations: Pluggable variants based on Z, bit depth, etc.?
////                Use aligned 1 byte/2 byte or 2 byte/1 byte accesses?
////                Factor out subscript expression if VC doesn't.

//// How about  srcRGB(), srcRGBA(), srcGrey(), srcDuo(), srcCMYK(), etc.
////         or src3(), src4(), src1(), src2(), src4(), ...
//// Ditto for  pgetRGB(), pgetRGBA(), etc.?


EXTERNC int srcp(int x, int y);
//int srcp(UINT_PTR x, UINT_PTR y);

EXTERNC int pgetp(int x, int y);

EXTERNC int tgetp(int x, int y);

//// Overlaps when Z<4???

//// Use "pluggable" versions of tsetp: tsetp4 when Z >=4;
///  tsetp1, tsetp2, tsetp3 when Z==1, 2, 3, resp.

EXTERNC int tsetp(int x, int y, int val);

EXTERNC int t2getp(int x, int y);

EXTERNC int t3getp(int x, int y);

EXTERNC int t4getp(int x, int y);

EXTERNC int t2setp(int x, int y, int val);

EXTERNC int t3setp(int x, int y, int val);

EXTERNC int t4setp(int x, int y, int val);

EXTERNC int psetp(int x, int y, int val);

EXTERNC int getWindowsVersion(void);

EXTERNC int setPreviewCursor(INT_PTR res);

EXTERNC int getPreviewCursor(void);

EXTERNC int deleteRgn(INT_PTR Rgn);

//EXTERNC int comp(const void *a, const void *b )

//EXTERNC int quickSort(int first, int last)


//Added by Harald Heim, May 17, 2003
EXTERNC int quickSort(int first, int last);

EXTERNC int quickFill(int x, int y, int z, int buffer, int radius, int xstart, int ystart, int xend, int yend);

EXTERNC int quickMedian(int low, int high);//(elem_type arr[], int n) ;

EXTERNC int getCtlClass(int n);

EXTERNC int getCtlView(int n);

EXTERNC int getComboOpen(int n);

EXTERNC int getCtlItemCount(int n);

EXTERNC int getCtlItemTop(int n);

EXTERNC int getCtlItemPos(int n, int item, int val);

EXTERNC int setCtlItemTop(int n, int item);

EXTERNC int getCtlItemText(int n, int item, char * str);

EXTERNC int setCtlItemText(int n, int item, char * str);

EXTERNC int setCtlItemState(int n, int item, int state);

EXTERNC int deleteCtlItem(int n, int item);

EXTERNC int deleteCtlItems(int n);

EXTERNC int setCtlGroupText(int n, int group, char * str);
EXTERNC int setCtlGroupState(int n, int group, int state);
EXTERNC int getCtlGroupState(int n, int group);
EXTERNC int getCtlGroupCount(int n);
EXTERNC int setCtlItemGroup(int n, int item, int group);
EXTERNC int setCtlGroupView(int n, int on);

EXTERNC int setCtlImageList(int n, int i);
EXTERNC int setCtlItemImage(int n, int item, int i);
EXTERNC int getCtlItemImage(int n, int item);
EXTERNC int setCtlItemSpacing(int n, int x, int y, int applyLabelHeight);
EXTERNC int setCtlExtStyle(int n, int exstyle);
EXTERNC int clearCtlExtStyle(int n, int exstyle);


EXTERNC int fillDir(int n, int attr, int wildcard);

EXTERNC int getResSize(char* restype, char* resname);

EXTERNC int copyResToArray (char* restype, char*resname, int arraynr);
EXTERNC int copyResToArrayEx (char* restype, char* resname, int arraynr, int X, int Y, int Z);

EXTERNC int getScreenSize(int s, int fAbs);
EXTERNC int getDialogMaxSize(int s);

EXTERNC int setDialogShowState(int state);
EXTERNC int setDialogShowStateEx(int state, int width, int height);
EXTERNC int getDialogRestoreWidth();
EXTERNC int getDialogRestoreHeight();
EXTERNC int getDialogWindowState();

EXTERNC int setDialogSizeMax(void);

EXTERNC int setDialogSizeGrip(int state);

EXTERNC void unlockPreviewFrame(int state);

EXTERNC int PixelsToHDBUs(int h) ;

EXTERNC int PixelsToVDBUs(int v) ;

EXTERNC int setCtlMate(int n, int s);
EXTERNC int getCtlMate(int n);

EXTERNC int setCtlTab(int n, int t, int s);

EXTERNC int shiftTabSheet(int t, int shiftval);

EXTERNC int getCtlTab(int n, int t);

EXTERNC int setCtlDefTab(int n, int t, int s);
EXTERNC int getCtlDefTab(int n, int m);

EXTERNC int setCtlAnchor(int n, int flags);

EXTERNC int updateAnchors(int dialogWidth, int dialogHeight);
EXTERNC int updateAnchor(int n, int dialogWidth, int dialogHeight);

EXTERNC int lockCtlScaling(int lock);

EXTERNC int scaleCtls(int dialogWidth,int dialogHeight) ;

                 


EXTERNC int countProcessors(void);
EXTERNC void doMultiThreading(LPTHREAD_START_ROUTINE ThreadFunction, int useMT, int useSync);


EXTERNC int waitForThread(INT_PTR hThread, int ms, int userinput);

EXTERNC int isThreadActive(INT_PTR hThread);

EXTERNC int getThreadRetVal(INT_PTR hThread);

EXTERNC int terminateThread(INT_PTR hThread);


/***** Critical Section APIs *****/

EXTERNC INT_PTR createCriticalSection(void);
	
#ifdef HAVE_XP  //requires XP or later
EXTERNC int createCriticalSectionAndSpinCount(DWORD dwSpinCount);
#endif //XP

#ifdef HAVE_VISTA //requires Vista or later
EXTERNC int createCriticalSectionEx(DWORD dwSpinCount, DWORD flags);
#endif //Vista

#ifdef HAVE_XP  //requires XP or later
EXTERNC int setCriticalSectionSpinCount(int hCS, DWORD spinCount);
#endif //XP

EXTERNC BOOL enterCriticalSection(INT_PTR hCS);

#if _WIN32_WINNT >= 0x0400 
EXTERNC BOOL tryEnterCriticalSection(INT_PTR hCS);
#endif

EXTERNC BOOL leaveCriticalSection(INT_PTR hCS);

EXTERNC BOOL deleteCriticalSection(INT_PTR hCS);


/****** Interlocked Variable API's ******/

EXTERNC int interlockedIncrement(INT_PTR pvar);
    
EXTERNC int interlockedDecrement(INT_PTR pvar);
    
EXTERNC int interlockedExchange(INT_PTR ptarget, int value);
    
EXTERNC int interlockedExchangeAdd(INT_PTR ptarget, int value);
    
EXTERNC int interlockedCompareExchange(INT_PTR pdest, int exchange, int comparand);


/****** Thread Synchronization API's ******/

EXTERNC int createSync(int number);

EXTERNC int waitForSync(int syncnr, int timeout);

EXTERNC int deleteSync(void);


//Simple Section items
EXTERNC void createSection();
EXTERNC void enterSection();
EXTERNC void leaveSection();
EXTERNC void deleteSection();



/****** EVENT API'S ******/

EXTERNC int triggerEvent(int n, int event, int previous );

EXTERNC int doEvents(void);


EXTERNC int restoreRect (void);

EXTERNC int requestRect (int inLeft, int inTop, int inRight, int inBottom, int scaleFactor);



/********************************************************************/
/*                 DLL Access Routines                              */
/*                                                                  */
/*  1. Original code contributed by Florian Xhumari, as transmitted */
/*     by Harald Heim.                                              */
/*  2. Remove commented-out code (Alex Hunter).                     */
/*                                                                  */
/********************************************************************/

EXTERNC INT_PTR loadLib(INT_PTR libname);

EXTERNC int freeLib(INT_PTR hinstLib);

EXTERNC int callLibFmc(int fn, ...);

	/**
	 * Calls a function fn that takes exactly the same parameters as used
	 * to call this function, except first argument (fn) is replaced by
     * a pointer to the FM Context record (fmcp).
     * This function is agnostic about the return value, which may be void,
     * int (returned in EAX), double (returned on top of NDP stack ST(0)),
     * etc., depending on an appropriate cast of the function signature.
     *
	 * Example of called functions in a DLL: 
     *
     *      __declspec(dllexport)
     *      int MyForEveryTile(FMcontext *fmcp, int X, int Y) { return false; }
     *
     *      __declspec(dllexport)
     *      double MyFloat(FMcontext *fmcp, int a, double x, double y) {
     *          return a*(x - y);
     *      }
     *
     * To invoke these functions from FM:
     *
     *      int g_hMyDll = loadLibrary("MyDll");
     *      if (!g_hMyDll) ERROR...
     *
     *      int g_pfnMyForEveryTile = getLibraryFunction(g_hMyDll, "MyForEveryTile");
     *      if (!g_pfnMyForEveryTile) ERROR...
     *
     *      int g_pfnMyFloat = getLibraryFunction(g_hMyDll, "MyFloat");
     *      if (!g_pfnMyFloat) ERROR...
     *
     *      int iRes = callLibraryIntFast(g_pfnMyForEveryTile, 100, 200);
     *
     *      double fRes = callLibraryDoubleFast(g_pfnMyFloat, 10, 1.2, 3.14);
     *
     * where FM effectively defines callLibraryIntFast and callLibraryDoubleFast as:
     *
     *      typedef int (*ifn1v) (int, ...);
     *      typedef double (*dfn1v) (int, ...);
     *      #define callLibraryIntFast      ((ifn1v)callLibraryFast)
     *      #define callLibraryDoubleFast   ((dfn1v)callLibraryFast)
	 */

EXTERNC int callLib(int fn, ...);


EXTERNC int __cdecl getCpuReg(int nr);

EXTERNC void __cdecl finit(void);

EXTERNC int __cdecl fstsw(void);

EXTERNC int __cdecl fstcw(void);

EXTERNC void __cdecl fldcw(int cw);


/****************** End DLL Access Routines *************************/




EXTERNC int setPreviewDragMode (int mode);

EXTERNC int linearInterpolate (int v1,int v2, double x);

EXTERNC int cosineInterpolate (int v1,int v2, double x);

EXTERNC int cubicInterpolate (int v0,int v1,int v2,int v3,double x);

EXTERNC int hermiteInterpolate (int T1,int P1,int P2,int T2, double s);

EXTERNC int getImageTitle (char * n);

EXTERNC int getEXIFSize ();
int getEXIFData (int * buffer);

EXTERNC int destroyMenu(INT_PTR hMenu);

EXTERNC int insertMenuItem (INT_PTR hMenu, int uItem, char* itemName, int fState, INT_PTR subMenu);
EXTERNC int insertMenuItemEx (INT_PTR hMenu, int uItem, char* itemName, int fState, int bullet, INT_PTR subMenu);

EXTERNC int trackPopupMenu (INT_PTR hMenu, int type, int x, int y, int style);

EXTERNC int setMenu (INT_PTR hMenu);

EXTERNC int getCtlFocus (void);

EXTERNC int setCtlFocus (int n);

EXTERNC int checkCtlFocus (int n);

EXTERNC int checkDialogFocus (void);


EXTERNC INT_PTR findFirstFile (LPCTSTR lpFileName, LPSTR foundItem, int *dwFileAttributes);

EXTERNC int findNextFile (INT_PTR hFindFile, LPSTR foundItem, int *dwFileAttributes);

EXTERNC BOOL findClose(INT_PTR hFindFile);



EXTERNC int getLocaleInfo(LCID Locale,
                            LCTYPE LCType,
                            LPSTR  lpLCData,
                            int    cchData);

EXTERNC int getSpecialFolder (int val, char* str);

EXTERNC int setCtlStyle(int n, int flags);

EXTERNC int clearCtlStyle(int n, int flags);

EXTERNC int setCtlStyleEx(int n, int flags);

EXTERNC int clearCtlStyleEx(int n, int flags);

EXTERNC int setCtlBuddyStyle(int n, int buddy, int flags);

EXTERNC int clearCtlBuddyStyle(int n, int buddy, int flags);

EXTERNC int setCtlBuddyStyleEx(int n, int buddy, int flags);

EXTERNC int clearCtlBuddyStyleEx(int n, int buddy, int flags);

EXTERNC int setCtlBuddyFontColor(int n, int color);
EXTERNC int setCtlBuddyColor(int n, int color);
EXTERNC int setCtlBuddyMaxSize(int n, int maxwidth, int maxheight);

EXTERNC int setCtlEditColor(int n, int color);
EXTERNC int setCtlEditTextColor(int n, int color);

EXTERNC int getSysColor(int n);

EXTERNC int getSysMem(int n);

EXTERNC int checkArrayAlloc(int nr);

EXTERNC INT_PTR getArrayAddress(int nr);

EXTERNC INT_PTR getBufferAddress(int nr);


EXTERNC int setCtlScripting (int n, int state);

EXTERNC int checkScriptVal(int n);

EXTERNC int enableScriptVal(int n, int state);

EXTERNC int getScriptVal(int n);

EXTERNC int setScriptVal(int n, int val);

EXTERNC int cnvX(int k, int off, int d,
                   INT_PTR pGetf, int x, int y, int z);

EXTERNC int cnvY(int k, int off, int d,
                   INT_PTR pGetf, int x, int y, int z);
                   
EXTERNC int fm_abort(void);

EXTERNC int testAbort(void);

EXTERNC int updatePreview(int n);

EXTERNC int updateProgress(int n, int d);


/**********************************************************************/
/* getFilterInstallDir()
/*
/*  Returns a string containing the full path of the directory from
/*  which this filter was loaded (which is presumably the installation
/*  directory).
/*
/*  As a side effect, this function also copies the path name into
/*  the predefined string filterInstallDir[256].
/*
/**********************************************************************/
EXTERNC char *getFilterInstallDir(void);

/**********************************************************************/
/* appendEllipsis(s)
/*
/*  Appends an ellipsis (...) to a string.
/*
/*  Caution: The string returned by appendEllipsis() resides in static
/*           storage and will be overwritten by the next call to
/*           appendEllipsis().  If you need the appended string to
/*           persist, copy it to your own storage.
/*
/**********************************************************************/
EXTERNC char *appendEllipsis(const char *s);

/**********************************************************************/
/* stripEllipsis(s)
/*
/*  Strips a trailing ellipsis (if any) from a string.
/*
/*  Caution: The string returned by stripEllipsis() resides in static
/*           storage and will be overwritten by the next call to
/*           stripEllipsis().  If you need the stripped string to
/*           persist, copy it to your own storage.
/*
/**********************************************************************/
EXTERNC char *stripEllipsis(const char *s);

/**********************************************************************/
/* formatString(s)
/*
/*  Formats a string by interpolating the following special codes:
/*
/*       !A = Author (filterAuthorText)
/*       !a = About (filterAboutText)
/*       !C = Category (filterCategoryText)
/*       !c = Copyright (filterCopyrightText)
/*       !D = Description (filterDescriptionText)
/*   **  !d = Document title (from propTitle (titl)) -> documentTitleText
/*   XX  !d = current date (oops, used above)
/*       !F = Filename (filterFilenameText)
/*       !f = Filter case (filterCaseText)
/*       !H = Host (filterHostText)
/*       !h = Image height (imageHeight)
/*   **  !I = filterInstallDir
/*   **  !i = filterImageTitle (same as documentTitleText ??)
/*       !M = Image Mode (filterImageModeText)
/*       !m = Image Mode (numeric, from imageMode)
/*       !O = Organization (filterOrganizationText)
/*   **  !S = Serial string (hostSerialString)
/*   **  !s = Binary serial number (hostSerialNumber)
/*       !T = Title (filterTitleText)
/*       !t = Title with any trailing ellipsis stripped
/*   XX  !t = current time (oops, used above)
/*       !U = URL (filterURLText)
/*       !V = Version (filterVersionText)
/*   **  !v = FilterMeister Version string???
/*       !w = Image width (imageWidth)
/*   **  !Y - current year (e.g. 2003) -- for use in Copyright
/*       !z = Proxy zoom factor (zoomFactor)
/*       !! = !
/*
/* ** - Not yet implemented.
/*
/*  Also translates many of the latest HTML entity strings to
/*  their corresponding ISO codes (or sometimes fakes the
/*  translation by using low-valued ASCII codes or a combination
/*  of ASCII characters).  An HTML entity string has the general
/*  format '&entity;'.  It must begin with the ampersand character (&)
/*  and end with a semicolon (;). The string 'entity' between the
/*  & and the ; must be a sequence of two or more ASCII lowercase
/*  or uppercase letters (the string is case sensitive).
/*
/*  HTML numeric character references (both decimal and hexadecimal)
/*  are also recognized and translated.  The decimal format is
/*  '&#D;', where D is a decimal number which refers to the ISO 10646 
/*  decimal character number D.  The hexadecimal syntax is '&#xH;' or 
/*  '&#XH;,, where H is a hexadecimal number which refers to the ISO 
/*  10646 hexadecimal character number H.  Hexadecimal numbers in 
/*  numeric character references are case-insensitive. 
/*
/*  Here are some examples of numeric character references:
/*
/*	&#229;		(in decimal) represents the letter "a" with a small circle 
/*				above it (used, for example, in Norwegian). 
/*	&#xE5;		(in hexadecimal) represents the same character. 
/*	&#Xe5;		(in hexadecimal) represents the same character as well. 
/*	&#1048;		(in decimal) represents the Cyrillic capital letter "I". 
/*	&#x6C34;	(in hexadecimal) represents the Chinese character for water. 
/*
/*  As examples of HTML character entity references, FM will translate the 
/*  following HTML entities as indicated:
/*
/*      Entity  Translation Meaning
/*      ------  ----------- ------------------------------------
/*		&quot;	    "       double quote
/*		&amp;	    &       ampersand
/*		&apos;	    '       apostrophe
/*		&lt;	    <       less than
/*		&gt;	    >       greater than
/*		&nbsp;		        non-breaking space
/*		&iexcl;		¡       inverted exclamation mark
/*      &iquest;    ¿       inverted question mark
/*		&cent;		¢       cent sign
/*		&pound;		£       pound sign
/*		&copy;		©       copyright
/*      &reg;       ®       registered trademark
/*		&trade;     tm      trademark
/*		&yen;		¥       Yen sign
/*      &euro;      €       Euro sign
/*      
/*  For a complete list of all HTML entities currently translated by 
/*  FM, see the code below.
/*
/*  Note that, in order to represent a bare ampersand (&), you can
/*  code the entity string '&amp;'.  Since this is rather cumbersome,
/*  FM will also recognize and translate two successive ampersands
/*  (&&) into a single ampersand (&) in the formatted string.
/*  Furthermore, any occurrence of an ampersand (&) that does not
/*  initiate a recognized HTML entity will also be translated as
/*  a bare ampersand (&).
/*
/*  For example, the following strings:
/*
/*      "This is an ampersand:'&amp;'; &amp; so is this: &amp;."
/*      "This is an ampersand:'&&'; && so is this: &&."
/*      "This is an ampersand:'&'; & so is this: &."
/*
/*  will all be formatted as:
/*
/*      "This is an ampersand:'&'; & so is this: &."
/*
/*  For possible future implementation:
/*
/*       !<n> = value of ctl(<n>) where <n> is a decimal number
/*              (e.g., for updating a static text control?)
/*
/*  Note: The formatting is nonrecursive, to prevent inadvertent
/*        infinite loops.
/*
/*  Caution: The string returned by formatString() has a maximum
/*           length of 1024 chars (not including the terminating
/*           NUL), and will be truncated if necessary.  It resides 
/*           in (thread-local) static storage and will be overwritten
/*           by the next call to formatString() in the current thread.
/*           If you need the formatted string to persist, copy it to 
/*           your own storage.
/*
/**********************************************************************/

EXTERNC char *formatString(const char *s);


EXTERNC char *getCtlText(int n);


EXTERNC void lockCtlUpdate (int state);


EXTERNC int setCtlLabel(int n, char * iText);


/**********************************************************************/
/* setCtlText(n, "text")
/*
/*  Sets the text label or content for control n to the given text string.
/*
/**********************************************************************/

EXTERNC int setCtlText(int n, char* iText);

/**********************************************************************/
/* setCtlTextv(n, "format",...)
/*
/*  Sets the text label or content for control n to the expanded
/*  printf-style format string.
/*
/**********************************************************************/
EXTERNC int setCtlTextv(int n, char* iFmt,...);


/**********************************************************************/
/* enableToolTipBalloon(enable)
/*
/*  Enables or disables the balloon shape of all tool tips, based on
/*  the value of boolean value "enable".
/*
/**********************************************************************/

EXTERNC int enableToolTipBalloon(int enable);

// NOTE: Shouldn't we have a version of SetCtlToolTip with printf-style
//       formatting?  E.g., setCtlToolTipv(n, s, "format",...)

/**********************************************************************/
/* setCtlToolTip(n, "Tool Tip Text", s)
/*
/*  Sets the tool tip text for control n to the specified string, with
/*  style s (TTF_CENTERTIP, TTF_RTLREADING, TTF_TRACK, TTF_ABSOLUTE,
/*  TTF_TRANSPARENT), or deletes the tool tip for this control if the
/*  text argument is NULL or "".
/*
/**********************************************************************/
EXTERNC int setCtlToolTip(int n, char * iText, int s);


EXTERNC int setToolTipDelay(int m, int iTime);



EXTERNC char * getCtlImage(int n);


/**********************************************************************/
/* setCtlImage(n, "Image Name", 'X')
/*
/*  Sets the image (if any) associated with control n to the specified
/*  filename or embedded resource name, with image type 'X', where
/*  'X' is one of the following character constants:
/*
/*      'B' -   bitmap file (.bmp)
/*      'W' -   Windows (old-style) metafile (.wmf)
/*      'E' -   enhanced metafile (.emf)
/*      'I' -   icon file (.ico)
/*      'C' -   cursor file (.cur)
/*      'J' -   JPEG file (.jpg)
/*      'G' -   GIF file (.gif)
/*      'M' -   MIG (mouse-ivo graphics) file (.mig)
/*       0  -   unspecified file type
/*
/**********************************************************************/
EXTERNC int setCtlImage(int n, char * iName, int iType);

/**********************************************************************/
/* setCtlTicFreq(n, m)
/*
/*  Sets the frequency with which tick marks are displayed for slider
/*  control n. For example, if the frequency is set to 2, a tick mark
/*  is displayed for every other increment in the slider’s range. The
/*  default setting for the frequency is 1 (that is, every increment 
/*  in the range is associated with a tick mark).
/*
/**********************************************************************/
EXTERNC int setCtlTicFreq(int n, int m);

/**********************************************************************/
/* setWindowTheme(hwnd, pszSubAppName, pszSubIdList)
/*
/*  Sets the Visual Theme for a given window to the specified theme.
/*
/*  Calls the following XP API function (if it exists):
/*
/*  THEMEAPI SetWindowTheme(HWND hwnd, LPCWSTR pszSubAppName, 
/*    LPCWSTR pszSubIdList);
/*
/*  N.B.  The 2nd and 3rd args must be UNICODE strings!
/**********************************************************************/

//Internal Theme Variable -> theme off by default -> replaced by gParams->gDialogTheme
//int DefaultTheme=0;

// EXTERNC int drawThemePart(int n, RECT * rc, LPCWSTR topic, int part, int state);


/**********************************************************************/
/* setDefaultWindowTheme(hwnd)
/*
/*  Sets the Visual Theme for a given window to the default visual
/*  theme, which for now means to turn OFF any XP Visual Styles.
/*
/**********************************************************************/
// EXTERNC HRESULT setDefaultWindowTheme(HWND hwnd);


/**********************************************************************/
/* setCtlTheme(n, pszSubAppName, pszSubIdList)
/*
/*  Sets the Visual Theme for control n to the specified theme.
/*
/**********************************************************************/
//int setCtlTheme(int n, int pszSubAppName, int pszSubIdList) //Removed  infront of setCtlTheme



//New Styles Functions


EXTERNC int getAppTheme (void);

// EXTERNC int setDialogTheme (int state);

// EXTERNC int setCtlTheme(int n, int state);

//int setDialogEvent (int state);
//int clearDialogEvent (int state);



EXTERNC int createFont(int i, int size, int bold, int italic, char * fontname);

EXTERNC int deleteFont (int i);

EXTERNC int setCtlFont(int n, int i);



/**********************************************************************/
/* setCtlFontColor(n, color)
/*
/*  Sets the text color for control n to the specified RGB-triple value.
/*
/**********************************************************************/
EXTERNC int setCtlFontColor(int n, int color);

EXTERNC int getCtlFontColor(int n);

/**********************************************************************/
/* setCtlColor(n, color)
/*
/*  Sets the background color of control n to the specified RGB-triple.
/*  (This actually has no effect for many control styles; the effect is
/*  specific to each control style, and will be specified under the
/*  description for each style.)
/*
/*        Color = -1 means transparent.
/*
/**********************************************************************/
EXTERNC int setCtlColor(int n, int color);


EXTERNC int setCtlSysColor(int n, int con);

EXTERNC int setCtlFontSysColor(int n, int con);


/**********************************************************************/
/* getCtlColor(n)
/*
/*  Returns the current background color of control n as an RGB-triple,
/*  or -1 if n is not a valid control.
/*
/**********************************************************************/
EXTERNC int getCtlColor(int n);


EXTERNC int setCtlDefVal(int n, int defval);
EXTERNC int getCtlDefVal(int n);

/**********************************************************************/
/* setCtlVal(n, val)
/*
/*  Sets the value of control n to val.
/*
/*  Returns the previous value of control n, or -1 if n is not a
/*  valid control.
/*
/**********************************************************************/
EXTERNC int setCtlVal(int n, int val);


EXTERNC int setCtlDivisor(int n, int div);

EXTERNC int getCtlDivisor(int n);

EXTERNC int setCtlGamma(int n, int gamma);

EXTERNC int setCtlProperties(int n, int props);

EXTERNC int clearCtlProperties(int n, int props);

EXTERNC int setCtlRange(int n, int lo, int hi);

/**********************************************************************/
/* setCtlLineSize(n, a)
/*
/*  Sets the small step ("line size") value for control n to a, and
/*  returns the previous small step value.
/*
/**********************************************************************/
EXTERNC int setCtlLineSize(int n, int a);

/**********************************************************************/
/* setCtlPageSize(n, b)
/*
/*  Sets the large step ("page size") value for control n to b, and
/*  returns the previous large step value.
/*
/**********************************************************************/
EXTERNC int setCtlPageSize(int n, int b);

EXTERNC int setCtlThumbSize(int n, int a);


/*************************************************************/
//
//  A couple of local helper functions...
//
/*************************************************************/

EXTERNC void computeBuddy1Pos(int n, int x, int y, int w, int h, int xx, int yy, RECT *pr);

EXTERNC void computeBuddy2Pos(int n, int x, int y, int w, int h, RECT *pr);



/**********************************************************************/
/* setCtlPos(n, x, y, w, h)
/*
/*  Sets the position of the upper lefthand corner of control n to 
/*  coordinates (x,y) within the client area of the dialog box; sets 
/*  the width of the control to w and the height to h.  All measurements 
/*  are in dialog box units (DBUs).  To leave a particular parameter
/*  unchanged, specify a value of -1 for that parameter.
/*
/**********************************************************************/
EXTERNC int setCtlPos(int n, int x, int y, int w, int h);

EXTERNC int getCtlDefPos(int n, int item);
EXTERNC int setCtlDefPos(int n, int x, int y, int width, int height);

EXTERNC int setCtlPixelPos(int n, int x, int y, int w, int h);
EXTERNC int getCtlPixelPos(int n, int w);

EXTERNC int setCtlEditSize(int n, int w, int h);
EXTERNC int setEditBoxUpdate(int ms);

EXTERNC int getCtlRange(int n, int w);



/**********************************************************************/
/* setCtlAction(n, a)
/*
/*  Sets the default action for control n to a, where a is CA_NONE,
/*  CA_CANCEL, CA_APPLY, CA_PREVIEW, CA_EDIT, CA_ABOUT, or CA_RESET.
/*  Returns the previous default action for the control (or 0 if the
/*  control index is invalid).
/*
/*  The default action is the action that will be taken if the default
/*  OnCtl handler is invoked for control n.  The meaning of each action
/*  is:
/*
/*  CA_NONE     No action.  This is the default action for radio buttons.
/*
/*  CA_CANCEL   FM exits, leaving the original source image unaltered.
/*              This is the default action for the Cancel button,
/*
/*  CA_APPLY    The filter is applied to the original source image,
/*              and FM exits.  This is the default action for the OK
/*              button.
/*
/*  CA_PREVIEW  The filter is applied to the proxy image, and all previews
/*              are updated.  This is the default action for most user
/*              controls, including the STANDARD, SCROLLBAR, TRACKBAR,
/*              PUSHBUTTON, and CHECKBOX controls.
/*
/*  CA_EDIT     FM enters or leaves Edit Mode.  This is the default
/*              action for the Edit control.
/*
/*  CA_ABOUT    FM displays the ABOUT dialog box.
/*
/*  CA_RESET    Resets all user controls to their initial values.
/*
/**********************************************************************/
EXTERNC int setCtlAction(int n, int a);

/**********************************************************************/
/* enableCtl(n, level)
/*
/*  Determines whether control n is visible and whether it is enabled
/*  by specifying level = 0 (invisible and disabled), 1 (visible but
/*  disabled), or 3 (visible and enabled).  Returns the previous enable
/*  level, or 0 if n is not a valid control index.
/*
/*  NOTE: Due to an early documentation error, level=-1 is considered
/*        equivalent to level=3.  However, other bits in level are
/*        reserved for future use, and should not be randomly set!
/*
/**********************************************************************/
EXTERNC void setEnableMode(int state);
EXTERNC int enableCtl(int n, int level);

/**********************************************************************/
/* enableCtl(n) support function
/*
/*  Determines whether a control should be visible, disabled or 
/*	invisible and sets it as such. Returns 0.
/*
/**********************************************************************/
EXTERNC int sub_enableCtl(int n);

/**********************************************************************/
/* deleteCtl(n)
/*
/*  Deletes control n.
/*
/**********************************************************************/
EXTERNC int deleteCtl(int n);

/**********************************************************************/
/* createPredefinedCtls()
/*
/*  Creates the pre-defined user controls.
/*
/**********************************************************************/
EXTERNC void createPredefinedCtls(void);

/**********************************************************************/
/* resetAllCtls()
/*
/*  Resets all controls (deletes them).
/*
/**********************************************************************/
EXTERNC int resetAllCtls(void);

/**********************************************************************/
/* createCtl(n, c, t, x, y, w, h, s, sx, p, e)
/*
/*  Dynamically creates a control with index n, class c, text t,
/*  coordinates (x,y), width w, height h, style s, extended style sx,
/*  properties p, and enable level e.
/*  All measurements are in DBUs.
/*  For x, y, w, and h, a value of -1 means use the default value.
/*
/**********************************************************************/
EXTERNC int createCtl(int n, int c, char * t, int x, int y, int w, int h,
                        int s, int sx, int p, int e);

/**********************************************************************/
/* initCtl(n)
/*
/*  Initializes control n to default values.
/*
/**********************************************************************/
EXTERNC int initCtl(int n);


/////// BUGNOTE:
/////// Should not use negative x, y coords as flags, since they
/////// may legitimately be negative; e.g., in a multi-monitor
/////// environment.  !!!!!!! ?????? !!!!!!

/**********************************************************************/
/*  setDialogPos(fAbs, x, y, w, h)
/*
/*  Sets the position and size of the dialog window.  If fAbs is true,
/*  x and y are absolute screen coordinates; otherwise, x and y are
/*  relative to the upper-left corner of the client area in the host
/*  application's main window.  If x and y are set to -1, the dialog
/*  window will be centered within the host client area or the working
/*  area of the entire screen, depending on whether fAbs is false or
/*  true, respectively.  Otherwise, if either x or y is negative, the
/*  position of the dialog will not be changed. If w or h is negative,
/*  the size of the dialog window will not be changed.
/*
/*  All measurements are in dialog box units (DBUs).
/*
/**********************************************************************/
EXTERNC int setDialogPos(int fAbs, int x, int y, int w, int h);


/**********************************************************************/
/* setDialogMaxMin()
/*  Set the minum and maximum size of the dialog
/*
/**********************************************************************/
EXTERNC int setDialogMinMax(int mintrackX, int mintrackY, int maxtrackX, int maxtrackY);



EXTERNC int getDialogPos(int w, int t);


/**********************************************************************/
/* getDialogWidth()
/*  Gets the width of the client area of the dialog.
/*
/**********************************************************************/
EXTERNC int getDialogWidth(void);

/**********************************************************************/
/* getDialogHeight()
/*  Gets the height of the client area of the dialog.
/*
/**********************************************************************/
EXTERNC int getDialogHeight(void);



/**********************************************************************/
/* setDialogStyle(TitleBar |...)
/*  Sets various styles of the dialog.
/*
/**********************************************************************/
EXTERNC int setDialogStyle(int flags);

/**********************************************************************/
/* clearDialogStyle(TitleBar |...)
/*  Clears various styles of the dialog.
/*
/**********************************************************************/
EXTERNC int clearDialogStyle(int flags);

// !!!! Should recalc Edit-mode dialog size when TOOLWINDOW is set/cleared
// !!!! and when dialog is resized!!!!


/**********************************************************************/
/* setDialogStyleEx(ToolWindow |...)
/*  Sets various extended styles of the dialog.
/*
/**********************************************************************/
EXTERNC int setDialogStyleEx(int flags);

/**********************************************************************/
/* clearDialogStyleEx(ToolWindow |...)
/*  Clears various extended styles of the dialog.
/*
/**********************************************************************/
EXTERNC int clearDialogStyleEx(int flags);

/**********************************************************************/
/* setDialogText("title")
/*  Sets the caption in the title bar.
/*
/**********************************************************************/
EXTERNC int setDialogText(char * title);

/**********************************************************************/
/* setDialogTextv("format", ...)
/*  Sets the caption in the title bar with printf-style formatting.
/*
/**********************************************************************/
EXTERNC int setDialogTextv(char * iFmt,...);

/**********************************************************************/
/* setDialogGradient(color1,color2,direction)
/*  Fills the background of the client area of the dialog box with
/*  a vertical (direction=0) or horizontal (direction=1) linear gradient.
/*
/**********************************************************************/
EXTERNC int setDialogGradient(int color1, int color2, int direction);

/**********************************************************************/
/* setDialogColor(color)
/*  Sets the background color for the client area of the dialog box.
/*
/**********************************************************************/
EXTERNC int setDialogColor(int color);

EXTERNC int getDialogColor();

/**********************************************************************/
/* setDialogImage(filename)
/*  Applies an image from the specified file as the background image
/*  for the client area of the dialog box, tiling or stretching it to
/*  fit if requested by setDialogImageMode.
/*
/**********************************************************************/
EXTERNC int setDialogImage(char * filename);

/**********************************************************************/
/* setDialogImageMode({EXACT==0|TILED==1|STRETCHED==2}, <stretch_mode>)
/*  Specifies whether the background image is to the tiled or
/*  stretched to fill the entire dialog, or used exactly as is.
/*  If the image is to be stretched, then the stretch mode is
/*  also specified.
/*
/**********************************************************************/
EXTERNC int setDialogImageMode(int mode, int stretchMode);

/**********************************************************************/
/* setDialogDragMode({TitleBar==0|Background==1|None==2})
/*  Determines whether the filter dialog box can be dragged by the
/*  title bar only (0), by dragging the title bar or anywhere on the
/*  dialog background (1), or not at all (2).  The default mode is 0.
/**********************************************************************/
EXTERNC int setDialogDragMode(int mode);

/**********************************************************************/
/* setDialogRegion(<region_expression>)
/*  Sets the clipping region (outline) of the filter dialog.
/*
/**********************************************************************/
EXTERNC int setDialogRegion(INT_PTR rgn);

/**********************************************************************/
/* REGION createRectRgn(xLeft, yTop, xRight, yBottom)
/*  Creates a simple rectangular region, with coordinates given in DBUs
/*  relative to the upper-left corner of the dialog box.  <left> is the
/*  x-coordinate of the left edge of the rectangle; <right> is the
/*  x-coordinate of the right edge.  <top> is the y-coordinate of the
/*  top edge of the rectangle; <bottom> is the y-coordinate of the
/*  bottom edge.  The <left> and <top> coordinates are inclusive (i.e.,
/*  pixels at these coordinates are included in the rectangle).  The
/*  <right> and <bottom> coordinates are exclusive (i.e., pixels up to
/*  but *not* including these coordinates are part of the rectangle).
/*
/**********************************************************************/
EXTERNC INT_PTR createRectRgn(int xLeft, int yTop, int xRight, int yBottom);

/**********************************************************************/
/* REGION createRoundRectRgn(xLeft, yTop, xRight, yBottom, eWidth, eHeight)
/*  Creates a rectangular region with rounded corners.  <left>, <top>,
/*  <right>, and <bottom> are the same as for 'RECT' and describe the
/*  unrounded rectangle.  <width> and <height> specify the width and
/*  height of an ellipse; quarter sections of this ellipse are used to
/*  form the rounded corners of the rectangle.  All measurements are
/*  in DBUs.
/*
/**********************************************************************/
EXTERNC INT_PTR createRoundRectRgn(int xLeft, int yTop, int xRight, int yBottom,
                                 int eWidth, int eHeight);

/**********************************************************************/
/* REGION createCircularRgn(xLeft, yTop, iDiameter)
/*  Creates a simple circular region.  <left> and <top> give the x- and
/*  y-coordinates of the upper-left corner of the bounding box for the
/*  circle.  <diameter> gives the diameter of the circle.  All measurements
/*  are in DBUs.
/*
/**********************************************************************/
EXTERNC INT_PTR createCircularRgn(int xLeft, int yTop, int iDiameter);

/**********************************************************************/
/* REGION createEllipticRgn(xLeft, yTop, xRight, yBottom)
/*  Creates an elliptical region.  <left>, <top>, <right>, and <bottom>
/*  define the coordinates of the bounding box for the ellipse.  All
/*  measurements are in DBUs.
/*
/**********************************************************************/
EXTERNC INT_PTR createEllipticRgn(int xLeft, int yTop, int xRight, int yBottom);

/**********************************************************************/
/* REGION createPolyRgn(fillMode, nPoints, x1, y1, x2, y2, x3, y3, ...)
/*  Creates an arbitrary polygon from a list of pairs of vertices.  This
/*  list must consist of at least 3 pairs of x- and y-coordinates (in
/*  DBUs).  The total number of coordinates must be even.  The polygon is
/*  defined by drawing an imaginary line from the first vertex to the
/*  second, then to each subsequent vertex, and finally back to the
/*  original vertex.  The region is then defined by "filling" this polygon
/*  according to the fill mode.  The fill mode must be specified as either
/*  'ALTERNATE' (1) or 'WINDING' (2).
/*
/**********************************************************************/
EXTERNC INT_PTR createPolyRgn(int fillMode, int nPoints, int x1, int y1,
                            int x2, int y2, int x3, int y3, ...);

/**********************************************************************/
/* REGION createDialogRgn();
/*  Is a predefined rectangular region consisting of the default dialog
/*  box itself.
/*
/**********************************************************************/
EXTERNC int createDialogRgn(void);

/**********************************************************************/
/* REGION createTitlebarRgn();
/*  Is a predefined rectangular region defining the area of the dialog
/*  title bar (if present).
/*
/**********************************************************************/
EXTERNC int createTitlebarRgn(void);

/**********************************************************************/
/* REGION createMenubarRgn();
/*  Is a predefined rectangular region defining the area of the dialog
/*  menu bar (if any).
/*
/**********************************************************************/
EXTERNC int createMenubarRgn(void);

/**********************************************************************/
/* REGION createClientRgn();
/*  Is a predefined rectanglar region defining the client area of the
/*  dialog box.
/*
/**********************************************************************/
EXTERNC int createClientRgn(void);

/**********************************************************************/
/* REGION createStatusbarRgn();
/*  Is a predefined rectangular region defining the area of the dialog
/*  status bar (if any).
/*
/**********************************************************************/
EXTERNC int createStatusbarRgn(void);

/**********************************************************************/
/* BOOL playSoundWave(szWaveFile);
/*  Plays the specified wave file asynchronously.  Looks first for
/*  an embedded WAVE resource with the specified name; else searches
/*  the usual paths for the specified wave file.
/*
/**********************************************************************/
EXTERNC BOOL playSoundWave(INT_PTR szWaveFile);

/**********************************************************************/
/* BOOL playSoundWaveLoop(szWaveFile);
/*  Plays the specified wave file repeatedly.  Looks first for
/*  an embedded WAVE resource with the specified name; else searches
/*  the usual paths for the specified wave file.
/*
/**********************************************************************/
BOOL playSoundWaveLoop(INT_PTR szWaveFile);

/**********************************************************************/
/* BOOL playSoundWaveSync(szWaveFile);
/*  Plays the specified wave file synchronously.  Looks first for
/*  an embedded WAVE resource with the specified name; else searches
/*  the usual paths for the specified wave file.
/*
/**********************************************************************/
EXTERNC BOOL playSoundWaveSync(INT_PTR szWaveFile);

EXTERNC int fm_sleep(int msecs);

EXTERNC int fm_RGB(int r, int g, int b) ;
    
EXTERNC int RGBA(int r, int g, int b, int a) ;
    
EXTERNC int Rval(int rgba) ;
    
EXTERNC int Gval(int rgba) ;
    
EXTERNC int Bval(int rgba) ;
    
EXTERNC int Aval(int rgba);

/**********************************************************************/
/* iError = getOpenFileName( flags,
/*                     lpstrFile, nMaxFile,
/*                     [&nFIleOffset], [&nFileExtension],
/*                     [lpstrFileTitle], nMaxFileTitle,
/*                     [lpstrFilter], 
/*                     [lpstrCustomFilter], nMaxCustFilter,
/*                     [&nFilterIndex],
/*                     [lpstrInitialDir],
/*                     [lpstrDialogTitle],
/*                     [lpstrDefExt],
/*                     [&oflags]
/*                   );
/*
/*  Invokes the Open file common dialog.
/*
/**********************************************************************/
EXTERNC int getOpenFileName( int flags,
                     LPSTR lpstrFile, int nMaxFile,
                     int *pnFileOffset, int *pnFileExtension,
                     LPSTR lpstrFileTitle, int nMaxFileTitle,
                     LPCSTR lpstrFilter,
                     LPSTR lpstrCustomFilter, int nMaxCustFilter,
                     int *pnFilterIndex,
                     LPCSTR lpstrInitialDir,
                     LPCSTR lpstrDialogTitle,
                     LPCSTR lpstrDefExt,
                     int *pOflags
                   );

/**********************************************************************/
/* iError = getSaveFileName( flags,
/*                     lpstrFile, nMaxFile,
/*                     [&nFIleOffset], [&nFileExtension],
/*                     [lpstrFileTitle], nMaxFileTitle,
/*                     [lpstrFilter], 
/*                     [lpstrCustomFilter], nMaxCustFilter,
/*                     [&nFilterIndex],
/*                     [lpstrInitialDir],
/*                     [lpstrDialogTitle],
/*                     [lpstrDefExt],
/*                     [&oflags]
/*                   );
/*
/*  Invokes the Save As... file common dialog.
/*
/**********************************************************************/
EXTERNC int getSaveFileName( int flags,
                     LPSTR lpstrFile, int nMaxFile,
                     int *pnFileOffset, int *pnFileExtension,
                     LPSTR lpstrFileTitle, int nMaxFileTitle,
                     LPCSTR lpstrFilter,
                     LPSTR lpstrCustomFilter, int nMaxCustFilter,
                     int *pnFilterIndex,
                     LPCSTR lpstrInitialDir,
                     LPCSTR lpstrDialogTitle,
                     LPCSTR lpstrDefExt,
                     int *pOflags
                   );


EXTERNC int chooseColor2(int n, int initialColor, int ctlPreview, int language);

/**********************************************************************/
/* chooseColor(initialColor, szPrompt, ...)
/*
/*  Invokes the host app's color picker to choose a color.
/*  Returns the chosen color, or -1 if an error occurred.
/*
/**********************************************************************/
EXTERNC int chooseColor(int initialColor, const char *szPrompt, ...);

/**********************************************************************/
/* setRegRoot(int hkey)
/*  Sets the current registry root key.
/*
/**********************************************************************/
EXTERNC int setRegRoot(HKEY hkey);

/**********************************************************************/
/* getRegRoot(int *hkey)
/*  Retrieves the current registry root key.
/*
/**********************************************************************/
EXTERNC int getRegRoot(HKEY *phkey);

/**********************************************************************/
/* setRegPath(lpsz szPath[, varargs]...)
/*  Sets the current registry path, with printf-style formatting.
/*  Returns ERROR_INVALID_DATA if expanded path string is too long;
/*  else returns ERROR_SUCCESS.
/*
/**********************************************************************/
EXTERNC int setRegPath(LPCSTR szPath,...);

/**********************************************************************/
/* getRegPath(lpsz szPath, int maxPathLen)
/*  Retrieves the current (not yet interpolated) registry path.
/*  Returns ERROR_INVALID_DATA if caller's buffer is too small;
/*  else returns ERROR_SUCCESS.
/*
/**********************************************************************/
EXTERNC int getRegPath(LPSTR szPath, int maxPathLen);

/**********************************************************************/
/* putRegInt(int iValue, lpsz szValueName[, varargs]...)
/*  Stores an integer value into the registry, with printf-style formatting
/*  for the value name.
/*
/**********************************************************************/
EXTERNC int putRegInt(int iValue, LPCSTR szValueName,...);

/**********************************************************************/
/* getRegInt(int *iValue, lpsz szValueName[, varargs]...)
/*  Gets an integer value from the registry, with printf-style formatting
/*  for the value name.
/*
/**********************************************************************/
EXTERNC int getRegInt(int *iValue, LPCSTR szValueName,...);

/**********************************************************************/
/* putRegString(lpsz szString, lpsz szValueName[, vargargs]...)
/*  Stores a string value into the registry, with printf-style formatting
/*  for the value name.
/*
/**********************************************************************/
EXTERNC int putRegString(LPCSTR szString, LPCSTR szValueName,...);

/**********************************************************************/
/* getRegString(lpsz szString, int iMaxlen, lpsz szValueName[, varargs]...)
/*  Gets a string value from the registry, with printf-style formatting
/*  for the value name.
/*
/**********************************************************************/
EXTERNC int getRegString(LPSTR szString, int iMaxlen, LPCSTR szValueName,...);

/**********************************************************************/
/* putRegData(void *data, int dataLen, lpsz szValueName[, vargargs]...)
/*  Stores a set of binary data into the registry, with printf-style
/*  formatting for the value name.
/*
/**********************************************************************/
EXTERNC int putRegData(const void *pData, int dataLen, LPCSTR szValueName,...);

/**********************************************************************/
/* getRegData(void *data, int dataLen, lpsz szValueName[, vargargs]...)
/*  Gets a set of binary data from the registry, with printf-style
/*  formatting for the value name.
/*
/**********************************************************************/
EXTERNC int getRegData(void *pData, int dataLen, LPCSTR szValueName,...);

/**********************************************************************/
/* enumRegSubKey(int index, lpsz szSubKey, int maxSubKeyLen)
/*  Gets name of n-th subkey under current registry key.
/*
/**********************************************************************/
EXTERNC int enumRegSubKey(int index, LPSTR szSubKey, int maxSubKeyLen);

/**********************************************************************/
/* enumRegValue(int index, lpsz szValueName, int maxValueNameLen,
/*              int *iType, int *cbData)
/*  Gets name, type, and data size of n-th value under current registry
/*  key.
/*
/**********************************************************************/
EXTERNC int enumRegValue(int index, LPSTR szValueName, int maxValueNameLen,
                    int *iType, int *cbData);

/**********************************************************************/
/* deleteRegSubKey(lpsz szSubKey[, varargs]...)
/*  Deletes specified subkey (and all its values) below the current
/*  key.  The subkey name is specified with printf-style formatting.
/*  The specified subkey must not have sub-subkeys (at least on NT;
/*  Win95 may allow deletion of an entire key subtree?).
/*
/**********************************************************************/
EXTERNC int deleteRegSubKey(LPCSTR szSubKey,...);

/**********************************************************************/
/* deleteRegValue(lpsz szValueName[, vargargs]...)
/*  Deletes specified value under the current key.  The value name is
/*  specified with printf-style formatting.
/*
/**********************************************************************/
EXTERNC int deleteRegValue(LPCSTR szValueName,...);


EXTERNC int shellExec(char* szVerb, char* szFilename, char* szParams, char* szDefDir);

EXTERNC int msgBox(UINT uType, LPCTSTR lpCaption, LPCTSTR lpFmt,...);


EXTERNC int fm_printf(char* lpFmt,...);

EXTERNC int Info(char* lpFmt,...);

EXTERNC int Warn(char* lpFmt,...);

EXTERNC int Error(char* lpFmt,...);

EXTERNC int ErrorOk(char* lpFmt,...);

EXTERNC int YesNo(char* lpFmt,...);

EXTERNC int YesNoCancel(char* lpFmt,...);



#ifdef FMDIB
    EXTERNC int startSetPixel (int n);
	EXTERNC int startSetPixelSS (int n, int factor);
    EXTERNC int endSetPixel (int n);
	EXTERNC int startSetPixelDC(HDC dc);
	EXTERNC int endSetPixelDC(HDC dc);
    EXTERNC int setPixel (int x, int y, UINT color);
	EXTERNC UINT getPixel (int x, int y);
    EXTERNC int setFill(int n, UINT color);
	EXTERNC int setPenWidth(int width);
	EXTERNC int setPenStyle(int e);
	EXTERNC int setLine(int start_x, int start_y, int end_x, int end_y, UINT color);
	EXTERNC int setLineAA(int start_x, int start_y, int end_x, int end_y, UINT color);
	EXTERNC int setRectFill(int left, int top, int right, int bottom, UINT color);
	EXTERNC int setRectFrame(int left, int top, int right, int bottom, UINT color);
	EXTERNC int setRectGradient(int left, int top, int right, int bottom, UINT color_TL, UINT color_BR, int horizontal);
	EXTERNC int setRadialGradient(int x0, int y0, int x1, int y1, int r, COLORREF c0, COLORREF c1, int nPart);
	EXTERNC int setEllipse(int left, int top, int right, int bottom, UINT color);
	EXTERNC int setEllipseFill(int left, int top, int right, int bottom, UINT color);
    EXTERNC HFONT setFont(int size, int angle, int bold, int italic, char * fontname);
	EXTERNC int setText(int x, int y, UINT color, int alignment, char * text);
	EXTERNC int setTextv(int x, int y, UINT color, int alignment, char * lpFmt, ...);
    EXTERNC int drawText (int x, int y, char* text);
#endif



#include <time.h>


EXTERNC int startClock();
EXTERNC int stopClock();

EXTERNC int convertColor(int16 sourceSpace, int resultSpace, FilterColor color);

EXTERNC int getProperty(int property, int notsupported);

EXTERNC int setCtlClass(int iCtl, int iClass, int absy, int height, int thumbSize);

EXTERNC int isValidFilename (char * filename);

EXTERNC int convertToFilename (char * filename);

EXTERNC int stripEndSpaces(char * string);

EXTERNC int toLowerCase(char * str);

EXTERNC int retrieveFilename(char * path, char * filename);
EXTERNC int retrieveFilenameNoExt(char * path, char * filename);
EXTERNC int retrieveFolder(char * path, char * folder);

EXTERNC int fileExists(char *fileName);

//int addKeyShortcut (int n, int key, int fVirt);

EXTERNC int getCtlItemString(int n, int item, char * string);

EXTERNC int gammaCtlVal(int iCtl, int val, int invert);

EXTERNC void drawPreviewColor(HDC hDC);
EXTERNC void drawSizeGrip();

EXTERNC void resetTabCtls(int iCtl);

EXTERNC void removeChar(char * str, char garbage);

EXTERNC int fm_isnan(double x);
EXTERNC int fm_isinf(double x);

EXTERNC int getSysDPI(double * factorX, double * factorY);
EXTERNC BOOL checkUIScaling();
EXTERNC BOOL isHDPIAware();

EXTERNC void deactivateFlicks(int n);

EXTERNC void setDialogTitle(char * title);

EXTERNC int evalZoomCombo(int n, int ctlPlus, int ctlMinus, int previous);
EXTERNC int evalZoomButtons(int n, int ctlCombo, int ctlPlus, int ctlMinus);

// Added by Ognen Genchev
EXTERNC int setCtlEditPos(int n, int x, int y);
EXTERNC int setCustomCtl(int n, int iName);
EXTERNC int setCtlInitVal(int n, int val);
EXTERNC int setFrameColor(int n, int color);
// EXTERNC struct DialConfig;  // forward declaration
// Create a custom dial control inside a dialog
EXTERNC double fclamp (double x, double minVal, double maxVal);
EXTERNC double fsmoothstep (double edge0, double edge1, double x);
// end