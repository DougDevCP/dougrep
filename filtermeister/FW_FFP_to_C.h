///////////////////////////////////////////////////////////////////////////////////
//  File: FW_FFP_to_C.h
//
//  Header file of FilterMeisterVS which translates FilterMeister language 
//  expressions to C/C++
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

#include <math.h> // for cos / sin / tan redirect


#ifdef __cplusplus
#define EXTERNC extern "C"
#else
#define EXTERNC
#endif


//---------------------------------------------------------------
// Preprocessor definitions for converting the FFP language to C
//---------------------------------------------------------------

//Bit operators
#define bitSet(a,b) ((a) |= (1 << (b))) 
#define bitClear(a,b) ((a) &= ~(1 << (b)))
#define bitFlip(a,b) ((a) ^= (1 << (b))) 
#define bitTest(a,b) (((a) & (1 << (b))) !=0 ? true:false)

#define bitmaskSet(x,y) ((x) |= (y)) 
#define bitmaskClear(x,y) ((x) &= (~(y))) 
#define bitmaskFlip(x,y) ((x) ^= (y)) 
#define bitmaskTest(x,y) ((x) & (y)) //!=0 ? -1:0 


//Handlers
//#define CtlDef int CtlDef (FMcontext * const fmc)
#define OnCtl(n) int OnCtl(FMcontext * const fmc, int n, int e, int previous)
#define OnWindowStart int OnWindowStart(FMcontext * const fmc)
#define OnFilterStart int OnFilterStart(FMcontext * const fmc)
#define ForEveryTile int ForEveryTile(FMcontext * const fmc)
//#define ForEveryRow int ForEveryRow(FMcontext * const fmc)
#define OnFilterEnd int OnFilterEnd(FMcontext * const fmc)
#define OnFilterExit void OnFilterExit()

#undef val //for the normal val(a,b,c) function

//Image access functions for 8bit as well 16bit image data
#define src(x,y,z) fmc->src(x,y,z)
//#define src(x,y,z) fast_src(x,y,z)
//#define src(x,y,z) imageMode<10 ? ((unsigned8 *)MyAdjustedInData) [((x)-x_start)*srcColDelta + ((y)-y_start)*srcRowDelta + z] : ((unsigned16 *)MyAdjustedInData) [((x)-x_start)*srcColDelta + ((y)-y_start)*srcRowDelta + z]

#define pset(x,y,z,a) fmc->pset(x,y,z,a)
//#define pset(x,y,z,a) fast_pset(x,y,z,a)
//#define pset(x,y,z,a) if (imageMode<10) ((unsigned8 *)gStuff->outData)[((x)-x_start)*dstColDelta + ((y)-y_start)*dstRowDelta + z] = a; else ((unsigned16 *)gStuff->outData)[((x)-x_start)*dstColDelta + ((y)-y_start)*dstRowDelta + z] = a

#define pget(x,y,z) fmc->pget(x,y,z)
#define pgetr(x,y,z) fmc->pgetr(x,y,z)

//#define need_tbuf fmc->need_tbuf
//#define need_t2buf fmc->need_t2buf
//#define need_t3buf fmc->need_t3buf
//#define need_t4buf fmc->need_t4buf

//#define RGB(a,b,c) fm_RGB(a,b,c)

#define tget(x,y,z) fmc->tget(x,y,z)
#define t2get(x,y,z) fmc->t2get(x,y,z)
#define t3get(x,y,z) fmc->t3get(x,y,z)
#define t4get(x,y,z) fmc->t4get(x,y,z)
#define tset(x,y,z,a) fmc->tset(x,y,z,a)
#define t2set(x,y,z,a) fmc->t2set(x,y,z,a)
#define t3set(x,y,z,a) fmc->t3set(x,y,z,a)
#define t4set(x,y,z,a) fmc->t4set(x,y,z,a)

#define setCtlTextColor setCtlFontColor
#define getCtlTextColor getCtlFontColor

//Control values
#undef ctl
#define ctl(a) fmc->pre_ctl[a]

