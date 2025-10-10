///////////////////////////////////////////////////////////////////////////////////
//  File: FW_GeneralFunctions.cpp
//
//  Source file of FilterMeisterVS which provides the dialog and image processing
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


//At least Win XP for ListView
//#define _WIN32_WINNT 0x501
//At least Win Vista for ListView
#define _WIN32_WINNT 0x600


#include <stdlib.h> // abs, rand
#include <malloc.h> // _expand, _msize
#include <math.h>
#include <string.h>
#include <time.h>   // clock, time.
#include <limits.h> // INT_MAX

#include <stdio.h>  // vsprintf, _vsnprintf
#include <stdarg.h> // va_list, etc.
#include <assert.h>
#include <errno.h>
#include <direct.h> //directory stuff

#include <windows.h>
#include <commctrl.h>       //for UDACCEL

#include <SHLOBJ.H> //For SHGetSpecialFolderLocation

#ifndef APP
    #include "PIUtilities.h"
#endif

#include "AfhFM.h"
#include "AfhFMcontext.h"

#include "resource.h"
#include "FW_FFP.h"

#if SKIN
	#include "../../FW_SpecialFunctions.h"
#endif


//#include <Uxtheme.h>


GPtr /*const*/ MyGlobals;
void *MyAdjustedMaskData;
void *MyAdjustedInData;
HWND MyHDlg; 
HWND ghTT; 
HANDLE hDllInstance = 0;
GPtr globals;
MINMAXINFO MaxMinInfo;
FMcontext fmc;
HFONT Font[N_FONTS];
int mtAbortFlag;
SPBasicSuite * sSPBasic;
BigDocumentStruct * gBigDocumentData;
FilterRecord * gFilterRecord;
char    *gDialogImage;
struct MULTITHREAD mt; 


//Distance Array
/*static*/ struct Vect_s
{
    short x, y;
};
/*static*/ struct Vect_s *dist;

int ClickDrag=0;



//Added by Harald Heim		// ARE THESE THREAD-SAFE??
static int bitDepthMode = 8;
int mode_maxVal=255;
int mode_midVal=128;
int mode_quarVal=64;
int mode_bitMultiply=1;

static int psetpMode = 0;

int edgeMode = 0;
int MouseOverWhenInvisible = 0;
int EditBoxUpdate = 500; //Was zero before

int sizeGrip=0;
int unlockedPreviewFrame=0;

int ctlUpdateLocked = false;


int arrayMode = 0; //Is set to allocation by the host in DoStart
void *Array[100]; //Initialized in DoStart
BufferID ArrayID[100]; //for BufferProc //Initialized in DoStart
UINT_PTR ArraySize[100];
int ArrayBytes[100], ArrayX[100], ArrayY[100], ArrayZ[100]; //Initialized in DoStart
int ArrayPadding[100]; //Initialized in DoStart
BufferID bufferID = 0;

HIMAGELIST ImageList[10] ={0,0,0,0,0,0,0,0,0,0}; 

int PhotoPlusArray = -1; //PhotoPlus uses 0 as ArrayID, which causes crash under Windows 7 when freeing the array

static INT_PTR *ThreadHandle = NULL;
static INT_PTR *ThreadMem = NULL;
static int ThreadCount = 0;


int getAppTheme (void);
int appTheme = -1;




//------------------------------------------------------------------------------
// ----------------------fm_ functions------------------------------------------
//------------------------------------------------------------------------------


//----------------------------------------------------------------
//
//  A few utility routines
//

/**********************************************************************/
/* Map4DBUsToPixels(&x1, &y1, &x2, &y2)
/*
/*  Maps 4 coordinates from Dialog Box Units to pixels.
/*  x1 and x2 are mapped as horizontal coordinates;
/*  y1 and y2 are mapped as vertical coordinates.
/*  (Horizontal and vertical mappings are generally identical,
/*  but not always!)
/**********************************************************************/
void Map4DBUsToPixels(int *x1, int *y1, int *x2, int *y2) {
    RECT r;
    r.left = *x1;
    r.top = *y1;
    r.right = *x2;
    r.bottom = *y2;
    MapDialogRect(fmc.hDlg, &r);
    *x1 = r.left;
    *y1 = r.top;
    *x2 = r.right;
    *y2 = r.bottom;
} /*Map4DBUsToPixels*/

