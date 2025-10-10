///////////////////////////////////////////////////////////////////////////////////
//  File: AfhFMcontext.h
//
//  Header file with the FilterMeisterVS Execution Environment (Context) definitions
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

#ifndef AFHFMCONTEXT_H_DEF
#define AFHFMCONTEXT_H_DEF

#include <windows.h>
#include "AfhFM.h"

/******************* Context Structure Definition ************************/
#if 0
typedef int (*fmf0)(void);
typedef int (*fmf1)(int);
typedef int (*fmf2)(int,int);
#endif
typedef int (*fmf3)(int,int,int);
typedef int (*fmf4)(int,int,int,int);
#if 0
typedef int (*fmf5)(int,int,int,int,int);
typedef int (*fmf6)(int,int,int,int,int,int);
typedef int (*fmf7)(int,int,int,int,int,int,int);
typedef int (*fmf8)(int,int,int,int,int,int,int,int);
typedef int (*fmf9)(int,int,int,int,int,int,int,int,int);
typedef int (*fmf10)(int,int,int,int,int,int,int,int,int,int);
typedef int (*fmf11)(int,int,int,int,int,int,int,int,int,int,int);
typedef int (*fmf12)(int,int,int,int,int,int,int,int,int,int,int,int);
typedef int (*fmf13)(int,int,int,int,int,int,int,int,int,int,int,int,int);
typedef int (*fmf14)(int,int,int,int,int,int,int,int,int,int,int,int,int,int);
typedef int (*fmf15)(int,int,int,int,int,int,int,int,int,int,int,int,int,int,int);

typedef int (*fmf16)(int,int,int,int,int,int,int,int,int,int,int,int,int,int,int,int);
typedef int (*fmf17)(int,int,int,int,int,int,int,int,int,int,int,int,int,int,int,int,int);
typedef int (*fmf18)(int,int,int,int,int,int,int,int,int,int,int,int,int,int,int,int,int,int);
typedef int (*fmf19)(int,int,int,int,int,int,int,int,int,int,int,int,int,int,int,int,int,int,int);
typedef int (*fmf20)(int,int,int,int,int,int,int,int,int,int,int,int,int,int,int,int,int,int,int,int);


typedef int (*fmf1u)(unsigned int);
typedef double (*fmf1vd)(int,...);

typedef int (*fmf1v)(int,...);
typedef int (*fmf2v)(int, int,...);
typedef int (*fmf3v)(int, int, int,...);

typedef int (*fmf8v)(int,int,int,int,int,int,int,int,...);


typedef int (*fmfd)(double);
typedef int (*fmf2di)(double,double);

typedef double (*fmf1d)(int);
typedef double (*fmf2d)(int,int);
typedef double (*fmf4f)(int,int,int,int);

typedef double (*fmf1dd)(double);
typedef double (*fmf2dd)(double,double);
typedef double (*fmf2ddv)(double,double,...);
typedef double (*fmfdid)(double,int);

typedef int (*fmfid)(int,double);

typedef void (*fmf0n)(void);
typedef void (*fmf1n)(int);
typedef void (*fmf1nu)(unsigned int);

typedef int (*fmf5d)(double,double,int,int,int);

typedef int (*fmf4d)(int,double,double,int);

typedef int (*fmf3f)(int,int, double);

typedef int (*fmf5f)(int,int,int,int, double);

//Added by Harald Heim for threeDto2D()
typedef int (*fmf10d)(int,int,int,int,int,int,int,double,double,double);

typedef int (*fmf1u)(UINT);
typedef int (*fmf2u)(int,UINT);
typedef int (*fmf3u)(int,int,UINT);
typedef int (*fmf4u)(int,int,int,UINT);
typedef int (*fmf5u)(int,int,int,int,UINT);
typedef int (*fmf5u)(int,int,int,int,UINT);
typedef int (*fmf6u)(int,int,int,int,int,UINT);
typedef int (*fmf5u3)(int,int,UINT, int,int);
typedef int (*fmf5u3v)(int,int,UINT,int,int,...);
typedef int (*fmf7iui)(int,int,int,int,UINT,UINT,int);

typedef int (*fmf1ca)(CTLACTION);

typedef int (*fmf3uf)(UINT,UINT,double);

typedef int (*fmf6iud)(int, int, ULONG, double, double, UINT);

typedef int  (*fmf5d2)(int,int,int,int,double);
#endif