//Formulas
//#define scl(a,il,ih,ol,oh)  ( ih==il ? 0 : ((a) - (il))*((oh) - (ol))/((ih) - (il)) + (ol) )
#define clip(a,b,c) if (a<b) a=b; else if (a>c) a=c
#define clip2(a) if (a<0) a=0; else if (a>maxval) a=maxval
#define clip8(a) if (a<0) a=0; else if (a>255) a=255
#define invert8(a) a=255-(a)

#define cblend(val1,val2,fac,max) (((max)-(fac))*(val1) + (fac)*(val2))/(max)
#define cblend255(val1,val2,fac) fac==255 ? val2 : ((255-(fac))*(val1) + (fac)*(val2))/255
#define cblend128(val1,val2,fac) fac==128 ? val2 : ((128-(fac))*(val1) + (fac)*(val2))/128
#define cblend100(val1,val2,fac) fac==100 ? val2 : ((100-(fac))*(val1) + (fac)*(val2))/100
#define cblend100m(val1,val2,fac) fac==-100 ? val1 : ((100-(fac+100))*(val1) + (fac+100)*(val2))/100
#define cblend128m(val1,val2,fac) fac==-128 ? val1 : ((128-(fac+128))*(val1) + (fac+128)*(val2))/128

#define overlay(a,b,midval,maxval) (a)<(midval) ? (2*(a)*(b))/(maxval) : (maxval)-(2*(maxval-(a))*((maxval)-(b)))/(maxval);
#define overlay2(a,b) (a)<(midval) ? (2*(a)*(b))/(maxval) : (maxval)-(2*(maxval-(a))*((maxval)-(b)))/(maxval);

#define getCtlVal(a) fmc->pre_ctl[a]

#define sleep(a) Sleep(a)


//SDK
/*
#define Gray16Mode plugInModeGray16
#define RGBMode plugInModeRGBColor
#define GrayScaleMode plugInModeGrayScale
*/


//Global Vars
#define i0 fmc->globvars.i0
#define i1 fmc->globvars.i1
#define i2 fmc->globvars.i2
#define i3 fmc->globvars.i3
#define i4 fmc->globvars.i4
#define i5 fmc->globvars.i5
#define i6 fmc->globvars.i6
#define i7 fmc->globvars.i7
#define i8 fmc->globvars.i8
#define i9 fmc->globvars.i9

#define j0 fmc->globvars.j0
#define j1 fmc->globvars.j1
#define j2 fmc->globvars.j2
#define j3 fmc->globvars.j3
#define j4 fmc->globvars.j4
#define j5 fmc->globvars.j5
#define j6 fmc->globvars.j6
#define j7 fmc->globvars.j7
#define j8 fmc->globvars.j8
#define j9 fmc->globvars.j9

#define k0 fmc->globvars.k0
#define k1 fmc->globvars.k1
#define k2 fmc->globvars.k2
#define k3 fmc->globvars.k3
#define k4 fmc->globvars.k4
#define k5 fmc->globvars.k5
#define k6 fmc->globvars.k6
#define k7 fmc->globvars.k7
#define k8 fmc->globvars.k8
#define k9 fmc->globvars.k9


#define x0 fmc->globvars.x0
#define x1 fmc->globvars.x1
#define x2 fmc->globvars.x2
#define x3 fmc->globvars.x3
#define x4 fmc->globvars.x4
#define x5 fmc->globvars.x5
#define x6 fmc->globvars.x6
#define x7 fmc->globvars.x7
#define x8 fmc->globvars.x8
#define x9 fmc->globvars.x9

#define y0 fmc->globvars.y0
#define y1 fmc->globvars.y1
#define y2 fmc->globvars.y2
#define y3 fmc->globvars.y3
#define y4 fmc->globvars.y4
#define y5 fmc->globvars.y5
#define y6 fmc->globvars.y6
#define y7 fmc->globvars.y7
#define y8 fmc->globvars.y8
#define y9 fmc->globvars.y9

#define z0 fmc->globvars.z0
#define z1 fmc->globvars.z1
#define z2 fmc->globvars.z2
#define z3 fmc->globvars.z3
#define z4 fmc->globvars.z4
#define z5 fmc->globvars.z5
#define z6 fmc->globvars.z6
#define z7 fmc->globvars.z7
#define z8 fmc->globvars.z8
#define z9 fmc->globvars.z9