/**********************************************************************/
/* Map2DBUsToPixels(&x1, &y1)
/*
/*  Same as Map4DBUsToPixels(), but maps only 2 coordinates instead of 4.
/*  x1 is mapped as a horizontal coordinate;
/*  y1 is mapped as a vertical coordinate.
/**********************************************************************/
void Map2DBUsToPixels(int *x1, int *y1) {
    RECT r;
    r.left = *x1;
    r.top = *y1;
    r.right = 0;
    r.bottom = 0;
    MapDialogRect(fmc.hDlg, &r);
    *x1 = r.left;
    *y1 = r.top;
} /*Map2DBUsToPixels*/

/**********************************************************************/
/* HDBUsToPixels(h)
/*
/*  Returns the number of pixels comprising h horizontal DBUs.
/*
/**********************************************************************/
int HDBUsToPixels(int h) {
    RECT r;
    r.left = h;
    r.top = 0;
    r.right = 0;
    r.bottom = 0;
    MapDialogRect(fmc.hDlg, &r);
    return r.left;
} /*fm_HDBUsToPixels*/

/**********************************************************************/
/* VDBUsToPixels(v)
/*
/*  Returns the number of pixels comprising v vertical DBUs.
/*
/**********************************************************************/
int VDBUsToPixels(int v) {
    RECT r;
    r.left = 0;
    r.top = v;
    r.right = 0;
    r.bottom = 0;
    MapDialogRect(fmc.hDlg, &r);
    return r.top;
} /*fm_VDBUsToPixels*/



//----------------------------------------------------------------
// Utility routines for simulating Filter Factory

int pset(int x, int y, int z, int val)
{ 

    // Clamp value to [0,255].
#if 0
    if (val > 255) val = 255;
    else if (val < 0) val = 0;
#elif 0 //using unsigned should be faster...
    val = ((unsigned)val <= 255) ? val : (val <= 0) ? 0 : 255;
#else //or is this faster??
    if ((unsigned)val > 255) {
        val = (val <= 0) ? 0 : 255;
    }//needs to be clamped
#endif
    if (x >= fmc.x_start && x < fmc.x_end &&
        y >= fmc.y_start && y < fmc.y_end &&
        z >= 0 && z < fmc.dstColDelta )
    {   //valid coordinate.

/*
#ifndef DONT_USE_MASK

        if (fmc.doingProxy && gStuff->haveMask){ //Simulate final result in preview
            
            int MaskValue = msk(x,y); //src(x,y,-1);
            
            if (MaskValue>0){
                if (MaskValue<255){
                    val = (val*MaskValue + src(x,y,z)*(255-MaskValue) )/255;
                }
                    
                ((unsigned8 *)gStuff->outData)
                    [(x-fmc.x_start)*fmc.dstColDelta + (y-fmc.y_start)*fmc.dstRowDelta + z] = val;
            } 

        } else { //regular pset

            ((unsigned8 *)gStuff->outData)
                [(x-fmc.x_start)*fmc.dstColDelta + (y-fmc.y_start)*fmc.dstRowDelta + z] = val;
        }

#else
*/
        ((unsigned8 *)gStuff->outData)
                [(x-fmc.x_start)*fmc.dstColDelta + (y-fmc.y_start)*fmc.dstRowDelta + z] = val;
        
//#endif


    }
    return val; //clamped value
} /*fm_pset*/

int pset16(int x, int y, int z, int val)
{
    
    // 16-bit version.
    // Clamp value to [0,0x8000].
#if 0
    if (val > 0x8000) val = 0x8000;
    else if (val < 0) val = 0;
#elif 0 //using unsigned should be faster...
    val = ((unsigned)val <= 0x8000) ? val : (val <= 0) ? 0 : 0x8000;
#else //or is this faster??

#ifndef _WIN64
    if (fmc.hostSig != 0x50535039){ //if not PSP X3
#endif
        if ((unsigned)val > 0x8000) val = (val <= 0) ? 0 : 0x8000;
#ifndef _WIN64
    } else { //PSP X3
        if ((unsigned)val > 32767) val = (val <= 0) ? 0 : 32767;
    }
#endif

#endif
    if (x >= fmc.x_start && x < fmc.x_end &&
        y >= fmc.y_start && y < fmc.y_end &&
        z >= 0 && z < fmc.dstColDelta )
    {   //valid coordinates.
        
/*        
#ifndef DONT_USE_MASK