//Win32 API uses __stdcall
typedef int  (WINAPI *wf0)(void);   // wf0 is call-compatible with fmf0, but tool-chain doesn't know this!!
typedef HANDLE (WINAPI *wf0h)(void);      // ditto wf0h
typedef int  (WINAPI *wf4)(int,int,int,int);



// When an FM DLL is first called, it should check the
// major and minor version fields. If the major version is not
// the same as when the DLL was compiled, this means some
// incompatible changes (such as adding new fields to the context
// record inbetween old fields) have been made, and the DLL must
// abort or return an error code.
// 
// If the major version is the same as when the DLL was compiled,
// then check the minor version. If the minor version is greater
// than or equal to the version at the time the DLL was compiled,
// it means no incompatible changes have occurred (e.g., all new
// context record fields have been added to the end of the structure)
// and the DLL may proceed to execute. If the minor version is
// less than when the DLL was compiled, then the DLL is being
// called from an earlier version of FM (and some fields at the
// end of the context record may be missing), and the DLL should
// again abort execution or return an error code (unless it is
// very careful to avoid accessing any missing fields in the
// context record).

#define FMC_MAJOR_VERSION 1
#define FMC_MINOR_VERSION 0

typedef struct FMcontext_tag
{

    //Please add new items at the bottom of FMcontext and increase the FMC_MINOR_VERSION value
    //If that is not possible, please increase the FMC_MAJOR_VERSION value
    


    /////// FROZEN SECTION STARTS HERE ///////
    /* Undefined variables default to this field: */
    double undefined;
    /*const*/ int FMC_minor_version;
    /*const*/ int FMC_major_version;
    /*const*/ int FMC_struct_size;
    /////// FROZEN SECTION ENDS HERE ///////
    /* Temporary variable(s) */
    double temp;
    /* Public FF built-in variables */
    int r, g, b, a; //MUST be contiguous!
    int R, G, B, A; //MUST be contiguous!
    int c;
    int C;
    int i, u, v;
    int I, U, V;
    int x, y, z;
    int X, Y, Z;
    int d, m;
    int D, M;
    /* Public FF built-in variables for Premiere compatibility */
    int r0, g0, b0, a0; //same as r, g, b, a.
    int r1, g1, b1, a1; //same as r, g, b, a.
    int c0, c1;         //same as c.
    int /*i0,*/ u0, v0;     //same as i, u, v.
    int /*i1,*/ u1, v1;     //same as i, u, v.
    int d0, m0;         //same as d, m.
    int d1, m1;         //same as d, m.
    int rmax, gmax, bmax, amax; //same as R, G, B, A.
    int rmin, gmin, bmin, amin; //always 0.
    int cmax, cmin;             //same as C, 0.
    int imax, umax, vmax;       //same as I, U, V.
    int imin, umin, vmin;       //always 0.
    int xmax, ymax, zmax;       //same as X, Y, Z.
    int xmin, ymin, zmin;       //always 0.
    int dmax, mmax;             //same as D, M.
    int dmin, mmin;             //always 0.
    int p, pmax, pmin;  //same as z, zmax, zmin.
    int t, tmin, tmax, total;   // 0, 0, 1, 1.
    /* Public FF+ built-in variables */
    int n;      // control index in Ctl[n] handler
    int e;      // FM event code
    int previous;      // FM event value

    int ctlMsg;
    int ctlCode;
    CTLCLASS ctlClass;
    int dividend;
    int divisor;
    int scaleFactor;
	int enlargeFactor;
	int scrollFactor;
    int row;
    int rows;
    int col;
    int columns;
    int plane;
    int inPlanes;
    int srcRowDelta;
    int dstRowDelta;
    int mskRowDelta;
    int srcColDelta;
    int dstColDelta;
    int mskColDelta;
    int writeAlpha;
    int alphaMask;      // 0 or 0xff

// Following members will be copied to the standalone filter via the FMCODE resource... /*GLOBAL?*/
    int need_iuv;
    int need_d;
    int need_m;
    int need_tbuf;
    int need_t2buf;
	int need_t3buf;
	int need_t4buf;
    int needPremiere;   //Need Premiere compatibility support
    int isTileable;     /*GLOBAL?*/
    int needPadding;    /*GLOBAL?*/
    int tileHeight;//bandWidth;      /*GLOBAL?*/
    int tileWidth;

    int previewCheckbox;

// End of range of members that will be copied to standalone filter.
    int doingProxy;
    int doingScripting;
    int x_start, x_end;
    int y_start, y_end;
    unsigned char *tbuf;         // (Temporary) tile buffer.    /*GLOBAL?*/
    unsigned char *t2buf;        // (Temporary) tile buffer 2.
	unsigned char *t3buf;        // (Temporary) tile buffer 3.
	unsigned char *t4buf;        // (Temporary) tile buffer 4.
	BufferID tbufID;                                            /*GLOBAL?*/
	BufferID t2bufID;
	BufferID t3bufID;
	BufferID t4bufID;

	//int DESIGNTIME;
	int entryPoint;

    /* Some handy FF+ predeclared variables, preserved across invocations */
    /* Also contains user-declared global and static variables.           */
    PREDEF_GLOBALS globvars;                                    /*GLOBAL*/

    /* Private FF+ built-in variables */
    HWND hDlg;
    BYTE *srcPtr;
    BYTE *dstPtr;
    BYTE *mskPtr;
    BYTE *mskData;

#if BIGDOCUMENT
	/* BigDocument fields from Host, for 32-bit coordinate support */
	int	bigDocument;
	VPoint	imageSize32;
    VRect	filterRect32;
    int		filterRectWidth32;
    int		filterRectHeight32;
    VRect	inRect32;
    VRect	outRect32;
    VRect	maskRect32;
	VPoint	floatCoord32;
    VPoint	wholeSize32;
#endif

    /* Read-only information from host's Filter Record */
    int hostSerialNumber;
    int displayDialog;
    int imageWidth;
    int imageHeight;
    int filterRectLeft;
    int filterRectTop;
    int filterRectRight;
    int filterRectBottom;
    int filterRectWidth;
    int filterRectHeight;
    int planes;
    int planesWithoutAlpha;
    int maxSpace;
	int bufferSpace;
    BOOL isFloating;
    BOOL haveMask;

    BOOL autoMask;
    
    COLORREF backColor;     //background color in image color space
    COLORREF foreColor;     //foreground color in image color space
    COLORREF bgColor;       //background color in RGB color space
    COLORREF fgColor;       //foreground color in RGB color space
    int hostSig;

    INT_PTR platformData;
    double imageHRes;
    double imageVRes;

    int imageMode;
    int wholeWidth;
    int wholeHeight;
    int filterCase;
    BOOL samplingSupport;
    int zoomFactor;
    /* Public FF+ built-in strings and arrays */            /*GLOBAL?*/
    char filterAuthorText[256];
    char filterCategoryText[256];
    char filterTitleText[256];
    char filterFilenameText[256];
    char filterCopyrightText[256];
    char filterDescriptionText[256];
    char filterOrganizationText[256];
    char filterURLText[256];
    char filterVersionText[256];
    char filterCaseText[256];
    char filterImageModeText[256];
    char filterHostText[256];
    char filterInstallDir[256];
    char filterUniqueID[37];

    /* Some handy floating-point constants */
    /*const*/ double magic5;    // = 0.49999999999999 (carefully tuned for in-line OP_SQR!)
    /*const*/ double d512byPi;  // = 512.0/Pi (for c2d etc.)
    /*const*/ float  float1p0;  // = 1.0F (single-precision 32-bit float, for in-line powi etc.)
    /* NDP control words */
    int current_cw;
    /*const*/ int CW_FLOOR;
    /*const*/ int CW_CEIL;
    /*const*/ int CW_ROUND;
    /*const*/ int CW_CHOP;


    //Exceptions
    fmf3 src;
    fmf3 pget;
    fmf3 tget;
    fmf3 t2get;
	fmf3 t3get;
	fmf3 t4get;
    fmf4 pset;
    fmf4 tset;
    fmf4 t2set;
	fmf4 t3set;
	fmf4 t4set;
    fmf3 pgetr;



    /* Anonymous storage cells for put() and get() */       /*GLOBAL?*/
    int cell[N_CELLS];
    /* Cached settings of user controls */                  /*GLOBAL!*/
    int pre_ctl[N_CTLS];
#ifndef TRIG_ON_FLY
    // Precomputed trig tables                              /*GLOBAL?*/
    int pre_sin[1024];
    int pre_tan[512];
#endif

    //int renderTime; //meassure performance for instant sliders


}
FMcontext;

    /* Global variables. */

extern FMcontext fmc; //Previously defined in FW_PluginMain.c

#ifdef APP
FMcontext * const gFmcp;
#else
EXTERNC FMcontext * gFmcp;
#endif

#endif // close ifndef AFHFMCONTEXT_H_DEF