#define str0 fmc->globvars.str0
#define str1 fmc->globvars.str1
#define str2 fmc->globvars.str2
#define str3 fmc->globvars.str3
#define str4 fmc->globvars.str4
#define str5 fmc->globvars.str5
#define str6 fmc->globvars.str6
#define str7 fmc->globvars.str7
#define str8 fmc->globvars.str8
#define str9 fmc->globvars.str9
#define str10 fmc->globvars.str10
#define str11 fmc->globvars.str11
#define str12 fmc->globvars.str12
#define str13 fmc->globvars.str13
#define str14 fmc->globvars.str14
#define str15 fmc->globvars.str15
#define str16 fmc->globvars.str16
#define str17 fmc->globvars.str17
#define str18 fmc->globvars.str18
#define str19 fmc->globvars.str19


//FM Context
#define X fmc->X
#define Y fmc->Y
#define Z fmc->Z

//Do not use variables otherwise we get problems with multithreading 
//and some bugs may be hidden
/*
#define c fmc->c
#define r fmc->r
#define g fmc->g
#define b fmc->b
#define a fmc->a
#define x fmc->x
#define y fmc->y
#define z fmc->z
*/

//#define n fmc->n      // control index in Ctl[n] handler
//#define e fmc->e      // FM event code    
//#define previous fmc->previous      // FM event value

#define scaleFactor     fmc->scaleFactor
#define enlargeFactor   fmc->enlargeFactor
#define scrollFactor    fmc->scrollFactor

#define srcRowDelta fmc->srcRowDelta
#define dstRowDelta fmc->dstRowDelta
#define mskRowDelta fmc->mskRowDelta
#define srcColDelta fmc->srcColDelta
#define dstColDelta fmc->dstColDelta
#define mskColDelta fmc->mskColDelta

#define needPadding fmc->needPadding
#define doingProxy fmc->doingProxy
#define doingScripting fmc->doingScripting
#define x_start fmc->x_start
#define x_end   fmc->x_end
#define y_start fmc->y_start
#define y_end   fmc->y_end

//#define DESIGNTIME fmc->DESIGNTIME
#define ENTRYPOINT fmc->entryPoint

/* Read-only information from host's Filter Record */
#define displayDialog fmc->displayDialog

#if BIGDOCUMENT
	#define imageWidth fmc->imageSize32.h
	#define imageHeight fmc->imageSize32.v
	#define filterRectWidth fmc->filterRectWidth32
	#define filterRectHeight fmc->filterRectHeight32
	#define filterRectLeft fmc->filterRect32.left
	#define filterRectRight fmc->filterRect32.right
	#define filterRectTop fmc->filterRect32.top
	#define filterRectBottom fmc->filterRect32.bottom
#else
	#define imageWidth fmc->imageWidth
	#define imageHeight fmc->imageHeight
	#define filterRectWidth fmc->filterRectWidth
	#define filterRectHeight fmc->filterRectHeight
	#define filterRectLeft fmc->filterRectLeft
	#define filterRectRight fmc->filterRectRight
	#define filterRectTop fmc->filterRectTop
	#define filterRectBottom fmc->filterRectBottom
#endif

#define planes fmc->planes
#define planesWithoutAlpha fmc->planesWithoutAlpha
#define haveMask fmc->haveMask

#define backColor fmc->backColor    //background color in image color space
#define foreColor fmc->foreColor    //foreground color in image color space
#define hostSig fmc->hostSig

#define imageHRes fmc->imageHRes
#define imageVRes fmc->imageVRes

#define imageMode fmc->imageMode
#define zoomFactor fmc->zoomFactor
#define filterCase fmc->filterCase

#define isTileable fmc->isTileable
#define bandWidth tileHeight
#define tileHeight fmc->tileHeight 
#define tileWidth fmc->tileWidth
#define filterInstallDir fmc->filterInstallDir

#define FME_VALUECHANGED FME_CHANGED

typedef double (*fmf1dd)(double);
fmf1dd math_cos = cos;
fmf1dd math_sin = sin;
fmf1dd math_tan = tan;

//Functions that collide with C
#if defined(HARRYSFILTERS) | defined (PLUGINGALAXY)
    #define sin(n) fm_sin(n)
    #define cos(n) fm_cos(n)
    #define tan(n) fm_tan(n)
	#define fsin(n) fm_fsin(n)
    #define fcos(n) fm_fcos(n)
	#define ftan(n) fm_ftan(n)
	#define ffsin(n) math_sin(n)
    #define ffcos(n) math_cos(n)
	#define fftan(n) math_tan(n)
	#define fc2d(x,y) fm_fc2d(x,y)
	#define fc2m(x,y) fm_fc2m(x,y)
	#define fr2x(d,m) fm_fr2x(d,m)
	#define fr2y(d,m) fm_fr2y(d,m)
#else
	#define fsin(n) sin(n)
	#define fcos(n) cos(n)
	#define ftan(n) tan(n)
#endif


#define abort() {/*fm_abort();*/return true;} //fm_abort()
#define snprintf _snprintf
#define sqr(a) fm_sqr(a)


//Faster access for get and put
//#define get(a) fmc->cell[a & (N_CELLS-1)]
//#define put(a,b) fmc->cell[b & (N_CELLS-1)] = a
#define get(a) fmc->cell[a]
#define put(a,b) fmc->cell[b]=a


#define _eval_FFP for (y= y_start; y < y_end; y++){ \
		          if(updateProgress(y_end*stepCount*currentPass+y, y_end*stepCount*totalPasses)) abort(); \
                  for (x = x_start; x < x_end; x++) {\
                        r = src(x,y,0); \
                        g = src(x,y,1); \
                        b = src(x,y,2); \

#define _eval_FFP_end pset(x,y,0, r); \
                      pset(x,y,1, g); \
                      pset(x,y,2, b); \
                      }}


#define _eval_FFP_A for (y= y_start; y < y_end; y++){ \
		          if(updateProgress(y_end*stepCount*currentPass+y, y_end*stepCount*totalPasses)) abort(); \
                  for (x = x_start; x < x_end; x++) {\
                        r = src(x,y,0); \
                        g = src(x,y,1); \
                        b = src(x,y,2); \
                        if(alphaChannel) a = src(x,y,3);

#define _eval_FFP_A_end pset(x,y,0, r); \
                      pset(x,y,1, g); \
                      pset(x,y,2, b); \
                      if(alphaChannel) pset(x,y,3, a); \
                      }}

#define R r
#define G g
#define B b
#define A a

//C macro collision
/* #undef INVALID_HANDLE_VALUE
#define INVALID_HANDLE_VALUE -1
#undef NULL
#define NULL 0 */

#define setCtlImage(a,b,c) setCtlImage(a,b,0x42)
#define strcpy(a,b) strcpy((LPTSTR)a,(LPCTSTR)b)

#define createPopupMenu (INT_PTR)CreatePopupMenu
#define createMenu (INT_PTR)CreateMenu
#define mkdir _mkdir


#define VK_0 0x30
#define VK_1 0x31
#define VK_2 0x32
#define VK_3 0x33
#define VK_4 0x34
#define VK_5 0x35
#define VK_6 0x36
#define VK_7 0x37
#define VK_8 0x38
#define VK_9 0x39
#define VK_A 0x41
#define VK_B 0x42
#define VK_C 0x43
#define VK_D 0x44
#define VK_E 0x45
#define VK_F 0x46
#define VK_G 0x47
#define VK_H 0x48
#define VK_I 0x49
#define VK_J 0x4a
#define VK_K 0x4b
#define VK_L 0x4c
#define VK_M 0x4d
#define VK_N 0x4e
#define VK_O 0x4f
#define VK_P 0x50
#define VK_Q 0x51
#define VK_R 0x52
#define VK_S 0x53
#define VK_T 0x54
#define VK_U 0x55
#define VK_V 0x56
#define VK_W 0x57
#define VK_X 0x58
#define VK_Y 0x59
#define VK_Z 0x5a


#define CSIDL_APPDATA 26


#define To8Bit(a) (imageMode>9?(int)(a/128.5):a)
//#define To8Bit(a) imageMode>9?(a>>7)|(a>>15):a
//#define To8BitIf(a) if(imageMode>9)a=(a>>7)|(a>>15);
//#define To8BitVal(a) (a>>7)|(a>>15)


