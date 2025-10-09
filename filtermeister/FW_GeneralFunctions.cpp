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

      if (fmc.doingProxy && gStuff->haveMask){ //Simulate final result in preview
            
            int MaskValue = msk(x,y); //src(x,y,-1);
            
            if (MaskValue>0){
                if (MaskValue<65535){
                    val = (val*MaskValue/2 + src16(x,y,z)*(32768-MaskValue/2) )/32768; //32768
                }
                    
                *(unsigned16 *)(((unsigned8 *)gStuff->outData) +
                        (((x-fmc.x_start)*fmc.dstColDelta + z) << 1) +
                        (y-fmc.y_start)*fmc.dstRowDelta)
                    = val;

            } 

        } else { //regular pset

                *(unsigned16 *)(((unsigned8 *)gStuff->outData) +
                        (((x-fmc.x_start)*fmc.dstColDelta + z) << 1) +
                        (y-fmc.y_start)*fmc.dstRowDelta)
                = val;
        }

#else
*/        
            *(unsigned16 *)(((unsigned8 *)gStuff->outData) +
                        (((x-fmc.x_start)*fmc.dstColDelta + z) << 1) +
                        (y-fmc.y_start)*fmc.dstRowDelta)
                = val;
//#endif


    }
    return val; //clamped value
} /*fm_pset16*/


void fast_pset(int x, int y, int z, int val)
{     
    if (x >= fmc.x_start && x < fmc.x_end && y >= fmc.y_start && y < fmc.y_end && z >= 0 && z < fmc.dstColDelta ){
        
        if (fmc.imageMode<10){
            if ((unsigned)val > 255) val = (val <= 0) ? 0 : 255;
            ((unsigned8 *)gStuff->outData)[(x-fmc.x_start)*fmc.dstColDelta + (y-fmc.y_start)*fmc.dstRowDelta + z] = val;

        } else {
/*
            #ifndef _WIN64
                if (fmc.hostSig != 0x50535039){ //if not PSP X3
            #endif
                    if ((unsigned)val > 0x8000) val = (val <= 0) ? 0 : 0x8000;
            #ifndef _WIN64
                } else { //PSP X3
                    if ((unsigned)val > 32767) val = (val <= 0) ? 0 : 32767;
                }
            #endif
*/
            *(unsigned16 *)(((unsigned8 *)gStuff->outData) + (((x-fmc.x_start)*fmc.dstColDelta + z) << 1) + (y-fmc.y_start)*fmc.dstRowDelta) = val;
            //((unsigned16 *)gStuff->outData)[(x-fmc.x_start)*fmc.dstColDelta + (y-fmc.y_start)*fmc.dstRowDelta + z] = val;
        }
    }
}


int tset(int x, int y, int z, int val)
{
#if 1
    if (!fmc.tbuf) {
        // tbuf wasn't allocated????
        //ErrorOk("No tile buffer!");
        return 0;
    }
#endif
    // Store channel value in (temporary) tile buffer.
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
#if 0
    //Min version (tile buffer is size of output tile)
    if (x >= fmc.x_start && x < fmc.x_end &&
        y >= fmc.y_start && y < fmc.y_end &&
        z >= 0 && z < fmc.Z )
    {   //valid coordinate.
        //(optimize this later, after it's been debugged!!!)
        fmc.tbuf[((y-fmc.y_start)*fmc.columns + (x-fmc.x_start))*fmc.Z + z] = val;
    }
#else
    //Max version (tile buffer is size of input tile)
    if (x >= 0 && x < fmc.X &&
        y >= 0 && y < fmc.Y &&
        z >= 0 && z < fmc.Z )
    {   //valid coordinate.
        //(optimize this later, after it's been debugged!!!)
        fmc.tbuf[(y*fmc.X + x)*fmc.Z + z] = val;
    }
#endif
    return val; //clamped value
} /*fm_tset*/

int tset16(int x, int y, int z, int val)
{
#if 1
    if (!fmc.tbuf) {
        // tbuf wasn't allocated????
        //ErrorOk("No tile buffer!");
        return 0;
    }
#endif
    // Store channel value in (temporary) tile buffer.
    // Clamp value to [0,255].
    if (val > 32768) val = 32768;
    else if (val < 0) val = 0;
#if 0
    //Min version (tile buffer is size of output tile)
    if (x >= fmc.x_start && x < fmc.x_end &&
        y >= fmc.y_start && y < fmc.y_end &&
        z >= 0 && z < fmc.Z )
    {   //valid coordinate.
        //(optimize this later, after it's been debugged!!!)
        fmc.tbuf[((y-fmc.y_start)*fmc.columns + (x-fmc.x_start))*fmc.Z + z] = val;
    }
#else
    //Max version (tile buffer is size of input tile)
    if (x >= 0 && x < fmc.X &&
        y >= 0 && y < fmc.Y &&
        z >= 0 && z < fmc.Z )
    {   //valid coordinate.
        //(optimize this later, after it's been debugged!!!)
        *(unsigned16 *)(((unsigned8 *)fmc.tbuf) + 2*((y*fmc.X + x)*fmc.Z + z)) = val;
    }
#endif
    return val; //clamped value
} /*fm_tset16*/

int t2set(int x, int y, int z, int val)
{
#if 1
    if (!fmc.t2buf) {
        // t2buf wasn't allocated????
        //ErrorOk("No tile buffer 2!");
        return 0;
    }
#endif
    // Store channel value in (temporary) tile buffer 2.
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
#if 0
    //Min version (tile buffer is size of output tile)
    if (x >= fmc.x_start && x < fmc.x_end &&
        y >= fmc.y_start && y < fmc.y_end &&
        z >= 0 && z < fmc.Z )
    {   //valid coordinate.
        //(optimize this later, after it's been debugged!!!)
        fmc.t2buf[((y-fmc.y_start)*fmc.columns + (x-fmc.x_start))*fmc.Z + z] = val;
    }
#else
    //Max version (tile buffer is size of input tile)
    if (x >= 0 && x < fmc.X &&
        y >= 0 && y < fmc.Y &&
        z >= 0 && z < fmc.Z )
    {   //valid coordinate.
        //(optimize this later, after it's been debugged!!!)
        fmc.t2buf[(y*fmc.X + x)*fmc.Z + z] = val;
    }
#endif
    return val; //clamped value
} /*fm_t2set*/

int t2set16(int x, int y, int z, int val)
{
#if 1
    if (!fmc.t2buf) {
        // t2buf wasn't allocated????
        //ErrorOk("No tile buffer 2!");
        return 0;
    }
#endif
    // Store channel value in (temporary) tile buffer 2.
    // Clamp value to [0,255].
    if (val > 32768) val = 32768;
    else if (val < 0) val = 0;
#if 0
    //Min version (tile buffer is size of output tile)
    if (x >= fmc.x_start && x < fmc.x_end &&
        y >= fmc.y_start && y < fmc.y_end &&
        z >= 0 && z < fmc.Z )
    {   //valid coordinate.
        //(optimize this later, after it's been debugged!!!)
        fmc.t2buf[((y-fmc.y_start)*fmc.columns + (x-fmc.x_start))*fmc.Z + z] = val;
    }
#else
    //Max version (tile buffer is size of input tile)
    if (x >= 0 && x < fmc.X &&
        y >= 0 && y < fmc.Y &&
        z >= 0 && z < fmc.Z )
    {   //valid coordinate.
        //(optimize this later, after it's been debugged!!!)
        *(unsigned16 *)(((unsigned8 *)fmc.t2buf) + 2*((y*fmc.X + x)*fmc.Z + z)) = val;
    }
#endif
    return val; //clamped value
} /*fm_t2set16*/

int t3set(int x, int y, int z, int val)
{
#if 1
    if (!fmc.t3buf) {
        // t3buf wasn't allocated????
        //ErrorOk("No tile buffer 3!");
        return 0;
    }
#endif
    // Store channel value in (temporary) tile buffer 2.
    // Clamp value to [0,255].
    if (val > 255) val = 255;
    else if (val < 0) val = 0;
#if 0
    //Min version (tile buffer is size of output tile)
    if (x >= fmc.x_start && x < fmc.x_end &&
        y >= fmc.y_start && y < fmc.y_end &&
        z >= 0 && z < fmc.Z )
    {   //valid coordinate.
        //(optimize this later, after it's been debugged!!!)
        fmc.t3buf[((y-fmc.y_start)*fmc.columns + (x-fmc.x_start))*fmc.Z + z] = val;
    }
#else
    //Max version (tile buffer is size of input tile)
    if (x >= 0 && x < fmc.X &&
        y >= 0 && y < fmc.Y &&
        z >= 0 && z < fmc.Z )
    {   //valid coordinate.
        //(optimize this later, after it's been debugged!!!)
        fmc.t3buf[(y*fmc.X + x)*fmc.Z + z] = val;
    }
#endif
    return val; //clamped value
} /*fm_t3set*/

int t3set16(int x, int y, int z, int val)
{
#if 1
    if (!fmc.t3buf) {
        // t3buf wasn't allocated????
        //ErrorOk("No tile buffer 3!");
        return 0;
    }
#endif
    // Store channel value in (temporary) tile buffer 2.
    // Clamp value to [0,255].
    if (val > 32768) val = 32768;
    else if (val < 0) val = 0;
#if 0
    //Min version (tile buffer is size of output tile)
    if (x >= fmc.x_start && x < fmc.x_end &&
        y >= fmc.y_start && y < fmc.y_end &&
        z >= 0 && z < fmc.Z )
    {   //valid coordinate.
        //(optimize this later, after it's been debugged!!!)
        fmc.t3buf[((y-fmc.y_start)*fmc.columns + (x-fmc.x_start))*fmc.Z + z] = val;
    }
#else
    //Max version (tile buffer is size of input tile)
    if (x >= 0 && x < fmc.X &&
        y >= 0 && y < fmc.Y &&
        z >= 0 && z < fmc.Z )
    {   //valid coordinate.
        //(optimize this later, after it's been debugged!!!)
        *(unsigned16 *)(((unsigned8 *)fmc.t3buf) + 2*((y*fmc.X + x)*fmc.Z + z)) = val;
    }
#endif
    return val; //clamped value
} /*fm_t3set16*/

int t4set(int x, int y, int z, int val)
{
#if 1
    if (!fmc.t4buf) {
        // t4buf wasn't allocated????
        //ErrorOk("No tile buffer 4!");
        return 0;
    }
#endif
    // Store channel value in (temporary) tile buffer 2.
    // Clamp value to [0,255].
    if (val > 255) val = 255;
    else if (val < 0) val = 0;
#if 0
    //Min version (tile buffer is size of output tile)
    if (x >= fmc.x_start && x < fmc.x_end &&
        y >= fmc.y_start && y < fmc.y_end &&
        z >= 0 && z < fmc.Z )
    {   //valid coordinate.
        //(optimize this later, after it's been debugged!!!)
        fmc.t4buf[((y-fmc.y_start)*fmc.columns + (x-fmc.x_start))*fmc.Z + z] = val;
    }
#else
    //Max version (tile buffer is size of input tile)
    if (x >= 0 && x < fmc.X &&
        y >= 0 && y < fmc.Y &&
        z >= 0 && z < fmc.Z )
    {   //valid coordinate.
        //(optimize this later, after it's been debugged!!!)
        fmc.t4buf[(y*fmc.X + x)*fmc.Z + z] = val;
    }
#endif
    return val; //clamped value
} /*fm_t4set*/

int t4set16(int x, int y, int z, int val)
{
#if 1
    if (!fmc.t4buf) {
        // t4buf wasn't allocated????
        //ErrorOk("No tile buffer 4!");
        return 0;
    }
#endif
    // Store channel value in (temporary) tile buffer 2.
    // Clamp value to [0,255].
    if (val > 32768) val = 32768;
    else if (val < 0) val = 0;
#if 0
    //Min version (tile buffer is size of output tile)
    if (x >= fmc.x_start && x < fmc.x_end &&
        y >= fmc.y_start && y < fmc.y_end &&
        z >= 0 && z < fmc.Z )
    {   //valid coordinate.
        //(optimize this later, after it's been debugged!!!)
        fmc.t4buf[((y-fmc.y_start)*fmc.columns + (x-fmc.x_start))*fmc.Z + z] = val;
    }
#else
    //Max version (tile buffer is size of input tile)
    if (x >= 0 && x < fmc.X &&
        y >= 0 && y < fmc.Y &&
        z >= 0 && z < fmc.Z )
    {   //valid coordinate.
        //(optimize this later, after it's been debugged!!!)
        *(unsigned16 *)(((unsigned8 *)fmc.t4buf) + 2*((y*fmc.X + x)*fmc.Z + z)) = val;
    }
#endif
    return val; //clamped value
} /*fm_t4set16*/

int pget(const int x0, const int y0, const int z0)
// Must not modify its input parameters due to cnvX/Y in-line optimizations!!!
{
    register int x = x0, y = y0, z = z0;
    if (z >= 0 && z < fmc.dstColDelta)
    {   // get an rgba pixel
        // pin the coordinates (i.e., replicate edges)...
        // Should replicate at IMAGE edge, not TILE edge???
        if (x < fmc.x_start) x = fmc.x_start;
        else if (x >= fmc.x_end) x = fmc.x_end - 1;
        if (y < fmc.y_start) y = fmc.y_start;
        else if (y >= fmc.y_end) y = fmc.y_end - 1;
        //(optimize this later, after it's been debugged!!!)
        return ((unsigned8 *)gStuff->outData)
            [(x-fmc.x_start)*fmc.dstColDelta + (y-fmc.y_start)*fmc.dstRowDelta + z];
    }
    else {
        // Bad channel number???
        return 0;
    }
} /*fm_pget*/


//Added by Harald Heim Feb 9, 2002
int pget16(const int x0, const int y0, const int z0)
// Must not modify its input parameters due to cnvX/Y in-line optimizations!!!
{
    register int x = x0, y = y0, z = z0;

    if (z >= 0 && z < fmc.dstColDelta)
    {   // get an rgba pixel
        // pin the coordinates (i.e., replicate edges)...
        // Should replicate at IMAGE edge, not TILE edge???
        if (x < fmc.x_start) x = fmc.x_start;
        else if (x >= fmc.x_end) x = fmc.x_end - 1;
        if (y < fmc.y_start) y = fmc.y_start;
        else if (y >= fmc.y_end) y = fmc.y_end - 1;
        //(optimize this later, after it's been debugged!!!)
        return *(unsigned16 *)  ( ((unsigned8 *)gStuff->outData) +
            (((x-fmc.x_start)*fmc.dstColDelta + z)<<1 ) + (y-fmc.y_start)*fmc.dstRowDelta  );
    }
    else {
        // Bad channel number???
        return 0;
    }
} /*fm_pget16*/


int tget(const int x0, const int y0, const int z0)
// Must not modify its input parameters due to cnvX/Y in-line optimizations!!!
{
    register int x = x0, y = y0, z = z0;
#if 1
    if (!fmc.tbuf) {
        // tbuf wasn't allocated????
        //ErrorOk("No tile buffer!");
        return 0;
    }
#endif
    // Get channel value from (temporary) tile buffer.
    if (z >= 0 && z < fmc.Z)
    {   // get an rgba pixel
        // pin the coordinates (i.e., replicate edges)...
        // Should replicate at IMAGE edge, not TILE edge???
#if 0
        //Min version (tile buffer is size of output tile)
        if (x < fmc.x_start) x = fmc.x_start;
        else if (x >= fmc.x_end) x = fmc.x_end - 1;
        if (y < fmc.y_start) y = fmc.y_start;
        else if (y >= fmc.y_end) y = fmc.y_end - 1;
        //(optimize this later, after it's been debugged!!!)
        return fmc.tbuf[((y-fmc.y_start)*fmc.columns + (x-fmc.x_start))*fmc.Z + z];
#else
        //Max version (tile buffer is size of input tile)
        if (x < 0) x = 0;
        else if (x >= fmc.X) x = fmc.X - 1;
        if (y < 0) y = 0;
        else if (y >= fmc.Y) y = fmc.Y - 1;
        //(optimize this later, after it's been debugged!!!)
        return fmc.tbuf[(y*fmc.X + x)*fmc.Z + z];
#endif
    }
    else {
        // Bad channel number???
        return 0;
    }
} /*fm_tget*/


int tget16(const int x0, const int y0, const int z0)
// Must not modify its input parameters due to cnvX/Y in-line optimizations!!!
{
    register int x = x0, y = y0, z = z0;
#if 1
    if (!fmc.tbuf) {
        // tbuf wasn't allocated????
        //ErrorOk("No tile buffer!");
        return 0;
    }
#endif
    // Get channel value from (temporary) tile buffer.
    if (z >= 0 && z < fmc.Z)
    {   // get an rgba pixel
        // pin the coordinates (i.e., replicate edges)...
        // Should replicate at IMAGE edge, not TILE edge???
#if 0
        //Min version (tile buffer is size of output tile)
        if (x < fmc.x_start) x = fmc.x_start;
        else if (x >= fmc.x_end) x = fmc.x_end - 1;
        if (y < fmc.y_start) y = fmc.y_start;
        else if (y >= fmc.y_end) y = fmc.y_end - 1;
        //(optimize this later, after it's been debugged!!!)
        return fmc.tbuf[((y-fmc.y_start)*fmc.columns + (x-fmc.x_start))*fmc.Z + z];
#else
        //Max version (tile buffer is size of input tile)
        if (x < 0) x = 0;
        else if (x >= fmc.X) x = fmc.X - 1;
        if (y < 0) y = 0;
        else if (y >= fmc.Y) y = fmc.Y - 1;
        //(optimize this later, after it's been debugged!!!)
        return *(unsigned16 *)(((unsigned8 *)fmc.tbuf) + 2*((y*fmc.X + x)*fmc.Z + z));

#endif
    }
    else {
        // Bad channel number???
        return 0;
    }
} /*fm_tget16*/


int t2get(const int x0, const int y0, const int z0)
// Must not modify its input parameters due to cnvX/Y in-line optimizations!!!
{
    register int x = x0, y = y0, z = z0;
#if 1
    if (!fmc.t2buf) {
        // t2buf wasn't allocated????
        //ErrorOk("No tile buffer 2!");
        return 0;
    }
#endif
    // Get channel value from (temporary) tile buffer 2.
    if (z >= 0 && z < fmc.Z)
    {   // get an rgba pixel
        // pin the coordinates (i.e., replicate edges)...
        // Should replicate at IMAGE edge, not TILE edge???
#if 0
        //Min version (tile buffer is size of output tile)
        if (x < fmc.x_start) x = fmc.x_start;
        else if (x >= fmc.x_end) x = fmc.x_end - 1;
        if (y < fmc.y_start) y = fmc.y_start;
        else if (y >= fmc.y_end) y = fmc.y_end - 1;
        //(optimize this later, after it's been debugged!!!)
        return fmc.t2buf[((y-fmc.y_start)*fmc.columns + (x-fmc.x_start))*fmc.Z + z];
#else
        //Max version (tile buffer is size of input tile)
        if (x < 0) x = 0;
        else if (x >= fmc.X) x = fmc.X - 1;
        if (y < 0) y = 0;
        else if (y >= fmc.Y) y = fmc.Y - 1;
        //(optimize this later, after it's been debugged!!!)
        return fmc.t2buf[(y*fmc.X + x)*fmc.Z + z];
#endif
    }
    else {
        // Bad channel number???
        return 0;
    }
} /*fm_t2get*/

int t2get16(const int x0, const int y0, const int z0)
// Must not modify its input parameters due to cnvX/Y in-line optimizations!!!
{
    register int x = x0, y = y0, z = z0;
#if 1
    if (!fmc.t2buf) {
        // t2buf wasn't allocated????
        //ErrorOk("No tile buffer 2!");
        return 0;
    }
#endif
    // Get channel value from (temporary) tile buffer 2.
    if (z >= 0 && z < fmc.Z)
    {   // get an rgba pixel
        // pin the coordinates (i.e., replicate edges)...
        // Should replicate at IMAGE edge, not TILE edge???
#if 0
        //Min version (tile buffer is size of output tile)
        if (x < fmc.x_start) x = fmc.x_start;
        else if (x >= fmc.x_end) x = fmc.x_end - 1;
        if (y < fmc.y_start) y = fmc.y_start;
        else if (y >= fmc.y_end) y = fmc.y_end - 1;
        //(optimize this later, after it's been debugged!!!)
        return fmc.t2buf[((y-fmc.y_start)*fmc.columns + (x-fmc.x_start))*fmc.Z + z];
#else
        //Max version (tile buffer is size of input tile)
        if (x < 0) x = 0;
        else if (x >= fmc.X) x = fmc.X - 1;
        if (y < 0) y = 0;
        else if (y >= fmc.Y) y = fmc.Y - 1;
        //(optimize this later, after it's been debugged!!!)
		return *(unsigned16 *)(((unsigned8 *)fmc.t2buf) + 2*((y*fmc.X + x)*fmc.Z + z));
#endif
    }
    else {
        // Bad channel number???
        return 0;
    }
} /*fm_t2get16*/

int t3get(const int x0, const int y0, const int z0)
// Must not modify its input parameters due to cnvX/Y in-line optimizations!!!
{
    register int x = x0, y = y0, z = z0;
#if 1
    if (!fmc.t3buf) {
        // t3buf wasn't allocated????
        //ErrorOk("No tile buffer 3!");
        return 0;
    }
#endif
    // Get channel value from (temporary) tile buffer 2.
    if (z >= 0 && z < fmc.Z)
    {   // get an rgba pixel
        // pin the coordinates (i.e., replicate edges)...
        // Should replicate at IMAGE edge, not TILE edge???
#if 0
        //Min version (tile buffer is size of output tile)
        if (x < fmc.x_start) x = fmc.x_start;
        else if (x >= fmc.x_end) x = fmc.x_end - 1;
        if (y < fmc.y_start) y = fmc.y_start;
        else if (y >= fmc.y_end) y = fmc.y_end - 1;
        //(optimize this later, after it's been debugged!!!)
        return fmc.t3buf[((y-fmc.y_start)*fmc.columns + (x-fmc.x_start))*fmc.Z + z];
#else
        //Max version (tile buffer is size of input tile)
        if (x < 0) x = 0;
        else if (x >= fmc.X) x = fmc.X - 1;
        if (y < 0) y = 0;
        else if (y >= fmc.Y) y = fmc.Y - 1;
        //(optimize this later, after it's been debugged!!!)
        return fmc.t3buf[(y*fmc.X + x)*fmc.Z + z];
#endif
    }
    else {
        // Bad channel number???
        return 0;
    }
} /*fm_t3get*/

int t3get16(const int x0, const int y0, const int z0)
// Must not modify its input parameters due to cnvX/Y in-line optimizations!!!
{
    register int x = x0, y = y0, z = z0;
#if 1
    if (!fmc.t3buf) {
        // t3buf wasn't allocated????
        //ErrorOk("No tile buffer 3!");
        return 0;
    }
#endif
    // Get channel value from (temporary) tile buffer 2.
    if (z >= 0 && z < fmc.Z)
    {   // get an rgba pixel
        // pin the coordinates (i.e., replicate edges)...
        // Should replicate at IMAGE edge, not TILE edge???
#if 0
        //Min version (tile buffer is size of output tile)
        if (x < fmc.x_start) x = fmc.x_start;
        else if (x >= fmc.x_end) x = fmc.x_end - 1;
        if (y < fmc.y_start) y = fmc.y_start;
        else if (y >= fmc.y_end) y = fmc.y_end - 1;
        //(optimize this later, after it's been debugged!!!)
        return fmc.t3buf[((y-fmc.y_start)*fmc.columns + (x-fmc.x_start))*fmc.Z + z];
#else
        //Max version (tile buffer is size of input tile)
        if (x < 0) x = 0;
        else if (x >= fmc.X) x = fmc.X - 1;
        if (y < 0) y = 0;
        else if (y >= fmc.Y) y = fmc.Y - 1;
        //(optimize this later, after it's been debugged!!!)
		return *(unsigned16 *)(((unsigned8 *)fmc.t3buf) + 2*((y*fmc.X + x)*fmc.Z + z));
#endif
    }
    else {
        // Bad channel number???
        return 0;
    }
} /*fm_t3get16*/

int t4get(const int x0, const int y0, const int z0)
// Must not modify its input parameters due to cnvX/Y in-line optimizations!!!
{
    register int x = x0, y = y0, z = z0;
#if 1
    if (!fmc.t4buf) {
        // t4buf wasn't allocated????
        //ErrorOk("No tile buffer 4!");
        return 0;
    }
#endif
    // Get channel value from (temporary) tile buffer 2.
    if (z >= 0 && z < fmc.Z)
    {   // get an rgba pixel
        // pin the coordinates (i.e., replicate edges)...
        // Should replicate at IMAGE edge, not TILE edge???
#if 0
        //Min version (tile buffer is size of output tile)
        if (x < fmc.x_start) x = fmc.x_start;
        else if (x >= fmc.x_end) x = fmc.x_end - 1;
        if (y < fmc.y_start) y = fmc.y_start;
        else if (y >= fmc.y_end) y = fmc.y_end - 1;
        //(optimize this later, after it's been debugged!!!)
        return fmc.t4buf[((y-fmc.y_start)*fmc.columns + (x-fmc.x_start))*fmc.Z + z];
#else
        //Max version (tile buffer is size of input tile)
        if (x < 0) x = 0;
        else if (x >= fmc.X) x = fmc.X - 1;
        if (y < 0) y = 0;
        else if (y >= fmc.Y) y = fmc.Y - 1;
        //(optimize this later, after it's been debugged!!!)
        return fmc.t4buf[(y*fmc.X + x)*fmc.Z + z];
#endif
    }
    else {
        // Bad channel number???
        return 0;
    }
} /*fm_t4get*/

int t4get16(const int x0, const int y0, const int z0)
// Must not modify its input parameters due to cnvX/Y in-line optimizations!!!
{
    register int x = x0, y = y0, z = z0;
#if 1
    if (!fmc.t4buf) {
        // t4buf wasn't allocated????
        //ErrorOk("No tile buffer 4!");
        return 0;
    }
#endif
    // Get channel value from (temporary) tile buffer 2.
    if (z >= 0 && z < fmc.Z)
    {   // get an rgba pixel
        // pin the coordinates (i.e., replicate edges)...
        // Should replicate at IMAGE edge, not TILE edge???
#if 0
        //Min version (tile buffer is size of output tile)
        if (x < fmc.x_start) x = fmc.x_start;
        else if (x >= fmc.x_end) x = fmc.x_end - 1;
        if (y < fmc.y_start) y = fmc.y_start;
        else if (y >= fmc.y_end) y = fmc.y_end - 1;
        //(optimize this later, after it's been debugged!!!)
        return fmc.t4buf[((y-fmc.y_start)*fmc.columns + (x-fmc.x_start))*fmc.Z + z];
#else
        //Max version (tile buffer is size of input tile)
        if (x < 0) x = 0;
        else if (x >= fmc.X) x = fmc.X - 1;
        if (y < 0) y = 0;
        else if (y >= fmc.Y) y = fmc.Y - 1;
        //(optimize this later, after it's been debugged!!!)
		return *(unsigned16 *)(((unsigned8 *)fmc.t4buf) + 2*((y*fmc.X + x)*fmc.Z + z));
#endif
    }
    else {
        // Bad channel number???
        return 0;
    }
} /*fm_t4get16*/


int src(const int x0, const int y0, const int z0)
// Must not modify its input parameters due to cnvX/Y in-line optimizations!!!
{   ///// THIS VERSION ONLY WORKS WHEN FILTERING ENTIRE RECT AT ONCE!!!
    ///// OOPS- That's NFG for the proxy if it's clipped -- so in addPadding,
    /////     we set the pad amount astronomically high to always grab the
    /////     entire rect (assuming DONT_USE_MASK is set).
    register int x = x0, y = y0, z = z0;
    
    if (z >= 0 && z < fmc.planesWithoutAlpha) //3 //gStuff->planes //gStuff->inHiPlane
    {   // get an rgb pixel

        // pin the coordinates (i.e., replicate edges)...
        //Added by Harald Heim, Feb 13, 02

    //#ifndef HYPERTYLE
	#if !defined(HYPERTYLE) && !defined(PLUGINGALAXY)
        if (x < 0) x = 0; else if (x >= fmc.X) x = fmc.X - 1;
        if (y < 0) y = 0; else if (y >= fmc.Y) y = fmc.Y - 1;
    #else
        switch (edgeMode)
        {
            case 0:
                if (x < 0) x = 0; else if (x >= fmc.X) x = fmc.X - 1;
                if (y < 0) y = 0; else if (y >= fmc.Y) y = fmc.Y - 1;
                break;

            case 1:
                if (x < 0 || y < 0 || x >= fmc.X || y >= fmc.Y)
                    return 0;
				break;			//was missing!!
        
            case 2:
                //if (x < 0) x = (fmc.X - 1) + x;				/// or use MOD ???
                //else if (x >= fmc.X) x = (x -(fmc.X-1)) % fmc.X;
                //if (y < 0) y = (fmc.Y - 1) + y;
                //else if (y >= fmc.Y) y = (y -(fmc.Y-1)) % fmc.Y;
				if (x < 0) x = (fmc.X-1) + x%(fmc.X-1);
                else if (x >= fmc.X) x = x % (fmc.X-1);
                if (y < 0) y = (fmc.Y-1) + y%(fmc.Y-1);
                else if (y >= fmc.Y) y = y % (fmc.Y-1);
                break;
            
            case 3: //two reflection modes, one of period 2N, other of period 2N-2 (as here)?
                //if (x < 0) x = - x;
                //else if (x >= fmc.X) x = (fmc.X - 1) - (x - (fmc.X - 1));       //optimize?
                //if (y < 0) y = - y;
                //else if (y >= fmc.Y) y = (fmc.Y - 1) - (y - (fmc.Y - 1));
				if (x < 0) x = - x%(fmc.X-1);
                else if (x >= fmc.X) x = (fmc.X - 1) - x%(fmc.X-1);
				if (y < 0) y = - y%(fmc.Y-1);
                else if (y >= fmc.Y) y = (fmc.Y - 1) - y%(fmc.Y-1);
                break;
        }//end switch
    #endif

        return ((unsigned8 *)MyAdjustedInData)
            [(x-fmc.x_start)*fmc.srcColDelta + (y-fmc.y_start)*fmc.srcRowDelta + z];
        // note that srcPlaneDelta is always 1
    }

    else if (z == 3 || z < fmc.planes) //==3
    {   // get a mask pixel
#if 1
        // pin the coordinates (i.e., replicate edges)...
        //Added by Harald Heim, Jul 2, 02

    //#ifndef HYPERTYLE
	#if !defined(HYPERTYLE) && !defined(PLUGINGALAXY)
        if (x < 0) x = 0; else if (x >= fmc.X) x = fmc.X - 1;
        if (y < 0) y = 0; else if (y >= fmc.Y) y = fmc.Y - 1;
    #else
        switch (edgeMode)
        {
            case 0:
                if (x < 0) x = 0; else if (x >= fmc.X) x = fmc.X - 1;
                if (y < 0) y = 0; else if (y >= fmc.Y) y = fmc.Y - 1;
                break;

            case 1:
                if (x < 0 || y < 0 || x >= fmc.X || y >= fmc.Y)
                    return 0;
				break;			//was missing!!
        
            case 2:
                //if (x < 0) x = (fmc.X - 1) + x;				/// or use MOD ???
                //else if (x >= fmc.X) x = (x -(fmc.X-1)) % fmc.X;
                //if (y < 0) y = (fmc.Y - 1) + y;
                //else if (y >= fmc.Y) y = (y -(fmc.Y-1)) % fmc.Y;
				if (x < 0) x = (fmc.X-1) + x%(fmc.X-1);
                else if (x >= fmc.X) x = x % (fmc.X-1);
                if (y < 0) y = (fmc.Y-1) + y%(fmc.Y-1);
                else if (y >= fmc.Y) y = y % (fmc.Y-1);
                break;
            
            case 3: //two reflection modes, one of period 2N, other of period 2N-2 (as here)?
                //if (x < 0) x = - x;
                //else if (x >= fmc.X) x = (fmc.X - 1) - (x - (fmc.X - 1));       //optimize?
                //if (y < 0) y = - y;
                //else if (y >= fmc.Y) y = (fmc.Y - 1) - (y - (fmc.Y - 1));
				if (x < 0) x = - x%(fmc.X-1);
                else if (x >= fmc.X) x = (fmc.X - 1) -x%(fmc.X-1);
				if (y < 0) y = - y%(fmc.Y-1);
                else if (y >= fmc.Y) y = (fmc.Y - 1) -y%(fmc.Y-1);
                break;
        }//end switch
    #endif
#else
        // alpha outside image/selection is assumed 0...
        if (x < 0 || x >= fmc.X || y < 0 || y >= fmc.Y)
        {   // pin the coordinates (return 0 if outside, NOT replicate edges!!!)
            return 0;
        }
        else
#endif

        //Causes problems when reading the alpha of greyscale images, so z==3 was added
        if (gNoMask && z==3)
        {   // entire filter rect is presumed selected
            return 255;
        }

//Commented out by Harald Heim, Feb 09, 2002
/*
#ifndef DONT_USE_MASK
        else if (gStuff->haveMask)
        {   // get value from mask data
            return ((unsigned8 *)MyAdjustedMaskData)
                [(x-fmc.x_start)*fmc.mskColDelta + (y-fmc.y_start)*fmc.mskRowDelta];
        }
#endif
*/
        //else
        //{   // get value from alpha channel
            return ((unsigned8 *)MyAdjustedInData)
                [(x-fmc.x_start)*fmc.srcColDelta + (y-fmc.y_start)*fmc.srcRowDelta + z]; //3 //gStuff->planes
        //}
    }

    else
    {   // bad channel number??
        return 0;   // ???
    }
} /*fm_src*/

int src16(const int x0, const int y0, const int z0)
// Must not modify its input parameters due to cnvX/Y in-line optimizations!!!
{   // 16-bit version.
    ///// THIS VERSION ONLY WORKS WHEN FILTERING ENTIRE RECT AT ONCE!!!
    ///// OOPS- That's NFG for the proxy if it's clipped -- so in addPadding,
    /////     we set the pad amount astronomically high to always grab the
    /////     entire rect (assuming DONT_USE_MASK is set).
    register int x = x0, y = y0, z = z0;

    if (z >= 0 && z < fmc.planesWithoutAlpha) //3 //gStuff->planes
    {   // get an rgba... pixel
        // pin the coordinates (i.e., replicate edges)...
        
    //#ifndef HYPERTYLE
	#if !defined(HYPERTYLE) && !defined(PLUGINGALAXY)
        if (x < 0) x = 0; else if (x >= fmc.X) x = fmc.X - 1;
        if (y < 0) y = 0; else if (y >= fmc.Y) y = fmc.Y - 1;
    #else
		switch (edgeMode)
        {
            case 0:
                if (x < 0) x = 0; else if (x >= fmc.X) x = fmc.X - 1;
                if (y < 0) y = 0; else if (y >= fmc.Y) y = fmc.Y - 1;
                break;

            case 1:
                if (x < 0 || y < 0 || x >= fmc.X || y >= fmc.Y)
                    return 0;
        
           case 2:
                //if (x < 0) x = (fmc.X - 1) + x;				/// or use MOD ???
                //else if (x >= fmc.X) x = (x -(fmc.X-1)) % fmc.X;
                //if (y < 0) y = (fmc.Y - 1) + y;
                //else if (y >= fmc.Y) y = (y -(fmc.Y-1)) % fmc.Y;
				if (x < 0) x = (fmc.X-1) + x%(fmc.X-1);
                else if (x >= fmc.X) x = x % (fmc.X-1);
                if (y < 0) y = (fmc.Y-1) + y%(fmc.Y-1);
                else if (y >= fmc.Y) y = y % (fmc.Y-1);
                break;
            
            case 3: //two reflection modes, one of period 2N, other of period 2N-2 (as here)?
                //if (x < 0) x = - x;
                //else if (x >= fmc.X) x = (fmc.X - 1) - (x - (fmc.X - 1));       //optimize?
                //if (y < 0) y = - y;
                //else if (y >= fmc.Y) y = (fmc.Y - 1) - (y - (fmc.Y - 1));
				if (x < 0) x = - x%(fmc.X-1);
                else if (x >= fmc.X) x = (fmc.X - 1) -x%(fmc.X-1);
				if (y < 0) y = - y%(fmc.Y-1);
                else if (y >= fmc.Y) y = (fmc.Y - 1) -y%(fmc.Y-1);
                break;

        }//end switch
    #endif

        return *(unsigned16 *)(((unsigned8 *)MyAdjustedInData) +
                               (((x-fmc.x_start)*fmc.srcColDelta + z) << 1) +
                               (y-fmc.y_start)*fmc.srcRowDelta);
    }


    // Must be upated for Photoshop CS where we have transparency in 16bit mode!!!

    else if (z == 3 || z < fmc.planes) //==3 //gStuff->planes
    {   // get a fictitious alpha channel pixel
        // alpha outside image/selection is assumed 0...
        //if (x < 0 || x >= fmc.X || y < 0 || y >= fmc.Y)
        //{   // pin the coordinates (return 0 if outside, NOT replicate edges!!!)
        //    return 0;
        //}
        //else {//still needs to be tested
        //#ifndef HYPERTYLE
		#if !defined(HYPERTYLE) && !defined(PLUGINGALAXY)
            if (x < 0) x = 0; else if (x >= fmc.X) x = fmc.X - 1;
		    if (y < 0) y = 0; else if (y >= fmc.Y) y = fmc.Y - 1;
        #else
            switch (edgeMode)
			{
				case 0:
					if (x < 0) x = 0; else if (x >= fmc.X) x = fmc.X - 1;
					if (y < 0) y = 0; else if (y >= fmc.Y) y = fmc.Y - 1;
					break;

				case 1:
					if (x < 0 || y < 0 || x >= fmc.X || y >= fmc.Y)
						return 0;
        
				case 2:
					//if (x < 0) x = (fmc.X - 1) + x;				/// or use MOD ???
					//else if (x >= fmc.X) x = (x -(fmc.X-1)) % fmc.X;
					//if (y < 0) y = (fmc.Y - 1) + y;
					//else if (y >= fmc.Y) y = (y -(fmc.Y-1)) % fmc.Y;
					if (x < 0) x = (fmc.X-1) + x%(fmc.X-1);
					else if (x >= fmc.X) x = x % (fmc.X-1);
					if (y < 0) y = (fmc.Y-1) + y%(fmc.Y-1);
					else if (y >= fmc.Y) y = y % (fmc.Y-1);
					break;
	            
				case 3: //two reflection modes, one of period 2N, other of period 2N-2 (as here)?
					//if (x < 0) x = - x;
					//else if (x >= fmc.X) x = (fmc.X - 1) - (x - (fmc.X - 1));       //optimize?
					//if (y < 0) y = - y;
					//else if (y >= fmc.Y) y = (fmc.Y - 1) - (y - (fmc.Y - 1));
					if (x < 0) x = - x%(fmc.X-1);
					else if (x >= fmc.X) x = (fmc.X - 1) -x%(fmc.X-1);
					if (y < 0) y = - y%(fmc.Y-1);
					else if (y >= fmc.Y) y = (fmc.Y - 1) -y%(fmc.Y-1);
					break;

			}//end switch
        #endif

			//Causes problems when reading the alpha of greyscale images, so z==3 was added
			if (gNoMask && z==3)
			{   // entire filter rect is presumed selected
				return 32768;
			}
			
			return *(unsigned16 *)(((unsigned8 *)MyAdjustedInData) +
                               (((x-fmc.x_start)*fmc.srcColDelta + z) << 1) + //3 //+gStuff->planes
                               (y-fmc.y_start)*fmc.srcRowDelta);
            //return  65535; //0x8000;  // assume max opacity
        //}
    }
    else
    {   // invalid channel number
        return 0;
    }
} /*fm_src16*/



int fast_src (int x, int y, int z){

    if (x < 0) x = 0; else if (x >= fmc.X) x = fmc.X - 1;
    if (y < 0) y = 0; else if (y >= fmc.Y) y = fmc.Y - 1;
    
    if (fmc.imageMode<10)
        return ((unsigned8 *)MyAdjustedInData) [(x-fmc.x_start)*fmc.srcColDelta + (y-fmc.y_start)*fmc.srcRowDelta + z];
    else
        return *(unsigned16 *)(((unsigned8 *)MyAdjustedInData) + (((x-fmc.x_start)*fmc.srcColDelta + z) << 1) + (y-fmc.y_start)*fmc.srcRowDelta);
}


int fast_src3 (int x, int y, int z){

    if (x < 0) x = - x%(fmc.X-1);
	else if (x >= fmc.X) x = (fmc.X - 1) - x%(fmc.X-1);
	if (y < 0) y = - y%(fmc.Y-1);
    else if (y >= fmc.Y) y = (fmc.Y - 1) - y%(fmc.Y-1);
    
    if (fmc.imageMode<10)
        return ((unsigned8 *)MyAdjustedInData) [(x-fmc.x_start)*fmc.srcColDelta + (y-fmc.y_start)*fmc.srcRowDelta + z];
    else
        return *(unsigned16 *)(((unsigned8 *)MyAdjustedInData) + (((x-fmc.x_start)*fmc.srcColDelta + z) << 1) + (y-fmc.y_start)*fmc.srcRowDelta);
}

int fast_pget3 (int x, int y, int z){

    if (x < 0) x = - x%(fmc.X-1);
	else if (x >= fmc.X) x = (fmc.X - 1) - x%(fmc.X-1);
	if (y < 0) y = - y%(fmc.Y-1);
    else if (y >= fmc.Y) y = (fmc.Y - 1) - y%(fmc.Y-1);

	if (fmc.imageMode<10)
		return ((unsigned8 *)gStuff->outData)
			[(x-fmc.x_start)*fmc.dstColDelta + (y-fmc.y_start)*fmc.dstRowDelta + z];
	else
		return *(unsigned16 *)  ( ((unsigned8 *)gStuff->outData) +
			(((x-fmc.x_start)*fmc.dstColDelta + z)<<1 ) + (y-fmc.y_start)*fmc.dstRowDelta  );
}

int fast_t3get3 (int x, int y, int z){

    if (x < 0) x = - x%(fmc.X-1);
	else if (x >= fmc.X) x = (fmc.X - 1) - x%(fmc.X-1);
	if (y < 0) y = - y%(fmc.Y-1);
    else if (y >= fmc.Y) y = (fmc.Y - 1) - y%(fmc.Y-1);

	if (fmc.imageMode<10)
		return fmc.t3buf[(y*fmc.X + x)*fmc.Z + z];
	else
		return *(unsigned16 *)(((unsigned8 *)fmc.t3buf) + 2*((y*fmc.X + x)*fmc.Z + z));
}

int fast_t4get3 (int x, int y, int z){

    if (x < 0) x = - x%(fmc.X-1);
	else if (x >= fmc.X) x = (fmc.X - 1) - x%(fmc.X-1);
	if (y < 0) y = - y%(fmc.Y-1);
    else if (y >= fmc.Y) y = (fmc.Y - 1) - y%(fmc.Y-1);

	if (fmc.imageMode<10)
		return fmc.t4buf[(y*fmc.X + x)*fmc.Z + z];
	else
		return *(unsigned16 *)(((unsigned8 *)fmc.t4buf) + 2*((y*fmc.X + x)*fmc.Z + z));
}

int fast_frad3(double d, double m, int z)
{
    return fast_src3((int)fm_fr2x(d,m) + (fmc.X/2),
                   (int)fm_fr2y(d,m) + (fmc.Y/2),
                   z);
}
int fast_fpgetr3(double d, double m, int z)
{
    return fast_pget3((int)fm_fr2x(d,m) + (fmc.X/2),
                   (int)fm_fr2y(d,m) + (fmc.Y/2),
                   z);
}
int fast_ft3getr3(double d, double m, int z)
{
    return fast_t3get3((int)fm_fr2x(d,m) + (fmc.X/2),
                 (int)fm_fr2y(d,m) + (fmc.Y/2),
                    z);
}
int fast_ft4getr3(double d, double m, int z)
{
    return fast_t4get3((int)fm_fr2x(d,m) + (fmc.X/2),
                 (int)fm_fr2y(d,m) + (fmc.Y/2),
                    z);
}





int r2x(int d, int m)
{
    return (int)((double)m * cos( (twopi/1024.0)*(double)d ) + 0.0); // trunc or round???
}

int r2y(int d, int m)
{
    return (int)((double)m * sin( (twopi/1024.0)*(double)d ) + 0.0); // trunc or round???
}

double fm_fr2x(double d, double m)
{
    return m * cos( (twopi/1024.0) * d );
}
double fm_fr2y(double d, double m)
{
	return m * sin( (twopi/1024.0) * d );
}


int rad(int d, int m, int z)
{
    return fmc.src(r2x(d,m) + (fmc.X/2),
                   r2y(d,m) + (fmc.Y/2),
                   z);
}

int pgetr(int d, int m, int z)
{
    return fmc.pget(r2x(d,m) + (fmc.X/2),
                   r2y(d,m) + (fmc.Y/2),
                   z);
}

int frad(double d, double m, int z)
{
    return fmc.src((int)fm_fr2x(d,m) + (fmc.X/2),
                   (int)fm_fr2y(d,m) + (fmc.Y/2),
                   z);
}
int fpgetr(double d, double m, int z)
{
    return fmc.pget((int)fm_fr2x(d,m) + (fmc.X/2),
                   (int)fm_fr2y(d,m) + (fmc.Y/2),
                   z);
}

int psetr(int d, int m, int z, int val)
{
    return fmc.pset(r2x(d,m) + (fmc.X/2),
                   r2y(d,m) + (fmc.Y/2),
                   z, val);
}

int tgetr(int d, int m, int z)
{
    return fmc.tget(r2x(d,m) + (fmc.X/2),
                   r2y(d,m) + (fmc.Y/2),
                   z);
}

int tsetr(int d, int m, int z, int val)
{
    return fmc.tset(r2x(d,m) + (fmc.X/2),
                   r2y(d,m) + (fmc.Y/2),
                   z, val);
}

int t2getr(int d, int m, int z)
{
    return fmc.t2get(r2x(d,m) + (fmc.X/2),
                    r2y(d,m) + (fmc.Y/2),
                    z);
}

int t2setr(int d, int m, int z, int val)
{
    return fmc.t2set(r2x(d,m) + (fmc.X/2),
                    r2y(d,m) + (fmc.Y/2),
                    z, val);
}

int t3getr(int d, int m, int z)
{
    return fmc.t3get(r2x(d,m) + (fmc.X/2),
                    r2y(d,m) + (fmc.Y/2),
                    z);
}

int t3setr(int d, int m, int z, int val)
{
    return fmc.t3set(r2x(d,m) + (fmc.X/2),
                    r2y(d,m) + (fmc.Y/2),
                    z, val);
}

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
    //benchmark = 6.1
    return (int)( (256.0 * 0.63661977236758134308) * atan2((double)y, (double)x) );
#endif
#endif
}

int c2m(int x, int y)
{
    //FF benchmark = 11.50 / 6.25 -- uses precomputed table for x/y!!!
#if 0
    //benchmark = 15.10
    //advantage: doesn't easily overflow -- IF convert x, y to double first!!!
    return (int)( sqrt((double)(x*x + y*y)) );
#elif 1
    //benchmark = 12.62 / 8.47
    return isqrt(x*x + y*y);
#elif 0
    //benchmark = 12.34 / 8.25
    register unsigned int v = x*x + y*y;
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
#else
    //benchmark = 15.81 for 2 N-R's, 18.71 for 3 N-R's.
    int d = abs(x) + abs(y);
    int h;
    if (d == 0) return 0;
    h = x*x + y*y;      //easily overflows!
    d = (h/d + d) >> 1; //1st N-R iteration
    d = (h/d + d) >> 1; //2nd N-R iteration
    //d = (h/d + d) >> 1; //3rd N-R iteration
    return d;
#endif
}

double fm_fc2d(double x, double y)
{
    return 256.0 * 0.63661977236758134308 * atan2(y, x);
}

double fm_fc2m(double x, double y)
{
    return sqrt(x*x + y*y);
}

int get(int i)
{
    return fmc.cell[i & (N_CELLS-1)];
}

int put(int v, int i)
{
    return fmc.cell[i & (N_CELLS-1)] = v;
}

int cnv(int m11, int m12, int m13,
                  int m21, int m22, int m23,
                  int m31, int m32, int m33,
                  int d)
{
    int x = fmc.x;
    int y = fmc.y;
    int z = fmc.z;

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

/********* FF+ built-in functions ***********/


INT_PTR pointer_to_buffer(int a, int x, int y, int z)
{
    if (a==0) //Input buffer
    {
        return (INT_PTR)MyAdjustedInData +(x-fmc.x_start)*fmc.srcColDelta + (y-fmc.y_start)*fmc.srcRowDelta + z;
    }
    else if (a==1) //Temp1 buffer
    {
        return (INT_PTR)fmc.tbuf + ((y-fmc.y_start)*fmc.columns + (x-fmc.x_start))*fmc.Z + z;
    }
    else if (a==2) //Temp2 buffer
    {
        return (INT_PTR)fmc.t2buf + ((y-fmc.y_start)*fmc.columns + (x-fmc.x_start))*fmc.Z + z;
    }
    else if (a==3) //Output buffer
    {
        return (INT_PTR)gStuff->outData + (x-fmc.x_start)*fmc.dstColDelta + (y-fmc.y_start)*fmc.dstRowDelta + z;
    }
    else if (a==4) //Temp3 buffer
    {
        return (INT_PTR)fmc.t3buf + ((y-fmc.y_start)*fmc.columns + (x-fmc.x_start))*fmc.Z + z;
    }
    else if (a==5) //Temp4 buffer
    {
        return (INT_PTR)fmc.t4buf + ((y-fmc.y_start)*fmc.columns + (x-fmc.x_start))*fmc.Z + z;
	}

	return false;

}


//function added by Harald Heim on Feb 5, 2002
int phaseshift(int a, int b) 
    //a=Color Value, b=Shift Value from 0 to 512
{   
    if (b < 0) b=0;
    else if (b > 512) b=512;

    if (b==0)
        return a;
    else
        //return 255 - scl( fm_sin( scl(a,0,255,256-b*4,768+b*4) ) ,-512,511,0,255 );
        return 255 - ( fm_sin( scl(a,0,255,256-b*4,768+b*4) ) +512 ) / 4;

}


//This function is NOT THREAD_SAFE!!!!
int blend(int a, int b, int z, int m, int r) 
    //a=Bottom Color Value, b=Top Color Value, m=Blend Mode, r = blending ratio from 0 (bottom) to 255 (top)
{   
    //THREAD_LOCAL 
    static int noise, col;
    int calc, scaled;
	

    if (a < 0) a=0;	//afh 01-may-2008
    else if (a > mode_maxVal) a=mode_maxVal;	//afh 01-may-2008
    if (b < 0) b=0;
    else if (b > mode_maxVal) b=mode_maxVal;
    if (r < 0) r=0;
    else if (r > mode_maxVal) r=mode_maxVal;

    if (r==255 && m<4)
        return b;
    else if (r==0)
        return a;
    else
    {
        
        switch (m)
        {
            case 0://Normal
                if (r==255)
					return b;
				else
					return ((255-r)*a + r*b)/255;
					//return mix(a,b,255-r,255);

            case 1: //Dissolve
                if (z==0) noise=rnd(0,255);
                return r < noise ? a : b;
				//return r < rnd(0,255) ? a : b;

            case 2: //Threshold
                if (z==0) col = a;
                return col < (255-r)*mode_bitMultiply ? a : b;

            case 3: //Threshold 2
                if (z==0) col = b;
                return col < (255-r)*mode_bitMultiply ? a : b;

            case 4: //Multiply
                calc = a*b/mode_maxVal;
                break;

            case 5: //Screen
                calc = mode_maxVal-((mode_maxVal-a)*(mode_maxVal-b))/mode_maxVal;
                break;

            case 6: //Overlay
                calc = a<mode_midVal ? (2*a*b)/mode_maxVal : mode_maxVal-(2*(mode_maxVal-a)*(mode_maxVal-b))/mode_maxVal;
                break;

            case 7: //Soft Light
                //scaled = scl(a,0,maxVal,64,192);
                scaled = a/2 + mode_quarVal;
                calc =  a<mode_midVal ? (2*b*scaled)/mode_maxVal : mode_maxVal-2*(mode_maxVal-scaled)*(mode_maxVal-b)/mode_maxVal;
                break;
            
            case 8: //Hard Light -> Same as overlay!
                calc = b<mode_midVal ? (2*a*b)/mode_maxVal : mode_maxVal-(2*(mode_maxVal-a)*(mode_maxVal-b))/mode_maxVal;
                break;

            case 9://Dodge
                //calc = maxVal*a/max(1,maxVal-b);
                calc = b >= mode_maxVal? mode_maxVal : min(a * mode_maxVal / (mode_maxVal - b), mode_maxVal);
                //calc = (a * 256 / (256 - b)) - 1; //Faster Alternative
                break;

            case 10: //Burn
                //calc = maxVal-(maxVal*(maxVal-a)/(b+1));
                calc = b <= 0? 0 : max(mode_maxVal - ((mode_maxVal - a) * mode_maxVal / b), 0);
                break;

            case 11://Darken
                calc = min(a,b);
                break;

            case 12: //Lighten
                calc = max(a,b);
                break;

            case 13://Exclusion
                calc = mode_maxVal-(((mode_maxVal-a)*(mode_maxVal-b)/mode_maxVal)+(a*b/mode_maxVal));
                break;

            case 14://Difference
                calc = dif(a,b);
                break;

            case 15://Negative Difference 1
                calc = mode_maxVal-dif(a,mode_maxVal-b);
                break;

            case 16://Negative Difference 2
                calc = dif(mode_maxVal-a,b);
                break;

            case 17://Subtract
                calc = a-b;
                break;

            case 18://Add
                calc = a+b;
                break;

            case 19://Expose
                calc = a+a-b;
				break;

			/*case 20://Soft Light 2
                //calc = b > mode_midVal ? 
				//		mode_maxVal - (mode_maxVal-a) * (mode_maxVal-(b-mode_midVal)) / mode_maxVal :
				//	    ((2*b-mode_maxVal) * (a-a*a/mode_maxVal))/ mode_maxVal  + a;
				{
					double value;
					double cb = (double)a / mode_maxVal;
					double cs = (double)b / mode_maxVal;
					if (cs <= .5) {
						value = cb - (1-2*cs) * cb * (1-cb);
					} else {
						value = cb <= .25 ? ((16*cb-12)*cb+4)*cb : sqrt(cb);
						value = cb + (2*cs-1) * (value-cb);
					}
					calc = (int)(value * mode_maxVal);
				}*/

        }//end switch


        if (r==255)
			return calc;
		else
			return ((255-r)*a + r*calc)/255;
			//return mix(a, calc, 255-r,255);

	}

}

//Used by Plugin Galaxy

#include <math.h> 

#define rnd(a,b) (rand()%(b-a)+a)
#define dif(a,b) (((a)-(b)>=0)?(a)-(b):(b)-(a))
#define sqr(x) ( x > 0 ? (int)(sqrt((double)x) + 0.0) : 0 )

void clipColor(int * red, int *green, int * blue){
	int l = (int)(0.3 * *red + 0.59 * *green + 0.11 * *blue);
	int n = min(*red,min(*green,*blue));
	int x = max(*red,max(*green,*blue));
	if (n < 0){
		*red = l + (((*red-l)*l)/(l-n));
		*green = l + (((*green-l)*l)/(l-n));
		*blue = l + (((*blue-l)*l)/(l-n));
	}
	if (x > mode_maxVal){
		*red = l + (((*red-l)*(mode_maxVal-l))/(x-l));
		*green = l + (((*green-l)*(mode_maxVal-l))/(x-l));
		*blue = l + (((*blue-l)*(mode_maxVal-l))/(x-l));
	}
}

void setLum(int * red, int *green, int * blue, int l){
	int d = l - (int)(0.3 * *red + 0.59 * *green + 0.11 * *blue);
	*red += d;
	*green += d;
	*blue += d;
	clipColor(red,green,blue);
}

#define PSWAP(x,y) {int* t;t=x;x=y;y=t;}

void setSat(int * red, int *green, int * blue, int s){
	int * cmax = red;
	int * cmid = green;
	int * cmin = blue;
	
	//Sort
	if (*cmax < *cmin) PSWAP(cmax,cmin)
	if (*cmin > *cmid) PSWAP(cmin,cmid)
	if (*cmax < *cmid) PSWAP(cmax,cmid)

	if (*cmax > *cmin){
		*cmid = (((*cmid - *cmin)*s)/(*cmax - *cmin));
		*cmax = s;
	} else 
		*cmid = *cmax = 0;
	*cmin = 0;
}

#undef PSWAP
//#define SWAP(x,y) x^=y;y^=x;x^=y

//a=Bottom Color Value, b=Top Color Value, m=Blend Mode, r = blending ratio from 0 (bottom) to 255 (top)
int blend2(int a[4], int b[4], int calc[4], int planes, int mode, int ratio)
{   
	int i, value;

	for (i=0; i<planes; i++){
		if (a[i] < 0) a[i]=0; else if (a[i] > mode_maxVal) a[i]=mode_maxVal;
		if (b[i] < 0) b[i]=0; else if (b[i] > mode_maxVal) b[i]=mode_maxVal;
	}
	
	if (ratio==0){
		
		return true; //No need to do anything

	} else {

        switch (mode){

            case 0://Normal
				for (i=0; i<planes; i++) 
					calc[i] = b[i];
				break;

            case 1: //Dissolve
                value = rand()%255;
                for (i=0; i<planes; i++) 
					calc[i] = ratio < value ? a[i] : b[i];
				break;


			case 3://Darken
                for (i=0; i<planes; i++) 
					calc[i] = min(a[i],b[i]);
                break;

            case 4: //Multiply
                for (i=0; i<planes; i++) 
					calc[i] = a[i]*b[i]/mode_maxVal;
                break;

			case 5: //Color Burn
				for (i=0; i<planes; i++) 
					calc[i] = b[i] <= 0? 0 : max(mode_maxVal - ((mode_maxVal - a[i]) * mode_maxVal / b[i]), 0);
				break;
			
			case 6: //Linear Burn
				for (i=0; i<planes; i++){
					calc[i] = a[i] + b[i] - mode_maxVal;
					if (calc[i] < 0) calc[i] = 0;
				}
				break;
			
			case 7: //Darker Color
				if (a[0]+a[1]+a[2] > b[0]+b[1]+b[2])
					for (i=0; i<planes; i++) 
						calc[i] = b[i];
				else
					for (i=0; i<planes; i++) 
						calc[i] = a[i];
				break;


            case 9: //Lighten
                for (i=0; i<planes; i++) 
					calc[i] = max(a[i],b[i]);
                break;

			case 10: //Screen
                for (i=0; i<planes; i++) 
					//calc[i] = mode_maxVal-((mode_maxVal-a[i])*(mode_maxVal-b[i]))/mode_maxVal;
					calc[i] = a[i] + b[i] - (a[i] * b[i])/mode_maxVal;
                break;

			case 11: //Color Dodge
				for (i=0; i<planes; i++) 
					calc[i] = b[i] == mode_maxVal? mode_maxVal : min(a[i] * mode_maxVal / (mode_maxVal - b[i]), mode_maxVal);
				break;

			case 12: //Linear Dodge (Add)
				for (i=0; i<planes; i++){
					calc[i] = a[i] + b[i];
					if (calc[i] > mode_maxVal) calc[i] = mode_maxVal;
				}
				break;

			case 13: //Lighter Color
				if (a[0]+a[1]+a[2] > b[0]+b[1]+b[2])
					for (i=0; i<planes; i++) 
						calc[i] = a[i];
				else
					for (i=0; i<planes; i++) 
						calc[i] = b[i];
				break;


            case 15: //Overlay
                for (i=0; i<planes; i++) 
					calc[i] = a[i]<mode_midVal ? (2*a[i]*b[i])/mode_maxVal : 
					                              mode_maxVal-(2*(mode_maxVal-a[i])*(mode_maxVal-b[i]))/mode_maxVal;
                break;

            case 16: //Soft Light
			{
				double value,cb,cs;
				for (i=0; i<planes; i++){ 
					//Almost identical to Photoshop's Soft Light
					//calc[i] = b[i] > mode_midVal ? mode_maxVal - (mode_maxVal-a[i]) * (mode_maxVal-(b[i]-mode_midVal)) / mode_maxVal :
					//					          ((2*b[i]-mode_maxVal) * (a[i]-a[i]*a[i]/mode_maxVal))/ mode_maxVal  + a[i];
					
					cb = (double)a[i] / mode_maxVal;
					cs = (double)b[i] / mode_maxVal;
					if (cs <= .5) {
						value = cb - (1-2*cs) * cb * (1-cb);
					} else {
						value = cb <= .25 ? ((16*cb-12)*cb+4)*cb : sqrt(cb);
						value = cb + (2*cs-1) * (value-cb);
					}
					calc[i] = (int)(value * mode_maxVal);
				}				
				break;
			}
            case 17: //Hard Light -> Very similar to overlay!
                for (i=0; i<planes; i++) 
					calc[i] = b[i]<mode_midVal ? (2*a[i]*b[i])/mode_maxVal : 
												  mode_maxVal-(2*(mode_maxVal-a[i])*(mode_maxVal-b[i]))/mode_maxVal;
												  //a[i] + (2*b[i]-mode_maxVal) - (a[i] * (2*b[i]-mode_maxVal))/mode_maxVal;
                break;

			case 18: //Vivid Light
				for (i=0; i<planes; i++){
					if (b[i] <= mode_midVal){
						if (b[i]==0) 
							calc[i] = 0;
						else 
							calc[i] = mode_maxVal - (mode_maxVal-a[i])*mode_maxVal / (2*b[i]);
					} else {
						if (b[i]==mode_maxVal)
							calc[i] = mode_maxVal;
						else
							calc[i] = a[i]*mode_maxVal / (2*(mode_maxVal-b[i]));
					}
					if (calc[i] < 0) calc[i] = 0;
					else if (calc[i] > mode_maxVal) calc[i] = mode_maxVal;
				}
				break;

			case 19: //Linear Light
				for (i=0; i<planes; i++){
					calc[i] = a[i] + 2*b[i] - mode_maxVal;
					if (calc[i] < 0) calc[i] = 0;
					else if (calc[i] > mode_maxVal) calc[i] = mode_maxVal;
				}
				break;

			case 20: //Pin Light
				for (i=0; i<planes; i++){
					calc[i] = a[i] < 2*b[i] - mode_maxVal ? 2*b[i] - mode_maxVal: a[i] > 2*b[i] ? 2*b[i] : a[i];
					if (calc[i] < 0) calc[i] = 0;
					else if (calc[i] > mode_maxVal) calc[i] = mode_maxVal;
				}
				break;

			case 21: //Hard Mix
				for (i=0; i<planes; i++) 
					calc[i] = a[i] < mode_maxVal-b[i] ? 0: mode_maxVal;
				break;

			
			case 23: //Difference
                for (i=0; i<planes; i++) 
					calc[i] = dif(a[i],b[i]);
                break;
		
            case 24: //Exclusion
                for (i=0; i<planes; i++) 
					calc[i] = mode_maxVal-(((mode_maxVal-a[i])*(mode_maxVal-b[i])/mode_maxVal)+(a[i]*b[i]/mode_maxVal));
                break;

			case 25: //Subtract
				for (i=0; i<planes; i++){
					calc[i] = a[i]-b[i];
					if (calc[i] < 0) calc[i] = 0;
				}
				break;
		
            case 26: //Divide
				for (i=0; i<planes; i++){
					calc[i] = b[i] == 0? 0: a[i] * mode_maxVal / b[i];
					if (calc[i] > mode_maxVal) calc[i] = mode_maxVal;
				}
                break;


			case 28: //Hue
				for (i=0; i<planes; i++)
					calc[i] = b[i];
				setSat(&calc[0], &calc[1], &calc[2], max(a[0],max(a[1],a[2])) - min(a[0],min(a[1],a[2])) );
				setLum(&calc[0], &calc[1], &calc[2], (int)(0.3*a[0]+0.59*a[1]+0.11*a[2]));				
				break;

			case 29: //Saturation
				for (i=0; i<planes; i++)
					calc[i] = a[i];
				value = (int)(0.3*a[0]+0.59*a[1]+0.11*a[2]);
				setSat(&calc[0], &calc[1], &calc[2], max(b[0],max(b[1],b[2])) - min(b[0],min(b[1],b[2])) );
				setLum(&calc[0], &calc[1], &calc[2], value);
				break;

			case 30: //Color
				for (i=0; i<planes; i++)
					calc[i] = b[i];
				setLum(&calc[0], &calc[1], &calc[2], (int)(0.3*a[0]+0.59*a[1]+0.11*a[2]));
				break;

			case 31: //Luminosity
				for (i=0; i<planes; i++)
					calc[i] = a[i];
				setLum(&calc[0], &calc[1], &calc[2], (int)(0.3*b[0]+0.59*b[1]+0.11*b[2]));
				break;

							
			case 33://Exposure
				for (i=0; i<planes; i++){
					calc[i] = a[i]+a[i]-b[i];
					if (calc[i] < 0) calc[i] = 0;
					else if (calc[i] > mode_maxVal) calc[i] = mode_maxVal;
				}
				break;

			case 34://Overexposure
				for (i=0; i<planes; i++){
					calc[i] = b[i]+b[i]-a[i];
					if (calc[i] < 0) calc[i] = 0;
					else if (calc[i] > mode_maxVal) calc[i] = mode_maxVal;
				}
				break;

			case 35: //Soft Mood -> previously Soft Light
                for (i=0; i<planes; i++){
					value = a[i]/2 + mode_quarVal;
					calc[i] = a[i]<mode_midVal ? (2*b[i]*value)/mode_maxVal : mode_maxVal-2*(mode_maxVal-value)*(mode_maxVal-b[i])/mode_maxVal;
				}
                break;

			case 36: //Hard Mood -> GIMP's Grain Merge (Similar to Overlay, but harder)
				for (i=0; i<planes; i++){
					calc[i] = a[i] + b[i] - mode_midVal;
					if (calc[i] < 0) calc[i] = 0; else if (calc[i] > mode_maxVal) calc[i] = mode_maxVal;
				}
				break;

            case 37://Add
				for (i=0; i<planes; i++)
					calc[i] = a[i]+b[i];
                break;

			case 38://Negative Difference 1
				for (i=0; i<planes; i++)
					calc[i] = mode_maxVal-dif(a[i],mode_maxVal-b[i]);
                break;

            case 39://Negative Difference 2
				for (i=0; i<planes; i++)
					calc[i] = dif(mode_maxVal-a[i],b[i]);
                break;

            case 40: //Threshold
				for (i=0; i<planes; i++)
					calc[i] = a[0] < (255-ratio)*mode_bitMultiply ? a[i] : b[i];
				break;

            case 41: //Threshold 2
				for (i=0; i<planes; i++)
                	calc[i] = b[0] < (255-ratio)*mode_bitMultiply ? a[i] : b[i];
				break;
			
			/*case 41: //Reflect -> Similar to Hard Mix, but softer
				for (i=0; i<planes; i++){
					if (b[i] == mode_maxVal)
					  calc[i] = mode_maxVal;
					else {
					  calc[i] = a[i]*a[i] /(mode_maxVal-b[i]);
					  if (calc[i] > mode_maxVal) calc[i] = mode_maxVal;
					}
				}
				break;*/
			/*case 41: //Phoenix -> colorful
				for (i=0; i<planes; i++)
					calc[i] = min(a[i],b[i]) - max(a[i],b[i]) + mode_maxVal;
				break;*/
			/*case 41: //Stamp -> Similar to Hard Light, but stronger
				for (i=0; i<planes; i++){
					calc[i] = a[i] + 2*b[i] - (mode_maxVal+1);
					if (calc[i] < 0) calc[i] = 0; else if (calc[i] > mode_maxVal) calc[i] = mode_maxVal;
				}
				break;*/
			
        }//end switch


		//Opacity
		if (ratio<255){
			for (i=0; i<planes; i++)
				calc[i] = ((255-ratio)*a[i] + ratio*calc[i])/255;
		}
	}
	
	return true;

}

#undef dif
#undef rnd
#undef sqr



//function added by Harald Heim on Feb 5, 2002
int contrast(int a, int b) 
    //a=Color Value, b=Contrast Value from -127 to 128
{
    if (b < -127) b=-127;	//afh 01-may-2008
    else if (b > 128) b=128;	//afh 01-may-2008

    return scl(a,b,257-b,0,255);
}

//function added by Harald Heim on Feb 5, 2002
int posterize(int a, int b) 
    //a=Color Value, b=Posterize Value/Number of Colors from 0 to 255
{ 
    if (b < 0) b=0;
    else if (b > 255) b=255;

    //Bad formula
    /*
    if (b==0)
        return 0;
    if (b==255)
        return a;
    else
        return scl( scl(a,0,255,0,b) ,0,b,0,255 );
    */

    if (b==0)
        return a;
    else
        return a - (a % b);
}


//function added by Harald Heim on Feb 5, 2002
int saturation(int r, int g, int b, int z, int sat) 
    // r,g,b = color values, z = returned channel, sat = saturation value between -500 and 500
{
    int gray,sat2,channel;

    //Which channel to return?
    if (z==0) channel=r;
    else if (z==1) channel=g;
    else if (z==2) channel=b;

    if (sat==0)
    {
        return channel;
    }
    else
    {
        //test limits
        if (sat < -500) b=-500;
        else if (sat > 500) b=500;

        //Gray value
        //gray= (r+b+g)/3;
        gray =( max(max(r,g),b) + min(min(r,g),b) ) /2;

        if (sat==-500)
        {
            return gray;
        }
        else
        {           
            //Saturate or desaturate?
            if (sat < 0) sat2=(sat/5)+100;
            else sat2=sat+100;

            return ( (100-sat2)*gray + sat2*channel )/100;
        }

    }

}

//function added by Harald Heim on Feb 6, 2002
int msk(int x, int y)
{
        if (x < 0 || x >= fmc.X || y < 0 || y >= fmc.Y)
        {   // pin the coordinates (return 0 if outside, NOT replicate edges!!!)
            return 0;
        }
        else if (gStuff->haveMask)
        { // get value from selection mask

            //if (gStuff->imageMode > 9
            //) {
            //    return *(unsigned16 *)(((unsigned8 *)MyAdjustedMaskData) +
            //                   (((x-fmc.x_start)*fmc.mskColDelta)) +  // <<1
            //                   (y-fmc.y_start)*fmc.mskRowDelta);    
            //} else {    
           
                return ((unsigned8 *)MyAdjustedMaskData)
                    [(x-fmc.x_start)*fmc.mskColDelta + (y-fmc.y_start)*fmc.mskRowDelta];    
            //}
        }
        /*else if (!gNoMask)
        {   // get value from alpha channel
            return ((unsigned8 *)MyAdjustedInData)
                [(x-fmc.x_start)*fmc.srcColDelta + (y-fmc.y_start)*fmc.srcRowDelta + 3];
        }*/
        else
        {
            //if (gStuff->imageMode > 9
            //  ) {
            //    return 65535; //32768;
            //}else {
                return 255;
            //}

        }
}


/**
//function added by Harald Heim on Feb 6, 2002
int egw(int b, int t, int v)
{
    int swapvalue;

    if (t==0 && b==0)
        return 0;
    if (b==t)
        return b;

    //added by Harald Heim on Mar 8, 2002
    b=abs(b);	//afh 01-may-2008
    t=abs(t);	//afh 01-may-2008
    if (b>t)
    {
        swapvalue = t;
        t = b;
        b = swapvalue;
    }

    if (v > t)
        return b + (v-b) % (t-b) ;
    else if (v < b)
        return t - (b - v) % (t-b);
    else
        return v;

}/**/

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
int egw(int edge_a, int edge_b, int value)
{
	int swapvalue;

	//RC    if (edge_b == 0 && edge_a == 0)  return 0;   redundant with next statement??
	if (edge_a == edge_b) return edge_a;

	if (edge_a < 0) edge_a = -edge_a;
	if (edge_b < 0) edge_b = -edge_b;
	if (edge_a > edge_b)
	{
		swapvalue 	= edge_b;
		edge_b 		= edge_a;
		edge_a 		= swapvalue;
	}

	if (value > edge_b)
		return edge_a + (value - edge_a) % (edge_b - edge_a) ;
	else if (value < edge_a)
		return edge_b - (edge_a - value) % (edge_b - edge_a);
	else
		return value;
}


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
int egm(int edge_a, int edge_b, int value)
{
	int swapvalue;

	if (edge_a == edge_b) return edge_a;

	if (edge_a < 0) edge_a = -edge_a;
	if (edge_b < 0) edge_b = -edge_b;
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

}

/*
//function added by Harald Heim on Feb 10, 2002
int tbuffer_initialize(int i)
{
    if (i==0) 
        Dont_Initialize_Tbuffer = 0; //Tbuffer isn't initialized
    else
        Dont_Initialize_Tbuffer = 1; //Tbuffer is initialized
    
    return true;

}

//function added by Harald Heim on Feb 10, 2002
int t2buffer_initialize(int i)
{
    if (i==0) 
        Dont_Initialize_T2buffer = 0; //T2buffer isn't initialized
    else
        Dont_Initialize_T2buffer = 1; //T2buffer is initialized
    
    return true;

}
*/

//// Use an enum for edge mode, expose to FD???

//function added by Harald Heim on Feb 13, 2002
int set_edge_mode(int mode)
{ 
//mode=0 for regular behaviour
//mode=1 for black border
//mode=2 for edge wrap
//mode=3 for edge mirror
    if (mode < 0 && mode > 3) return false;

    edgeMode = mode;
    return true;
}


int set_bitdepth_mode(int mode)
{ 
    bitDepthMode = mode;

	if (bitDepthMode==16){ 
		mode_maxVal=32768;
		mode_midVal=16384;
		mode_quarVal=8192;
		mode_bitMultiply=128;
	} else {
		mode_maxVal=255;
		mode_midVal=128;
		mode_quarVal=64;
		mode_bitMultiply=1;
	}

    return true;
}

int get_bitdepth_mode()
{ 
    return bitDepthMode;
}

/// Use pluggable variants of psetp() instead of testing a flag at run-time!!!

int set_psetp_mode(int mode)
{ 
//mode=0 for regular behaviour (not possible to set alpha channel to zero) for more performance and avoiding bugs
//mode=1 for always setting the alpha channel value

    if (mode < 0 && mode > 1) return false;

    psetpMode = mode;
    return true;
}




//function added by Harald Heim on Feb 11, 2002
int igetArrayEx(int nr, double x, double y, int z, int mode)
{ 
    int x_int, y_int, x_int1, y_int1;
    double fracx, fracy;
    int pos;

	if (z < 0 || z > ArrayZ[nr]) return 0;
        
    if (mode==0) //Nearest Neighbor
    {
        
        x_int=(int) x;
        y_int=(int) y;

        if (x_int < 0) x_int = 0; else if (x_int >= ArrayX[nr]) x_int = ArrayX[nr] - 1;
        if (y_int < 0) y_int = 0; else if (y_int >= ArrayY[nr]) y_int = ArrayY[nr] - 1;

		pos = (( (y_int+ArrayPadding[nr])*ArrayX[nr] + (x_int+ArrayPadding[nr]) )*ArrayZ[nr] + z);//*ArrayBytes[nr];
		switch(ArrayBytes[nr]) {
			case 1:
				return ((unsigned8 *)Array[nr]) [pos];
			case 2:
				return  ((unsigned16 *)Array[nr]) [pos];
			case 4:
				return  ((unsigned32 *)Array[nr]) [pos];
		}

    } else {
        
        if (mode<4){

			int v1,v2,v3,v4;
			//Cached Values
			static int x_int_old, y_int_old, x_int1_old, y_int1_old,mode_old, edgeMode_old;
			static double fracx_old, fracy_old, x_old, y_old;

            if (x==x_old && y==y_old && mode==mode_old && edgeMode==edgeMode_old)
            {
                //Set cached values
                fracx=fracx_old;
                fracy=fracy_old;
                x_int=x_int_old;
                y_int=y_int_old; 
                x_int1=x_int1_old;
                y_int1=y_int1_old;
            }

            else //Calculate the values
            {

                if (x < 0) x = 0; else if (x >= ArrayX[nr]) x = ArrayX[nr] - 1;
				if (y < 0) y = 0; else if (y >= ArrayY[nr]) y = ArrayY[nr] - 1;

                x_int=(int) x;
                y_int=(int) y;

                x_int1=x_int+1;
                y_int1=y_int+1;

                if (x_int1 >= ArrayX[nr]) x_int1 = (ArrayX[nr] - 1);
                if (y_int1 >= ArrayY[nr]) y_int1 = (ArrayY[nr]- 1);

                fracx=x-x_int;
                fracy=y-y_int;


                if (mode==2) //Bicosine
                {
                    if (fracx==fracy)
                    {
                        fracx= (1.0-cos(fracx*3.1415927))*0.5;
                        fracy=fracx;
                    }
                    else
                    {
                        fracx= (1.0-cos(fracx*3.1415927))*0.5;
                        fracy= (1.0-cos(fracy*3.1415927))*0.5;
                    }
                }
                else if (mode == 1) //Bisquare
                {
                    fracx *= fracx;
                    fracy *= fracy;
                }
            
            }//Calculate not cached

			switch(ArrayBytes[nr]) {
				case 1:
					pos = (( (y_int+ArrayPadding[nr])*ArrayX[nr] + (x_int+ArrayPadding[nr]) )*ArrayZ[nr] + z);
					v1 = ((unsigned8 *)Array[nr]) [pos];
					pos = (( (y_int1+ArrayPadding[nr])*ArrayX[nr] + (x_int+ArrayPadding[nr]) )*ArrayZ[nr] + z);
					v2 = ((unsigned8 *)Array[nr]) [pos];
					pos = (( (y_int+ArrayPadding[nr])*ArrayX[nr] + (x_int1+ArrayPadding[nr]) )*ArrayZ[nr] + z);
					v3 = ((unsigned8 *)Array[nr]) [pos];
					pos = (( (y_int1+ArrayPadding[nr])*ArrayX[nr] + (x_int1+ArrayPadding[nr]) )*ArrayZ[nr] + z);
					v4 = ((unsigned8 *)Array[nr]) [pos];
					break;
				case 2:
					pos = (( (y_int+ArrayPadding[nr])*ArrayX[nr] + (x_int+ArrayPadding[nr]) )*ArrayZ[nr] + z);
					v1 = ((unsigned16 *)Array[nr]) [pos];
					pos = (( (y_int1+ArrayPadding[nr])*ArrayX[nr] + (x_int+ArrayPadding[nr]) )*ArrayZ[nr] + z);
					v2 = ((unsigned16 *)Array[nr]) [pos];
					pos = (( (y_int+ArrayPadding[nr])*ArrayX[nr] + (x_int1+ArrayPadding[nr]) )*ArrayZ[nr] + z);
					v3 = ((unsigned16 *)Array[nr]) [pos];
					pos = (( (y_int1+ArrayPadding[nr])*ArrayX[nr] + (x_int1+ArrayPadding[nr]) )*ArrayZ[nr] + z);
					v4 = ((unsigned16 *)Array[nr]) [pos];
					break;
				case 4:
					pos = (( (y_int+ArrayPadding[nr])*ArrayX[nr] + (x_int+ArrayPadding[nr]) )*ArrayZ[nr] + z);
					v1 = ((unsigned32 *)Array[nr]) [pos];
					pos = (( (y_int1+ArrayPadding[nr])*ArrayX[nr] + (x_int+ArrayPadding[nr]) )*ArrayZ[nr] + z);
					v2 = ((unsigned32 *)Array[nr]) [pos];
					pos = (( (y_int+ArrayPadding[nr])*ArrayX[nr] + (x_int1+ArrayPadding[nr]) )*ArrayZ[nr] + z);
					v3 = ((unsigned32 *)Array[nr]) [pos];
					pos = (( (y_int1+ArrayPadding[nr])*ArrayX[nr] + (x_int1+ArrayPadding[nr]) )*ArrayZ[nr] + z);
					v4 = ((unsigned32 *)Array[nr]) [pos];
					break;  
			}

            //Set static variables for caching
            x_old = x;
            y_old = y;
            mode_old = mode;
            edgeMode_old=edgeMode;
            fracx_old=fracx;
            fracy_old=fracy;
            x_int_old=x_int;
            y_int_old=y_int; 
            x_int1_old=x_int1;
            y_int1_old=y_int1;

            return (int) ((1.0-fracx)* ((1.0-fracy)*v1 + fracy*v2)  + fracx* ((1.0-fracy)*v3 + fracy*v4));
    

        } else if (mode==4) {//bicubic interpolation
        
            //For Bicubic Interpolation 
           double nx,ny,ox3,oy3,nx3,ny3;
           double ix[4];
           int j,k;
           int i[4];
           double x1,x2,x3,x4,y1,y2,y3,y4,y9;
        
           if (x < 0) x = 0; else if (x >= ArrayX[nr]) x = (double)(ArrayX[nr] - 1);
		   if (y < 0) y = 0; else if (y >= ArrayY[nr]) y = (double)(ArrayY[nr] - 1);

           y_int = (int)y;
           x_int = (int)x;

           fracx=x-x_int;
           fracy=y-y_int;

           nx = 1. - fracx;
           ny = 1. - fracy;
           ox3 = (fracx * fracx * fracx) - fracx;
           oy3 = (fracy * fracy * fracy) - fracy;
           nx3 = (nx * nx * nx) - nx;
           ny3 = (ny * ny * ny) - ny;

            
           for (j=-1;j<3;j++){

               y_int1 = y_int + j;

               if (y_int1 < 0) y_int1 = 0;
               if (y_int1 >= ArrayY[nr]) y_int1 = (ArrayY[nr] - 1);
            
               for (k=-1;k<3;k++) {
                    
                    x_int1 = x_int + k;

                    if (x_int1 < 0) x_int1 = 0;
                    if (x_int1 >= ArrayX[nr]) x_int1 = (ArrayX[nr] - 1);
               
					pos = (( (y_int1+ArrayPadding[nr])*ArrayX[nr] + (x_int1+ArrayPadding[nr]) )*ArrayZ[nr] + z);
					switch(ArrayBytes[nr]) {
						case 1: i[k+1] = ((unsigned8 *)Array[nr]) [pos]; break;
						case 2: i[k+1] = ((unsigned16 *)Array[nr]) [pos]; break;
						case 4: i[k+1] = ((unsigned32 *)Array[nr]) [pos];
					}

                }//for k

               
               x1 = (i[2] - i[1]) - (i[1] - i[0]);
               x2 = (i[3] - i[2]) - (i[2] - i[1]);
               x3 = x1 - x2 / 4.;
               x4 = x2 - x1 / 4.;
               ix[j+1] = (fracx * i[2]) + (nx * i[1])  + (((x4 * ox3) + (x3 * nx3)) / 3.75);
           }//for j


           // Y
           y1 = (ix[2] - ix[1]) - (ix[1] - ix[0]);
           y2 = (ix[3] - ix[2]) - (ix[2] - ix[1]);
           y3 = y1 - y2 / 4.;
           y4 = y2 - y1 / 4.;
           y9 = (fracy * ix[2]) + (ny * ix[1])  + (((y4 * oy3) + (y3 * ny3)) / 3.75);

           return (int)y9;

        }//end bicubic

    }// end interpolation methods

    return 0;

} /*fm_iget*/


int iget(double x, double y, int z, int buffer, int mode)
{ 
    int x_int, y_int, x_int1, y_int1;
    double fracx, fracy;
/*
	//Does not work with multi-threading
    //Cached values
    THREAD_LOCAL static int x_int_old, y_int_old, x_int1_old, y_int1_old,mode_old, edgeMode_old;
    THREAD_LOCAL static double fracx_old, fracy_old, x_old, y_old;
*/

    //Info ("x: %f - y: %f",x,y);

    if (z >= 0 && z < fmc.Z)
    {
        
        if (mode==0) //Nearest Neighbor
        {
            
			//x_int=(int) x;
            //y_int=(int) y;
			//x_int=(int)(x>INT_MAX?INT_MAX:x); 
            //y_int=(int)(y>INT_MAX?INT_MAX:y); 
			x_int=(int)(unsigned int)x; //Avoid crash with too large double value becoming negative int
            y_int=(int)(unsigned int)y;

        //#ifndef HYPERTYLE
		#if !defined(HYPERTYLE) && !defined(PLUGINGALAXY)
            if (x_int < 0) x_int = 0; else if (x_int >= fmc.X) x_int = fmc.X - 1;
            if (y_int < 0) y_int = 0; else if (y_int >= fmc.Y) y_int = fmc.Y - 1;
        #else
            switch(edgeMode)
            {
                case 0:
                    if (x_int < 0) x_int = 0; else if (x_int >= fmc.X) x_int = fmc.X - 1;
                    if (y_int < 0) y_int = 0; else if (y_int >= fmc.Y) y_int = fmc.Y - 1;
                    break;

                case 1:
                    if (x_int < 0 || y_int < 0 || x_int >= fmc.X || y_int >= fmc.Y)
                        return 0;
					break;				//was missing!!
                
                case 2:
                    //if (x_int < 0) x_int = (fmc.X - 1) + x_int;
					//else if (x_int >= fmc.X) x_int = (x_int - (fmc.X - 1)) % fmc.X;
                    //if (y_int < 0) y_int = (fmc.Y - 1) + y_int;
                    //else if (y_int >= fmc.Y) y_int = (y_int - (fmc.Y - 1)) % fmc.Y;
					if (x_int < 0) x_int = (fmc.X-1) + x_int%(fmc.X-1);
					else if (x_int >= fmc.X) x_int = x_int % (fmc.X-1);
					if (y_int < 0) y_int = (fmc.Y-1) + y_int%(fmc.Y-1);
					else if (y_int >= fmc.Y) y_int = y_int % (fmc.Y-1);
                    break;

                case 3:
                    //if (x_int < 0) x_int = - x_int;
                    //else if (x_int >= fmc.X) x_int = (fmc.X - 1) - (x_int - (fmc.X - 1));
                    //if (y_int < 0) y_int = - y_int;
                    //else if (y_int >= fmc.Y) y_int = (fmc.Y - 1) - (y_int - (fmc.Y - 1));
					if (x_int < 0) x_int = - x_int%(fmc.X-1);
                    else if (x_int >= fmc.X) x_int = (fmc.X - 1) -x_int%(fmc.X-1);
					if (y_int < 0) y_int = - y_int%(fmc.Y-1);
                    else if (y_int >= fmc.Y) y_int = (fmc.Y - 1) -y_int%(fmc.Y-1);
					break;	//good practice

				default:	//no special handling
					;	//afh 01-may-2008
            }//end switch 
        #endif

            switch (buffer)
            {
				default:	//do something reasonable even if "buffer" isn't.
                case 0: //input buffer
                    if (gStuff->imageMode < 10)
                        return ((unsigned8 *)MyAdjustedInData)
                            [(x_int-fmc.x_start)*fmc.srcColDelta + (y_int-fmc.y_start)*fmc.srcRowDelta + z];
                    else
                        return *(unsigned16 *)(((unsigned8 *)MyAdjustedInData) +
                               (((x_int-fmc.x_start)*fmc.srcColDelta + z) << 1) +
                               (y_int-fmc.y_start)*fmc.srcRowDelta);
                case 1: //t buffer 1
                    if (gStuff->imageMode < 10)
						return fmc.tbuf[(y_int*fmc.X + x_int)*fmc.Z + z];
					else
						return *(unsigned16 *)(((unsigned8 *)fmc.tbuf) + 2*((y_int*fmc.X + x_int)*fmc.Z + z));
                case 2: //t buffer 2
					if (gStuff->imageMode < 10)
						return fmc.t2buf[(y_int*fmc.X + x_int)*fmc.Z + z];
					else
						return *(unsigned16 *)(((unsigned8 *)fmc.t2buf) + 2*((y_int*fmc.X + x_int)*fmc.Z + z));
                case 3: //output buffer
                    if (gStuff->imageMode < 10)
                        return ((unsigned8 *)gStuff->outData)
                            [(x_int-fmc.x_start)*fmc.dstColDelta + (y_int-fmc.y_start)*fmc.dstRowDelta + z];
                    else
                        return *(unsigned16 *)  ( ((unsigned8 *)gStuff->outData) +
                            (((x_int-fmc.x_start)*fmc.dstColDelta + z)<<1 ) + (y_int-fmc.y_start)*fmc.dstRowDelta  );
				case 4: //t buffer 3
					if (gStuff->imageMode < 10)
						return fmc.t3buf[(y_int*fmc.X + x_int)*fmc.Z + z];
					else
						return *(unsigned16 *)(((unsigned8 *)fmc.t3buf) + 2*((y_int*fmc.X + x_int)*fmc.Z + z));
				case 5: //t buffer 4
					if (gStuff->imageMode < 10)
						return fmc.t4buf[(y_int*fmc.X + x_int)*fmc.Z + z];
					else
						return *(unsigned16 *)(((unsigned8 *)fmc.t4buf) + 2*((y_int*fmc.X + x_int)*fmc.Z + z));
            }


        }

        else 
        {
            
            if (mode<4) 
            {
				int v1,v2,v3,v4;

/*
                //Does not work with multi-threading
				if (x==x_old && y==y_old && mode==mode_old && edgeMode==edgeMode_old)
                {
                    //Set cached values
                    fracx=fracx_old;
                    fracy=fracy_old;
                    x_int=x_int_old;
                    y_int=y_int_old; 
                    x_int1=x_int1_old;
                    y_int1=y_int1_old;
                }

                else //Calculate the values
*/
                {

                //#ifndef HYPERTYLE
				#if !defined(HYPERTYLE) && !defined(PLUGINGALAXY)
                    if (x < 0) x = 0; else if (x >= fmc.X) x = (double)(fmc.X - 1);
                    if (y < 0) y = 0; else if (y >= fmc.Y) y = (double)(fmc.Y - 1);
                #else
                    switch (edgeMode)
                    {
                        case 0:
                            if (x < 0) x = 0; else if (x >= fmc.X) x = (double)(fmc.X - 1);
                            if (y < 0) y = 0; else if (y >= fmc.Y) y = (double)(fmc.Y - 1);
                            break;

                        case 1:
                            if (x < 0 || y < 0 || x >= (fmc.X - 1)|| y >= (fmc.Y - 1))
                                return 0;
							break;				//was missing!!
                        
                        case 2:
                            //if (x < 0) x = fmc.X + x;
                            //else if (x >= fmc.X) x = fmod(x - fmc.X , fmc.X) ;
                            //if (y < 0) y = fmc.Y + y;
                            //else if (y >= fmc.Y) y = fmod(y - fmc.Y , fmc.Y);
							if (x < 0) x = (fmc.X-1) + fmod(x,(fmc.X-1));
							else if (x >= fmc.X) x = fmod(x,(fmc.X-1));
							if (y < 0) y = (fmc.Y-1) + fmod(y,(fmc.Y-1));
							else if (y >= fmc.Y) y = fmod(y,(fmc.Y-1));
                            break;

                        case 3:
                            //if (x < 0) x = - x;
                            //else if (x >= fmc.X) x = (fmc.X - 1) - (x - (fmc.X - 1));
                            //if (y < 0) y = - y;
                            //else if (y >= fmc.Y) y = (fmc.Y - 1) - (y - (fmc.Y - 1));
							if (x < 0) x = - fmod(x,fmc.X - 1);
							else if (x >= fmc.X) x = (fmc.X - 1) - fmod(x,fmc.X - 1);
							if (y < 0) y = - fmod(y,fmc.Y - 1);
							else if (y >= fmc.Y) y = (fmc.Y - 1) - fmod(y,fmc.Y - 1);
							break;

						default:	//no special handling
							;	//afh 01-may-2008
                    }//end switch
                #endif

                    x_int=(int) x;
                    y_int=(int) y;
					
                    x_int1=x_int+1;
                    y_int1=y_int+1;

                //#ifndef HYPERTYLE
				#if !defined(HYPERTYLE) && !defined(PLUGINGALAXY)
                    if (x_int1 >= fmc.X) x_int1 = (fmc.X - 1);
                    if (y_int1 >= fmc.Y) y_int1 = (fmc.Y - 1);
                #else
                    switch (edgeMode)
                    {
						default:
                        case 0: 
                            if (x_int1 >= fmc.X) x_int1 = (fmc.X - 1);
                            if (y_int1 >= fmc.Y) y_int1 = (fmc.Y - 1);
                            break;
                        case 1: 
                            if (x_int1 >= fmc.X) x_int1 = (fmc.X - 1);
                            if (y_int1 >= fmc.Y) y_int1 = (fmc.Y - 1);
                            break;
                        case 2:
                            //if (x_int1 >= fmc.X) x_int1 = (x_int1  - fmc.X) % fmc.X;
                            //if (y_int1 >= fmc.Y) y_int1 = (y_int1  - fmc.Y) % fmc.Y;
							if (x_int1 >= fmc.X) x_int1 = x_int1 % (fmc.X-1);
							if (y_int1 >= fmc.Y) y_int1 = y_int1 % (fmc.Y-1);
                            break;
                        case 3:
                            //if (x_int1 >= fmc.X) x_int1 = (fmc.X - 1) - (x_int1 - (fmc.X - 1));
                            //if (y_int1 >= fmc.Y) y_int1 = (fmc.Y - 1)- (y_int1 - (fmc.Y - 1));
							if (x_int1 >= fmc.X) x_int1 = (fmc.X - 1) -x_int1%(fmc.X-1);
							if (y_int1 >= fmc.Y) y_int1 = (fmc.Y - 1) -y_int1%(fmc.Y-1);
							break;	//afh 01-may-2008
                    }//end switch
                #endif

                    //Info ("x_int: %d - y_ing: %d",x_int,y_int);

                    fracx=x-x_int;
                    fracy=y-y_int;

                    //Info ("fracx: %f - fracy: %f",fracx,fracy);
                    

                    if (mode==2) //Bicosine
                    {
                        if (fracx==fracy)
                        {
                            fracx= (1.0-cos(fracx*3.1415927))*0.5;
                            fracy=fracx;
                        }
                        else
                        {
                            fracx= (1.0-cos(fracx*3.1415927))*0.5;
                            fracy= (1.0-cos(fracy*3.1415927))*0.5;
                        }
                    }
                    else if (mode == 1) //Bisquare
                    {
                        fracx *= fracx;
                        fracy *= fracy;
                    }
                
                }//Calculate not cached

                
                switch (buffer)
                {
					default:
                    case 0: //input buffer
                        if (gStuff->imageMode < 10){
							v1 = ((unsigned8 *)MyAdjustedInData)
                                [(x_int-fmc.x_start)*fmc.srcColDelta + (y_int-fmc.y_start)*fmc.srcRowDelta + z];
                            v2 = ((unsigned8 *)MyAdjustedInData)
                                [(x_int-fmc.x_start)*fmc.srcColDelta + (y_int1-fmc.y_start)*fmc.srcRowDelta + z];
                            v3 = ((unsigned8 *)MyAdjustedInData)
                                [(x_int1-fmc.x_start)*fmc.srcColDelta + (y_int-fmc.y_start)*fmc.srcRowDelta + z];
                            v4 = ((unsigned8 *)MyAdjustedInData)
                                [(x_int1-fmc.x_start)*fmc.srcColDelta + (y_int1-fmc.y_start)*fmc.srcRowDelta + z];
							/*int xval = (x_int-fmc.x_start)*fmc.srcColDelta ;
							int xval1 = (x_int1-fmc.x_start)*fmc.srcColDelta ;
							int yval = (y_int-fmc.y_start)*fmc.srcRowDelta + z;
							int yval1 = (y_int1-fmc.y_start)*fmc.srcRowDelta + z;
							v1 = ((unsigned8 *)MyAdjustedInData)[xval + yval];
                            v2 = ((unsigned8 *)MyAdjustedInData)[xval + yval1];
                            v3 = ((unsigned8 *)MyAdjustedInData)[xval1 + yval];
                            v4 = ((unsigned8 *)MyAdjustedInData)[xval1 + yval1];*/
                        } else {
                            v1 = *(unsigned16 *)(((unsigned8 *)MyAdjustedInData) +
                               (((x_int-fmc.x_start)*fmc.srcColDelta + z) << 1) +
                               (y_int-fmc.y_start)*fmc.srcRowDelta);
                            v2 = *(unsigned16 *)(((unsigned8 *)MyAdjustedInData) +
                               (((x_int-fmc.x_start)*fmc.srcColDelta + z) << 1) +
                               (y_int1-fmc.y_start)*fmc.srcRowDelta);
                            v3 = *(unsigned16 *)(((unsigned8 *)MyAdjustedInData) +
                               (((x_int1-fmc.x_start)*fmc.srcColDelta + z) << 1) +
                               (y_int-fmc.y_start)*fmc.srcRowDelta);
                            v4 = *(unsigned16 *)(((unsigned8 *)MyAdjustedInData) +
                               (((x_int1-fmc.x_start)*fmc.srcColDelta + z) << 1) +
                               (y_int1-fmc.y_start)*fmc.srcRowDelta);
                        }
                        break;

                    case 1: //temp buffer 1
						if (gStuff->imageMode < 10){
							v1 = fmc.tbuf[(y_int*fmc.X + x_int)*fmc.Z + z];
							v2 = fmc.tbuf[(y_int1*fmc.X + x_int)*fmc.Z + z];
							v3 = fmc.tbuf[(y_int*fmc.X + x_int1)*fmc.Z + z];
							v4 = fmc.tbuf[(y_int1*fmc.X + x_int1)*fmc.Z + z]; 
						} else {
							v1 = *(unsigned16 *)(((unsigned8 *)fmc.tbuf) + 2*((y_int*fmc.X + x_int)*fmc.Z + z));
							v2 = *(unsigned16 *)(((unsigned8 *)fmc.tbuf) + 2*((y_int1*fmc.X + x_int)*fmc.Z + z));
							v3 = *(unsigned16 *)(((unsigned8 *)fmc.tbuf) + 2*((y_int*fmc.X + x_int1)*fmc.Z + z));
							v4 = *(unsigned16 *)(((unsigned8 *)fmc.tbuf) + 2*((y_int1*fmc.X + x_int1)*fmc.Z + z));
						}
                        break;

                    case 2: //temp buffer 2
						if (gStuff->imageMode < 10){
							v1 = fmc.t2buf[(y_int*fmc.X + x_int)*fmc.Z + z];
							v2 = fmc.t2buf[(y_int1*fmc.X + x_int)*fmc.Z + z];
							v3 = fmc.t2buf[(y_int*fmc.X + x_int1)*fmc.Z + z];
							v4 = fmc.t2buf[(y_int1*fmc.X + x_int1)*fmc.Z + z];
						} else {
							v1 = *(unsigned16 *)(((unsigned8 *)fmc.t2buf) + 2*((y_int*fmc.X + x_int)*fmc.Z + z));
							v2 = *(unsigned16 *)(((unsigned8 *)fmc.t2buf) + 2*((y_int1*fmc.X + x_int)*fmc.Z + z));
							v3 = *(unsigned16 *)(((unsigned8 *)fmc.t2buf) + 2*((y_int*fmc.X + x_int1)*fmc.Z + z));
							v4 = *(unsigned16 *)(((unsigned8 *)fmc.t2buf) + 2*((y_int1*fmc.X + x_int1)*fmc.Z + z));
						}
                        break;

                    case 3: //output buffer
                        if (gStuff->imageMode < 10){
                            v1 = ((unsigned8 *)gStuff->outData)
                                [(x_int-fmc.x_start)*fmc.dstColDelta + (y_int-fmc.y_start)*fmc.dstRowDelta + z];
                            v2 = ((unsigned8 *)gStuff->outData)
                                [(x_int-fmc.x_start)*fmc.dstColDelta + (y_int1-fmc.y_start)*fmc.dstRowDelta + z];
                            v3 = ((unsigned8 *)gStuff->outData)
                                [(x_int1-fmc.x_start)*fmc.dstColDelta + (y_int-fmc.y_start)*fmc.dstRowDelta + z];
                            v4 = ((unsigned8 *)gStuff->outData)
                                [(x_int1-fmc.x_start)*fmc.dstColDelta + (y_int1-fmc.y_start)*fmc.dstRowDelta + z];
                        } else {
                            v1 = *(unsigned16 *)  ( ((unsigned8 *)gStuff->outData) +
                                (((x_int-fmc.x_start)*fmc.dstColDelta + z)<<1 ) + (y_int-fmc.y_start)*fmc.dstRowDelta  );
                            v2 = *(unsigned16 *)  ( ((unsigned8 *)gStuff->outData) +
                                (((x_int-fmc.x_start)*fmc.dstColDelta + z)<<1 ) + (y_int1-fmc.y_start)*fmc.dstRowDelta  );
                            v3 = *(unsigned16 *)  ( ((unsigned8 *)gStuff->outData) +
                                (((x_int1-fmc.x_start)*fmc.dstColDelta + z)<<1 ) + (y_int-fmc.y_start)*fmc.dstRowDelta  );
                            v4 = *(unsigned16 *)  ( ((unsigned8 *)gStuff->outData) +
                                (((x_int1-fmc.x_start)*fmc.dstColDelta + z)<<1 ) + (y_int1-fmc.y_start)*fmc.dstRowDelta  );
                        }
						break;
					
					case 4: //temp buffer 3
						if (gStuff->imageMode < 10){
							v1 = fmc.t3buf[(y_int*fmc.X + x_int)*fmc.Z + z];
							v2 = fmc.t3buf[(y_int1*fmc.X + x_int)*fmc.Z + z];
							v3 = fmc.t3buf[(y_int*fmc.X + x_int1)*fmc.Z + z];
							v4 = fmc.t3buf[(y_int1*fmc.X + x_int1)*fmc.Z + z];
						} else {
							v1 = *(unsigned16 *)(((unsigned8 *)fmc.t3buf) + 2*((y_int*fmc.X + x_int)*fmc.Z + z));
							v2 = *(unsigned16 *)(((unsigned8 *)fmc.t3buf) + 2*((y_int1*fmc.X + x_int)*fmc.Z + z));
							v3 = *(unsigned16 *)(((unsigned8 *)fmc.t3buf) + 2*((y_int*fmc.X + x_int1)*fmc.Z + z));
							v4 = *(unsigned16 *)(((unsigned8 *)fmc.t3buf) + 2*((y_int1*fmc.X + x_int1)*fmc.Z + z));
						}
                        break;
					case 5: //temp buffer 4
						if (gStuff->imageMode < 10){
							v1 = fmc.t4buf[(y_int*fmc.X + x_int)*fmc.Z + z];
							v2 = fmc.t4buf[(y_int1*fmc.X + x_int)*fmc.Z + z];
							v3 = fmc.t4buf[(y_int*fmc.X + x_int1)*fmc.Z + z];
							v4 = fmc.t4buf[(y_int1*fmc.X + x_int1)*fmc.Z + z];
						} else {
							v1 = *(unsigned16 *)(((unsigned8 *)fmc.t4buf) + 2*((y_int*fmc.X + x_int)*fmc.Z + z));
							v2 = *(unsigned16 *)(((unsigned8 *)fmc.t4buf) + 2*((y_int1*fmc.X + x_int)*fmc.Z + z));
							v3 = *(unsigned16 *)(((unsigned8 *)fmc.t4buf) + 2*((y_int*fmc.X + x_int1)*fmc.Z + z));
							v4 = *(unsigned16 *)(((unsigned8 *)fmc.t4buf) + 2*((y_int1*fmc.X + x_int1)*fmc.Z + z));
						}
                        break;
                }
                

/*
			    //Does not work with multi-threading
                //Set static variables for caching
                x_old = x;
                y_old = y;
                mode_old = mode;
                edgeMode_old=edgeMode;
                fracx_old=fracx;
                fracy_old=fracy;
                x_int_old=x_int;
                y_int_old=y_int; 
                x_int1_old=x_int1;
                y_int1_old=y_int1;
*/

                return (int) ((1.0-fracx)* ((1.0-fracy)*v1 + fracy*v2)  + fracx* ((1.0-fracy)*v3 + fracy*v4));
        
            }

            else if (mode==4) //bicubic interpolation
            {

                //For Bicubic Interpolation 
               double nx,ny,ox3,oy3,nx3,ny3;
               double ix[4];//ix1, ix2, ix3, ix4;
               int j,k;
               int i[4];//i1,i2,i3,i4;
               double x1,x2,x3,x4,y1,y2,y3,y4,y9;
            
             //#ifndef HYPERTYLE
			 #if !defined(HYPERTYLE) && !defined(PLUGINGALAXY)
                if (x < 0) x = 0; else if (x >= fmc.X) x = (double)(fmc.X - 1);
                if (y < 0) y = 0; else if (y >= fmc.Y) y = (double)(fmc.Y - 1);
             #else
                switch (edgeMode)
                {
                    case 0:
                        if (x < 0) x = 0; else if (x >= fmc.X) x = (double)(fmc.X - 1);
                        if (y < 0) y = 0; else if (y >= fmc.Y) y = (double)(fmc.Y - 1);
                        break;
                    case 1:
                        if (x < 0 || y < 0 || x >= (fmc.X - 1)|| y >= (fmc.Y - 1))
                            return 0;
						break;				//was missing!!
                    case 2:
                        //if (x < 0) x = fmod(fmc.X + x , fmc.X);
                        //else if (x >= fmc.X) x = fmod(x - fmc.X , fmc.X);
                        //if (y < 0) y = fmod(fmc.Y + y , fmc.Y);
                        //else if (y >= fmc.Y) y = fmod(y - fmc.Y , fmc.Y);
						if (x < 0) x = (fmc.X-1) + fmod(x,(fmc.X-1));
						else if (x >= fmc.X) x = fmod(x,(fmc.X-1));
						if (y < 0) y = (fmc.Y-1) + fmod(y,(fmc.Y-1));
						else if (y >= fmc.Y) y = fmod(y,(fmc.Y-1));
                        break;
                    case 3:
                        //else if (x >= fmc.X) x = (fmc.X - 1) - (x - (fmc.X - 1));
						//else if (y >= fmc.Y) y = (fmc.Y - 1) - (y - (fmc.Y - 1));
						if (x < 0) x = - fmod(x,fmc.X - 1);
                        else if (x >= fmc.X) x = (fmc.X - 1) - fmod(x,fmc.X - 1);
                        if (y < 0) y = - fmod(y,fmc.Y - 1);
                        else if (y >= fmc.Y) y = (fmc.Y - 1) - fmod(y,fmc.Y - 1);
						break;
					default:
						;	//afh 01-may-2008
                }//end switch
            #endif

               y_int = (int)y; //yy
               x_int = (int)x; //xx

               fracx=x-x_int;
               fracy=y-y_int;

               nx = 1. - fracx;
               ny = 1. - fracy;
               ox3 = (fracx * fracx * fracx) - fracx;
               oy3 = (fracy * fracy * fracy) - fracy;
               nx3 = (nx * nx * nx) - nx;
               ny3 = (ny * ny * ny) - ny;

                
               for (j=-1;j<3;j++){

                   y_int1 = y_int + j;

                //#ifndef HYPERTYLE
			    #if !defined(HYPERTYLE) && !defined(PLUGINGALAXY)
                    if (y_int1 < 0) y_int1 = 0;
                    if (y_int1 >= fmc.Y) y_int1 = (fmc.Y - 1);
                #else
                   switch (edgeMode)
                    {
                    case 0: 
                        if (y_int1 < 0) y_int1 = 0;
                        else if (y_int1 >= fmc.Y) y_int1 = (fmc.Y - 1);
                        break;
                    case 1: 
						if (y_int1 >= fmc.Y) y_int1 = (fmc.Y - 1);
                        break;
                    case 2:
                        //if (y_int1 >= fmc.Y) y_int1 = (y_int1  - fmc.Y)% fmc.Y;
						if (y_int1 >= fmc.Y) y_int1 = y_int1 % (fmc.Y-1);
                        break;
                    case 3:
						//if (y_int1 < 0) y_int1 = - y_int1;
						//else if (y_int1 >= fmc.Y) y_int1 = (fmc.Y - 1)- (y_int1 - (fmc.Y - 1));
						if (y_int1 < 0) y_int1 = - y_int1%(fmc.Y-1);
						else if (y_int1 >= fmc.Y) y_int1 = (fmc.Y - 1) -y_int1%(fmc.Y-1);
						break;
					default:
						break;	//afh 01-may-2008
                    }//end switch
                #endif
               
                   for (k=-1;k<3;k++) {
                        
                        x_int1 = x_int + k;

                    //#ifndef HYPERTYLE
					#if !defined(HYPERTYLE) && !defined(PLUGINGALAXY)
                        if (x_int1 < 0) x_int1 = 0;
                        if (x_int1 >= fmc.X) x_int1 = (fmc.X - 1);
                    #else
                        switch (edgeMode)
                        {
                            case 0: 
                                if (x_int1 < 0) x_int1 = 0; //Required for PSP X3 and older
                                if (x_int1 >= fmc.X) x_int1 = (fmc.X - 1);
                                break;
                            case 1: 
								if (x_int1 < 0) x_int1 = 0; //Required for PSP X3 and older
                                if (x_int1 >= fmc.X) x_int1 = (fmc.X - 1);
                                break;
                            case 2:
								if (x_int1 < 0) x_int1 = 0; //Required for PSP X3 and older
                                //if (x_int1 >= fmc.X) x_int1 = (x_int1 - fmc.X) % fmc.X;
								if (x_int1 >= fmc.X) x_int1 = x_int1 % (fmc.X-1);
                                break;                        
							case 3:
								if (x_int1 < 0) x_int1 = 0; //Required for PSP X3 and older
								//if (x_int1 >= fmc.X) x_int1 = (fmc.X - 1) - (x_int1 - (fmc.X - 1));
								if (x_int1 >= fmc.X) x_int1 = (fmc.X - 1) - x_int1%(fmc.X-1);
								break;
							default:
								break;	//afh 01-may-2008
                        }//end switch
                    #endif

                       switch (buffer){
							default:
                            case 0: //input buffer
                                if (gStuff->imageMode < 10)
                                    i[k+1] = ((unsigned8 *)MyAdjustedInData)
                                        [(x_int1 -fmc.x_start)*fmc.srcColDelta + (y_int1 -fmc.y_start)*fmc.srcRowDelta + z];
                                else
                                    i[k+1] = *(unsigned16 *)(((unsigned8 *)MyAdjustedInData) +
                                        (((x_int1-fmc.x_start)*fmc.srcColDelta + z) << 1) +
                                        (y_int1-fmc.y_start)*fmc.srcRowDelta);
                                break;
                            case 1: //temp buffer 1
								if (gStuff->imageMode < 10)
									i[k+1] = fmc.tbuf[( y_int1 *fmc.X + x_int1 )*fmc.Z + z];
								else
									i[k+1] = *(unsigned16 *)(((unsigned8 *)fmc.tbuf) + 2*((y_int1*fmc.X + x_int1)*fmc.Z + z));
                                break;
                            case 2: //temp buffer 2
								if (gStuff->imageMode < 10)
									i[k+1] = fmc.t2buf[( y_int1 *fmc.X + x_int1 )*fmc.Z + z];
								else
									i[k+1] = *(unsigned16 *)(((unsigned8 *)fmc.t2buf) + 2*((y_int1*fmc.X + x_int1)*fmc.Z + z));
                                break;
                            case 3: //output buffer
                                if (gStuff->imageMode < 10)
                                    i[k+1] = ((unsigned8 *)gStuff->outData)
                                        [(x_int1 -fmc.x_start)*fmc.dstColDelta + (y_int1 -fmc.y_start)*fmc.dstRowDelta + z];
                                else
                                    i[k+1] = *(unsigned16 *)  ( ((unsigned8 *)gStuff->outData) +
                                        (((x_int1-fmc.x_start)*fmc.dstColDelta + z)<<1 ) + (y_int1-fmc.y_start)*fmc.dstRowDelta  );
								break;
							case 4: //temp buffer 3
								if (gStuff->imageMode < 10)
									i[k+1] = fmc.t3buf[( y_int1 *fmc.X + x_int1 )*fmc.Z + z];
								else
									i[k+1] = *(unsigned16 *)(((unsigned8 *)fmc.t3buf) + 2*((y_int1*fmc.X + x_int1)*fmc.Z + z));
                                break;
							case 5: //temp buffer 4
								if (gStuff->imageMode < 10)
									i[k+1] = fmc.t4buf[( y_int1 *fmc.X + x_int1 )*fmc.Z + z];
								else
									i[k+1] = *(unsigned16 *)(((unsigned8 *)fmc.t4buf) + 2*((y_int1*fmc.X + x_int1)*fmc.Z + z));
                                break;
                        }

                    }//for k

                   
                   x1 = (i[2] - i[1]) - (i[1] - i[0]);
                   x2 = (i[3] - i[2]) - (i[2] - i[1]);
                   x3 = x1 - x2 / 4.;
                   x4 = x2 - x1 / 4.;
                   ix[j+1] = (fracx * i[2]) + (nx * i[1])
                    + (((x4 * ox3) + (x3 * nx3)) / 3.75);
               }//for j


               // Y
               y1 = (ix[2] - ix[1]) - (ix[1] - ix[0]);
               y2 = (ix[3] - ix[2]) - (ix[2] - ix[1]);
               y3 = y1 - y2 / 4.;
               y4 = y2 - y1 / 4.;
               y9 = (fracy * ix[2]) + (ny * ix[1])
                + (((y4 * oy3) + (y3 * ny3)) / 3.75);


                //Set static variables for caching
                //nx,ny,ox3,oy3,nx3,ny3

               return (int)y9;

            }//end bicubic



        }// end interpolation methods


    }
    // bad channel number??
    return 0; 	//afh 01-may-2008
} /*fm_iget*/




//function added by Harald Heim on Feb 13, 2002
int bCircle(int x, int y, int centerx, int centery, int radius)
{
    return c2m(x-centerx,y-centery) <= radius ? 1: 0;
    
}

//function added by Harald Heim on Feb 13, 2002
//int bEllipse(int x, int y, int centerx, int centery, int radiusx, int radiusy)
//{
//  return c2m(scl(x,0,fmc.X,-radiusx*fmc.Y/fmc.X,radiusx*fmc.Y/fmc.X) -centerx , scl(y,fmc.Y,0,-radiusy,radiusy) -centery ) >= fmc.Y/2 ? 1: 0;
//}



//function added by Harald Heim on Feb 13, 2002
int bRect(int x, int y, int centerx, int centery, int radiusx, int radiusy)
{
    return  x >= centerx - radiusx  && x <= centerx + radiusx && y >= centery-radiusy && y <= centery+radiusy  ? 1: 0;
    
}

//function added by Harald Heim on Feb 13, 2002
int bRect2(int x, int y, int topx, int topy, int bottomx, int bottomy)
{
    return  x >= topx && x <= bottomx && y >= topy && y <= bottomy  ? 1: 0;
    
}

//function added by Harald Heim on Feb 13, 2002
int bTriangle(int x, int y, int centerx, int centery, int radius)
{
    int calc = y * fmc.X / fmc.Y / 2;
    int topy = centery - radius;

    return  x - centerx + calc >= topy && x - centerx - calc <= -topy && y - centery < radius  ? 1 : 0;
    
}



//function added by Harald Heim on Apr 6, 2002
int refreshWindow (void)
{
    ////return UpdateWindow(fmc.hDlg);
    //InvalidateRect(gParams->ctl[n].hCtl, NULL, TRUE );
    //return UpdateWindow(gParams->ctl[n].hCtl);

    //InvalidateRect(fmc.hDlg, NULL, TRUE );
    //return UpdateWindow(fmc.hDlg);

    return RedrawWindow(
            fmc.hDlg,   // handle of entire dialog window
            NULL,   // address of structure with update rectangle, NULL=>entire client area
            NULL,   // handle of update region
            RDW_ERASE|RDW_INVALIDATE|RDW_ERASENOW|RDW_UPDATENOW     // array of redraw flags
       );
	
}

int refreshWindowNoErase (void)
{
    return RedrawWindow(
            fmc.hDlg,   // handle of entire dialog window
            NULL,   // address of structure with update rectangle, NULL=>entire client area
            NULL,   // handle of update region
            RDW_INVALIDATE     // array of redraw flags
       );

}

//function added by Harald Heim on Apr 6, 2002
int refreshRgn (INT_PTR Rgn)
{

    if (Rgn == 0) return 0;

    return RedrawWindow(
            fmc.hDlg,   // handle of entire dialog window
            NULL,   // address of structure with update rectangle, NULL=>entire client area
            (HRGN)Rgn,  // handle of update region
            RDW_ERASE|RDW_INVALIDATE|RDW_ERASENOW|RDW_UPDATENOW     // array of redraw flags
       );

}


int setCtlOrder (int n, int order){

    HWND Control;
    INT_PTR InsertAfter = (INT_PTR)order;
    
    if (n<0 || n > N_CTLS) return false;
    
    //HWND_TOP        0
    //HWND_BOTTOM     1
    //HWND_TOPMOST    -1
    //HWND_NOTOPMOST  -2
    if (order < -2 || order > 1) return false;

    //if (n == CTL_PREVIEW) Control = GetDlgItem(fmc.hDlg, 101);
    //else if (n == CTL_FRAME) Control = GetDlgItem(fmc.hDlg, IDC_PROXY_GROUPBOX);
    //else if (n == CTL_PROGRESS) Control = GetDlgItem(fmc.hDlg, IDC_PROGRESS1);
    //else 
	Control = gParams->ctl[n].hCtl;
    
    if (Control==NULL) return false;

    SetWindowPos(Control, (HWND)InsertAfter, 0, 0, 0, 0, SWP_FRAMECHANGED|SWP_NOACTIVATE|SWP_NOSIZE|SWP_NOMOVE);
    InvalidateRect(gParams->ctl[n].hCtl, NULL, TRUE );
	UpdateWindow(gParams->ctl[n].hCtl);
    
    return true;
}


//function added by Harald Heim on Apr 6, 2002
int refreshCtl (int n, ... )
{
	//Optional argument
	int nonClient = false;
	va_list argptr;
	va_start(argptr, n); //Start after variable n
    nonClient = va_arg(argptr, long);
    va_end(argptr);


    if (gParams->ctl[n].hCtl==NULL && n != CTL_PREVIEW) return false;

    
    if (gParams->ctl[n].ctlclass == CC_TAB){
		int i;

		InvalidateRect(gParams->ctl[n].hCtl, NULL, TRUE );
		UpdateWindow(gParams->ctl[n].hCtl);
		if (nonClient) SendMessage(gParams->ctl[n].hCtl, WM_NCPAINT, 1, 0);

		for (i = 0; i < N_CTLS; ++i) {
			if (gParams->ctl[i].inuse && gParams->ctl[i].tabControl == n &&  gParams->ctl[i].tabSheet == gParams->ctl[n].val){
				InvalidateRect(gParams->ctl[i].hCtl, NULL, TRUE );
				UpdateWindow(gParams->ctl[i].hCtl);
				if (nonClient) SendMessage(gParams->ctl[i].hCtl, WM_NCPAINT, 1, 0);
				if (gParams->ctl[i].ctlclass == CC_STANDARD || gParams->ctl[i].ctlclass == CC_SLIDER){
					InvalidateRect(gParams->ctl[i].hBuddy1, NULL, TRUE );
					UpdateWindow(gParams->ctl[i].hBuddy1);
					SetWindowPos(gParams->ctl[i].hBuddy1, NULL, 0, 0, 0, 0,
                        SWP_FRAMECHANGED|SWP_NOZORDER|SWP_NOACTIVATE|SWP_NOSIZE|SWP_NOMOVE);
					if (nonClient) SendMessage(gParams->ctl[n].hBuddy1, WM_NCPAINT, 1, 0);
					
					InvalidateRect(gParams->ctl[i].hBuddy2, NULL, TRUE );
					UpdateWindow(gParams->ctl[i].hBuddy2);
					if (nonClient) SendMessage(gParams->ctl[n].hBuddy2, WM_NCPAINT, 1, 0);
				}
			}	
		}

	/*} else if (gParams->ctl[n].ctlclass == CC_BITMAP){

		setCtlPos(n, gParams->ctl[n].xPos+1, -1, -1, -1);
		setCtlPos(n, gParams->ctl[n].xPos-1, -1, -1, -1);
		InvalidateRect(gParams->ctl[n].hCtl, NULL, TRUE );
		UpdateWindow(gParams->ctl[n].hCtl);
    */
	} else if (n == CTL_PREVIEW){

		ShowOutputBuffer(globals, fmc.hDlg, fDragging);

	} else {
		////return UpdateWindow(fmc.hDlg);
        HWND Control;

        if (gParams->ctl[n].ctlclass == CC_STANDARD || gParams->ctl[n].ctlclass == CC_SLIDER){
			InvalidateRect(gParams->ctl[n].hBuddy1, NULL, TRUE );
			UpdateWindow(gParams->ctl[n].hBuddy1);
			SetWindowPos(gParams->ctl[n].hBuddy1, NULL, 0, 0, 0, 0, SWP_FRAMECHANGED|SWP_NOZORDER|SWP_NOACTIVATE|SWP_NOSIZE|SWP_NOMOVE);
			if (nonClient) SendMessage(gParams->ctl[n].hBuddy1, WM_NCPAINT, 1, 0);
			InvalidateRect(gParams->ctl[n].hBuddy2, NULL, TRUE );
			UpdateWindow(gParams->ctl[n].hBuddy2);
			if (nonClient) SendMessage(gParams->ctl[n].hBuddy2, WM_NCPAINT, 1, 0);
		}
		
        //if (n == CTL_PROGRESS) Control = GetDlgItem(fmc.hDlg, IDC_PROGRESS1);
        //else 
		Control = gParams->ctl[n].hCtl;

        InvalidateRect(Control, NULL, TRUE );
		UpdateWindow(Control);
		if (nonClient) SendMessage(Control, WM_NCPAINT, 1, 0);
		
	}

	return true;

}

int refreshItems(int n){

	if (n<0 || n > N_CTLS || gParams->ctl[n].ctlclass != CC_LISTVIEW) return false;

	return (int) ListView_RedrawItems(gParams->ctl[n].hCtl,0,getCtlItemCount(n));
}

int lockCtl (int n)
{
    return (int)SendMessage(gParams->ctl[n].hCtl,WM_SETREDRAW,false,0);//LockWindowUpdate(gParams->ctl[n].hCtl);
}

int unlockCtl (int n)
{
    return (int)SendMessage(gParams->ctl[n].hCtl,WM_SETREDRAW,true,0);//LockWindowUpdate(NULL);
}


//function added by Harald Heim on Apr 6, 2002
int lockWindow (int c)
{
    if (c==0)
        return LockWindowUpdate(NULL);
	else
        return LockWindowUpdate(fmc.hDlg);
}


//function added by Harald Heim on Apr 2, 2002
int setZoom (int n, int m)
{
    if (!fmc.doingProxy) return 0;
    
    if (n<1 && n!=-888) n=1;
    else if (n>MAX_SCALE_FACTOR) n=MAX_SCALE_FACTOR;

    //function added by Harald Heim on Jun 13, 2002
    fmc.scaleFactor = n;
    
    return sub_setZoom(fmc.hDlg, n, m); //,1);
}

int setZoomEx (int n, int m)
{
#ifdef HIGHZOOM

    if (fmc.doingProxy==FALSE) return 0;
    
    if (n<1 && n!=-888) n=1;
    else if (n>16) n=16;

	if (n>1) m=1; //No enlargement for if < 100%

	if (m<1) m=1;
    else if (m>16) m=16;

    //function added by Harald Heim on Jun 13, 2002
    fmc.scaleFactor = n;
	//fmc.enlargeFactor = m; //is set in AfhFMUI
    
    return sub_setZoom(fmc.hDlg, n, m);
#else

	return 0;

#endif
}



int set_array_mode(int mode)
{

	if (mode == 1){	
#ifndef APP
		if (gStuff->bufferProcs && 
			gStuff->bufferProcs->allocateProc &&
			gStuff->bufferProcs->lockProc &&
			gStuff->bufferProcs->unlockProc &&
			gStuff->bufferProcs->freeProc){
				arrayMode = 1;
				return true;
		} else 
#endif
        {
			arrayMode = 0;
			return false;
		}
	} else {
		arrayMode = 0;
		return true;
	}
	
	return false;
}


#ifndef APP

INT_PTR allocHost (int size)
{
    BufferID bufferid;

    if (AllocateBuffer(size, &bufferid) == noErr){   
        return (INT_PTR)bufferid;
    } else {
        return (INT_PTR)NULL;
    }
}

INT_PTR lockHost (INT_PTR bufferid)
{

    return (INT_PTR)LockBuffer((BufferID)bufferid,true);
}


int freeHost (INT_PTR bufferid)
{
    UnlockBuffer((BufferID)bufferid);
	FreeBuffer((BufferID)bufferid);

    return true;
}

#endif


//function added by Harald Heim on Apr 9, 2002
int sub_allocArray (int nr, int X, int Y, int Z, int bytes, int padding)
{

    //BufferProcs* bufferProcs = gStuff->bufferProcs;
    int size;

    if (nr<0 || nr>99) return 0; //Array 100 isn't allocatable

    if (bytes==1 || bytes==2 || bytes==4 || bytes==8) ArrayBytes[nr]=bytes;
    else return 0;

    if (X <= 0) return 0;
    ArrayX[nr]=X;
    if (Y <= 0) ArrayY[nr]=1; else ArrayY[nr]=Y;
    if (Z <= 0) ArrayZ[nr]=1; else ArrayZ[nr]=Z;

    ArrayX[nr] += 2*padding;
    ArrayY[nr] += 2*padding;    // ACK!!?? Could easily triple the size of a 1-dim array, or worse!!?? Harry:_ But who would use padding for a 1-dim array???
    ArrayPadding[nr] = padding;

    size = ArrayX[nr] * ArrayY[nr] * ArrayZ[nr] * ArrayBytes[nr];
    if (size == 0) return 0;

    //Avoid reallocation of the same size
    if (ArraySize[nr] == size && Array[nr]!=NULL) return 1;
	
	//Not necessary
	//Free array if it already exists and has a different size
	//if (ArraySize[nr] != size && Array[nr]!=NULL) freeArray(nr);

    //Info ("%d", bufferProcs->spaceProc() );


	if (arrayMode == 1){
			
		//Free previous buffer
#ifndef APP
        if (ArrayID[nr]!=0 || nr == PhotoPlusArray) { 
			UnlockBuffer(ArrayID[nr]); //gStuff->bufferProcs->unlockProc(ArrayID[nr]);
			FreeBuffer(ArrayID[nr]); //gStuff->bufferProcs->freeProc(ArrayID[nr]);
			if (nr == PhotoPlusArray) PhotoPlusArray = -1;
		} else if (Array[nr]!=NULL){

			free (Array[nr]);
		}
#else
        if (Array[nr]!=NULL) free (Array[nr]);
#endif
		
		ArrayID[nr] = 0;
		Array[nr] = 0;

#ifndef APP
		if (AllocateBuffer(size, &ArrayID[nr]) == noErr){ //if (gStuff->bufferProcs->allocateProc(size, &ArrayID[nr]) == noErr){
			Array[nr] = LockBuffer(ArrayID[nr], true);//Array[nr] = gStuff->bufferProcs->lockProc(ArrayID[nr], true);
			if (ArrayID[nr]==0) PhotoPlusArray=nr;
		} else {
#endif
			Array[nr] = realloc (Array[nr], (unsigned32) size); //Fall back to Windows API
                                                                //(realloc could be malloc, since Array[nr]==0 here?-AFH)
            ArrayID[nr] = 0; //for safety?-afh
#ifndef APP
		}
#endif

	}else {
        //but what if ArrayID[nr] != 0 ?? Need to free it??-AFH	
#ifndef APP
        if (ArrayID[nr]!=0) { 
			UnlockBuffer(ArrayID[nr]); 
			FreeBuffer(ArrayID[nr]);
            ArrayID[nr] = 0;
		} 
#endif
		Array[nr] = realloc (Array[nr], (unsigned32) size);
	}
    

    if (Array[nr]==NULL)
    {
        ArrayID[nr]=0;
		ArraySize[nr]=0;
        ArrayBytes[nr]=0;
        ArrayX[nr]=0;
        ArrayY[nr]=0;
        ArrayZ[nr]=0;
        ArrayPadding[nr]=0;

        ErrorOk ("Unable to allocate enough memory.");
        fm_abort();

        return 0;
    }
    else
    {   
        ArraySize[nr]=size;
        return 1;
    }

}


//function added by Harald Heim on Apr 9, 2002
int allocArray (int nr, int X, int Y, int Z, int bytes)
{
    
    return sub_allocArray (nr, X, Y, Z, bytes, 0);
    
}

//function added by Harald Heim on Oct 12, 2003
int allocArrayPad (int nr, int X, int Y, int Z, int bytes, int padding)
{

    return sub_allocArray (nr, X, Y, Z, bytes, padding);

}

int freeArray (int nr)
{

    if (nr>=0 && nr<=99 && Array[nr] != NULL)
    {

/*#ifdef IMGSOURCE
        if (nr==10){
            GlobalFree (Array[nr]);
        } else 
#endif*/
#ifndef APP
        if (ArrayID[nr]!=0 || nr == PhotoPlusArray){ //was allocated with Buffer Proc
			UnlockBuffer(ArrayID[nr]);//gStuff->bufferProcs->unlockProc(ArrayID[nr]);
			FreeBuffer(ArrayID[nr]);//gStuff->bufferProcs->freeProc(ArrayID[nr]); 
			if (nr == PhotoPlusArray) PhotoPlusArray = -1;
        } else {
#endif
			free (Array[nr]);       //check result for safety??
#ifndef APP
        }
#endif
        
        Array[nr] = NULL;
		ArrayID[nr]=0;
        ArraySize[nr]=0;
        ArrayBytes[nr]=0;
        ArrayX[nr]=0;
        ArrayY[nr]=0;
        ArrayZ[nr]=0;
        ArrayPadding[nr]=0;
		
        return 1;
    }

    return 0;

}

void swapArray(int nr, int nr2)
{
	void * pTemp;
	BufferID bTemp;
	INT_PTR ptrTemp;
	int temp;

	pTemp = Array[nr];
	Array[nr] = Array[nr2];
	Array[nr2] = pTemp;

	bTemp = ArrayID[nr];
	ArrayID[nr] = ArrayID[nr2];
	ArrayID[nr2] = bTemp;

	ptrTemp=ArraySize[nr];
	ArraySize[nr]=ArraySize[nr2];
	ArraySize[nr2]=ptrTemp;

    temp=ArrayBytes[nr];
	ArrayBytes[nr]=ArrayBytes[nr2];
	ArrayBytes[nr2]=temp;

	temp=ArrayX[nr];
	ArrayX[nr]=ArrayX[nr2];
	ArrayX[nr2]=temp;

	temp=ArrayY[nr];
	ArrayY[nr]=ArrayY[nr2];
	ArrayY[nr2]=temp;

	temp=ArrayZ[nr];
	ArrayZ[nr]=ArrayZ[nr2];
	ArrayZ[nr2]=temp;

	temp=ArrayPadding[nr];
	ArrayPadding[nr]=ArrayPadding[nr2];
	ArrayPadding[nr2]=temp;
}


#if HALFFLOAT

///////////////////////////////////////////////////////////////////////////
//
// Copyright (c) 2002, Industrial Light & Magic, a division of Lucas
// Digital Ltd. LLC
// 
// All rights reserved.
// 
// Redistribution and use in source and binary forms, with or without
// modification, are permitted provided that the following conditions are
// met:
// *       Redistributions of source code must retain the above copyright
// notice, this list of conditions and the following disclaimer.
// *       Redistributions in binary form must reproduce the above
// copyright notice, this list of conditions and the following disclaimer
// in the documentation and/or other materials provided with the
// distribution.
// *       Neither the name of Industrial Light & Magic nor the names of
// its contributors may be used to endorse or promote products derived
// from this software without specific prior written permission. 
// 
// THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS
// "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT
// LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR
// A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT
// OWNER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL,
// SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT
// LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE,
// DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY
// THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
// (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE
// OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
//
///////////////////////////////////////////////////////////////////////////


__inline unsigned short floatToHalf (unsigned int i)
{
    // Our floating point number, f, is represented by the bit
    // pattern in integer i.  Disassemble that bit pattern into
    // the sign, s, the exponent, e, and the significand, m.
    // Shift s into the position where it will go in in the
    // resulting half number.
    // Adjust e, accounting for the different exponent bias
    // of float and half (127 versus 15).

    register int s =  (i >> 16) & 0x00008000;
    register int e = ((i >> 23) & 0x000000ff) - (127 - 15);
    register int m =   i        & 0x007fffff;

    // Now reassemble s, e and m into a half:
    if (e <= 0){
		if (e < -10){
			// E is less than -10.  The absolute value of f is
			// less than HALF_MIN (f may be a small normalized
			// float, a denormalized float or a zero).
			//
			// We convert f to a half zero.

			return 0;
		}

		// E is between -10 and 0.  F is a normalized float,
		// whose magnitude is less than HALF_NRM_MIN.
		//
		// We convert f to a denormalized half. 

		m = (m | 0x00800000) >> (1 - e);

		// Round to nearest, round "0.5" up.
		//
		// Rounding may cause the significand to overflow and make
		// our number normalized.  Because of the way a half's bits
		// are laid out, we don't have to treat this case separately;
		// the code below will handle it correctly.

		if (m &  0x00001000)
			m += 0x00002000;

		// Assemble the half from s, e (zero) and m.

		return s | (m >> 13);

    } else if (e == 0xff - (127 - 15)) {
		if (m == 0){
			// F is an infinity; convert f to a half
			// infinity with the same sign as f.
			return s | 0x7c00;

		} else {
			// F is a NAN; we produce a half NAN that preserves
			// the sign bit and the 10 leftmost bits of the
			// significand of f, with one exception: If the 10
			// leftmost bits are all zero, the NAN would turn 
			// into an infinity, so we have to set at least one
			// bit in the significand.
			
			m >>= 13;
			return s | 0x7c00 | m | (m == 0);
		}

    } else {

		// E is greater than zero.  F is a normalized float.
		// We try to convert f to a normalized half.
		
		// Round to nearest, round "0.5" up
		
		if (m &  0x00001000) {

			m += 0x00002000;

			if (m & 0x00800000) {
				m =  0;		// overflow in significand,
				e += 1;		// adjust exponent
			}
		}

		// Handle exponent overflow

		if (e > 30) {
			// overflow ();	// Cause a hardware floating point overflow;
			return s | 0x7c00;	// if this returns, the half becomes an
		}   			// infinity with the same sign as f.

		// Assemble the half from s, e and m.
		return s | (e << 10) | (m >> 13);
    }
} 

__inline unsigned int halfToFloat (unsigned short y)
{

    int s = (y >> 15) & 0x00000001;
    int e = (y >> 10) & 0x0000001f;
    int m =  y        & 0x000003ff;

    if (e == 0) {
		if (m == 0) {
			// Plus or minus zero
			return s << 31;
		} else {
			// Denormalized number -- renormalize it
			while (!(m & 0x00000400)){
				m <<= 1;
				e -=  1;
			}
			e += 1;
			m &= ~0x00000400;
		}
    } else if (e == 31) {
		if (m == 0){
			// Positive or negative infinity
			return (s << 31) | 0x7f800000;
		} else {
			// Nan -- preserve sign and significand bits
			return (s << 31) | 0x7f800000 | (m << 13);
		}
    }

    // Normalized number
    e = e + (127 - 15);
    m = m << 13;

    // Assemble s, e and m.
    return (s << 31) | (e << 23) | m;
}

#endif


int getArray (int nr, int x, int y, int z)
{

    int pos;

    if (Array[nr]==NULL || nr<0 || nr>99 || z<0 || z >= ArrayZ[nr] || ArrayBytes[nr] == 8) return 0;

    // No check on x and y within bounds???? -- allows wrapping of bounds???
    // Harry: Isn't that done by the edgeMode stuff???
        

#ifndef HYPERTYLE
//#if !defined(HYPERTYLE) && !defined(PLUGINGALAXY)
    if (x < 0-ArrayPadding[nr]) x = 0- ArrayPadding[nr];
    else if (x >= ArrayX[nr]-ArrayPadding[nr]) x = ArrayX[nr] - 1-ArrayPadding[nr];
    if (y < 0-ArrayPadding[nr]) y = 0-ArrayPadding[nr];
    else if (y >= ArrayY[nr]-ArrayPadding[nr]) y = ArrayY[nr] - 1-ArrayPadding[nr];
#else
    switch (edgeMode)
        {
            case 0:
                // Replicate pixels at edge
                if (x < 0-ArrayPadding[nr]) x = 0- ArrayPadding[nr];
                else if (x >= ArrayX[nr]-ArrayPadding[nr]) x = ArrayX[nr] - 1-ArrayPadding[nr];
                if (y < 0-ArrayPadding[nr]) y = 0-ArrayPadding[nr];
                else if (y >= ArrayY[nr]-ArrayPadding[nr]) y = ArrayY[nr] - 1-ArrayPadding[nr];
                break;

            case 1:
                // Black (zero) padding
                if (x < 0-ArrayPadding[nr]|| y < 0-ArrayPadding[nr]|| x >= ArrayX[nr]-ArrayPadding[nr]|| y >= ArrayY[nr]-ArrayPadding[nr])
                    return 0;
				//break;			//needed, or is fall-thru intentional here??
        
            case 2:
                // Wrap around
                if (x < 0) x = ArrayX[nr]-ArrayPadding[nr] + x;   /// doesn't work for x < -2X ?? use MOD ???
                else if (x >= ArrayX[nr]-ArrayPadding[nr]) x = x - ArrayX[nr]-ArrayPadding[nr];
                if (y < 0) y = ArrayY[nr]-ArrayPadding[nr]  + y;
                else if (y >= ArrayY[nr]-ArrayPadding[nr]) y = y - ArrayY[nr]-ArrayPadding[nr];
                break;
            
            case 3: 
                // Two possible reflection modes, one of period 2N, other of period 2N-2 (as here)?
                if (x < 0) x = - x;
                else if (x >= ArrayX[nr]-ArrayPadding[nr]) x = (ArrayX[nr] - 1-ArrayPadding[nr]) - (x - (ArrayX[nr] - 1-ArrayPadding[nr]));       //optimize?
                if (y < 0) y = - y;
                else if (y >= ArrayY[nr]-ArrayPadding[nr]) y = (ArrayY[nr] - 1-ArrayPadding[nr]) - (y - (ArrayY[nr] - 1-ArrayPadding[nr]));
				break;
        }//end switch
#endif

    ///pos = y*ArrayX[nr]*ArrayZ[nr]*ArrayBytes[nr] + x*ArrayZ[nr]*ArrayBytes[nr] + z*ArrayBytes[nr];
    pos = (( (y+ArrayPadding[nr])*ArrayX[nr] + (x+ArrayPadding[nr]) )*ArrayZ[nr] + z);//*ArrayBytes[nr];

    //if (pos <0 || pos >= ArraySize[nr]) return 0; // not necessary anymore because of edge mode **WRONG** unless mod used!!!


    /*if (ArrayBytes[nr] == 1)
        return ((unsigned8 *)Array[nr]) [pos];
    else if (ArrayBytes[nr] == 2)
        return  *(unsigned16 *)(((unsigned8 *)Array[nr]) + pos);
    else if (ArrayBytes[nr] == 4)
        return  *(unsigned32 *)(((unsigned8 *)Array[nr]) + pos);*/
    
    switch(ArrayBytes[nr]) {
		case 1:
	        return ((unsigned8 *)Array[nr]) [pos];
		case 2:
            return  ((unsigned16 *)Array[nr]) [pos];
	        //return  *(unsigned16 *)(((unsigned8 *)Array[nr]) + pos);
		case 4:
            return  ((unsigned32 *)Array[nr]) [pos];
	        //return  *(unsigned32 *)(((unsigned8 *)Array[nr]) + pos);
	}

    return 0; // or better return -1 to indicate something really wrong???

}

int fast_getArray (int nr, int x, int y, int z)
{
    int pos;

    if (x < 0) x = 0; else if (x >= ArrayX[nr]) x = ArrayX[nr] - 1;
    if (y < 0) y = 0; else if (y >= ArrayY[nr]) y = ArrayY[nr] - 1;

    pos = (( y*ArrayX[nr] + x )*ArrayZ[nr] + z);//*ArrayBytes[nr];
    
	switch(ArrayBytes[nr]) {
		case 1:
	        return ((unsigned8 *)Array[nr]) [pos];
		case 2:
            return  ((unsigned16 *)Array[nr]) [pos];
	        //return  *(unsigned16 *)(((unsigned8 *)Array[nr]) + pos);
		case 4:
            return  ((unsigned32 *)Array[nr]) [pos];
	        //return  *(unsigned32 *)(((unsigned8 *)Array[nr]) + pos);
	}

	return 0;
}

//int UNSAFE_get_pos[100];

int UNSAFE_getArray (int nr, int x, int y, int z)
{
    const int pos = (( (y+ArrayPadding[nr])*ArrayX[nr] + (x+ArrayPadding[nr]) )*ArrayZ[nr] + z)*ArrayBytes[nr];
    //UNSAFE_get_pos[nr] = (( (y+ArrayPadding[nr])*ArrayX[nr] + (x+ArrayPadding[nr]) )*ArrayZ[nr] + z)*ArrayBytes[nr];

	switch(ArrayBytes[nr]) {
		case 1:
	        return ((unsigned8 *)Array[nr]) [pos];
		case 2:
	        return  *(unsigned16 *)(((unsigned8 *)Array[nr]) + pos);
		case 4:
	        return  *(unsigned32 *)(((unsigned8 *)Array[nr]) + pos);
	}

	return 0;
}

/*
int UNSAFE_getArrayNext8 (int nr)
{  
    return ((unsigned8 *)Array[nr]) [UNSAFE_get_pos[nr]++];
}
int UNSAFE_getArrayNext16 (int nr)
{
    UNSAFE_get_pos[nr] += 2;
    return  *(unsigned16 *)(((unsigned8 *)Array[nr]) + UNSAFE_get_pos[nr]);
}
int UNSAFE_getArrayNext32 (int nr)
{  
    UNSAFE_get_pos[nr] += 4;
    return  *(unsigned32 *)(((unsigned8 *)Array[nr]) + UNSAFE_get_pos[nr]);
}
*/


double fgetArray (int nr, int x, int y, int z)
{

    UINT_PTR pos;

    if (Array[nr]==NULL || nr<0 || nr>99 || z<0 || z >= ArrayZ[nr] || ArrayBytes[nr] < 2) return 0.0;

    // No edge effects as for getArray above???


    pos = (( (y+ArrayPadding[nr])*ArrayX[nr] + (x+ArrayPadding[nr]) )*ArrayZ[nr] + z)*ArrayBytes[nr];
    if (pos <0 || pos >= ArraySize[nr]) return 0.0;

#if HALFFLOAT
	if (ArrayBytes[nr] == 2){
		UIF fval;
		fval.i = halfToFloat( *(unsigned short *)(((unsigned8 *)Array[nr]) + pos) );
		return (double)fval.f;

    } else 
#endif		
	if (ArrayBytes[nr] == 4)
		return *(float *)(((unsigned8 *)Array[nr]) + pos);
    else if (ArrayBytes[nr] == 8){
        return *(double *)(((unsigned8 *)Array[nr]) + pos);
    } else
        return 0.0; // or return some other value such as Indefinite Nan???
}


double UNSAFE_fgetArray (int nr, int x, int y, int z)
{
    const int pos = (((y + ArrayPadding[nr]) * ArrayX[nr] + (x + ArrayPadding[nr])) * ArrayZ[nr] + z) * ArrayBytes[nr];

	switch(ArrayBytes[nr]) {
		case 4:
			return *(float *)(((unsigned8 *)Array[nr]) + pos);
		case 8:
			return *(double *)(((unsigned8 *)Array[nr]) + pos);
#if HALFFLOAT
		case 2: {
			UIF fval;
			fval.i = halfToFloat( *(unsigned short *)(((unsigned8 *)Array[nr]) + pos) );
			return (double)fval.f;
		}
#endif
	}

    return 0.0;
}


//function added by Harald Heim on Apr 9, 2002
int putArray (int nr, int x, int y, int z, int val)
{

    UINT_PTR pos;

    if (Array[nr]==NULL || nr<0 || nr>99 || ArrayBytes[nr] == 8) return 0;
    //if ( x<0 || y<0 || z<0 || x >= ArrayX[nr] || y >= ArrayY[nr] || z >= ArrayZ[nr]  ) return 0;
    
    // clamp val to correct range...
    if (ArrayBytes[nr]==1){
        if (val < 0) val = 0;
        else if (val > 255 ) val = 255;
    } else if (ArrayBytes[nr]==2){
        if (val < 0) val = 0;
        else if (val > 65535 ) val = 65535;
    }

    //pos = y*ArrayX[nr]*ArrayZ[nr]*ArrayBytes[nr] + x*ArrayZ[nr]*ArrayBytes[nr] + z*ArrayBytes[nr];
    pos = (( (y+ArrayPadding[nr])*ArrayX[nr] + (x+ArrayPadding[nr]) )*ArrayZ[nr] + z);//*ArrayBytes[nr];
    
    if (pos <0 || pos*ArrayBytes[nr] >= ArraySize[nr]) return 0;


    /*if (ArrayBytes[nr] == 1)
        ((unsigned8 *)Array[nr]) [pos] = val;
    else if (ArrayBytes[nr] == 2)
        *(unsigned16 *)(((unsigned8 *) Array[nr]) + pos)  = val;
    else if (ArrayBytes[nr] == 4)
        *(unsigned32 *)(((unsigned8 *) Array[nr]) + pos ) = val;
    else
        return 0; // or better return -1 ?
    */

    switch(ArrayBytes[nr]) {
		case 1:
	        ((unsigned8 *)Array[nr]) [pos] = val;
            break;
		case 2:
	        ((unsigned16 *)Array[nr]) [pos] = val;
            break;
		case 4:
	        ((unsigned32 *)Array[nr]) [pos] = val;	
	}
    
    return 1;

}


void fast_putArray (int nr, int x, int y, int z, int val)
{
    const UINT_PTR pos = (( (y+ArrayPadding[nr])*ArrayX[nr] + (x+ArrayPadding[nr]) )*ArrayZ[nr] + z);//*ArrayBytes[nr];

    if (pos <0 || pos*ArrayBytes[nr] >= ArraySize[nr]) return;

	switch(ArrayBytes[nr]) {
		case 1:
	        ((unsigned8 *)Array[nr]) [pos] = val;
            break;
		case 2:
	        ((unsigned16 *)Array[nr]) [pos] = val;
            break;
		case 4:
	        ((unsigned32 *)Array[nr]) [pos] = val;	
	}
}

int UNSAFE_putArray (int nr, int x, int y, int z, int val)
{
    const int pos = (( (y+ArrayPadding[nr])*ArrayX[nr] + (x+ArrayPadding[nr]) )*ArrayZ[nr] + z)*ArrayBytes[nr];

	switch(ArrayBytes[nr]) {
		case 1:
	        ((unsigned8 *)Array[nr]) [pos] = val;
			return 1;
		case 2:
	        *(unsigned16 *)(((unsigned8 *) Array[nr]) + pos)  = val;
			return 1;
		case 4:
	        *(unsigned32 *)(((unsigned8 *) Array[nr]) + pos ) = val;
			return 1;
		default:
			return 0;
	}

	return 1;
}

int fputArray (int nr, int x, int y, int z, double val)
{

    UINT_PTR pos;

    if (Array[nr]==NULL || nr<0 || nr>99 || ArrayBytes[nr] < 2) return 0;
    
    pos = (( (y+ArrayPadding[nr])*ArrayX[nr] + (x+ArrayPadding[nr]) )*ArrayZ[nr] + z)*ArrayBytes[nr];
    if (pos <0 || pos >= ArraySize[nr]) return 0;

	//if (val> 3.4E+38) val=3.4E+38; else if (val< 3.4E-38) val=3.4E-38;

#if HALFFLOAT    
	if (ArrayBytes[nr] == 2){
		UIF fval;
		fval.f = (float)val;
		*(unsigned short *)(((unsigned8 *) Array[nr]) + pos ) = floatToHalf( fval.i );
    } else 
#endif		
	if (ArrayBytes[nr] == 4)
		*(float *)(((unsigned8 *) Array[nr]) + pos ) = (float)val;
	else if (ArrayBytes[nr] == 8)
		*(double *)(((unsigned8 *) Array[nr]) + pos ) = val;
    else 
        return 0; //bad value for ArrayBytes[nr].
	
    return 1;
}

int UNSAFE_fputArray (int nr, int x, int y, int z, double val)
{
    const int pos = (( (y+ArrayPadding[nr])*ArrayX[nr] + (x+ArrayPadding[nr]) )*ArrayZ[nr] + z)*ArrayBytes[nr];
    
	switch (ArrayBytes[nr]) {
		case 4:
			*(float *)(((unsigned8 *) Array[nr]) + pos ) = (float)val;
			return 1;
		case 8:
			*(double *)(((unsigned8 *) Array[nr]) + pos ) = val;
			return 1;
#if HALFFLOAT
		case 2: {
				UIF fval;
				fval.f = (float)val;
				*(unsigned short *)(((unsigned8 *) Array[nr]) + pos ) = floatToHalf( fval.i );
			}
			return 1;
#endif
		default:
			return 0;
	}
	
    return 1;
}



/*
__inline unsigned short float2le (double val)
{
  double v;
  int e;

  v = val;
  if (v < 1e-32) {
    return 0;
  }
  else {
    v = frexp(v,&e) * 256.0/v;
	return (unsigned char)(val * v)<<8 | (unsigned char)(e + 128);
  }
} 

__inline double le2float(unsigned short le)
{
  double f;
  unsigned char le1 = le >> 8;
  unsigned char le2 = le - (le1 << 8);

  if (le1) {   //nonzero pixel
    f = ldexp(1.0,le2-(int)(128+8));
	return le1 * f;
  }
  else
    return 0;
} 


int fputArrayLE (int nr, int x, int y, int z, double val)
{
    int pos;

    if (Array[nr]==NULL || nr<0 || nr>99 || ArrayBytes[nr] < 2) return 0;
    
    pos = (( (y+ArrayPadding[nr])*ArrayX[nr] + (x+ArrayPadding[nr]) )*ArrayZ[nr] + z)*ArrayBytes[nr];
    if (pos <0 || pos >= ArraySize[nr]) return 0;

	*(unsigned short *)(((unsigned8 *) Array[nr]) + pos ) = float2le (val);
	
    return 1;

}

double fgetArrayLE (int nr, int x, int y, int z)
{
    int pos;

    if (Array[nr]==NULL || nr<0 || nr>99 || z<0 || z >= ArrayZ[nr] || ArrayBytes[nr] < 2) return 0;

    pos = (( (y+ArrayPadding[nr])*ArrayX[nr] + (x+ArrayPadding[nr]) )*ArrayZ[nr] + z)*ArrayBytes[nr];
    if (pos <0 || pos >= ArraySize[nr]) return 0;

	return le2float ( *(unsigned short *)(((unsigned8 *)Array[nr]) + pos) );

}
*/



int putArrayString (int nr, int x, LPSTR string)
{

    UINT_PTR pos;

    if (nr<0 || nr>99 || Array[nr]==NULL) return false;
    
    pos = (x+ArrayPadding[nr])*ArrayY[nr];
    if (pos <0 || pos + (int)strlen(string) >= ArraySize[nr]) return false; //strlen+1 for NUL byte???-Alex

    strcpy ((char *)((unsigned8 *)Array[nr] + pos), string);

    return true;

}


char dummystr[] = "Not Available"; //Return this string to avoid memory access violations

char* getArrayString (int nr, int x)//, LPSTR string)
{

    UINT_PTR pos;

    if (nr<0 || nr>99 || Array[nr]==NULL) return (char*)(unsigned8 *)dummystr;
    
    pos = (x+ArrayPadding[nr])*ArrayY[nr];
    if (pos <0 || pos >= ArraySize[nr]) return (char*)(unsigned8 *)dummystr;

    //strcpy (string, (char *)((unsigned8 *)Array[nr] + pos));

    return (char*)((unsigned8 *)Array[nr] + pos);//true;

}




//function added by Harald Heim on Jun 4, 2002
unsigned8 gMskVal(int row, int rowOff, int col, int colOff)
{

    //This code produces a transparent-blank preview and final image when applying plugin to a layer
    /*if ((gNoMask || !(gParams->flags & AFH_USE_HOST_PADDING)) &&
        (row+rowOff < minRow || row+rowOff > maxRow ||
         col+colOff < minCol || col+colOff > maxCol) )
        return 0;   // Simulate the padding... 
    else */

    if (gNoMask)
        return 255; // Entire filterRect is inside the mask.
    else
        return fmc.mskPtr[rowOff*fmc.mskRowDelta + colOff*fmc.mskColDelta];
} /*gMskVal*/



#define maxBlurRadius 3
#define mskVal(row,rowOff,col,colOff) \
    gMskVal(row,rowOff,col,colOff)


// "Distance to mask" array of vectors.
#define DIST(i,j) dist[(i)*(fmc.columns+2*maxBlurRadius) + (j)]


#ifndef APP

//function added by Harald Heim on Jun 4, 2002
int calcSBD(int bevWidth)
{
    int i,j,i0,j0; 

#if BIGDOCUMENT
	VRect scaledFilterRect;
#else
    Rect scaledFilterRect;
#endif
    OSErr err = noErr;

    int32 distSize = (fmc.rows + 2*maxBlurRadius)*(fmc.columns + 2*maxBlurRadius)*sizeof(struct Vect_s);

    //Info("distSize: %d",distSize);

    //free buffer if necessary
    if (bufferID != 0)
    {
        UnlockBuffer(bufferID);
        FreeBuffer(bufferID);
        bufferID = 0;
    }

    // Allocate the dist array...
    err = AllocateBuffer(distSize, &bufferID);

    if (err != noErr) {
        // oops.  what now???
        gResult = err;
        return 0;
    }
    dist = (struct Vect_s *)LockBuffer(bufferID, FALSE);
    // Offset to account for non-0 lower bounds...
    dist += maxBlurRadius*(fmc.columns + 2*maxBlurRadius + 1);


#if BIGDOCUMENT
		scaledFilterRect = GetFilterRect();
		scaleRect32 (&scaledFilterRect, 1, fmc.scaleFactor);
#else
        copyRect (&scaledFilterRect, &gStuff->filterRect);
		scaleRect (&scaledFilterRect, 1, fmc.scaleFactor);
#endif
   

    // Compute min/max values for column and row indices that still lie
    // within the (scaled) filterRect (used to simulate padding of accesses
    // outside the bounds of filterRect).
    /*minCol = scaledFilterRect.left   - gStuff->outRect.left;
    maxCol = scaledFilterRect.right  - gStuff->outRect.left - 1;
    minRow = scaledFilterRect.top    - gStuff->outRect.top;
    maxRow = scaledFilterRect.bottom - gStuff->outRect.top - 1;*/

    { //scope
    //const int bevWidth = (ctl(7)*2+fmc.scaleFactor)/(fmc.scaleFactor*2);  // round to nearest
    const int bw = bevWidth;
    const int bwnz = bw ? bw : 1;  // Non-zero, so safe as a divisor...
    const int bw2 = bw*bw;
    const int bw2nz = bwnz*bwnz;   // Non-zero, so safe as a divisor...

    /* set local globals */
    gBevWidth = bevWidth;




        for (i=-maxBlurRadius; i < fmc.rows + maxBlurRadius; i++)
        {
            for (j=-maxBlurRadius; j < fmc.columns + maxBlurRadius; j++)
            {
                DIST(i,j).x = bw + 1;
                DIST(i,j).y = bw + 1;
            } // for j

        } //for i



        /*
        //MEthod1: TOO SLOW!!!
        // pass 1: find distance (vector) from nearest unmasked point...
        if (!(gParams->flags & AFH_OPTIMIZE_BEVEL))
        {   // method 1: O(n^4) brute force
            for (i=-maxBlurRadius; i < fmc.rows + maxBlurRadius; i++)
            {
                fmc.mskPtr =
                    fmc.mskData + (i * fmc.mskRowDelta) - (maxBlurRadius * fmc.mskColDelta);

                for (j=-maxBlurRadius; j < fmc.columns + maxBlurRadius; j++)
                {
                    if (mskVal(i,0,j,0))
                    {   // inside the mask
                        int d = DIST(i,j).x*DIST(i,j).x + DIST(i,j).y*DIST(i,j).y;

                        for (i0 = -bevWidth; i0 <= bevWidth; i0++)
                        {
                            for (j0 = -bevWidth; j0 <= bevWidth; j0++)
                            {
                                int a0 = mskVal(i,i0,j,j0);
                                int d0 = i0*i0 + j0*j0;
                                if (a0 == 0 && d0 <= bw2)
                                {   // found a point outside the mask
                                    if (d0 < d)
                                    {   // closer than previous...
                                        DIST(i,j).x = j0;
                                        DIST(i,j).y = i0;
                                        d = i0*i0 + j0*j0;
                                    }
                                }
                            } // for j0
                        } // for i0
                    } //if

                    fmc.mskPtr += fmc.mskColDelta;

                } // for j
            } // for i 
        } //bevel method 1
        */
        //else
        {


            // Method 2: O(n^2+) router simulation.
            // (j,i) = center of router bit.
            // For each (j,i) on the outside edge of the mask,
            // sweep out the path of the router blade, recording
            // any new minima to points hit inside the mask.
            // By optimizing the sweep, we can keep the whole
            // thing down to slightly worse than O(n^2).
            int pad = bevWidth + maxBlurRadius;

            for (i=-pad; i < fmc.rows + pad; i++)
            {
                fmc.mskPtr = fmc.mskData + (i * fmc.mskRowDelta) - (pad * fmc.mskColDelta);

                for (j=-pad; j < fmc.columns + pad; j++)
                {
                    if (mskVal(i,0,j,0) == 0)
                    {   // outside the mask...
                        // is it on the edge of the mask?
                        int i1 = -bevWidth;
                        int i2 =  bevWidth;
                        int i3 = -bevWidth;
                        int i4 =  bevWidth;
    #if 0
                        if (mskVal(i, 1, j, 0) ||
                            mskVal(i,-1, j, 0) ||
                            mskVal(i, 0, j, 1) ||
                          //mskVal(i, 1, j, 1) ||
                          //mskVal(i, 1, j,-1) ||
                          //mskVal(i,-1, j, 1) ||
                          //mskVal(i,-1, j,-1) ||
                            mskVal(i, 0, j,-1) )
    #else
                        if (i < -maxBlurRadius)
                            i1 = -i - maxBlurRadius;
                        else if (mskVal(i,-1,j,0))
                            i1 = -bevWidth;  // can tighten this...
                        else
                            i1 = 0;
                        if (i + i1 < -maxBlurRadius)    ////// redundant????????
                            i1 = -i - maxBlurRadius;

                        if (i > fmc.rows + maxBlurRadius - 1)
                            i2 = -i + fmc.rows + maxBlurRadius - 1;
                        else if (mskVal(i,1,j,0))
                            i2 = bevWidth;  // can tighten this...
                        else
                            i2 = 0;
                        if (i + i2 > fmc.rows + maxBlurRadius -1)
                            i2 = -i + fmc.rows + maxBlurRadius - 1;

                        if (j < -maxBlurRadius)
                            i3 = -j - maxBlurRadius;
                        else if (mskVal(i,0,j,-1))
                            i3 = -bevWidth;  // can tighten this...
                        else
                            i3 = 0;
                        if (j + i3 < -maxBlurRadius)
                            i3 = -j - maxBlurRadius;

                        if (j > fmc.columns + maxBlurRadius - 1)
                            i4 = -j + fmc.columns + maxBlurRadius - 1;
                        else if (mskVal(i,0,j,1))
                            i4 = bevWidth;  // can tighten this...
                        else
                            i4 = 0;
                        if (j + i4 > fmc.columns + maxBlurRadius -1)
                            i4 = -j + fmc.columns + maxBlurRadius - 1;

                        //i1 = mskVal(i,-1, j, 0) ? -bevWidth : 0;
                        //i2 = mskVal(i, 1, j, 0) ?  bevWidth : 0;
                        //i3 = mskVal(i, 0, j,-1) ? -bevWidth : 0;
                        //i4 = mskVal(i, 0, j, 1) ?  bevWidth : 0;
    #endif
                        {   // skip altogether if (i1 >= i2) && (i3 >= i4) ???
                            for (i0 = i1; i0 <= i2; i0++)
                            {
                                for (j0 = i3; j0 <= i4; j0++)
                                {
                                    int a0 = mskVal(i,i0,j,j0);
                                    int d0 = i0*i0 + j0*j0;
                                    if (a0 != 0 && d0 <= bw2)
                                    {   // found a point inside the mask
                                        int d = DIST(i+i0,j+j0).x*DIST(i+i0,j+j0).x + 
                                                DIST(i+i0,j+j0).y*DIST(i+i0,j+j0).y;
                                        if (d0 < d)
                                        {   // closer than previous...
                                            DIST(i+i0,j+j0).x = -j0;
                                            DIST(i+i0,j+j0).y = -i0;
                                        }
                                    }
                                } /* for j0 */
                            } /* for i0 */
                        } /*if on edge*/
                    } /*if*/

                    fmc.mskPtr += fmc.mskColDelta;

                } /* for j */
            } /* for i */
        } /*bevel method 2*/

    }//scope

    return 1;

}

int freeSBD(void)
{
    // Release the dist array...
    UnlockBuffer(bufferID);
    FreeBuffer(bufferID);
    bufferID = 0;
    return 1;
}

int getSBD(int j, int i)
{
    int valx,valy;
    if (j < fmc.x_start || j > fmc.x_end-1) return 0;
    if (i < fmc.y_start || i > fmc.y_end-1) return 0;
    i=i-fmc.y_start;
    j=j-fmc.x_start;
    if (i<0) i=0;
    else if (i> fmc.rows-1) i=fmc.rows-1;
    if (j<0) j=0;
    else if (j> fmc.columns-1) j=fmc.columns-1;
    valx=DIST(i,j).x;
    valy=DIST(i,j).y;

    return valx*valx + valy*valy;

}

int getSBDangle(int j, int i, int a10)
{
    int valx,valy;
    double phi = -((double) a10 / 100.0) * 6.28318530717958647693 / 360.0;

    if (j < fmc.x_start || j > fmc.x_end-1) return 0;
    if (i < fmc.y_start || i > fmc.y_end-1) return 0;
    i=i-fmc.y_start;
    j=j-fmc.x_start;
    if (i<0) i=0;
    else if (i> fmc.rows-1) i=fmc.rows-1;
    if (j<0) j=0;
    else if (j> fmc.columns-1) j=fmc.columns-1;
    valx=DIST(i,j).x;
    valy=DIST(i,j).y;

    //return (int)( (256.0 * 0.63661977236758134308) *  (atan2((double)valy, (double)valx) )   );
    return (int)( 512.0 *  cos (atan2((double)valy, (double)valx) - phi)   );

}


int getSBDX(int j, int i)
{
    if (j < fmc.x_start || j > fmc.x_end-1) return 0;
    if (i < fmc.y_start || i > fmc.y_end-1) return 0;
    i=i-fmc.y_start;
    j=j-fmc.x_start;
    if (i<0) i=0;
    else if (i> fmc.rows-1) i=fmc.rows-1;
    if (j<0) j=0;
    else if (j> fmc.columns-1) j=fmc.columns-1;
    return DIST(i,j).x;
}

int getSBDY(int j, int i)
{
    if (j < fmc.x_start || j > fmc.x_end-1) return 0;
    if (i < fmc.y_start || i > fmc.y_end-1) return 0;
    i=i-fmc.y_start;
    j=j-fmc.x_start;
    if (i<0) i=0;
    else if (i> fmc.rows-1) i=fmc.rows-1;
    if (j<0) j=0;
    else if (j> fmc.columns-1) j=fmc.columns-1;
    return DIST(i,j).y;
}

#endif



int getCtlPos (int n, int w){

    int val=0;
    
    //RECT rcCtl;
    //GetWindowRect( gParams->ctl[n].hCtl, &rcCtl) 
    //ScreenToClient (fmc.hDlg, (LPPOINT)&rcCtl);
    //PixelsToVDBUs 

	if (n == CTL_PREVIEW){ //|| getCtlClass(n)==CC_COMBOBOX){

        RECT rcCtl;
		HWND hwnd;

		if (n == CTL_PREVIEW) hwnd = GetDlgItem(fmc.hDlg, 101);
		else gParams->ctl[n].hCtl;
    
        GetWindowRect( hwnd, &rcCtl );
		ScreenToClient (fmc.hDlg, (LPPOINT)&rcCtl);
		ScreenToClient (fmc.hDlg, (LPPOINT)&(rcCtl.right));

		switch (w){
			case 0: val = PixelsToHDBUs(rcCtl.left); break;
			case 1: val = PixelsToVDBUs(rcCtl.top); break;
			case 2: val = PixelsToHDBUs(rcCtl.right - rcCtl.left); break;
			case 3: val = PixelsToVDBUs(rcCtl.bottom - rcCtl.top); break;
		}
	
	} else {

		switch (w){
			case 0: val = gParams->ctl[n].xPos; break;
			case 1: val = gParams->ctl[n].yPos; break;
			case 2: val = gParams->ctl[n].width; break;
			case 3: val = gParams->ctl[n].height; break;
		}
	}

    return val;
}


int getCtlCoord (int n, int w)
{

    RECT rcCtl;
    POINT pt;
    int val;

        
    if (n == CTL_PREVIEW){

#if BIGDOCUMENT
		int proxW, inW, proxH, inH;
#else
        short proxW, inW, proxH, inH;
#endif
        
        if ( GetWindowRect( GetDlgItem(fmc.hDlg, 101), &rcCtl) )
        {   
            GetCursorPos(&pt);

            if (pt.x >= rcCtl.left && pt.x < rcCtl.right &&
                pt.y >= rcCtl.top  && pt.y < rcCtl.bottom) 
            {

#if BIGDOCUMENT
				fmc.inRect32 = GetInRect();	//needed??
                if (w==0){ //X-Coordinate
                    proxW = gProxyRect.right - gProxyRect.left;
                    inW = fmc.inRect32.right - fmc.inRect32.left;
    
                    if (inW < proxW) //Proxy full used
                        return  pt.x - rcCtl.left - (proxW - inW)/2 - 1; 
                    else //Proxy partially used
                        return  fmc.x_start + pt.x - rcCtl.left - 1;

                } else { //Y-Coordinate
                    proxH = gProxyRect.bottom - gProxyRect.top;
                    inH = fmc.inRect32.bottom - fmc.inRect32.top;
                
                    if (inH < proxH)
                        return  pt.y - rcCtl.top - (proxH - inH)/2 - 1; 
                    else
                        return  fmc.y_start + pt.y - rcCtl.top - 1;
                }

#else
                if (w==0){ //X-Coordinate
                    proxW = gProxyRect.right - gProxyRect.left;
                    inW = gStuff->inRect.right - gStuff->inRect.left;
    
                    if (inW*fmc.enlargeFactor < proxW-fmc.enlargeFactor){ //Proxy full used
                        val = pt.x - rcCtl.left - (proxW - inW*fmc.enlargeFactor)/2 - 1; 
                        #ifdef HIGHZOOM
                            val /= fmc.enlargeFactor;
						#endif
                    }else { //Proxy partialy used
                        #ifdef HIGHZOOM
                            val = fmc.x_start + (pt.x - rcCtl.left - 1)/fmc.enlargeFactor;
                        #else
                            val = fmc.x_start + pt.x - rcCtl.left - 1;
                        #endif
                    }

                } else { //Y-Coordinate
                    proxH = gProxyRect.bottom - gProxyRect.top;
                    inH = gStuff->inRect.bottom - gStuff->inRect.top;
                
                    if (inH*fmc.enlargeFactor < proxH-fmc.enlargeFactor){
                        val = pt.y - rcCtl.top - (proxH - inH*fmc.enlargeFactor)/2 - 1; 
                        #ifdef HIGHZOOM
                            val /= fmc.enlargeFactor;
                        #endif
                    } else {
                        #ifdef HIGHZOOM
                            val = fmc.y_start + (pt.y - rcCtl.top - 1)/fmc.enlargeFactor;
                        #else
                            val = fmc.y_start + pt.y - rcCtl.top - 1;
                        #endif
                    }
                }
#endif                
                return val;

            } else {
                
                return -1;
            }
        }
        return -1; //error from GetWindowRect afh 01-may2008
    
    } else { //Other Controls

        if ( GetWindowRect(gParams->ctl[n].hCtl, &rcCtl) ) //GetWindowRect( GetDlgItem(fmc.hDlg, 101), &rcCtl) )
        {   
            GetCursorPos(&pt);
            if (pt.x >= rcCtl.left && pt.x < rcCtl.right &&
                pt.y >= rcCtl.top  && pt.y < rcCtl.bottom) 
            {
                     if (w==0){ //X-Coordinate
                        return  pt.x - rcCtl.left;// - 1;

                     } else { 
                        return  pt.y - rcCtl.top;// -1;

                     }
            }
            else
            {
                return -1;
            }
            
        }
        return -1;	//error from GetWindowRect afh 01-may2008
    }
}



// function added by Harald Heim on Aug 9, 2002
int getPreviewCoordX (void)
{
    
    //Redirect to new function
    return getCtlCoord (CTL_PREVIEW, 0);
    
    /*RECT rcCtl;
    POINT pt;
    short   proxW, inW;


        if ( GetWindowRect( GetDlgItem(fmc.hDlg, 101), &rcCtl) )
        {   
            GetCursorPos(&pt);

            if (pt.x >= rcCtl.left && pt.x < rcCtl.right &&
                pt.y >= rcCtl.top  && pt.y < rcCtl.bottom) 
            {
                
                proxW = gProxyRect.right - gProxyRect.left;
                inW = gStuff->inRect.right - gStuff->inRect.left;
    
                if (inW < proxW) //Proxy full used
                    return  pt.x - rcCtl.left - (proxW - inW)/2 - 1; 
                else //Proxy partialy used
                    return  fmc.x_start + pt.x - rcCtl.left - 1;
                
            }
            else
            {
                return -1;
            }
        }*/
    
}

// function added by Harald Heim on Aug 9, 2002
int getPreviewCoordY (void)
{

    //Redirect to new function
    return getCtlCoord (CTL_PREVIEW, 1);
    
    /*RECT rcCtl;
    POINT pt;
    short   proxH, inH;

        if ( GetWindowRect(GetDlgItem(fmc.hDlg, 101), &rcCtl) )
        {   
            GetCursorPos(&pt);

            if (pt.x >= rcCtl.left && pt.x < rcCtl.right &&
                pt.y >= rcCtl.top  && pt.y < rcCtl.bottom) 
            {
                
                proxH = gProxyRect.bottom - gProxyRect.top;
                inH = gStuff->inRect.bottom - gStuff->inRect.top;
                
                if (inH < proxH)
                    return  pt.y - rcCtl.top - (proxH - inH)/2 - 1; 
                else
                    return  fmc.y_start + pt.y - rcCtl.top - 1;

            }
            else
            {
                return -1;
            }
        }*/

}




// function added by Harald Heim on Aug 12, 2002
int getAsyncKeyState (int t)
{
	//Swap mouse buttons for left hand setting
	if (t==VK_LBUTTON || t==VK_RBUTTON){
		if (GetSystemMetrics(SM_SWAPBUTTON)){
			if (t==VK_LBUTTON) t = VK_RBUTTON;
			else if (t==VK_RBUTTON) t = VK_LBUTTON;
		}
	}

   return GetAsyncKeyState (t);
}


// function added by Harald Heim on Aug 12, 2002
int getAsyncKeyStateF (int t)
{
    if (GetForegroundWindow() == MyHDlg)
        return getAsyncKeyState (t);
    else
        return 0;
}


int getAsyncKeyStateFC (int t)
{
    if (GetForegroundWindow() == MyHDlg){
		int n;
		HWND focWindow = GetFocus();

		//Check if edit box is focused
		for (n = 0; n < N_CTLS; n++) {
			if (gParams->ctl[n].ctlclass == CC_STANDARD || gParams->ctl[n].ctlclass == CC_SLIDER){
				if (focWindow == gParams->ctl[n].hBuddy1) return 0;
			}
		}

        if (getCtlClass(getCtlFocus()) == CC_COMBOBOX || getCtlClass(getCtlFocus()) == CC_LISTBOX){
            if (getAsyncKeyStateF(VK_MENU) <= -32767)
                return getAsyncKeyState (t);
        } else
             return getAsyncKeyState (t);
    } 

    return 0;

}


// function added by Harald Heim on Jun 6, 2002
int mouseOverWhenInvisible (int t)
{
    MouseOverWhenInvisible = t;
    
    return 1;
}


int arrayExists (int nr){

	if (nr<0 || nr>99) return false;

	if (Array[nr]!=NULL) return true;

	return false;
}


int getArrayDim (int nr, int dim)
{
	
	if (nr<0 || nr>99) return 0;

	if (Array[nr]!=NULL){
		if (dim==0)
			return ArrayX[nr];
		else if (dim==1)
			return ArrayY[nr];
		else if (dim==2)
			return ArrayZ[nr];
        else if (dim==3)
            return ArrayBytes[nr];

#if 0   //extended return values...
        else if (dim==-1)
            return (int)Array[nr];
        else if (dim==-2)
            return (int)ArrayID[nr];
        else if (dim==-3)
            return ArraySize[nr];
        else if (dim==-4)
            return ArrayBytes[nr];
        else if (dim==-5)
            return ArrayPadding[nr];
#endif
	}
	
	return 0;

}

int copyArray (int src, int dest)
{

    if (src<0 || src>99) return 0;	// afh 01-may2008
    if (Array[src]==NULL) return 0;
    if (ArraySize[src] == 0) return 0;
    if (dest<0 || dest>99) return 0;
    //Addded by Harald Heim, March 26, 2003
    if (src == dest) return 1;
    
#ifndef APP
	if (arrayMode == 1){
			
		//Free previous buffer
		//if (ArrayID[dest]!=0) {
		if (ArrayID[dest]!=0 || dest == PhotoPlusArray) { 
			gStuff->bufferProcs->unlockProc(ArrayID[dest]);
			gStuff->bufferProcs->freeProc(ArrayID[dest]); 
			if (dest == PhotoPlusArray) PhotoPlusArray = -1;
		} else if (Array[dest]!=0){
			free (Array[dest]);
		}
		
		ArrayID[dest] = 0;
		Array[dest] = 0;
		
		if (gStuff->bufferProcs->allocateProc((int32) ArraySize[src], &ArrayID[dest]) == noErr){
			Array[dest] = gStuff->bufferProcs->lockProc(ArrayID[dest], true);
		} else {
			Array[dest] = realloc (Array[dest], (unsigned32) ArraySize[src]); //Fall back to Windows API
		}

	}else {
#endif

		Array[dest] = realloc (Array[dest], ArraySize[src]);

#ifndef APP
	}
#endif
    

    if (Array[dest]==NULL)
    {
        ArrayID[dest]=0;
		ArraySize[dest]=0;
        ArrayX[dest] = 0;
        ArrayY[dest] = 0;
        ArrayZ[dest] = 0;
        ArrayPadding[dest]=0;
        return 0;
    }
    else
    {   
        memcpy (Array[dest], Array[src], ArraySize[src] );

        ArrayX[dest] = ArrayX[src];
        ArrayY[dest] = ArrayY[src];
        ArrayZ[dest] = ArrayZ[src];
        ArrayBytes[dest] = ArrayBytes[src];
        ArraySize[dest]=ArraySize[src];
        ArrayPadding[dest]=ArrayPadding[src];
        return 1;
    }
    return 0;

}

/*
int swapArray (int src, int dest)
{

    INT_PTR temp;
    int t;

    if (src<0 || src>99) return 0;	// afh 01-may2008
    if (dest<0 || dest>99) return 0;
    if (src == dest) return true;
    
    temp = (INT_PTR)Array[src];
    Array[src] = Array[dest];
    Array[dest] = (void *)temp;

    temp = (INT_PTR)ArrayID[src];
    ArrayID[src] = ArrayID[dest];
    ArrayID[dest] = (BufferID) temp;

	temp = ArraySize[src];
    ArraySize[src] = ArraySize[dest];
    ArraySize[dest] = temp;

    t = ArrayX[src];
    ArrayX[src] = ArrayX[dest];
    ArrayX[dest] = t;

    t = ArrayY[src];
    ArrayY[src] = ArrayY[dest];
    ArrayY[dest] = t;

    t = ArrayZ[src];
    ArrayZ[src] = ArrayZ[dest];
    ArrayZ[dest] = t;

    t = ArrayBytes[src];
    ArrayBytes[src] = ArrayBytes[dest];
    ArrayBytes[dest] = t;

    t = ArrayPadding[src];
    ArrayPadding[src] = ArrayPadding[dest];
    ArrayPadding[dest] = t;
    
    return true;
}*/



int fillArray (int nr, int val)
{
    UINT_PTR i;

    if (nr<0 || nr>99 || Array[nr]==NULL || ArraySize[nr] == 0) return 0;	// afh 01-may-2008

    if (ArrayBytes[nr]==1){
        
        if (val<0) val=0; 
        else if (val>255) val=255;

        memset( Array[nr], val, ArraySize[nr] );

    } else if (ArrayBytes[nr]==2){

        if (val<0) val=0; 
        else if (val>65535) val=65535;

        //this could be optimized...
        for (i=0;i<ArraySize[nr];i+=2){
             *(unsigned16 *)(((unsigned8 *) Array[nr]) + i)  = val;
        }

    } else if (ArrayBytes[nr]==4){
        
        //this could be optimized...
        for (i=0;i<ArraySize[nr];i+=4){
            *(unsigned32 *)(((unsigned8 *) Array[nr]) + i ) = val;
        }
    
	} else if (ArrayBytes[nr]==8){
        double dval = (double)val;
        
        for (i=0;i<ArraySize[nr];i+=8){
            //could maybe do a 64-bit int copy??-afh
            *(double *)(((unsigned8 *) Array[nr]) + i ) = dval;
        }
	}

    return true;
}

//----------------------------------------------------------------
//  bool = ffillArray(nr, dval)
//
//      Fill an Array (nr) with a f.p. value dval.
//      dval will be converted to half, float, or double if
//      ArrayBytes[nr] is 2, 4, or 8, resp.
//
//  Returns false if failed (bad value for nr, Array not
//      allocated, ArrayBytes not 2, 4, or 8, etc.).
//  Returns true if successful.
//----------------------------------------------------------------
int ffillArray (int nr, double dval)
{
    UINT_PTR i;

    if (nr<0 || nr>99 || Array[nr]==NULL || ArraySize[nr] == 0) return false;

#if HALFFLOAT
    if (ArrayBytes[nr]==2){
        //fill with half (16-bit f.p.) value
        unsigned short hval;
		UIF fval;
		fval.f = (float)dval;
        hval = floatToHalf( fval.i );

        //this could be optimized...
        for (i=0;i<ArraySize[nr];i+=2){
             *(unsigned short *)(((unsigned8 *) Array[nr]) + i) = hval;
        }

    } else 
#endif		
	if (ArrayBytes[nr]==4){
        //fill with float (32-bit f.p.) value
        float fval = (float)dval;
        //this could be optimized...
        for (i=0;i<ArraySize[nr];i+=4){
            *(float *)(((unsigned8 *) Array[nr]) + i ) = fval;
        }
    
	} else if (ArrayBytes[nr]==8){
        //fill with double (64-bit f.p.) value
        for (i=0;i<ArraySize[nr];i+=8){
            //could maybe do a 64-bit int fill instead??
            *(double *)(((unsigned8 *) Array[nr]) + i ) = dval;
        }
    } else
        return false; //bad value for ArrayBytes[nr].

    return true;

}//ffillArray




int ctlEnabled(int n)
{
//#if 0
    //// NO! Do not perpetuate this travesty!!!
    //// See note at enableCtl(). AFH 20Aug2008
    int state= (int)gParams->ctl[n].enabled & 3;

    if (state==3) return -1;
    else return state;
//#else
    return gParams->ctl[n].enabled;
//#endif
} 

#define MAX_RECURSIVE_TAB_CONTROLS 16
/*	ctlEnabledAs

	Returns the enabled state as the control is rendered on screen, not the internal state.
	Initially the "enabledAs" state depends on whether it has a Tab control assigned, but
	it could be easily changed to allow other parent controls or external criteria.
	
	Returns:
		0 (invisible and disabled)
		1 (visible but disabled)
		3 (visible and enabled)
*/ //MWVDL 2008-08-19
int sub_ctlEnabledAs(int n, int depth)
{
	//if(depth < 0) {
	//	ErrorOk("More than %d recursive tab controls", MAX_RECURSIVE_TAB_CONTROLS);
	//}
	
	if ((n < 0 || n >= N_CTLS || !gParams->ctl[n].inuse) || n==CTL_PREVIEW ){ //&& n != CTL_PROGRESS && n != CTL_FRAME && n != CTL_ZOOM) { //&& n != CTL_PREVIEW
        return 0;
    }

    {//scope
        int level = gParams->ctl[n].enabled;					// by default, level is n.enabled
        const int t = gParams->ctl[n].tabControl;
        if (t >= 0) {											// is tabControl set
	        if (t < N_CTLS											// does tabControl exist? (in range)
		     && gParams->ctl[t].inuse								// is tabControl used?
	         && (gParams->ctl[t].ctlclass == CC_TAB || gParams->ctl[t].ctlclass == CC_LISTBAR)   // is tabControl a tab control?
	         && gParams->ctl[t].val == gParams->ctl[n].tabSheet) {	// is tabControl's sheet same as n's sheet?
		        level = min(level, sub_ctlEnabledAs(t, --depth));		// lowest state of enabled, ctlEnabledAs(tab)
	        } else {
		        level = 0;											// invisible
	        }
        }
	    return level;
    }//scope
}
int ctlEnabledAs(int n)
{
	return sub_ctlEnabledAs(n, MAX_RECURSIVE_TAB_CONTROLS);
}



//function added by Harald Heim on Dec 7, 2002
int rgb2lab(int r, int g, int b, int z)
{

    double rVal, gVal, bVal;
    double xVal, yVal, zVal;
    double lVal, aVal;
    double fX, fY, fZ;

    THREAD_LOCAL static int Cr,Cg,Cb;
    THREAD_LOCAL static int Cl,Ca,Cb2;


    if (r==Cr && g==Cg && b==Cb){ //Read from Cache

        if (z==0)
            return Cl;
        else if (z==1)
            return Ca;
        else
            return Cb2;   


    } else { //Calculate
        
        Cr=r;
        Cg=g;
        Cb=b;

        rVal = (double) r;
        gVal = (double) g;
        bVal = (double) b;

        xVal = 0.412453 * rVal + 0.357580 * gVal + 0.180423 * bVal;
        xVal /= (255.0 * 0.950456);
        yVal = 0.212671 * rVal + 0.715160 * gVal + 0.072169 * bVal;
        yVal /=  255.0;
        zVal = 0.019334 * rVal + 0.119193 * gVal + 0.950227 * bVal;
        zVal /= (255.0 * 1.088754);
        

        /*
        //Provided by Bill MacBeth
        xVal = 0.00170178 *rVal + 0.00147537 * gVal + 0.000744423 * bVal;
        yVal = 0.000834004 * rVal + 0.00280455 * gVal + 0.000283016 * bVal;
        zVal = 0.0000696389 * rVal + 0.000429320 * gVal + 0.00342261 * bVal;
        */

        if (yVal > 0.008856){
          fY = pow(yVal, 1.0 / 3.0);
          lVal = 116.0 * fY - 16.0;
        } else {
          fY = 7.787 * yVal + 16.0 / 116.0;
          lVal = 903.3 * yVal;
        }
        
        if (xVal > 0.008856)
            fX = pow(xVal, 1.0 / 3.0);
        else
            fX = 7.787 * xVal + 16.0 / 116.0;
                
        if (zVal > 0.008856)
            fZ = pow(zVal, 1.0 / 3.0);
        else
            fZ = 7.787 * zVal + 16.0 / 116.0;


        lVal = lVal * 2.56;
        aVal = 500.0 * (fX - fY)+128.0;
        bVal = 200.0 * (fY - fZ)+128.0;

        Cl=(int) lVal;
        Ca=(int) aVal;
        Cb2=(int) bVal;

        if (z==0)
            return Cl;
        else if (z==1)
            return Ca;
        else
            return Cb2;   
    }


}


//function added by Harald Heim on Dec 7, 2002
int lab2rgb(int l, int a, int b, int z)
{
    double rVal, gVal, bVal;
    double xVal, yVal, zVal;
    double lVal, aVal;
    double fX, fY, fZ;

    THREAD_LOCAL static int Cr,Cg,Cb;
    THREAD_LOCAL static int Cl,Ca,Cb2;


    if (l==Cl && a==Ca && b==Cb2){ //Read from Cache

        if (z==0) {         
            return Cr;
        } else if (z==1) {
            return Cg;
        } else {
            return Cb;
        }

    } else { //Calculate

        Cl=l;
        Ca=a;
        Cb2=b;

        lVal = (double) l;
        aVal = (double) a;
        bVal = (double) b;


        lVal= lVal / 2.56;
        aVal = aVal - 128;
        bVal = bVal - 128;


        fY = pow((lVal + 16.0) / 116.0, 3.0);
        if (fY < 0.008856)
            fY = lVal / 903.3;
        yVal = fY;

        if (fY > 0.008856)
        fY = pow(fY, 1.0 / 3.0);
        else
        fY = 7.787 * fY + 16.0 / 116.0;

        fX = aVal / 500.0 + fY;
        if (fX > 0.206893)
            xVal = pow(fX, 3.0);
        else
            xVal = (fX - 16.0 / 116.0) / 7.787;

        fZ = fY - bVal /200.0;
        if (fZ > 0.206893)
            zVal = pow(fZ, 3.0);
        else
            zVal = (fZ - 16.0 / 116.0) / 7.787;

        
        xVal *= (0.950456 * 255.0);
        yVal *=             255.0;
        zVal *= (1.088754 * 255.0);
        rVal = xVal *  3.240479 - yVal * 1.537150 - zVal * 0.498535;
        gVal = xVal * -0.969256 + yVal * 1.875992 + zVal * 0.041556;
        bVal = xVal *  0.055648 - yVal * 0.204043 + zVal * 1.057311;
        
        /*
        //Provided by Bill MacBeth
        rVal =   xVal * 7.853828E+02 - yVal * 6.342260E-03 - zVal * 2.056949E-03;
        gVal = - xVal * 2.471603E+02 + yVal * 7.356831E-03 + zVal * 1.629647E-04;
        bVal =   xVal * 1.544968E+01 - yVal * 7.349398E-04 + zVal * 3.808315E-03;
        */

        Cr=(int) (rVal + 0.5);
        Cg=(int) (gVal + 0.5);
        Cb=(int) (bVal + 0.5);

        if (z==0) {         
            return Cr;
        } else if (z==1) {
            return Cg;
        } else {
            return Cb;
        }

    }


}

//function added by Harald Heim on Dec 13, 2002
int setClickDrag(int b)
{
    //b=0 for left click drag
    //b=1 for right click drag
    //b=2 for no click drag at all
    ClickDrag=b;
    return 1;
}


/*
VOID CALLBACK MyTimerProc( 
    HWND hwnd,        // handle to window for timer messages 
    UINT message,     // WM_TIMER message 
    UINT idTimer,     // timer identifier 
    DWORD dwTime)     // current system time 
{ 

        fmc.n = idTimer-3;
        fmc.e = FME_TIMER;
        fmc.previous = 0;//previous;
        fmc.ctlMsg = WM_TIMER;//event;//WM_SIZE;
        fmc.ctlCode = 0;
        fmc.ctlClass = 0;//gParams->ctl[n].ctlclass;
        
        X86_call_fmf1(gParams->CB_onCtl, (int)&fmc);

}*/


//function added by Harald Heim on Dec 13, 2002
int setTimerEvent(int nr, int t, int state)
{
    //nr -> timer number 0 - 9
    //t -> trigger time
    //state==0 -> kill timer
    //state==1 -> set timer
    unsigned int Retval;

    if (nr<0 && nr>9) return 0;

    KillTimer(fmc.hDlg, 3 + nr); //Cancel any previous trigger.
    
    if (state==1){
        //Retval = SetTimer(fmc.hDlg, 3 + nr, t, (TIMERPROC) MyTimerProc); //Don't use message cue
        Retval = (int)SetTimer(fmc.hDlg, 3 + nr, t, NULL);
    }

    return (int) Retval;
}



int getDisplaySettings(int s)
{
   DEVMODE dm;
   dm.dmSize = sizeof(DEVMODE);
   dm.dmDriverExtra = 0;

   EnumDisplaySettings(NULL,ENUM_CURRENT_SETTINGS,&dm);
   
   switch (s) {   
        case 0:
            return (int) dm.dmBitsPerPel;
        case 1:
            return (int) dm.dmPelsWidth;
        case 2:
            return (int) dm.dmPelsHeight;
        case 3:
            return (int) dm.dmDisplayFrequency; 
   }
    return 0;

}

//function added by Harald Heim on Dec 14, 2002
int scrollPreview(int mode, int ox, int oy, int update)
{

    int RetVal;
#if BIGDOCUMENT
	VRect scaledFilterRect;
#else
    Rect scaledFilterRect;
#endif
    
    if (mode==1){
        ox = ox - (fmc.X-1)/2;
        oy = oy - (fmc.Y-1)/2;
    }

    RetVal = sub_scrollPreview(fmc.hDlg, mode, ox, oy, update);

    //Setup values
        
#if BIGDOCUMENT
	fmc.filterRect32 = GetFilterRect();	//needed?
	scaledFilterRect = fmc.filterRect32;
    scaleRect32(&scaledFilterRect, 1, fmc.scaleFactor);
    X0 = scaledFilterRect.left;
    Y0 = scaledFilterRect.top;
	fmc.inRect32 = GetInRect();	//needed??
    fmc.x_start = fmc.inRect32.left - X0;
    fmc.y_start = fmc.inRect32.top - Y0;
#else
    copyRect (&scaledFilterRect, &gStuff->filterRect);
    scaleRect (&scaledFilterRect, 1, fmc.scaleFactor);
    X0 = scaledFilterRect.left;
    Y0 = scaledFilterRect.top;
    fmc.x_start = gStuff->inRect.left - X0;
    fmc.y_start = gStuff->inRect.top - Y0;
#endif    

    fmc.X = fmc.xmax = scaledFilterRect.right - X0;
    fmc.Y = fmc.ymax = scaledFilterRect.bottom - Y0;
    fmc.x_end = fmc.x_start + fmc.columns;
    fmc.y_end = fmc.y_start + fmc.rows;

    return RetVal;

}


//function added by Harald Heim on Dec 23, 2002
int rgb2hsl(int r, int g, int b, int z)
{

    //int h,s,l;
    double rf,gf,bf,hf,sf,lf, cmax,cmin,diff;
    
    THREAD_LOCAL static int Cr,Cg,Cb;
    THREAD_LOCAL static int Ch,Cs,Cl;


    if (r==Cr && g==Cg && b==Cb){ //Read from Cache

        if (z==0)
            return Ch;
        else if (z==1)
            return Cs;
        else
            return Cl;    


    } else { //Calculate
        
        Cr=r;
        Cg=g;
        Cb=b;

        //Scale rgb to float
        rf=(double) r / (255.0*mode_bitMultiply);
        gf=(double) g / (255.0*mode_bitMultiply);
        bf=(double) b / (255.0*mode_bitMultiply);


        //Convert RGB to HSL
        cmax = bf;
        cmin = bf;
        if (rf > cmax) cmax = rf;
        if (gf > cmax) cmax = gf;
        if (rf < cmin) cmin = rf;
        if (gf < cmin) cmin = gf;


        lf = (cmax + cmin)/2.0;

        if (cmax==cmin){
            sf = 0.0;
            hf = 0.0;
        } else {
              diff = cmax-cmin;
			  if (diff==0) {
                    sf = 0;
			  } else if (lf < 0.5) {
					sf = diff/(cmax+cmin); 
              } else {
					sf = diff/(2.0-cmax-cmin);
              }

              if (rf==cmax) {
					hf = (gf-bf)/diff;
              } else if (gf==cmax) {
					hf = 2.0 + (bf-rf)/diff;
              } else {
					hf = 4.0+(rf-gf)/diff;
              }
              hf = hf/6.0;
              if (hf < 0.0) { hf = hf+1.0;}
        }   

        
        //Scale hsl to int
        Ch=(int) (hf*255*mode_bitMultiply);
        Cs=(int) (sf*255*mode_bitMultiply);
        Cl=(int) (lf*255*mode_bitMultiply);


        if (z==0)
            return Ch;
        else if (z==1)
            return Cs;
        else
            return Cl;      

    }

}




//function added by Harald Heim on Dec 7, 2002
int hsl2rgb(int h, int s, int l, int z)
{

    //int r,g,b;
    double rf,gf,bf,hf,sf,lf, hf2, m1,m2;
    
    THREAD_LOCAL static int Cr,Cg,Cb;
    THREAD_LOCAL static int Ch,Cs,Cl;


    if (h==Ch && s==Cs && l==Cl){ //Read from Cache

        if (z==0)
            return Cr;
        else if (z==1)
            return Cg;
        else
            return Cb;    


    } else { //Calculate

        Ch=h;
        Cs=s;
        Cl=l;

        //Scale to float
        hf=(double) h / (255.0*mode_bitMultiply);
        sf=(double) s / (255.0*mode_bitMultiply);
        lf=(double) l / (255.0*mode_bitMultiply);
        

        //Convert HSL to RGB
        if (sf == 0.0) {
            rf = lf;
            gf = lf;
            bf = lf;
        } else {
                if (lf <= 0.5) {m2 = lf*(1.0+sf);} else {m2 = lf+sf-(lf*sf);}
                m1 = 2.0 * lf - m2;

            //Calculate Red
            hf2=hf+1.0/3.0;             
            if (hf2 < 0) {hf2 = hf2 + 1.0;}
            if (hf2 > 1) {hf2 = hf2 - 1.0;}
            if (6.0 * hf2 < 1){
                rf = (m1+(m2-m1)*hf2*6.0);}
            else{
                if (2.0 * hf2 < 1){
                   rf = m2;
                }
                else {
                   if (3.0*hf2 < 2.0) {rf = (m1+(m2-m1)*((2.0/3.0)-hf2)*6.0);} else {rf = m1;}
                } 
            }

            //Calculate Green               
            if (hf < 0) {hf = hf + 1.0;}
            if (hf > 1) {hf = hf - 1.0;}
            if (6.0 * hf < 1){
                gf = (m1+(m2-m1)*hf*6.0);}
            else {
                if (2.0 * hf < 1){
                   gf = m2;
                }
                else {
                   if (3.0*hf < 2.0) {gf = (m1+(m2-m1)*((2.0/3.0)-hf)*6.0);} else {gf = m1;}
                } 
            }

            //Calculate Blue
            hf2=hf-1.0/3.0;             
            if (hf2 < 0) {hf2 = hf2 + 1.0;}
            if (hf2 > 1) {hf2 = hf2 - 1.0;}
            if (6.0 * hf2 < 1) {
                bf = (m1+(m2-m1)*hf2*6.0);}
            else {
                if (2.0 * hf2 < 1){
                   bf = m2;
                }
                else {
                   if (3.0*hf2 < 2.0) {bf = (m1+(m2-m1)*((2.0/3.0)-hf2)*6.0);} else {bf = m1;}
                } 
            }

        }


        //Scale rgb to int
        Cr=(int) (rf*255*mode_bitMultiply);
        Cg=(int) (gf*255*mode_bitMultiply);
        Cb=(int) (bf*255*mode_bitMultiply);

        if (z==0)
            return Cr;
        else if (z==1)
            return Cg;
        else
            return Cb;

    }
}



void rgb2hsl_fast8(int * r, int * g, int * b)
{
    int h,s,l,cmax,cmin,diff;
    
	if (*r<0) *r=0; else if (*r>255) *r=255;
	if (*g<0) *g=0; else if (*g>255) *g=255;
	if (*b<0) *b=0; else if (*b>255) *b=255;

    cmax = *b;
    cmin = *b;
    if (*r > cmax) cmax = *r;
    if (*g > cmax) cmax = *g;
    if (*r < cmin) cmin = *r;
    if (*g < cmin) cmin = *g;

	l = (cmax + cmin)/2;
    if (cmax==cmin){
        s = 0;
        h = 0;
    } else {
		  diff = cmax-cmin;
		  if (diff==0) {
                s = 0;
		  } else if (l < mode_midVal) {
				s = diff*mode_maxVal/(cmax+cmin); 
          } else {
				s = diff*mode_maxVal/(2*mode_maxVal-cmax-cmin);
          }

          if (*r==cmax) {
				h = (*g-*b)*mode_maxVal/diff;
          } else if (*g==cmax) {
				h = 2*mode_maxVal + (*b-*r)*mode_maxVal/diff;
          } else {
				h = 4*mode_maxVal + (*r-*g)*mode_maxVal/diff;
          }
          h = h / 6;
          if (h < 0) h += mode_maxVal;
    }   

    *r =h;
    *g =s;
    *b =l;
}

void hsl2rgb_fast8(int * h, int * s, int * l)
{
    int r,g,b, h2,m1,m2;
	int h1 = *h;
	int s1 = *s;
	int l1 = *l;

    if (s == 0) {
        r = l1;
        g = l1;
        b = l1;
    } else {
        if (l1 <= mode_midVal) m2 = l1 * (mode_maxVal+s1) / mode_maxVal; 
		else m2 = (l1+s1) - (l1 * s1)/mode_maxVal;
        m1 = 2 * l1 - m2;

        //Calculate Red
        h2=h1+mode_maxVal/3;
        if (h2 < 0) h2 = h2 + mode_maxVal;
        if (h2 > mode_maxVal) h2 = h2 - mode_maxVal;
		if (6 * h2 < mode_maxVal) r = (m1 + (m2-m1)*h2*6 / mode_maxVal);
        else if (2 * h2 < mode_maxVal) r = m2;
        else if (3 * h2 < 2*mode_maxVal) r = (m1 + (m2-m1)*((2*mode_maxVal/3)-h2)*6 / mode_maxVal);
		else r = m1;
        
        //Calculate Green               
        if (h1 < 0) h1 = h1 + mode_maxVal;
        if (h1 > mode_maxVal) h1 = h1 - mode_maxVal;
        if (6 * h1 < mode_maxVal) g = (m1+(m2-m1)* h1 *6 / mode_maxVal);
        else if (2 * h1 < mode_maxVal) g = m2;
        else if (3 * h1 < 2*mode_maxVal) g = (m1+(m2-m1)*((2*mode_maxVal/3)-h1)*6 / mode_maxVal);
		else g = m1;
        
        //Calculate Blue
        h2=h1-mode_maxVal/3;
        if (h2 < 0) h2 = h2 + mode_maxVal;
        if (h2 > mode_maxVal) h2 = h2 - mode_maxVal;
        if (6 * h2 < mode_maxVal) b = (m1+(m2-m1)*h2*6 / mode_maxVal);
        else if (2 * h2 < mode_maxVal) b = m2;
        else if (3 * h2 < 2*mode_maxVal) b = (m1+(m2-m1)*((2*mode_maxVal/3)-h2)*6 / mode_maxVal);
		else b = m1;
    }

    *h=r;
    *s=g;
    *l=b;
}


void rgb2hsl_fast16(int * r, int * g, int * b)
{
    double h,s,l,cmax,cmin,diff;
	double rf,gf,bf;
 
	rf=(double) *r / 32768.0;
    gf=(double) *g / 32768.0;
    bf=(double) *b / 32768.0;

    cmax = bf;
    cmin = bf;
    if (rf > cmax) cmax = rf;
    if (gf > cmax) cmax = gf;
    if (rf < cmin) cmin = rf;
    if (gf < cmin) cmin = gf;

	l = (cmax + cmin)/2.0;
    if (cmax==cmin){
        s = 0.0;
        h = 0.0;
    } else {
		  diff = cmax-cmin;
		  if (diff==0) {
                s = 0;
		  } else if (l < 0.5) {
				s = diff/(cmax+cmin); 
          } else {
				s = diff/(2.0-cmax-cmin);
          }

          if (rf==cmax) {
				h = (gf-bf)/diff;
          } else if (gf==cmax) {
				h = 2.0 + (bf-rf)/diff;
          } else {
				h = 4.0 + (rf-gf)/diff;
          }
          h = h / 6.0;
          if (h < 0) h += 1.0;
    }   

    *r =(int)(h*32768.0);
    *g =(int)(s*32768.0);
    *b =(int)(l*32768.0);
}

void hsl2rgb_fast16(int * h, int * s, int * l)
{
    double r,g,b, h2,m1,m2;
	double h1=(double) *h / 32768.0;
    double s1=(double) *s / 32768.0;
    double l1=(double) *l / 32768.0;
	
    if (s == 0) {
        r = l1;
        g = l1;
        b = l1;
    } else {
        if (l1 <= 0.5) m2 = l1 * (1.0+s1);
		else m2 = (l1+s1) - (l1 * s1);
        m1 = 2.0 * l1 - m2;

        //Calculate Red
        h2=h1+1.0/3.0;
        if (h2 < 0) h2 = h2 + 1.0;
        if (h2 > 1) h2 = h2 - 1.0;
		if (6.0 * h2 < 1) r = (m1 + (m2-m1)*h2*6.0);
        else if (2.0 * h2 < 1) r = m2;
        else if (3.0 * h2 < 2.0) r = (m1 + (m2-m1)*((2.0/3.0)-h2)*6.0);
		else r = m1;
        
        //Calculate Green               
        if (h1 < 0) h1 = h1 + 1.0;
        if (h1 > 1) h1 = h1 - 1.0;
        if (6.0 * h1 < 1) g = (m1+(m2-m1)* h1 *6.0);
        else if (2.0 * h1 < 1) g = m2;
        else if (3.0 * h1 < 2.0) g = (m1+(m2-m1)*((2.0/3.0)-h1)*6.0);
		else g = m1;
        
        //Calculate Blue
        h2=h1-1.0/3.0;
        if (h2 < 0) h2 = h2 + 1.0;
        if (h2 > 1) h2 = h2 - 1.0;
        if (6.0 * h2 < 1) b = (m1+(m2-m1)*h2*6.0);
        else if (2.0 * h2 < 1) b = m2;
        else if (3.0 * h2 < 2.0) b = (m1+(m2-m1)*((2.0/3.0)-h2)*6.0);
		else b = m1;
    }

    *h=(int)(r*32768.0);
    *s=(int)(g*32768.0);
    *l=(int)(b*32768.0);
}



int rgb2hsl_safe(int r, int g, int b, int z)
{

    double rf,gf,bf,hf,sf,lf, cmax,cmin,diff;
    
	int Cr,Cg,Cb;
    int Ch,Cs,Cl;


        Cr=r;
        Cg=g;
        Cb=b;

        //Scale rgb to float
        rf=(double) r / (255.0*mode_bitMultiply);
        gf=(double) g / (255.0*mode_bitMultiply);
        bf=(double) b / (255.0*mode_bitMultiply);


        //Convert RGB to HSL
        cmax = bf;
        cmin = bf;
        if (rf > cmax) cmax = rf;
        if (gf > cmax) cmax = gf;
        if (rf < cmin) cmin = rf;
        if (gf < cmin) cmin = gf;


        lf = (cmax + cmin)/2.0;

        if (cmax==cmin){
            sf = 0.0;
            hf = 0.0;
        } else {
              diff = cmax-cmin;
			  if (diff==0) {
                    sf = 0;
			  } else if (lf < 0.5) {
					sf = diff/(cmax+cmin); 
              } else {
					sf = diff/(2.0-cmax-cmin);
              }

              if (rf==cmax) {
					hf = (gf-bf)/diff;
              } else if (gf==cmax) {
					hf = 2.0 + (bf-rf)/diff;
              } else {
					hf = 4.0+(rf-gf)/diff;
              }
              hf = hf/6.0;
              if (hf < 0.0) { hf = hf+1.0;}
        }   

        
        //Scale hsl to int
        Ch=(int) (hf*255*mode_bitMultiply);
        Cs=(int) (sf*255*mode_bitMultiply);
        Cl=(int) (lf*255*mode_bitMultiply);


        if (z==0)
            return Ch;
        else if (z==1)
            return Cs;
        else
            return Cl;      

}


int hsl2rgb_safe(int h, int s, int l, int z)
{

    //int r,g,b;
    double rf,gf,bf,hf,sf,lf, hf2, m1,m2;
    
    int Cr,Cg,Cb;
    int Ch,Cs,Cl;


    Ch=h;
    Cs=s;
    Cl=l;

    //Scale to float
    hf=(double) h / (255.0*mode_bitMultiply);
    sf=(double) s / (255.0*mode_bitMultiply);
    lf=(double) l / (255.0*mode_bitMultiply);
    

    //Convert HSL to RGB
    if (sf == 0.0) {
        rf = lf;
        gf = lf;
        bf = lf;
    } else {
            if (lf <= 0.5) {m2 = lf*(1.0+sf);} else {m2 = lf+sf-(lf*sf);}
            m1 = 2.0 * lf - m2;

        //Calculate Red
        hf2=hf+1.0/3.0;             
        if (hf2 < 0) {hf2 = hf2 + 1.0;}
        if (hf2 > 1) {hf2 = hf2 - 1.0;}
        if (6.0 * hf2 < 1){
            rf = (m1+(m2-m1)*hf2*6.0);}
        else{
            if (2.0 * hf2 < 1){
               rf = m2;
            }
            else {
               if (3.0*hf2 < 2.0) {rf = (m1+(m2-m1)*((2.0/3.0)-hf2)*6.0);} else {rf = m1;}
            } 
        }

        //Calculate Green               
        if (hf < 0) {hf = hf + 1.0;}
        if (hf > 1) {hf = hf - 1.0;}
        if (6.0 * hf < 1){
            gf = (m1+(m2-m1)*hf*6.0);}
        else {
            if (2.0 * hf < 1){
               gf = m2;
            }
            else {
               if (3.0*hf < 2.0) {gf = (m1+(m2-m1)*((2.0/3.0)-hf)*6.0);} else {gf = m1;}
            } 
        }

        //Calculate Blue
        hf2=hf-1.0/3.0;             
        if (hf2 < 0) {hf2 = hf2 + 1.0;}
        if (hf2 > 1) {hf2 = hf2 - 1.0;}
        if (6.0 * hf2 < 1) {
            bf = (m1+(m2-m1)*hf2*6.0);}
        else {
            if (2.0 * hf2 < 1){
               bf = m2;
            }
            else {
               if (3.0*hf2 < 2.0) {bf = (m1+(m2-m1)*((2.0/3.0)-hf2)*6.0);} else {bf = m1;}
            } 
        }

    }


    //Scale rgb to int
    Cr=(int) (rf*255*mode_bitMultiply);
    Cg=(int) (gf*255*mode_bitMultiply);
    Cb=(int) (bf*255*mode_bitMultiply);

    if (z==0)
        return Cr;
    else if (z==1)
        return Cg;
    else
        return Cb;

}






//function added by Harald Heim on Feb 6, 2002
int rgb2ycbcr(int r, int g, int b, int z)
{
    // [  Y'  601 ] [ 0.299     0.587     0.114    ] [ R' ] 
    // [  PB  601 ]=[-0.168736 -0.331264  0.5      ]*[ G' ] 
    // [  PR  601 ] [ 0.5      -0.418688 -0.081312 ] [ B' ]   
    
    if (z == 0)
        return (299*r + 587*g + 114*b)/1000;
    else if (z == 1)
        return (-169*r - 331*g + 500*b)/1000 + 128*mode_bitMultiply;
    else if (z == 2)
        return (500*r - 419*g - 81*b)/1000 + 128*mode_bitMultiply;
    else
        return 0;
}

//function added by Harald Heim on Feb 8, 2002
int ycbcr2rgb(int y, int cb, int cr, int z)
{
    cb = cb - 128*mode_bitMultiply;
    cr = cr - 128*mode_bitMultiply;
    
    //[ R' ] [ 1.        0.        1.402    ] [  Y'  601 ] 
    //[ G' ]=[ 1.       -0.344136 -0.714136 ]*[  PB  601 ] 
    //[ B' ] [ 1.        1.772     0.       ] [  PR  601 ]  

    if (z == 0)
        return y + 1402*cr/1000; 
    else if (z == 1)
        return y - (344*cb + 714*cr)/1000; 
    else if (z == 2)
        return y + 1772*cb/1000;
    else
        return 0;
}


//function added by Harald Heim on Jun 17, 2003
int rgb2ypbpr(int r, int g, int b, int z)
{
    //Y=  0.2122*Red+0.7013*Green+0.0865*Blue    
    //Pb=-0.1162*Red-0.3838*Green+0.5000*Blue    
    //Pr= 0.5000*Red-0.4451*Green-0.0549*Blue
    
    if (z == 0)
        return (212*r + 701*g + 87*b)/1000;
    else if (z == 1)
        return (-116*r - 384*g + 500*b)/1000 + 128*mode_bitMultiply;
    else if (z == 2)
        return (500*r - 445*g - 55*b)/1000 + 128*mode_bitMultiply;
    else
        return 0;
}

//function added by Harald Heim on Jun 17, 2003
int ypbpr2rgb(int y, int pb, int pr, int z)
{
    pb = pb - 128*mode_bitMultiply;
    pr = pr - 128*mode_bitMultiply;
    
    //Red  =1*Y+0.0000*Pb+1.5756*Pr
    //Green=1*Y-0.2253*Pb+0.5000*Pr
    //Blue =1*Y+1.8270*Pb+0.0000*Pr

    if (z == 0)
        return y + 1576*pr/1000; 
    else if (z == 1)
        return y - (225*pb + 500*pr)/1000; 
    else if (z == 2)
        return y + 1827*pb/1000;
    else
        return 0;
}


//// Optimizations: Pluggable variants based on Z, bit depth, etc.?
////                Use aligned 1 byte/2 byte or 2 byte/1 byte accesses?
////                Factor out subscript expression if VC doesn't.

//// How about  srcRGB(), srcRGBA(), srcGrey(), srcDuo(), srcCMYK(), etc.
////         or src3(), src4(), src1(), src2(), src4(), ...
//// Ditto for  pgetRGB(), pgetRGBA(), etc.?

//Added by Harald Heim, January 2003

int srcp(int x, int y)
//int srcp(UINT_PTR x, UINT_PTR y)
{   
    if (x < 0) x = 0;
    else if (x >= fmc.X) x = fmc.X - 1;
    if (y < 0) y = 0;
    else if (y >= fmc.Y) y = fmc.Y - 1;


    if ((x == fmc.X-1 && y == fmc.Y-1) || (fmc.isTileable && x == fmc.x_end-1 && y == fmc.y_end-1)){ //Second condition to avoid problems in PSP8

        int r,g,b,a;

        UINT_PTR pos = (x-fmc.x_start)*fmc.srcColDelta + (y-fmc.y_start)*fmc.srcRowDelta;

        r = ((unsigned8 *)MyAdjustedInData) [pos];
        g = ((unsigned8 *)MyAdjustedInData) [pos + 1];
        b = ((unsigned8 *)MyAdjustedInData) [pos + 2];

        if (fmc.Z>3){
            a = ((unsigned8 *)MyAdjustedInData) [pos + 3];
        } else 
            a = 255;

        return (((a & 0xff) << 8 | (b & 0xff)) << 8 | (g & 0xff)) << 8 | (r & 0xff);
    

    } else { //if (x < fmc.X-1  || y < fmc.Y-1){

        return *(unsigned32 *)(((unsigned8 *)MyAdjustedInData) +
                               (x-fmc.x_start)*fmc.srcColDelta +
                               (y-fmc.y_start)*fmc.srcRowDelta   );
    }


}


int pgetp(int x, int y)
{   

    if (x < fmc.x_start) x = fmc.x_start;
    else if (x >= fmc.x_end) x = fmc.x_end - 1;
    if (y < fmc.y_start) y = fmc.y_start;
    else if (y >= fmc.y_end) y = fmc.y_end - 1;

    
    if ((x == fmc.X-1 && y == fmc.Y-1) || (fmc.isTileable && x == fmc.x_end-1 && y == fmc.y_end-1)){ //Second condition to avoid problems in PSP8
    
        int r,g,b,a;
        
        int pos = (x-fmc.x_start)*fmc.dstColDelta + (y-fmc.y_start)*fmc.dstRowDelta;

        r = ((unsigned8 *)gStuff->outData) [pos];
        g = ((unsigned8 *)gStuff->outData) [pos + 1];
        b = ((unsigned8 *)gStuff->outData) [pos + 2];

        if (fmc.Z>3){
            a = ((unsigned8 *)gStuff->outData) [pos + 3];
        } else 
            a = 255;

        return (((a & 0xff) << 8 | (b & 0xff)) << 8 | (g & 0xff)) << 8 | (r & 0xff);


    } else { //if (x < fmc.X  && y < fmc.Y-1){
        
        return  *(unsigned32 *)(((unsigned8 *)gStuff->outData) +
                                (x-fmc.x_start)*fmc.dstColDelta + 
                                (y-fmc.y_start)*fmc.dstRowDelta  );                 
    }


}



int tgetp(int x, int y)
{   

    //int r,g,b;

    //Added by Harald Heim, April 20, 2003
    if (x < 0) x = 0;
    else if (x >= fmc.X) x = fmc.X - 1;
    if (y < 0) y = 0;
    else if (y >= fmc.Y) y = fmc.Y - 1;


    return *(unsigned32 *) (fmc.tbuf + (y*fmc.X + x)*fmc.Z );


    /*r= fmc.tbuf[(y*fmc.X + x)*fmc.Z + 0];
    g= fmc.tbuf[(y*fmc.X + x)*fmc.Z + 1];
    b= fmc.tbuf[(y*fmc.X + x)*fmc.Z + 2];

    return ((b & 0xff) << 8 | (g & 0xff)) << 8 | (r & 0xff);*/
    

}

//// Overlaps when Z<4???

//// Use "pluggable" versions of tsetp: tsetp4 when Z >=4;
///  tsetp1, tsetp2, tsetp3 when Z==1, 2, 3, resp.

int tsetp(int x, int y, int val)
{   
    //int r,g,b;

    //r = val & 0xff;
    //g = val >> 8 & 0xff;
    //b = val >> 16 & 0xff;

    /// WHY CLAMP COORDINATES?  Shouldn't the tsetp be ignored if out of bounds????
    if (x < 0) x = 0;
    else if (x >= fmc.X) x = fmc.X - 1;
    if (y < 0) y = 0;
    else if (y >= fmc.Y) y = fmc.Y - 1;

    
    *(unsigned32 *) ( fmc.tbuf + (y*fmc.X + x)*fmc.Z ) = val;

    //fmc.tbuf[(y*fmc.X + x)*fmc.Z + 0] = r;
    //fmc.tbuf[(y*fmc.X + x)*fmc.Z + 1] = g;
    //fmc.tbuf[(y*fmc.X + x)*fmc.Z + 2] = b;

    return val;

}

int t2getp(int x, int y)
{   

    //int r,g,b;

    //Added by Harald Heim, April 20, 2003
    if (x < 0) x = 0;
    else if (x >= fmc.X) x = fmc.X - 1;
    if (y < 0) y = 0;
    else if (y >= fmc.Y) y = fmc.Y - 1;


    return *(unsigned32 *) (fmc.t2buf + (y*fmc.X + x)*fmc.Z );


    /*r= fmc.t2buf[(y*fmc.X + x)*fmc.Z + 0];
    g= fmc.t2buf[(y*fmc.X + x)*fmc.Z + 1];
    b= fmc.t2buf[(y*fmc.X + x)*fmc.Z + 2];

    return ((b & 0xff) << 8 | (g & 0xff)) << 8 | (r & 0xff);*/
    

}

int t3getp(int x, int y)
{   

    //int r,g,b;

    //Added by Harald Heim, April 20, 2003
    if (x < 0) x = 0;
    else if (x >= fmc.X) x = fmc.X - 1;
    if (y < 0) y = 0;
    else if (y >= fmc.Y) y = fmc.Y - 1;


    return *(unsigned32 *) (fmc.t3buf + (y*fmc.X + x)*fmc.Z );


    /*r= fmc.t3buf[(y*fmc.X + x)*fmc.Z + 0];
    g= fmc.t3buf[(y*fmc.X + x)*fmc.Z + 1];
    b= fmc.t3buf[(y*fmc.X + x)*fmc.Z + 2];

    return ((b & 0xff) << 8 | (g & 0xff)) << 8 | (r & 0xff);*/
    

}

int t4getp(int x, int y)
{   

    //int r,g,b;

    //Added by Harald Heim, April 20, 2003
    if (x < 0) x = 0;
    else if (x >= fmc.X) x = fmc.X - 1;
    if (y < 0) y = 0;
    else if (y >= fmc.Y) y = fmc.Y - 1;


    return *(unsigned32 *) (fmc.t4buf + (y*fmc.X + x)*fmc.Z );


    /*r= fmc.t4buf[(y*fmc.X + x)*fmc.Z + 0];
    g= fmc.t4buf[(y*fmc.X + x)*fmc.Z + 1];
    b= fmc.t4buf[(y*fmc.X + x)*fmc.Z + 2];

    return ((b & 0xff) << 8 | (g & 0xff)) << 8 | (r & 0xff);*/
    

}

//// Use "pluggable" versions of psetp: psetp4 when Z >=4;
///  psetp1, psetp2, psetp3 when Z==1, 2, 3, resp.

int t2setp(int x, int y, int val)
{   
    //int r,g,b;

    //r = val & 0xff;
    //g = val >> 8 & 0xff;
    //b = val >> 16 & 0xff;

    /// WHY CLAMP COORDINATES?  Shouldn't the setp be ignored if out of bounds????
    if (x < 0) x = 0;
    else if (x >= fmc.X) x = fmc.X - 1;
    if (y < 0) y = 0;
    else if (y >= fmc.Y) y = fmc.Y - 1;

    
    *(unsigned32 *) ( fmc.t2buf + (y*fmc.X + x)*fmc.Z ) = val;

    //fmc.tbuf[(y*fmc.X + x)*fmc.Z + 0] = r;
    //fmc.tbuf[(y*fmc.X + x)*fmc.Z + 1] = g;
    //fmc.tbuf[(y*fmc.X + x)*fmc.Z + 2] = b;

    return val;

}

int t3setp(int x, int y, int val)
{   
    //int r,g,b;

    //r = val & 0xff;
    //g = val >> 8 & 0xff;
    //b = val >> 16 & 0xff;
    if (x < 0) x = 0;
    else if (x >= fmc.X) x = fmc.X - 1;
    if (y < 0) y = 0;
    else if (y >= fmc.Y) y = fmc.Y - 1;

    
    *(unsigned32 *) ( fmc.t3buf + (y*fmc.X + x)*fmc.Z ) = val;

    //fmc.t3buf[(y*fmc.X + x)*fmc.Z + 0] = r;
    //fmc.t3buf[(y*fmc.X + x)*fmc.Z + 1] = g;
    //fmc.t3buf[(y*fmc.X + x)*fmc.Z + 2] = b;

    return val;

}

int t4setp(int x, int y, int val)
{   
    //int r,g,b;

    //r = val & 0xff;
    //g = val >> 8 & 0xff;
    //b = val >> 16 & 0xff;
    if (x < 0) x = 0;
    else if (x >= fmc.X) x = fmc.X - 1;
    if (y < 0) y = 0;
    else if (y >= fmc.Y) y = fmc.Y - 1;

    
    *(unsigned32 *) ( fmc.t4buf + (y*fmc.X + x)*fmc.Z ) = val;

    //fmc.t4buf[(y*fmc.X + x)*fmc.Z + 0] = r;
    //fmc.t4buf[(y*fmc.X + x)*fmc.Z + 1] = g;
    //fmc.t4buf[(y*fmc.X + x)*fmc.Z + 2] = b;

    return val;

}

int psetp(int x, int y, int val)
{   

    int r,g,b,a, pos;

    /// WHY CLAMP COORDINATES?  Shouldn't the setp be ignored if out of bounds????
    if (x < fmc.x_start) x = fmc.x_start;
    else if (x >= fmc.x_end) x = fmc.x_end - 1;
    if (y < fmc.y_start) y = fmc.y_start;
    else if (y >= fmc.y_end) y = fmc.y_end - 1;


    //if ((x == fmc.X-1 && y == fmc.Y-1) || (fmc.isTileable && x == fmc.x_end-1 && y == fmc.y_end-1)) //Second condition to avoid problems in PSP8
    //{

        //// Don't need to mask bytes with 0xff (does VC optimize it away?).
        //// if Z > 3, just do 32bit store?
    
        r = val & 0xff;
        g = val >> 8 & 0xff;
        b = val >> 16 & 0xff;
        
        pos = (x-fmc.x_start)*fmc.dstColDelta + (y-fmc.y_start)*fmc.dstRowDelta;

        ((unsigned8 *)gStuff->outData) [pos] = r;
        ((unsigned8 *)gStuff->outData) [pos + 1] = g;
        ((unsigned8 *)gStuff->outData) [pos + 2] = b;

        if (fmc.Z>3){
            a =  val >> 24 & 0xff;

            if (psetpMode || a>0){ //Avoid messup of alpha channel and being forced to read it out, only if set_psetp_mode(1) was used.
                ((unsigned8 *)gStuff->outData)[pos + 3] = a;
            }
        }


    /*} else {


        *(unsigned32 *)( ((unsigned8 *)gStuff->outData) +
                                (x-fmc.x_start)*fmc.dstColDelta  +
                                (y-fmc.y_start)*fmc.dstRowDelta    )
                        =  val;
    }*/

    return 1;

}


/*
#include <Lm.h> //Requires Netapi32.lib

bool GetNewerWindowsVersion(DWORD& major, DWORD& minor)
{	

#ifndef FILTERHUB

	LPBYTE pinfoRawData;
	if (NERR_Success == NetWkstaGetInfo(NULL, 100, &pinfoRawData))
	{
		WKSTA_INFO_100 * pworkstationInfo = (WKSTA_INFO_100 *)pinfoRawData;
		major = pworkstationInfo->wki100_ver_major;
		minor = pworkstationInfo->wki100_ver_minor;
		NetApiBufferFree(pinfoRawData);
		return true;
	}

#endif

	return false;
}
*/


int getWindowsVersion(void)
{
    static int version = 0;
    
    OSVERSIONINFO osvi;
    BOOL bOsVersionInfoEx;

	if (version != 0) return version; //Use cached value


    osvi.dwOSVersionInfoSize = sizeof(OSVERSIONINFO);
    if( !(bOsVersionInfoEx = GetVersionEx ((OSVERSIONINFO *) &osvi)) ){
      osvi.dwOSVersionInfoSize = sizeof (OSVERSIONINFO);
      if (! GetVersionEx ( (OSVERSIONINFO *) &osvi) ) return FALSE;
    }

    switch (osvi.dwPlatformId) {

      //Windows NT product family.
      case VER_PLATFORM_WIN32_NT:

		
		 if ( osvi.dwMajorVersion >= 7 || (osvi.dwMajorVersion == 6 && osvi.dwMinorVersion >= 2) ){
			//For newer Windows version GetVersionEx() does not work
			DWORD major = 0; 
			DWORD minor = 0;
			
			version = 13; //Windows 8 - 10

/*
			if (GetNewerWindowsVersion(major,minor)){
				//Info("%d %d",major,minor);
				if (major==6 && minor==3)
					version = 14; //Windows 8.1
				else if ((major==6 && minor>=4) || major>=7) //major>=10
					version = 15; //Windows 10 -> version number 10.0, Win10 betas may return 6.4
			}
*/
		 } else if ( osvi.dwMajorVersion == 6 && osvi.dwMinorVersion == 1 ) //&& osvi.dwMinorVersion == 0
            version = 12; //Windows 7
         else if ( osvi.dwMajorVersion == 6) //&& osvi.dwMinorVersion == 0
            version = 11; //Windows Vista
         else if ( osvi.dwMajorVersion == 5 && osvi.dwMinorVersion == 2 )
            version = 10; //Windows Server 2003
         else if ( osvi.dwMajorVersion == 5 && osvi.dwMinorVersion == 1 )
            version = 9; //Windows XP 
         else if ( osvi.dwMajorVersion == 5 && osvi.dwMinorVersion == 0 )
            version = 8;//Windows 2000 
         else if ( osvi.dwMajorVersion <= 4 )
            version = 7;//Windows NT 

         /*
         // Test for specific product on Windows NT 4.0 SP6 and later.
         if( bOsVersionInfoEx )
         {
            // Test for the workstation type.
            if ( osvi.wProductType == VER_NT_WORKSTATION )
            {
               if( osvi.dwMajorVersion == 4 )
                  printf ( "Workstation 4.0 " );
               else if( osvi.wSuiteMask & VER_SUITE_PERSONAL )
                  printf ( "Home Edition " );
               else
                  printf ( "Professional " );
            }
            
            // Test for the server type.
            else if ( osvi.wProductType == VER_NT_SERVER )
            {
               if( osvi.dwMajorVersion == 5 && osvi.dwMinorVersion == 2 )
               {
                  if( osvi.wSuiteMask & VER_SUITE_DATACENTER )
                     printf ( "Datacenter Edition " );
                  else if( osvi.wSuiteMask & VER_SUITE_ENTERPRISE )
                     printf ( "Enterprise Edition " );
                  else if ( osvi.wSuiteMask == VER_SUITE_BLADE )
                     printf ( "Web Edition " );
                  else
                     printf ( "Standard Edition " );
               }

               else if( osvi.dwMajorVersion == 5 && osvi.dwMinorVersion == 0 )
               {
                  if( osvi.wSuiteMask & VER_SUITE_DATACENTER )
                     printf ( "Datacenter Server " );
                  else if( osvi.wSuiteMask & VER_SUITE_ENTERPRISE )
                     printf ( "Advanced Server " );
                  else
                     printf ( "Server " );
               }

               else  // Windows NT 4.0 
               {
                  if( osvi.wSuiteMask & VER_SUITE_ENTERPRISE )
                     printf ("Server 4.0, Enterprise Edition " );
                  else
                     printf ( "Server 4.0 " );
               }
            }
         }
         else  // Test for specific product on Windows NT 4.0 SP5 and earlier
         {
            HKEY hKey;
            char szProductType[BUFSIZE];
            DWORD dwBufLen=BUFSIZE;
            LONG lRet;

            lRet = RegOpenKeyEx( HKEY_LOCAL_MACHINE,
               "SYSTEM\\CurrentControlSet\\Control\\ProductOptions",
               0, KEY_QUERY_VALUE, &hKey );
            if( lRet != ERROR_SUCCESS )
               return FALSE;

            lRet = RegQueryValueEx( hKey, "ProductType", NULL, NULL,
               (LPBYTE) szProductType, &dwBufLen);
            if( (lRet != ERROR_SUCCESS) || (dwBufLen > BUFSIZE) )
               return FALSE;

            RegCloseKey( hKey );

            if ( lstrcmpi( "WINNT", szProductType) == 0 )
               printf( "Workstation " );
            if ( lstrcmpi( "LANMANNT", szProductType) == 0 )
               printf( "Server " );
            if ( lstrcmpi( "SERVERNT", szProductType) == 0 )
               printf( "Advanced Server " );

            printf( "%d.%d ", osvi.dwMajorVersion, osvi.dwMinorVersion );
         }

      // Display service pack (if any) and build number.

         if( osvi.dwMajorVersion == 4 && 
             lstrcmpi( osvi.szCSDVersion, "Service Pack 6" ) == 0 )
         {
            HKEY hKey;
            LONG lRet;

            // Test for SP6 versus SP6a.
            lRet = RegOpenKeyEx( HKEY_LOCAL_MACHINE,
               "SOFTWARE\\Microsoft\\Windows NT\\CurrentVersion\\Hotfix\\Q246009",
               0, KEY_QUERY_VALUE, &hKey );
            if( lRet == ERROR_SUCCESS )
               printf( "Service Pack 6a (Build %d)\n", osvi.dwBuildNumber & 0xFFFF );         
            else // Windows NT 4.0 prior to SP6a
            {
               printf( "%s (Build %d)\n",
                  osvi.szCSDVersion,
                  osvi.dwBuildNumber & 0xFFFF);
            }

            RegCloseKey( hKey );
         }
         else // Windows NT 3.51 and earlier or Windows 2000 and later
         {
            printf( "%s (Build %d)\n",
               osvi.szCSDVersion,
               osvi.dwBuildNumber & 0xFFFF);
         }
        */

         break;

      // Test for the Windows 95 product family.
      case VER_PLATFORM_WIN32_WINDOWS:

         if (osvi.dwMajorVersion == 4 && osvi.dwMinorVersion == 0){
             version = 2; //Windows 95
             if ( osvi.szCSDVersion[1] == 'C' || osvi.szCSDVersion[1] == 'B' )
                version = 3; //Windows 95 OSR2
         } else if (osvi.dwMajorVersion == 4 && osvi.dwMinorVersion == 10) {
             version = 4; //Windows 98 
             if ( osvi.szCSDVersion[1] == 'A' )
                version = 5;//Windows 98 SE
         } else if (osvi.dwMajorVersion == 4 && osvi.dwMinorVersion == 90){
             version = 6;//Windows ME
         } 
         break;

      //Windows 3
	  case VER_PLATFORM_WIN32s:

         version = 1; //Win32s
         break;
    }

	return version;

}


int setPreviewCursor(INT_PTR res)
{

    if (GetForegroundWindow() == MyHDlg)
        return sub_setPreviewCursor(res);
    else
        return 0;
        
}

int getPreviewCursor(void)
{

    return (int)sub_getPreviewCursor();

}


//Added by Harald Heim, March 12, 2003
int deleteRgn(INT_PTR Rgn)
{

    return DeleteObject ((HGDIOBJ) Rgn);

}


//Added by Harald Heim, May 17, 2003
/*
int comp(const void *a, const void *b )
{
    return *(int *)a - * (int *)b;
}

int quickSort(int first, int last)
{

    if (first<0 || first> N_CELLS-1) return 0;
    if (last<0 || last> N_CELLS-1) return 0;
    if (first > last || first == last) return 0;

    //qsort( (void*) &fmc.cell[first] , (size_t) (last - first), sizeof(int), (int(*)(const void*, const void*)) comp );
    qsort( (void*) &fmc.cell[first] , (size_t) (last - first + 1), sizeof(int),  comp );

    return 1;

}
*/




//Added by Harald Heim, May 17, 2003
int quickSort(int first, int last)
{

    int temp;

    if (first<0 || first> N_CELLS-1 || last<0 || last> N_CELLS-1 || first > last || first == last) return 0;

    
    if (first < last){

        int pivot = fmc.cell[first];
        int i = first;
        int j = last;
        
        while (i < j){

            while (fmc.cell[i] <= pivot && i < last){
                    i += 1;
            }

            while (fmc.cell[j] >= pivot && j > first){
                j -= 1;
            }
                    
            if (i < j){
                temp = fmc.cell[i];
                fmc.cell[i] = fmc.cell[j];
                fmc.cell[j] = temp;
            }

        }

        //temp = fmc.cell[first];
        fmc.cell[first] = fmc.cell[j];
        fmc.cell[j] = pivot; //temp;

        quickSort(first, j-1);
        quickSort(j +1, last);
    }

    return 1;

}


//Added by Harald Heim, May 17, 2003
int quickFill(int x, int y, int z, int buffer, int radius, int xstart, int ystart, int xend, int yend)
{

    int i,j;
    int count=0;
    int n = radius*2 + 1;
    int yCoord=0,xCoord=0,value=0;

    if (n*n > N_CELLS) return 0; //not enough put/get cells;


    for (i = 0; i < n; i++) {
    for (j = 0; j < n; j++) {

        xCoord = x - radius + i;
        yCoord = y - radius + j;

        if (buffer<10){ //Needs to be done otherwise Memory Access Error -> why???
            if (xCoord  < 0) xCoord  = 0;
            else if (xCoord  >= fmc.X) xCoord  = fmc.X - 1;
            if (yCoord < 0) yCoord = 0;
            else if (yCoord >= fmc.Y) yCoord = fmc.Y - 1;
        }

        if (xCoord < xstart-fmc.needPadding) xCoord = xstart-fmc.needPadding;
        else if (xCoord >= xend+fmc.needPadding) xCoord = xend - 1+fmc.needPadding;
        if (yCoord < ystart-fmc.needPadding) yCoord = ystart-fmc.needPadding;
        else if (yCoord >= yend+fmc.needPadding) yCoord = yend - 1+fmc.needPadding;
            

        switch (buffer){
            case 0: //input buffer
                if (gStuff->imageMode < 10){
                    value = ((unsigned8 *)MyAdjustedInData)
                        [(xCoord -fmc.x_start)*fmc.srcColDelta + (yCoord - fmc.y_start)*fmc.srcRowDelta + z];
                } else {
                    value =  *(unsigned16 *)(((unsigned8 *)MyAdjustedInData) +
                                   (((xCoord-fmc.x_start)*fmc.srcColDelta + z) << 1) +
                                   (yCoord-fmc.y_start)*fmc.srcRowDelta);
                }
                break;
            case 1: //temp buffer 1
                value = fmc.tbuf[( yCoord *fmc.X + xCoord )*fmc.Z + z];
                break;
            case 2: //temp buffer 2
                value = fmc.t2buf[( yCoord *fmc.X + xCoord )*fmc.Z + z];
                break;
            case 3: //output buffer
                value = ((unsigned8 *)gStuff->outData)
                    [(xCoord -fmc.x_start)*fmc.dstColDelta + (yCoord -fmc.y_start)*fmc.dstRowDelta + z];
            case 10:
            case 11:
            case 12:
            case 13:
            case 14:
            case 15:
            case 16:
            case 17:
            case 18:
            case 19:
                if (ArrayBytes[buffer-10] == 1)
                    value = ((unsigned8 *)Array[buffer-10]) [ (( (yCoord+fmc.needPadding) *ArrayX[buffer-10] + (xCoord+fmc.needPadding) )*ArrayZ[buffer-10] + z) ];
                else if (ArrayBytes[buffer-10] == 2)
                    value =  *(unsigned16 *)(((unsigned8 *)Array[buffer-10]) + (( (yCoord+fmc.needPadding) *ArrayX[buffer-10] + (xCoord+fmc.needPadding) )*ArrayZ[buffer-10] + z)*ArrayBytes[buffer-10] );
                else if (ArrayBytes[buffer-10] == 4)
                    value =  *(unsigned32 *)(((unsigned8 *)Array[buffer-10]) + (( (yCoord+fmc.needPadding) *ArrayX[buffer-10] + (xCoord+fmc.needPadding) )*ArrayZ[buffer-10] + z)*ArrayBytes[buffer-10] );
        }//switch

        fmc.cell[count] = value;
        count++;
    }}


    return 1;

}



/*
 *  This Quickselect routine is based on the algorithm described in
 *  "Numerical recipes in C", Second Edition,
 *  Cambridge University Press, 1992, Section 8.5, ISBN 0-521-43108-5
 *  This code by Nicolas Devillard - 1998. Public domain.
 */ 

#define ELEM_SWAP(a,b) { register int t=(a);(a)=(b);(b)=t; }


int quickMedian(int low, int high)//(elem_type arr[], int n) 
{
    int median = (low + high) / 2;
    int middle, ll, hh;

    if (low<0 || low> N_CELLS-1 || high<0 || high> N_CELLS-1 || low > high || low == high) return 0;


    for (;;) {
        if (high <= low) /* One element only */
            return fmc.cell[median] ;

        if (high == low + 1) {  /* Two elements only */
            if (fmc.cell[low] > fmc.cell[high])
                ELEM_SWAP(fmc.cell[low], fmc.cell[high]) ;
            return fmc.cell[median] ;
        }

        /* Find median of low, middle and high items; swap into position low */
        middle = (low + high) / 2;
        if (fmc.cell[middle] > fmc.cell[high])    ELEM_SWAP(fmc.cell[middle], fmc.cell[high]) ;
        if (fmc.cell[low] > fmc.cell[high])       ELEM_SWAP(fmc.cell[low], fmc.cell[high]) ;
        if (fmc.cell[middle] > fmc.cell[low])     ELEM_SWAP(fmc.cell[middle], fmc.cell[low]) ;

        /* Swap low item (now in position middle) into position (low+1) */
        ELEM_SWAP(fmc.cell[middle], fmc.cell[low+1]) ;

        /* Nibble from each end towards middle, swapping items when stuck */
        ll = low + 1;
        hh = high;
        for (;;) {
            do ll++; while (fmc.cell[low] > fmc.cell[ll]) ;
            do hh--; while (fmc.cell[hh]  > fmc.cell[low]) ;

            if (hh < ll)
            break;

            ELEM_SWAP(fmc.cell[ll], fmc.cell[hh]) ;
        }

        /* Swap middle item (in position low) back into correct position */
        ELEM_SWAP(fmc.cell[low], fmc.cell[hh]) ;

        /* Re-set active partition */
        if (hh <= median)
            low = ll;
        if (hh >= median)
            high = hh - 1;
    }

}
#undef ELEM_SWAP



char * getCtlText(int n)
{
	static char dummystr[20];
	strcpy(dummystr, "Not Available"); //Return this string to avoid memory access violations

    if (n < 0 || n >= N_CTLS || !gParams->ctl[n].inuse) {        
			return (char *)(unsigned8 *)dummystr;
    }

	return (char *) (unsigned8 *) gParams->ctl[n].label;

} /*fm_getCtlText*/



int getCtlClass(int n)
{

	if (n == CTL_PREVIEW)

		return CC_PREVIEW;
	
	else if (n == CTL_PROGRESS)

		return CC_PROGRESSBAR;

	else if (n == CTL_ZOOM)

		return CC_ZOOM;

	else if ( n >= 0 && n < N_CTLS && gParams->ctl[n].inuse){
        
        return gParams->ctl[n].ctlclass;
    }

    return 0;  

} /*fm_getCtlClass*/


int getCtlView(int n){

	int styles;

	if (n < 0 || n >= N_CTLS || !gParams->ctl[n].inuse || gParams->ctl[n].ctlclass != CC_LISTVIEW) {
        return FALSE;
    }

	styles = (int)GetWindowLong(gParams->ctl[n].hCtl, GWL_EXSTYLE);

	if (styles & LVS_ICON) return LVS_ICON;
	else if (styles & LVS_REPORT) return LVS_REPORT;
	else if (styles &  LVS_SMALLICON ) return  LVS_SMALLICON ;
	else if (styles & LVS_LIST) return LVS_LIST;
	else return (int) ListView_GetView(gParams->ctl[n].hCtl); //Only work under XP and higher
	
	return -1; //Unknown
}


int getComboOpen(int n){

	if (n < 0 || n >= N_CTLS || !gParams->ctl[n].inuse || gParams->ctl[n].ctlclass != CC_COMBOBOX) {
        return FALSE;
    }

	return (int)SendMessage(gParams->ctl[n].hCtl,CB_GETDROPPEDSTATE,0,0);

}


int getCtlItemCount(int n)
{

	if (n < 0 || n >= N_CTLS || !gParams->ctl[n].inuse) {
        return FALSE;
    }

	if (gParams->ctl[n].ctlclass == CC_LISTBOX || gParams->ctl[n].ctlclass == CC_LISTBAR)
		return (int) SendMessage(gParams->ctl[n].hCtl, LB_GETCOUNT, 0, 0);
	else if (gParams->ctl[n].ctlclass == CC_COMBOBOX)
#if SKIN
		if (isSkinActive() && gParams->skinStyle>0)
			return (int) getCtlItemString(n,-1,NULL);
		else
#endif
			return (int) SendMessage(gParams->ctl[n].hCtl, CB_GETCOUNT, 0, 0);
	else if (gParams->ctl[n].ctlclass == CC_TAB)
		return TabCtrl_GetItemCount(gParams->ctl[n].hCtl);
	else if (gParams->ctl[n].ctlclass == CC_LISTVIEW)
		return ListView_GetItemCount(gParams->ctl[n].hCtl);
	else 
		return -1;

}

#ifndef LVM_ISITEMVISIBLE
	#define LVM_ISITEMVISIBLE (LVM_FIRST+182) 
	#define ListView_IsItemVisible(hwnd,index) (UINT)SNDMSG((hwnd),LVM_ISITEMVISIBLE,(WPARAM)(index),(LPARAM)0) 
#endif

int getCtlItemTop(int n)
{

	if (n < 0 || n >= N_CTLS || !gParams->ctl[n].inuse) {
        return FALSE;
    }

	if (gParams->ctl[n].ctlclass == CC_LISTBOX || gParams->ctl[n].ctlclass == CC_LISTBAR)
		return (int) SendMessage(gParams->ctl[n].hCtl, LB_GETTOPINDEX, 0, 0);
	else if (gParams->ctl[n].ctlclass == CC_COMBOBOX)
		return (int) SendMessage(gParams->ctl[n].hCtl, CB_GETTOPINDEX, 0, 0);
	else if (gParams->ctl[n].ctlclass == CC_LISTVIEW){
		int i;
		int itemcount = getCtlItemCount(n);
		for (i=0;i<itemcount;i++) if (ListView_IsItemVisible(gParams->ctl[n].hCtl,i)) break;
		if (i==itemcount-1) return 0; else return i;
		//return ListView_GetTopIndex(gParams->ctl[n].hCtl); // Does not work!!!
	} else 
		return -1;

}

int getCtlItemPos(int n, int item, int w)
{
	RECT rc;
	int val=0;

	if (n < 0 || n >= N_CTLS || !gParams->ctl[n].inuse || gParams->ctl[n].ctlclass != CC_TOOLBAR) {
        return FALSE;
    }

#if SKIN
	if (isSkinActive() && gParams->skinStyle>0){
		rc.left = item*skinGetToolItemWidth();
		rc.top = 0;
	} else
#endif
		SendMessage(gParams->ctl[n].hCtl, TB_GETRECT, item, (LPARAM)&rc);
	
	switch (w){
		case 0: val = PixelsToHDBUs(rc.left); break;
		case 1: val = PixelsToVDBUs(rc.top); break;
		case 2: val = PixelsToHDBUs(rc.right - rc.left); break;
		case 3: val = PixelsToVDBUs(rc.bottom - rc.top); break;
	}

	return val;

}

int setCtlItemTop(int n, int item)
{
	int retval;

	if (n < 0 || n >= N_CTLS || !gParams->ctl[n].inuse) {
        return FALSE;
    }

	if (gParams->ctl[n].ctlclass == CC_LISTBOX || gParams->ctl[n].ctlclass == CC_LISTBAR)
		retval = (int)SendMessage(gParams->ctl[n].hCtl, LB_SETTOPINDEX, item, 0);
	else if (gParams->ctl[n].ctlclass == CC_COMBOBOX)
		retval = (int)SendMessage(gParams->ctl[n].hCtl, CB_SETTOPINDEX, item, 0);
	else if (gParams->ctl[n].ctlclass == CC_LISTVIEW){
		return (int)ListView_EnsureVisible(gParams->ctl[n].hCtl,item,FALSE);
	/*
		RECT prc ={0,0,0,0};
		POINT pItemPosition;
		if (ListView_GetViewRect(gParams->ctl[n].hCtl,&prc)){
			if (ListView_GetItemPosition(gParams->ctl[n].hCtl,item,&pItemPosition)){
				if (ListView_Scroll(gParams->ctl[n].hCtl,0, pItemPosition.y + prc.top)){
					return true;
				}
			} else { //Less good alternative
				ListView_EnsureVisible(gParams->ctl[n].hCtl,item,FALSE);
			}
		}
	*/
		return false;
	} else
		return false;

	if (retval == LB_ERR) //|| retval == CB_ERR
		return false;
	else
		return true;

}

int getCtlItemText(int n, int item, char * str)
{
	int retval;

	if (n < 0 || n >= N_CTLS || !gParams->ctl[n].inuse) {
        return FALSE;
    }

	if (gParams->ctl[n].ctlclass == CC_LISTBOX || gParams->ctl[n].ctlclass == CC_LISTBAR)
		retval = (int) SendMessage(gParams->ctl[n].hCtl, LB_GETTEXT, item, (LPARAM)str);
	
	else if (gParams->ctl[n].ctlclass == CC_LISTVIEW){
		ListView_GetItemText(gParams->ctl[n].hCtl, item, 0, (LPTSTR)str, 255);
		retval = LB_OKAY;

	} else if (gParams->ctl[n].ctlclass == CC_COMBOBOX)
#if SKIN
		if (isSkinActive() && gParams->skinStyle>0)
			retval = (int) getCtlItemString(n,item,str);
		else
#endif
			retval = (int) SendMessage(gParams->ctl[n].hCtl, CB_GETLBTEXT, item, (LPARAM)str);

	else if (gParams->ctl[n].ctlclass == CC_TAB){
		TCITEM tabItem; 
		
		tabItem.mask = TCIF_TEXT;
		tabItem.pszText = (char *)str;
		tabItem.cchTextMax = 256;

		if (TabCtrl_GetItem(gParams->ctl[n].hCtl, item, &tabItem)){
			retval = LB_OKAY;
		} else
			retval = LB_ERR;
	}

	if (retval == LB_ERR) //|| retval == CB_ERR
		return false;
	else
		return true;

}
 
int setCtlItemText(int n, int item, char * str)
{
	int retval = -1;
	
	if (n < 0 || n >= N_CTLS || !gParams->ctl[n].inuse) {
        return FALSE;
    }
	
    if (gParams->ctl[n].ctlclass == CC_LISTBOX || gParams->ctl[n].ctlclass == CC_LISTBAR)
        retval = (int) SendMessage(gParams->ctl[n].hCtl, LB_INSERTSTRING, item, (LPARAM)str);
    
	else if (gParams->ctl[n].ctlclass == CC_LISTVIEW){

		LVITEM lvi;

		 // Initialize LVITEM members that are common to all items.
		lvi.mask   = LVIF_TEXT; // | LVIF_IMAGE|LVIF_STATE ;
		lvi.iItem  = item;
		lvi.iSubItem  = 0;
		lvi.pszText  = (LPTSTR)str; //NULL; //LPSTR_TEXTCALLBACK; // Sends an LVN_GETDISPINFO message.
		lvi.cchTextMax = lstrlen(str);
		
		retval = (int) ListView_InsertItem (gParams->ctl[n].hCtl, &lvi);
		
	} else if (gParams->ctl[n].ctlclass == CC_COMBOBOX)
		retval = (int) SendMessage(gParams->ctl[n].hCtl, CB_INSERTSTRING, item, (LPARAM)str);
	
	else if (gParams->ctl[n].ctlclass == CC_TAB){

		int tabCount = TabCtrl_GetItemCount(gParams->ctl[n].hCtl);
		TCITEM tabItem; 
		
		tabItem.mask = TCIF_TEXT | TCIF_IMAGE; 
		tabItem.iImage = -1; 
		tabItem.pszText = (char *)str; 
		
		if (item<0) item = tabCount+1;
		
		if (item>tabCount)
			retval = (int)TabCtrl_InsertItem(gParams->ctl[n].hCtl, item, &tabItem);
		else
			retval = (int)TabCtrl_SetItem(gParams->ctl[n].hCtl, item, &tabItem);


	}
	
	//Write to fmc label
	if (gParams->ctl[n].ctlclass == CC_COMBOBOX){
		if (strlen(gParams->ctl[n].label)+strlen(str) < sizeof(gParams->ctl[n].label)-1) { //1022
			strcat(gParams->ctl[n].label,str);
			strcat(gParams->ctl[n].label,"\n");
		}
	}

	return retval;

}

int setCtlItemState(int n, int item, int state){

	LVITEM lvi;

	if (n < 0 || n >= N_CTLS || !gParams->ctl[n].inuse || gParams->ctl[n].ctlclass != CC_LISTVIEW) {
        return FALSE;
    }

	lvi.mask   = LVIF_STATE;
	lvi.iItem  = item;
	lvi.iSubItem = 0;
	if (state==0){
		lvi.state = 0;
		lvi.stateMask = LVIS_CUT|LVIS_DROPHILITED|LVIS_FOCUSED|LVIS_SELECTED;
	} else {
		lvi.state = state;
		lvi.stateMask = state;
	}
	
	return (int) ListView_SetItem (gParams->ctl[n].hCtl, &lvi);
}

int deleteCtlItem(int n, int item)
{

	if (n < 0 || n >= N_CTLS || !gParams->ctl[n].inuse) {
        return FALSE;
    }

	if (gParams->ctl[n].ctlclass == CC_LISTBOX || gParams->ctl[n].ctlclass == CC_LISTBAR)
		return (int) SendMessage(gParams->ctl[n].hCtl, LB_DELETESTRING, item, 0);
	
	else if (gParams->ctl[n].ctlclass == CC_COMBOBOX)
		return (int) SendMessage(gParams->ctl[n].hCtl, CB_DELETESTRING, item, 0);
	
	else if (gParams->ctl[n].ctlclass == CC_TAB)
		return (int)TabCtrl_DeleteItem(gParams->ctl[n].hCtl, item);

	return false;

}

int deleteCtlItems(int n)
{
	if (n < 0 || n >= N_CTLS || !gParams->ctl[n].inuse) {
        return FALSE;
    }
	
	if (gParams->ctl[n].ctlclass == CC_TAB){
		return (int)TabCtrl_DeleteAllItems (gParams->ctl[n].hCtl);

	} else if (gParams->ctl[n].ctlclass == CC_LISTBOX || gParams->ctl[n].ctlclass == CC_LISTBAR || gParams->ctl[n].ctlclass == CC_COMBOBOX) {
		return setCtlText(n,"");
	}
	
	return false;
}


#define LVGS_COLLAPSIBLE 0x00000008
#define LVGS_NOHEADER 0x00000004

int setCtlGroupText(int n, int group, char * str) //int startitem,
{
	LVGROUP grp;
	wchar_t wstr[512];
	
	//At least Win XP required
	//if (getWindowsVersion()<9) return FALSE; 

	if (n < 0 || n >= N_CTLS || !gParams->ctl[n].inuse || gParams->ctl[n].ctlclass != CC_LISTVIEW) {
        return FALSE;
    }

	mbstowcs(wstr,str,strlen(str)+1);
   
	#define LVGF_TITLEIMAGE 0x00001000 
	#define LVGF_EXTENDEDIMAGE 0x00002000 
	
	//ZeroMemory(&grp,sizeof(LVGROUP));
    grp.cbSize    = sizeof(LVGROUP);
    grp.mask      = LVGF_HEADER | LVGF_GROUPID  | LVGF_STATE;// | LVGF_TITLEIMAGE;// | LVGF_ALIGN;
    grp.pszHeader = (LPWSTR)wstr;
	grp.iGroupId  = group;
	grp.stateMask = 0;//LVGS_COLLAPSIBLE; //
	grp.state = 0;//LVGS_COLLAPSIBLE; //
	//grp.uAlign = LVGA_HEADER_CENTER;
	//grp.iTitleImage = 0;

    return (int) ListView_InsertGroup(gParams->ctl[n].hCtl, -1, &grp); //startitem
}

int setCtlGroupState(int n, int group, int state){

	//int flags=0;
	LVGROUP grp;

	if (n < 0 || n >= N_CTLS || !gParams->ctl[n].inuse || gParams->ctl[n].ctlclass != CC_LISTVIEW) {
        return FALSE;
    }

	//#define LVGS_FOCUSED 0x00000010
	//#define LVGS_SELECTED 0x00000020
	/*if (state==-1) flags |= LVGS_NORMAL ;
	else if (state==0) flags |= LVGS_HIDDEN; 
	else if (state==1) flags |= LVGS_COLLAPSED;
	else if (state==2) flags |= LVGS_FOCUSED;
	else if (state==3) flags |= LVGS_SELECTED;*/
	
	grp.cbSize    = sizeof(LVGROUP);
    grp.mask      = LVGF_STATE;
	grp.iGroupId  = group;
	grp.state = state;
	grp.stateMask = state;
    
	return (int)ListView_SetGroupInfo(gParams->ctl[n].hCtl, group, &grp);
}

int getCtlGroupState(int n, int group)
{
	LVGROUP grp;

	if (n < 0 || n >= N_CTLS || !gParams->ctl[n].inuse || gParams->ctl[n].ctlclass != CC_LISTVIEW) {
        return FALSE;
    }

	#define LVGS_SELECTED 0x00000020

	grp.cbSize    = sizeof(LVGROUP);
    grp.mask      = LVGF_STATE;
	grp.iGroupId  = group;
	grp.stateMask = LVGS_NORMAL | LVGS_COLLAPSED | LVGS_SELECTED | LVGS_HIDDEN;
	grp.state = LVGS_NORMAL | LVGS_COLLAPSED | LVGS_SELECTED | LVGS_HIDDEN;
	
	ListView_GetGroupInfo(gParams->ctl[n].hCtl, group, &grp);

	return (int) grp.state;
}

int getCtlGroupCount(int n)
{
	if (n < 0 || n >= N_CTLS || !gParams->ctl[n].inuse || gParams->ctl[n].ctlclass != CC_LISTVIEW) {
        return FALSE;
    }

	#define LVM_GETGROUPCOUNT         (LVM_FIRST + 152)
	#define ListView_GetGroupCount(hwnd) SNDMSG((hwnd), LVM_GETGROUPCOUNT, (WPARAM)0, (LPARAM)0)

	return (int)ListView_GetGroupCount(gParams->ctl[n].hCtl);
}

int getCtlItemGroup(int n, int item)
{
	LVITEM lvi;

	if (n < 0 || n >= N_CTLS || !gParams->ctl[n].inuse || gParams->ctl[n].ctlclass != CC_LISTVIEW) {
        return FALSE;
    }

	 // Initialize LVITEM members that are common to all items.
	lvi.mask   = LVIF_GROUPID;
	lvi.iItem  = item;
	lvi.iSubItem = 0;
	
	ListView_GetItem (gParams->ctl[n].hCtl, &lvi);

	return (int) lvi.iGroupId;
}

int setCtlItemGroup(int n, int item, int group)
{
	LVITEM lvi;

	if (n < 0 || n >= N_CTLS || !gParams->ctl[n].inuse || gParams->ctl[n].ctlclass != CC_LISTVIEW) {
        return FALSE;
    }

	 // Initialize LVITEM members that are common to all items.
	lvi.mask   = LVIF_GROUPID;
	lvi.iItem  = item;
	lvi.iSubItem = 0;
	lvi.iGroupId = group;
	
	return (int) ListView_SetItem (gParams->ctl[n].hCtl, &lvi);
}

int setCtlImageList(int n, int i){

	if (n < 0 || n >= N_CTLS || !gParams->ctl[n].inuse || gParams->ctl[n].ctlclass != CC_LISTVIEW || ImageList[i]==NULL) {
        return FALSE;
    }

	ListView_SetImageList(gParams->ctl[n].hCtl,ImageList[i],LVSIL_NORMAL);

	return true;

}

int setCtlItemImage(int n, int item, int i){
	
	LVITEM lvi;

	if (n < 0 || n >= N_CTLS || !gParams->ctl[n].inuse || gParams->ctl[n].ctlclass != CC_LISTVIEW) {
        return FALSE;
    }

	lvi.mask   = LVIF_IMAGE;
	lvi.iItem  = item;
	lvi.iSubItem = 0;
	lvi.iImage = i;
	
	return (int) ListView_SetItem (gParams->ctl[n].hCtl, &lvi);

}

int getCtlItemImage(int n, int item){
	
	LVITEM lvi;

	if (n < 0 || n >= N_CTLS || !gParams->ctl[n].inuse || gParams->ctl[n].ctlclass != CC_LISTVIEW) {
        return FALSE;
    }

	lvi.mask   = LVIF_IMAGE;
	lvi.iItem  = item;
	lvi.iSubItem = 0;
	
	ListView_GetItem (gParams->ctl[n].hCtl, &lvi);
	
	return (int) lvi.iImage;

}

int labelHeight = 13;

int setCtlItemSpacing(int n, int width, int height, int applyLabelHeight){

	if (n < 0 || n >= N_CTLS || !gParams->ctl[n].inuse || !(gParams->ctl[n].ctlclass==CC_LISTVIEW || gParams->ctl[n].ctlclass==CC_TOOLBAR) ) {
        return FALSE;
    }

	if (gParams->ctl[n].ctlclass == CC_LISTVIEW){

		if (width==-1 || height==-1){
			int spacing = ListView_GetItemSpacing (gParams->ctl[n].hCtl,FALSE);
			if (width==-1) width = LOWORD(spacing);
			if (height==-1) height = HIWORD(spacing);
		}

		//Add or remove Label Height
		if (applyLabelHeight != 0){
			RECT prc;
			ListView_GetItemRect(gParams->ctl[n].hCtl,0,&prc,LVIR_LABEL);
			if (prc.bottom - prc.top > 0) labelHeight = prc.bottom - prc.top; //in case collapsed don't use
			if (applyLabelHeight>0)
				height += labelHeight;
			else 
				height -= labelHeight;
		}

		return ListView_SetIconSpacing(gParams->ctl[n].hCtl,width,height);

	} else {
		
		return (int)SendMessage(gParams->ctl[n].hCtl, TB_SETBUTTONSIZE, 0, MAKELPARAM(HDBUsToPixels(width),VDBUsToPixels(height)) );

	}
}

int setCtlExtStyle(int n, int exstyle){

	if (n < 0 || n >= N_CTLS || !gParams->ctl[n].inuse || gParams->ctl[n].ctlclass != CC_LISTVIEW) {
        return FALSE;
    }

	/*if (exstyle = LVS_EX_HIDELABELS){
		if (gParams->ctl[n].style & LVS_REPORT) return false;
		setCtlItemSpacing(n,-1,-1,-1); //Remove label height
	}*/

	return ListView_SetExtendedListViewStyle(gParams->ctl[n].hCtl,exstyle);
}

int clearCtlExtStyle(int n, int exstyle){

	int flags;

	if (n < 0 || n >= N_CTLS || !gParams->ctl[n].inuse || gParams->ctl[n].ctlclass != CC_LISTVIEW) {
        return FALSE;
    }

	flags = ListView_GetExtendedListViewStyle(gParams->ctl[n].hCtl);
	flags &= ~exstyle;

	/*if (exstyle = LVS_EX_HIDELABELS){
		setCtlItemSpacing(n,-1,-1,1); //Add label height
	}*/

	return ListView_SetExtendedListViewStyle(gParams->ctl[n].hCtl,flags);
}


int setCtlGroupView(int n, int on)
{
	if (n < 0 || n >= N_CTLS || !gParams->ctl[n].inuse || gParams->ctl[n].ctlclass != CC_LISTVIEW) {
        return FALSE;
    }

	return (int) ListView_EnableGroupView(gParams->ctl[n].hCtl, (BOOL)on);
}





int fillDir(int n, int attr, int wildcard)
{
	if (n < 0 || n >= N_CTLS || !gParams->ctl[n].inuse) {
        return FALSE;
    }

	if (gParams->ctl[n].ctlclass == CC_LISTBOX || gParams->ctl[n].ctlclass == CC_LISTBAR)
		return (int) SendMessage(gParams->ctl[n].hCtl, LB_DIR, attr, wildcard);
	else if (gParams->ctl[n].ctlclass == CC_COMBOBOX)
		return (int) SendMessage(gParams->ctl[n].hCtl, CB_DIR, attr, wildcard);
	else 
		return -1;

}



INT_PTR getResAddress(INT_PTR restype, INT_PTR resname){

	HRSRC hRes;         // handle/ptr. to res. info.
    int cb;             // Size of resource (bytes)
    HRSRC hResLoad;     // handle to loaded resource  
    char *hResPointer;  // pointer to resource data 
    
    
    // Locate the resource. 
    //hRes = FindResource(hDllInstance,MAKEINTRESOURCE(resname),(LPCTSTR)restype);
	hRes = FindResource((HMODULE)hDllInstance,(LPCTSTR)resname,(LPCTSTR)restype);
    if (hRes == NULL) { 
        return FALSE;
    } 

    // Check for size of resource
    if ((cb=SizeofResource((HMODULE)hDllInstance, hRes)) <= 0)
		return FALSE;

	// Load the resource into global memory.
    hResLoad = (HRSRC)LoadResource((HMODULE)hDllInstance, hRes);
    if (hResLoad == NULL) { 
        return FALSE;
    } 

    // Lock the resource into global memory.
    hResPointer = (char *)LockResource(hResLoad);
    if (hResPointer == NULL) {
        return FALSE;
    } else 
		return (INT_PTR)hResPointer;

}

int getResSize(char* restype, char* resname){

	HRSRC hRes;         // handle/ptr. to res. info.
    int cb;             // Size of resource (bytes)
    
    // Locate the resource.
    //hRes = FindResource(hDllInstance,MAKEINTRESOURCE(resname),(LPCTSTR)restype);
	hRes = FindResource((HMODULE)hDllInstance,(LPCTSTR)resname,(LPCTSTR)restype);
    if (hRes == NULL) { 
        return FALSE;
    } 

	// Check for size of resource and allocate array
    if ((cb=SizeofResource((HMODULE)hDllInstance, hRes)) > 0)
		return (int)cb;
	else 
		return 0;

}

int copyResToArray (char* restype, char* resname, int arraynr)
{
    HRSRC hRes;         // handle/ptr. to res. info.
    int cb;             // Size of resource (bytes)
    HRSRC hResLoad;     // handle to loaded resource  
    char *hResPointer;  // pointer to resource data 
    
    
    // Locate the resource. 
    //hRes = FindResource(hDllInstance,(LPCTSTR)resname,(LPCTSTR)restype);
    //hRes = FindResource(hDllInstance,MAKEINTRESOURCE(resname),(LPCTSTR)restype);
	hRes = FindResource((HMODULE)hDllInstance,(LPCTSTR)resname,(LPCTSTR)restype);
    if (hRes == NULL) { 
        
        //Instead try to find a file with the same name as resname
        FILE *file;
        int cb;
        
        file = fopen( (LPCTSTR)resname, "rb" );
        if (file==NULL) return FALSE;

        // Determine file length...
        fseek(file, 0, SEEK_END);
        cb = ftell(file);
        rewind(file);

        if (allocArray (arraynr,cb,0,0,1))
            fread((void *)getArrayAddress(arraynr), 1, cb, file);

        fclose(file);
        return true;

    } 

    // Check for size of resource and allocate array
    if ((cb=SizeofResource((HMODULE)hDllInstance, hRes)) > 0)
    {
        allocArray (arraynr,cb,0,0,1);
    } else 
        return FALSE;


    // Load the resource into global memory.
    hResLoad = (HRSRC)LoadResource((HMODULE)hDllInstance, hRes);
    if (hResLoad == NULL) { 
        return FALSE;
    } 

    // Lock the resource into global memory.
    hResPointer = (char *)LockResource(hResLoad);
    if (hResPointer == NULL) {
        return FALSE;
    }

    //Copy the ressource data into the array
    memcpy (Array[arraynr],hResPointer,cb);


    return TRUE;
}


int copyResToArrayEx (char* restype, char* resname, int arraynr, int X, int Y, int Z)
{
    HRSRC hRes;         // handle/ptr. to res. info.
    int cb;             // Size of resource (bytes)
    HRSRC hResLoad;     // handle to loaded resource  
    char *hResPointer;  // pointer to resource data 
    
    
    // Locate the resource. 
    //hRes = FindResource(hDllInstance,(LPCTSTR)resname,(LPCTSTR)restype);
    //hRes = FindResource(hDllInstance,MAKEINTRESOURCE(resname),(LPCTSTR)restype);
	hRes = FindResource((HMODULE)hDllInstance,(LPCTSTR)resname,(LPCTSTR)restype);
    if (hRes == NULL) { 
        
        //Instead try to find a file with the same name as resname
        FILE *file;
        int cb;
        
        file = fopen( (LPCTSTR)resname, "rb" );
        if (file==NULL) return FALSE;

        // Determine file length...
        fseek(file, 0, SEEK_END);
        cb = ftell(file);
        rewind(file);

		
		if (allocArray (arraynr,X,Y,Z,1))
			fread((void *)getArrayAddress(arraynr), 1, min(X*Y*Z,cb), file);
		
        fclose(file);

		if (X*Y*Z==cb)
			return true;
		else
			return false;
    } 

    // Check for size of resource and allocate array
    if ((cb=SizeofResource((HMODULE)hDllInstance, hRes)) > 0){
        allocArray (arraynr,X,Y,Z,1);
    } else 
        return FALSE;


    // Load the resource into global memory.
    hResLoad = (HRSRC)LoadResource((HMODULE)hDllInstance, hRes);
    if (hResLoad == NULL) { 
        return FALSE;
    } 

    // Lock the resource into global memory.
    hResPointer = (char *)LockResource(hResLoad);
    if (hResPointer == NULL) {
        return FALSE;
    }

    //Copy the ressource data into the array
    memcpy (Array[arraynr],hResPointer,min(X*Y*Z,cb));


    return TRUE;
}


int getScreenSize(int s, int fAbs){

	int xDiff=0,yDiff=0;
	RECT workArea;
	
	SystemParametersInfo(SPI_GETWORKAREA, 0, &workArea, 0); //Work area

	if (!fAbs){
		RECT rcClient, rcWind; 
		//Get difference between window and client area
        GetClientRect(fmc.hDlg, &rcClient); 
        GetWindowRect(fmc.hDlg, &rcWind); 
        xDiff = (rcWind.right - rcWind.left) - (rcClient.right-rcClient.left); 
        yDiff = (rcWind.bottom - rcWind.top) - (rcClient.bottom-rcClient.top); 
	}
	
	//Center on screen
	if (s==0) 
		return workArea.right - workArea.left - xDiff; 
	else 
		return workArea.bottom - workArea.top - yDiff;
}

//Maximized client area
int getDialogMaxSize(int s){

	RECT workArea;
	
	SystemParametersInfo(SPI_GETWORKAREA, 0, &workArea, 0); //Work area
	
	if (s==0) 
		return PixelsToHDBUs(workArea.right - workArea.left);
	else {
		return PixelsToVDBUs(workArea.bottom - workArea.top - GetSystemMetrics(SM_CYCAPTION)); //Subtract title bar height
	}
}



int setDialogShowState(int state)
{
    return ShowWindow (fmc.hDlg, state);
}

int setDialogShowStateEx(int state, int width, int height){

	RECT rc;
	RECT workArea;
	WINDOWPLACEMENT wndpl;

	width = HDBUsToPixels(width);
	height = VDBUsToPixels(height);

	SystemParametersInfo(SPI_GETWORKAREA, 0, &workArea, 0); //Work area
	GetWindowRect(fmc.hDlg, &rc);

	//Center on screen
	rc.left = ((workArea.right - workArea.left) - width)/2; 
	rc.top = ((workArea.bottom - workArea.top) - height)/2;
	rc.right = rc.left + width;
	rc.bottom = rc.top + height;

	wndpl.length = sizeof(WINDOWPLACEMENT);
	wndpl.flags = 0;
	wndpl.showCmd = SW_MAXIMIZE;
	wndpl.rcNormalPosition = rc;

	if (state==SW_MAXIMIZE) fMaximized = true; //Produce FME_EXITSIZE event
	if (state==SW_MINIMIZE) fMinimized = true; //Produce FME_EXITSIZE event

	return SetWindowPlacement(fmc.hDlg,&wndpl);
};

int getDialogRestoreWidth(){
	WINDOWPLACEMENT wndpl;
	wndpl.length = sizeof(WINDOWPLACEMENT);
	GetWindowPlacement(fmc.hDlg, &wndpl);
	return PixelsToHDBUs((int)(wndpl.rcNormalPosition.right-wndpl.rcNormalPosition.left));
};

int getDialogRestoreHeight(){
	WINDOWPLACEMENT wndpl;
	wndpl.length = sizeof(WINDOWPLACEMENT);
	GetWindowPlacement(fmc.hDlg, &wndpl);
	return PixelsToVDBUs((int)(wndpl.rcNormalPosition.bottom-wndpl.rcNormalPosition.top));
};


//SW_HIDE             0
//SW_NORMAL           1
//#define SW_SHOWMINIMIZED    2
//#define SW_MAXIMIZE         3
int getDialogWindowState(){
	WINDOWPLACEMENT wndpl;
	wndpl.length = sizeof(WINDOWPLACEMENT);
	GetWindowPlacement(fmc.hDlg, &wndpl);
	return (int)wndpl.showCmd;
};



int setDialogSizeMax(void)
{

    RECT rect;

    if (SystemParametersInfo(SPI_GETWORKAREA,0,&rect,0)){

        RECT rcClient, rcWind; 
        int xDiff,yDiff;

        //Get difference between window and client area
        GetClientRect(fmc.hDlg, &rcClient); 
        GetWindowRect(fmc.hDlg, &rcWind); 
        xDiff = (rcWind.right - rcWind.left) - rcClient.right; 
        yDiff = (rcWind.bottom - rcWind.top) - rcClient.bottom; 

        //Does not work correctly for visual styles:
        //xDiff = 2*GetSystemMetrics(SM_CXDLGFRAME);
        //yDiff = 2*GetSystemMetrics(SM_CYDLGFRAME);
        
        setDialogPos ( true, 
            PixelsToHDBUs(rect.left), 
            PixelsToVDBUs(rect.top),
            PixelsToHDBUs(rect.right - rect.left - xDiff+1),
            PixelsToVDBUs(rect.bottom - rect.top - yDiff+1)
        );

    } else { //old unprecise method

        //int ScreenWidth = getDisplaySettings(1); //dm.dmPelsWidth
	    //int ScreenHeight = getDisplaySettings(2); //dm.dmPelsHeight

        DEVMODE dm;
        dm.dmSize = sizeof(DEVMODE);
        dm.dmDriverExtra = 0;

        if (EnumDisplaySettings(NULL,ENUM_CURRENT_SETTINGS,&dm))
        setDialogPos ( true, -1, -1,PixelsToHDBUs(dm.dmPelsWidth-4),PixelsToVDBUs(dm.dmPelsHeight-4) ); //-4
    }

    return true;
}


int setDialogSizeGrip(int state)
{
    
    if (sizeGrip != state){
        sizeGrip = state;
        if (sizeGrip == 0) refreshWindow(); //Make sure the grip vanishes
    }

    return true;

}

//Scale or do not scale preview and frame together
void unlockPreviewFrame(int state)
{
	unlockedPreviewFrame = state;
}


int PixelsToHDBUs(int h) 
{
    int result=0;
    int buX=0;
    SIZE  size;
    HDC hdc = GetDC(fmc.hDlg);
    HFONT hDlgFont = (HFONT) SendMessage(fmc.hDlg, WM_GETFONT, 0, 0L);
    HFONT hFontOld = (HFONT)SelectObject(hdc,hDlgFont);

	if (h==0) return 0;

    if (GetTextExtentPoint32(hdc,"ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrst"
                "uvwxyz",52,&size)){
        
        buX = (size.cx/26+1)/2;

        //Convert to DBU
        if (buX != 0) result = (h * 4) / buX;
    }

    SelectObject(hdc,hFontOld);
    ReleaseDC(fmc.hDlg, hdc);

    return result;
}


int PixelsToVDBUs(int v) 
{
    int result=0;
    int buY=0;
    TEXTMETRIC tm;
    HDC hdc = GetDC(fmc.hDlg);
    HFONT hDlgFont = (HFONT) SendMessage(fmc.hDlg, WM_GETFONT, 0, 0L);
    HFONT hFontOld = (HFONT)SelectObject(hdc,hDlgFont);

	if (v==0) return 0;

    if (GetTextMetrics(hdc,&tm)){
        
        buY = (WORD)tm.tmHeight;

        //Convert to DBU
        if (buY != 0) result =  (v * 8) / buY;          
    }

    SelectObject(hdc,hFontOld);
    ReleaseDC(fmc.hDlg, hdc);

    return result;
}


int setCtlMate(int n, int s){

    if (n < 0 || n >= N_CTLS) return FALSE;
    if (s < 0 || s >= N_CTLS) return FALSE;

    gParams->ctl[n].mateControl = s;

    return true;
}

int getCtlMate(int n){

    if (n < 0 || n >= N_CTLS) return FALSE;

    return gParams->ctl[n].mateControl;
}



int setCtlTab(int n, int t, int s)
{
	if (n < 0 || n >= N_CTLS || !gParams->ctl[n].inuse) {
        return FALSE; //not a valid control
    }
    
    if (t == -1) {
        // disassociate this control from any Tab control
	    gParams->ctl[n].tabControl = t;
	    gParams->ctl[n].tabSheet = s;
        //setCtlColor(n, -1); //set background color back to transparent???
        sub_enableCtl(n); //automatically enable or disable the control
#if PLUGINGALAXY
		if (gParams->ctl[n].ctlclass == CC_STATICTEXT || gParams->ctl[n].ctlclass == CC_CHECKBOX)
			setCtlColor(n,-1);
#endif

	#if SKIN
		if (isSkinActive() && gParams->skinStyle>1){
			if (gParams->ctl[n].ctlclass == CC_STATICTEXT){
				setCtlColor(n, -1);
			}
		}
	#endif

        return true;
    }

    if (t < 0 || t >= N_CTLS || !gParams->ctl[t].inuse || (gParams->ctl[t].ctlclass != CC_TAB && gParams->ctl[t].ctlclass != CC_LISTBAR)) {
        return FALSE; // t is not a valid Tab control
    }
	
	gParams->ctl[n].tabControl = t;
	gParams->ctl[n].tabSheet = s;

#if SKIN
	if (!(isSkinActive() && gParams->skinStyle>0))
#endif	
	{

		//Change background color to tab color if theme is active
		if (gParams->ctl[n].ctlclass == CC_CHECKBOX || gParams->ctl[n].ctlclass == CC_RADIOBUTTON || 
		   (gParams->ctl[n].ctlclass == CC_OWNERDRAW && (gParams->ctl[n].properties & CTP_DRAWITEM)) ) 
		   {
			   if ( getAppTheme() && (gParams->ctl[t].theme==1  || (gParams->ctl[t].theme==-1 && gParams->gDialogTheme==1)) ){
				   setCtlColor(n,getCtlColor(t));
			   }
		}
		
		//Make Groupbox text back like tab sheet
		if (gParams->ctl[n].ctlclass == CC_GROUPBOX){
			//Info ("%d: %d,%d,%d",n,Rval(getCtlColor(gParams->ctl[n].tabControl)),Gval(getCtlColor(gParams->ctl[n].tabControl)),Bval(getCtlColor(gParams->ctl[n].tabControl)) );
			//if (getAppTheme() && (gParams->ctl[t].theme==1  || (gParams->ctl[t].theme==-1 && gParams->gDialogTheme==1)) ) 
			if (gParams->ctl[t].ctlclass == CC_LISTBAR)
				setCtlColor(n, getCtlColor(gParams->ctl[t].tabControl));
			else
				setCtlColor(n, getCtlColor(t)); //gParams->ctl[n].tabControl
		} else if (gParams->ctl[n].ctlclass == CC_SLIDER ||gParams->ctl[n].ctlclass == CC_TRACKBAR){
			setCtlColor(n, getCtlColor(t));
		}
	}

#if SKIN
	if (isSkinActive() && gParams->skinStyle>1){
		if (gParams->ctl[n].ctlclass == CC_STATICTEXT){
			setCtlColor(n, skinGetColor());
		}
	}
#endif

	//Info ("%d,%d\n%d, %d",t,s,gParams->ctl[t].val,gParams->ctl[t].enabled);

	//Automatically enable or disable control
	sub_enableCtl (n);	//MWVDL 2008-06-17

	return true;

}


int shiftTabSheet(int t, int shiftval){

    int i;

    for (i=0;i<N_CTLS;i++){
        if (gParams->ctl[i].tabControl == t){
            gParams->ctl[i].tabSheet += shiftval;
        }
    }

    return true;
}


int getCtlTab(int n, int t){

    if (n < 0 || n >= N_CTLS) return -1;

    if (t==0) 
        return gParams->ctl[n].tabControl;
    else 
        return gParams->ctl[n].tabSheet;

}

int setCtlDefTab(int n, int t, int s)
{
	gParams->ctl[n].defTabControl = t;
    gParams->ctl[n].defTabSheet = s;
	return true;
}

int getCtlDefTab(int n, int m)
{
	if (m==0)return gParams->ctl[n].defTabControl;
    else return gParams->ctl[n].defTabSheet;
}


int setCtlAnchor(int n, int flags)
{
	int dialogWidth,dialogHeight;
	RECT rcParent;
	
	if ( (n < 0 || n >= N_CTLS || !gParams->ctl[n].inuse)) { //&& n != CTL_PREVIEW && n != CTL_PROGRESS && n != CTL_ZOOM
        return FALSE;
    }

	if (flags == -1) //Use already assigned anchor
        flags = gParams->ctl[n].anchor;
    else
        gParams->ctl[n].anchor = flags;
    
    if (flags==0) return true;


	GetClientRect(fmc.hDlg, &rcParent);
	dialogWidth = PixelsToHDBUs(rcParent.right);
	dialogHeight = PixelsToVDBUs(rcParent.bottom);


/*
	if (n == CTL_ZOOM){ //n == CTL_PREVIEW || n == CTL_PROGRESS || 

        RECT rcCtl;
    
        //if (n == CTL_PREVIEW)
		//	GetWindowRect( GetDlgItem(fmc.hDlg, 101), &rcCtl );
		//else if (n == CTL_PROGRESS)
		//	GetWindowRect( GetDlgItem(fmc.hDlg, IDC_PROGRESS1), &rcCtl );
		//else if (n == CTL_ZOOM)
			GetWindowRect( GetDlgItem(fmc.hDlg, IDC_BUTTON2), &rcCtl );
		
		ScreenToClient (fmc.hDlg, (LPPOINT)&rcCtl);
		ScreenToClient (fmc.hDlg, (LPPOINT)&(rcCtl.right));

		if (flags & 16) //ANCHOR_HCENTER
            gParams->ctl[n].distWidth = PixelsToHDBUs(rcCtl.left) - dialogWidth/2;
        else
            gParams->ctl[n].distWidth = dialogWidth - PixelsToHDBUs(rcCtl.right);

		if (flags & 32) //ANCHOR_VCENTER
		    gParams->ctl[n].distHeight = PixelsToVDBUs(rcCtl.top) - dialogHeight/2;
        else
            gParams->ctl[n].distHeight = dialogHeight - PixelsToVDBUs(rcCtl.bottom);

		gParams->ctl[n].width = PixelsToHDBUs(rcCtl.right - rcCtl.left);
		gParams->ctl[n].height = PixelsToVDBUs(rcCtl.bottom - rcCtl.top);
		gParams->ctl[n].xPos = PixelsToHDBUs(rcCtl.left);
		gParams->ctl[n].yPos = PixelsToVDBUs(rcCtl.top);

	} else {*/

		if (flags & ANCHOR_HCENTER)
            gParams->ctl[n].distWidth = gParams->ctl[n].xPos - dialogWidth/2;
        else
            gParams->ctl[n].distWidth = dialogWidth - (gParams->ctl[n].xPos + gParams->ctl[n].width);

        if (flags & ANCHOR_VCENTER)
		    gParams->ctl[n].distHeight = gParams->ctl[n].yPos - dialogHeight/2;
        else
            gParams->ctl[n].distHeight = dialogHeight - (gParams->ctl[n].yPos + gParams->ctl[n].height);
	//}

	
	return true;
}


int updateAnchors(int dialogWidth, int dialogHeight){

    int n;

    if (!fmc.doingProxy) return false;
    
    
    //Get dialog size if necessary
    if (dialogWidth == -1 || dialogHeight == -1){
        RECT rcParent;
        GetClientRect(fmc.hDlg, &rcParent);
        if (dialogWidth == -1) dialogWidth = PixelsToHDBUs(rcParent.right);
	    if (dialogHeight == -1) dialogHeight = PixelsToVDBUs(rcParent.bottom);
    }


    for (n = 0;  n < N_CTLS;  n++)
    {
		if ( (gParams->ctl[n].inuse) && gParams->ctl[n].anchor > 0){ //|| n==CTL_PREVIEW || n==CTL_PROGRESS || n==CTL_ZOOM

		/*
	        if (n == CTL_ZOOM){ //n == CTL_PREVIEW || n == CTL_PROGRESS || 
		        RECT rcCtl;
    
                //if (n == CTL_PREVIEW)
			    //    GetWindowRect( GetDlgItem(fmc.hDlg, 101), &rcCtl );
		        //else if (n == CTL_PROGRESS)
			    //    GetWindowRect( GetDlgItem(fmc.hDlg, IDC_PROGRESS1), &rcCtl );
		        //else if (n == CTL_ZOOM)
			        GetWindowRect( GetDlgItem(fmc.hDlg, IDC_BUTTON2), &rcCtl );
		        
		        ScreenToClient (fmc.hDlg, (LPPOINT)&rcCtl);
		        ScreenToClient (fmc.hDlg, (LPPOINT)&(rcCtl.right));

		        if (gParams->ctl[n].anchor & 16) //ANCHOR_HCENTER
                    gParams->ctl[n].distWidth = PixelsToHDBUs(rcCtl.left) - dialogWidth/2;
                else
                    gParams->ctl[n].distWidth = dialogWidth - PixelsToHDBUs(rcCtl.right);

		        if (gParams->ctl[n].anchor & 32) //ANCHOR_VCENTER
		            gParams->ctl[n].distHeight = PixelsToVDBUs(rcCtl.top) - dialogHeight/2;
                else
                    gParams->ctl[n].distHeight = dialogHeight - PixelsToVDBUs(rcCtl.bottom);

	        } else { */
		
		        if (gParams->ctl[n].anchor & ANCHOR_HCENTER)
                    gParams->ctl[n].distWidth = gParams->ctl[n].xPos - dialogWidth/2;
                else
                    gParams->ctl[n].distWidth = dialogWidth - (gParams->ctl[n].xPos + gParams->ctl[n].width);

                if (gParams->ctl[n].anchor & ANCHOR_VCENTER)
		            gParams->ctl[n].distHeight = gParams->ctl[n].yPos - dialogHeight/2;
                else
                    gParams->ctl[n].distHeight = dialogHeight - (gParams->ctl[n].yPos + gParams->ctl[n].height);
	        //}
		}
	}

    return true;

}

int updateAnchor(int n, int dialogWidth, int dialogHeight){

    if (!fmc.doingProxy) return false;
    
    if ( (n < 0 || n >= N_CTLS || !gParams->ctl[n].inuse) ) { //&& n != CTL_PREVIEW && n != CTL_PROGRESS && n != CTL_ZOOM
        return FALSE;
    }
    
    //Get dialog size if necessary
    if (dialogWidth == -1 || dialogHeight == -1){
        RECT rcParent;
        GetClientRect(fmc.hDlg, &rcParent);
        if (dialogWidth == -1) dialogWidth = PixelsToHDBUs(rcParent.right);
	    if (dialogHeight == -1) dialogHeight = PixelsToVDBUs(rcParent.bottom);
    }


	if ( (gParams->ctl[n].inuse) && gParams->ctl[n].anchor > 0){ // || n==CTL_PREVIEW || n==CTL_PROGRESS || n==CTL_ZOOM

/*	
        if (n == CTL_ZOOM){ //n == CTL_PREVIEW || n == CTL_PROGRESS || 
	        RECT rcCtl;

            //if (n == CTL_PREVIEW)
		    //    GetWindowRect( GetDlgItem(fmc.hDlg, 101), &rcCtl );
	        //else if (n == CTL_PROGRESS)
		    //    GetWindowRect( GetDlgItem(fmc.hDlg, IDC_PROGRESS1), &rcCtl );
	        //else if (n == CTL_ZOOM)
		        GetWindowRect( GetDlgItem(fmc.hDlg, IDC_BUTTON2), &rcCtl );
	        
	        ScreenToClient (fmc.hDlg, (LPPOINT)&rcCtl);
	        ScreenToClient (fmc.hDlg, (LPPOINT)&(rcCtl.right));

	        if (gParams->ctl[n].anchor & 16) //ANCHOR_HCENTER
                gParams->ctl[n].distWidth = PixelsToHDBUs(rcCtl.left) - dialogWidth/2;
            else
                gParams->ctl[n].distWidth = dialogWidth - PixelsToHDBUs(rcCtl.right);

	        if (gParams->ctl[n].anchor & 32) //ANCHOR_VCENTER
	            gParams->ctl[n].distHeight = PixelsToVDBUs(rcCtl.top) - dialogHeight/2;
            else
                gParams->ctl[n].distHeight = dialogHeight - PixelsToVDBUs(rcCtl.bottom);

        } else {
*/
	        if (gParams->ctl[n].anchor & 16) //ANCHOR_HCENTER
                gParams->ctl[n].distWidth = gParams->ctl[n].xPos - dialogWidth/2;
            else
                gParams->ctl[n].distWidth = dialogWidth - (gParams->ctl[n].xPos + gParams->ctl[n].width);

            if (gParams->ctl[n].anchor & 32) //ANCHOR_VCENTER
	            gParams->ctl[n].distHeight = gParams->ctl[n].yPos - dialogHeight/2;
            else
                gParams->ctl[n].distHeight = dialogHeight - (gParams->ctl[n].yPos + gParams->ctl[n].height);
        //}
	}

    return true;

}



int lockCtlScaling(int lock){

    gParams->scaleLock = lock;

    return true;
}


int scaleCtls(int dialogWidth, int dialogHeight){

    int n;

    if (!fmc.doingProxy) return false;

    //Get dialog size if necessary
    if (dialogWidth == -1 || dialogHeight == -1){
        RECT rcParent;
        GetClientRect(fmc.hDlg, &rcParent);
        if (dialogWidth == -1) dialogWidth = PixelsToHDBUs(rcParent.right);
	    if (dialogHeight == -1) dialogHeight = PixelsToVDBUs(rcParent.bottom);
    }


	lockWindow(1);
	
	for (n = 0;  n < N_CTLS;  n++)
    {
		if ( (gParams->ctl[n].inuse) && gParams->ctl[n].anchor > 0){ //|| n==CTL_PREVIEW || n==CTL_PROGRESS || n==CTL_ZOOM
			
			int newx = gParams->ctl[n].xPos;
			int newy = gParams->ctl[n].yPos;
			int neww = gParams->ctl[n].width;
			int newh = gParams->ctl[n].height;

            //if (n==CTL_PREVIEW) Info ("%d, %d",gParams->ctl[n].distWidth,gParams->ctl[n].distHeight);
			
			//Scale horizontal
			if ( gParams->ctl[n].anchor & ANCHOR_HCENTER ) 
                newx  = dialogWidth/2 +  gParams->ctl[n].distWidth;
            else if ( (gParams->ctl[n].anchor & ANCHOR_LEFT) && (gParams->ctl[n].anchor & ANCHOR_RIGHT)) 
				neww = neww + ((dialogWidth - (gParams->ctl[n].xPos + neww)) - gParams->ctl[n].distWidth);
			//Move horizontal
			else if ( !(gParams->ctl[n].anchor & ANCHOR_LEFT) && (gParams->ctl[n].anchor & ANCHOR_RIGHT)) 
				newx  = newx + ((dialogWidth - (gParams->ctl[n].xPos + neww)) - gParams->ctl[n].distWidth);

			//Scale vertical
			if ( gParams->ctl[n].anchor & ANCHOR_VCENTER ) 
                newy  = dialogHeight/2 +  gParams->ctl[n].distHeight;
            if ( (gParams->ctl[n].anchor & ANCHOR_TOP) && (gParams->ctl[n].anchor & ANCHOR_BOTTOM)) 
				newh = newh + ((dialogHeight - (gParams->ctl[n].yPos + newh)) - gParams->ctl[n].distHeight);
			//Move vertical
			else if ( !(gParams->ctl[n].anchor & ANCHOR_TOP) && (gParams->ctl[n].anchor & ANCHOR_BOTTOM))
				newy  = newy + ((dialogHeight - (gParams->ctl[n].yPos + newh)) - gParams->ctl[n].distHeight);
			
			//Do not make preview smaller to avoid crash
			if (n==CTL_PREVIEW){
				if (neww<10) neww=10;
				if (newh<10) newh=10;
			}

			//if (n==CTL_PROGRESS) Info ("scale");

            /*if ( neww == gParams->ctl[n].width && newh == gParams->ctl[n].height) 
                setCtlPos(n,newx,newy,-1,-1);
            else*/

            /*if (gParams->ctl[n].ctlclass == CC_COMBOBOX)
                setCtlPos(n,newx,newy,-1,15); //less flicker when resizing
            else*/
				setCtlPos(n,newx,newy,neww,newh);
			
		}
	}

	lockWindow(0);

    return true;

}






/*
HDC PixelhDC=0;
HDC BufferhDC=0;
HBITMAP hbmBuffer=0;
HFONT fontDB=0;


int startSetPixel (int n)
{
        //RECT rcCtl;
        //GetWindowRect(gParams->ctl[n].hCtl, &rcCtl);

        if (n==-1)
            PixelhDC = GetDC(fmc.hDlg);
        else
            PixelhDC = GetDC(gParams->ctl[n].hCtl);
        
        //BufferhDC = CreateCompatibleDC(PixelhDC);
        //hbmBuffer = CreateCompatibleBitmap(BufferhDC, rcCtl.right-rcCtl.left, rcCtl.bottom-rcCtl.top);
        //SelectObject(BufferhDC, hbmBuffer);

        return true;

}

int endSetPixel (int n)
{
        //RECT rcCtl;
        //GetWindowRect(gParams->ctl[n].hCtl, &rcCtl);

        //BitBlt(PixelhDC, 0, 0, rcCtl.right-rcCtl.left, rcCtl.bottom-rcCtl.top, BufferhDC, 0, 0, SRCCOPY);
        //DeleteObject (hbmBuffer);
        //DeleteDC (BufferhDC);

        if (n==-1)
            ReleaseDC (fmc.hDlg, PixelhDC);
        else    
            ReleaseDC (gParams->ctl[n].hCtl, PixelhDC);
        
        return true;
        
}

int setPixel (int x, int y, int value)
{   
            
        if (PixelhDC==0) return false;

        return SetPixelV (PixelhDC,x,y,(COLORREF) value);
        //return SetPixelV (BufferhDC,x,y,(COLORREF) value);

}
*/



int countProcessors(void){

	SYSTEM_INFO SystemInfo;
	
	GetSystemInfo(&SystemInfo);
 
	return (int)SystemInfo.dwNumberOfProcessors;

    //return 1;
}


void doMultiThreading(LPTHREAD_START_ROUTINE ThreadFunction, int useMT, int useSync){
	
	if (useMT)  //Use multithreading
		mt.totalcpu = countProcessors();
	else 
		mt.totalcpu  = 1;

	if (mt.totalcpu > 1) { //More than one processor
        
		HANDLE * hThread = NULL;
		DWORD * dwThread = NULL;
		INT_PTR index;

		hThread = (HANDLE *)malloc(sizeof(HANDLE)*mt.totalcpu);
		dwThread = (DWORD *)malloc(sizeof(DWORD)*mt.totalcpu);

		//if (mt.totalcpu > MAXCPU) mt.totalcpu = MAXCPU; // Up to 16 processors possible for now

		if (hThread && dwThread){
			if (useSync) createSync(mt.totalcpu);
			for (index=1; index < mt.totalcpu; index++) {
				hThread[index-1] = CreateThread( NULL, 0, ThreadFunction, (LPVOID)index, 0, &dwThread[index-1] ); //&dwThread[index-1]
			} 
		}
		ThreadFunction(0);

		if (hThread && dwThread){
			//Wait for all threads to finish
			WaitForMultipleObjects( mt.totalcpu-1, hThread, TRUE, INFINITE );
			for (index=0; index<mt.totalcpu-1; index++) CloseHandle(hThread[index]); 
			free(hThread);
			free(dwThread);
			if (useSync) deleteSync();
		}
		
	} else { //Only one Processor

		ThreadFunction(0);

	}   

}


/****** THREAD APIs ******/

#if 0

INT_PTR triggerThread(int n, int event, int previous ){

    HANDLE hThread;
    DWORD dwThread;
    FMcontext *pfmc_copy;

    if ( (n < 0 || n >= N_CTLS) && event != FME_CUSTOMEVENT ) return false;

    pfmc_copy = malloc(sizeof(FMcontext));
    if (!pfmc_copy) return false;   //couldn't allocate FMC as TLS
	

    //could do this in the thread, so it can be multitasked? 
    //but then it's hard to pass other params	
#if 0
    memcpy(pfmc_copy,&fmc,sizeof(FMcontext)); //Copy values of original FMC
#else
    *pfmc_copy = fmc; //Copy values of original FMC 
#endif

    //Set new values
    pfmc_copy->n = n;
    pfmc_copy->e = event;
    pfmc_copy->previous = previous;
    pfmc_copy->ctlMsg = event;
    pfmc_copy->ctlCode = 0;

    if (event==FME_CUSTOMEVENT)
    	pfmc_copy->ctlClass = CC_UNUSED;
    else
        pfmc_copy->ctlClass = gParams->ctl[n].ctlclass;

    
    //Do call
    //RetVal = X86_call_fmf1(gParams->CB_onCtl, (int)&fmc);
    hThread = CreateThread( NULL,
                            0, 
                            (LPTHREAD_START_ROUTINE)OnCtl,
                            (LPVOID)pfmc_copy, 
                            0, 
                            &dwThread);

	if (hThread) { //Success
		ThreadCount++;
		ThreadHandle = realloc(ThreadHandle, ThreadCount*sizeof(int));
		ThreadHandle[ThreadCount-1]	= (INT_PTR)hThread;
		ThreadMem = realloc(ThreadMem, ThreadCount*sizeof(int));
		ThreadMem[ThreadCount-1] = (INT_PTR)pfmc_copy;
		return (INT_PTR)hThread;
	} else //Failure
		free(pfmc_copy);


    return false;
}


int waitForThread(INT_PTR hThread, int ms, int userinput){

	int retval = 0; 
	
	if (ThreadCount==0) return false; // No Threads available

	if (hThread==0){ //Wait for all threads
		
		if (ms==0){

			retval = WaitForMultipleObjects( 
						ThreadCount, 
						(HANDLE *)ThreadHandle, 
						TRUE, 
						0); // 0 => immediate return

		} else {

			MSG msg;
			int count=0;
		
			while (WaitForMultipleObjects( 
						ThreadCount, 
						(HANDLE *)ThreadHandle, 
						TRUE, 
						10) == WAIT_TIMEOUT){ //Use a 10 ms time-out

				if (ms!=INFINITE){
					count = count + 10;
					if (count >= ms) {
						retval = WAIT_TIMEOUT;
						break;
					}
				}
			
                if (userinput != -1){ //Use -1 to deactivate the message pump
				    //Flush key and mouse messages
				    if (userinput==0){
					    while( PeekMessage(&msg, NULL, WM_KEYFIRST, WM_KEYLAST, PM_REMOVE ) );
					    while( PeekMessage(&msg, NULL, WM_MOUSEFIRST, WM_MOUSELAST, PM_REMOVE ) );
				    }

				    //Message Pump
				    while (PeekMessage (&msg, NULL, 0, 0, PM_REMOVE)) {
					    TranslateMessage(&msg);
					    DispatchMessage(&msg);
				    }
                }
                
			}

		}

		if (retval != WAIT_TIMEOUT) {
			int i;

			for (i=0; i<ThreadCount; i++) 
				if (ThreadHandle[i]!=0) CloseHandle((HANDLE)ThreadHandle[i]);

			for (i=0; i<ThreadCount; i++) 
				if (ThreadMem[i]!=0) free ((void *)ThreadMem[i]);

			free(ThreadHandle);
			ThreadHandle = NULL;

			free(ThreadMem);
			ThreadMem = NULL;

			ThreadCount = 0;
		}

	} /*else { //Wait for specific thread
		
		retval = WaitForSingleObject( 
						(HANDLE)hThread, 
						(DWORD) ms );
		CloseHandle((HANDLE)hThread);
	}*/	


	if (retval==WAIT_TIMEOUT)
		return false;
	else
		return true;
}


int isThreadActive(INT_PTR hThread){

	LPDWORD lpExitCode=0;

	if (ThreadCount==0) return false;

	if (hThread==0){
		int i;
		for (i=0; i<ThreadCount; i++){ 
			if (GetExitCodeThread((HANDLE)ThreadHandle[i],lpExitCode)){	
				if (*lpExitCode == STILL_ACTIVE) return true;
			}
		}

		// Cleanup Threads
		for (i=0; i<ThreadCount; i++) 
			if (ThreadHandle[i]!=0) CloseHandle((HANDLE)ThreadHandle[i]);

		for (i=0; i<ThreadCount; i++) 
			if (ThreadMem[i]!=0) free ((void *)ThreadMem[i]);

		free(ThreadHandle);
		ThreadHandle = NULL;

		free(ThreadMem);
		ThreadMem = NULL;

		ThreadCount = 0;

	} else {
		if (GetExitCodeThread((HANDLE)hThread,lpExitCode)){	
			if (*lpExitCode == STILL_ACTIVE) return true;
		}
	}

	return false;
}


int getThreadRetVal(INT_PTR hThread){

	DWORD dwExitCode=0;

	if (ThreadCount==0) return false;

	if (hThread!=0){
		if (GetExitCodeThread((HANDLE)hThread,&dwExitCode)){	
			//if (dwExitCode != STILL_ACTIVE) 
				return dwExitCode;
		}
	}

	return 0;

}



int terminateThread(INT_PTR hThread){

	if (ThreadCount==0) return true;

	if (hThread==0){
		int i;
				
		for (i=0; i<ThreadCount; i++){ 
			if (ThreadHandle[i]!=0) {
				TerminateThread ((HANDLE)ThreadHandle[i], 0);
				CloseHandle((HANDLE)ThreadHandle[i]);
			}
		}

		for (i=0; i<ThreadCount; i++) 
			if (ThreadMem[i]!=0) free ((void *)ThreadMem[i]);

		free(ThreadHandle);
		ThreadHandle = NULL;

		free(ThreadMem);
		ThreadMem = NULL;

		ThreadCount = 0;
	
	} /*else {

		TerminateThread (hThread, 0);
		CloseHandle(hThread);

	}*/

	return true;

}

#endif


/***** Critical Section APIs *****/

INT_PTR createCriticalSection(void) {
    //returns handle to CS, or 0 if failed
    LPCRITICAL_SECTION lpcs = (LPCRITICAL_SECTION)malloc(sizeof(CRITICAL_SECTION));
    if (lpcs) {
        // CS was successfully allocated, now initialize it
        InitializeCriticalSection(lpcs);
    }
    return (INT_PTR)lpcs;    //returns 0 if couldn't be allocated
}/*createCriticalSection*/


#ifdef HAVE_XP  //requires XP or later
int createCriticalSectionAndSpinCount(DWORD dwSpinCount) {
    //returns handle to CS, or 0 if failed
    LPCRITICAL_SECTION lpcs = malloc(sizeof(CRITICAL_SECTION));
    if (lpcs) {
        // CS was successfully allocated, now initialize it..
        // Returns 0 if error.
        if (InitializeCriticalSectionAndSpinCount(lpcs, dwSpinCount))
            return (int)lpcs;   //success -- return handle to CS
        else
            return 0;           //failure -- return 0
    }
    return (int)lpcs;    //returns 0 if couldn't be allocated
}/*fm_createCriticalSection*/
#endif //XP


#ifdef HAVE_VISTA //requires Vista or later
int createCriticalSectionEx(DWORD dwSpinCount, DWORD flags) {
    // flags should be 0 or CRITICAL_SECTION_NO_DEBUG_INFO
    //returns handle to CS, or 0 if failed
    LPCRITICAL_SECTION lpcs = malloc(sizeof(CRITICAL_SECTION));
    if (lpcs) {
        // CS was successfully allocated, now initialize it..
        // Returns 0 if error.
        if (InitializeCriticalSectionEx(lpcs, dwSpinCount, flags))
            return (int)lpcs;   //success -- return handle to CS
        else
            return 0;           //failure -- return 0
    }
    return (int)lpcs;    //returns 0 if couldn't be allocated
}/*fm_createCriticalSection*/
#endif //Vista


#ifdef HAVE_XP  //requires XP or later
int setCriticalSectionSpinCount(int hCS, DWORD spinCount) {
    //returns previous spin count, or 0 if hCS is 0
    //if hCS is 0, SetCriticalSectionSpinCount will GP fault!
    if (!hCS) return 0;
    return SetCriticalSectionSpinCount((LPCRITICAL_SECTION)hCS, spinCount);
}/*setCriticalSectionSpinCount*/
#endif //XP


BOOL enterCriticalSection(INT_PTR hCS) {
    //returns true upon successful entry, false if hCS is 0.
    //if hCS is 0, EnterCriticalSection will GP fault!
    if (!hCS) return false;
    EnterCriticalSection((LPCRITICAL_SECTION)hCS);
    return true;
}/*fm_enterCriticalSection*/


#if _WIN32_WINNT >= 0x0400 
BOOL tryEnterCriticalSection(INT_PTR hCS) {
    //returns true if entered CS, false if not (or hCS is 0)
    //if hCS is 0, TryEnterCriticalSection will GP fault!
    if (!hCS) return false;
    return TryEnterCriticalSection((LPCRITICAL_SECTION)hCS);
}/*fm_tryEnterCriticalSection*/
#endif


BOOL leaveCriticalSection(INT_PTR hCS) {
    //returns true upon successful exit, false if hCS is 0.
    //if hCS is 0, LeaveCriticalSection will GP fault!
    if (!hCS) return false;
    LeaveCriticalSection((LPCRITICAL_SECTION)hCS);
    return true;
}/*fm_leaveCriticalSection*/


BOOL deleteCriticalSection(INT_PTR hCS) {
    // DE-initialize the CS...
    //returns true if successful, false if hCS is 0.
    //if hCS is 0, DeleteCriticalSection will GP fault!
    if (!hCS) return false;
    DeleteCriticalSection((LPCRITICAL_SECTION)hCS);
    //now delete the CS object itself
    free((void *)hCS);
    hCS = 0;    //Mark the CS handle no longer valid
    return true;
}/*fm_deleteCriticalSection*/


/****** Interlocked Variable API's ******/

int interlockedIncrement(INT_PTR pvar) {
    // Atomically increments a variable,
    // returning the resulting incremented value.
    // 486+ only? Win98+/NT4.0+?
    return InterlockedIncrement((LONG *)pvar);
}

int interlockedDecrement(INT_PTR pvar) {
    // Atomically decrements a variable,
    // returning the resulting decremented value.
    // 486+ only? Win98+/NT4.0+?
    return InterlockedDecrement((LONG *)pvar);
}

int interlockedExchange(INT_PTR ptarget, int value) {
    // Atomically sets the target int variable to the 
    // specified value, and returns the previous value
    // of the target variable.
    return InterlockedExchange((LONG *)ptarget, value);
}

int interlockedExchangeAdd(INT_PTR ptarget, int value) {
    // Atomically adds the specified value to 
    // the target int variable, and returns the previous
    // value of the target variable.
    return InterlockedExchangeAdd((LONG *)ptarget, value);
}

int interlockedCompareExchange(INT_PTR pdest, int exchange, int comparand) {
    // Atomically compares the value of the destination variable
    // with the comparand value.  If equal, then the exchange
    // value is stored in the destination variable;
    // otherwise the destination variable is not modified.
    // Returns the initial value of the destination variable.
#if defined(_WIN64) || _MSC_VER >= 1200
    return (int)InterlockedCompareExchange((LONG*)pdest, (LONG)exchange, (LONG)comparand);
#else
    return (int)InterlockedCompareExchange((PVOID)pdest, (PVOID)exchange, (PVOID)comparand);
#endif
}


/****** Thread Synchronization API's ******/

#if 0   //#1: Harry's first version (+ Alex's mods)

int NumberOfThreads=0;
int *lpThreadArray=NULL;
LPCRITICAL_SECTION lpcs1=NULL;


int createSync(int number){

    if (number<2) return false;
        
    NumberOfThreads = number;
    lpThreadArray = calloc(number*2,4);
    if (!lpThreadArray) return false;

    lpcs1 = malloc(sizeof(CRITICAL_SECTION));
    if (!lpcs1) {
        free(lpThreadArray);
        lpThreadArray = NULL;
        return false;
    }
    InitializeCriticalSection(lpcs1);

    return true;
}


int waitForSync(int syncnr, int timeout){

    int val,i, count;
    BOOL success=false;
    int ThreadID = (int)GetCurrentThreadId();
    int timeelapsed=false;
    int starttime = clock(); 
    
    
    if (NumberOfThreads<2 || lpThreadArray==NULL || lpcs1==NULL) return false; //In case Sync was not initialized correctly


    EnterCriticalSection(lpcs1);
        for (i=0;i<NumberOfThreads;i++){

            val = lpThreadArray[i*2];

            if (val==0){
                lpThreadArray[i*2] = ThreadID; 
                lpThreadArray[i*2+1] = syncnr;
                success = true;
                break;
        
            } else if (val==ThreadID){
                lpThreadArray[i*2+1] = syncnr;
                success = true;
                break;
            }

        }
    LeaveCriticalSection(lpcs1);

    if (!success) return false; //because an unknown thread entered waitForSync



    while (true){
        count=0;
        for (i=0;i<NumberOfThreads;i++){
            if (lpThreadArray[i*2]!=0 && lpThreadArray[i*2+1] >= syncnr) count++;
        }

        if (count>=NumberOfThreads){
            timeelapsed=false;
            break;
        }

        if (timeout!=0xffffffff && clock()-starttime >= timeout) {
            timeelapsed=true; 
            break;
        }

        Sleep(0);
    }  


    if (timeelapsed)
        return false;
    else
        return true;
}


int deleteSync(void){

    NumberOfThreads = 0;

    if (lpThreadArray!=NULL){
        free (lpThreadArray);
        lpThreadArray=NULL;
    }

    if (lpcs1 != NULL){
        DeleteCriticalSection(lpcs1);
        free (lpcs1);
        lpcs1=NULL;
    }

    return true;
}

#elif 0 //#2: Tighten up the Critical Section *******************************************

int NumberOfThreads=0;
int *lpThreadArray=NULL;
LPCRITICAL_SECTION lpcs1=NULL;


int createSync(int number){

    if (number<2) return false;
        
    NumberOfThreads = number;
    lpThreadArray = calloc(number*2,4);
    if (!lpThreadArray) return false;

    lpcs1 = malloc(sizeof(CRITICAL_SECTION));
    if (!lpcs1) {
        free(lpThreadArray);
        lpThreadArray = NULL;
        return false;
    }
    InitializeCriticalSection(lpcs1);

    return true;
}


int waitForSync(int syncnr, int timeout){

    int val,i, count;
    BOOL success=false;
    int ThreadID = (int)GetCurrentThreadId();
    int timeelapsed=false;
    int starttime = clock(); 
    
    
    if (NumberOfThreads<2 || lpThreadArray==NULL || lpcs1==NULL) return false; //In case Sync was not initialized correctly


    //search for an empty slot (or one we already own)...

    for (i=0;i<NumberOfThreads;i++){

        //fetching, testing, and claiming slot i
        //must be atomic...
        EnterCriticalSection(lpcs1);        /****** enter CS ******/
        val = lpThreadArray[i*2];
        if (val==0){
            //take ownership of this slot
            lpThreadArray[i*2] = ThreadID;
            //now no one else can claim slot i
            LeaveCriticalSection(lpcs1);    /****** exit CS ******/

            //record our current sync number
            lpThreadArray[i*2+1] = syncnr;
            success = true;
            break;
    
        }
        LeaveCriticalSection(lpcs1);         /****** exit CS ******/

        //someone already owns slot i, but who?
        //(once slot i is claimed, it never gets
        //changed until deletion time)
        if (val==ThreadID){
            //we already own this slot...
            //record our current sync number
            lpThreadArray[i*2+1] = syncnr;
            success = true;
            break;
        }

    }//for i

    if (!success) return false; //because an unknown thread entered waitForSync



    while (true){
        count=0;
        for (i=0;i<NumberOfThreads;i++){
            if (lpThreadArray[i*2]!=0 && lpThreadArray[i*2+1] >= syncnr) count++;
        }

        if (count>=NumberOfThreads){
            timeelapsed=false;
            break;
        }

        if (timeout!=0xffffffff && clock()-starttime >= timeout) {
            timeelapsed=true; 
            break;
        }

        Sleep(0);
    }  


    if (timeelapsed)
        return false;
    else
        return true;
}


int deleteSync(void){

    NumberOfThreads = 0;

    if (lpThreadArray!=NULL){
        free (lpThreadArray);
        lpThreadArray=NULL;
    }

    if (lpcs1 != NULL){
        DeleteCriticalSection(lpcs1);
        free (lpcs1);
        lpcs1=NULL;
    }

    return true;
}

#elif 0 //#3: Using Interlocked Instruction instead of Critical Section ******************************

int NumberOfThreads=0;
int *lpThreadArray=NULL;


int createSync(int number){

    if (number<2) return false;
        
    NumberOfThreads = number;
    lpThreadArray = calloc(number*2,4);
    if (!lpThreadArray) return false;

    return true;
}


int waitForSync(int syncnr, int timeout){

    int val,i, count;
    BOOL success=false;
    int ThreadID = (int)GetCurrentThreadId();
    int timeelapsed=false;
    int starttime = clock(); 
    
    
    if (NumberOfThreads<2 || lpThreadArray==NULL) return false; //In case Sync was not initialized correctly


    //search for an empty slot (or one we already own)...

    for (i=0;i<NumberOfThreads;i++){

        //fetching, testing, and claiming slot i
        //must be atomic...
        val = (int)InterlockedCompareExchange((PVOID *)&lpThreadArray[i*2], (PVOID)ThreadID, 0);
        //val is the *previous* value of this slot
        if (val==0){
            //we took ownership of this slot
            //now no one else can claim slot i
            //record our current sync number
            lpThreadArray[i*2+1] = syncnr;
            success = true;
            break;
    
        }

        //someone already owns slot i, but who?
        //(once slot i is claimed, it never gets
        //changed until deletion time)
        if (val==ThreadID){
            //we already own this slot...
            //record our current sync number
            lpThreadArray[i*2+1] = syncnr;
            success = true;
            break;
        }

    }//for i

    if (!success) return false; //because an unknown thread entered waitForSync



    while (true){
        count=0;
        for (i=0;i<NumberOfThreads;i++){
            if (lpThreadArray[i*2]!=0 && lpThreadArray[i*2+1] >= syncnr) count++;
        }

        if (count>=NumberOfThreads){
            timeelapsed=false;
            break;
        }

        if (timeout!=0xffffffff && clock()-starttime >= timeout) {
            timeelapsed=true; 
            break;
        }

        Sleep(0);
    }  


    if (timeelapsed)
        return false;
    else
        return true;
}


int deleteSync(void){

    NumberOfThreads = 0;

    if (lpThreadArray!=NULL){
        free (lpThreadArray);
        lpThreadArray=NULL;
    }

    return true;
}

#elif 1 //#4: Tighten up the While-loop ******************************

int NumberOfThreads=0;
int *lpThreadArray=NULL;


int createSync(int number){

    if (number<2) return false;
        
    NumberOfThreads = number;
    lpThreadArray = (int *)calloc(number*2,4);
    if (!lpThreadArray) return false;

    return true;
}//createSync


int waitForSync(int syncnr, int timeout){

    int val, i;
    BOOL success=false;
    int ThreadID = (int)GetCurrentThreadId();
    BOOL timeelapsed=false;
    int starttime = clock(); 
    
    if (NumberOfThreads<2 || lpThreadArray==NULL) return false; //In case Sync was not initialized correctly

    //search for an empty slot (or one we already own)...

    for (i=0; i<NumberOfThreads; i++){

        //fetching, testing, and claiming slot i
        //must be atomic...
        #if defined(_WIN64) || _MSC_VER >= 1200
            val = (int)InterlockedCompareExchange((LONG*)&lpThreadArray[i*2], (LONG)ThreadID, 0);
        #else
            val = (int)InterlockedCompareExchange((PVOID *)&lpThreadArray[i*2], (PVOID)ThreadID, 0);
        #endif
        //val is the *previous* value of this slot
        if (val==0){
            //we took ownership of this slot
            //now no one else can claim slot i
            //record our current sync number
            lpThreadArray[i*2+1] = syncnr;
            success = true;
            break;
    
        }

        //someone already owns slot i, but who?
        //(once slot i is claimed, it never gets
        //changed until deletion time)
        if (val==ThreadID){
            //we already own this slot...
            //record our current sync number
            lpThreadArray[i*2+1] = syncnr;
            success = true;
            break;
        }

    }//for i

    if (!success) return false; //because an unknown thread entered waitForSync


    // Wait until all threads are at the current syncnr or greater.

    for (i=0; i<NumberOfThreads && !timeelapsed; i++) {
        //wait until thread i reaches syncnr or greater...
        while(lpThreadArray[i*2]==0 || lpThreadArray[i*2+1] < syncnr) {
            //check for timeout
            if ((timeout!=INFINITE && clock()-starttime >= timeout) || mtAbortFlag) {
                timeelapsed=true; 
                break;
            }//if timeout
            //yield this timeslice
            Sleep(0);
        }//while
    }//for

    //Either we timed out, or all threads are at current syncnr or greater...
    return !timeelapsed;

}//waitForSync


int deleteSync(void){

    NumberOfThreads = 0;

    if (lpThreadArray!=NULL){
        free (lpThreadArray);
        lpThreadArray=NULL;
    }

    return true;
}//deleteSync

#elif 0 //#5: Cache mySlot ******************************

int NumberOfThreads=0;
int *lpThreadArray=NULL;


int createSync(int number){

    if (number<2) return false;
        
    NumberOfThreads = number;
    lpThreadArray = calloc(number*2,4);
    if (!lpThreadArray) return false;

    return true;
}//createSync


int waitForSync(int syncnr, int timeout){

    THREAD_LOCAL static int mySlot = -1;
    int val, i;
    BOOL success=false;
    int ThreadID = (int)GetCurrentThreadId();
    BOOL timeelapsed=false;
    int starttime = clock(); 
    
    if (NumberOfThreads<2 || lpThreadArray==NULL) return false; //In case Sync was not initialized correctly

    //search for an empty slot (if we don't already own one)...

    if (mySlot == -1) {
        //not yet assigned a slot...
        for (i=0; i<NumberOfThreads; i++){
            //fetching, testing, and claiming slot i
            //must be atomic...
            val = (int)InterlockedCompareExchange((PVOID *)&lpThreadArray[i*2], (PVOID)ThreadID, 0);
            //val is the *previous* value of this slot
            if (val==0){
                //we took ownership of this slot
                //now no one else can claim slot i
                //cache our assigned slot#
                mySlot = i;
                success = true;
                break;
            }//if
        }//for i
        if (!success) return false; //because an unknown thread entered waitForSync
    }//if

    lpThreadArray[mySlot*2+1] = syncnr;  //record our current syncnr


    // Wait until all threads are at the current syncnr or greater.

    for (i=0; i<NumberOfThreads && !timeelapsed; i++) {
        //wait until thread i reaches syncnr or greater...
        while(lpThreadArray[i*2]==0 || lpThreadArray[i*2+1] < syncnr) {
            //check for timeout
            if (timeout!=INFINITE && clock()-starttime >= timeout) {
                timeelapsed=true; 
                break;
            }//if timeout
            //yield this timeslice
            Sleep(0);
        }//while
    }//for

    //Either we timed out, or all threads are at current syncnr or greater...
    return !timeelapsed;

}//waitForSync


int deleteSync(void){

    NumberOfThreads = 0;

    if (lpThreadArray!=NULL){
        free (lpThreadArray);
        lpThreadArray=NULL;
    }

    return true;
}//deleteSync

#elif 0 //#6: Assign mySlot directly ******************************

int NumberOfThreads=0;
int *lpThreadArray=NULL;
int NextSlot=0;


int createSync(int number){

    if (number<2) return false;
        
    NumberOfThreads = number;
    NextSlot = 0;
    lpThreadArray = calloc(number,4);
    if (!lpThreadArray) return false;

    return true;
}//createSync


int waitForSync(int syncnr, int timeout){

    THREAD_LOCAL static int mySlot = -1;
    int i;
    BOOL timeelapsed=false;
    int starttime = clock(); 
    
    if (NumberOfThreads<2 || lpThreadArray==NULL) return false; //In case Sync was not initialized correctly
    if (syncnr < 1) return false; //syncnr must be 1 or greater

    //first time around, assign next slot in ThreadArray to this thread...

    if (mySlot == -1) {
        //not yet assigned a slot...
#if 0 //InterlockedIncrement requires Win98+ or NT 4.0+ for proper return value
        mySlot = InterlockedIncrement(&NextSlot) - 1;
#else
        mySlot = InterlockedExchangeAdd(&NextSlot, 1); //should work on all cpus and Win versions?
#endif
        if (mySlot >= NumberOfThreads) return false; //an unknown thread entered waitForSync??
    }//if

    lpThreadArray[mySlot] = syncnr;  //record our current syncnr


    // Wait until all threads are at the current syncnr or greater.

    for (i=0; i<NumberOfThreads && !timeelapsed; i++) {
        //wait until thread i reaches syncnr or greater...
        //(note that an as-yet empty slot will have a syncnr value of 0)
        while(lpThreadArray[i] < syncnr) {
            //check for timeout
            if (timeout!=INFINITE && clock()-starttime >= timeout) {
                timeelapsed=true; 
                break;
            }//if timeout
            //yield this timeslice
            Sleep(0);
        }//while
    }//for

    //Either we timed out, or all threads are at current syncnr or greater...
    return !timeelapsed;

}//waitForSync


int deleteSync(void){

    NumberOfThreads = 0;

    if (lpThreadArray!=NULL){
        free (lpThreadArray);
        lpThreadArray=NULL;
    }

    return true;
}//deleteSync

#elif 1 //#7: Let syncnr be TLS rather than a param ******************************

int NumberOfThreads=0;
int *lpThreadArray=NULL;
int NextSlot=0;


int createSync(int number){

    if (number<2) return false;
        
    NumberOfThreads = number;
    NextSlot = 0;
    lpThreadArray = calloc(number,sizeof(*lpThreadArray));
    if (!lpThreadArray) return false;

    return true;
}//createSync


int waitForSync(int unused, int timeout){

    THREAD_LOCAL static int mySlot = -1;
    THREAD_LOCAL static int syncnr = 0;
    int i;
    BOOL timeelapsed=false;
    int starttime = clock(); 
    
    if (NumberOfThreads<2 || lpThreadArray==NULL) return false; //In case Sync was not initialized correctly

    syncnr++;   //increment current syncnr level (must always be > 0)
    if(syncnr <= 0) return false;   //oops

    //first time around, assign next slot in ThreadArray to this thread...

    if (mySlot == -1) {
        //not yet assigned a slot...
#if 0 //InterlockedIncrement requires Win98+ or NT 4.0+ for proper return value
        mySlot = InterlockedIncrement(&NextSlot) - 1;
#else
        mySlot = InterlockedExchangeAdd(&NextSlot, 1); //should work on all cpus and Win versions?
#endif
        if (mySlot >= NumberOfThreads) return false; //an unknown thread entered waitForSync??
    }//if

    lpThreadArray[mySlot] = syncnr;  //record our current syncnr


    // Wait until all threads are at the current syncnr or greater.

    for (i=0; i<NumberOfThreads && !timeelapsed; i++) {
        //wait until thread i reaches syncnr or greater...
        //(note that an as-yet empty slot will have a syncnr value of 0)
        while(lpThreadArray[i] < syncnr) {
            //check for timeout
            if (timeout!=INFINITE && clock()-starttime >= timeout) {
                timeelapsed=true; 
                break;
            }//if timeout
            //yield this timeslice
            Sleep(0);
        }//while
    }//for

    //Either we timed out, or all threads are at current syncnr or greater...
    return !timeelapsed;

}//waitForSync


int deleteSync(void){

    NumberOfThreads = 0;

    if (lpThreadArray!=NULL){
        free (lpThreadArray);
        lpThreadArray=NULL;
    }

    return true;
}//deleteSync

#endif //Sync API versions ********************************************



LPCRITICAL_SECTION lpcs = NULL;

void createSection(){
	if (lpcs) deleteSection();
	lpcs = (LPCRITICAL_SECTION)malloc(sizeof(CRITICAL_SECTION));
	if (lpcs)
		InitializeCriticalSection(lpcs);
}

void enterSection(){
	if (!lpcs) return;
	EnterCriticalSection(lpcs);
}

void leaveSection(){
	if (!lpcs) return;
	LeaveCriticalSection(lpcs);
}

void deleteSection(){	
	if (!lpcs) return;
	DeleteCriticalSection(lpcs);
	free(lpcs);
	lpcs = NULL;
}



/****** EVENT API'S ******/

int triggerEvent(int n, int event, int previous ){

        int RetVal=0;

        //Save values
        int n2 = fmc.n;
        int e2 = fmc.e;
        int previous2 = fmc.previous;
        int ctlMsg2 = fmc.ctlMsg;
        int ctlCode2 = fmc.ctlCode;
        int ctlClass2 = fmc.ctlClass;
    

		if ( (n < 0 || n >= N_CTLS) && event != FME_CUSTOMEVENT ) return false;


        //Set new values
        fmc.n = n;
        fmc.e = event;
        fmc.previous = previous;
        fmc.ctlMsg = event;//WM_SIZE;
        fmc.ctlCode = 0;

		if (event==FME_CUSTOMEVENT)
			fmc.ctlClass = CC_UNUSED;
		else
			fmc.ctlClass = gParams->ctl[n].ctlclass;
        
		//Do call
        //RetVal = OnCtl((FMcontext * const)&fmc, n, event, previous);
		RetVal = call_FFP(FFP_OnCtl, (INT_PTR)&fmc);

        //Restore old values
        fmc.n = n2;
        fmc.e = e2;
        fmc.previous = previous2;
        fmc.ctlMsg = ctlMsg2;
        fmc.ctlCode = ctlCode2;
        fmc.ctlClass = (CTLCLASS)ctlClass2;

        return RetVal;
}


int doEvents(void) {  

        MSG msg; 
        
        while ( PeekMessage(&msg, fmc.hDlg, 0, 0, PM_REMOVE ) )  {  
            TranslateMessage(&msg);
            DispatchMessage(&msg);
            if (msg.message == WM_QUIT) break; // controlled by windows
            if (msg.message == WM_CLOSE && msg.hwnd == fmc.hDlg) break;
            //if( m_bShutDown != FALSE ) break; // controlled by program
        }

        return true;
}


#if APP

//int cachedScaleFactor;
int cachedFMC[4];

int requestRect (int inLeft, int inTop, int inRight, int inBottom, int scaleFactor)
{

	if (fmc.doingProxy){

		//Only for 100% zoom
		if (scaleFactor != 1) return false; //fmc.scaleFactor==1 || 

		//cachedScaleFactor = fmc.scaleFactor;
		//fmc.scaleFactor = scaleFactor;
		//populateFMC(IMAGE,IMAGEOUT);

		cachedFMC[0]=fmc.x_start;
		cachedFMC[1]=fmc.y_start;
		cachedFMC[2]=fmc.x_end;
		cachedFMC[3]=fmc.y_end;

		//Set values necessary for src()
		gStuff->inData = Array[IMAGE];
		MyAdjustedInData = gStuff->inData;
		fmc.x_start = 0;
		fmc.y_start = 0;
		fmc.x_end = fmc.X = ArrayX[IMAGE];
		fmc.y_end = fmc.Y = ArrayY[IMAGE];
		fmc.srcRowDelta = ArrayX[IMAGE]*ArrayZ[IMAGE]*ArrayBytes[IMAGE];
		fmc.srcColDelta = ArrayZ[IMAGE];
	}
    
	//Info ("%d x %d",fmc.X,fmc.Y);
	return true;
}

int restoreRect (void)
{

	if (fmc.doingProxy){

		//if (fmc.scaleFactor==1) return false; 

		//fmc.scaleFactor = cachedScaleFactor;
		//populateFMC(PREVIEW,PREVIEWOUT);
		//Info ("%d,%d - %d,%d",fmc.x_start,fmc.y_start,fmc.x_end,fmc.y_end);

		gStuff->inData = Array[PREVIEW];
		MyAdjustedInData = gStuff->inData;
		fmc.x_start = cachedFMC[0];
		fmc.y_start = cachedFMC[1];
		fmc.x_end = cachedFMC[2];
		fmc.y_end = cachedFMC[3];
		fmc.srcRowDelta = ArrayX[PREVIEW]*ArrayZ[PREVIEW]*ArrayBytes[PREVIEW];
		fmc.srcColDelta = ArrayZ[PREVIEW];
	}
    
	//Info ("%d x %d",fmc.X,fmc.Y);
	return true;
}

#else

	  
int restoreRect (void){

    //Only run in proxy mode
    if (!fmc.doingProxy) return false;

    //imageOffset.x += 1;
    //imageOffset.y += 1;

    GetProxyItemRect (fmc.hDlg, &gProxyRect);
    SetupFilterRecordForProxy (globals, (long *)&fmc.scaleFactor, fmc.scaleFactor, &imageOffset);


    //Restore Values

{ //scope
#if BIGDOCUMENT
	VRect scaledFilterRect;
#else
	Rect scaledFilterRect;
#endif
    OSErr err = noErr;	

#if BIGDOCUMENT
    fmc.outRect32 = GetOutRect();	//needed??
#endif

    fmc.scaleFactor = gStuff->inputRate >> 16;
    fmc.srcPtr = (unsigned8 *) MyAdjustedInData;
    fmc.dstPtr = (unsigned8 *) gStuff->outData;
    fmc.mskData = (unsigned8 *) MyAdjustedMaskData;
#if BIGDOCUMENT
    fmc.rows = fmc.outRect32.bottom - fmc.outRect32.top;
    fmc.columns = fmc.outRect32.right - fmc.outRect32.left;
#else
    fmc.rows = gStuff->outRect.bottom - gStuff->outRect.top;
    fmc.columns = gStuff->outRect.right - gStuff->outRect.left;
#endif
    fmc.srcRowDelta = gStuff->inRowBytes;
    fmc.srcColDelta = gStuff->inHiPlane - gStuff->inLoPlane + 1;
    fmc.inPlanes = fmc.srcColDelta;
    fmc.dstRowDelta = gStuff->outRowBytes;
    fmc.dstColDelta = gStuff->planes;
    fmc.mskRowDelta = gStuff->maskRowBytes;
    fmc.mskColDelta = 1;

#ifndef DONT_USE_MASK
    if (gStuff->haveMask)
    {   // mask stuff already set up for selection mask...
        gNoMask = FALSE;
    }
    else
#endif
    if (fmc.inPlanes >= 4)
    {   // Channel 3 is probably an alpha channel, so
        // use the alpha channel (layer transparency mask)
        // instead of the selection mask
        fmc.mskData = (unsigned8 *) MyAdjustedInData + 3;
        fmc.mskRowDelta = fmc.srcRowDelta;
        fmc.mskColDelta = fmc.srcColDelta;
        gNoMask = FALSE;
    }
    else
    {   // No selection mask and no transparency channel, so
        // we'll just fake a mask that includes the entire
        // filterRect (which will give us a nice rectangular
        // bevel).
        gNoMask = TRUE;
    }

#if BIGDOCUMENT
	//scaledFilterRect = GetFilterRect();
	fmc.filterRect32 = GetFilterRect();	//needed?
	scaledFilterRect = fmc.filterRect32;
	scaleRect32(&scaledFilterRect, 1, fmc.scaleFactor);
    X0 = scaledFilterRect.left;
    Y0 = scaledFilterRect.top;
	fmc.inRect32 = GetInRect();	//needed??
    fmc.x_start = fmc.inRect32.left - X0;
    fmc.y_start = fmc.inRect32.top - Y0;
#else
    copyRect (&scaledFilterRect, &gStuff->filterRect);
    scaleRect (&scaledFilterRect, 1, fmc.scaleFactor);
    X0 = scaledFilterRect.left;
    Y0 = scaledFilterRect.top;
    fmc.x_start = gStuff->inRect.left - X0;
    fmc.y_start = gStuff->inRect.top - Y0;
#endif  


    fmc.x_end = fmc.x_start + fmc.columns;
    fmc.y_end = fmc.y_start + fmc.rows;

    fmc.X = fmc.xmax = scaledFilterRect.right - X0;
    fmc.Y = fmc.ymax = scaledFilterRect.bottom - Y0;

}//scope

    return true;

}


//Note: Requires rect coordinates in original image size
int requestRect (int inLeft, int inTop, int inRight, int inBottom, int scaleFactor)
{
    long    maxScaleFactor;
#if BIGDOCUMENT
	VRect    scaledImage;
#else
    Rect    scaledImage;
#endif

    /*
    //Scale params to 100%
    inLeft *= scaleFactor;
    inTop *= scaleFactor;
    inRight *= scaleFactor;
    inBottom *= scaleFactor;
    */		  

    //Only run in proxy mode
    if (!fmc.doingProxy) return false;

#if BIGDOCUMENT
        fmc.inRect32.left    = inLeft;
        fmc.inRect32.right   = inRight;
        fmc.inRect32.top     = inTop;
        fmc.inRect32.bottom  = inBottom;
		SetInRect(fmc.inRect32);

        fmc.filterRect32 = GetFilterRect();
		CalcProxyScaleFactor (&fmc.filterRect32, &gProxyRect, &maxScaleFactor);
#else
        gStuff->inRect.left   = (short)inLeft;
        gStuff->inRect.right  = (short)inRight;
        gStuff->inRect.top    = (short)inTop;
        gStuff->inRect.bottom = (short)inBottom;
        CalcProxyScaleFactor (&gStuff->filterRect, &gProxyRect, &maxScaleFactor); //Calc maxScaleFactor
#endif

    if (scaleFactor == -888)
    {   // -888 means use calculated optimal scale factor...
        scaleFactor = maxScaleFactor;
    }

// Make sure scale factor is in range [1,MAX_SCALE_FACTOR].
    if (scaleFactor < 1) scaleFactor = 1;
    if (scaleFactor > MAX_SCALE_FACTOR) scaleFactor = MAX_SCALE_FACTOR;

#if LOWZOOM
	scaleFactor = roundUpScaleFactor(scaleFactor);
#else
    // Skip scalefactor 13 or 15...
    if (scaleFactor == 13 || scaleFactor == 15)
        scaleFactor++;  //use next higher scaleFactor (14 or 16)
#endif
		   
#if BIGDOCUMENT
	//fmc.inRect32 = fmc.filterRect32; 
    fmc.inRect32 = GetInRect();	//needed??
	scaleRect32(&fmc.inRect32, 1, scaleFactor);
	scaledImage = fmc.inRect32;
    SetInRect(fmc.inRect32);	//needed??
#else
    //copyRect (&gStuff->inRect, &gStuff->filterRect);
    scaleRect (&gStuff->inRect, 1, scaleFactor);
    copyRect (&scaledImage, &gStuff->inRect);
#endif

    if ((gParams->flags & (AFH_DRAGGING_PROXY|AFH_ZERO_OUTPUT_DRAG|AFH_DRAG_FILTERED_OUTPUT))
                       == (AFH_DRAGGING_PROXY|AFH_ZERO_OUTPUT_DRAG)) 
    {   // Don't need to set up output Rect.  Could save some time.
       #if BIGDOCUMENT
            PISetRect32(&fmc.outRect32, 0, 0, 0, 0); 
            SetOutRect(fmc.outRect32);
       #else
            PISetRect (&gStuff->outRect, 0, 0, 0, 0);
	   #endif
    } else {
       #if BIGDOCUMENT
			fmc.outRect32 = fmc.inRect32;
            SetOutRect(fmc.outRect32);
       #else
            copyRect (&gStuff->outRect, &gStuff->inRect);
       #endif
    }

    gStuff->inLoPlane = gStuff->outLoPlane = 0;
    gStuff->inHiPlane = gStuff->outHiPlane = gStuff->planes - 1;

#if BIGDOCUMENT
  #ifndef DONT_USE_MASK
	if (gStuff->haveMask)
		fmc.maskRect32 = fmc.inRect32;
    else
  #endif
        PISetRect32 (&fmc.maskRect32, 0, 0, 0, 0);
	SetMaskRect(fmc.maskRect32);
#else				 
  #ifndef DONT_USE_MASK
    if (gStuff->haveMask)
        copyRect(&gStuff->maskRect, &gStuff->inRect);
    else
  #endif
        PISetRect (&gStuff->maskRect, 0, 0, 0, 0);
#endif

    gStuff->inputRate = gStuff->maskRate = long2fixed(scaleFactor);
    
    if (gParams->flags & AFH_USE_HOST_PADDING)
    {
        gStuff->inputPadding = 0;   // 0=black.
        gStuff->outputPadding = 0;  // 0=black.
        gStuff->maskPadding = 0;    // 0=black (non-selected).
    }
    else
    {
        gStuff->inputPadding = plugInDoesNotWantPadding;
        gStuff->outputPadding = plugInDoesNotWantPadding;
        gStuff->maskPadding = plugInDoesNotWantPadding;
    }

    // Now call AdvanceState() for real, to get the actual proxy pixels.
    addPadding(globals);

    gResult = AdvanceState();

    if (gResult != noErr) {
        // We use the special combo MB_ICONHAND|MB_SYSTEMMODAL
        // to ensure that the message box will be displayed
        // regardless of low memory...
        if (gResult == memFullErr) {
            msgBox( MB_ICONHAND|MB_SYSTEMMODAL,
                    fmc.filterTitleText,//"FilterMeister",
                    "Not enough memory to perform requestRect() at this zoom level."
                    );
            return false;
        }
        else {
            msgBox( MB_ICONHAND|MB_SYSTEMMODAL,
                    fmc.filterTitleText,//"FilterMeister",
                    "Unknown error (%d) while perfoming requestRect()\n",
                    gResult
                    );
            return false;
        }
    }
    
    subtractPadding(globals);

    // Need to set outRect correctly for ShowOutput();
#if BIGDOCUMENT
    fmc.inRect32 = GetInRect();	//needed??	//This was commented out before
	fmc.outRect32 = fmc.inRect32;
	SetOutRect(fmc.outRect32);
#else
    copyRect (&gStuff->outRect, &gStuff->inRect);
#endif

    // Now gStuff->inData should have the (possibly padded) subsampled version of input data,
    // gStuff->maskData may have the (possibly padded) subsampled version of the mask data,
    // and gStuff->outData should be a buffer the size of input data that
    // could be used as a temporary buffer to do intermediate filtering
    // to display proxies

    // MyAdjustedInData and MyAdjustedMaskData will point to the start of the NON-padded
    // data.

    //Setup Values

    //fmc.scaleFactor = gStuff->inputRate >> 16; //Don't use because it confuses restoreImageData()
    fmc.srcPtr = (unsigned8 *) MyAdjustedInData;
    fmc.dstPtr = (unsigned8 *) gStuff->outData;
    fmc.mskData = (unsigned8 *) MyAdjustedMaskData;
#if BIGDOCUMENT
	fmc.outRect32 = GetOutRect();	//needed??
	fmc.rows = fmc.outRect32.bottom - fmc.outRect32.top;
    fmc.columns = fmc.outRect32.right - fmc.outRect32.left;
#else
    fmc.rows = gStuff->outRect.bottom - gStuff->outRect.top;
    fmc.columns = gStuff->outRect.right - gStuff->outRect.left;
#endif
    fmc.srcRowDelta = gStuff->inRowBytes;
    fmc.srcColDelta = gStuff->inHiPlane - gStuff->inLoPlane + 1;
    fmc.inPlanes = fmc.srcColDelta;
    fmc.dstRowDelta = gStuff->outRowBytes;
    fmc.dstColDelta = gStuff->planes;
    fmc.mskRowDelta = gStuff->maskRowBytes;
    fmc.mskColDelta = 1;

#ifndef DONT_USE_MASK
    if (gStuff->haveMask)
    {   // mask stuff already set up for selection mask...
        gNoMask = FALSE;
    }
    else
#endif
    if (fmc.inPlanes >= 4)
    {   // Channel 3 is probably an alpha channel, so
        // use the alpha channel (layer transparency mask)
        // instead of the selection mask
        fmc.mskData = (unsigned8 *) MyAdjustedInData + 3;
        fmc.mskRowDelta = fmc.srcRowDelta;
        fmc.mskColDelta = fmc.srcColDelta;
        gNoMask = FALSE;
    }
    else
    {   // No selection mask and no transparency channel, so
        // we'll just fake a mask that includes the entire
        // filterRect (which will give us a nice rectangular
        // bevel).
        gNoMask = TRUE;
    }

    /*fmc.x_start = gStuff->inRect.left;
    fmc.y_start = gStuff->inRect.top;
    fmc.x_end = gStuff->inRect.right;
    fmc.y_end = gStuff->inRect.bottom;*/
    fmc.x_start = 0;
    fmc.y_start = 0;
    
#if BIGDOCUMENT		   
        fmc.x_end = fmc.inRect32.right - fmc.inRect32.left;
        fmc.y_end = fmc.inRect32.bottom - fmc.inRect32.top;
#else    
        fmc.x_end = gStuff->inRect.right - gStuff->inRect.left;
        fmc.y_end = gStuff->inRect.bottom - gStuff->inRect.top;
#endif

    fmc.X = fmc.xmax = fmc.x_end - fmc.x_start;
    fmc.Y = fmc.ymax = fmc.y_end - fmc.y_start;

    return true;

} /*fm_requestRect*/


#endif



//#if NSSOFTENER2

/********************************************************************/
/*                 DLL Access Routines                              */
/*                                                                  */
/*  1. Original code contributed by Florian Xhumari, as transmitted */
/*     by Harald Heim.                                              */
/*  2. Remove commented-out code (Alex Hunter).                     */
/*                                                                  */
/********************************************************************/


typedef void (*FMLIB_FN)();


INT_PTR loadLib(INT_PTR libname) 
{
    HINSTANCE retval;
	
	if (!strrchr((char *)libname, 0x5C)){
		char string[512];

		strcpy(string, fmc.filterInstallDir); //Always has an ending \ sign
		strcat(string,(char *)libname);
		retval = LoadLibrary((LPCTSTR)string);
		if (retval != 0) return (INT_PTR) retval;
	}

	return (INT_PTR)LoadLibrary((char *)libname); 
}


int freeLib(INT_PTR hinstLib)
{
	return FreeLibrary((HINSTANCE)hinstLib);
}


INT_PTR getLibFn(INT_PTR hinstLib, INT_PTR fnName)
{
	return (INT_PTR)GetProcAddress((HINSTANCE)hinstLib, (char *)fnName);
}


// Disable "no return value" warnings
#ifdef _WIN64
    #pragma warning(disable:4716)
#else
    #pragma warning(disable:4035)
#endif

#ifndef _WIN64
__declspec(naked)
#endif
int callLibFmc(int fn, ...)
{
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

#ifndef _WIN64

    __asm {
#if 0
        mov eax, [esp+4]        ; eax <- fn
        mov [esp+4], ebx        ; replace fn with ebx (fmcp) in arg list
        jmp eax                 ; jmp to fn, which returns to caller
#elif 1
        mov eax, [esp+4]        ; eax <- fn
        mov ebx, gFmcp          ; (In case we weren't called from FM-generated code!)
        mov [esp+4], ebx        ; replace fn with ebx (fmcp) in arg list
        jmp eax                 ; jmp to fn, which returns to caller
#elif 0
        mov eax, ebx            ; eax <- ebx (=fmcp)
        xchg eax, [esp+4]       ; swap with fn in arg list; eax now has fn  //don't use XCHG per Fog!!!
        jmp eax                 ; jmp to fn, which returns to caller
#else
        ; old version, no fmcp argument...
        jmp dword ptr [esp+4]   ; jump to the function; it will return to caller of current function
#endif
        //
        // return result will be in eax (for callLibraryIntFast) or
        // on top of NDP stack ST(0) (for callLIbraryDoubleFast) --
        // only the caller cares which.
	}
#endif//Win64
}

#if 0
////////////// TEST PATCH /////////////////////
     
           typedef int (*ifn1v) (int, ...);
           typedef double (*dfn1v) (int, ...);
           #define fm_callLibraryIntFast      ((ifn1v)fm_callLibraryFast)
           #define fm_callLibraryDoubleFast   ((dfn1v)fm_callLibraryFast)

void xyxxy() {
           int g_hMyDll = gFmcp->loadLibrary((INT_PTR)"MyDll");
           if (!g_hMyDll) MessageBeep(MB_OK);
           {
           int g_pfnMyForEveryTile = gFmcp->getLibraryFunction(g_hMyDll, (INT_PTR)"MyForEveryTile");
           if (!g_pfnMyForEveryTile) MessageBeep(MB_OK);
           {    
           int g_pfnMyFloat = gFmcp->getLibraryFunction(g_hMyDll, (INT_PTR)"MyFloat");
           if (!g_pfnMyFloat) MessageBeep(MB_OK);
           {
           int iRes = fm_callLibraryIntFast(g_pfnMyForEveryTile, 100, 200);
           double x1,x2,x3,x4,x5,x6,x7,x8,x9;
           double fRes = fm_callLibraryDoubleFast(g_pfnMyFloat, 10, 1.2, 3.14);

           x1 = sin(x1)+cos(x2)+sin(x3+x4)*cos(x5-x6);
           }}}
}
///////////////////////////////////
#endif

#ifndef _WIN64
__declspec(naked)
#endif
int callLib(int fn, ...)
{
	/**
	 * Calls a function that takes the same parameters as used
	 * to call this function with the exception of the first one, fn
	 * Example of called function: int myFunction(double a, double b) { return a + b; }
	 */

	/**
	 * Use all data in previous function's frame (including all arguments passed
	 * to the current function) as arguments to the function fn. Actually,
	 * not all data, but all data excluding last argument (fn).
	 *
	 * Then, call function fn and remove added arguments.
	 *
	 * It is assumed that called function follows CDECL (or STDCALL) conventions, in particular
	 * it preserves register ebx.
	 */

#ifndef _WIN64
	__asm {
        ; prolog
        push ebp            ; save must-save registers
        ;;push ebx          ; not used
        push esi
        push edi
        mov ebp, esp        ; our new stack frame

        dec ecx             ; upon entry, ecx contains # of actual args (including fn)
        shl ecx, 3          ; subtract the fn argument and convert to (worst case) arg byte count
        lea esi, [ebp+5*4]	; skip 5 dwords (pushed edi, esi, ebp, eip and first argument (fn))

		sub esp, ecx		; allocate space on stack
		mov edi, esp		; destination: end of added space in stack
		shr	ecx, 2			; will be moving dwords, so divide count by 4
		rep movs dword ptr [edi]	; copy argument list, excluding fn
		call dword ptr [ebp+4*4] ; call the function

		;;;add esp, XXX		; remove arguments (but STDCALL may have already removed them!!!)

        ; epilog
        mov esp, ebp        ; restore our stack frame
        pop edi             ; restore callee-save registers
        pop esi
        ;;pop ebx           ; wasn't used
        pop ebp             ; restore caller's stack frame
        ret                 ; caller will pop actual args

	}
#endif

}


#ifndef _WIN64
__declspec(naked)
#endif
int __cdecl getCpuReg(int nr)
{
    // Get value of specified CPU register:
    // nr: 0 = edi, 1 = esi, 2 = ebp, 3 = esp, 4 = ebx, 5 = edx, 6 = ecx, 7 = eax, 8 = eip
#ifndef _WIN64
    __asm {                 ;        +0    1    2        3    4    5    6    7    8    9
        pushad              ; esp -> edi, esi, ebp, old esp, ebx, edx, ecx, eax, eip, nr
        add dword ptr [esp+3*4], 2*4  ; correct saved esp for push nr, push eip
        mov eax, [esp+9*4]  ; eax <- nr
        and eax, 0x0f       ; prevent GPF on out-of-range arg.
        mov eax, [esp+eax*4]    ; load desired register
        add esp, 8*4        ; restore esp
        ret
    }
#endif
}

#ifndef _WIN64
__declspec(naked)
#endif
void __cdecl finit(void)
{
#ifndef _WIN64
    __asm {
        finit               ; init cw to 0x037f, sw to 0, tag word to 0xffff, rest to 0
        ret
    }
#endif
}

#ifndef _WIN64
__declspec(naked)
#endif
int __cdecl fstsw(void)
{
#ifndef _WIN64
    __asm {
        xor eax, eax        ; clear 32-bit eax
        fstsw ax            ; return 16-bit status word in ax
        ret
    }
#endif
}

#ifndef _WIN64
__declspec(naked)
#endif
int __cdecl fstcw(void)
{
#ifndef _WIN64
    __asm {
        push 0              ; clear a dword on the stack
        fstcw word ptr[esp] ; store control word in low 16 bits or dword
        pop eax             ; pop zero-extended control word into eax
        ret
    }
#endif
}

#ifndef _WIN64
__declspec(naked)
#endif
void __cdecl fldcw(int cw)
{
#ifndef _WIN64
    __asm {
        fldcw word ptr[esp+4]   ; load control word from low 16 bits of argument
        ret
    }
#endif
}

// restore "no return value" warnings
#ifdef _WIN64
    #pragma warning (default: 4716)
#else
    #pragma warning (default: 4035)
#endif


/****************** End DLL Access Routines *************************/

//#endif //#if 0


int setPreviewDragMode (int mode)
{
    //0
    //1 = ZeroDrag //Default!
    //2 = OutputDrag
    //3 = Zero + Output
    
    if (mode==0){
        gParams->flags &= ~AFH_ZERO_OUTPUT_DRAG;
        gParams->flags &= ~AFH_DRAG_FILTERED_OUTPUT;
		//gParams->flags |= AFH_QUICK_TOGGLE;
    } else if (mode==1){
        gParams->flags |= AFH_ZERO_OUTPUT_DRAG;
        gParams->flags &= ~AFH_DRAG_FILTERED_OUTPUT;
		//gParams->flags |= AFH_QUICK_TOGGLE;
    } else if (mode==2){
        gParams->flags &= ~AFH_ZERO_OUTPUT_DRAG;
        gParams->flags |= AFH_DRAG_FILTERED_OUTPUT;
		//gParams->flags |= AFH_QUICK_TOGGLE;
    } else if (mode==3){
        gParams->flags |= AFH_ZERO_OUTPUT_DRAG;
        gParams->flags |= AFH_DRAG_FILTERED_OUTPUT;
		//gParams->flags |= AFH_QUICK_TOGGLE;

	}/*else if (mode==4){
        gParams->flags &= ~AFH_ZERO_OUTPUT_DRAG;
        gParams->flags &= ~AFH_DRAG_FILTERED_OUTPUT;
		gParams->flags &= ~AFH_QUICK_TOGGLE;
    } else if (mode==5){
        gParams->flags |= AFH_ZERO_OUTPUT_DRAG;
        gParams->flags &= ~AFH_DRAG_FILTERED_OUTPUT;
		gParams->flags &= ~AFH_QUICK_TOGGLE;
    } else if (mode==6){
        gParams->flags &= ~AFH_ZERO_OUTPUT_DRAG;
        gParams->flags |= AFH_DRAG_FILTERED_OUTPUT;
		gParams->flags &= ~AFH_QUICK_TOGGLE;
    } else if (mode==7){
        gParams->flags |= AFH_ZERO_OUTPUT_DRAG;
        gParams->flags |= AFH_DRAG_FILTERED_OUTPUT;
		gParams->flags &= ~AFH_QUICK_TOGGLE;
    }*/

    return true;
    
}



int linearInterpolate (int v1,int v2, double x)
{

    return  (int)(v1*(1.0-x) + v2*x);
}

int cosineInterpolate (int v1,int v2, double x)
{

    double ft = x * 3.1415927;
	double f = (1.0 - cos(ft)) * 0.5;

	return  (int) (v1*(1.0-f) + v2*f);

}

int cubicInterpolate (int v0,int v1,int v2,int v3,double x)
{

    int P = (v3 - v2) - (v0 - v1);
	int Q = (v0 - v1) - P;
	int R = v2 - v0;
	int S = v1;

    return (int)(P*x*x*x + Q*x*x + R*x + S);

}


int hermiteInterpolate (int T1,int P1,int P2,int T2, double s)
{
  double s2 = s*s;
  double s3 = s2*s;

  double h1 =  2*s3 - 3*s2 + 1;          // calculate basis function 1
  double h2 = -2*s3 + 3*s2;              // calculate basis function 2
  double h3 =   s3 - 2*s2 + s;         // calculate basis function 3
  double h4 =   s3 -  s2;              // calculate basis function 4

  return (int)(h1*P1 +                  // multiply and sum all funtions
             h2*P2 +                    // together to build the interpolated
             h3*T1 +                    // point along the curve.
             h4*T2);
}


int getImageTitle (char * n) //LPSTR
{

#ifdef APP

	if (app.strInputFiles+app.inputIndex*(MAX_PATH+1) != NULL)
		strcpy(n,app.strInputFiles+app.inputIndex*(MAX_PATH+1));

	return true;

#else

    GetPropertyProc pGPP;
    OSErr err;
    intptr_t simpleProperty;
    Handle complexProperty;
    int success = false;


    pGPP = gStuff->getPropertyObsolete;
    if (gStuff->propertyProcs) {
        if (gStuff->propertyProcs->propertyProcsVersion >= kCurrentPropertyProcsVersion &&
            gStuff->propertyProcs->numPropertyProcs >= 1 &&
            gStuff->propertyProcs->getPropertyProc) {
            //prefer this version of getPropertyProc over the obsolete one
            pGPP = gStuff->propertyProcs->getPropertyProc;
        }
    } //propertyProcs != 0

    if (pGPP) {

        err = pGPP('8BIM', 'titl', 0, &simpleProperty, &complexProperty);
        if (err == noErr) {
            int size;
            LPSTR ptr;
            size = HostGetHandleSize(gStuff->handleProcs, complexProperty);
            
            if (size>0){
                size = min(size, 256);  //limit to avoid buff overflow
                ptr = HostLockHandle(gStuff->handleProcs, complexProperty, FALSE);
           
                if (ptr != NULL){
                
                    //strcpy (n, ptr); //Problem with PSP 7 when using strcpy
                    memcpy (n, ptr, size);
                    memset (n+size,0,1);
                
                    success = true;
                }

                HostUnlockHandle(gStuff->handleProcs, complexProperty);
                HostDisposeHandle(gStuff->handleProcs, complexProperty);

                if (success)
                    return true;
                else
                    return false;
            
            } 

        }

    }   

#endif

    return false;

}

int getEXIFSize (){

    int size = 0;

#ifndef APP

    GetPropertyProc pGPP;
    OSErr err;
    intptr_t simpleProperty;
    Handle complexProperty;
    int success = false;

    pGPP = gStuff->getPropertyObsolete;
    if (gStuff->propertyProcs) {
        if (gStuff->propertyProcs->propertyProcsVersion >= kCurrentPropertyProcsVersion &&
            gStuff->propertyProcs->numPropertyProcs >= 1 &&
            gStuff->propertyProcs->getPropertyProc) {
            //prefer this version of getPropertyProc over the obsolete one
            pGPP = gStuff->propertyProcs->getPropertyProc;
        }
    }

    if (pGPP) {
        err = pGPP('8BIM', 'EXIF', 0, &simpleProperty, &complexProperty);
        if (err == noErr) {
            size = HostGetHandleSize(gStuff->handleProcs, complexProperty);
        }
    } else {
        size = -1; //Not supported by host
    }

#endif

    return size;
}



int getEXIFData (int * buffer){

//Memory buffer n is allocated by function itself
//Return value is buffer size

#ifndef APP

    GetPropertyProc pGPP;
    OSErr err;
    intptr_t simpleProperty;
    Handle complexProperty;
    int success = false;

    pGPP = gStuff->getPropertyObsolete;
    if (gStuff->propertyProcs) {
        if (gStuff->propertyProcs->propertyProcsVersion >= kCurrentPropertyProcsVersion &&
            gStuff->propertyProcs->numPropertyProcs >= 1 &&
            gStuff->propertyProcs->getPropertyProc) {
            //prefer this version of getPropertyProc over the obsolete one
            pGPP = gStuff->propertyProcs->getPropertyProc;
        }
    }

    if (pGPP) {

        err = pGPP('8BIM', 'EXIF', 0, &simpleProperty, &complexProperty);
        if (err == noErr) {
            int size;
            LPSTR ptr;
            size = HostGetHandleSize(gStuff->handleProcs, complexProperty);
            
            if (size>0){
                ptr = HostLockHandle(gStuff->handleProcs, complexProperty, FALSE);

                if (ptr != NULL){
                    //buffer = malloc(size);
                    if (buffer){
                        memcpy (buffer, ptr, size);
                        success = true;
                    }
                }

                HostUnlockHandle(gStuff->handleProcs, complexProperty);
                HostDisposeHandle(gStuff->handleProcs, complexProperty);

                if (success) return true;//size;
            } 

        }

    }   

#endif

    return 0;
}



int destroyMenu(INT_PTR hMenu)
{
    // cdecl -> stdcall thunk
    return DestroyMenu((HMENU)hMenu);
}


int insertMenuItem (INT_PTR hMenu, int uItem, char* itemName, int fState, INT_PTR subMenu)
{
    return insertMenuItemEx (hMenu, uItem, itemName, fState, false, subMenu);
}


int insertMenuItemEx (INT_PTR hMenu, int uItem, char* itemName, int fState, int bullet, INT_PTR subMenu)
{

    MENUITEMINFO mii;

    if (uItem<1){ //Not smaller than 1
        //uItem=1;
		return false;
	}

    mii.cbSize = sizeof(MENUITEMINFO);

    
    mii.fMask = MIIM_STATE | MIIM_ID | MIIM_TYPE;
    if (subMenu != 0){ 
            mii.fMask = mii.fMask | MIIM_SUBMENU; 
            mii.hSubMenu = (HMENU) subMenu;
    }

    if (strcmp ((char *)itemName, "---") == 0)
        mii.fType = MFT_SEPARATOR;
    else 
        mii.fType = MFT_STRING;

    if (bullet){
        mii.hbmpChecked = NULL;
        mii.fType |= MFT_RADIOCHECK;
    }

    mii.fState = fState;
    mii.wID = uItem;
    mii.dwTypeData = (LPTSTR)itemName;
    mii.cch = (UINT)strlen ((char *)itemName);


    InsertMenuItem ( (HMENU)hMenu, uItem-1, MF_BYPOSITION, &mii);


    return true;
}


int trackPopupMenu (INT_PTR hMenu, int type, int x, int y, int style)
{
    RECT r;
    POINT pt;


	if (type==1){ //Display at cursor coordinates
        
        GetCursorPos(&pt);
        x = pt.x;
        y = pt.y;
        
    } else { //Use provided coordinates

		RECT rcClient, rcWind; 
    
		//Vista & 7 etc.
		if (getWindowsVersion()>=11) {
			x += 3;
			y += 2;
		}

		//Convert DBUs to Pixel
        x = HDBUsToPixels(x);
        y = VDBUsToPixels(y);

		//Try to calculate the title bar height
		if (GetWindowLong(fmc.hDlg, GWL_STYLE) & WS_CAPTION){
			GetClientRect(fmc.hDlg, &rcClient); 
			GetWindowRect(fmc.hDlg, &rcWind); 
			y += ((rcWind.bottom - rcWind.top) - (rcClient.bottom - rcClient.top)) - ((rcWind.right - rcWind.left) - (rcClient.right - rcClient.left)); 
			y += 1;
		}
    
        //Map to Screen coordinates
        GetWindowRect(fmc.hDlg, &r);
        x += r.left;
        y += r.top;
    }

    if (style==0) 
        style = TPM_TOPALIGN | TPM_LEFTALIGN | TPM_NONOTIFY | TPM_RETURNCMD | TPM_LEFTBUTTON;
    else
        style |= TPM_RETURNCMD;

    return TrackPopupMenuEx((HMENU)hMenu, (UINT)style, x, y, fmc.hDlg, NULL);

}


int setMenu (INT_PTR hMenu){


    return SetMenu (fmc.hDlg, (HMENU)hMenu);

}



int getCtlFocus (void)
{
    HWND focWindow = GetFocus();
    int focCtl = -1, n;

    if (focWindow == NULL) return -1;

    for (n = 0; n < N_CTLS; n++) {
        if (gParams->ctl[n].hCtl == focWindow){
            focCtl = n;
            break;
        }
    }

	if (n==N_CTLS) return -1;

	return focCtl;
}


int setCtlFocus (int n)
{
    HWND prevWindow;

    if ( (n < 0 || n >= N_CTLS || !gParams->ctl[n].inuse) && n!= CTL_PREVIEW) {
        return FALSE;
    }

    prevWindow = SetFocus (gParams->ctl[n].hCtl);

    if (prevWindow == NULL) return false;

	return true;
}

int checkCtlFocus (int n)
{
    
	if ( (n < 0 || n >= N_CTLS || !gParams->ctl[n].inuse) && n!= CTL_PREVIEW) {
        return FALSE;
    }

    if (GetFocus() == gParams->ctl[n].hCtl)
        return true;
    else
        return false;

}

int checkDialogFocus (void)
{
    
	if (GetForegroundWindow() == MyHDlg)
        return true;
    else
        return false;

}


    //// WARNING!! findFirstFile() may modify the lpFileName string!!! /////

INT_PTR findFirstFile (LPCTSTR lpFileName, LPSTR foundItem, int *dwFileAttributes)
{
    HANDLE hFindFile;
    WIN32_FIND_DATA FindFileData = {0};
    
    //Check if no file pattern and add one
    if (strcmp(lpFileName+strlen(lpFileName)-1,"\\")==0)
        strncat ((char *)lpFileName, "*", 1);//strncat ((const char *)lpFileName, "*", 1);

    hFindFile = FindFirstFile(lpFileName, &FindFileData);

    if (hFindFile != INVALID_HANDLE_VALUE){
        strcpy (foundItem, FindFileData.cFileName);//Pass back item
        *dwFileAttributes = (int) FindFileData.dwFileAttributes;
    }

    return (INT_PTR)hFindFile;

}


int findNextFile (INT_PTR hFindFile, LPSTR foundItem, int *dwFileAttributes)
{
    BOOL val;
    WIN32_FIND_DATA FindFileData = {0};
 
    val = FindNextFile ((HANDLE) hFindFile, &FindFileData);

    if (val!=0){
        strcpy (foundItem, FindFileData.cFileName);//Pass back item
        *dwFileAttributes = (int) FindFileData.dwFileAttributes;
    }

    return (int) val;

}

BOOL findClose(INT_PTR hFindFile)
{
    // cdecl -> strdcall thunk
    return FindClose((HANDLE)hFindFile);
}


int getLocaleInfo(LCID Locale,
                            LCTYPE LCType,
                            LPSTR  lpLCData,
                            int    cchData)
{
    // cdecl -> stdcall thunk
    return GetLocaleInfo(Locale, LCType, lpLCData, cchData);
}



int getSpecialFolder (int val, char * str)
{

	LPITEMIDLIST ppidl; //LPITEMIDLIST 

	if (SHGetSpecialFolderLocation (fmc.hDlg, val, &ppidl) == NOERROR) {
			if (SHGetPathFromIDList(ppidl, (char *)str))
				return true;
			else
				return false;
	} else {

		return false;
	}

}; 


int setCtlStyle(int n, int flags)
{
    
    if ( (n < 0 || n >= N_CTLS || !gParams->ctl[n].inuse) ) {
        return FALSE;
    }
    
    gParams->ctl[n].style |= flags;

	SetWindowLong(gParams->ctl[n].hCtl, GWL_STYLE,
        GetWindowLong(gParams->ctl[n].hCtl, GWL_STYLE) | flags);
    //use SWP_FRAMECHANGED to force nonclient area update...
    return SetWindowPos(gParams->ctl[n].hCtl, NULL,
                        0, 0, 0, 0,
                        SWP_FRAMECHANGED|SWP_NOZORDER|SWP_NOACTIVATE|SWP_NOSIZE|SWP_NOMOVE);
} /*fm_setCtlStyle*/

int clearCtlStyle(int n, int flags)
{

    if ( (n < 0 || n >= N_CTLS || !gParams->ctl[n].inuse) ) {
        return FALSE;
    }

    gParams->ctl[n].style &= ~flags;
	
	SetWindowLong(gParams->ctl[n].hCtl, GWL_STYLE,
        GetWindowLong(gParams->ctl[n].hCtl, GWL_STYLE) & ~flags);
    //use SWP_FRAMECHANGED to force nonclient area update...
    return SetWindowPos(gParams->ctl[n].hCtl, NULL,
                        0, 0, 0, 0,
                        SWP_FRAMECHANGED|SWP_NOZORDER|SWP_NOACTIVATE|SWP_NOSIZE|SWP_NOMOVE);
} /*fm_clearCtlStyle*/

int setCtlStyleEx(int n, int flags)
{
    
    HWND hCtl;

    if ( (n < 0 || n >= N_CTLS || !gParams->ctl[n].inuse) ) { //&& n!= CTL_PREVIEW
        return FALSE;
    }
    
    gParams->ctl[n].styleEx |= flags;

    //if (n == CTL_PREVIEW)
    //    hCtl = GetDlgItem(fmc.hDlg, 101);
    //else
        hCtl = gParams->ctl[n].hCtl;
	
	SetWindowLong(hCtl, GWL_EXSTYLE,
        GetWindowLong(hCtl, GWL_EXSTYLE) | flags);

	if (gParams->ctl[n].ctlclass == CC_LISTBAR){
		setCtlPos(n, -1, -1, -1, gParams->ctl[n].height); //Avoid collapsing
	}

    //use SWP_FRAMECHANGED to force nonclient area update...
    return SetWindowPos(hCtl, NULL,
                        0, 0, 0, 0,
                        SWP_FRAMECHANGED|SWP_NOZORDER|SWP_NOACTIVATE|SWP_NOSIZE|SWP_NOMOVE);
} /*fm_setCtlStyleEx*/

int clearCtlStyleEx(int n, int flags)
{
    
    if ( (n < 0 || n >= N_CTLS || !gParams->ctl[n].inuse) ) {
        return FALSE;
    }
    
    gParams->ctl[n].styleEx &= ~flags;

	SetWindowLong(gParams->ctl[n].hCtl, GWL_EXSTYLE,
        GetWindowLong(gParams->ctl[n].hCtl, GWL_EXSTYLE) & ~flags);
    //use SWP_FRAMECHANGED to force nonclient area update...
    return SetWindowPos(gParams->ctl[n].hCtl, NULL,
                        0, 0, 0, 0,
                        SWP_FRAMECHANGED|SWP_NOZORDER|SWP_NOACTIVATE|SWP_NOSIZE|SWP_NOMOVE);
} /*fm_clearCtlStyleEx*/



int setCtlBuddyStyle(int n, int buddy, int flags)
{
    HWND hBuddy;

    if ( (n < 0 || n >= N_CTLS || !gParams->ctl[n].inuse) ) {
        return FALSE;
    }

	if (buddy==1){
		hBuddy = gParams->ctl[n].hBuddy1;
		if (hBuddy==NULL) return false;
        // FIX by Ognen Genchev
        // the OR condition is removed so it can directly assign flags to the structure member
        gParams->ctl[n].buddy1Style = flags;
        // end
    } else {
        hBuddy = gParams->ctl[n].hBuddy2;
        if (hBuddy==NULL) return false;
        // FIX by Ognen Genchev
        // the OR condition is removed so it can directly assign flags to the structure member
        gParams->ctl[n].buddy2Style = flags;
        // end
    }
	

	SetWindowLong(hBuddy, GWL_STYLE,
        GetWindowLong(hBuddy, GWL_STYLE) | flags);
    //use SWP_FRAMECHANGED to force nonclient area update...
    return SetWindowPos(hBuddy, NULL,
                        0, 0, 0, 0,
                        SWP_FRAMECHANGED|SWP_NOZORDER|SWP_NOACTIVATE|SWP_NOSIZE|SWP_NOMOVE);
} /*fm_setCtlBuddyStyle*/

int clearCtlBuddyStyle(int n, int buddy, int flags)
{
    HWND hBuddy;

    if ( (n < 0 || n >= N_CTLS || !gParams->ctl[n].inuse) ) {
        return FALSE;
    }

	if (buddy==1){
		hBuddy = gParams->ctl[n].hBuddy1;
		if (hBuddy==NULL) return false;
		gParams->ctl[n].buddy1Style &= ~flags;
	} else {
		hBuddy = gParams->ctl[n].hBuddy2;
		if (hBuddy==NULL) return false;
		gParams->ctl[n].buddy2Style &= ~flags;
	}
	

	SetWindowLong(hBuddy, GWL_STYLE,
        GetWindowLong(hBuddy, GWL_STYLE) & ~flags);
    //use SWP_FRAMECHANGED to force nonclient area update...
    return SetWindowPos(hBuddy, NULL,
                        0, 0, 0, 0,
                        SWP_FRAMECHANGED|SWP_NOZORDER|SWP_NOACTIVATE|SWP_NOSIZE|SWP_NOMOVE);
} /*fm_clearBuddyCtlStyle*/


int setCtlBuddyStyleEx(int n, int buddy, int flags)
{
    HWND hBuddy;

    if ( (n < 0 || n >= N_CTLS || !gParams->ctl[n].inuse) ) {
        return FALSE;
    }

	if (buddy==1){
		hBuddy = gParams->ctl[n].hBuddy1;
		if (hBuddy==NULL) return false;
        // FIX by Ognen Genchev
        // the OR condition is removed so it can directly assign flags to the structure member
        gParams->ctl[n].buddy1StyleEx = flags;
        // end
    } else {
        hBuddy = gParams->ctl[n].hBuddy2;
        if (hBuddy==NULL) return false;
        // FIX by Ognen Genchev
        // the OR condition is removed so it can directly assign flags to the structure member
        gParams->ctl[n].buddy2StyleEx = flags;
        // end
    }
	

	SetWindowLong(hBuddy, GWL_EXSTYLE,
        GetWindowLong(hBuddy, GWL_EXSTYLE) | flags);
    //use SWP_FRAMECHANGED to force nonclient area update...
    return SetWindowPos(hBuddy, NULL,
                        0, 0, 0, 0,
                        SWP_FRAMECHANGED|SWP_NOZORDER|SWP_NOACTIVATE|SWP_NOSIZE|SWP_NOMOVE);
} /*fm_setCtlBuddyStyleEx*/

int clearCtlBuddyStyleEx(int n, int buddy, int flags)
{
    HWND hBuddy;

    if ( (n < 0 || n >= N_CTLS || !gParams->ctl[n].inuse) ) {
        return FALSE;
    }

	if (buddy==1){
		hBuddy = gParams->ctl[n].hBuddy1;
		if (hBuddy==NULL) return false;
		gParams->ctl[n].buddy1StyleEx &= ~flags;
	} else {
		hBuddy = gParams->ctl[n].hBuddy2;
		if (hBuddy==NULL) return false;
		gParams->ctl[n].buddy2StyleEx &= ~flags;
	}
	

	SetWindowLong(hBuddy, GWL_EXSTYLE,
        GetWindowLong(hBuddy, GWL_EXSTYLE) & ~flags);
    //use SWP_FRAMECHANGED to force nonclient area update...
    return SetWindowPos(hBuddy, NULL,
                        0, 0, 0, 0,
                        SWP_FRAMECHANGED|SWP_NOZORDER|SWP_NOACTIVATE|SWP_NOSIZE|SWP_NOMOVE);
} /*fm_clearCtlBuddyStyleEx*/


int setCtlBuddyFontColor(int n, int color)
{
    if (n < 0 || n >= N_CTLS || !gParams->ctl[n].inuse) {
        return -1;
    }

	gParams->ctl[n].textColor = color;
	//Kludge to force a window update...
	//return setCtlText(n, (char *)gParams->ctl[n].label);
	return true;
}

int setCtlBuddyColor(int n, int color)
{
    if (n < 0 || n >= N_CTLS || !gParams->ctl[n].inuse) {
        return -1;
    }

	gParams->ctl[n].buddyBkColor = color;
	//Kludge to force a window update...
	//return setCtlText(n, (char *)gParams->ctl[n].label);
	return true;
}

int setCtlBuddyMaxSize(int n, int maxwidth, int maxheight)
{
    if (n < 0 || n >= N_CTLS || !gParams->ctl[n].inuse) {
        return -1;
    }

	gParams->ctl[n].sb.maxw2 = HDBUsToPixels(maxwidth);
	gParams->ctl[n].sb.maxh2 = VDBUsToPixels(maxheight);

	return true;
}

int setCtlEditColor(int n, int color)
{
    if (n < 0 || n >= N_CTLS || !gParams->ctl[n].inuse) {
        return -1;
    }

	gParams->ctl[n].editColor = color;
	//Kludge to force a window update...
	//return setCtlText(n, (char *)gParams->ctl[n].label);
	return true;
}

int setCtlEditTextColor(int n, int color)
{
    if (n < 0 || n >= N_CTLS || !gParams->ctl[n].inuse) {
        return -1;
    }

	gParams->ctl[n].editTextColor = color;
	//Kludge to force a window update...
	//return setCtlText(n, (char *)gParams->ctl[n].label);
	return true;
}


int getSysColor(int n)
{	
	return (int)GetSysColor(n);
}

int getSysMem(int n)
{	
	int val;

	//Win 95, 98, ME, NT
//	if (getWindowsVersion() <= 4){
	
		MEMORYSTATUS statex;

	  	
	  	statex.dwLength = sizeof (statex);
	  	GlobalMemoryStatus (&statex);
	  	
		switch (n){
			case 0:
				val = statex.dwMemoryLoad; // percent of memory in use 	
				break;
			case 1:
				val = (int)statex.dwTotalPhys / 1024;     // bytes of physical memory 
				break; 
			case 2:
				val = (int)statex.dwAvailPhys / 1024;     // free physical memory bytes 	
				break; 
			case 3:
				val = (int)statex.dwTotalPageFile / 1024; // bytes of paging file 
				break; 
			case 4:
				val = (int)statex.dwAvailPageFile / 1024; // free bytes of paging file 
				break; 
			case 5:
				val = (int)statex.dwTotalVirtual / 1024;  // user bytes of address space 
				break; 
			case 6:
				val = (int)statex.dwAvailVirtual / 1024;  // free user bytes 
				break; 
			
		}
	
	//Win 2000, XP, Vista	
/*	} else {
		
		MEMORYSTATUSEX statex;


  		statex.dwLength = sizeof (statex);
  		GlobalMemoryStatusEx (&statex);

		switch (n){
			case 0:
				val = statex.dwMemoryLoad; // percent of memory in use 	
				break;
			case 1:
				val = (int)(statex.ullTotalPhys / 1024);     // bytes of physical memory 
				break; 
			case 2:
				val = (int)(statex.ullAvailPhys / 1024);     // free physical memory bytes 	
				break; 
			case 3:
				val = (int)(statex.ullTotalPageFile / 1024); // bytes of paging file 
				break; 
			case 4:
				val = (int)(statex.ullAvailPageFile / 1024); // free bytes of paging file 
				break; 
			case 5:
				val = (int)(statex.ullTotalVirtual / 1024);  // user bytes of address space 
				break; 
			case 6:
				val = (int)(statex.ullAvailVirtual / 1024);  // free user bytes 
				break; 
			
		}
	}*/

	return val;
}
 
 
int checkArrayAlloc(int nr){

	int val;

	if (nr<0 || nr>99) val=0;
	else if (Array[nr]==NULL) val=0;
	else if (ArrayID[nr]!=0) val=2;
	else val=1;
	
	return val;
}


INT_PTR getArrayAddress(int nr){

	if (nr>=0 && nr<=99 && Array[nr] != NULL)
		return (INT_PTR)Array[nr];
	else
		return 0;
}

INT_PTR getBufferAddress(int nr){

	if (nr == 0) //Source Buffer
		return (INT_PTR)MyAdjustedInData;
	else if (nr == 1) //T1
		return (INT_PTR)fmc.tbuf;
	else if (nr == 2) //T2
		return (INT_PTR)fmc.t2buf;
	else if (nr == 3) //Output Buffer
		return (INT_PTR)gStuff->outData;
	else if (nr == 4) //T3
		return (INT_PTR)fmc.t3buf;
	else if (nr == 5) //T4
		return (INT_PTR)fmc.t4buf;
	else
		return 0;
}


int setCtlScripting (int n, int state)
{
    
    if ( (n < 0 || n >= N_CTLS || !gParams->ctl[n].inuse) ) {
        return FALSE;
    }

    if (state != 0) state = -1;
    gParams->ctl[n].scripted = state;

    return true;
}


int checkScriptVal(int n)
{
    
    if (n < 0 || n >= N_SCRIPTITEMS) {
        return false;
    }
#ifdef SCRIPTABLE
    return (int)ScriptParamUsed[n];
#else
    return 0;
#endif

}

int enableScriptVal(int n, int state)
{
    
    if (n < 0 || n >= N_SCRIPTITEMS) {
        return false;
    }

    if (state !=0 ) state = -1;
#ifdef SCRIPTABLE
    ScriptParamUsed[n] = state;
#endif

    return true;
}

int getScriptVal(int n)
{
    
    if (n < 0 || n >= N_SCRIPTITEMS) {
        return 0;
    }
#ifdef SCRIPTABLE
    return ScriptParam[n];
#else
    return 0;
#endif

}

int setScriptVal(int n, int val)
{
    
    if (n < 0 || n >= N_SCRIPTITEMS) {
        return false;
    }
#ifdef SCRIPTABLE
    ScriptParam[n] = val;
#endif
    return true;
}


INT_PTR getDialogHandle(void)
{
    return (INT_PTR)fmc.hDlg;
}

INT_PTR getCtlHandle(int n)
{
    if ( (n < 0 || n >= N_CTLS || !gParams->ctl[n].inuse) ) {
        return 0;
    }

    return (INT_PTR)gParams->ctl[n].hCtl;
}



#if IMGSOURCE6
	#include "../../ImgSource6.h"
#endif


#ifdef FMDIB
    //Martijn's setPixel functions
    #include "fmdib.h"
#elif defined(GDIPLUS)
	#include "graphicsplus_fml.h"
#endif











//*******************************************************



int cnvX(int k, int off, int d,
                   INT_PTR pGetf, int x, int y, int z)
{
    int i;
    int sum = 0;
    int n = k*2 + 1;

#if 0
    //check for valid indexing into cell[]...
    if (n + off > N_CELLS) {
        return -1;  //invalid
    }
    else if (n + off < 0) {
        return -1;  //invalid
    }
#endif

#if 0
    if (d == 0)
    {   // default divisor is sum of weights...
        for (i = 0; i < n; i++) {
            d += fmc.cell[(off + i) & (N_CELLS-1)];
        }
        if (d == 0)
            return -1; // give up if divisor still 0
    }
#endif

    for (i = 0; i < n; i++) {
        sum += fmc.cell[(off + i) & (N_CELLS-1)] * ((fmf3)pGetf)(x - k + i, y, z);
    }
    return sum / d;
} /*fm_cnvX*/

int cnvY(int k, int off, int d,
                   INT_PTR pGetf, int x, int y, int z)
{
    int i;
    int sum = 0;
    int n = k*2 + 1;

#if 0
    //check for valid indexing into cell[]...
    if (n + off > N_CELLS) {
        return -1;  //invalid
    }
    else if (n + off < 0) {
        return -1;  //invalid
    }
#endif

#if 0
    if (d == 0)
    {   // default divisor is sum of weights...
        for (i = 0; i < n; i++) {
            d += fmc.cell[(off + i) & (N_CELLS-1)];
        }
        if (d == 0)
            return -1; // give up if divisor still 0
    }
#endif

    for (i = 0; i < n; i++) {
        sum += fmc.cell[(off + i) & (N_CELLS-1)] * ((fmf3)pGetf)(x, y - k + i, z);
    }
    return sum / d;
} /*fm_cnvY*/



int fm_abort(void) {
    
    //RaiseException(STATUS_CONTROL_C_EXIT,         // exception code 
    //    0,                    // continuable exception 
    //    0, NULL);             // no arguments 

    gResult = userCanceledErr;

    return 0;  //no return expected
} //fm_abort



int testAbort(void) {
    if (fmc.doingProxy) {
        //check for any character typed...
        MSG msg;
        if (PeekMessage(
            &msg,   // pointer to structure for message
            NULL,   // handle to window (NULL => any window belonging to this thread)
            WM_KEYDOWN,     // first message
            WM_KEYDOWN,     // last message
            PM_REMOVE       // removal flags
           ))
        { 
            if (msg.wParam == VK_ESCAPE) {
#if 0
                // User pressed escape -- ask if he really wants
                // to abort.
                if (YesNo("Abort filter operation?") == IDYES)
                    return userCanceledErr;
                else
                    return 0;
#else
                // Unconditional abort.
                return userCanceledErr;
#endif
            }
            else if (msg.wParam == VK_PAUSE) {
                // User pressed pause -- ask if he wants
                // to continue or abort.
#if 0
                if (Warn("Filter operation paused...\n\n"
                         "Select OK to continue\n"
                         "or Cancel to abort.") == IDCANCEL)
#else
                if (msgBox(MB_ICONWARNING|MB_YESNO,
                           "Pause",//"FilterMeister Pause",
                           "Continue?") == IDNO)
#endif
                    return userCanceledErr;
                else
                    return 0;
            }
#if 0
            else if (msg.wParam == VK_APPS) {
                // Context menu key ==> unconditional abort...
                return userCanceledErr;
            }
#endif
            // Ignore any other key-down
            return 0;
        };
    }

#ifndef APP
    else {
        // Running filter on main image.
        if (TestAbort ())
        {
            gResult = userCanceledErr;
            //longjmp()
        }
    }
#else
    
    if (gResult == userCanceledErr) return userCanceledErr;

#endif

    return gResult;
} //fm_testAbort



void setProgressMain(int val){
#if SKIN
	if (isSkinActive() && isSkinCtl(CTL_PROGRESS)){
		gParams->ctl[CTL_PROGRESS].val = val;
		skinDraw(CTL_PROGRESS,1);
	} else 
#endif
		SendMessage(GetDlgItem(MyHDlg, IDC_PROGRESS1), PBM_SETPOS, val, 0);
}


int updateProgress(int n, int d) {

	static int lastn = -1;
	static int lastd = -1;

	//if (fmc.doingProxy) return 0;

	if (d <= 0) d = 1;

	//No unnecessary updates
	if (n==lastn && d==lastd) return gResult;
	lastn = n;
	lastd = d;


#ifndef APP
    if (fmc.doingProxy) {
#endif

    #ifdef APP

		if (app.doProgress){

            int n1 = n;
            int d1 = d;

            clock_t timer = clock()-app.progressStartTime;
            //char timeDisplay[64];

            //if (app.progressTotal>0)
            {
                int min = ((app.progressCurrent-1)*app.progressTaskTotal+(app.progressTaskCurrent-1)) *100 / (app.progressTotal*app.progressTaskTotal);
                int max = ((app.progressCurrent-1)*app.progressTaskTotal+app.progressTaskCurrent) *100 / (app.progressTotal*app.progressTaskTotal);

                n1 = min + (n*(max-min))/d ;
                d1 = 100;
            }

            if (!app.progressDisplayed) {
                if (timer>1000) {

					if (app.hProgress==NULL) displayProgressWindow(1);

					if (app.hProgress!=NULL){
						HWND hLabel1 = GetDlgItem(app.hProgress, 1001);
	                    
						ShowWindow(app.hProgress, SW_SHOW);
						app.progressDisplayed = true;
						SendMessage(GetDlgItem(app.hProgress, IDC_PROGRESS1), PBM_SETPOS, (n1*100)/d1, 0);
						
						//Update Label
						InvalidateRect(hLabel1, NULL, TRUE );
						UpdateWindow(hLabel1);
					}
                }
           } else {
               if (app.hProgress!=NULL) SendMessage(GetDlgItem(app.hProgress, IDC_PROGRESS1), PBM_SETPOS, (n1*100)/d1, 0);
				   
               //Also use main window progress bar
               if (!fmc.doingProxy && app.progressDisplayed) 
				   //SendMessage(GetDlgItem(MyHDlg, IDC_PROGRESS1), PBM_SETPOS, (n*100)/d, 0);
				   setProgressMain((n*100)/d);
           }
            
           /*if (app.progressDisplayed && n>0) {
                HWND hLabel2 = GetDlgItem(app.hProgress, 1006); 
                //Update Time
                sprintf (timeDisplay,"Time: %.1f of %d seconds",timer/1000.0, (int)((timer*d/n)/1000.0) );
                SetWindowText(hLabel2, timeDisplay);
           }*/

		} else
    #endif
			//SendMessage(GetDlgItem(MyHDlg, IDC_PROGRESS1), PBM_SETPOS, (n*100)/d, 0);
			setProgressMain((n*100)/d);


			gParams->ctl[CTL_PROGRESS].val = (n*100)/d; //update cached values for reading progress bar value with ctl()
			fmc.pre_ctl[CTL_PROGRESS] = (n*100)/d; //update cached values for reading progress bar value with ctl()
			return testAbort();

#ifndef APP
    }
    else {
        PIUpdateProgress(n, d);
        if (TestAbort ())
        {
            gResult = userCanceledErr;
            //longjmp()
        }
    }
#endif

    return gResult;

//#else

//    return true;

//#endif

} //fm_updateProgress




int updatePreview(int n) {
    //n is ignored for now ,,, it will be the id of the preview
    //control to be updated.
    if (fmc.doingProxy) {
        //update progress bar for proxy....
        // thread is tied up computing the filter, so
        // WM_PAINT doesn't get processed in message loop.
        UpdateProxy(MyHDlg, FALSE);
        // So update the proxy immediately!
        ShowOutputBuffer(globals, MyHDlg, FALSE /*fDragging*/);
        return 1;  //doingProxy
    }
    return 0;   //not doingProxy, so no update
} //fm_updatePreview

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
char *getFilterInstallDir(void)
{
    int i;
    DWORD ret = GetModuleFileName((HMODULE)hDllInstance,
                                  fmc.filterInstallDir,
                                  sizeof(fmc.filterInstallDir));
    // Locate the final '\', if any...
    for (i = ret - 1; i >= 0 && fmc.filterInstallDir[i] != '\\'; --i)
        ;
    // Truncate string after final '\' (or set to null string if none).
    fmc.filterInstallDir[i+1] = '\0';
    return fmc.filterInstallDir;
} /*fm_getFilterInstallDir*/

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
char *appendEllipsis(const char *s)
{
    THREAD_LOCAL static char f[512];        //Warning: static!!!!

    strncpy(f, s, sizeof(f)-4); //leave room for ellipsis
    f[sizeof(f)-4] = '\0';      //ensure f is null-terminated.
    return strcat(f, "...");
} /*fm_appendEllipsis*/

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
char *stripEllipsis(const char *s)
{
    THREAD_LOCAL static char f[512];        //Warning: static!!!! NOT THREAD-SAFE!!
    int n = (int)strlen(s);
    if (n >= 3 && strcmp(&s[n-3], "...") == 0) {
        // strip ellipsis from end of s
        n -= 3;
    }
    n = min(n, sizeof(f)-1);    //truncate to fit in f if necessary
    strncpy(f, s, n);           //copy n chars to f
    f[n] = '\0';                //add terminating null
    return f;
} /*fm_stripEllipsis*/

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

char *formatString(const char *s)
{
    THREAD_LOCAL static char f[1024];        //Warning: static!!!! NOT THREAD-SAFE!!
    unsigned int cbLeft = sizeof(f) - 1;
    unsigned int cbCopied;
	int n;
    char *p = f;
    char *q;
    char buffer[32];

    while (*s) {
        if (*s == '!') {
            s++;
            switch (*s) {
            case 'A':
                q = fmc.filterAuthorText;
                break;
            case 'a':
                q = gParams->filterAboutText;
                break;
            case 'C':
                q = fmc.filterCategoryText;
                break;
            case 'c':
                q = fmc.filterCopyrightText;
                break;
            case 'D':
                q = fmc.filterDescriptionText;
                break;
            case 'F':
                q = fmc.filterFilenameText;
                break;
            case 'f':
                q = fmc.filterCaseText;
                break;
            case 'H':
                q = fmc.filterHostText;
                break;
            case 'h':
                q = _itoa(fmc.imageHeight, buffer, 10);
                break;
            case 'M':
                q = fmc.filterImageModeText;
                break;
            case 'm':
                q = _itoa(fmc.imageMode, buffer, 10);
                break;
            case 'O':
                q = fmc.filterOrganizationText;
                break;
            case 'T':
                q = fmc.filterTitleText;
                break;
            case 't':
                q = stripEllipsis(fmc.filterTitleText);
                break;
            case 'U':
                q = fmc.filterURLText;
                break;
            case 'V':
                q = fmc.filterVersionText;
                break;
            case 'w':
                q = _itoa(fmc.imageWidth, buffer, 10);
                break;
            case 'Y':
                {   time_t tm = time(NULL);
                    q = _itoa(localtime(&tm)->tm_year + 1900, buffer, 10);
                }
                break;
            case 'z':
                q = _itoa(fmc.zoomFactor, buffer, 10);
                break;
            case '!':
                q = "!";
                break;
            default:
                //copy a '!' to the formatted string
                q = "!";
                s--;    //back up one char to rescan the char after the '!'
                break;
            } //switch
            //copy the portion of the interpolated string
            //that will fit in the formatted string...
            cbCopied = (unsigned int)min(cbLeft, strlen(q));
            strncpy(p, q, cbCopied);
            p += cbCopied;
            cbLeft -= cbCopied;
        }
		else if (*s == '&') {
			//possible HTML entity...

			//define a macro to compare against a named entity
			#define ENTITY(entity, subst) \
				if (!strncmp(s, (entity), n = sizeof(entity)-1)) {\
					q = (subst); goto subst_entity;\
				}

            /* Cases are sorted by (first char of) entity name for faster lookup */
            switch (s[1]) {
			case '#':
				/* Numeric character reference */
				//we don't accept UNICODE numeric entities above 0x00ff for now...
				#define MAX_NUMERIC_ENTITY_VAL	255
				{
					unsigned int iVal = 0;
					const char *s2 = s + 2; // point past '#'
					n = 2;	// chars eaten so far
					if (*s2 == 'x' || *s2 == 'X') {
						/* hexadecimal numeric entity */
						s2++;
						n++;
						if (!isxdigit(*s2)) break;	// error, must be at least one digit
						while (isxdigit(*s2)) {
							iVal = iVal * 16 + ( isdigit(*s2) ? *s2 - '0' : (*s2 & 7) + 9 );
							// check for overflow
							if (iVal > MAX_NUMERIC_ENTITY_VAL) break;	// error, value too large
							s2++;
							n++;
						}//while hex digit
					}
					else {
						/* decimal numeric entity */
						if (!isdigit(*s2)) break;	// error, must be at least one digit
						while (isdigit(*s2)) {
							iVal = iVal * 10 + (*s2 - '0');
							// check for overflow
							if (iVal > MAX_NUMERIC_ENTITY_VAL) break;	// error, value too large
							s2++;
							n++;
						}//while decimal digit
					}
					if (*s2 != ';') break;	// Error, no final ';'
					n++;
					// disallow certain graphic values
					if (iVal < 0x00 ||		// NUL allowed?
						iVal == 0x7f ||		// 0x7f reserved for multibyte leader?
						iVal == 0x81 ||		// 0x81 reserved for multibyte????
						iVal > 0xff )		// only allow 8-bit codes? TRANSLATE other ISO-xxxx to Windows charset???
											// Could normalize by stripping leading 0's, then lookup e.g. "&#8084;"
											//  -- nahh, might as well lookup binary val directly.
					{
						iVal = 0x81;		// graphic to represent non-displayable coe?
					}
					q = (char *)&iVal;		// treat as NUL-terminated string. (tricky cast!!!)
				}
				cbCopied = 1;				// copy exactly 1 byte, which may be a NUL byte
				goto subst_entity2;
				#undef MAX_NUMERIC_ENTITY_VAL
				break;
            case '&':
			    ENTITY("&&",		"&");		// & (&& => verbatim &)
                break;
            case 'A':
			    ENTITY("&AElig;",	"\xC6");	// Æ
			    ENTITY("&Aacute;",	"\xC1");	// Á
			    ENTITY("&Acirc;",	"\xC2");	// Â
			    ENTITY("&Agrave;",	"\xC0");	// À
			    ENTITY("&Alpha;",	"A");		// A (Greek Alpha, use Latin A)
			    ENTITY("&Aring;",	"\xC5");	// Å
			    ENTITY("&Atilde;",	"\xC3");	// Ã
			    ENTITY("&Auml;",	"\xC4");	// Ä
                break;
            case 'B':
				ENTITY("&BEL;",		"\x07");	// BEL (ASCII Control code, not ISO)
				ENTITY("&BS;",		"\x08");	// BS (ASCII Control code, not ISO)
			    ENTITY("&Beta;",	"B");		// B (Greek Beta, use Latin B)
                break;
            case 'C':
				ENTITY("&CR;",		"\x0D");	// CR (ASCII Control code, not ISO)
				ENTITY("&CRLF;",	"\r\n");	// CR/LF (ASCII Control code sequence, not ISO)
			    ENTITY("&Ccedil;",	"\xC7");	// Ç
			    ENTITY("&Chi;",	    "X");		// X (Greek Chi, use Latin X)
			    ENTITY("&Colon;",	"::");		// :: (double colon)
                break;
            case 'D':
			    ENTITY("&Dagger;",	"\x87");	// ‡ (double dagger)
                break;
            case 'E':
			    ENTITY("&ESC;",		"\x1B");	// ESC (ASCII control code, not ISO entity)
			    ENTITY("&ETH;",		"\xD0");	// Ð
			    ENTITY("&Eacute;",	"\xC9");	// É
			    ENTITY("&Ecirc;",	"\xCA");	// Ê
			    ENTITY("&Egrave;",	"\xC8");	// È
			    ENTITY("&Epsilon;",	"E");		// E (Greek Epsilon, use Latin E)
			    ENTITY("&Eta;",	    "H");		// H (Greek Eta, use Latin H)
			    ENTITY("&Euml;",	"\xCB");	// Ë
			    ENTITY("&Exclam;",	"\x13");	// OEM (double exclamation mark !!)
                break;
			case 'F':
				ENTITY("&FF;",		"\x0c");	// FF (ASCII Control code, not ISO)
                break;
			case 'H':
				ENTITY("&HT;",		"\x09");	// HT (ASCII Control code, not ISO)
				break;
            case 'I':
			    ENTITY("&Iacute;",	"\xCD");	// Í
			    ENTITY("&Icirc;",	"\xCE");	// Î
			    ENTITY("&Igrave;",	"\xCC");	// Ì
			    ENTITY("&Iota;",	"I");		// I (Greek Iota, use Latin I)
			    ENTITY("&Iuml;",	"\xCF");	// Ï
                break;
            case 'K':
			    ENTITY("&Kappa;",	"K");		// K (Greek Kappa, use Latin K)
                break;
            case 'L':
				ENTITY("&LF;",		"\x0A");	// LF (ASCII Control code, not ISO)
                break;
            case 'M':
			    ENTITY("&Mu;",	    "M");		// M (Greek Mu, use Latin M)
                break;
            case 'N':
				///ENTITY("&NUL;",		"\x00");	// NUL (ASCII Control code, not ISO)	/// NFG!!!
			    ENTITY("&Ntilde;",	"\xD1");	// Ñ
			    ENTITY("&Nu;",	    "N");		// N (Greek Nu, use Latin N)
                break;
            case 'O':
			    ENTITY("&OElig;",	"\x8C");	// Œ (capital ligature OE)
			    ENTITY("&Oacute;",	"\xD3");	// Ó
			    ENTITY("&Ocirc;",	"\xD4");	// Ô
			    ENTITY("&Ograve;",	"\xD2");	// Ò
			    ENTITY("&Omicron;",	"O");		// O (Greek Omicron, use Latin O)
			    ENTITY("&Oslash;",	"\xD8");	// Ø
			    ENTITY("&Otilde;",	"\xD5");	// Õ
			    ENTITY("&Ouml;",	"\xD6");	// Ö
                break;
            case 'P':
			    ENTITY("&Prime;",	"\"");		// " (seconds, inches; use &quot;)
                break;
            case 'R':
			    ENTITY("&Rho;",	    "P");		// P (Greek Rho, use Latin P)
                break;
            case 'S':
			    ENTITY("&Scaron;",	"\x8A");	// Š (capital S with caron)
                break;
            case 'T':
			    ENTITY("&THORN;",	"\xDE");	// Þ
			    ENTITY("&Tau;",	    "T");		// T (Greek Tau, use Latin T)
                break;
            case 'U':
			    ENTITY("&Uacute;",	"\xDA");	// Ú
			    ENTITY("&Ucirc;",	"\xDB");	// Û
			    ENTITY("&Ugrave;",	"\xD9");	// Ù
			    ENTITY("&Upsilon;",	"Y");		// Y (Greek Upsilon, use Latin Y)
			    ENTITY("&Uuml;",	"\xDC");	// Ü
                break;
            case 'V':
				ENTITY("&VT;",		"\x0B");	// VT (ASCII Control code, not ISO)
                break;
            case 'Y':
			    ENTITY("&Yacute;",	"\xDD");	// Ý
			    ENTITY("&Yuml;",	"\x9F");	// Ÿ (capital Y with umlaut/diaeresis)
                break;
            case 'Z':
			    ENTITY("&Zcaron;",	"\x8E");	// Ž (capital Z with caron, not official?)
			    ENTITY("&Zeta;",	"Z");		// Z (Greek Zeta, use Latin Z)
                break;
            case 'a':
			    ENTITY("&aacute;",	"\xE1");	// á
			    ENTITY("&acirc;",	"\xE2");	// â
			    ENTITY("&acute;",	"\xB4");	// ´
			    ENTITY("&aelig;",	"\xE6");	// æ
			    ENTITY("&agrave;",	"\xE0");	// à
			    ENTITY("&amp;",		"\x26");	// &
			    ENTITY("&apos;",	"\x27");	// ' (not official HTML)
			    ENTITY("&aring;",	"\xE5");	// å
			    ENTITY("&atilde;",	"\xE3");	// ã
			    ENTITY("&auml;",	"\xE4");	// ä
                break;
            case 'b':
			    ENTITY("&bdquo;",	"\x84");	// „ (double low-9 quotation mark)
#if 0
			    ENTITY("&beamedsixteenthnotes;", "\x0E");   // OEM (beamed 16th notes)
#endif
			    ENTITY("&beta;",	"\xDF");	// ß (Greek beta, use &szlig; -- yecch!)
			    ENTITY("&blk14;",	"\x0F");	// OEM LIGHT SHADE  
			    ENTITY("&boxdl;",	"\x02");	// OEM BOX DRAWINGS LIGHT DOWN AND LEFT  
			    ENTITY("&boxdr;",	"\x01");	// OEM BOX DRAWINGS LIGHT DOWN AND RIGHT  
			    ENTITY("&boxh;",	"\x06");	// OEM BOX DRAWINGS LIGHT HORIZONTAL  
			    ENTITY("&boxhd;",	"\x16");	// OEM BOX DRAWINGS LIGHT HORIZONTAL AND DOWN
			    ENTITY("&boxhu;",	"\x15");	// OEM BOX DRAWINGS LIGHT HORIZONTAL AND UP
			    ENTITY("&boxul;",	"\x04");	// OEM BOX DRAWINGS LIGHT UP AND LEFT  
			    ENTITY("&boxur;",	"\x03");	// OEM BOX DRAWINGS LIGHT UP AND RIGHT  
			    ENTITY("&boxv;",	"\x05");	// OEM BOX DRAWINGS LIGHT VERTICAL  
			    ENTITY("&boxvh;",	"\x10");	// OEM BOX DRAWINGS LIGHT VERTICAL AND HORIZONTAL  
			    ENTITY("&boxvl;",	"\x17");	// OEM BOX DRAWINGS LIGHT VERTICAL AND LEFT  
			    ENTITY("&boxvr;",	"\x19");	// OEM BOX DRAWINGS LIGHT VERTICAL AND RIGHT  
			    ENTITY("&brvbar;",	"\xA6");	// ¦
			    ENTITY("&bull;",	"\x95");	// • (use • for bullet)
			    ENTITY("&bull2;",	"\x07");	// OEM (bullet style 2)
				ENTITY("&bull3;",	"\x7F");	//  (unofficial; narrow unfilled rectangle)
				ENTITY("&bull4;",	"\x81");	//  (unofficial; wide unfilled rectangle)
                break;
            case 'c':
			    ENTITY("&ccedil;",	"\xE7");	// ç
			    ENTITY("&cedil;",	"\xB8");	// ¸
			    ENTITY("&cent;",	"\xA2");	// ¢
			    ENTITY("&circ;",	"\x88");	// ˆ (modifier letter circumflex accent)
			    ENTITY("&colone;",	":=");		// := (colon equal)
			    ENTITY("&copy;",	"\xA9");	// ©
			    ENTITY("&crarr;",	"\x11\x04");	// OEM (subst. for carriage return arrow)
			    ENTITY("&curren;",	"\xA4");	// ¤
                break;
            case 'd':
			    ENTITY("&dagger;",	"\x86");	// † (dagger)
			    ENTITY("&deg;",		"\xB0");	// °
			    ENTITY("&divide;",	"\xF7");	// ÷
                break;
            case 'e':
			    ENTITY("&eacute;",	"\xE9");	// é
			    ENTITY("&ecirc;",	"\xEA");	// ê
			    ENTITY("&egrave;",	"\xE8");	// è
			    ENTITY("&empty;",	"\xD8");	// Ø (use capital O-slash for empty set)
			    ENTITY("&emsp;",	" ");		//   (em-space, use normal space)
			    ENTITY("&ensp;",	" ");		//   (en-space, use normal space)
			    ENTITY("&eth;",		"\xF0");	// ð
			    ENTITY("&euml;",	"\xEB");	// ë
			    ENTITY("&euro;",	"\x80");	// € (use € for euro sign)
                break;
            case 'f':
			    ENTITY("&female;",	"\x0C");	// OEM (female)
			    ENTITY("&fnof;",	"\x83");	// ƒ (f with hook)
			    ENTITY("&frac12;",	"\xBD");	// ½
			    ENTITY("&frac14;",	"\xBC");	// ¼
			    ENTITY("&frac34;",	"\xBE");	// ¾
			    ENTITY("&frasl;",	"/");		// / (use / for fraction slash)
                break;
            case 'g':
			    ENTITY("&ge;",		">=");		// >= (synthetic)
			    ENTITY("&gt;",		"\x3E");	// >
                break;
            case 'h':
			    ENTITY("&hellip;",	"\x85");	// … (horizontal ellipsis)
                break;
            case 'i':
			    ENTITY("&iacute;",	"\xED");	// í
			    ENTITY("&ibull2;",	"\x08");	// OEM (inverse bullet style 2)
			    ENTITY("&icirc;",	"\xEE");	// î
			    ENTITY("&iexcl;",	"\xA1");	// ¡
			    ENTITY("&igrave;",	"\xEC");	// ì
			    ENTITY("&image;",	"I");		// I imaginary part (blackletter I)
			    ENTITY("&iquest;",	"\xBF");	// ¿
			    ENTITY("&iuml;",	"\xEF");	// ï
                break;
            case 'l':
			    ENTITY("&laquo;",	"\xAB");	// «
			    ENTITY("&larr;",	"\x1B");	// OEM (left arrow)
			    ENTITY("&ldquo;",	"\x93");	// “ (left double quotation mark)
			    ENTITY("&le;",		"<=");		// <= (synthetic)
			    ENTITY("&lowast;",	"*");		// * (use asterisk for low asterisk)
			    ENTITY("&lsaquo;",	"\x8B");	// ‹ (single left angle quotation)
			    ENTITY("&lsquo;",	"\x91");	// ‘ (left single quotation mark)
			    ENTITY("&lt;",		"\x3C");	// <
			    ENTITY("&ltrif;",	"\x11");	// OEM (left-pointing triangle, filled)
                break;
            case 'm':
			    ENTITY("&macr;",	"\xAF");	// ¯ (spacing macron)
			    ENTITY("&male;",	"\x0B");	// OEM (male)
			    ENTITY("&mdash;",	"\x97");	// — (em dash)
			    ENTITY("&micro;",	"\xB5");	// µ
			    ENTITY("&middot;",	"\xB7");	// ·
			    ENTITY("&minus;",	"\x96");	// - (use en-dash for minus symbol)
			    ENTITY("&mu;",	    "\xB5");	// µ (mu, use &micro;)
                break;
            case 'n':
			    ENTITY("&nbsp;",	"\xA0");	//	 (no-break space)
			    ENTITY("&ndash;",	"\x96");	// – (en dash)
			    ENTITY("&ne;",		"!=");		// ? (not equal, use !=)
			    ENTITY("&not;",		"\xAC");	// ¬
			    ENTITY("&ntilde;",	"\xF1");	// ñ
                break;
            case 'o':
			    ENTITY("&oacute;",	"\xF3");	// ó
			    ENTITY("&ocirc;",	"\xF4");	// ô
			    ENTITY("&oelig;",	"\x9C");	// œ (small ligature oe)
			    ENTITY("&ograve;",	"\xF2");	// ò
			    ENTITY("&oline;",	"\xAF");	// ¯ (overline, use macron)
			    ENTITY("&omicron;",	"o");	    // o (Greek omicron, use Latin o)
			    ENTITY("&ordf;",	"\xAA");	// ª
			    ENTITY("&ordm;",	"\xBA");	// º
			    ENTITY("&oslash;",	"\xF8");	// ø
			    ENTITY("&otilde;",	"\xF5");	// õ
			    ENTITY("&ouml;",	"\xF6");	// ö
                break;
            case 'p':
			    ENTITY("&para;",	"\xB6");	// ¶
			    ENTITY("&permil;",	"\x89");	// ‰ (per mille)
			    ENTITY("&pilcrow;",	"\x14");	// OEM (pilcrow, unfilled paragaph mark)
			    ENTITY("&plusmn;",	"\xB1");	// ±
			    ENTITY("&pound;",	"\xA3");	// £
			    ENTITY("&prime;",	"'");		// ' (minutes, feet; use &apos;)
                break;
            case 'q':
			    ENTITY("&quot;",	"\x22");	// "
                break;
            case 'r':
			    ENTITY("&raquo;",	"\xBB");	// »
			    ENTITY("&rarr;",	"\x1A");	// OEM (right arrow)
			    ENTITY("&rdquo;",	"\x94");	// ” (right double quotation mark)
			    ENTITY("&real;",	"R");		// R real part (blackletter R)
			    ENTITY("&reg;",		"\xAE");	// ®
			    ENTITY("&rsaquo;",	"\x9B");	// › (single right angle quotation)
			    ENTITY("&rsquo;",	"\x92");	// ’ (right single quotation mark)
                break;
            case 's':
			    ENTITY("&sbquo;",	"\x82");	// ‚ (single low-9 quotation mark)
			    ENTITY("&scaron;",	"\x9A");	// š (small s with caron)
			    ENTITY("&sdot;",	"·");		// · (dot operator)
			    ENTITY("&sect;",	"\xA7");	// §
#if 0
			    ENTITY("&shy;",		"\xAD");	// ­ (soft hyphen, doesn't work in FM)
#else
			    ENTITY("&shy;",		"");		// ­ (soft hyphen doesn't break in FM, always use "")
#endif
			    ENTITY("&sim;",		"~");		// ~ (use tilde for 'similar to')
#if 0
				ENTITY("&sup0;",	"\xB0");	// ° (unofficial; use &deg; in MS Sans Serif, Courier, Courier New, Fixedsys, System, most fixed-width fonts)
#else
				ENTITY("&sup0;",	"\xBA");	// ° (unofficial; use &ordm; in Arial, Lucida Console, most propertional fonts.)
#endif
			    ENTITY("&sup1;",	"\xB9");	// ¹
			    ENTITY("&sup2;",	"\xB2");	// ²
			    ENTITY("&sup3;",	"\xB3");	// ³
			    ENTITY("&szlig;",	"\xDF");	// ß
                break;
            case 't':
			    ENTITY("&thinsp;",	" ");		//   (thin space, use normal space)
			    ENTITY("&thorn;",	"\xFE");	// þ
			    ENTITY("&tilde;",	"\x98");	// ˜ (small tilde)
			    ENTITY("&times;",	"\xD7");	// ×
#if 1
			    ENTITY("&trade;",	"\x99");	// ™ (use ™ for trademark)
#else
			    ENTITY("&trade;",	"(tm)");	// ™ (use (tm) for trademark)
#endif
			    ENTITY("&twonotes;", "\x0E");	// OEM (two beamed 16th notes)
                break;
            case 'u':
			    ENTITY("&uacute;",	"\xFA");	// ú
			    ENTITY("&uarr;",	"\x18");	// OEM (up arrow)
			    ENTITY("&ucirc;",	"\xFB");	// û
			    ENTITY("&ugrave;",	"\xF9");	// ù
			    ENTITY("&uml;",		"\xA8");	// ¨
			    ENTITY("&uuml;",	"\xFC");	// ü
                break;
            case 'v':
			    ENTITY("&varr;",	"\x12");	// OEM (up/down arrow; not ISO)
                break;
            case 'w':
			    ENTITY("&weierp;",	"P");		// P powerset (script P)
                break;
            case 'y':
			    ENTITY("&yacute;",	"\xFD");	// ý
			    ENTITY("&yen;",		"\xA5");	// ¥
			    ENTITY("&yuml;",	"\xFF");	// ÿ
                break;
            case 'z':
			    ENTITY("&zcaron;",	"\x9E");	// ž (small z with caron, not official?)
			    ENTITY("&zwj;",		"");		//  zero width joiner (no char)
			    ENTITY("&zwnj;",	"");		//  zero width non-joiner (no char)
                break;
            }//switch
			assert(*s == '&');
			// no entity match: copy the single '&'...
			q = "&"; n = 1;

subst_entity:
            //copy the portion of the entity string value
            //that will fit in the formatted string...
            cbCopied = (unsigned int)min(cbLeft, strlen(q));
subst_entity2:	//entry when cbCopied has already been computed (to copy NULs)
            memcpy(p, q, cbCopied); //use memcpy, not str(n)cpy, to copy internal NULs.
            p += cbCopied;
            cbLeft -= cbCopied;
			s += n - 1;
			#undef ENTITY
		}
        else if (cbLeft > 0) {
            //copy this character verbatim to the formatted string
            *p++ = *s;
            cbLeft--;
        }
        s++;
    } //while
    *p = '\0';  //terminate the formatted string
    return f;
} /*fm_formatString*/


void lockCtlUpdate (int state){

	ctlUpdateLocked = state;
}


int setCtlLabel(int n, char * iText){

    RECT    wRect;

    if (n < 0 || n >= N_CTLS || !gParams->ctl[n].inuse) {
        return FALSE;
    }

    if (gParams->ctl[n].ctlclass != CC_LISTBAR && gParams->ctl[n].ctlclass != CC_COMBOBOX) return false;

    if (iText) {
        strncpy(gParams->ctl[n].label2, (char *)iText, 32+1);
        gParams->ctl[n].label2[32] = '\0';   //ensure null-terminated.
    } else {
        gParams->ctl[n].label2[0] = '\0'; //No text string
    }


    {//scope
        //Resize the static text buddy to exactly fit the text,
        //to minimize problems with overlapping controls...
		SIZE  size = {60,14};
        RECT  r;
        HWND  hWnd = GetDlgItem(fmc.hDlg, IDC_BUDDY2+n);
        HDC   hDC = GetDC(hWnd);
        HFONT hOldFont = (HFONT)SelectObject(hDC, gParams->ctl[n].hFont);
        LPSTR p = formatString(gParams->ctl[n].label2);
        //CAUTION: p points to volatile storage (which will
		//be overwritten by calls to Info(), etc.)!!

        GetTextExtentPoint32(hDC, p, (int)strlen(p), &size);
		if (gParams->ctl[n].sb.maxw2 > 0 && size.cx > gParams->ctl[n].sb.maxw2)
			gParams->ctl[n].sb.w2 = gParams->ctl[n].sb.maxw2;
		else
			gParams->ctl[n].sb.w2 = size.cx;    //text width in pixels
		if (gParams->ctl[n].sb.maxh2 > 0 && size.cy > gParams->ctl[n].sb.maxh2)
			gParams->ctl[n].sb.h2 = gParams->ctl[n].sb.maxh2;
		else
			gParams->ctl[n].sb.h2 = size.cy;    //text height in pixels
        computeBuddy2Pos(n,
                         gParams->ctl[n].xPos,
                         gParams->ctl[n].yPos,
                         gParams->ctl[n].width,
                         gParams->ctl[n].height,
                         &r);
		        
		SetWindowPos(gParams->ctl[n].hBuddy2, NULL,
                     r.left,    //x
                     r.top,     //y
                     r.right,   //w
                     r.bottom,  //h
                     SWP_NOZORDER | SWP_NOACTIVATE | SWP_NOCOPYBITS);
        //Cleanup...
        SelectObject(hDC, hOldFont);
        ReleaseDC(hWnd, hDC);
        //Set the actual control text...
        SetWindowText(hWnd, p);

        //It seems a static text control does not erase the
        //background when it receives a WM_SETTEXT message
        //(maybe because we are handling WM_CTLCOLORSTATIC
        //and setting the background to transparent??), so
        //we brute-force it... (this may be overkill??)
		if (!ctlUpdateLocked){
			if (gParams->ctl[n].enabled){
				GetWindowRect(hWnd, &wRect); 
				ScreenToClient (fmc.hDlg, (LPPOINT)&wRect);
				ScreenToClient (fmc.hDlg, (LPPOINT)&(wRect.right));
				return RedrawWindow(
	#if 0
						//just updating the control window doesn't work :(
						hWnd,   // handle of control window
	#else
						fmc.hDlg,   // handle of entire dialog window
	#endif
	#if 1
						&wRect, // address of structure with update rectangle
	#else
						NULL,   // address of structure with update rectangle, NULL=>entire client area
	#endif
						NULL,   // handle of update region
						RDW_ERASE|RDW_INVALIDATE|RDW_ERASENOW|RDW_UPDATENOW     // array of redraw flags
				   );
			}
		}

    }//scope
   
    return true;

}


int setCtlTextW(int n, WCHAR * iText){

    if (n < 0 || n >= N_CTLS || !gParams->ctl[n].inuse) {
        return FALSE;
    }

    return SetWindowTextW(gParams->ctl[n].hCtl, iText);
}



/**********************************************************************/
/* setCtlText(n, "text")
/*
/*  Sets the text label or content for control n to the given text string.
/*
/**********************************************************************/
int setCtlText(int n, char * iText)
{
    // Could check nargs and pass to setCtlTextv() if nargs > 2 ????
    RECT    wRect;

    if (n < 0 || n >= N_CTLS || !gParams->ctl[n].inuse) {
        return FALSE;
    }

    if (iText) {
        strncpy(gParams->ctl[n].label, (char *)iText, MAX_LABEL_SIZE+1);
        gParams->ctl[n].label[MAX_LABEL_SIZE] = '\0';   //ensure null-terminated.
    }
    else {
        //No text string
        gParams->ctl[n].label[0] = '\0';
    }


#if SKIN
	if (isSkinActive() && isSkinCtl(n) && gParams->ctl[n].ctlclass != CC_STANDARD){
		if (ctlEnabledAs(n)) skinDraw(n,0);
		return true;
	}
#endif

    switch (gParams->ctl[n].ctlclass) {
	
	case CC_STATICTEXT:
    case CC_CHECKBOX:
    case CC_RADIOBUTTON:
        //SetWindowText(gParams->ctl[n].hCtl, formatString(gParams->ctl[n].label));
		SetWindowText(gParams->ctl[n].hCtl, formatString((char *)iText));

        if (gParams->ctl[n].bkColor == (COLORREF) -1) {
            //It seems these controls with a transparent
            //background color do not erase the background
            //when they receive a WM_SETTEXT message
            //(maybe because we are handling WM_CTLCOLOR*
            //and setting the background to transparent??), so
            //we brute-force it... (this may be overkill??)

            if (gParams->ctl[n].enabled){
			//if (ctlEnabledAs(n)){
                int retval;
		        GetWindowRect(gParams->ctl[n].hCtl, &wRect); 
		        ScreenToClient (fmc.hDlg, (LPPOINT)&wRect);
		        ScreenToClient (fmc.hDlg, (LPPOINT)&(wRect.right));
                retval = RedrawWindow(
				        fmc.hDlg,   // handle of entire dialog window
				        &wRect, // address of structure with update rectangle
				        NULL,   // handle of update region
				        RDW_ERASE|RDW_INVALIDATE|RDW_ERASENOW|RDW_UPDATENOW // array of redraw flags
		           );
                return retval;
	        }
        }
        
        return TRUE;
        break;

    case CC_STANDARD:
	case CC_SLIDER:
        //specific to default scrollbars...
        {//scope
            //Resize the static text buddy to exactly fit the text,
            //to minimize problems with overlapping controls...
            SIZE  size = {60,14};
            RECT  r;
            HWND  hWnd = GetDlgItem(fmc.hDlg, IDC_BUDDY2+n);
            HDC   hDC = GetDC(hWnd);
            HFONT hOldFont = (HFONT)SelectObject(hDC, gParams->ctl[n].hFont);
            LPSTR p = formatString(gParams->ctl[n].label);
            //CAUTION: p points to volatile storage (which will
            //be overwritten by calls to Info(), etc.)!!
            GetTextExtentPoint32(hDC, p, (int)strlen(p), &size);
            if (gParams->ctl[n].sb.maxw2 > 0 && size.cx > gParams->ctl[n].sb.maxw2)
				gParams->ctl[n].sb.w2 = gParams->ctl[n].sb.maxw2;
			else
				gParams->ctl[n].sb.w2 = size.cx;    //text width in pixels
			if (gParams->ctl[n].sb.maxh2 > 0 && size.cy > gParams->ctl[n].sb.maxh2)
				gParams->ctl[n].sb.h2 = gParams->ctl[n].sb.maxh2;
			else
				gParams->ctl[n].sb.h2 = size.cy;    //text height in pixels
            computeBuddy2Pos(n,
                             gParams->ctl[n].xPos,
                             gParams->ctl[n].yPos,
                             gParams->ctl[n].width,
                             gParams->ctl[n].height,
                             &r);
            SetWindowPos(gParams->ctl[n].hBuddy2, NULL,
                         r.left,    //x
                         r.top,     //y
                         r.right,   //w
                         r.bottom,  //h
                         SWP_NOZORDER | SWP_NOACTIVATE | SWP_NOCOPYBITS);
            //Cleanup...
            SelectObject(hDC, hOldFont);
            ReleaseDC(hWnd, hDC);
            //Set the actual control text...
            SetWindowText(hWnd, p);

            //It seems a static text control does not erase the
            //background when it receives a WM_SETTEXT message
            //(maybe because we are handling WM_CTLCOLORSTATIC
            //and setting the background to transparent??), so
            //we brute-force it... (this may be overkill??)

			if (!ctlUpdateLocked){
				//if (gParams->ctl[n].enabled){
				if (ctlEnabledAs(n)){
					GetWindowRect(hWnd, &wRect); 
					ScreenToClient (fmc.hDlg, (LPPOINT)&wRect);
					ScreenToClient (fmc.hDlg, (LPPOINT)&(wRect.right));
					return RedrawWindow(
		#if 0
							//just updating the control window doesn't work :(
							hWnd,   // handle of control window
		#else
							fmc.hDlg,   // handle of entire dialog window
		#endif
		#if 1
							&wRect, // address of structure with update rectangle
		#else
							NULL,   // address of structure with update rectangle, NULL=>entire client area
		#endif
							NULL,   // handle of update region
							RDW_ERASE|RDW_INVALIDATE|RDW_ERASENOW|RDW_UPDATENOW     // array of redraw flags
					   );
				}
			}

        }//scope
        break;

    case CC_EDIT:
        //SetWindowText(gParams->ctl[n].hCtl, formatString(gParams->ctl[n].label));
		SetWindowText(gParams->ctl[n].hCtl, formatString((char *)iText));

        //for an Edit control, we need to call UpdateWindow() to force a
        // WM_PAINT if we want an immediate update
        if (gParams->ctl[n].enabled){
		//if (ctlEnabledAs(n)){
			return UpdateWindow(gParams->ctl[n].hCtl);
		} else
			return TRUE;
        break;

    case CC_COMBOBOX:
        SendMessage(gParams->ctl[n].hCtl, CB_RESETCONTENT, 0, 0);
		{
            //The following old code causes a problem when compiled as a 64bit plugin
			//Info("%d\n%s",*iText,iText);
            //if (*iText!=0) {
        #if 0      
                //#if _WIN64 || _MSC_VER >= 1400

                    int i;
			        char *a;
			        char copy[MAX_LABEL_SIZE+1];
                    strncpy(copy, iText, MAX_LABEL_SIZE+1);

                    /*
                    a = strtok(copy,"\n");
                    for (i=0;i>=0;i++){
				        SendMessage(gParams->ctl[n].hCtl, CB_ADDSTRING, 0, (LPARAM)a); //formatString(a)
				        a = strtok(NULL,"\n");
				        if (a==0)break;
			        }*/

                    a = strtok(copy,"\n"); 
                    while(a!=NULL){
                        SendMessage(gParams->ctl[n].hCtl, CB_ADDSTRING, 0, (LPARAM)a); //formatString(a)
                        a=strtok(NULL,"\n");
                    }
        #endif
                //#else
                    char *p, *q, q_save;
                    char* copy;
    			    
                    copy = (char *)malloc(strlen(iText)+1);
                    strcpy(copy,iText);

                    //p = gParams->ctl[n].label;
			        //p = iText; //Allow combo box text that is longer than 1023 bytes 
                    p = copy; 
			        while (*p) {
                        //for (q = p; *q != '\0' && *q != '\n'; q++)
                        //    ;
                        q = p; while(*q != '\0' && *q != '\n') q++;

                        q_save = *q;
                        *q = '\0';
                        SendMessage(gParams->ctl[n].hCtl, CB_ADDSTRING, 0, (LPARAM)p); //formatString(p)
				        *q = q_save;
                        p = q;
                        if (*p) p++;
                    } //while 

                    free (copy);
    			
                //#endif
                
            //}
			
        }
        // set the control value to set the initial selection
        setCtlVal(n, gParams->ctl[n].val);
        break;

    case CC_LISTBOX:
    case CC_LISTBAR:
        SendMessage(gParams->ctl[n].hCtl, LB_RESETCONTENT, 0, 0);
        {
            int count=0;
    #if 0            
            //#if _WIN64 || _MSC_VER >= 1400

			    ///int i;
			    char *a;
			    char copy[65535]; //MAX_LABEL_SIZE+1
			    strncpy(copy, (char *)iText, 65535); //MAX_LABEL_SIZE+1

			    /*a = strtok(copy,"\n");
			    for (i=0;i>=0;i++){
				    SendMessage(gParams->ctl[n].hCtl, LB_ADDSTRING, 0, (LPARAM)a);//formatString(a)
				    a = strtok(NULL,"\n");
				    if (a==0)break;
                    count++;
			    }*/

                a = strtok(copy,"\n"); 
                while(a!=NULL){
                    SendMessage(gParams->ctl[n].hCtl, CB_ADDSTRING, 0, (LPARAM)a); //formatString(a)
                    a=strtok(NULL,"\n");
                    count++;
                }

             //#else
    #endif    
                char *p, *q, q_save;
                char* copy;
                int chars, maxchars=0;
                SIZE sz = {0,0};
                //char str[512];
                int pxsize=0; //i,
                //POINT pt;

                copy = (char *)malloc(strlen(iText)+1);
                strcpy(copy,iText);

                //p = gParams->ctl[n].label;
			    //p = (char *)iText; //Allow list box text that is longer than 1023 bytes 
                p = copy; 
                while (*p) {
                    //for (q = p; *q != '\0' && *q != '\n'; q++)
                    //    ;
                    q = p; while(*q != '\0' && *q != '\n') q++;

                    q_save = *q;
                    *q = '\0';
                    SendMessage(gParams->ctl[n].hCtl, LB_ADDSTRING, 0, (LPARAM)p); //(LPARAM)formatString(p));

                    if (gParams->ctl[n].style & WS_HSCROLL){
                        chars = (int)strlen(p);
                        if (chars>maxchars) {
                            //TEXTMETRIC tm; 
                            maxchars = chars;
                            //GetTextExtentPoint32(GetDC(gParams->ctl[n].hCtl),(LPCTSTR)p,chars,&sz);
                            
                            //Info ("%d",GetMapMode(GetDC(gParams->ctl[n].hCtl)));
                            //pt.x = sz.cx;
                            //pt.y = sz.cy;
                            //LPtoDP(GetDC(gParams->ctl[n].hCtl),&pt,1);
                            //GetTextMetrics(GetDC(gParams->ctl[n].hCtl), &tm); 
                            //sz.cx -= tm.tmOverhang;
                        }
                    }

                    *q = q_save;
                    p = q;
                    if (*p) p++;
                    count++;
                } //while

                if (gParams->ctl[n].style & WS_HSCROLL && maxchars>0){

                    TEXTMETRIC tm;
                    GetTextMetrics(GetDC(gParams->ctl[n].hCtl), &tm); 
                    SendMessage(gParams->ctl[n].hCtl, LB_SETHORIZONTALEXTENT, (WPARAM)(tm.tmAveCharWidth-2) * (maxchars+2), 0);
                    
                    //SendMessage(gParams->ctl[n].hCtl, LB_SETHORIZONTALEXTENT, (WPARAM)sz.cx, 0);
                }

                free(copy);
			
            //#endif
            

            if (gParams->ctl[n].ctlclass==CC_LISTBAR && count>0 && gParams->ctl[n].height < 20) SendMessage(gParams->ctl[n].hCtl, LB_SETCOLUMNWIDTH, HDBUsToPixels(gParams->ctl[n].width/count), 0);
			
        }
        // set the control value to set the initial selection
        setCtlVal(n, gParams->ctl[n].val);
        break;
	
	case CC_LISTVIEW:
		{
			int i;
			char *a;
			char copy[MAX_LABEL_SIZE+1];
			LVITEM lvi;

			ListView_DeleteAllItems(gParams->ctl[n].hCtl);
			ListView_RemoveAllGroups(gParams->ctl[n].hCtl);

			lvi.mask   = LVIF_TEXT; // | LVIF_IMAGE|LVIF_STATE ;
			lvi.iSubItem  = 0;
			
			strncpy(copy, (char *)iText, MAX_LABEL_SIZE+1);
			if (iText[0] == 0) return false; //Delete items only

			a = strtok(copy,"\n");
			for (i=0;i>=0;i++){
			  	
				lvi.iItem  = i;
				lvi.pszText  = (LPTSTR)a;
				ListView_InsertItem (gParams->ctl[n].hCtl, &lvi);
					
				a = strtok(NULL,"\n");
				if (a==0)break;
			}

		}
		break;

	case CC_TAB:
		{
			TCITEM tabItem; 
			int i;
			char *a;
			char copy[MAX_LABEL_SIZE+1];

			TabCtrl_DeleteAllItems (gParams->ctl[n].hCtl); 
			
			strncpy(copy, (char *)iText, MAX_LABEL_SIZE+1);

			tabItem.mask = TCIF_TEXT | TCIF_IMAGE; 
			tabItem.iImage = -1; 

			a = strtok(copy,"\n");
			for (i=0;i>=0;i++){
			  	
				tabItem.pszText = a; 
				TabCtrl_InsertItem(gParams->ctl[n].hCtl, i, &tabItem);

				a = strtok(NULL,"\n");
				if (a==0)break;
			}

#if SKIN
			//if (isSkinActive()) skinDraw(n,2); //Erase Background
#endif

		}	
		break;

	case CC_TOOLBAR:
		{
			int itemNumber = 0;
			TBBUTTON * tbButtons;
			char * a;
			int i;
			char copy[MAX_LABEL_SIZE+1];
			//SIZE size;

			
			if (strcmp(iText,"")!=0){

				int n_buttons = (int)SendMessage(gParams->ctl[n].hCtl,TB_BUTTONCOUNT,0,0);

				//Delete old buttons
				for (i=0;i<n_buttons;i++){
					SendMessage(gParams->ctl[n].hCtl,TB_DELETEBUTTON,0,0);
				}

				//Count items
				strncpy(copy, (char *)iText, MAX_LABEL_SIZE+1);
				a = strtok(copy,"\n");
				for (i=0;i>=0;i++){
					itemNumber++;
					a = strtok(NULL,"\n");
					if (a==0)break;
				}
				
				//Alloc
				tbButtons = (TBBUTTON *)calloc(itemNumber*sizeof(TBBUTTON),1);

				//Add items
				strncpy(copy, (char *)iText, MAX_LABEL_SIZE+1);
				a = strtok(copy,"\n");
				for (i=0;i>=0;i++){
			  		tbButtons[i].iBitmap = I_IMAGENONE;
					tbButtons[i].idCommand = i;
					tbButtons[i].fsState = TBSTATE_ENABLED;
					tbButtons[i].fsStyle = BTNS_DROPDOWN|BTNS_AUTOSIZE;
					tbButtons[i].dwData = 0;
					tbButtons[i].iString = (INT_PTR)a;
					a = strtok(NULL,"\n");
					if (a==0)break;
				}

				SendMessage(gParams->ctl[n].hCtl, TB_BUTTONSTRUCTSIZE, (WPARAM)sizeof(TBBUTTON), 0);
				SendMessage(gParams->ctl[n].hCtl, TB_ADDBUTTONS,       (WPARAM)itemNumber, (LPARAM)tbButtons);

				//setCtlPos(n,-1,-1,-1,-1);
				//SendMessage(gParams->ctl[n].hCtl, TB_AUTOSIZE, 0, 0);
				//Get new size
				//SendMessage(gParams->ctl[n].hCtl, TB_GETMAXSIZE, 0, (LPARAM)&size);
				//gParams->ctl[n].width = size.cx;
				//gParams->ctl[n].height = size.cy;
			}

		}
		break;

    default:
        //most other controls....
        return SetWindowText(gParams->ctl[n].hCtl, formatString(gParams->ctl[n].label));
        break;
    } //switch
    return FALSE;   //failed
} /*fm_setCtlText*/

/**********************************************************************/
/* setCtlTextv(n, "format",...)
/*
/*  Sets the text label or content for control n to the expanded
/*  printf-style format string.
/*
/**********************************************************************/
int setCtlTextv(int n, char* iFmt,...)
{
    char szBuffer[1024];
    va_list ap;

    va_start(ap, iFmt);
    _vsnprintf(szBuffer, sizeof(szBuffer), (char *)iFmt, ap);
    szBuffer[sizeof(szBuffer)-1] = 0;   //ensure NUL-terminated
    va_end(ap);
    return setCtlText(n, (char*)szBuffer);
} /*fm_setCtlTextv*/


/**********************************************************************/
/* enableToolTipBalloon(enable)
/*
/*  Enables or disables the balloon shape of all tool tips, based on
/*  the value of boolean value "enable".
/*
/**********************************************************************/
// For older versions of the SDK include files...
#ifndef TTS_BALLOON

#define TTS_NOANIMATE           0x10
#define TTS_NOFADE              0x20
#define TTS_BALLOON             0x40
#define TTS_CLOSE               0x80

//flags
#define TTF_PARSELINKS          0x1000


// ToolTip Icons (Set with TTM_SETTITLE)
#define TTI_NONE                0
#define TTI_INFO                1
#define TTI_WARNING             2
#define TTI_ERROR               3

#define TTM_SETTITLEA           (WM_USER + 32)  // wParam = TTI_*, lParam = char* szTitle
#define TTM_SETTITLEW           (WM_USER + 33)  // wParam = TTI_*, lParam = wchar* szTitle

#ifdef UNICODE
#define TTM_SETTITLE            TTM_SETTITLEW
#else
#define TTM_SETTITLE            TTM_SETTITLEA
#endif

#endif  //old include file


int enableToolTipBalloon(int enable) {
	LONG dwStyle = (LONG)GetWindowLong(ghTT, GWL_STYLE);

	int oldStyle = (dwStyle & TTS_BALLOON)? TRUE : FALSE;

	if (enable != FALSE) {
        dwStyle = WS_POPUP | TTS_ALWAYSTIP | TTS_NOPREFIX | TTS_BALLOON;

	} else {
        dwStyle = WS_POPUP | TTS_ALWAYSTIP | TTS_NOPREFIX | WS_BORDER;
	}

	SetWindowLong(ghTT, GWL_STYLE, dwStyle);

	return oldStyle;
}

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
int setCtlToolTip(int n, char * iText, int s)
{
    int iss = TRUE;
    TOOLINFO ti;
    int safe_style = s & (TTF_CENTERTIP | TTF_RTLREADING | TTF_TRACK |
                          TTF_ABSOLUTE | TTF_TRANSPARENT);

    if (n < 0 || n >= N_CTLS || !gParams->ctl[n].inuse) {
        return FALSE;
    }

    if (gParams->ctl[n].tooltip[0] == '\0') {
        //no current tool tip for this control
        if (iText != 0 && ((char *)iText)[0] != '\0') {
           
			if (n==CTL_PREVIEW){
				ti.cbSize = sizeof(TOOLINFO);
                ti.uFlags = safe_style | TTF_IDISHWND | TTF_SUBCLASS;//0;                      // or TTF_SUBCLASS???
                ti.hwnd = MyHDlg;
                ti.uId = (UINT) CTL_PREVIEW;         // our id for this control
                ti.rect.left = gProxyRect.left;
                ti.rect.top = gProxyRect.top;
                ti.rect.right = gProxyRect.right;
                ti.rect.bottom = gProxyRect.bottom;
                ti.hinst = (HINSTANCE)hDllInstance;
			} else {
				//register a tooltip for this control
				ti.cbSize = sizeof(TOOLINFO);
				ti.uFlags = safe_style | TTF_IDISHWND | TTF_SUBCLASS;
				ti.hwnd = MyHDlg;
				ti.uId = (UINT_PTR) gParams->ctl[n].hCtl;
                // IIRC: leaving the rect undefined results in no tooltips under
                //       NT 4.0+ and XP.  Setting to 0 seems to fix the problem.
                ti.rect.left = ti.rect.top = ti.rect.right = ti.rect.bottom = 0;
				ti.hinst = (HINSTANCE)hDllInstance;
			}
            ti.lpszText = formatString((char *)iText);
            iss = (int)SendMessage(ghTT, TTM_ADDTOOL, 0, (LPARAM)&ti);
            if (gParams->ctl[n].ctlclass == CC_STANDARD || gParams->ctl[n].ctlclass == CC_SLIDER) {
                //set tooltips for buddies
                ti.uId = (UINT_PTR) gParams->ctl[n].hBuddy1;
                iss = (int)SendMessage(ghTT, TTM_ADDTOOL, 0, (LPARAM)&ti);
#if 1
                ti.uId = (UINT_PTR) gParams->ctl[n].hBuddy2;
                iss = (int)SendMessage(ghTT, TTM_ADDTOOL, 0, (LPARAM)&ti);
#endif
            }
        }
    }
    else {
        //already have a tool tip for this control
        if (iText != 0 && ((char *)iText)[0] != '\0') {
            //update text for this tool tip...
            ti.cbSize = sizeof(TOOLINFO);
            ti.uFlags = safe_style | TTF_IDISHWND;
            ti.hwnd = MyHDlg;
            ti.uId = (UINT_PTR) gParams->ctl[n].hCtl;
            //ti.rect = xxx;
            ti.hinst = (HINSTANCE)hDllInstance;
            ti.lpszText = formatString((char *)iText);
            iss = (int)SendMessage(ghTT, TTM_UPDATETIPTEXT, 0, (LPARAM)&ti);
            if (gParams->ctl[n].ctlclass == CC_STANDARD  || gParams->ctl[n].ctlclass == CC_SLIDER) {
                //update tooltips for buddies
                ti.uId = (UINT_PTR) gParams->ctl[n].hBuddy1;
                iss = (int)SendMessage(ghTT, TTM_UPDATETIPTEXT, 0, (LPARAM)&ti);
#if 1
                ti.uId = (UINT_PTR) gParams->ctl[n].hBuddy2;
                iss = (int)SendMessage(ghTT, TTM_UPDATETIPTEXT, 0, (LPARAM)&ti);
#endif
            }
        }
        else {
            //delete this tool tip
            ti.cbSize = sizeof(TOOLINFO);
            ti.uFlags = safe_style | TTF_IDISHWND;
            ti.hwnd = MyHDlg;
            ti.uId = (UINT_PTR) gParams->ctl[n].hCtl;
            //ti.rect = xxx;
            ti.hinst = (HINSTANCE)hDllInstance;
            ti.lpszText = NULL;
            iss = (int)SendMessage(ghTT, TTM_DELTOOL, 0, (LPARAM)&ti);
            if (gParams->ctl[n].ctlclass == CC_STANDARD  || gParams->ctl[n].ctlclass == CC_SLIDER) {
                //delete tooltips for buddies
                ti.uId = (UINT_PTR) gParams->ctl[n].hBuddy1;
                iss = (int)SendMessage(ghTT, TTM_DELTOOL, 0, (LPARAM)&ti);
                ti.uId = (UINT_PTR) gParams->ctl[n].hBuddy2;
                iss = (int)SendMessage(ghTT, TTM_DELTOOL, 0, (LPARAM)&ti);
            }
        }
    }

    //save tool tip text for this control
    if (iText == 0) {
        gParams->ctl[n].tooltip[0] = '\0';   //null string
    }
    else {
        strncpy(gParams->ctl[n].tooltip, (char *)iText, MAX_TOOLTIP_SIZE+1);
        gParams->ctl[n].tooltip[MAX_TOOLTIP_SIZE] = '\0';   //ensure null-terminated
    }

    return iss;    //success/failure
} /*fm_setCtlToolTip*/


int setToolTipDelay(int m, int iTime)
{

	WPARAM w;

	if (m==1)
		w = TTDT_INITIAL;
	else if (m==2)
		w = TTDT_AUTOPOP;
	else if (m==3)
		w = TTDT_RESHOW;
	else //m==0
		w = TTDT_AUTOMATIC;

	if (iTime==-1){
		// TTDT_AUTOMATIC with initial < 0 sets all values to default...
        SendMessage(ghTT, TTM_SETDELAYTIME, w, MAKELONG(-1, 0));

	} else {
		// TTDT_AUTOMATIC sets autopop = initial*10, reshow = initial/5...
        SendMessage(ghTT, TTM_SETDELAYTIME, w, MAKELONG(iTime, 0));
	}

	return true;
}


char * getCtlImage(int n)
{

	if (n < 0 || n >= N_CTLS || !gParams->ctl[n].inuse) {
        return FALSE;
    }

	//if (gParams->ctl[n].image[0] == '\0')

	return (char *)((unsigned8 *)gParams->ctl[n].image);

}


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
int setCtlImage(int n, char * iName, int iType)
{
    int iss = TRUE;

    if (n < 0 || n >= N_CTLS || !gParams->ctl[n].inuse) {
        return FALSE;
    }

    if (gParams->ctl[n].image[0] == '\0') {
        //no current image for this control
        if (iName != 0 && ((char *)iName)[0] != '\0') {
            //set image for this control
        }
    }
    else {
        //already have an image for this control
        
        if (gParams->ctl[n].ctlclass == CC_PUSHBUTTON) {
            //set no image, and delete previous bitmap
            HBITMAP hPrevImage = (HBITMAP)SendMessage(gParams->ctl[n].hCtl,
                                              BM_SETIMAGE,
                                              IMAGE_BITMAP,
                                              (LPARAM)NULL);

        } else if (gParams->ctl[n].ctlclass == CC_IMAGE) {
            //select old bitmaps back into DC's, then
            //delete all image bitmaps and DC's.

            if (gParams->ctl[n].im.hbmOld) {
                SelectObject(gParams->ctl[n].im.hdcMem, gParams->ctl[n].im.hbmOld);
                gParams->ctl[n].im.hbmOld = 0;
            }
            if (gParams->ctl[n].im.hbm) {
                DeleteObject(gParams->ctl[n].im.hbm);
                gParams->ctl[n].im.hbm = 0;
            }
            if (gParams->ctl[n].im.hdcMem) {
                DeleteDC(gParams->ctl[n].im.hdcMem);
                gParams->ctl[n].im.hdcMem = 0;
            }

            if (gParams->ctl[n].im.hbmOldAnd) {
                SelectObject(gParams->ctl[n].im.hdcAnd, gParams->ctl[n].im.hbmOldAnd);
                gParams->ctl[n].im.hbmOldAnd = 0;
            }
            if (gParams->ctl[n].im.hbmAnd) {
                DeleteObject(gParams->ctl[n].im.hbmAnd);
                gParams->ctl[n].im.hbmAnd = 0;
            }
            if (gParams->ctl[n].im.hdcAnd) {
                DeleteDC(gParams->ctl[n].im.hdcAnd);
                gParams->ctl[n].im.hdcAnd = 0;
            }
#if 0
            //force update to reset background????
            InvalidateRect(gParams->ctl[n].hCtl, NULL, TRUE /*erase background*/);
            UpdateWindow(gParams->ctl[n].hCtl);
            //Sleep(1000);
#endif
        }
        else if (gParams->ctl[n].ctlclass == CC_METAFILE) {
            //set no image, and delete previous image
            HENHMETAFILE hPrevImage;
            hPrevImage = (HENHMETAFILE)SendMessage(gParams->ctl[n].hCtl,
                                                   STM_SETIMAGE,
                                                   IMAGE_ENHMETAFILE,
                                                   (LPARAM)NULL);
            //delete previous image, if any
/*            if (hPrevImage) {
                if (!DeleteEnhMetaFile(hPrevImage)) {
                    //ErrorOk("DeleteEnhMetaFile failed, image = %8.8x", hPrevImage);
                }
                else {
                    //Info("Deleted metafile image = %8.8x", hPrevImage);
                }
            }
*/
        }
        else if (gParams->ctl[n].ctlclass == CC_BITMAP) {
            //set no image, and delete previous bitmap
            HBITMAP hPrevImage;
            hPrevImage = (HBITMAP)SendMessage(gParams->ctl[n].hCtl,
                                              STM_SETIMAGE,
                                              IMAGE_BITMAP,
                                              (LPARAM)NULL);
            //delete previous bitmap, if any
/*            
            if (hPrevImage) {
                if (!DeleteObject(hPrevImage)) {
                    //ErrorOk("DeleteObject failed, bitmap = %8.8x", hPrevImage);
                }
                else {
                    //Info("Deleted bitmap = %8.8x", hPrevImage);
                }
            }
*/
        }
        else if (gParams->ctl[n].ctlclass == CC_ICON) {
            //set no image (icons don't get deleted)
            HICON hPrevImage;
            hPrevImage = (HICON)SendMessage(gParams->ctl[n].hCtl,
                                            STM_SETIMAGE,
                                            IMAGE_ICON,
                                            (LPARAM)NULL);
            //no need to delete previous icon (in fact, can't!)
        }
        gParams->ctl[n].image[0] = '\0';   //set no image
    }

    //save new image name for this control
    if (iName == 0) {
        gParams->ctl[n].image[0] = '\0';   //null string
    }
    else {
        strncpy(gParams->ctl[n].image, (char *)iName, _MAX_PATH+1);
        gParams->ctl[n].image[_MAX_PATH] = '\0';   //ensure null-terminated
    }
    gParams->ctl[n].imageType = iType;



    if (gParams->ctl[n].ctlclass == CC_PUSHBUTTON) {

        if (gParams->ctl[n].image[0] != '\0') {
            HBITMAP hBmp;

            // First try loading the bitmap from an embedded BITMAP resource...
            hBmp = (HBITMAP)LoadImage((HINSTANCE)hDllInstance,
                             gParams->ctl[n].image,
                             IMAGE_BITMAP,
                             0, 0,
                             LR_DEFAULTCOLOR);
            //if (hBmp) Info("Loaded bitmap from resource: %s", gParams->ctl[n].image);

            // If it's not embedded, try searching for it as an
            // external bitmap file...
            if (!hBmp) {
                // Load bitmap from file...
                hBmp = (HBITMAP)LoadImage((HINSTANCE)hDllInstance,
                                FmSearchFile(gParams->ctl[n].image),
                                IMAGE_BITMAP,
                                0, 0,
                                LR_LOADFROMFILE);
            }
            if (!hBmp) {
                ErrorOk("Could not load bitmap %s", gParams->ctl[n].image);
                return FALSE;
            }
            else {
                HBITMAP hPrevImage;
                hPrevImage = (HBITMAP)SendMessage(gParams->ctl[n].hCtl,
                                                  BM_SETIMAGE,
                                                  IMAGE_BITMAP,
                                                  (LPARAM)hBmp);
            }
        }

    } else if (gParams->ctl[n].ctlclass == CC_BITMAP) {
        if (gParams->ctl[n].image[0] != '\0') {
            HBITMAP hBmp;

            // First try loading the bitmap from an embedded BITMAP resource...
            hBmp = (HBITMAP)LoadImage((HINSTANCE)hDllInstance,
                             gParams->ctl[n].image,
                             IMAGE_BITMAP,
                             0, 0,
                             LR_DEFAULTCOLOR);
            //if (hBmp) Info("Loaded bitmap from resource: %s", gParams->ctl[n].image);

            // If it's not embedded, try searching for it as an
            // external bitmap file...
            if (!hBmp) {
                // Load bitmap from file...
                hBmp = (HBITMAP)LoadImage((HINSTANCE)hDllInstance,
                                FmSearchFile(gParams->ctl[n].image),
                                IMAGE_BITMAP,
                                0, 0,
                                LR_LOADFROMFILE);
            }
            if (!hBmp) {
                ErrorOk("Could not load bitmap %s", gParams->ctl[n].image);
                return FALSE;
            }
            else {
                HBITMAP hPrevImage;
                //Info("hBmp = %x", hBmp);
                hPrevImage = (HBITMAP)SendMessage(gParams->ctl[n].hCtl,
                                                  STM_SETIMAGE,
                                                  IMAGE_BITMAP,
                                                  (LPARAM)hBmp);
                //delete previous bitmap
/*
                if (hPrevImage) {
                    if (!DeleteObject(hPrevImage)) {
                        //ErrorOk("DeleteObject failed, bitmap = %8.8x", hPrevImage);
                    }
                    else {
                        //Info("Deleted bitmap = %8.8x", hPrevImage);
                    }
                }
                //DeleteObject(hBmp);    //do this at end of dialog???
*/
#if 0
                return TRUE;        //unless need to force window update below...
#endif
            }
        }
    }//if CC_BITMAP

    else if (gParams->ctl[n].ctlclass == CC_ICON) {
        if (gParams->ctl[n].image[0] != '\0') {
            HICON   hIcon;

            // First try loading the icon from an embedded RT_ICON resource...
            hIcon = (HICON)LoadImage((HINSTANCE)hDllInstance,
                              gParams->ctl[n].image,
                              IMAGE_ICON,
                              0, 0,
                              LR_DEFAULTCOLOR);
            //if (hIcon) Info("Loaded icon from resource: %s", gParams->ctl[n].image);
            //Info("Loaded icon from resource: %8.8x", hIcon);

            // If it's not embedded, try searching for it as an
            // external icon file...
            if (!hIcon) {
                // Load icon from file...
                hIcon = (HICON)LoadImage((HINSTANCE)hDllInstance,
                                 FmSearchFile(gParams->ctl[n].image),
                                 IMAGE_ICON,
                                 0, 0,
                                 LR_LOADFROMFILE);
                //Info("Loaded icon from file: %8.8x", hIcon);
            }
            if (!hIcon) {
                ErrorOk("Could not load icon %s", gParams->ctl[n].image);
                return FALSE;
            }
            else {
                HICON hPrevImage;
                //Info("hIcon = %x", hIcon);
                hPrevImage = (HICON)SendMessage(gParams->ctl[n].hCtl,
                                                STM_SETIMAGE,
                                                IMAGE_ICON,
                                                (LPARAM)hIcon);
                //no need to delete previous icon (in fact, can't!)
#if 0
                return TRUE;        //unless need to force window update below...
#endif
            }
        }
    }//if CC_ICON

    else if (gParams->ctl[n].ctlclass == CC_METAFILE) {
      if (gParams->ctl[n].image[0] != '\0') {
        //enhanced metafile static control....
        HENHMETAFILE hImage = NULL;
        FILE *file;

        // Read from resource
        // Could also use LoadIMage (  IMAGE_ENHMETAFILE... ) ???
        HRSRC hRes;         // handle/ptr. to res. info.
        HRSRC hResLoad;     // handle to loaded resource  
        char *pFmMf;        // pointer to FMMETAFILE resource data 
        int cb;             // Size of resource (bytes)

        // Try to locate FMMETAFILE resource first...
        hRes = FindResource((HMODULE)hDllInstance, gParams->ctl[n].image, "FMMETAFILE"); 
        //Info("hRes = %x", hRes);
        if (hRes != NULL) { 
            // Get size of resource.
            cb = SizeofResource((HMODULE)hDllInstance, hRes);
            if (cb < 100) {
                //Implausible size
                ErrorOk("FMMETAFILE resource is too short (%d bytes)", cb);
            }
            else {
                // Load the FMMETAFILE resource into global memory. 
                hResLoad = (HRSRC)LoadResource((HMODULE)hDllInstance, hRes);
                //Info("hResLoad = %x", hResLoad);
                if (hResLoad == NULL) { 
                    ErrorOk("Could not load FMMETAFILE resource.");
                } 
                else {
                    // Lock the FMMETAFILE resource into global memory. 
                    pFmMf = (char *)LockResource(hResLoad); 
                    //Info("pFmMf = %x", pFmMf);
                    if (pFmMf == NULL) { 
                        ErrorOk("Could not lock FMMETAFILE resource.");
                    }
                    else {
                        // Determine what kind of metafile..,
                        if (((LPENHMETAHEADER)pFmMf)->iType == EMR_HEADER &&
                            ((LPENHMETAHEADER)pFmMf)->dSignature == ENHMETA_SIGNATURE) {
                            //It's an enhanced metafile
                            hImage = SetEnhMetaFileBits(cb, (BYTE *)pFmMf);
                        }
                        else {
                            //Probably a Windows metafile
                            if (*(DWORD *)pFmMf == 0x9AC6CDD7) {
                                //metafile has an Aldus placeable header
                                //--skip over it...
                                pFmMf += 22;
                                cb -= 22;
                                //Info("WMF adjusted 1st dword = %x", *(DWORD *)pFmMf);
                            }
#if 0
                            if (((LPMETAHEADER)pFmMf)->mtSize * 2 != (size_t)cb) {
                                ErrorOk("FMMETAFILE resource is wrong size (%d instead of %d)",
                                    cb, ((LPMETAHEADER)pFmMf)->mtSize * 2);
                            }
#else
                            if (((LPMETAHEADER)pFmMf)->mtSize * 2 > (size_t)cb) {
                                ErrorOk("FMMETAFILE resource is too small (%d instead of %d)",
                                    cb, ((LPMETAHEADER)pFmMf)->mtSize * 2);
                            }
#endif
                            else {
                                hImage = SetWinMetaFileBits(cb, (BYTE *)pFmMf,
                                                            NULL /*hdcRef*/,
                                                            NULL /*lpmfp*/);
                            }
                        }
                    }
                }
            }
        } //if hRes != NULL
        else {
            /* Search for metafile file in PATH and FM_PATH environment variables: */
            if ((file = fopen(FmSearchFile(gParams->ctl[n].image), "rb"))) {
                // Read from *.wmf (Plain Windows or Aldus Placeable) file
                // or *.emf (enhanced metafile).

                // Determine file length...
                fseek(file, 0, SEEK_END);
                cb = ftell(file);
                rewind(file);

                if (!(pFmMf = (char *)malloc(cb)))
                {
                    fclose(file);
                    ErrorOk("Could not allocate %d bytes for metafile", cb);
                }
                else {
                    //Now get the actual data bits...
                    cb = (int)fread(pFmMf, 1, cb, file);
                    //check for correct read length???
                    if (cb < 100) {
                        //implausible file size
                        ErrorOk("Metafile is too short (%d bytes)", cb);
                    }
                    // Determine what kind of metafile..,
                    else if (((LPENHMETAHEADER)pFmMf)->iType == EMR_HEADER &&
                        ((LPENHMETAHEADER)pFmMf)->dSignature == ENHMETA_SIGNATURE) {
                        //It's an enhanced metafile
                        hImage = SetEnhMetaFileBits(cb, (BYTE *)pFmMf);
                    }
                    else {
                        //Info("WMF 1st dword = %x", *(DWORD *)pFmMf);
                        if (*(DWORD *)pFmMf == 0x9AC6CDD7) {
                            //file has an Aldus placeable header
                            //--skip over it...
                            pFmMf += 22;
                            cb -= 22;
                            //Info("WMF adjusted 1st dword = %x", *(DWORD *)pFmMf);
                        }
#if 0
                        if (((LPMETAHEADER)pFmMf)->mtSize * 2 != (size_t)cb) {
                            ErrorOk("Metafile is wrong size (%d instead of %d)",
                                cb, ((LPMETAHEADER)pFmMf)->mtSize * 2);
                        }
#elif 1
                        if (((LPMETAHEADER)pFmMf)->mtSize * 2 > (size_t)cb) {
                            ErrorOk("Metafile is too small (%d instead of %d)",
                                cb, ((LPMETAHEADER)pFmMf)->mtSize * 2);
                        }
#else
                        if (((LPMETAHEADER)pFmMf)->mtSize * 2 != (size_t)cb &&
                            IDOK != Warn("Metafile is wrong size (%d instead of %d)",
                                         cb, ((LPMETAHEADER)pFmMf)->mtSize * 2)) {
                            //bad size and user pressed Cancel...
                            ; //do nothiing (so hImage == NULL)
                        }
#endif
                        else {
                            hImage = SetWinMetaFileBits(cb, (BYTE *)pFmMf,
                                                        NULL /*hdcRef*/,
                                                        NULL /*lpmfp*/);
                        }
                    }

                    free(pFmMf);
                }//if malloc

                fclose(file);
            }//if fopen
        }

        if (!hImage) {
            ErrorOk("Could not load metafile %s", gParams->ctl[n].image);
            return FALSE;
        }
        else {
            HENHMETAFILE hPrevImage;
            //Info("hImage = %x", hImage);
            //Info("EMF 1st dword = %x", *(DWORD *)hImage);
            hPrevImage = (HENHMETAFILE)SendMessage(gParams->ctl[n].hCtl,
                                                   STM_SETIMAGE,
                                                   IMAGE_ENHMETAFILE,
                                                   (LPARAM)hImage);
            //delete previous image
/*
            if (hPrevImage) {
                if (!DeleteEnhMetaFile(hPrevImage)) {
                    //ErrorOk("DeleteEnhMetaFile failed, image = %8.8x", hPrevImage);
                }
                else {
                    //Info("Deleted metafile image = %8.8x", hPrevImage);
                }
            }
*/
            //DeleteEnhMetaFile(hImage);    //do this at end of dialog???
        }
      }//if image not null
      ///////return TRUE;  //unless need to force refresh below...
      //fall through to update control...
    }//if CC_METAFILE


    //FORCE CONTROL UPDATE IF NEEDED!!!
#if 0
    //This doesn't work...
    InvalidateRect(gParams->ctl[n].hCtl, NULL, TRUE /*erase background*/);
    return iss && UpdateWindow(gParams->ctl[n].hCtl);
#elif 0
    //Try updating entire dialog!
    InvalidateRect(fmc.hDlg, NULL, TRUE /*erase background*/);
    return iss && UpdateWindow(fmc.hDlg);
#elif 0
    //This doesn't do it either...
    return iss && setCtlPos(n, -1, -1, -1, -1);
#elif 1
    //This works, but is pretty heavy-handed!!!
    setCtlPos(n, gParams->ctl[n].xPos+1, -1, -1, -1);
    setCtlPos(n, gParams->ctl[n].xPos-1, -1, -1, -1);
    InvalidateRect(gParams->ctl[n].hCtl, NULL, TRUE /*erase background*/);
    return iss && UpdateWindow(gParams->ctl[n].hCtl);
#elif 0
    //This works, but is pretty heavy-handed!!!
    setCtlPos(n, gParams->ctl[n].xPos+1, -1, -1, -1);
    setCtlPos(n, gParams->ctl[n].xPos-1, -1, -1, -1);
    return iss;
#else
    return iss;    //success/failure
#endif
} /*fm_setCtlImage*/

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
int setCtlTicFreq(int n, int m)
{
    if (n < 0 || n >= N_CTLS || !gParams->ctl[n].inuse) {
        return -1;
    }

    if (gParams->ctl[n].ctlclass == CC_TRACKBAR || gParams->ctl[n].ctlclass == CC_SLIDER) {
        gParams->ctl[n].tb.ticFreq = m;
        SendMessage(gParams->ctl[n].hCtl, TBM_SETTICFREQ, m /*freq*/, 0);
        return TRUE;
    }
    else {
        //not a TRACKBAR
        return FALSE;
    }
} /*fm_setCtlTicFreq*/

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
//#include <Uxtheme.h>

/*typedef HRESULT (STDAPICALLTYPE *LPSETWINDOWTHEME) (HWND, LPCWSTR, LPCWSTR);

LPSETWINDOWTHEME lpSetWindowTheme;
HINSTANCE hinstUXTHEMEDLL;

HRESULT setWindowTheme(HWND hwnd, LPCWSTR pszSubAppName, LPCWSTR pszSubIdList)
{
    HRESULT hres = -1;     //assume error
//#if 0                                                   // temporarily disable
    BOOL hres2 = 0;
    hinstUXTHEMEDLL = LoadLibrary("uxtheme.dll");
    if (hinstUXTHEMEDLL) {
        lpSetWindowTheme = (LPSETWINDOWTHEME) GetProcAddress(hinstUXTHEMEDLL, "SetWindowTheme");
        if (lpSetWindowTheme) {
            hres = lpSetWindowTheme( hwnd, pszSubAppName, pszSubIdList);
            //if (hres) Info("hres = %8.8x", hres);
            hres2 = FreeLibrary(hinstUXTHEMEDLL);
            //if (!hres2) Info("hres2 = %8.8x", hres2);
        }
        else {
            //Info("lpSetWindowTheme = %8.8x", lpSetWindowTheme);
        }
    }
    else {
        //Info("hinstUXTHEMEDLL = %8.8x", hinstUXTHEMEDLL);
    }
//#endif

    return hres;
} // setWindowTheme*/


//Internal Theme Variable -> theme off by default -> replaced by gParams->gDialogTheme
//int DefaultTheme=0;

typedef HANDLE HTHEME;
typedef HTHEME (STDAPICALLTYPE *LPOPENTHEMEDATA) (HWND, LPCWSTR);
LPOPENTHEMEDATA lpOpenThemeData;
typedef HRESULT (STDAPICALLTYPE *LPDRAWTHEMEBACK) (HTHEME, HDC, INT, INT, RECT*, RECT*);
LPDRAWTHEMEBACK lpDrawThemeBackground;
typedef HRESULT (STDAPICALLTYPE *LPCLOSETHEMEDATA) (HTHEME);
LPCLOSETHEMEDATA lpCloseThemeData;

/*int drawThemePart(int n, RECT * rc, LPCWSTR topic, int part, int state){

	HDC  hDC;
	HTHEME Theme;

	if (n >= 0 && n<N_CTLS)
		hDC = GetDC(gParams->ctl[n].hCtl);
	else
		hDC = GetDC(fmc.hDlg);

    hinstUXTHEMEDLL = LoadLibrary("uxtheme.dll");
    if (hinstUXTHEMEDLL) {
        lpOpenThemeData = (LPOPENTHEMEDATA) GetProcAddress(hinstUXTHEMEDLL, "OpenThemeData");
		lpDrawThemeBackground = (LPDRAWTHEMEBACK) GetProcAddress(hinstUXTHEMEDLL, "DrawThemeBackground");
		lpCloseThemeData = (LPCLOSETHEMEDATA) GetProcAddress(hinstUXTHEMEDLL, "CloseThemeData");

        Theme = lpOpenThemeData(fmc.hDlg,topic);
		lpDrawThemeBackground(Theme, hDC, part, state, rc, NULL);
		lpCloseThemeData(Theme);

		FreeLibrary(hinstUXTHEMEDLL);
		return true;
    }

	return false;
}*/


/**********************************************************************/
/* setDefaultWindowTheme(hwnd)
/*
/*  Sets the Visual Theme for a given window to the default visual
/*  theme, which for now means to turn OFF any XP Visual Styles.
/*
/**********************************************************************/
/*HRESULT setDefaultWindowTheme(HWND hwnd)
{
    // Turn off XP Visual Styles for now.
    if (gParams->gDialogTheme==0) //DefaultTheme
		return setWindowTheme(hwnd, L"", L""); //disable theme
	else
		return setWindowTheme(hwnd, NULL, NULL); //enable theme

} // setDefaultWindowTheme


/**********************************************************************/
/* setCtlTheme(n, pszSubAppName, pszSubIdList)
/*
/*  Sets the Visual Theme for control n to the specified theme.
/*
/**********************************************************************/
/*int setCtlTheme(int n, int pszSubAppName, int pszSubIdList) //Removed fm_ infront of setCtlTheme
{
    if (n < 0 || n >= N_CTLS || !gParams->ctl[n].inuse) {
        return -1;
    }

    // todo: convert ascii strings to unicode

    // todo: setWindowTheme for buddies...

    return setWindowTheme(gParams->ctl[n].hCtl, L"", L"" );  //for now
}
*/

/*
typedef HRESULT (STDAPICALLTYPE *LPENABLETHEMEDIALOGTEXTURE) (HWND, DWORD);

HRESULT setCtlTabTheme(int n, int val)
{
    HRESULT hres;
    DWORD flags;
    LPENABLETHEMEDIALOGTEXTURE lpEnableThemeDialogTexture;

    if (val==0) flags = ETDT_DISABLE; else flags = ETDT_ENABLETAB;

    hinstUXTHEMEDLL = LoadLibrary("uxtheme.dll");
    if (hinstUXTHEMEDLL) {
        lpEnableThemeDialogTexture = (LPENABLETHEMEDIALOGTEXTURE) GetProcAddress(hinstUXTHEMEDLL, "EnableThemeDialogTexture");
        if (lpSetWindowTheme) {
            hres = lpEnableThemeDialogTexture( gParams->ctl[n].hCtl, flags);
            Info ("setCtlTabTheme");
            FreeLibrary(hinstUXTHEMEDLL);
        }
        else {
            return false;
        }
    }
    else {
        return false;
    }

    return true;
}
*/



//New Styles Functions

typedef BOOL (STDAPICALLTYPE *LPISTHEMEACTIVE) (VOID);
LPISTHEMEACTIVE lpIsThemeActive;

typedef struct _DllVersionInfo {
    DWORD cbSize;
    DWORD dwMajorVersion;
    DWORD dwMinorVersion;
    DWORD dwBuildNumber;
    DWORD dwPlatformID;
} DLLVERSIONINFO;

//
// Following code is cribbed from:
//
//      http://www.codeproject.com/KB/tips/DetectTheme.aspx
//
// q.v. for possible errors and updates!
//
int getAppTheme (void){

    int ret = 0;
    OSVERSIONINFO ovi = {0};
    ovi.dwOSVersionInfoSize = sizeof ovi;
    GetVersionEx(&ovi);
    
    
    if (appTheme!=-1) return appTheme;


	if(ovi.dwMajorVersion>=5) // && ovi.dwMinorVersion==1)
    {
        //Windows XP or higher detected
        typedef BOOL WINAPI ISAPPTHEMED(void);
        typedef BOOL WINAPI ISTHEMEACTIVE(void);
        ISAPPTHEMED* pISAPPTHEMED = NULL;
        ISTHEMEACTIVE* pISTHEMEACTIVE = NULL;
        HMODULE hMod = LoadLibrary("uxtheme.dll");
        if(hMod)
        {
            pISAPPTHEMED = (ISAPPTHEMED*) GetProcAddress(hMod, "IsAppThemed");				
            pISTHEMEACTIVE = (ISTHEMEACTIVE*) GetProcAddress(hMod,"IsThemeActive");

            if(pISAPPTHEMED && pISTHEMEACTIVE)
            {
                if(pISAPPTHEMED() && pISTHEMEACTIVE())                
                {                
                    typedef HRESULT CALLBACK DLLGETVERSION(DLLVERSIONINFO*);
                    DLLGETVERSION* pDLLGETVERSION = NULL;

                    HMODULE hModComCtl = LoadLibrary("comctl32.dll");
                    if(hModComCtl)
                    {
                        pDLLGETVERSION = (DLLGETVERSION*) GetProcAddress(hModComCtl,"DllGetVersion");
                        if(pDLLGETVERSION)
                        {
                            DLLVERSIONINFO dvi = {0};
                            dvi.cbSize = sizeof dvi;
                            if(pDLLGETVERSION(&dvi) == NOERROR )
                            {
                                //ret = dvi.dwMajorVersion >= 6;
								if (dvi.dwMajorVersion >= 6) 
									ret = 1;	
                            }
                        }
                        FreeLibrary(hModComCtl);                    
                    }
                }
            }
            FreeLibrary(hMod);
        }
    }    
    
    appTheme = ret;

    return ret;

}


// int setDialogTheme (int state){

// 	if (state==0){
// 		//DefaultTheme=0;
// 		gParams->gDialogTheme=0;
// 		return setWindowTheme(fmc.hDlg, L"", L""); //disable theme
// 	} else {
		
// 		//These two lines make sure that the title bar does not looses its theme at the second invocation
// 	    SetWindowRgn(fmc.hDlg, NULL, TRUE /*redraw*/);
// 		gParams->gDialogRegion = NULL;

// 		//DefaultTheme=1;
// 		gParams->gDialogTheme=1;
// 		return setWindowTheme(fmc.hDlg, NULL, NULL); //enable theme
// 	}
// }

// Added by Ognen Genchev
int setCustomCtl(int n, int iName)
{
    if (n < 0 || n >= N_CTLS || !gParams->ctl[n].inuse) {
        return false;
    }

    hBitmapThumb[0] = (HBITMAP)LoadImage((HINSTANCE)hDllInstance, "c:\\Temp\\thumb.bmp", IMAGE_BITMAP, 0, 0, LR_LOADFROMFILE);
    hBitmapChannel[0] = (HBITMAP)LoadImage((HINSTANCE)hDllInstance, "c:\\Temp\\channel.bmp", IMAGE_BITMAP, 0, 0, LR_LOADFROMFILE);
    GetObject( hBitmapThumb[0], sizeof( BITMAP ), &bm );
    GetObject( hBitmapChannel[0], sizeof( BITMAP ), &bm );

    hBitmapThumb[1] = (HBITMAP)LoadImage((HINSTANCE)hDllInstance, "c:\\Temp\\thumb1.bmp", IMAGE_BITMAP, 0, 0, LR_LOADFROMFILE);
    hBitmapChannel[1] = (HBITMAP)LoadImage((HINSTANCE)hDllInstance, "c:\\Temp\\channel1.bmp", IMAGE_BITMAP, 0, 0, LR_LOADFROMFILE);
    GetObject( hBitmapThumb[1], sizeof( BITMAP ), &bm );
    GetObject( hBitmapChannel[1], sizeof( BITMAP ), &bm );

    return TRUE;
} // setCustomCtl()

// setFrameColor() added by Ognen Genchev
int setFrameColor(int n, int color)
{
    HWND hCtl = gParams->ctl[n].hCtl;

    if (n < 0 || n >= N_CTLS || !gParams->ctl[n].inuse) {
        return false;
    }
    // shapeColor converted to HGDIOBJ
    DeleteObject((HGDIOBJ)gParams->ctl[n].shapeColor);
    gParams->ctl[n].shapeColor = (COLORREF)color;

    InvalidateRect(hCtl, NULL, TRUE /*erase background*/);
    return UpdateWindow(hCtl);
}// setFrameColor()

/*int setCtlTheme(int n, int state)
{
    int nIDDlgItem;
	LPCWSTR pszSubAppName; 
	LPCWSTR pszSubIdList;
	int RetVal=0;

	//if (n < 0 || n >= N_CTLS) { //|| !gParams->ctl[n].inuse
	if ( (n < 0 || n >= N_CTLS || !gParams->ctl[n].inuse)) { //  && n != CTL_PROGRESS && n != CTL_FRAME && n != CTL_ZOOM
	    return -1;
    }

    // todo: setWindowTheme for buddies...

	if (n == CTL_PROGRESS) nIDDlgItem = IDC_PROGRESS1;
    else if (n == CTL_FRAME) nIDDlgItem = IDC_PROXY_GROUPBOX;
    else if (n == CTL_ZOOM) nIDDlgItem = 498; //Zoom Lable
    else  nIDDlgItem = IDC_CTLBASE+n;


	if (state==-1){ //DefaultTheme
		if (gParams->gDialogTheme==0) {
			pszSubAppName = L""; //disable theme
			pszSubIdList = L"";
		} else if (gParams->gDialogTheme==1){
			pszSubAppName = NULL; //enable theme
			pszSubIdList = NULL;
		}
		gParams->ctl[n].theme = -1;
	} else if (state==0){
		pszSubAppName = L""; //disable theme
		pszSubIdList = L"";
		gParams->ctl[n].theme = 0;
	} else { //Explorer theme for listview
		pszSubAppName = L"explorer";
		//pszSubAppName = NULL; //enable theme
		pszSubIdList = NULL;
		gParams->ctl[n].theme = 1;
	}


	if (n==CTL_ZOOM){
		setWindowTheme(GetDlgItem(fmc.hDlg, IDC_BUTTON1), pszSubAppName, pszSubIdList);
		setWindowTheme(GetDlgItem(fmc.hDlg, IDC_BUTTON2), pszSubAppName, pszSubIdList);
	}
	if (gParams->ctl[n].ctlclass == CC_STANDARD || gParams->ctl[n].ctlclass == CC_SLIDER) {
		setWindowTheme(GetDlgItem(fmc.hDlg, IDC_BUDDY1+n), pszSubAppName, pszSubIdList);
		setWindowTheme(GetDlgItem(fmc.hDlg, IDC_BUDDY2+n), pszSubAppName, pszSubIdList);
	}
	
	RetVal = setWindowTheme(GetDlgItem(fmc.hDlg, nIDDlgItem), pszSubAppName, pszSubIdList); 


	//Avoid black check box text under XP
	if (gParams->ctl[n].ctlclass == CC_CHECKBOX) {
		if ( getAppTheme() && (state==1  || (state==-1 && gParams->gDialogTheme==1)) && getCtlColor(n)==-1 )
			//setCtlColor(n,GetSysColor(COLOR_BTNFACE));
            setCtlColor(n,gParams->gDialogGradientColor1);
		else if ( (state==0  || (state==-1 && gParams->gDialogTheme==0)) && getCtlColor(n)==(int)GetSysColor(COLOR_BTNFACE) ) //Switch back to transparent
			setCtlColor(n,-1);
    }

	return RetVal;
	
}*/


#if 0

int setDialogEvent (int state){

	/*
	0 = None
	1 = Init event
	2 = Cancel event
	4 = Keydown event
	*/

	if (state<0) state=0; else if (state>7) state=7;

	if (state & 4) {
		if (KeyHook != NULL) UnhookWindowsHookEx(KeyHook); //Avoid Multiple Hooks

        /*
            #if _WIN64
                KeyHook = SetWindowsHookEx(WH_KEYBOARD_LL,LLKeyHookProc, (HINSTANCE) GetWindowLongPtr(fmc.hDlg, GWLP_HINSTANCE), GetCurrentThreadId());//(DWORD)NULL); //WH_KEYBOARD
            #else
                KeyHook = SetWindowsHookEx(WH_KEYBOARD_LL,LLKeyHookProc, (HINSTANCE) GetWindowLong(fmc.hDlg, GWL_HINSTANCE), GetCurrentThreadId());//(DWORD)NULL); //WH_KEYBOARD
            #endif
        */
        
		#if _WIN64 // || _MSC_VER >= 1400
            KeyHook = SetWindowsHookEx(WH_KEYBOARD,KeyHookProc, (HINSTANCE) GetWindowLongPtr(fmc.hDlg, GWLP_HINSTANCE), GetCurrentThreadId());//(DWORD)NULL); //WH_KEYBOARD
        #else
            KeyHook = SetWindowsHookEx(WH_KEYBOARD,KeyHookProc, (HINSTANCE) GetWindowLong(fmc.hDlg, GWL_HINSTANCE), GetCurrentThreadId());//(DWORD)NULL); //WH_KEYBOARD
        #endif
	}

    gParams->gDialogEvent |= state;
	
	return true;
}

int clearDialogEvent (int state){

	/*
	0 = None
	1 = Init event
	2 = Cancel event
	4 = Keydown event
	*/

	if (state<0) state=0; else if (state>7) state=7;

	if (state & 4) {
		if (KeyHook != NULL) UnhookWindowsHookEx(KeyHook);
	}

	gParams->gDialogEvent &= ~state;
	
	return true;
}

#endif



int createFont(int i, int size, int bold, int italic, char * fontname)
{
    HFONT hfont;
    
    if (i < 0 || i >= N_FONTS) {
        return false;
    }

    if (Font[i] != 0) DeleteObject(Font[i]);

    if (fontname==NULL || fontname[0]==0 || size==0) {
        /*NONCLIENTMETRICS ncMetrics;
        ncMetrics.cbSize = sizeof(NONCLIENTMETRICS);
        if (!SystemParametersInfo (SPI_GETNONCLIENTMETRICS,sizeof(NONCLIENTMETRICS),&ncMetrics,0)) return false;
        if (fontname==NULL) fontname = ncMetrics.lfMessageFont.lfFaceName;
        Info ("%s",fontname);
        if (size==0) {
            size = ncMetrics.lfMessageFont.lfHeight;
        }*/

        LOGFONT lgFont; 
        GetObject(GetStockObject(DEFAULT_GUI_FONT), sizeof(LOGFONT), &lgFont);
        if (fontname==NULL || fontname[0]==0) fontname = lgFont.lfFaceName;
        if (size==0) size = lgFont.lfHeight;
		//Info ("%s",fontname);
    }


	hfont = CreateFont(size, 0, 0, 0, (bold? 700 : 400), italic,
						false, false,
						DEFAULT_CHARSET/*ANSI_CHARSET*/, OUT_DEFAULT_PRECIS,
						CLIP_DEFAULT_PRECIS, ANTIALIASED_QUALITY, DEFAULT_PITCH,
						(LPCTSTR)fontname);

    if (hfont){
        Font[i] = hfont;
        return true;
    }

    return false;
}

int deleteFont (int i){

     if (i < 0 || i >= N_FONTS) {
        return false;
    }

     if (Font[i] != 0) {
        DeleteObject(Font[i]);
        Font[i] = 0;
        return true;
    }

    return false;
}


int setCtlFont(int n, int i)
{

    if (n < 0 || n >= N_CTLS || !gParams->ctl[n].inuse) {
        return false;
    }

	gParams->ctl[n].hFont = 0;

    if (i==-1){
		HFONT hfont = (HFONT)GetStockObject(DEFAULT_GUI_FONT);
		gParams->ctl[n].hFont = hfont;
	} else if (i >= 0 && i < N_FONTS && Font[i]!=0) {
		gParams->ctl[n].hFont = Font[i];
	}

	if (gParams->ctl[n].hFont){
        if (gParams->ctl[n].ctlclass ==CC_STANDARD || gParams->ctl[n].ctlclass ==CC_SLIDER){
            //SendMessage(gParams->ctl[n].hBuddy1, WM_SETFONT, (WPARAM)gParams->ctl[n].hFont, TRUE);
            SendMessage(gParams->ctl[n].hBuddy2, WM_SETFONT, (WPARAM)gParams->ctl[n].hFont, FALSE);
        } else if (gParams->ctl[n].ctlclass ==CC_COMBOBOX){
			SendMessage(gParams->ctl[n].hBuddy2, WM_SETFONT, (WPARAM)gParams->ctl[n].hFont, FALSE);
		} else {
            SendMessage(gParams->ctl[n].hCtl, WM_SETFONT, (WPARAM)gParams->ctl[n].hFont, FALSE);

			if (gParams->ctl[n].ctlclass ==CC_LISTBAR){
				setCtlPos(n, -1, -1, -1, gParams->ctl[n].height); //Avoid collapsing
			} else if (gParams->ctl[n].ctlclass ==CC_LISTVIEW || gParams->ctl[n].ctlclass ==CC_LISTBOX){
				enableCtl(n,0);
				enableCtl(n,-1);
			} else {
				//Kludge to force a window update...
				setCtlText(n, (char *)gParams->ctl[n].label); 
			}
		}

		return true;
    }
	
    return false;
}


/**********************************************************************/
/* setCtlFontColor(n, color)
/*
/*  Sets the text color for control n to the specified RGB-triple value.
/*
/**********************************************************************/
int setCtlFontColor(int n, int color)
{
    if (n < 0 || n >= N_CTLS || !gParams->ctl[n].inuse) {
        return -1;
    }

	if (gParams->ctl[n].ctlclass == CC_LISTVIEW) {
		ListView_SetTextColor(gParams->ctl[n].hCtl,color);
		return TRUE;
	} else {
		gParams->ctl[n].textColor = color;
		//Kludge to force a window update...
		return setCtlText(n, (char *)gParams->ctl[n].label);
	}

} /*fm_setCtlFontColor*/


int getCtlFontColor(int n)
{
    if (n < 0 || n >= N_CTLS || !gParams->ctl[n].inuse) {
        return -1;
    }
    
    return gParams->ctl[n].textColor;
}

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
int setCtlColor(int n, int color)
{
	HWND hCtl = gParams->ctl[n].hCtl;

	if (n == CTL_PREVIEW){
		hCtl = fmc.hDlg;//GetDlgItem(fmc.hDlg, 101);
		if (color >= 0) gParams->ctl[n].bkColor = color;
		if (!ctlUpdateLocked){
			InvalidateRect(fmc.hDlg, NULL, TRUE /*erase background*/);
			return UpdateWindow(fmc.hDlg);
		} else 
			return true;
	}
	

	if (n < 0 || n >= N_CTLS || !gParams->ctl[n].inuse) {
        return -1;
    }

	
	gParams->ctl[n].bkColor = color;
	DeleteObject(gParams->ctl[n].hBrush);
	gParams->ctl[n].hBrush = (HBRUSH)((color == -1) ?
								GetStockObject(HOLLOW_BRUSH) : 
								CreateSolidBrush(color));


    // Force a window update...
#if 1
    if (gParams->ctl[n].ctlclass == CC_TRACKBAR || gParams->ctl[n].ctlclass == CC_SLIDER) {
        // Do a dummy SETRANGEMAX with the REDRAW flag set...
        SendMessage(hCtl, TBM_SETRANGEMAX, TRUE /*redraw*/,
                    gParams->ctl[n].maxval);
        return TRUE;

    } else if (gParams->ctl[n].ctlclass == CC_LISTVIEW) {
		ListView_SetBkColor(hCtl,color);
		ListView_SetTextBkColor(hCtl,color);
		return TRUE;

	} else {
        //following doesn't work for trackbars
        //but is OK for checkbox, radiobutton, statictext, groupbox, etc.
        InvalidateRect(hCtl, NULL, TRUE /*erase background*/);
        return UpdateWindow(hCtl);
    }
#elif 0
    return TRUE; //NFG for checkbox, radiobutton, statictext, etc.
#elif 0
    // This doesn't work for anything...
    return (int)SendMessage(hCtl, WM_ERASEBKGND, (WPARAM)GetDC(hCtl), 0);
#else
    //Bring out the big guns...
    //Well, even this doesn't work for trackbars, and it's
    //probably vast overkill for other controls...
    GetWindowRect(hCtl, &wRect); 
    ScreenToClient (fmc.hDlg, (LPPOINT)&wRect);
    ScreenToClient (fmc.hDlg, (LPPOINT)&(wRect.right));
    return RedrawWindow(
#if 0
            //just updating the control window doesn't work :(
            hCtl,   // handle of control window
#else
            fmc.hDlg,   // handle of entire dialog window
#endif
#if 1
            &wRect, // address of structure with update rectangle
#else
            NULL,   // address of structure with update rectangle, NULL=>entire client area
#endif
            NULL,   // handle of update region
            RDW_ERASE|RDW_INVALIDATE|RDW_ERASENOW|RDW_UPDATENOW     // array of redraw flags
       );

#endif
    // REMEMBER TO DELETE THE BRUSH WHEN EXITING????
} /*fm_setCtlColor*/


int setCtlSysColor(int n, int con){

	return setCtlColor (n, (int)GetSysColor(con));

}

int setCtlFontSysColor(int n, int con){

	return setCtlFontColor (n, (int)GetSysColor(con));

}


/**********************************************************************/
/* getCtlColor(n)
/*
/*  Returns the current background color of control n as an RGB-triple,
/*  or -1 if n is not a valid control.
/*
/**********************************************************************/
int getCtlColor(int n)
{
    int color = gParams->ctl[n].bkColor;

    if (n < 0 || n >= N_CTLS || !gParams->ctl[n].inuse) {
        return -1;
    }

	if (gParams->ctl[n].ctlclass == CC_TAB){

#if SKIN
		if (isSkinActive() && gParams->skinStyle>0){
			
			color = skinGetColor();

		} else 
#endif
		{

			color = (int)GetSysColor(COLOR_BTNFACE);

			if ( getAppTheme() && (gParams->ctl[n].theme==1  || (gParams->ctl[n].theme==-1 && gParams->gDialogTheme==1)) ){
				
				if (getWindowsVersion() <= 10 ){ //XP & Win 2003
	            
					typedef HANDLE (STDAPICALLTYPE *LPOpenThemeData) (HWND, LPCWSTR);
					typedef HRESULT (STDAPICALLTYPE *LPGetThemeColor) (HTHEME,INT,INT,INT,COLORREF*);
					typedef HRESULT (STDAPICALLTYPE *LPCloseThemeData) (HTHEME);
					static LPOpenThemeData lpOpenThemeData = NULL;
					static LPGetThemeColor lpGetThemeColor = NULL;
					static LPCloseThemeData lpCloseThemeData = NULL;
					HANDLE vsTheme;

					HMODULE hMod = LoadLibrary("uxtheme.dll");
					if (hMod){
						lpOpenThemeData = (LPOpenThemeData) GetProcAddress(hMod, "OpenThemeData");
						lpGetThemeColor = (LPGetThemeColor) GetProcAddress(hMod,"GetThemeColor");
						lpCloseThemeData = (LPCloseThemeData) GetProcAddress(hMod,"CloseThemeData");

						vsTheme = lpOpenThemeData(gParams->ctl[n].hCtl,L"TAB");
						if (vsTheme){
							COLORREF pColor = 0;
							lpGetThemeColor(vsTheme, 9, 1, 3821, &pColor); //3802 //3821
							//lpGetThemeColor(vsTheme, 9, 1, 3821, pColor); //3802 //3821
							lpCloseThemeData(vsTheme);
							color = (int)pColor;
						}

						FreeLibrary(hMod);
					}

				} else { //Vista and 7

					color = 0xffffff; //pure white tab sheet background
				}
			}
		} 
	
	}

	return color;

} /*fm_getCtlColor*/


int setCtlDefVal(int n, int defval)
{
    if (n < 0 || n >= N_CTLS || !gParams->ctl[n].inuse) {
        return false;
    }

    gParams->ctl[n].defval = defval;

    // Added by Ognen Genchev
    gParams->ctl[n].val = defval;
    gParams->ctl[n].cancelVal = defval;
    // end
    
    return true;
}

int getCtlDefVal(int n)
{
    if (n < 0 || n >= N_CTLS || !gParams->ctl[n].inuse) {
        return false;
    }

    return gParams->ctl[n].defval;
}

/**********************************************************************/
/* setCtlVal(n, val)
/*
/*  Sets the value of control n to val.
/*
/*  Returns the previous value of control n, or -1 if n is not a
/*  valid control.
/*
/**********************************************************************/
int setCtlVal(int n, int val)
{
    int originalValue = val;    //Is this really needed???
    int prevValue;

    int val2;


    //Info("setCtlVal(%d, %d)", n, val);
    if (n < 0 || n >= N_CTLS || !gParams->ctl[n].inuse) {
        return -1;
    }

    prevValue = gParams->ctl[n].val;


    //Only manipulate the controls if dialog was displayed, otherwise we get problems
    if (fmc.displayDialog){ 

#if SKIN
		if (gParams->ctl[n].ctlclass != CC_STANDARD && gParams->ctl[n].ctlclass != CC_GROUPBOX){
			if (isSkinActive() && isSkinCtl(n)){
				gParams->ctl[n].val = val;
				fmc.pre_ctl[n] = val;
				if (ctlEnabledAs(n)) skinDraw(n,0);
				return prevValue;
			}
		}
#endif
        /**********************************************************************************************/
        /*  Fix by Ognen Genchev
        /*
        /*  Condition moved out of the switch statement to prevent resetting trackbar
        /*  value to 0 after reinitialization of the plugin.
        /**********************************************************************************************/
        if(gParams->ctl[n].ctlclass == CC_TRACKBAR){
            if (prevValue != originalValue) {
                //update the actual control if it changed
                SendMessage(gParams->ctl[n].hCtl, TBM_SETPOS, TRUE /*set*/, val);
            }
        }
        // end
        switch (gParams->ctl[n].ctlclass) {
        case CC_STANDARD:
	    case CC_SLIDER:
        case CC_SCROLLBAR:
        //  Fix by Ognen Genchev
        //  Added case CC_TRACKBAR to fix the problem with infinitely writing values
        //  below min and above max when using arrow keys to change trackbar value.
        case CC_TRACKBAR:
        // end
            if (gParams->ctl[n].minval > gParams->ctl[n].maxval) {
                //range is inverted...              //NFG for scrollbar -- thumb stays at left
                                                    // but we could fake it by using -val internally...
                if (val > gParams->ctl[n].minval) val = gParams->ctl[n].minval;
                else if (val < gParams->ctl[n].maxval) val = gParams->ctl[n].maxval;
                //make sure lineSize and pageSize are negative...       //// why not do this (only) in setCtlRange()
                                                                        //// and setCtlLineSize()/setCtlPageSize()
                                                                        //// i.e., only when linesize, pagesize, 
                                                                        //// minval, or maxval is modified ?
                if (gParams->ctl[n].lineSize > 0)
                    gParams->ctl[n].lineSize = -gParams->ctl[n].lineSize;
                if (gParams->ctl[n].pageSize > 0)
                    gParams->ctl[n].pageSize = -gParams->ctl[n].pageSize;
            }
            else {
                //normal range (min <= max)
                if (val < gParams->ctl[n].minval) val = gParams->ctl[n].minval;
                else if (val > gParams->ctl[n].maxval) val = gParams->ctl[n].maxval;
                //make sure lineSize and pageSize are positive...       ///// DITTO
                if (gParams->ctl[n].lineSize < 0)
                    gParams->ctl[n].lineSize = -gParams->ctl[n].lineSize;
                if (gParams->ctl[n].pageSize < 0)
                    gParams->ctl[n].pageSize = -gParams->ctl[n].pageSize;
            }
            //if (prevValue != originalValue || prevValue != val) {
                //update the actual control if it changed

			    if ( gParams->ctl[n].thumbSize > 0){
				    int range = abs(gParams->ctl[n].maxval - gParams->ctl[n].minval);
				    val2 = val * (range - gParams->ctl[n].thumbSize-1) / range;
                } else {
				    val2 = val;
			    }
            	
#if SKIN
				if (isSkinActive() && isSkinCtl(n)){
					
					gParams->ctl[n].val = val;
				    fmc.pre_ctl[n] = val;
					if (ctlEnabledAs(n)) skinDraw(n,0); //val2

				} else 
#endif				
				{

					if (gParams->ctl[n].gamma != 100){
						val2 = gammaCtlVal(n,val2,false);
					}

					if (gParams->ctl[n].minval > gParams->ctl[n].maxval) {
						//range is inverted...
						if (gParams->ctl[n].ctlclass == CC_SLIDER)
							SendMessage(gParams->ctl[n].hCtl, TBM_SETPOS, TRUE, gParams->ctl[n].minval + gParams->ctl[n].maxval - val2);
						else
							SetScrollPos(gParams->ctl[n].hCtl, SB_CTL,
									 gParams->ctl[n].minval + gParams->ctl[n].maxval - val2,
									 TRUE /*redraw*/);	
					}
					else {
						//normal range (min <= max)
						if (gParams->ctl[n].ctlclass == CC_SLIDER)
							SendMessage(gParams->ctl[n].hCtl, TBM_SETPOS, TRUE, val2);
						else
							SetScrollPos(gParams->ctl[n].hCtl, SB_CTL,
									 val2,
									 TRUE /*redraw*/);
					}

				}


                if ( (gParams->ctl[n].ctlclass == CC_STANDARD || gParams->ctl[n].ctlclass == CC_SLIDER) && (EditBoxUpdate==0 || GetFocus() != GetDlgItem(fmc.hDlg, IDC_BUDDY1+n)) ) {
                    //update the numeric readout field if it changed
                
    #if 1
                    if (gParams->ctl[n].divisor == 1) { // Set Integer Value

                        SetDlgItemInt(fmc.hDlg, IDC_BUDDY1+n,
                                      val,
                                      TRUE /*signed*/);

                    } else { // Set Double Value
                        char szBuffer[32];
                    
                        if (gParams->ctl[n].divisor<=10)
                            sprintf(szBuffer, "%.1f", (double) val / gParams->ctl[n].divisor );
                        else if (gParams->ctl[n].divisor<=100)
                            sprintf(szBuffer, "%.2f", (double) val / gParams->ctl[n].divisor );
                        else ///// if (gParams->ctl[n].divisor<=1000)
                            sprintf(szBuffer, "%.3f", (double) val / gParams->ctl[n].divisor );
                    
                        SetDlgItemText (fmc.hDlg, IDC_BUDDY1+n,(LPCTSTR) &szBuffer ) ;
                    }
    #else
                    char szBuffer[32];

                    sprintf(szBuffer, "%.3g", (double) val / gParams->ctl[n].divisor );
                    SetDlgItemText (fmc.hDlg, IDC_BUDDY1+n, (LPCTSTR) &szBuffer ) ;
    #endif


    #if 1
                    //if the text buddy is an Edit control, we need to
                    //call UpdateWindow() if we want an immediate update
                    UpdateWindow(GetDlgItem(fmc.hDlg, IDC_BUDDY1+n));
    #endif
                } //if CC_STANDARD
            //}//if
            break;
        // Commented out by Ognen Genchev. Fixed above.
        // case CC_TRACKBAR:
        //     if (prevValue != originalValue) {
        //         //update the actual control if it changed
            
        //         SendMessage(gParams->ctl[n].hCtl, TBM_SETPOS, TRUE /*set*/, val);
        //     }
        //     break;

        case CC_CHECKBOX:
            val &= 3;
            if (prevValue != val) {
                //update the control if it changed
                if ((int)SendMessage(gParams->ctl[n].hCtl, BM_GETCHECK, 0, 0) != val) {
                    //update the control itself if it was not already
                    //automatically updated...
                    SendMessage(gParams->ctl[n].hCtl, BM_SETCHECK, val, 0);
                }
            }
            break;

        //case CC_PUSHBUTTON: //Allow pushbuttons to have more than two values
        case CC_RADIOBUTTON:
            val &= 1;
            if (prevValue != val) {
                //update the control if it changed
                HWND nextCtl;
                int next;
                HWND thisCtl = gParams->ctl[n].hCtl;
                int nIter;
                switch (gParams->ctl[n].style & BS_TYPEMASK) {
                case BS_AUTORADIOBUTTON:
                // added by Ognen Genchev
                case BS_OWNERDRAW:
                // end
                    if ((int)SendMessage(thisCtl, BM_GETCHECK, 0, 0) != val) {
                        //update the control itself if it was not already
                        //automatically updated...
                        SendMessage(thisCtl, BM_SETCHECK, val, 0);
                    }
                    //for an auto radio button, set all other radio buttons
                    //in the current group to 0 if this button is being set
                    //to non-zero.
                    nIter = 0;
                    nextCtl = GetNextDlgGroupItem(fmc.hDlg, thisCtl, FALSE /*next*/);
                    while (nextCtl != NULL && nextCtl != thisCtl) {
                        if (++nIter > N_CTLS) {
                            // can't be more than N_CTLS user controls --
                            // we must be looping, so exit fast...
                            break;
                        }
                        next = GetDlgCtrlID(nextCtl) - IDC_CTLBASE;
                        if (next >= 0 && next < N_CTLS) {
                            if (gParams->ctl[next].ctlclass == CC_RADIOBUTTON) {
                                if (gParams->ctl[next].val != 0) {
                                    //set it to zero... (don't call setCtlVal to do
                                    //this, to avoid infinite recursion!!!!)
                                    gParams->ctl[next].val = 0;
                                    fmc.pre_ctl[next] = 0; //update cached value
                                    if ((int)SendMessage(nextCtl, BM_GETCHECK, 0, 0) != 0) {
                                        //update the control itself if it was not already
                                        //automatically updated...
                                        SendMessage(nextCtl, BM_SETCHECK, 0, 0);
                                    }
                                }//if
                            }//if
                        }
                        else {
                            //not a valid user control??? Get out quick!!
                            break;
                        }//if
                        nextCtl = GetNextDlgGroupItem(fmc.hDlg, nextCtl, FALSE /*next*/);
                    }//while
                    break;
                case BS_GROUPBOX:
                    //nothing to update for this control...
                    break;
                default:
                    //unconditionally update all other controls...
                    SendMessage(thisCtl, BM_SETCHECK, val, 0);
                    break;
                }//switch button subtype
            }//if value changed
            break;

        case CC_LISTBOX:
        case CC_LISTBAR:
            if (gParams->ctl[n].style & (LBS_MULTIPLESEL | LBS_EXTENDEDSEL)) {
                //Must use LB_SELITEMRANGEEX for multiple selection list boxes...
                SendMessage(gParams->ctl[n].hCtl, LB_SELITEMRANGEEX, val, val);
            }
            else {
                //Use LB_SETCURSEL for single-selection list boxes...
                val = (int)SendMessage(gParams->ctl[n].hCtl, LB_SETCURSEL, val, 0);
            }

			//Makes listbar work like a tab control
			gParams->ctl[n].val = val;
			fmc.pre_ctl[n] = val;
			if (gParams->ctl[n].ctlclass == CC_LISTBAR){
				if (val!=-1){
					int i;
					for (i = 0; i < N_CTLS; ++i) {
						if (gParams->ctl[i].inuse && gParams->ctl[i].tabControl == n &&  gParams->ctl[i].tabSheet != -1){
							sub_enableCtl(i);
						}	
					}
				}
			}

            break;

		case CC_LISTVIEW:
			if (val<0){//Deselect
				ListView_SetItemState(gParams->ctl[n].hCtl,fmc.pre_ctl[n],0, LVIS_SELECTED|LVIS_FOCUSED);
			} else {
				ListView_SetItemState(gParams->ctl[n].hCtl,val,LVIS_SELECTED|LVIS_FOCUSED, LVIS_SELECTED|LVIS_FOCUSED);
			}
			break;

        case CC_COMBOBOX:
            val = (int)SendMessage(gParams->ctl[n].hCtl, CB_SETCURSEL, val, 0);
            break;

	    case CC_TAB:
		    if (gParams->ctl[n].ctlclass == CC_TAB) TabCtrl_SetCurSel (gParams->ctl[n].hCtl,val);
		    
		    gParams->ctl[n].val = val;		//MWVDL Uncommented
		    fmc.pre_ctl[n] = val; //update cached value
		    //triggerEvent(n,FME_CLICKED,0);

		    if (val!=-1){
			    int i;
			    for (i = 0; i < N_CTLS; ++i) {
				    if (gParams->ctl[i].inuse && gParams->ctl[i].tabControl == n &&  gParams->ctl[i].tabSheet != -1){
					    sub_enableCtl(i);	//MWVDL 2008-07-18
				    }	
			    }
		    }
			
		    break;

        case CC_OWNERDRAW:
        case CC_STATICTEXT:
        case CC_FRAME:
        case CC_RECT:
        case CC_BITMAP:
        case CC_ICON:
        case CC_METAFILE:
        case CC_GROUPBOX:
        default:
            break;
        } //switch class

    }


    //CHECK FOR IN RANGE FOR ALL CONTROLS!!!

    gParams->ctl[n].val = val;
    fmc.pre_ctl[n] = val; //update cached value

    //if (n==39) Info ("setCtlVal: ctl(%d)=%d",n,fmc.pre_ctl[n]);

    return prevValue;
} /*fm_setCtlVal*/



int setCtlDivisor(int n, int div)
{
    if (n < 0 || n >= N_CTLS || !gParams->ctl[n].inuse) {
        return -1;
    }

    if (gParams->ctl[n].ctlclass == CC_STANDARD || gParams->ctl[n].ctlclass == CC_SLIDER) {
    
        if (div >= 1  && div <= 1000){
            gParams->ctl[n].divisor = div;
            // Update the control display if necessary
            setCtlVal(n, gParams->ctl[n].val);
            return true;
        }
    }

    return false;

} /*fm_setCtlDivisor*/



int getCtlDivisor(int n)
{
    if (n < 0 || n >= N_CTLS || !gParams->ctl[n].inuse) {
        return 1;
    }

    if (gParams->ctl[n].ctlclass == CC_STANDARD || gParams->ctl[n].ctlclass == CC_SLIDER) {
		return gParams->ctl[n].divisor;
    }

    return 1;

} /*fm_getCtlDivisor*/



int setCtlGamma(int n, int gamma)
{
    if (n < 0 || n >= N_CTLS || !gParams->ctl[n].inuse) {
        return -1;
    }

    if (gParams->ctl[n].ctlclass == CC_STANDARD || gParams->ctl[n].ctlclass == CC_SLIDER) {
    
        if (gamma >= 1  && gamma <= 1000){
            gParams->ctl[n].gamma = gamma;
            // Update the control display if necessary
            setCtlVal(n, gParams->ctl[n].val);
            return true;
        }
    }

    return false;

} /*fm_setCtlGamma*/



int setCtlProperties(int n, int props)
{
    int prevProps;
    if (n != CTL_PREVIEW && (n < 0 || n >= sizeof(gParams->ctl)/sizeof(gParams->ctl[0]) ||
        !gParams->ctl[n].inuse) ) {
        return -1;
    }
    prevProps = gParams->ctl[n].properties;
    gParams->ctl[n].properties |= props;
    if (gParams->ctl[n].ctlclass == CC_COMBOBOX &&
        gParams->ctl[n].properties & CTP_EXTENDEDUI) {
        // Set extended keyboard UI
        SendMessage(gParams->ctl[n].hCtl, CB_SETEXTENDEDUI, TRUE, 0);
    }
    return prevProps;
} /*fm_setCtlProperties*/

int clearCtlProperties(int n, int props)
{
    int prevProps;
    if (n < 0 || n >= sizeof(gParams->ctl)/sizeof(gParams->ctl[0]) ||
        !gParams->ctl[n].inuse) {
        return -1;
    }
    prevProps = gParams->ctl[n].properties;
    gParams->ctl[n].properties &= ~props;
    if (gParams->ctl[n].ctlclass == CC_COMBOBOX &&
        !(gParams->ctl[n].properties & CTP_EXTENDEDUI)) {
        // Reset extended keyboard UI
        SendMessage(gParams->ctl[n].hCtl, CB_SETEXTENDEDUI, FALSE, 0);
    }
    return prevProps;
} /*fm_clearCtlProperties*/

int setCtlRange(int n, int lo, int hi)
{   /* returns TRUE if success, else FALSE */
    int retVal = 0; //assume the worst
    if (n < 0 || n >= N_CTLS || !gParams->ctl[n].inuse) {
        return retVal;
    }
    gParams->ctl[n].minval = lo;
    gParams->ctl[n].maxval = hi;

    //update the actual control range
    switch (gParams->ctl[n].ctlclass) {
    case CC_STANDARD:
    case CC_SCROLLBAR:
        if (gParams->ctl[n].minval > gParams->ctl[n].maxval) {
            //range is inverted...
            retVal= SetScrollRange(gParams->ctl[n].hCtl,
                                   SB_CTL,
                                   gParams->ctl[n].maxval,
                                   gParams->ctl[n].minval,
                                   TRUE /*redraw*/);
        }
        else {
            //normal range (min <= max)
            retVal= SetScrollRange(gParams->ctl[n].hCtl,
                                   SB_CTL,
                                   gParams->ctl[n].minval,
                                   gParams->ctl[n].maxval,
                                   TRUE /*redraw*/);
        }
        break;

	case CC_SLIDER:
	case CC_TRACKBAR:
        SendMessage(gParams->ctl[n].hCtl, TBM_SETRANGEMIN, FALSE /*redraw*/,
                    gParams->ctl[n].minval);
        SendMessage(gParams->ctl[n].hCtl, TBM_SETRANGEMAX, TRUE /*redraw*/,
                    gParams->ctl[n].maxval);
        break;

    case CC_PUSHBUTTON:
    case CC_RADIOBUTTON:
    case CC_GROUPBOX:
        if (gParams->ctl[n].minval < 0) gParams->ctl[n].minval = 0;
        if (gParams->ctl[n].maxval > 1) gParams->ctl[n].maxval = 1;
        break;

    case CC_CHECKBOX:
        if (gParams->ctl[n].minval < 0) gParams->ctl[n].minval = 0;
        if (gParams->ctl[n].maxval > 2) gParams->ctl[n].maxval = 2;
        break;

    case CC_LISTBOX:
    case CC_LISTBAR:
    case CC_COMBOBOX:
        if (gParams->ctl[n].minval < -1) gParams->ctl[n].minval = -1;
        if (gParams->ctl[n].maxval > 0xFFFF) gParams->ctl[n].maxval = 0xFFFF;
        break;


    case CC_STATICTEXT:
    case CC_FRAME:
    case CC_RECT:
    case CC_BITMAP:
    case CC_ICON:
    case CC_METAFILE:
    case CC_OWNERDRAW:
        //allow any arbitrary range for these controls
    default:
        break;
    } //switch class

    //force value to lie within new range, and update position
    //and numeric readout if necessary (must be done AFTER
    //setting the scrollbar range)
    setCtlVal(n, gParams->ctl[n].val);
    return retVal;
} /*fm_setCtlRange*/

/**********************************************************************/
/* setCtlLineSize(n, a)
/*
/*  Sets the small step ("line size") value for control n to a, and
/*  returns the previous small step value.
/*
/**********************************************************************/
int setCtlLineSize(int n, int a)
{
    int prevValue;
    if (n < 0 || n >= N_CTLS || !gParams->ctl[n].inuse) {
        return -1;
    }
    prevValue = gParams->ctl[n].lineSize;
    gParams->ctl[n].lineSize = a;
    switch (gParams->ctl[n].ctlclass) {
    case CC_STANDARD:
	case CC_SLIDER:
    case CC_SCROLLBAR:
        if (gParams->ctl[n].minval > gParams->ctl[n].maxval) {
            //range is inverted...
            //make sure lineSize is negative...
            if (gParams->ctl[n].lineSize > 0)
                gParams->ctl[n].lineSize = -gParams->ctl[n].lineSize;
        }
        else {
            //normal range (min <= max)
            //make sure lineSize is positive...
            if (gParams->ctl[n].lineSize < 0)
                gParams->ctl[n].lineSize = -gParams->ctl[n].lineSize;
        }
		if (gParams->ctl[n].ctlclass == CC_SLIDER) 
			SendMessage(gParams->ctl[n].hCtl, TBM_SETLINESIZE, 0 /*unused*/, a);
		break; 
    case CC_TRACKBAR:
        SendMessage(gParams->ctl[n].hCtl, TBM_SETLINESIZE, 0 /*unused*/, a);
        break;
    default:
        break;
    } //switch class
    return prevValue;
} /*setCtlLineSize*/

/**********************************************************************/
/* setCtlPageSize(n, b)
/*
/*  Sets the large step ("page size") value for control n to b, and
/*  returns the previous large step value.
/*
/**********************************************************************/
int setCtlPageSize(int n, int b)
{
    int prevValue;
    if (n < 0 || n >= sizeof(gParams->ctl)/sizeof(gParams->ctl[0]) ||
        !gParams->ctl[n].inuse) {
        return -1;
    }
    prevValue = gParams->ctl[n].pageSize;
    gParams->ctl[n].pageSize = b;
    switch (gParams->ctl[n].ctlclass) {
    case CC_STANDARD:
	case CC_SLIDER:
    case CC_SCROLLBAR:
        if (gParams->ctl[n].minval > gParams->ctl[n].maxval) {
            //range is inverted...
            //make sure pageSize is negative...
            if (gParams->ctl[n].pageSize > 0)
                gParams->ctl[n].pageSize = -gParams->ctl[n].pageSize;
        }
        else {
            //normal range (min <= max)
            //make sure pageSize is positive...
            if (gParams->ctl[n].pageSize < 0)
                gParams->ctl[n].pageSize = -gParams->ctl[n].pageSize;
        }
		if (gParams->ctl[n].ctlclass == CC_SLIDER) 
			SendMessage(gParams->ctl[n].hCtl, TBM_SETPAGESIZE, 0 /*unused*/, b);
        break;
    case CC_TRACKBAR:
        SendMessage(gParams->ctl[n].hCtl, TBM_SETPAGESIZE, 0 /*unused*/, b);
        break;
    default:
        break;
    } //switch class
    return prevValue;
} /*fm_setCtlPageSize*/


int setCtlThumbSize(int n, int a)
{
    
    if (a>0 && (gParams->ctl[n].ctlclass == CC_SCROLLBAR || gParams->ctl[n].ctlclass == CC_STANDARD)){
    	
		SCROLLINFO si;
    	LPSCROLLINFO lpsi;

		if (a > gParams->ctl[n].maxval - gParams->ctl[n].minval - 2) 
			a = gParams->ctl[n].maxval - gParams->ctl[n].minval - 2; //avoid crash
    	
    	si.cbSize = sizeof(si);
    	si.fMask = SIF_PAGE; //| SIF_RANGE;
    	si.nPage = a;
    	//si.nMin = gParams->ctl[n].minval;
    	//si.nMax = gParams->ctl[n].maxval;

		lpsi = &si;
    	
    	SendMessage(gParams->ctl[n].hCtl, SBM_SETSCROLLINFO, TRUE, (LPARAM) lpsi);

    	gParams->ctl[n].thumbSize = a;
		//setCtlPageSize (n,a);

    	return true;
    
	} else if (a>0 && (gParams->ctl[n].ctlclass == CC_TRACKBAR || gParams->ctl[n].ctlclass == CC_SLIDER)){

        setCtlStyle(n,TBS_FIXEDLENGTH);
        SendMessage (gParams->ctl[n].hCtl, TBM_SETTHUMBLENGTH, (WPARAM) VDBUsToPixels(a), 0);
        // thumbSize set to 0 for proper bitmap display, previously had taken the value from a.
        // the a variable must be exactly the width of the thumb image for proper display.
        gParams->ctl[n].thumbSize = 0;

        return true;
    }
   
   return false;
   
} /*fm_setCtlThumbSize*/


/*************************************************************/
//
//  A couple of local helper functions...
//
/*************************************************************/

// Modified by Ognen Genchev. Added the xx and yy parameters.
void computeBuddy1Pos(int n, int x, int y, int w, int h, int xx, int yy, RECT *pr)
{
    // Compute position of the numedit buddy for a
    // STANDARD control.
    
    //Changed by Harald Heim, May 1, 2003
    pr->right = gParams->ctl[n].editwidth; //24; //20;               //width
    pr->bottom = gParams->ctl[n].editheight; //10;              //height

    if (gParams->ctl[n].style & SBS_VERT) { //gParams->ctl[n].ctlclass!=CC_LISTBAR && gParams->ctl[n].ctlclass!=CC_COMBOBOX && 
        //vertical scrollbar
        pr->left = x + (w - pr->right)/2;
//#ifdef LABEL_ON_TOP
        pr->top = y + h + 2;
//#else
       pr->top = y - pr->bottom - 2;
//#endif
    }
    else {
        //horizontal scrollbar
        pr->left = x + gParams->ctl[n].editXpos + xx;
        pr->top = y + gParams->ctl[n].editYpos + yy;

        // Commented out by Ognen Genchev
        // pr->left = x + w + 5;
        // pr->top = y + (h - pr->bottom)/2;
        // end
    }
    //map DBUs to pixels...
    MapDialogRect(fmc.hDlg, pr);
} /*computeBuddy1Pos*/


void computeBuddy2Pos(int n, int x, int y, int w, int h, RECT *pr)
{
    // Compute position of the static text buddy for a
    // STANDARD control.
#if 0
    pr->right = gParams->ctl[n].sb.w2;               //width (pixels)
    pr->bottom = gParams->ctl[n].sb.h2;              //height (pixels)
    if (gParams->ctl[n].ctlclass!= CC_LISTBAR && gParams->ctl[n].style & SBS_VERT) {
        //vertical scrollbar
        pr->left = x + (w - pr->right)/2;
//#ifdef LABEL_ON_TOP
        pr->top = y - 10;
//#else
//        pr->top = y + h + 2;
//#endif
    }
    else {
        //horizontal scrollbar
        pr->left = x - pr->right - 5;
#if 1
        pr->top = y + (h - pr->bottom)/2 + 1;
#else
        pr->top = y + (h - pr->bottom)/2;
#endif
    }
    //map DBUs to pixels...
    MapDialogRect(fmc.hDlg, pr);

#else

    pr->right = w;
    pr->bottom = h;
    if (gParams->ctl[n].ctlclass!=CC_LISTBAR && gParams->ctl[n].ctlclass!=CC_COMBOBOX && gParams->ctl[n].style & SBS_VERT) {
        //vertical scrollbar
        pr->left = x;
//#ifdef LABEL_ON_TOP
        pr->top = y - 10;
//#else
//        pr->top = y + h + 2;
//#endif
        //map DBUs to pixels...
        MapDialogRect(fmc.hDlg, pr);
        pr->left += (pr->right - gParams->ctl[n].sb.w2)/2;
    }
    else {

		//if (n==N_FXCTLS+68) Info ("%d, %d",pr->bottom,gParams->ctl[n].sb.h2);
		
		//horizontal scrollbar
        pr->left = x - 5;
        pr->top = y;
        //map DBUs to pixels...
        MapDialogRect(fmc.hDlg, pr);
        pr->left -= gParams->ctl[n].sb.w2;
		
		if (gParams->ctl[n].ctlclass==CC_COMBOBOX)
			pr->top += 4;
#if STYLER
		else if (gParams->ctl[n].ctlclass==CC_LISTBAR)
			pr->top += (pr->bottom - 20)/2;
#endif
		else{
			pr->top += (pr->bottom - gParams->ctl[n].sb.h2)/2;
		}

    }
    pr->right = gParams->ctl[n].sb.w2;               //width (pixels)
    pr->bottom = gParams->ctl[n].sb.h2;              //height (pixels)

#endif
    //Info("ctl%d Buddy2 pos = %d, %d, %d, %d", n, pr->left, pr->top, pr->right, pr->bottom);
} /*computeBuddy2Pos*/



int pixelUnit=0;

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
int setCtlPos(int n, int x, int y, int w, int h)
{
    RECT r;
    UINT flags = SWP_NOZORDER | SWP_NOACTIVATE | SWP_NOCOPYBITS;

    //added by Harald Heim, Dec 24, 2002
    //if ( (n < 0 || n >= sizeof(gParams->ctl)/sizeof(gParams->ctl[0]) || !gParams->ctl[n].inuse) &&
	if ( (n < 0 || n >= N_CTLS || !gParams->ctl[n].inuse) ) {  //&& n != CTL_PREVIEW  && n != CTL_PROGRESS && n != CTL_FRAME && n != CTL_ZOOM 
        return FALSE;
    }
	

	/*if (n == CTL_PREVIEW || n == CTL_PROGRESS || n == CTL_FRAME || n == CTL_ZOOM){
		if (x < 0 || y < 0) {
			flags |= SWP_NOMOVE;
		}
		if (w < 0 || h < 0) {
			flags |= SWP_NOSIZE;
		}
	}*/


if (pixelUnit==0) {

#if 0
    if (x < 0 || y < 0) {
        flags |= SWP_NOMOVE;
    }
    else {
        gParams->ctl[n].xPos = x;
        gParams->ctl[n].yPos = y;
    }
    if (w < 0 || h < 0) {
        flags |= SWP_NOSIZE;
    }
    else {
        gParams->ctl[n].width = w;
        gParams->ctl[n].height = h;
    }
#else
    if (x >= 0) gParams->ctl[n].xPos = x;
    if (y >= 0) gParams->ctl[n].yPos = y;
    if (w >= 0) gParams->ctl[n].width = w;
    if (h >= 0) gParams->ctl[n].height = h;
#endif

    x = r.left = gParams->ctl[n].xPos;
    y = r.top = gParams->ctl[n].yPos;
    w = r.right = gParams->ctl[n].width;
    h = r.bottom = gParams->ctl[n].height;
	
    MapDialogRect(fmc.hDlg, &r);

} else { //setCtlPixelPos

    if (x<0)
        x = HDBUsToPixels(gParams->ctl[n].xPos);
    else {
        gParams->ctl[n].xPos = PixelsToHDBUs(x);
    }
    r.left = x;
    
    if (y<0)
        y = VDBUsToPixels(gParams->ctl[n].yPos);
    else {
        gParams->ctl[n].yPos = PixelsToVDBUs(y);
    }
    r.top = y;

    if (w<0)
        w = HDBUsToPixels(gParams->ctl[n].width);
    else {
        gParams->ctl[n].width = PixelsToHDBUs(w);
    }
    r.right = w;
    
    if (h<0)
        h = VDBUsToPixels(gParams->ctl[n].height);
    else {
        gParams->ctl[n].height = PixelsToVDBUs(h);
    }
    r.bottom = h;

}


    if (gParams->ctl[n].ctlclass == CC_STANDARD || gParams->ctl[n].ctlclass == CC_SLIDER || gParams->ctl[n].ctlclass == CC_LISTBAR || gParams->ctl[n].ctlclass == CC_COMBOBOX) {
        RECT r2;

        // Commented out by Ognen Genchev
        // if (gParams->ctl[n].ctlclass != CC_LISTBAR && gParams->ctl[n].ctlclass != CC_COMBOBOX){
            //reposition the numedit control
            computeBuddy1Pos(n, x, y, w, h, NULL, NULL, &r2);
            SetWindowPos(gParams->ctl[n].hBuddy1, NULL,
                            r2.left,    //x
                            r2.top,     //y
                            r2.right,   //w
                            r2.bottom,  //h
                            flags);
        // }
            // end

        //reposition the static text control
        computeBuddy2Pos(n, x, y, w, h, &r2);
        SetWindowPos(gParams->ctl[n].hBuddy2, NULL,
                        r2.left,    //x
                        r2.top,     //y
                        r2.right,   //w
                        r2.bottom,  //h
                        flags);
    } //if CC_STANDARD


    //added by Harald Heim, Dec 24, 2002
    if (n == CTL_PREVIEW) {
        
		RECT imageRect;

		if (!unlockedPreviewFrame){

			double factor = 1.0;
			getSysDPI(&factor,0);

			if (gParams->ctl[CTL_FRAME].hCtl==NULL) gParams->ctl[CTL_FRAME].hCtl = GetDlgItem(fmc.hDlg, IDC_PROXY_GROUPBOX);
			
			gParams->ctl[CTL_FRAME].xPos = PixelsToHDBUs(r.left-3);
			gParams->ctl[CTL_FRAME].yPos = PixelsToVDBUs(r.top-(int)(9*factor+0.5));
			gParams->ctl[CTL_FRAME].width = PixelsToHDBUs(r.right+7);
			gParams->ctl[CTL_FRAME].height = PixelsToVDBUs(r.bottom+4+(int)(9*factor+0.5));

			//Auto position frame around preview
			SetWindowPos(gParams->ctl[CTL_FRAME].hCtl, NULL,
								r.left-3,					//x
								r.top-(int)(9*factor+0.5),      //y
								r.right+7,   //w
								r.bottom+4+(int)(9*factor+0.5),   //h
								flags);
		}
        
        // Invalidate Proxy Item
		if (gParams->ctl[CTL_PREVIEW].hCtl==NULL) gParams->ctl[CTL_PREVIEW].hCtl = GetDlgItem(fmc.hDlg, 101);
		
		GetWindowRect(gParams->ctl[CTL_PREVIEW].hCtl, &imageRect);
        ScreenToClient (fmc.hDlg, (LPPOINT)&imageRect);
        ScreenToClient (fmc.hDlg, (LPPOINT)&(imageRect.right));
        InvalidateRect (fmc.hDlg, &imageRect, TRUE);

		//Info ("%d,%d - %d,%d",r.left,r.top,r.right,r.bottom);
		
        //Position preview
        SetWindowPos(gParams->ctl[CTL_PREVIEW].hCtl, NULL,
                            r.left,     //x
                            r.top,      //y
                            r.right,    //w
                            r.bottom,   //h
                            flags);

		//Avoid problems for preview drawing without recalculation
		//But this causes crash for images larger than 2 GB, so deactivate it now !!!
		#ifndef APP
		//	restoreRect();
		#endif

        //Update Preview
        ShowOutputBuffer(globals, MyHDlg, FALSE /*fDragging*/);

        //Make FilterMeister aware of the changed proxy preview
        GetProxyItemRect (MyHDlg, &gProxyRect);

        // TODO: Update tooltip rect if in use...

		if (!fScaling) setCtlAnchor (n, gParams->ctl[n].anchor);

        return 1;

    } else if (n == CTL_ZOOM){

        HWND Control;
        int ret;


        Control = GetDlgItem(fmc.hDlg, IDC_BUTTON2);
        ret = SetWindowPos(Control, NULL,
                        r.left,     //x
                        r.top,      //y
                        HDBUsToPixels(12),    //w
                        VDBUsToPixels(10),   //h
                        flags);
        
        Control = GetDlgItem(fmc.hDlg, 498);
        ret = SetWindowPos(Control, NULL,
                        r.left + HDBUsToPixels(12),     //x
                        r.top,      //y
                        HDBUsToPixels(30),    //w
                        VDBUsToPixels(10),   //h
                        flags);


        Control = GetDlgItem(fmc.hDlg, IDC_BUTTON1);
        ret = SetWindowPos(Control, NULL,
                        r.left + HDBUsToPixels(42),     //x
                        r.top,      //y
                        HDBUsToPixels(12),    //w
                        VDBUsToPixels(10),   //h
                        flags);

		if (!fScaling) setCtlAnchor (n, gParams->ctl[n].anchor);

        return ret;

	} else {

        HWND Control = 0;
		int ret;

#if SKIN
		if (isSkinActive() && gParams->skinStyle>0){
			if (gParams->ctl[n].ctlclass == CC_COMBOBOX) 
				r.bottom = skinGetComboHeight(); //Limit control size
			else if (gParams->ctl[n].ctlclass == CC_TAB){
				ret = SetWindowPos(gParams->ctl[n].hBuddy1, NULL,
                            r.left,     //x
                            r.top,      //y
                            r.right,    //w
                            skinGetTabButtonHeight(),   //h
                            flags);
			}
		}
#endif

        // TODO: Set .hCtl for all predefined controls...

		//Needed for second invocation !!!
		if (gParams->ctl[n].hCtl)
			Control = gParams->ctl[n].hCtl;
        else if (n == CTL_ZOOM) Control = GetDlgItem(fmc.hDlg, IDC_BUTTON1);
        else if (n == CTL_FRAME) Control = GetDlgItem(fmc.hDlg, IDC_PROXY_GROUPBOX);
        else if (n == CTL_PROGRESS) Control = GetDlgItem(fmc.hDlg, IDC_PROGRESS1);
		// //else if (gParams->ctl[n].ctlclass == CC_TOOLBAR) Control = gParams->ctl[n].hBuddy2;
        //else 
		//	Control = gParams->ctl[n].hCtl;

		//if (n == CTL_PROGRESS)
		//	Info ("%d\n%d",Control,gParams->ctl[n].hCtl);

        ret = SetWindowPos(Control, NULL,
                            r.left,     //x
                            r.top,      //y
                            r.right,    //w
                            r.bottom,   //h
                            flags);

		if (!fScaling) setCtlAnchor (n, gParams->ctl[n].anchor);
        else {
            //scaling the control.  Force it to update value
            // (needed for STANDARD and SCROLLBAR controls to
            // keep the flashing highlight in sync with the thumb;
            // probably not needed by other controls, but better
            // safe than sorry..).
            if (GetFocus() == gParams->ctl[n].hCtl && 
                (gParams->ctl[n].ctlclass == CC_SCROLLBAR || gParams->ctl[n].ctlclass == CC_STANDARD) ) 
                    setCtlVal(n, fmc.pre_ctl[n]);
        }

		//Adjust Column
		if (gParams->ctl[n].ctlclass == CC_LISTVIEW){
			LVCOLUMN lvc;
			lvc.mask = LVCF_WIDTH;
			lvc.cx = r.right-22;
			ListView_SetColumn (gParams->ctl[n].hCtl,0,&lvc);
		}

		return ret;

    }

} /*fm_setCtlPos*/

int setCtlDefPos(int n, int x, int y, int width, int height)
{
	if (x>=0) gParams->ctl[n].defXPos = x;
	if (y>=0)  gParams->ctl[n].defYPos = y;
	if (width>=0) gParams->ctl[n].defwidth = width;
    if (height>=0) gParams->ctl[n].defheight = height;
	return true;
}

int getCtlDefPos(int n, int item)
{
	if (item==0) return gParams->ctl[n].defXPos;
	if (item==1) return gParams->ctl[n].defYPos;
	if (item==2) return gParams->ctl[n].defwidth;
	if (item==3) return gParams->ctl[n].defheight;
    return -1;
}

int setCtlPixelPos(int n, int x, int y, int w, int h)
{
    int retval;

    pixelUnit=1;
    retval = setCtlPos(n,x,y,w,h);
    pixelUnit=0;

    return retval;

} /*fm_setCtlPixelPos*/


int getCtlPixelPos(int n, int w)
{
    RECT rcCtl;

	if ( n < 0 || n >= N_CTLS || !gParams->ctl[n].inuse ) {
        return FALSE;
    }

    GetWindowRect( gParams->ctl[n].hCtl, &rcCtl );
	ScreenToClient (fmc.hDlg, (LPPOINT)&rcCtl);
	ScreenToClient (fmc.hDlg, (LPPOINT)&(rcCtl.right));

	switch (w){
		case 0: return rcCtl.left;
		case 1: return rcCtl.top;
		case 2: return (rcCtl.right - rcCtl.left);
		case 3: return (rcCtl.bottom - rcCtl.top);
	}

	return 0;
	
} /*fm_setCtlPixelPos*/

// setCtlEditPos() by Ognen Genchev.
int setCtlEditPos(int n, int x, int y)
{
    if (gParams->ctl[n].ctlclass == CC_STANDARD || gParams->ctl[n].ctlclass == CC_SLIDER) {
        RECT r1,r2;
        UINT flags = SWP_NOZORDER | SWP_NOACTIVATE | SWP_NOCOPYBITS;

        gParams->ctl[n].editXpos = r1.left = x; //If these two lines are not used -> Crash under 98 and ME //eh?? garbage for MapDialogRect??-afh
        gParams->ctl[n].editYpos = r1.top = y;
        r2.right = 0;
        r2.bottom = 0;
        
        MapDialogRect(fmc.hDlg, &r2);

        //reposition the numedit control
        computeBuddy1Pos(n, gParams->ctl[n].xPos, gParams->ctl[n].yPos, gParams->ctl[n].width, gParams->ctl[n].height, x, y, &r2);

        //Assign new width and height
        // r2.left = r1.left;
        // r2.top = r1.top;

        SetWindowPos(gParams->ctl[n].hBuddy1, NULL,
                        r2.left,    //x
                        r2.top,     //y
                        r2.right,   //w
                        r2.bottom,  //h
                        flags);

        return true;

    } //if CC_STANDARD

    return false;
} /*fm_setCtlEditPos*/
// end

int setCtlEditSize(int n, int w, int h)
{

    if (gParams->ctl[n].ctlclass == CC_STANDARD || gParams->ctl[n].ctlclass == CC_SLIDER) {
        RECT r1,r2;
        UINT flags = SWP_NOZORDER | SWP_NOACTIVATE | SWP_NOCOPYBITS;

        r1.left = 0; //If these two lines are not used -> Crash under 98 and ME
        r1.top = 0;
        gParams->ctl[n].editwidth =  r1.right = w;
        gParams->ctl[n].editheight = r1.bottom = h;
        
        MapDialogRect(fmc.hDlg, &r1);

        //reposition the numedit control
        computeBuddy1Pos(n, gParams->ctl[n].xPos, gParams->ctl[n].yPos, gParams->ctl[n].width, gParams->ctl[n].height, NULL, NULL, &r2);

        //Assign new width and height
        r2.right = r1.right;
        r2.bottom = r1.bottom;

        SetWindowPos(gParams->ctl[n].hBuddy1, NULL,
                        r2.left,    //x
                        r2.top,     //y
                        r2.right,   //w
                        r2.bottom,  //h
                        flags);

        return true;

    } //if CC_STANDARD

    return false;

} /*fm_setCtlEditBox*/



int setEditBoxUpdate(int ms)
{   

    EditBoxUpdate = ms;

    return true;
}



int getCtlRange(int n, int w)
{   
    
    if (n==CTL_PROGRESS){
        
        if (fmc.doingProxy) {
            if (w==0)
                return (int)SendMessage(GetDlgItem(MyHDlg, IDC_PROGRESS1), PBM_GETRANGE, TRUE, 0);
            else
                return (int)SendMessage(GetDlgItem(MyHDlg, IDC_PROGRESS1), PBM_GETRANGE, FALSE, 0);
            }

    } else if ( n >= 0 && n < N_CTLS && gParams->ctl[n].inuse){
        
        if (w==0)
            return gParams->ctl[n].minval;
        else
            return gParams->ctl[n].maxval;
    }

    return 0;            

}



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
int setCtlAction(int n, int a)
{
    int prev_action;

    if (n < 0 || n >= N_CTLS || !gParams->ctl[n].inuse) {
        return 0;
    }

    prev_action = gParams->ctl[n].action;
    gParams->ctl[n].action = (CTLACTION)a;
    return prev_action;
} /*fm_setCtlAction*/

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
int EnableMode = 0;

void setEnableMode(int state){
	//bit 0: Ignore tab state
	//bit 1: Do not deactivate tab controls
	EnableMode = state;
	//Info ("%d\nbit 0:%d\nbit1: %d",EnableMode,EnableMode & 1,!(EnableMode & 2));
}

int enableCtl(int n, int level)
{
    int prev_level;

    if (n < 0 || n >= N_CTLS) return 0;   //avoid array indexing error

    if (level == -1) level = 3; // see documentation note above

    prev_level = gParams->ctl[n].enabled;
    gParams->ctl[n].enabled = level & 3;    //clear reserved bits for now

    sub_enableCtl(n);

    return prev_level;
} /*enableCtl*/

/**********************************************************************/
/* enableCtl(n) support function
/*
/*  Determines whether a control should be visible, disabled or 
/*	invisible and sets it as such. Returns 0.
/*
/**********************************************************************/
int sub_enableCtl(int n)
{
    int nIDDlgItem;
	int level;

    if ( (n < 0 || n >= N_CTLS || !gParams->ctl[n].inuse) || n == CTL_PREVIEW ) { // && n != CTL_PROGRESS && n != CTL_FRAME && n != CTL_ZOOM
        return 0;
    }

	if (n == CTL_PROGRESS) nIDDlgItem = IDC_PROGRESS1;
    else if (n == CTL_FRAME) nIDDlgItem = IDC_PROXY_GROUPBOX;
    else if (n == CTL_ZOOM) nIDDlgItem = 498; //Zoom Label
	//else if (n == CTL_PREVIEW) nIDDlgItem = 101;
    else  nIDDlgItem = IDC_CTLBASE+n;

	//MWVDL 2008-07-18    
    if (EnableMode & 1)
		level = gParams->ctl[n].enabled;
	else
		level = ctlEnabledAs(n);
		

	EnableWindow(GetDlgItem(fmc.hDlg, nIDDlgItem), (level & 2) != 0);
	ShowWindow(GetDlgItem(fmc.hDlg, nIDDlgItem), (level & 1)?SW_SHOW:SW_HIDE);

	if (n == CTL_ZOOM){
		EnableWindow(GetDlgItem(fmc.hDlg, IDC_BUTTON1), (level & 2) != 0);
		ShowWindow(GetDlgItem(fmc.hDlg, IDC_BUTTON1), (level & 1)?SW_SHOW:SW_HIDE);
		EnableWindow(GetDlgItem(fmc.hDlg, IDC_BUTTON2), (level & 2) != 0);
		ShowWindow(GetDlgItem(fmc.hDlg, IDC_BUTTON2), (level & 1)?SW_SHOW:SW_HIDE);
	} else if (gParams->ctl[n].ctlclass == CC_STANDARD || gParams->ctl[n].ctlclass == CC_SLIDER) {
		EnableWindow(GetDlgItem(fmc.hDlg, IDC_BUDDY1+n), (level & 2) != 0);
		EnableWindow(GetDlgItem(fmc.hDlg, IDC_BUDDY2+n), (level & 2) != 0);
		ShowWindow(GetDlgItem(fmc.hDlg, IDC_BUDDY1+n), (level & 1)?SW_SHOW:SW_HIDE);
		ShowWindow(GetDlgItem(fmc.hDlg, IDC_BUDDY2+n), (level & 1)?SW_SHOW:SW_HIDE);
		//SendMessage(GetDlgItem(fmc.hDlg, IDC_BUDDY1+n), WM_NCPAINT, 1, 0); //Draw edit box frame
		//RedrawWindow(GetDlgItem(fmc.hDlg, IDC_BUDDY1+n),NULL,NULL,RDW_FRAME|RDW_INVALIDATE|RDW_ERASENOW|RDW_UPDATENOW);
		//InvalidateRect(GetDlgItem(fmc.hDlg, IDC_BUDDY1+n), NULL, TRUE);
		//RedrawWindow(GetDlgItem(fmc.hDlg, IDC_BUDDY1+n), NULL, NULL, RDW_FRAME | RDW_UPDATENOW | RDW_NOCHILDREN);
	} else if (gParams->ctl[n].ctlclass == CC_LISTBAR || gParams->ctl[n].ctlclass == CC_COMBOBOX){
		EnableWindow(GetDlgItem(fmc.hDlg, IDC_BUDDY2+n), (level & 2) != 0);
		ShowWindow(GetDlgItem(fmc.hDlg, IDC_BUDDY2+n), (level & 1)?SW_SHOW:SW_HIDE);
	}

	if (!(EnableMode & 2)){
		if (gParams->ctl[n].ctlclass == CC_TAB || gParams->ctl[n].ctlclass == CC_LISTBAR){		//MWVDL 2008-08-17: Without the level-check, disabling of controls on the tab should be possible.
			int i;
			for (i = 0; i < N_CTLS; ++i) {
				if (gParams->ctl[i].inuse								// Only process children of this tab.
				 && gParams->ctl[i].tabControl == n
				 && gParams->ctl[i].tabSheet == gParams->ctl[n].val) {
					sub_enableCtl(i);	//MWVDL 2008-06-17
				}
			}
		}
	}
    
	return 0;

} /*enableCtl*/

/**********************************************************************/
/* deleteCtl(n)
/*
/*  Deletes control n.
/*
/**********************************************************************/
int deleteCtl(int n)
{
    if (n < 0 || n >= N_CTLS) {
        //invalid control id
        return FALSE;
    }

    if (n == CTL_PROGRESS || n == CTL_FRAME || n == CTL_ZOOM){
    
        int nIDDlgItem;

        if (n == CTL_PROGRESS) nIDDlgItem = IDC_PROGRESS1;
        else if (n == CTL_FRAME) nIDDlgItem = IDC_PROXY_GROUPBOX;
        else if (n == CTL_ZOOM) nIDDlgItem = 498; //Zoom Lable

        DestroyWindow (GetDlgItem(fmc.hDlg, nIDDlgItem));
        
        if (n == CTL_ZOOM){
            DestroyWindow (GetDlgItem(fmc.hDlg, IDC_BUTTON1));
            DestroyWindow (GetDlgItem(fmc.hDlg, IDC_BUTTON2));
        }

		gParams->ctl[n].enabled = 0;
        
        
    } else if (gParams->ctl[n].inuse) {

        //delete any tool tip
        setCtlToolTip(n, 0, 0);

        //delete any associated image resources
        setCtlImage(n, 0, 0);

        if (gParams->ctl[n].hCtl) {
            //destroy the control
            DestroyWindow(gParams->ctl[n].hCtl);
            gParams->ctl[n].hCtl = NULL;
        }
        if (gParams->ctl[n].hBuddy1) {
            //destroy the first buddy control
            DestroyWindow(gParams->ctl[n].hBuddy1);
            gParams->ctl[n].hBuddy1 = NULL;
			//gParams->ctl[n].buddy1Style = 0;
			//gParams->ctl[n].buddy1StyleEx = 0;
        }
        if (gParams->ctl[n].hBuddy2) {
            //destroy the second buddy control
            DestroyWindow(gParams->ctl[n].hBuddy2);
            gParams->ctl[n].hBuddy2 = NULL;
			//gParams->ctl[n].buddy2Style = 0;
			//gParams->ctl[n].buddy2StyleEx = 0;
        }
        gParams->ctl[n].inuse = FALSE;
    }
    return TRUE;    //success
} /*fm_deleteCtl*/



/**********************************************************************/
/* createPredefinedCtls()
/*
/*  Creates the pre-defined user controls.
/*
/**********************************************************************/
void createPredefinedCtls(void)
{
	RECT rcCtl;

/*  
	//OK button
	createCtl(CTL_OK, CC_PUSHBUTTON, "OK", -1, -1, -1, -1, BS_DEFPUSHBUTTON, 0, 0, 3);
	setCtlAnchor (CTL_OK,ANCHOR_BOTTOM|ANCHOR_RIGHT);
    setCtlScripting (CTL_OK, false);
    
    //Cancel button
	createCtl(CTL_CANCEL, CC_PUSHBUTTON, "Cancel", -1, -1, -1, -1, 0, 0, 0, 3);
	setCtlAnchor (CTL_CANCEL,ANCHOR_BOTTOM|ANCHOR_RIGHT);
    setCtlScripting (CTL_CANCEL, false);
*/
    
    //FM Logo
    //createCtl(CTL_LOGO, CC_IMAGE, 0, -1, -1, -1, -1, SS_NOTIFY, 0, 0, 3);
    //setCtlImage(CTL_LOGO, (INT_PTR)"logo2.bmp", 'B');
    
	// setDefaultWindowTheme(gParams->ctl[CTL_PROGRESS].hCtl);
    // setDefaultWindowTheme(GetDlgItem(MyHDlg, IDC_BUTTON1));
    // setDefaultWindowTheme(gParams->ctl[CTL_ZOOM].hCtl); //Button 2
    // setDefaultWindowTheme(gParams->ctl[CTL_FRAME].hCtl);

	///// gParams->ctl[CTL_PROGRESS].inuse = true;  //breaks PROGRESS bar (scaled twice, created dynamically as well as statically, anything else?)
    //gParams->ctl[CTL_PROGRESS].enabled = 3;


	//Set coordinates for existing controls
	gParams->ctl[CTL_PREVIEW].hCtl = GetDlgItem(fmc.hDlg, 101);
	GetWindowRect( gParams->ctl[CTL_PREVIEW].hCtl, &rcCtl );
	ScreenToClient (fmc.hDlg, (LPPOINT)&rcCtl);
	ScreenToClient (fmc.hDlg, (LPPOINT)&(rcCtl.right));
	gParams->ctl[CTL_PREVIEW].width = PixelsToHDBUs(rcCtl.right - rcCtl.left);
	gParams->ctl[CTL_PREVIEW].height = PixelsToVDBUs(rcCtl.bottom - rcCtl.top);
	gParams->ctl[CTL_PREVIEW].xPos = PixelsToHDBUs(rcCtl.left);
	gParams->ctl[CTL_PREVIEW].yPos = PixelsToVDBUs(rcCtl.top);
	
	gParams->ctl[CTL_FRAME].hCtl = GetDlgItem(fmc.hDlg, IDC_PROXY_GROUPBOX);
	GetWindowRect( gParams->ctl[CTL_FRAME].hCtl, &rcCtl );
	ScreenToClient (fmc.hDlg, (LPPOINT)&rcCtl);
	ScreenToClient (fmc.hDlg, (LPPOINT)&(rcCtl.right));
	gParams->ctl[CTL_FRAME].width = PixelsToHDBUs(rcCtl.right - rcCtl.left);
	gParams->ctl[CTL_FRAME].height = PixelsToVDBUs(rcCtl.bottom - rcCtl.top);
	gParams->ctl[CTL_FRAME].xPos = PixelsToHDBUs(rcCtl.left);
	gParams->ctl[CTL_FRAME].yPos = PixelsToVDBUs(rcCtl.top);
	
	gParams->ctl[CTL_PROGRESS].hCtl = GetDlgItem(fmc.hDlg, IDC_PROGRESS1);
	GetWindowRect( gParams->ctl[CTL_PROGRESS].hCtl, &rcCtl );
	ScreenToClient (fmc.hDlg, (LPPOINT)&rcCtl);
	ScreenToClient (fmc.hDlg, (LPPOINT)&(rcCtl.right));
	gParams->ctl[CTL_PROGRESS].width = PixelsToHDBUs(rcCtl.right - rcCtl.left);
	gParams->ctl[CTL_PROGRESS].height = PixelsToVDBUs(rcCtl.bottom - rcCtl.top);
	gParams->ctl[CTL_PROGRESS].xPos = PixelsToHDBUs(rcCtl.left);
	gParams->ctl[CTL_PROGRESS].yPos = PixelsToVDBUs(rcCtl.top);

	gParams->ctl[CTL_ZOOM].hCtl = GetDlgItem(fmc.hDlg, IDC_BUTTON1);
	GetWindowRect( gParams->ctl[CTL_ZOOM].hCtl, &rcCtl );
	ScreenToClient (fmc.hDlg, (LPPOINT)&rcCtl);
	ScreenToClient (fmc.hDlg, (LPPOINT)&(rcCtl.right));
	gParams->ctl[CTL_ZOOM].width = PixelsToHDBUs(rcCtl.right - rcCtl.left);
	gParams->ctl[CTL_ZOOM].height = PixelsToVDBUs(rcCtl.bottom - rcCtl.top);
	gParams->ctl[CTL_ZOOM].xPos = PixelsToHDBUs(rcCtl.left);
	gParams->ctl[CTL_ZOOM].yPos = PixelsToVDBUs(rcCtl.top);

	//Set Anchors
	setCtlAnchor (CTL_PREVIEW,ANCHOR_LEFT|ANCHOR_RIGHT|ANCHOR_TOP|ANCHOR_BOTTOM);
	setCtlAnchor (CTL_FRAME,ANCHOR_LEFT|ANCHOR_RIGHT|ANCHOR_TOP|ANCHOR_BOTTOM);
	setCtlAnchor (CTL_PROGRESS,ANCHOR_BOTTOM);
	setCtlAnchor (CTL_ZOOM,ANCHOR_BOTTOM);

	//Set scripting
    setCtlScripting (CTL_PREVIEW, false);
	setCtlScripting (CTL_FRAME, false);
    setCtlScripting (CTL_PROGRESS, false);
    setCtlScripting (CTL_ZOOM, false);

} /*createPredefinedCtls*/

/**********************************************************************/
/* resetAllCtls()
/*
/*  Resets all controls (deletes them).
/*
/**********************************************************************/
int resetAllCtls(void)
{
    int i;

    //Delete any previously defined controls.
    for (i=0; i < N_CTLS; i++) {
        if (gParams->ctl[i].inuse) {
            deleteCtl(i);
        }
    }

    //Now define the default controls:
    createPredefinedCtls();

    //This is also a good place (for now) to reinitialize
    //some dialog attributes...

    //set clipping region to NULL.
    SetWindowRgn(fmc.hDlg, NULL, TRUE /*redraw*/);
    gParams->gDialogRegion = NULL;

    //set other dialog parameters back to defaults
#if 0
    gParams->gDialogGradientColor1 = RGB(0,0,0);        //black
    gParams->gDialogGradientColor2 = RGB(0,0,0);        //black
#else
    //Added by Harald Heim, March 9, 2003
    gParams->gDialogGradientColor1 = GetSysColor(COLOR_BTNFACE); //RGB(0xD4,0xD0,0xC8);   //yellow grey
    gParams->gDialogGradientColor2 = GetSysColor(COLOR_BTNFACE); //RGB(0xD4,0xD0,0xC8);   //yellow grey
    //gParams->gDialogGradientColor1 = RGB(0x5F,0x9E,0xA0);   //X11.cadetblue
    //gParams->gDialogGradientColor2 = RGB(0x5F,0x9E,0xA0);   //X11.cadetblue
#endif
    gParams->gDialogGradientDirection = 0;              //vertical
    gParams->gDialogImage_save = gDialogImage = NULL;   //no background image
    gParams->gDialogImageFileName[0] = '\0';
    gParams->gDialogImageMode = DIM_TILED;              //TILED
    gParams->gDialogImageStretchMode = COLORONCOLOR;
    gParams->gDialogDragMode = 0;                       //drag by titlebar
#if 0
    gParams->gDialogTitle[0] = '\0';                    //default dialog title
#elif 1
    //default dialog title is "filter title (image mode, filter case)"
    strcpy(gParams->gDialogTitle, "!t"); // (!M, !f)
#else
    //default dialog title is "filter title (filter case) [image mode]"
    strcpy(gParams->gDialogTitle, "!t (!f) [!M]");
#endif

	gParams->gDialogTheme = 0; //Default Dialog Theme


    //initialize registry access...
    //setRegRoot(HKEY_CURRENT_USER);
    //setRegPath("Software\\!O\\!C\\!t");

    // Initialize built-in strings...
    gParams->filterAuthorText[0] = 0;
    //gParams->filterCategoryText[0] = 0;       //NO!! Clobbers val set at compile time
    //gParams->filterTitleText[0] = 0;          //NO!! Clobbers val set at compile time
    gParams->filterFilenameText[0] = 0;
    gParams->filterCopyrightText[0] = 0;
    gParams->filterDescriptionText[0] = 0;
    gParams->filterOrganizationText[0] = 0;
    gParams->filterURLText[0] = 0;
    gParams->filterVersionText[0] = 0;
    //gParams->filterAboutText[0] = 0;          //NO!! Clobbers val set at compile time
    //fmc.filterUniqueID[0] = 0;

    // Initialize built-in strings...
    fmc.filterAuthorText[0] = 0;
    //fmc.filterCategoryText[0] = 0;            //NO!! Clobbers val set at compile time
    //fmc.filterTitleText[0] = 0;               //NO!! Clobbers val set at compile time
    fmc.filterFilenameText[0] = 0;
    fmc.filterCopyrightText[0] = 0;
    fmc.filterDescriptionText[0] = 0;
    fmc.filterOrganizationText[0] = 0;
    fmc.filterURLText[0] = 0;
    fmc.filterVersionText[0] = 0;
    //fmc.filterUniqueID[0] = 0;

#ifndef DONT_SAVE_GLOBALS
    //Zero out the predefined globals (but not filterCategoryText, etc.!!!) */
    memset(&fmc.globvars, 0, sizeof(fmc.globvars));
#endif //DONT_SAVE_GLOBALS

    //Restore initial dialog styles...
    SetWindowLong(fmc.hDlg, GWL_STYLE, gParams->gDialogInitialStyle | WS_VISIBLE);
    SetWindowLong(fmc.hDlg, GWL_EXSTYLE, gParams->gDialogInitialStyleEx);

#if 1
    //turn this on only if needed to force update of dialog styles
    SetWindowPos(fmc.hDlg, NULL,
                 0, 0, 0, 0,
                 SWP_FRAMECHANGED|SWP_NOZORDER|SWP_NOACTIVATE|SWP_NOSIZE|SWP_NOMOVE);
#endif

#if 1
    //turn this on only if necessary to force update of dialog background and fonts
    return RedrawWindow(
            fmc.hDlg,   // handle of window
            NULL,   // address of structure with update rectangle, NULL=>entire client area
            NULL,   // handle of update region
            RDW_ERASE|RDW_INVALIDATE|RDW_ERASENOW|RDW_UPDATENOW     // array of redraw flags
       );
#else
    return TRUE;
#endif
} /*fm_resetAllCtls*/



//Sub Classed Tab
#if 0
INT_PTR WINAPI TabSubclassProc (HWND hDlg, UINT wMsg, WPARAM wParam, LPARAM lParam)  
{
	int i, iCtl;

	switch (wMsg){

		//case WM_PAINT:
			//return TRUE;

		case WM_ERASEBKGND:
			//break;
			//return TRUE;
		//Clip controls to avoid flickering
		for (i=0;i<N_CTLS;i++) if (hDlg == gParams->ctl[i].hCtl) {iCtl=i; break;}
		if (iCtl==-1) break;
		{
			HDC hDC = (HDC) wParam;
			RECT rect;//r,rC;
			for (i = 0; i < N_CTLS; ++i) {
				if (gParams->ctl[i].inuse && gParams->ctl[i].enabled & 3 && gParams->ctl[i].ctlclass != CC_IMAGE){
					if (gParams->ctl[i].tabControl == iCtl && gParams->ctl[i].tabSheet == gParams->ctl[iCtl].val){ // ctlEnabledAs(i)
						GetWindowRect( gParams->ctl[i].hCtl, &rect );
						ScreenToClient (hDlg, (LPPOINT)&rect);
						ScreenToClient (hDlg, (LPPOINT)&(rect.right));
						ExcludeClipRect(hDC,rect.left,rect.top,rect.right-1,rect.bottom-1);
						/*if (gParams->ctl[i].hBuddy2 && gParams->ctl[i].buddyBkColor != -1){ //Non-transparent labels
							GetWindowRect( gParams->ctl[i].hBuddy2, &rect );
							ScreenToClient (hDlg, (LPPOINT)&rect);
							ScreenToClient (hDlg, (LPPOINT)&(rect.right));
							ExcludeClipRect(hDC,rect.left,rect.top,rect.right-1,rect.bottom-1);
						}
						if (gParams->ctl[i].hBuddy1){ //edit boxes
							GetWindowRect( gParams->ctl[i].hBuddy1, &rect );
							ScreenToClient (hDlg, (LPPOINT)&rect);
							ScreenToClient (hDlg, (LPPOINT)&(rect.right));
							ExcludeClipRect(hDC,rect.left,rect.top,rect.right-1,rect.bottom-1);
						}*/
					}
				}
			}
		}
		//return TRUE;
		break;
	}

	return CallWindowProc((WNDPROC)GetWindowLongPtr(hDlg,GWLP_USERDATA),hDlg,wMsg,wParam,lParam); //call the default system handler for the control
}
#endif

//Sub Classed Listview
INT_PTR WINAPI ListviewSubclassProc (HWND hDlg, UINT wMsg, WPARAM wParam, LPARAM lParam)  
{
	WNDPROC NextProc = NULL;

	switch (wMsg){

		case WM_LBUTTONDOWN:
		{
			int x = LOWORD(lParam);
            int y = HIWORD(lParam);
			#define LVHT_EX_GROUP_HEADER       0x10000000
			#define LVHT_EX_GROUP_FOOTER       0x20000000
			#define LVHT_EX_GROUP_COLLAPSE     0x40000000
			#define LVHT_EX_GROUP_BACKGROUND   0x80000000
			#define LVHT_EX_GROUP_STATEICON    0x01000000
			#define LVHT_EX_GROUP_SUBSETLINK   0x02000000
			#define LVHT_EX_GROUP              (LVHT_EX_GROUP_BACKGROUND | LVHT_EX_GROUP_COLLAPSE | LVHT_EX_GROUP_FOOTER | LVHT_EX_GROUP_HEADER | LVHT_EX_GROUP_STATEICON | LVHT_EX_GROUP_SUBSETLINK)
			#define ListView_HitTestEx(hwndLV, pinfo) \
					(int)SNDMSG((hwndLV), LVM_HITTEST, (WPARAM)-1, (LPARAM)(LV_HITTESTINFO *)(pinfo))
            	
			int i;
			int iCtl=-1;
			LVHITTESTINFO pinfo;
			RECT rcCtl;
			
			for (i=0;i<N_CTLS;i++) if (hDlg == gParams->ctl[i].hCtl) {iCtl=i; break;}
			if (iCtl==-1) break;
            
			GetWindowRect(gParams->ctl[i].hCtl, &rcCtl);
			ScreenToClient (hDlg, (LPPOINT)&rcCtl);
			ScreenToClient (hDlg, (LPPOINT)&(rcCtl.right));
	
			pinfo.pt.x = x-rcCtl.left;
			pinfo.pt.y = y-rcCtl.top;
			pinfo.flags = 0;


#ifndef _DEBUG
    __try { //Avoid 32-bit crash !!!
#endif
			if (ListView_HitTestEx(gParams->ctl[iCtl].hCtl,&pinfo)!=-1){
			//if (SendMessage(hDlg, LVM_HITTEST, -1, (LPARAM)&pinfo)){
				if (pinfo.flags & LVHT_EX_GROUP_HEADER) { //LVHT_EX_GROUP
					
					//Expand or collapse group
					if (getCtlGroupState(iCtl,pinfo.iItem)==LVGS_NORMAL)
						setCtlGroupState(iCtl,pinfo.iItem,LVGS_COLLAPSED);
					else
						setCtlGroupState(iCtl,pinfo.iItem,LVGS_NORMAL);

					//Avoid that first item in a group is automatically selected
					SendMessage (gParams->ctl[iCtl].hCtl,WM_LBUTTONDBLCLK,0,lParam);
					
					//Make sure that listview is redrawn
					enableCtl(iCtl,0);
					enableCtl(iCtl,-1);
				
					return TRUE;
				}
			}
#ifndef _DEBUG
	} __except (EXCEPTION_EXECUTE_HANDLER) {};
#endif

			break;
		}

		//Causes redraw issues so deactivate for now
		case WM_KEYDOWN:
		{
			int nextItem = -1;
			int key = (int)wParam;
			int i,j;
			int iCtl=-1;
			
			#define LVGS_FOCUSED 0x00000010
			#define LVGS_SELECTED 0x00000020
			
			if (key==VK_LEFT || key==VK_RIGHT){

				for (i=0;i<N_CTLS;i++) if (hDlg == gParams->ctl[i].hCtl) {iCtl=i; break;}
				if (iCtl==-1) break;
				
				for (j=0;j<getCtlGroupCount(iCtl);j++){

					int groupState = getCtlGroupState(iCtl,j);

					if ((groupState & LVGS_SELECTED || groupState & LVGS_FOCUSED)){ // && !(groupState & LVGS_HIDDEN)
						if (key==VK_RIGHT && (groupState & LVGS_COLLAPSED)){
							setCtlGroupState(iCtl,j,LVGS_NORMAL);
							setCtlFocus(iCtl);
							setCtlGroupState(iCtl,j,LVGS_FOCUSED);
							//Make sure that listview is redrawn
							enableCtl(iCtl,0);
							enableCtl(iCtl,-1);
							SetFocus(hDlg);
							return TRUE;
						} else if (key==VK_LEFT && !(groupState & LVGS_COLLAPSED)){
							setCtlGroupState(iCtl,j,LVGS_COLLAPSED);
							setCtlFocus(iCtl);
							setCtlGroupState(iCtl,j,LVGS_FOCUSED);
							//Make sure that listview is redrawn
							enableCtl(iCtl,0);
							enableCtl(iCtl,-1);
							SetFocus(hDlg);
							return TRUE;
						}
						break;
					}
				}


				//Jump to previous/next line with cursor left/right----
				if (key==VK_RIGHT){
					nextItem = ListView_GetNextItem(hDlg,fmc.pre_ctl[iCtl],LVNI_TORIGHT);
					if (nextItem==-1){
						nextItem = ListView_GetNextItem(hDlg,fmc.pre_ctl[iCtl],LVNI_ALL);
						if (nextItem!=-1){
							setCtlVal(iCtl,nextItem);
							return TRUE;
						}
					}
				} else if (key==VK_LEFT){
					nextItem = ListView_GetNextItem(hDlg,fmc.pre_ctl[iCtl],LVNI_TOLEFT);
					if (nextItem==-1){
						#define LVNI_PREVIOUS 0x0020
						nextItem = ListView_GetNextItem(hDlg,fmc.pre_ctl[iCtl],LVNI_PREVIOUS);
						if (nextItem!=-1){
							setCtlVal(iCtl,nextItem);
							return TRUE;
						}
					}
				}


/*
			} else if (key==VK_UP || key==VK_DOWN ){ // || key==VK_PAGEUP || key==VK_PAGEDOWN
				
				for (i=0;i<N_CTLS;i++) if (hDlg == gParams->ctl[i].hCtl) {iCtl=i; break;}

				//Only if multi select is active
				//if ( !(GetWindowLong(gParams->ctl[iCtl].hCtl, GWL_STYLE) & LVS_SINGLESEL))
				{
					int val, nextval, group, nextgroup;				
					val = ListView_GetNextItem(gParams->ctl[iCtl].hCtl,-1, LVNI_FOCUSED);
					group = getCtlItemGroup(iCtl,val);

					if (key==VK_UP) nextval = ListView_GetNextItem(gParams->ctl[iCtl].hCtl,val, LVNI_ABOVE);//val-1;
					else nextval = ListView_GetNextItem(gParams->ctl[iCtl].hCtl,val, LVNI_BELOW);//val+1;

					//Expand next group if neccesary 
					if (nextval == -1){
						if (key==VK_UP && group != 0){
							for (i=val;i>=0;i--){
								if (getCtlItemGroup(iCtl,i)==group-1) {
									nextval=i;
									break;
								}
							}
						} else if (key==VK_DOWN && group != getCtlGroupCount(iCtl)-1){
							for (i=val;i<getCtlItemCount(iCtl);i++){
								if (getCtlItemGroup(iCtl,i)==group+1) {
									nextval=i;
									break;
								}
							}
						}
					}

					if (nextval<0 || nextval >= getCtlItemCount(iCtl)) return TRUE;

					nextgroup = getCtlItemGroup(iCtl,nextval);

					//Info ("%d - %d",group,nextgroup);

					//Avoid that all items of a group are selected
					if (group != nextgroup) {
						
						//Expand next group if neccesary
						if (getCtlGroupState(iCtl,nextgroup) & LVGS_COLLAPSED) {
							setCtlGroupState(iCtl,nextgroup,LVGS_NORMAL);
							ListView_EnsureVisible(gParams->ctl[iCtl].hCtl,nextval,true);
						}

						setCtlItemState(iCtl,val,0);
						setCtlVal(iCtl,nextval);
						ListView_EnsureVisible(gParams->ctl[iCtl].hCtl,nextval,true);
						return TRUE;
					}
				}
*/
			}
			break;
		}
		
  	}//wMsg	


	NextProc = (WNDPROC)GetWindowLongPtr(hDlg,GWLP_USERDATA);
	return CallWindowProc(NextProc,hDlg,wMsg,wParam,lParam); //call the default system handler for the control (button)
}

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
int createCtl(int n, int c, char * t, int x, int y, int w, int h,
                        int s, int sx, int p, int e)
{
    if (n < 0 || n >= N_CTLS) {
        //invalid control id
        return 0;
    }

	//s = s | WS_CLIPSIBLINGS;

    if (gParams->ctl[n].inuse) {
        // kill previous control with this id
        deleteCtl(n);
    }

    gParams->ctl[n].ctlclass = (CTLCLASS)c;
    gParams->ctl[n].hCtl = NULL;
    gParams->ctl[n].hBuddy1 = NULL;
    gParams->ctl[n].hBuddy2 = NULL;

    if (t) {
        strncpy(gParams->ctl[n].label, (char *)t, MAX_LABEL_SIZE+1);
        gParams->ctl[n].label[MAX_LABEL_SIZE] = '\0';   //ensure null-terminated.
    }
    else {
        //No text string
        gParams->ctl[n].label[0] = '\0';
    }

    // -1 means use default value for x, y, w, or h.

    // Default size depends on control class (and sometimes
    // control index, style, or properties)...
    switch (c) {
    case CC_PUSHBUTTON:
        if (w < 0) w = 34;
        if (h < 0) h = 14;
        break;
    case CC_RECT:
    case CC_FRAME:
    case CC_IMAGE:
    case CC_BITMAP:
    case CC_ICON:
    case CC_METAFILE:
    case CC_ANIMATION:
        if (w < 0) w = 32;
        if (h < 0) h = (n == CTL_LOGO) ? 43 : 32;
        break;
    case CC_GROUPBOX:
        if (w < 0) w = 100;
        if (h < 0) h = 32;
        break;
    case CC_LISTVIEW:
	case CC_LISTBOX:
    case CC_COMBOBOX:
	    if (w < 0) w = 90;
        if (h < 0) h = 32;
        break;
    case CC_LISTBAR:
        if (w < 0) w = 90;
        if (h < 0) h = 10;
        break;
    case CC_STANDARD:
    case CC_SCROLLBAR:
        if (s & SBS_VERT) {
            //vertical scrollbar
            if (w < 0) w = 8;
            if (h < 0) h = 90;
        }
        else {
            //horizontal scrollbar
            if (w < 0) w = 90;
            if (h < 0) h = 8;
        }
        break;
    case CC_TRACKBAR:
	case CC_SLIDER:
        if (s & TBS_VERT) {
            //vertical trackbar
            if (w < 0) w = 10;
            if (h < 0) h = 90;
        }
        else {
            //horizontal trackbar
            if (w < 0) w = 90;
            if (h < 0) h = 10;
        }
        break;
    case CC_PREVIEW:
        if (w < 0) w = 228;
        if (h < 0) h = 154;
        break;
    case CC_TOOLBAR:
		if (w < 0) w = 150;
        if (h < 0) h = 13;
		break;
	case CC_TAB:
		if (w < 0) w = 120;
        if (h < 0) h = 60;
		break;
	case CC_SPINNER:
    case CC_UPDOWN:
    case CC_CHECKBOX:
    case CC_RADIOBUTTON:
    case CC_OWNERDRAW:
	case CC_PROGRESSBAR:
    case CC_EDIT:
    case CC_STATICTEXT:
    case CC_TOOLTIP:
    default:
        if (w < 0) w = 90;
        if (h < 0) h = 8;
        break;
    }//switch class

    // Default position depends on whether the control is
    // a predefined control...
    switch (n) {
	//Changed by Harald Heim, March 9, 2003
    case CTL_OK:
        if (x < 0) x = 408;//306;
        if (y < 0) y = 160;//126;
        break;
    case CTL_CANCEL:
        if (x < 0) x = 446;//269;
        if (y < 0) y = 160;//126;
        break;

    case CTL_PREVIEW:
        if (x < 0) x = 5;
        if (y < 0) y = 5;
        break;
	case CTL_PROGRESS:
    case CTL_HOST:
    default:
        //Changed by Harald Heim, March 9, 2003
        if (x < 0) x = 350 - (n/10 % 10)*5;
        if (y < 0) y = 10*(n % 10) + 5;
        break;
    }//switch n

    // Default action depends on control class (and sometimes
    // control index)...
    switch (c) {
    case CC_RADIOBUTTON:
    case CC_GROUPBOX:
    case CC_LISTVIEW:
	case CC_LISTBOX:
    case CC_COMBOBOX:
	case CC_EDIT:
	case CC_TAB:
	case CC_TOOLBAR:
        // Default action for these is NONE
        gParams->ctl[n].action = CA_NONE;
        break;
    case CC_PUSHBUTTON:
    case CC_STANDARD:
	case CC_SLIDER:
    case CC_SCROLLBAR:
    case CC_TRACKBAR:
    case CC_SPINNER:
    case CC_UPDOWN:
    case CC_LISTBAR:
    case CC_CHECKBOX:
    case CC_OWNERDRAW:
	case CC_PROGRESSBAR:
    case CC_STATICTEXT:
    case CC_RECT:
    case CC_FRAME:
    case CC_IMAGE:
    case CC_BITMAP:
    case CC_ICON:
    case CC_METAFILE:
    case CC_TOOLTIP:
    case CC_PREVIEW:
    case CC_ANIMATION:
    default:
        //default action for most controls is PREVIEW
        gParams->ctl[n].action = CA_PREVIEW;
        break;
    }//switch class

    //Default actions for predefined controls are different...
    switch (n) {
    case CTL_OK:
        gParams->ctl[n].action = CA_APPLY;
        break;
    case CTL_CANCEL:
        gParams->ctl[n].action = CA_CANCEL;
        break;

    case CTL_PREVIEW:
    case CTL_PROGRESS:
    case CTL_HOST:
    default:
        break;
    }//switch n


    gParams->ctl[n].xPos = x;
    gParams->ctl[n].yPos = y;
    gParams->ctl[n].width = w;
    gParams->ctl[n].height = h;
    gParams->ctl[n].style = s;
    gParams->ctl[n].styleEx = sx;
    gParams->ctl[n].properties = p;
    gParams->ctl[n].hBrush = (HBRUSH)GetStockObject(HOLLOW_BRUSH);
#if 0
    gParams->ctl[n].bkColor = GetSysColor(COLOR_BTNFACE);
#else
    if (c == CC_GROUPBOX){ //Avoid that text of a group box is stricked through
        gParams->ctl[n].bkColor = GetSysColor(COLOR_BTNFACE);
    } else
        gParams->ctl[n].bkColor = (COLORREF) -1;    //transparent

	gParams->ctl[n].buddyBkColor = (COLORREF) -1;    //transparent
	gParams->ctl[n].editColor = (COLORREF) -1;    //do not apply
	gParams->ctl[n].editTextColor = (COLORREF) -1;    //do not apply
#endif


#if 0
    gParams->ctl[n].textColor = GetSysColor(COLOR_BTNTEXT);
#else
    //Changed by Harald Heim, March 22, 2003
    //gParams->ctl[n].textColor = RGB(255,255,255);  //fontcolor=white
    gParams->ctl[n].textColor = GetSysColor(COLOR_WINDOWTEXT);

#endif
    gParams->ctl[n].hFont = (HFONT)GetStockObject(DEFAULT_GUI_FONT);
    gParams->ctl[n].fontSize = 8;

    gParams->ctl[n].enabled = e;

    gParams->ctl[n].val = 0;
    gParams->ctl[n].initVal = 0;
    gParams->ctl[n].minval = 0;
    gParams->ctl[n].maxval = 255;
    gParams->ctl[n].lineSize = 1;
    gParams->ctl[n].pageSize = 10;
	gParams->ctl[n].thumbSize = 0;
    gParams->ctl[n].divisor = 1;
    gParams->ctl[n].tooltip[0] = '\0';
    gParams->ctl[n].image[0] = '\0';
    gParams->ctl[n].imageType = 0;

    switch (c) {
    case CC_STANDARD:
    case CC_SCROLLBAR:
	case CC_TRACKBAR:
	case CC_SLIDER:
        {                   ///////  BUG!!!!  Buddy2 (static text) should be created first
                            ///////           for correct tab order!!
            //create a scrollbar with a static text buddy
            //and a NUMEDIT buddy.
            HWND hCtl, hBuddy1, hBuddy2;//, hBuddy3;
            int alignStyle;
            /* Convert dialog units to pixels */
            RECT r;

			r.left = x;
			r.top = y;
			r.right = w;
			r.bottom = h;
			MapDialogRect(fmc.hDlg, &r);


			if (c==CC_TRACKBAR || c==CC_SLIDER){
				
				// Should link dynamically, so only load comctl32.dll
				// if necessary!
				InitCommonControls();

				//r.left = x;
				//r.top = y;
				//r.right = w;
				//r.bottom = h;
				//MapDialogRect(fmc.hDlg, &r);

				//Default tick frequency is 1.
				gParams->ctl[n].tb.ticFreq = 1;

				// force default styles (note TBS_HORZ==0 and TBS_BOTTOM==0).
				gParams->ctl[n].style |= WS_VISIBLE | WS_CHILD | WS_TABSTOP | TBS_NOTICKS | TBS_BOTH;// | TBS_FIXEDLENGTH; //| 0x1000L; //0x1000L = TBS_TRANSPARENTBKGND

				// Transparent backgrounds don't work well for trackbars,
				// so change the default to opaque light gray.
				DeleteObject(gParams->ctl[n].hBrush); //superfluous for stock object
				gParams->ctl[n].hBrush = CreateSolidBrush(GetSysColor(COLOR_BTNFACE));//GetStockObject(LTGRAY_BRUSH); //GetStockObject(HOLLOW_BRUSH);
				gParams->ctl[n].bkColor = GetSysColor(COLOR_BTNFACE);

				// create the trackbar control
				hCtl = CreateWindowEx(  
					gParams->ctl[n].styleEx,        // extended styles
					TRACKBAR_CLASS,                 // predefined class 
					"",                             // control text
					gParams->ctl[n].style,          // styles
					r.left,                         // starting x position 
					r.top,                          // starting y position 
					r.right,                        // control width 
					r.bottom,                       // control height 
					fmc.hDlg,                       // parent window 
					(HMENU)(INT_PTR)(IDC_CTLBASE+n),        // control ID
					(HINSTANCE)hDllInstance,
					NULL);                          // pointer not needed 

                //SetClassLong(hCtl, GCL_HBRBACKGROUND, (LONG)GetStockObject(HOLLOW_BRUSH));

				if (!hCtl) return FALSE;    //failed to create window.
				gParams->ctl[n].hCtl = hCtl;

                //gParams->ctl[n].thumbSize = 8;
                //SendMessage (gParams->ctl[n].hCtl, TBM_SETTHUMBLENGTH, (WPARAM) VDBUsToPixels(8), 0);
                
				// Turn off XP Visual Styles
				//setDefaultWindowTheme(hCtl);
				gParams->ctl[n].inuse = TRUE;
				// setCtlTheme(n, (int)gParams->ctl[n].theme);
                

				if (gParams->ctl[n].style & TBS_TOOLTIPS) {
					int side;
					//set side for automatic tooltips
					if (gParams->ctl[n].properties & CTP_BOTTOM) {
						side = TBTS_BOTTOM;
					}
					else if (gParams->ctl[n].properties & CTP_RIGHT) {
						side = TBTS_RIGHT;
					}
					else if (gParams->ctl[n].properties & CTP_LEFT) {
						side = TBTS_LEFT;
					}
					else {
						//default is TOP
						side = TBTS_TOP;
					}
					SendMessage(gParams->ctl[n].hCtl, TBM_SETTIPSIDE, side, 0);
				}


			} else {

				//r.left = x;
				//r.top = y;
				//r.right = w;
				//r.bottom = h;
				//MapDialogRect(fmc.hDlg, &r);

				// force default styles (note SBS_HORZ==0).
				gParams->ctl[n].style |= WS_VISIBLE | WS_CHILD | WS_TABSTOP;

				// create the scrollbar control
				hCtl = CreateWindowEx(  
					gParams->ctl[n].styleEx,        // extended styles
					"SCROLLBAR",                    // predefined class 
					"",                             // control text
					gParams->ctl[n].style           // styles
					& ~WS_BORDER,                   // ...except WS_BORDER (see below)
					r.left,                         // starting x position 
					r.top,                          // starting y position 
					r.right,                        // control width 
					r.bottom,                       // control height 
					fmc.hDlg,                       // parent window 
					(HMENU)(INT_PTR)(IDC_CTLBASE+n),        // control ID
					(HINSTANCE)hDllInstance,
					NULL);                          // pointer not needed 

				if (!hCtl) return FALSE;    //failed to create window.
				gParams->ctl[n].hCtl = hCtl;

				// Turn off XP Visual Styles
				//setDefaultWindowTheme(hCtl);
			}


			
            if (c != CC_STANDARD && c != CC_SLIDER) break;

            //create the NUMEDIT buddy control
            if (s & SBS_VERT || s & TBS_VERT) {
                //vertical scrollbar
                alignStyle = ES_CENTER;
            }
            else {
                //horizontal scrollbar
                alignStyle = ES_LEFT;
            }
            
			//gParams->ctl[n].buddy1StyleEx |= WS_EX_STATICEDGE; //staticedge replaces black border as default?
			
			computeBuddy1Pos(n, x, y, w, h, NULL, NULL, &r);

            hBuddy1 = CreateWindowEx(  
                gParams->ctl[n].buddy1StyleEx, //0 /*WS_EX_STATICEDGE*/,               // extended styles
                "EDIT",                         // predefined class 
                "",                            // control text
                alignStyle | WS_VISIBLE | WS_CHILD /*| WS_BORDER*/ | WS_TABSTOP
#if 1
                | ES_MULTILINE | ES_WANTRETURN  // to get EN_MAXTEXT when user hits return
#endif
                /*| ES_NUMBER*/,                // styles (ES_NUMBER doesn't allow negative numbers!)
                r.left,                         // starting x position 
                r.top,                          // starting y position 
                r.right,                        // button width 
                r.bottom,                       // button height 
                fmc.hDlg,                       // parent window 
                (HMENU) (INT_PTR)(IDC_BUDDY1+n),         // control ID
				(HINSTANCE)hDllInstance,
                NULL);                          // pointer not needed 

            if (!hBuddy1) return FALSE;    //failed to create window.
            gParams->ctl[n].hBuddy1 = hBuddy1;

			//Add border here, otherwise we cannot remove it anymore
			SetWindowLong(gParams->ctl[n].hBuddy1, GWL_STYLE,
				GetWindowLong(gParams->ctl[n].hBuddy1, GWL_STYLE) | WS_BORDER);
			gParams->ctl[n].buddy1Style = WS_BORDER;

			SetWindowPos(gParams->ctl[n].hBuddy1, NULL, 0, 0, 0, 0,
                        SWP_FRAMECHANGED|SWP_NOZORDER|SWP_NOACTIVATE|SWP_NOSIZE|SWP_NOMOVE);

            // Turn off XP Visual Styles
            //setDefaultWindowTheme(hBuddy1);

			/*
			 hBuddy3 = CreateWindowEx(  
                0,//gParams->ctl[n].buddy1StyleEx, 
                "msctls_updown32",                      // predefined class 
                "",                            // control text
                WS_VISIBLE | WS_CHILD | WS_TABSTOP | UDS_ALIGNRIGHT | UDS_ARROWKEYS,
                0,                         // starting x position 
                0,                          // starting y position 
                0,                        // button width 
                0,                       // button height 
                fmc.hDlg,                       // parent window 
                (HMENU) (IDC_BUDDY1+n),         // control ID
                hDllInstance,
                NULL);                          // pointer not needed 

			SendMessage(hBuddy3, UDM_SETBUDDY, (WPARAM)gParams->ctl[n].hBuddy1, 0);
			*/

            //create the static text (label) buddy
            if (s & SBS_VERT || s & TBS_VERT) {
                //vertical scrollbar
                alignStyle = SS_CENTER;
            }
            else {
                //horizontal scrollbar
                alignStyle = SS_RIGHT;
            }
            alignStyle |= (s & WS_BORDER);  //OR in border style from main control
            gParams->ctl[n].sb.w2 = 0;      //dummy width
            gParams->ctl[n].sb.h2 = 0;      //dummy height
			gParams->ctl[n].sb.maxw2 = 0;   //dummy width
            gParams->ctl[n].sb.maxh2 = 0;   //dummy height
            computeBuddy2Pos(n, x, y, w, h, &r);

            hBuddy2 = CreateWindowEx(  
                gParams->ctl[n].buddy2StyleEx, //0,                              // extended styles
                "STATIC",                       // predefined class 
                "",                             // control text (set later...)
                WS_VISIBLE | WS_CHILD |
                SS_NOTIFY |                     // (So tooltips will work)
                gParams->ctl[n].buddy2Style | alignStyle,                     // styles
                r.left,                         // starting x position 
                r.top,                          // starting y position 
                r.right,                        // button width 
                r.bottom,                       // button height 
                fmc.hDlg,                       // parent window 
                (HMENU) (INT_PTR)(IDC_BUDDY2+n),         // control ID
				(HINSTANCE)hDllInstance,
                NULL);                          // pointer not needed 

            if (!hBuddy2) return FALSE;    //failed to create window.
            gParams->ctl[n].hBuddy2 = hBuddy2;

            // Turn off XP Visual Styles
            //setDefaultWindowTheme(hBuddy2);
			gParams->ctl[n].inuse = TRUE;
			// setCtlTheme(n, (int)gParams->ctl[n].theme);
			

            // Set control font to same as dialog font
            {
                HFONT hDlgFont;
                if ((hDlgFont = (HFONT)SendMessage(fmc.hDlg, WM_GETFONT, 0, 0L)) != NULL)
                {
                    gParams->ctl[n].hFont = hDlgFont;
                    SendMessage(hBuddy1, WM_SETFONT, (WPARAM)hDlgFont, 0L);  //redraw=FALSE
                    SendMessage(hBuddy2, WM_SETFONT, (WPARAM)hDlgFont, 0L);  //redraw=FALSE
                }
            }

            // Set the control text now, so the size of the static text control
            // can be computed correctly...
            if (gParams->ctl[n].label[0] != '\0') {
                gParams->ctl[n].inuse = TRUE;   //so setCtlText can work
                setCtlText(n, (char *)formatString(gParams->ctl[n].label));
            }
        }//scope
        break;

/*
    case 33333333: //CC_TRACKBAR:
        {
            //create a trackbar control
            //(and a NUMEDIT buddy.???)
            HWND hCtl;
            // Convert dialog units to pixels
            RECT r;

            // Should link dynamically, so only load comctl32.dll
            // if necessary!
            InitCommonControls();

            r.left = x;
            r.top = y;
            r.right = w;
            r.bottom = h;
            MapDialogRect(fmc.hDlg, &r);

            //Default tick frequency is 1.
            gParams->ctl[n].tb.ticFreq = 1;

            // force default styles (note TBS_HORZ==0 and TBS_BOTTOM==0).
            gParams->ctl[n].style |= WS_VISIBLE | WS_CHILD | WS_TABSTOP;

            // Transparent backgrounds don't work well for trackbars,
            // so change the default to opaque light gray.
            DeleteObject(gParams->ctl[n].hBrush); //superfluous for stock object
            gParams->ctl[n].hBrush = CreateSolidBrush(GetSysColor(COLOR_BTNFACE));//GetStockObject(LTGRAY_BRUSH);
            gParams->ctl[n].bkColor = GetSysColor(COLOR_BTNFACE);

            // create the trackbar control
            hCtl = CreateWindowEx(  
                gParams->ctl[n].styleEx,        // extended styles
                TRACKBAR_CLASS,                 // predefined class 
                "",                             // control text
                gParams->ctl[n].style,          // styles
                r.left,                         // starting x position 
                r.top,                          // starting y position 
                r.right,                        // control width 
                r.bottom,                       // control height 
                fmc.hDlg,                       // parent window 
                (HMENU) (INT_PTR)(IDC_CTLBASE+n),        // control ID
                hDllInstance,
                NULL);                          // pointer not needed 

            if (!hCtl) return FALSE;    //failed to create window.
            gParams->ctl[n].hCtl = hCtl;

            // Turn off XP Visual Styles
            //setDefaultWindowTheme(hCtl);
			gParams->ctl[n].inuse = TRUE;
			setCtlTheme(n, (int)gParams->ctl[n].theme);


            if (gParams->ctl[n].style & TBS_TOOLTIPS) {
                int side;
                //set side for automatic tooltips
                if (gParams->ctl[n].properties & CTP_BOTTOM) {
                    side = TBTS_BOTTOM;
                }
                else if (gParams->ctl[n].properties & CTP_RIGHT) {
                    side = TBTS_RIGHT;
                }
                else if (gParams->ctl[n].properties & CTP_LEFT) {
                    side = TBTS_LEFT;
                }
                else {
                    //default is TOP
                    side = TBTS_TOP;
                }
                SendMessage(gParams->ctl[n].hCtl, TBM_SETTIPSIDE, side, 0);
            }

#if 0
            // Set control font to same as dialog font
            {
                HFONT hDlgFont;
                if ((hDlgFont = (HFONT)SendMessage(fmc.hDlg, WM_GETFONT, 0, 0L)) != NULL)
                {
                    gParams->ctl[n].hFont = hDlgFont;
                    SendMessage(hCtl, WM_SETFONT, (WPARAM)hDlgFont, 0L);  //redraw=FALSE
                }
            }
#endif
        }
        break;
*/

    case CC_PUSHBUTTON:
    case CC_CHECKBOX:
    case CC_RADIOBUTTON:
    case CC_GROUPBOX:
    case CC_OWNERDRAW:
	    {
            //create a button control
            HWND hCtl;
            /* Convert dialog units to pixels */
            RECT r;

			r.left = x;
            r.top = y;
            r.right = w;
            r.bottom = h;
            MapDialogRect(fmc.hDlg, &r);
			
            // force default styles.
            gParams->ctl[n].style |= WS_VISIBLE | WS_CHILD;
            if (c != CC_RADIOBUTTON && c != CC_GROUPBOX) {
                //only first radio button in group should have TABSTOP.
                gParams->ctl[n].style |= WS_TABSTOP;
            }
#if 0
            if (c == CC_RADIOBUTTON && (gParams->ctl[n].style & WS_GROUP)) {
                //set TABSTOP on first radio button in GROUP???
                gParams->ctl[n].style |= WS_TABSTOP;
            }
#endif
#if 0
            if (c == CC_GROUPBOX) {
                //groupbox should always have GROUP style???
                gParams->ctl[n].style |= WS_GROUP;
            }
#endif
            gParams->ctl[n].maxval = 2;

			
            // create the button control
            hCtl = CreateWindowEx(  
                gParams->ctl[n].styleEx,        // extended styles
                "BUTTON",                       // predefined class 
                formatString(gParams->ctl[n].label),  // control text
                gParams->ctl[n].style,          // styles
                r.left,                         // starting x position 
                r.top,                          // starting y position 
                r.right,                        // control width 
                r.bottom,                       // control height 
                fmc.hDlg,                       // parent window 
                (HMENU) (INT_PTR)(IDC_CTLBASE+n),        // control ID
				(HINSTANCE)hDllInstance,
                NULL);                          // pointer not needed 

            if (!hCtl) return FALSE;    //failed to create window.
            gParams->ctl[n].hCtl = hCtl;

            // Turn off XP Visual Styles
            //setDefaultWindowTheme(hCtl);
			gParams->ctl[n].inuse = TRUE;
			// setCtlTheme(n, (int)gParams->ctl[n].theme);

            // Set control font to same as dialog font
            {
                HFONT hDlgFont;
                if ((hDlgFont = (HFONT)SendMessage(fmc.hDlg, WM_GETFONT, 0, 0L)) != NULL)
                {
                    gParams->ctl[n].hFont = hDlgFont;
                    SendMessage(hCtl, WM_SETFONT, (WPARAM)hDlgFont, 0L);  //redraw=FALSE
                }
            }
        }
        break;

    case CC_IMAGE:
        gParams->ctl[n].im.hdcMem = 0;
        gParams->ctl[n].im.hdcAnd = 0;
        gParams->ctl[n].im.hbm = 0;
        gParams->ctl[n].im.hbmOld = 0;
        gParams->ctl[n].im.hbmOldAnd = 0;
        gParams->ctl[n].style |= SS_OWNERDRAW;  //force ownerdraw style
        //fall through...
    case CC_STATICTEXT:
    case CC_FRAME:
    case CC_RECT:
    case CC_BITMAP:
        //gParams->ctl[n].style |= SS_BITMAP;  //force style
    case CC_ICON:
        //gParams->ctl[n].style |= SS_ICON;  //force style
    case CC_METAFILE:
        //gParams->ctl[n].style |= SS_ENHMETAFILE;  //force style
        {
            //create a static control
            HWND hCtl;
            /* Convert dialog units to pixels */
            RECT r;

            r.left = x;
            r.top = y;
            r.right = w;
            r.bottom = h;
            MapDialogRect(fmc.hDlg, &r);

            // force default styles.
            gParams->ctl[n].style |= WS_VISIBLE | WS_CHILD;

            // create the static control
            hCtl = CreateWindowEx(  
                gParams->ctl[n].styleEx,        // extended styles
                "STATIC",                       // predefined class 
                formatString(gParams->ctl[n].label),  // control text
                gParams->ctl[n].style,          // styles
                r.left,                         // starting x position 
                r.top,                          // starting y position 
                r.right,                        // control width 
                r.bottom,                       // control height 
                fmc.hDlg,                       // parent window 
                (HMENU) (INT_PTR)(IDC_CTLBASE+n),        // control ID
				(HINSTANCE)hDllInstance,
                NULL);                          // pointer not needed 

            if (!hCtl) return FALSE;    //failed to create window.
            gParams->ctl[n].hCtl = hCtl;

            // Turn off XP Visual Styles
            //setDefaultWindowTheme(hCtl);
			gParams->ctl[n].inuse = TRUE;
			// setCtlTheme(n, (int)gParams->ctl[n].theme);

            // Set control font to same as dialog font
            {
                HFONT hDlgFont;
                if ((hDlgFont = (HFONT)SendMessage(fmc.hDlg, WM_GETFONT, 0, 0L)) != NULL)
                {
                    gParams->ctl[n].hFont = hDlgFont;
                    SendMessage(hCtl, WM_SETFONT, (WPARAM)hDlgFont, 0L);  //redraw=FALSE
                }
            }
        }
        break;

    case CC_COMBOBOX:
        {
            //create a combo box control
            HWND hCtl;
            /* Convert dialog units to pixels */
            RECT r;

            r.left = x;
            r.top = y;
            r.right = w;
            r.bottom = h;
            MapDialogRect(fmc.hDlg, &r);

            // force default styles.
            gParams->ctl[n].style |= WS_VISIBLE | WS_CHILD | WS_TABSTOP;

            // Combo box-specific parameters
            gParams->ctl[n].val = -1;       //no item selected
            gParams->ctl[n].initVal = -1;   //no item selected
            gParams->ctl[n].minval = -1;
            gParams->ctl[n].maxval = 0xFFFF;    //max item index

            // create the combo box control
            hCtl = CreateWindowEx(  
                gParams->ctl[n].styleEx,        // extended styles
                "COMBOBOX",                     // predefined class 
                "",                             // control text
                gParams->ctl[n].style,          // styles
                r.left,                         // starting x position 
                r.top,                          // starting y position 
                r.right,                        // control width 
                r.bottom,                       // control height 
                fmc.hDlg,                       // parent window 
                (HMENU) (INT_PTR)(IDC_CTLBASE+n),        // control ID
				(HINSTANCE)hDllInstance,
                NULL);                          // pointer not needed 

            if (!hCtl) return FALSE;    //failed to create window.
            gParams->ctl[n].hCtl = hCtl;

            // Turn off XP Visual Styles
            //setDefaultWindowTheme(hCtl);
			gParams->ctl[n].inuse = TRUE;
			// setCtlTheme(n, (int)gParams->ctl[n].theme);


            if (gParams->ctl[n].properties & CTP_EXTENDEDUI) {
                // Set extended keyboard UI
                SendMessage(hCtl, CB_SETEXTENDEDUI, TRUE, 0);
            }

            // Set control font to same as dialog font
            {
                HFONT hDlgFont;
                if ((hDlgFont = (HFONT)SendMessage(fmc.hDlg, WM_GETFONT, 0, 0L)) != NULL)
                {
                    gParams->ctl[n].hFont = hDlgFont;
                    SendMessage(hCtl, WM_SETFONT, (WPARAM)hDlgFont, 0L);  //redraw=FALSE
                }
            }

            // Populate the list box from the control text string...
            gParams->ctl[n].inuse = TRUE;   //so setCtlText can work
            setCtlText(n, (char *)gParams->ctl[n].label);


			//Label
			{
                HWND hBuddy2;
                int alignStyle;
            
                alignStyle = SS_RIGHT;
                alignStyle |= (s & WS_BORDER);  //OR in border style from main control
                gParams->ctl[n].sb.w2 = 0;      //dummy width
                gParams->ctl[n].sb.h2 = 0;      //dummy height
				gParams->ctl[n].sb.maxw2 = 0;      //dummy width
                gParams->ctl[n].sb.maxh2 = 0;      //dummy height
                computeBuddy2Pos(n, x, y, w, h, &r);

                hBuddy2 = CreateWindowEx(  
                    gParams->ctl[n].buddy2StyleEx, //0,                              // extended styles
                    "STATIC",                       // predefined class 
                    "",                             // control text (set later...)
                    WS_VISIBLE | WS_CHILD |
                    SS_NOTIFY |                     // (So tooltips will work)
                    gParams->ctl[n].buddy2Style | alignStyle,                     // styles
                    r.left,                         // starting x position 
                    r.top,                          // starting y position 
                    r.right,                        // button width 
                    r.bottom,                       // button height 
                    fmc.hDlg,                       // parent window 
                    (HMENU) (INT_PTR)(IDC_BUDDY2+n),         // control ID
					(HINSTANCE)hDllInstance,
                    NULL);                          // pointer not needed 

                if (!hBuddy2) return FALSE;    //failed to create window.
                gParams->ctl[n].hBuddy2 = hBuddy2;

                // Turn off XP Visual Styles
                //setDefaultWindowTheme(hBuddy2);
			    gParams->ctl[n].inuse = TRUE;
			    // setCtlTheme(n, (int)gParams->ctl[n].theme);
			    
                // Set control font to same as dialog font
                {
                    HFONT hDlgFont;
                    if ((hDlgFont = (HFONT)SendMessage(fmc.hDlg, WM_GETFONT, 0, 0L)) != NULL)
                    {
                        gParams->ctl[n].hFont = hDlgFont;
                        //SendMessage(hBuddy1, WM_SETFONT, (WPARAM)hDlgFont, 0L);  //redraw=FALSE
                        SendMessage(hBuddy2, WM_SETFONT, (WPARAM)hDlgFont, 0L);  //redraw=FALSE
                    }
                }

                // Set the control text now, so the size of the static text control
                // can be computed correctly...
                /*if (gParams->ctl[n].label2[0] != '\0') {
                    gParams->ctl[n].inuse = TRUE;   //so setCtlText can work
                    setCtlLabel(n, (char *)formatString(gParams->ctl[n].label2));
                }*/
            }

        }
        break;

    case CC_LISTBOX:
    case CC_LISTBAR:
        {
            //create a list box control
            HWND hCtl;
            /* Convert dialog units to pixels */
            RECT r;

            r.left = x;
            r.top = y;
            r.right = w;
            r.bottom = h;
            MapDialogRect(fmc.hDlg, &r);

            // force default styles.
            gParams->ctl[n].style |= LBS_NOTIFY | WS_VISIBLE | WS_CHILD | WS_TABSTOP;
            if (c==CC_LISTBAR) {
                gParams->ctl[n].style |= LBS_MULTICOLUMN; //| WS_BORDER; 
                //gParams->ctl[n].styleEx |= WS_EX_CLIENTEDGE;
            }

            // List box-specific parameters
            gParams->ctl[n].val = -1;       //no item selected
            gParams->ctl[n].initVal = -1;   //no item selected
            gParams->ctl[n].minval = -1;
            gParams->ctl[n].maxval = 0xFFFF;    //max item index

			DeleteObject(gParams->ctl[n].hBrush); //superfluous for stock object
			gParams->ctl[n].hBrush = CreateSolidBrush(GetSysColor(COLOR_BTNFACE));
			gParams->ctl[n].bkColor = GetSysColor(COLOR_BTNFACE);
			
            // create the list box control
            hCtl = CreateWindowEx(  
                gParams->ctl[n].styleEx,        // extended styles
                "LISTBOX",                      // predefined class 
                "",                             // control text
                gParams->ctl[n].style,          // styles
                r.left,                         // starting x position 
                r.top,                          // starting y position 
                r.right,                        // control width 
                r.bottom,                       // control height 
                fmc.hDlg,                       // parent window 
                (HMENU) (INT_PTR)(IDC_CTLBASE+n),        // control ID
				(HINSTANCE)hDllInstance,
                NULL);                          // pointer not needed 

            if (!hCtl) return FALSE;    //failed to create window.
            gParams->ctl[n].hCtl = hCtl;

            // Turn off XP Visual Styles
            //setDefaultWindowTheme(hCtl);
			gParams->ctl[n].inuse = TRUE;
			// setCtlTheme(n, (int)gParams->ctl[n].theme);

#if 0
            {//scope
                RECT rc;
                GetWindowRect(hCtl, &rc);
                Info("Set to (%d,%d,%d,%d)\n"
                     "Adjusted to (%d,%d,%d,%d)",
                     r.left, r.top, r.right, r.bottom,
                     rc.left, rc.top, rc.right, rc.bottom);
            }//scope
#endif

            // Set control font to same as dialog font
            {
                HFONT hDlgFont;
                if ((hDlgFont = (HFONT)SendMessage(fmc.hDlg, WM_GETFONT, 0, 0L)) != NULL)
                {
                    gParams->ctl[n].hFont = hDlgFont;
                    SendMessage(hCtl, WM_SETFONT, (WPARAM)hDlgFont, 0L);  //redraw=FALSE
                }
            }

            // Populate the list box from the control text string...
            gParams->ctl[n].inuse = TRUE;   //so setCtlText can work
            setCtlText(n, (char *)gParams->ctl[n].label);

            
           //Label
           if (c==CC_LISTBAR) {

                HWND hBuddy2;
                int alignStyle;
            
                alignStyle = SS_RIGHT;
                alignStyle |= (s & WS_BORDER);  //OR in border style from main control
                gParams->ctl[n].sb.w2 = 0;      //dummy width
                gParams->ctl[n].sb.h2 = 0;      //dummy height
                computeBuddy2Pos(n, x, y, w, h, &r);

                hBuddy2 = CreateWindowEx(  
                    gParams->ctl[n].buddy2StyleEx, //0,                              // extended styles
                    "STATIC",                       // predefined class 
                    "",                             // control text (set later...)
                    WS_VISIBLE | WS_CHILD |
                    SS_NOTIFY |                     // (So tooltips will work)
                    gParams->ctl[n].buddy2Style | alignStyle,                     // styles
                    r.left,                         // starting x position 
                    r.top,                          // starting y position 
                    r.right,                        // button width 
                    r.bottom,                       // button height 
                    fmc.hDlg,                       // parent window 
                    (HMENU) (INT_PTR)(IDC_BUDDY2+n),         // control ID
					(HINSTANCE)hDllInstance,
                    NULL);                          // pointer not needed 

                if (!hBuddy2) return FALSE;    //failed to create window.
                gParams->ctl[n].hBuddy2 = hBuddy2;

                // Turn off XP Visual Styles
                //setDefaultWindowTheme(hBuddy2);
			    gParams->ctl[n].inuse = TRUE;
			    // setCtlTheme(n, (int)gParams->ctl[n].theme);
			    
                // Set control font to same as dialog font
                {
                    HFONT hDlgFont;
                    if ((hDlgFont = (HFONT)SendMessage(fmc.hDlg, WM_GETFONT, 0, 0L)) != NULL)
                    {
                        gParams->ctl[n].hFont = hDlgFont;
                        //SendMessage(hBuddy1, WM_SETFONT, (WPARAM)hDlgFont, 0L);  //redraw=FALSE
                        SendMessage(hBuddy2, WM_SETFONT, (WPARAM)hDlgFont, 0L);  //redraw=FALSE
                    }
                }

                // Set the control text now, so the size of the static text control
                // can be computed correctly...
                /*if (gParams->ctl[n].label2[0] != '\0') {
                    gParams->ctl[n].inuse = TRUE;   //so setCtlText can work
                    setCtlLabel(n, (char *)formatString(gParams->ctl[n].label2));
                }*/
            
            }


        }
        break;

	case CC_LISTVIEW:
		{
			HWND hCtl;
            RECT r;
			INITCOMMONCONTROLSEX icex;           // Structure for control initialization.

			icex.dwICC = ICC_LISTVIEW_CLASSES;
			InitCommonControlsEx(&icex);

            r.left = x;
            r.top = y;
            r.right = w;
            r.bottom = h;
            MapDialogRect(fmc.hDlg, &r);

            // force default styles.
            gParams->ctl[n].style |= WS_CHILD | LVS_ICON | LVS_NOCOLUMNHEADER | LVS_ALIGNTOP | LVS_AUTOARRANGE; //LVS_EDITLABELS; //LVS_REPORT
			//if (getAppTheme() && getWindowsVersion()>=11)
				gParams->ctl[n].style |= LVS_NOLABELWRAP; //Works fine under Vista/7 but not under XP or without visual styles
			
            // List box-specific parameters
            gParams->ctl[n].val = -1;       //no item selected
            gParams->ctl[n].initVal = -1;   //no item selected
            gParams->ctl[n].minval = -1;
            gParams->ctl[n].maxval = 0xFFFF;    //max item index

			DeleteObject(gParams->ctl[n].hBrush); //superfluous for stock object
			gParams->ctl[n].hBrush = CreateSolidBrush(GetSysColor(COLOR_BTNFACE));
			gParams->ctl[n].bkColor = GetSysColor(COLOR_BTNFACE);
			
            // create the list box control
            hCtl = CreateWindowEx(  
                gParams->ctl[n].styleEx,        // extended styles
                WC_LISTVIEW,                    // predefined class 
                "",                            // control text
                gParams->ctl[n].style,          // styles
                r.left,                         // starting x position 
                r.top,                          // starting y position 
                r.right,                        // control width 
                r.bottom,                       // control height 
                fmc.hDlg,                       // parent window 
                (HMENU) (INT_PTR)(IDC_CTLBASE+n),        // control ID
				(HINSTANCE)hDllInstance,
                NULL);                          // pointer not needed 

			if (!hCtl) return FALSE;    //failed to create window.
            gParams->ctl[n].hCtl = hCtl;


			ListView_SetExtendedListViewStyle(hCtl,LVS_EX_DOUBLEBUFFER|LVS_EX_FULLROWSELECT); //|LVS_EX_LABELTIP
			
			//if (getAppTheme() && getWindowsVersion()<11) ListView_SetExtendedListViewStyle(hCtl,LVS_EX_BORDERSELECT); //Permanently deactivates full row select under XP

			//Insert column
			{
				LVCOLUMN lvc;
				lvc.mask = LVCF_WIDTH; //  LVCF_FMT | LVCF_TEXT | LVCF_SUBITEM;
				//lvc.cx = HDBUsToPixels(w)+10;
				lvc.cx = r.right+9;
				//Info ("%d,%d",r.right,r.left);
				ListView_InsertColumn (hCtl,0,&lvc);
				//ListView_SetColumnWidth(hCtl,0,LVSCW_AUTOSIZE);
			}

			gParams->ctl[n].inuse = TRUE;

			// Turn off XP Visual Styles
			// setCtlTheme(n, (int)gParams->ctl[n].theme);

            // Set control font to same as dialog font
            {
                HFONT hDlgFont;
                if ((hDlgFont = (HFONT)SendMessage(fmc.hDlg, WM_GETFONT, 0, 0L)) != NULL)
                {
                    gParams->ctl[n].hFont = hDlgFont;
                    SendMessage(hCtl, WM_SETFONT, (WPARAM)hDlgFont, 0L);  //redraw=FALSE
                }
            }

            // Populate the list box from the control text string...
            gParams->ctl[n].inuse = TRUE;   //so setCtlText can work
            //setCtlText(n, (char *)gParams->ctl[n].label);


			//Subclass
			if (getAppTheme()){
				WNDPROC OldProc = (WNDPROC)SetWindowLongPtr(hCtl,GWLP_WNDPROC,(LONG_PTR)ListviewSubclassProc);
				SetWindowLongPtr(hCtl,GWLP_USERDATA,(LONG_PTR)OldProc);
			}

		}
		break;	

    case CC_EDIT:
		{
            //create a edit control
            HWND hCtl;
            /* Convert dialog units to pixels */
            RECT r;

            r.left = x;
            r.top = y;
            r.right = w;
            r.bottom = h;
            MapDialogRect(fmc.hDlg, &r);

            // force default styles.
            gParams->ctl[n].style |= WS_VISIBLE | WS_CHILD | WS_TABSTOP;

            // create the static control
            hCtl = CreateWindowEx(  
                gParams->ctl[n].styleEx,        // extended styles
                "EDIT",                       // predefined class 
                "",//formatString(gParams->ctl[n].label), // control text
                gParams->ctl[n].style,          // styles
                r.left,                         // starting x position 
                r.top,                          // starting y position 
                r.right,                        // control width 
                r.bottom,                       // control height 
                fmc.hDlg,                       // parent window 
                (HMENU) (INT_PTR)(IDC_CTLBASE+n),        // control ID
				(HINSTANCE)hDllInstance,
                NULL);                          // pointer not needed 

            if (!hCtl) return FALSE;    //failed to create window.
            gParams->ctl[n].hCtl = hCtl;

            // Turn off XP Visual Styles
            gParams->ctl[n].inuse = TRUE;
			// setCtlTheme(n, (int)gParams->ctl[n].theme);

            // Set control font to same as dialog font
            {
                HFONT hDlgFont;
                if ((hDlgFont = (HFONT)SendMessage(fmc.hDlg, WM_GETFONT, 0, 0L)) != NULL)
                {
                    gParams->ctl[n].hFont = hDlgFont;
                    SendMessage(hCtl, WM_SETFONT, (WPARAM)hDlgFont, 0L);  //redraw=FALSE
                }
            }
        }
        break;

	case CC_TAB:
		{
            //create a tab control
            HWND hCtl;
            /* Convert dialog units to pixels */
            RECT r;
			//TCITEM tabItem; 

			
			InitCommonControls(); 

            r.left = x;
            r.top = y;
            r.right = w;
            r.bottom = h;
            MapDialogRect(fmc.hDlg, &r);

            // force default styles.
            gParams->ctl[n].style |= WS_CHILD | WS_CLIPSIBLINGS | WS_VISIBLE;// | WS_CLIPCHILDREN;

            // create the static control
            hCtl = CreateWindowEx(  
                gParams->ctl[n].styleEx,        // extended styles
                WC_TABCONTROL,                  // predefined class 
                "",//formatString(gParams->ctl[n].label), // control text
                gParams->ctl[n].style,          // styles
                r.left,                         // starting x position 
                r.top,                          // starting y position 
                r.right,                        // control width 
                r.bottom,                       // control height 
                fmc.hDlg,                       // parent window 
                (HMENU) (INT_PTR)(IDC_CTLBASE+n),        // control ID
				(HINSTANCE)hDllInstance,
                NULL);                          // pointer not needed 

            if (!hCtl) return FALSE;    //failed to create window.
            gParams->ctl[n].hCtl = hCtl;

			//TabCtrl_SetMinTabWidth (gParams->ctl[n].hCtl, 20);

            // Turn off XP Visual Styles
            gParams->ctl[n].inuse = TRUE;
			// setCtlTheme(n, (int)gParams->ctl[n].theme);

            // Set control font to same as dialog font
            {
                HFONT hDlgFont;
                if ((hDlgFont = (HFONT)SendMessage(fmc.hDlg, WM_GETFONT, 0, 0L)) != NULL)
                {
                    gParams->ctl[n].hFont = hDlgFont;
                    SendMessage(hCtl, WM_SETFONT, (WPARAM)hDlgFont, 0L);  //redraw=FALSE
                }
            }

            setCtlText(n,t);

			//Add tab button
			//tabItem.mask = TCIF_TEXT | TCIF_IMAGE; 
			//tabItem.iImage = -1; 
			//tabItem.pszText = "Tab1"; 

			//TabCtrl_InsertItem(hCtl, 0, &tabItem);

			//Subclass
			/*if (getAppTheme()){
				WNDPROC OldProc = (WNDPROC)SetWindowLongPtr(hCtl,GWLP_WNDPROC,(LONG_PTR)TabSubclassProc);
				SetWindowLongPtr(hCtl,GWLP_USERDATA,(LONG_PTR)OldProc);
			}*/

        }
        break;

	case CC_TOOLBAR:
		{

			//create a tab control
            HWND hCtl;
            RECT r;
			//HWND hBuddy2;
			
			InitCommonControls(); 

			//Info ("%d, %d\n%d x %d",x,y,w,h);

            r.left = x;
            r.top = y;
            r.right = w;
            r.bottom = h;
            MapDialogRect(fmc.hDlg, &r);

			/*
			//Use static window for embedding the tool bar in order to position it
			hBuddy2 = CreateWindowEx(  
                0, //gParams->ctl[n].buddy2StyleEx, //0,                              // extended styles
                "STATIC",                       // predefined class 
                "",                             // control text (set later...)
                WS_VISIBLE | WS_CHILD |
                //SS_NOTIFY |                      // (So tooltips will work)
                0, //gParams->ctl[n].buddy2Style,// | alignStyle,                     // styles
                r.left,                         // starting x position 
                r.top,                          // starting y position 
                r.right,                        // button width 
                r.bottom,                       // button height 
                fmc.hDlg,                       // parent window 
                (HMENU) (INT_PTR)(IDC_BUDDY2+n),         // control ID
                hDllInstance,
                NULL);                          // pointer not needed 

            if (!hBuddy2) return FALSE;    //failed to create window.
            gParams->ctl[n].hBuddy2 = hBuddy2;
			*/

			//gParams->ctl[n].bkColor = getSysColor(COLOR_BTNFACE);


            // force default styles.
            gParams->ctl[n].style |= WS_CHILD | WS_VISIBLE | TBSTYLE_FLAT | TBSTYLE_LIST | CCS_NODIVIDER | CCS_NOPARENTALIGN | CCS_NORESIZE;// TBSTYLE_WRAPABLE | | TBSTYLE_FLAT | CCS_NOPARENTALIGN | CCS_NORESIZE;  | WS_CLIPSIBLINGS
			//if (!getAppTheme()) gParams->ctl[n].style |= TBSTYLE_FLAT; // | CCS_NOPARENTALIGN | CCS_NORESIZE;

            // create the static control
            hCtl = CreateWindowEx(  
                gParams->ctl[n].styleEx,        // extended styles
                TOOLBARCLASSNAME,                  // predefined class 
                "",//formatString(gParams->ctl[n].label), // control text
                gParams->ctl[n].style,          // styles
                r.left,                         // starting x position 
                r.top,                          // starting y position 
                r.right,                        // control width 
                r.bottom,                       // control height 
				fmc.hDlg, //gParams->ctl[n].hBuddy2, //getAppTheme()?gParams->ctl[n].hBuddy2:fmc.hDlg, //fmc.hDlg,                       // parent window 
                (HMENU) (INT_PTR)(IDC_CTLBASE+n),        // control ID
				(HINSTANCE)hDllInstance,
                NULL);                          // pointer not needed 

            if (!hCtl) return FALSE;    //failed to create window.
            gParams->ctl[n].hCtl = hCtl;

			//Redirect messages to main window
			SendMessage(hCtl, TB_SETPARENT, (WPARAM)fmc.hDlg, 0);

			//TabCtrl_SetMinTabWidth (gParams->ctl[n].hCtl, 20);
			//SendMessage(hCtl, TB_SETBUTTONWIDTH, 0, MAKELPARAM(50,50));  //redraw=FALSE
			
            // Turn off XP Visual Styles
            gParams->ctl[n].inuse = TRUE;
			// setCtlTheme(n, (int)gParams->ctl[n].theme);

            // Set control font to same as dialog font
            {
                HFONT hDlgFont;
                if ((hDlgFont = (HFONT)SendMessage(fmc.hDlg, WM_GETFONT, 0, 0L)) != NULL)
                {
                    gParams->ctl[n].hFont = hDlgFont;
                    SendMessage(hCtl, WM_SETFONT, (WPARAM)hDlgFont, 0L);  //redraw=FALSE
                }
            }

            setCtlText(n,t);

		}
        break;

    case CC_UNUSED:
    case CC_SPINNER:
    case CC_UPDOWN:
    case CC_PROGRESSBAR:
	case CC_TOOLTIP:
    case CC_ANIMATION:
    case CC_PREVIEW:
    default:
        break;
    } //switch

    gParams->ctl[n].inuse = TRUE;
    gParams->ctl[n].tabControl = -1;    //no tab control
    gParams->ctl[n].tabSheet = -1;      //no tab sheet
    setCtlRange(n, gParams->ctl[n].minval, gParams->ctl[n].maxval);
    setCtlLineSize(n, gParams->ctl[n].lineSize);
    setCtlPageSize(n, gParams->ctl[n].pageSize);
    setCtlVal(n, gParams->ctl[n].initVal);
    enableCtl(n, e); // visible/enabled
	setCtlAnchor (n,ANCHOR_RIGHT);

    // Set default tooltips for predefined controls...
    switch (n) {

/*
    //Don't need them in my FocalBlade Plugin
    case CTL_OK:
        setCtlToolTip(n, (INT_PTR)"Apply filter to main image", 0);
        break;
    case CTL_CANCEL:
        setCtlToolTip(n, (INT_PTR)"Exit without applying filter", 0);
        break;
*/

    case CTL_LOGO:
        setCtlToolTip(n, "About this filter", 0);
        break;
    case CTL_PREVIEW:
    case CTL_PROGRESS:
    case CTL_HOST:
    default:
        break;
    }//switch n

    // update cached control values...
#if 0
    fmc.pre_ctl[n] = gParams->ctl[n].val;   //already done by setCtlVal()
#endif
    return TRUE;    //success
} /*createCtl*/

/**********************************************************************/
/* initCtl(n)
/*
/*  Initializes control n to default values.
/*
/**********************************************************************/
int initCtl(int n)
{
    char szBuffer[1024];

    if (n < 0 || n >= N_CTLS) {
        return FALSE;
    }

    sprintf(szBuffer, "Control &%d:", n);
    createCtl(n,
                 CC_STANDARD,
                 szBuffer,
#if 0
                 270 - (n/10 % 10)*5,   //x
                 10*(n % 10) + 5,       //y
                 90,                    //w
                 8,                     //h
#else
                 -1,                    //x default
                 -1,                    //y default
                 -1,                    //h default
                 -1,                    //w default
#endif
                 0,                     //styles
                 0,                     //extended styles
                 0,                     //properties
                 3);                    //visible/enabled
    return TRUE;
} /*fm_initCtl*/


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
int setDialogPos(int fAbs, int x, int y, int w, int h)
{
    UINT flags = SWP_NOZORDER | SWP_NOACTIVATE;
    HWND hParent;
    int  nHeight;
    int  nWidth;
    RECT rcDialog;
    RECT rcParent;
    RECT workArea;
    int  xScreen;
    int  yScreen;

#if 0
    Info("setDialogPos(%d, %d, %d, %d, %d)", fAbs, x, y, w, h);
#endif

    /* Convert dialog units to pixels */
    {
        RECT r;
        r.left = x;
        r.top = y;
        r.right = w;
        r.bottom = h;
        MapDialogRect(fmc.hDlg, &r);
        // Need to add SM_CYCAPTION, SM_CXDLGFRAME, etc., depending on dialog box style?
        // Or could add the difference between dialog window size and dialog client size,
        // so it automatically adjusts for caption/no caption, border style, etc.?
        if (x >= 0) x = r.left;
        if (y >= 0) y = r.top;
#if 1
        GetWindowRect(fmc.hDlg, &rcDialog);
        GetClientRect(fmc.hDlg, &rcParent);
        if (w >= 0) w = r.right + (rcDialog.right-rcDialog.left) - (rcParent.right-rcParent.left);
        if (h >= 0) h = r.bottom + (rcDialog.bottom-rcDialog.top) - (rcParent.bottom-rcParent.top);

#else
        if (w >= 0) w = r.right + 2*GetSystemMetrics(SM_CXDLGFRAME);
        if (h >= 0) h = r.bottom + 2*GetSystemMetrics(SM_CYDLGFRAME) + GetSystemMetrics(SM_CYCAPTION);
#endif
    }

    if (w < 0 || h < 0) {
        // don't resize the dialog
        flags |= SWP_NOSIZE;
        GetWindowRect(fmc.hDlg, &rcDialog);
        nWidth  = rcDialog.right  - rcDialog.left;
        nHeight = rcDialog.bottom - rcDialog.top;
    }
    else {
        nWidth = w;
        nHeight = h;
        //could optimize following by doing it only for stretched images
        //and vertical gradients...
        flags |= SWP_NOCOPYBITS;  //so background will be redrawn when changing size
    }

    if (x == -1 && y == -1) {
        // center the dialog on screen if fAbs (or no parent);
        // else center in client area of parent.
        if  (fAbs || !(hParent = GetParent(fmc.hDlg)))
            hParent = GetDesktopWindow();

        GetClientRect(hParent, &rcParent);
        ClientToScreen(hParent, (LPPOINT)&rcParent.left);  // point(left,  top)
        ClientToScreen(hParent, (LPPOINT)&rcParent.right); // point(right, bottom)

        
        //Get Screen area without the task bar 
        //-> the bottom of the dialog isn't hidden by the taskbar anymore
        SystemParametersInfo(SPI_GETWORKAREA, 0, &workArea, 0);

        x = max(rcParent.right - rcParent.left - nWidth, 0) / 2
            + rcParent.left;
        //xScreen = GetSystemMetrics(SM_CXSCREEN);
        xScreen = workArea.right - workArea.left;
        if  (x + nWidth > xScreen)
            x = max (0, xScreen - nWidth);

        y = max(rcParent.bottom - rcParent.top - nHeight, 0) / 3
            + rcParent.top;        
        //yScreen = GetSystemMetrics(SM_CYSCREEN);
        yScreen = workArea.bottom - workArea.top;
        if  (y + nHeight > yScreen)
            y = max(0 , yScreen - nHeight);
    }
    else if (x < 0 || y < 0) {
        // don't move the dialog
        flags |= SWP_NOMOVE;
    }
    else if (!fAbs) {
        // Compute coords relative to client area of parent...
        if  ( ! (hParent = GetParent(fmc.hDlg)))
            hParent = GetDesktopWindow();

        GetClientRect(hParent, &rcParent);
        ClientToScreen(hParent, (LPPOINT)&rcParent.left);  // point(left,  top)

        x += rcParent.left;
        y += rcParent.top;

    }


    return SetWindowPos(fmc.hDlg, NULL,
                        x, y, nWidth, nHeight,
                        flags);

} /*fm_setDialogPos*/


/**********************************************************************/
/* setDialogMaxMin()
/*  Set the minum and maximum size of the dialog
/*
/**********************************************************************/
int setDialogMinMax(int mintrackX, int mintrackY, int maxtrackX, int maxtrackY) //int maxsizeX,int maxsizeY,int maxposX, int maxposY,
{

    RECT rcDialog;
    RECT rcParent;
    int borderWidth, borderHeight;

    //Calulate window border width and height to add to MaxMinInfo
    GetWindowRect(fmc.hDlg, &rcDialog);
    GetClientRect(fmc.hDlg, &rcParent);
    borderWidth = (rcDialog.right-rcDialog.left) - (rcParent.right-rcParent.left);
    borderHeight = (rcDialog.bottom-rcDialog.top) - (rcParent.bottom-rcParent.top);
	
    //Set sizes
	if (mintrackX<=0) 
        MaxMinInfo.ptMinTrackSize.x = 0;
    else
        MaxMinInfo.ptMinTrackSize.x = HDBUsToPixels(mintrackX) + borderWidth;

	if (mintrackY<=0) 
        MaxMinInfo.ptMinTrackSize.y =0;
    else
        MaxMinInfo.ptMinTrackSize.y = VDBUsToPixels(mintrackY) + borderHeight;

	if (maxtrackX<=0) 
        MaxMinInfo.ptMaxTrackSize.x = 0;
    else
        MaxMinInfo.ptMaxTrackSize.x = HDBUsToPixels(maxtrackX) + borderWidth; 

	if (maxtrackY<=0) 
        MaxMinInfo.ptMaxTrackSize.y = 0;
    else
        MaxMinInfo.ptMaxTrackSize.y = VDBUsToPixels(maxtrackY) + borderHeight;

    return true;

}/*fm_getDialogWidth*/



int getDialogPos(int w, int t)
{

	RECT rc;
	int val;

	if (t==0){ //Full window
		
		GetWindowRect(fmc.hDlg, &rc);

	} else { //Client area

		GetClientRect(fmc.hDlg, &rc);
	}

	switch (w){
		case 0: val = PixelsToHDBUs(rc.left); break;
		case 1: val = PixelsToVDBUs(rc.top); break;
		case 2: val = PixelsToHDBUs(rc.right - rc.left); break;
		case 3: val = PixelsToVDBUs(rc.bottom - rc.top); break;
	}

	return val;

}


/**********************************************************************/
/* getDialogWidth()
/*  Gets the width of the client area of the dialog.
/*
/**********************************************************************/
int getDialogWidth(void)
{
    RECT rcParent;
      
    //Get Pixel values
    GetClientRect(fmc.hDlg, &rcParent);
    //Convert to DBU
    return PixelsToHDBUs(rcParent.right);

}/*fm_getDialogWidth*/

int getDialogPixelWidth(void)
{
    RECT rcParent;
    GetClientRect(fmc.hDlg, &rcParent);
    return rcParent.right;
}

/**********************************************************************/
/* getDialogHeight()
/*  Gets the height of the client area of the dialog.
/*
/**********************************************************************/
int getDialogHeight(void)
{
    RECT rcParent;

    //Get Pixel values
    GetClientRect(fmc.hDlg, &rcParent);
    //Convert to DBU
    return PixelsToVDBUs(rcParent.bottom);

}/*fm_getDialogHeight*/

int getDialogPixelHeight(void)
{
    RECT rcParent;
    GetClientRect(fmc.hDlg, &rcParent);
    return rcParent.bottom;
}


/**********************************************************************/
/* setDialogStyle(TitleBar |...)
/*  Sets various styles of the dialog.
/*
/**********************************************************************/
int setDialogStyle(int flags)
{
#if 0
    //disallow MaxBox for now -- it allows dialog to be maximized by
    //double-clicking the titlebar (NoSysMenu must also be specified).
    //The problem is that if you exit while maximized, then re-invoke,
    //it comes back in normal mode but with max size, so can't then
    //"unmaximize" the dialog.  Yecch.
    flags &= ~WS_MAXIMIZEBOX;
#endif
    SetWindowLong(fmc.hDlg, GWL_STYLE,
        GetWindowLong(fmc.hDlg, GWL_STYLE) | flags);
    //use SWP_FRAMECHANGED to force nonclient area update...
    return SetWindowPos(fmc.hDlg, NULL,
                        0, 0, 0, 0,
                        SWP_FRAMECHANGED|SWP_NOZORDER|SWP_NOACTIVATE|SWP_NOSIZE|SWP_NOMOVE);
} /*fm_setDialogStyle*/

/**********************************************************************/
/* clearDialogStyle(TitleBar |...)
/*  Clears various styles of the dialog.
/*
/**********************************************************************/
int clearDialogStyle(int flags)
{
    SetWindowLong(fmc.hDlg, GWL_STYLE,
        GetWindowLong(fmc.hDlg, GWL_STYLE) & ~flags);
    //use SWP_FRAMECHANGED to force nonclient area update...
    return SetWindowPos(fmc.hDlg, NULL,
                        0, 0, 0, 0,
                        SWP_FRAMECHANGED|SWP_NOZORDER|SWP_NOACTIVATE|SWP_NOSIZE|SWP_NOMOVE);
} /*fm_clearDialogStyle*/

// !!!! Should recalc Edit-mode dialog size when TOOLWINDOW is set/cleared
// !!!! and when dialog is resized!!!!


/**********************************************************************/
/* setDialogStyleEx(ToolWindow |...)
/*  Sets various extended styles of the dialog.
/*
/**********************************************************************/
int setDialogStyleEx(int flags)
{
    SetWindowLong(fmc.hDlg, GWL_EXSTYLE,
        GetWindowLong(fmc.hDlg, GWL_EXSTYLE) | flags);
    //use SWP_FRAMECHANGED to force nonclient area update...
    return SetWindowPos(fmc.hDlg, NULL,
                        0, 0, 0, 0,
                        SWP_FRAMECHANGED|SWP_NOZORDER|SWP_NOACTIVATE|SWP_NOSIZE|SWP_NOMOVE);
} /*fm_setDialogStyleEx*/

/**********************************************************************/
/* clearDialogStyleEx(ToolWindow |...)
/*  Clears various extended styles of the dialog.
/*
/**********************************************************************/
int clearDialogStyleEx(int flags)
{
    SetWindowLong(fmc.hDlg, GWL_EXSTYLE,
        GetWindowLong(fmc.hDlg, GWL_EXSTYLE) & ~flags);
    //use SWP_FRAMECHANGED to force nonclient area update...
    return SetWindowPos(fmc.hDlg, NULL,
                        0, 0, 0, 0,
                        SWP_FRAMECHANGED|SWP_NOZORDER|SWP_NOACTIVATE|SWP_NOSIZE|SWP_NOMOVE);
} /*fm_clearDialogStyleEx*/

/**********************************************************************/
/* setDialogText("title")
/*  Sets the caption in the title bar.
/*
/**********************************************************************/
int setDialogText(char * title)
{
    strncpy(gParams->gDialogTitle, (char *)title, MAX_LABEL_SIZE+1);
    gParams->gDialogTitle[MAX_LABEL_SIZE] = '\0';   //ensure null-terminated
    return SetWindowText(fmc.hDlg, formatString(gParams->gDialogTitle));
} /*fm_setDialogText*/

/**********************************************************************/
/* setDialogTextv("format", ...)
/*  Sets the caption in the title bar with printf-style formatting.
/*
/**********************************************************************/
int setDialogTextv(char * iFmt,...)
{
    char szBuffer[1024];
    va_list ap;

    va_start(ap, iFmt);
    _vsnprintf(szBuffer, sizeof(szBuffer), (char *)iFmt, ap);
    szBuffer[sizeof(szBuffer)-1] = 0;   //ensure NUL-terminated
    va_end(ap);
    return setDialogText(szBuffer);
} /*fm_setDialogTextv*/

/**********************************************************************/
/* setDialogGradient(color1,color2,direction)
/*  Fills the background of the client area of the dialog box with
/*  a vertical (direction=0) or horizontal (direction=1) linear gradient.
/*
/**********************************************************************/
int setDialogGradient(int color1, int color2, int direction)
{
    if (gParams->gDialogGradientColor1 == (COLORREF)color1 &&
        gParams->gDialogGradientColor2 == (COLORREF)color2 &&
        gParams->gDialogGradientDirection == direction) {
        //quick exit if no change...
        return TRUE;
    }

    gParams->gDialogGradientColor1 = color1;
    gParams->gDialogGradientColor2 = color2;
    gParams->gDialogGradientDirection = direction;
    /* Force the background to be redrawn */
#if 0
    InvalidateRect(fmc.hDlg,
                          NULL, /*entire client area */
                          TRUE); /*erase background*/
    return UpdateWindow(fmc.hDlg);   //force a WM_PAINT
#elif 1
    return RedrawWindow(
            fmc.hDlg,   // handle of window
            NULL,   // address of structure with update rectangle, NULL=>entire client area
            NULL,   // handle of update region
            RDW_ERASE|RDW_INVALIDATE|RDW_ERASENOW|RDW_UPDATENOW     // array of redraw flags
       );
#else
    //What if just send WM_ERASEBKGND???
    return (int)SendMessage(fmc.hDlg, WM_ERASEBKGND, 0, 0 );
#endif
} /*fm_setDialogGradient*/

/**********************************************************************/
/* setDialogColor(color)
/*  Sets the background color for the client area of the dialog box.
/*
/**********************************************************************/
int setDialogColor(int color)
{
    return setDialogGradient(color, color, 0);
} /*fm_setDialogColor*/

int setDialogSysColor(int con)
{
	int color = (int)GetSysColor(con);
    return setDialogGradient(color, color, 0);
} /*fm_setDialogSysColor*/

int getDialogColor()
{
	return gParams->gDialogGradientColor1;
}


/**********************************************************************/
/* setDialogImage(filename)
/*  Applies an image from the specified file as the background image
/*  for the client area of the dialog box, tiling or stretching it to
/*  fit if requested by setDialogImageMode.
/*
/**********************************************************************/
int setDialogImage(char * filename)
{
    strncpy(gParams->gDialogImageFileName, (char *)filename, _MAX_PATH+1);
    gParams->gDialogImageFileName[_MAX_PATH] = '\0';   //ensure null-terminated
    gParams->gDialogImage_save = gDialogImage = gParams->gDialogImageFileName;
    /* Force the background to be redrawn */
    return RedrawWindow(
            fmc.hDlg,   // handle of window
            NULL,   // address of structure with update rectangle, NULL=>entire client area
            NULL,   // handle of update region
            RDW_ERASE|RDW_INVALIDATE|RDW_ERASENOW|RDW_UPDATENOW     // array of redraw flags
       );
} /*fm_setDialogImage*/

/**********************************************************************/
/* setDialogImageMode({EXACT==0|TILED==1|STRETCHED==2}, <stretch_mode>)
/*  Specifies whether the background image is to the tiled or
/*  stretched to fill the entire dialog, or used exactly as is.
/*  If the image is to be stretched, then the stretch mode is
/*  also specified.
/*
/**********************************************************************/
int setDialogImageMode(int mode, int stretchMode)
{
    gParams->gDialogImageMode = mode;
    if (stretchMode != 0) gParams->gDialogImageStretchMode = stretchMode;
    /* Force the background to be redrawn */
    return RedrawWindow(
            fmc.hDlg,   // handle of window
            NULL,   // address of structure with update rectangle, NULL=>entire client area
            NULL,   // handle of update region
            RDW_ERASE|RDW_INVALIDATE|RDW_ERASENOW|RDW_UPDATENOW     // array of redraw flags
       );
} /*setDialogImageMode*/

/**********************************************************************/
/* setDialogDragMode({TitleBar==0|Background==1|None==2})
/*  Determines whether the filter dialog box can be dragged by the
/*  title bar only (0), by dragging the title bar or anywhere on the
/*  dialog background (1), or not at all (2).  The default mode is 0.
/**********************************************************************/
int setDialogDragMode(int mode)
{
    gParams->gDialogDragMode = mode;
    return TRUE;
} /*fm_setDialogDragMode*/

/**********************************************************************/
/* setDialogRegion(<region_expression>)
/*  Sets the clipping region (outline) of the filter dialog.
/*
/**********************************************************************/
int setDialogRegion(INT_PTR rgn)
{
    //gParams->gDialogRegion = (HRGN)rgn; //save across invocations
    return SetWindowRgn(
                fmc.hDlg,   // handle to window whose window region is to be set
                (HRGN)rgn,  // handle to region 
                TRUE    // window redraw flag 
                       );
    //delete the component regions with DeleteObj()???
} /*fm_setDialogRegion*/

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
INT_PTR createRectRgn(int xLeft, int yTop, int xRight, int yBottom)
{
    Map4DBUsToPixels(&xLeft, &yTop, &xRight, &yBottom);
    return (INT_PTR)CreateRectRgn(xLeft, yTop, xRight, yBottom);
} /*fm_createRectRgn*/

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
INT_PTR createRoundRectRgn(int xLeft, int yTop, int xRight, int yBottom,
                                 int eWidth, int eHeight)
{
    Map4DBUsToPixels(&xLeft, &yTop, &xRight, &yBottom);
    Map2DBUsToPixels(&eWidth, &eHeight);
    return (INT_PTR)CreateRoundRectRgn(xLeft, yTop, xRight, yBottom,
                                   eWidth, eHeight);
} /*fm_createRoundRectRgn*/

/**********************************************************************/
/* REGION createCircularRgn(xLeft, yTop, iDiameter)
/*  Creates a simple circular region.  <left> and <top> give the x- and
/*  y-coordinates of the upper-left corner of the bounding box for the
/*  circle.  <diameter> gives the diameter of the circle.  All measurements
/*  are in DBUs.
/*
/**********************************************************************/
INT_PTR createCircularRgn(int xLeft, int yTop, int iDiameter)
{
    int xRight = xLeft + iDiameter;
    int yBottom = yTop + iDiameter;
    Map4DBUsToPixels(&xLeft, &yTop, &xRight, &yBottom);
    return (INT_PTR)CreateEllipticRgn(xLeft, yTop, xRight, yBottom);
} /*fm_createCircularRgn*/

/**********************************************************************/
/* REGION createEllipticRgn(xLeft, yTop, xRight, yBottom)
/*  Creates an elliptical region.  <left>, <top>, <right>, and <bottom>
/*  define the coordinates of the bounding box for the ellipse.  All
/*  measurements are in DBUs.
/*
/**********************************************************************/
INT_PTR createEllipticRgn(int xLeft, int yTop, int xRight, int yBottom)
{
    Map4DBUsToPixels(&xLeft, &yTop, &xRight, &yBottom);
    return (INT_PTR)CreateEllipticRgn(xLeft, yTop, xRight, yBottom);
} /*fm_createEllipticRgn*/

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
INT_PTR createPolyRgn(int fillMode, int nPoints, int x1, int y1,
                            int x2, int y2, int x3, int y3, ...)
{
    int i;

    //Map 1st three mandatory points to pixels.
    Map4DBUsToPixels(&x1, &y1, &x2, &y2);
    Map2DBUsToPixels(&x3, &y3);

    //Map any remaining optional points.
    //Following is non-portable!!!! (Depends on __cdecl calling
    //convention to push args right-to-left as contiguous LONGs
    //on the stack.)
    for (i = 3; i < nPoints; i++) {
        Map2DBUsToPixels(&(&x1)[2*i], &(&y1)[2*i]);
    }

    return (INT_PTR)CreatePolygonRgn(
            //Following is non-portable!!!! (Depends on __cdecl calling
            //convention to push args right-to-left as contiguous LONGs
            //on the stack.)
            (CONST POINT *)&x1, // pointer to array of points 
            nPoints,    // number of points in array 
            fillMode    // polygon-filling mode 
           );
} /*fm_createPolyRgn*/  

/**********************************************************************/
/* REGION createDialogRgn();
/*  Is a predefined rectangular region consisting of the default dialog
/*  box itself.
/*
/**********************************************************************/
int createDialogRgn(void)
{
    // Should emit a warning or error !!!!
    return 0;   //for now...
} /*fm_createDialogRgn*/

/**********************************************************************/
/* REGION createTitlebarRgn();
/*  Is a predefined rectangular region defining the area of the dialog
/*  title bar (if present).
/*
/**********************************************************************/
int createTitlebarRgn(void)
{
    // Should emit a warning or error !!!!
    return 0;   //for now...
} /*fm_createTitlebarRgn*/

/**********************************************************************/
/* REGION createMenubarRgn();
/*  Is a predefined rectangular region defining the area of the dialog
/*  menu bar (if any).
/*
/**********************************************************************/
int createMenubarRgn(void)
{
    // Should emit a warning or error !!!!
    return 0;   //for now...
} /*fm_createMenubarRgn*/

/**********************************************************************/
/* REGION createClientRgn();
/*  Is a predefined rectanglar region defining the client area of the
/*  dialog box.
/*
/**********************************************************************/
int createClientRgn(void)
{
    // Should emit a warning or error !!!!
    return 0;   //for now...
} /*fm_createClientRgn*/

/**********************************************************************/
/* REGION createStatusbarRgn();
/*  Is a predefined rectangular region defining the area of the dialog
/*  status bar (if any).
/*
/**********************************************************************/
int createStatusbarRgn(void)
{
    // Should emit a warning or error !!!!
    return 0;   //for now...
} /*fm_createStatusbarRgn*/

/**********************************************************************/
/* BOOL playSoundWave(szWaveFile);
/*  Plays the specified wave file asynchronously.  Looks first for
/*  an embedded WAVE resource with the specified name; else searches
/*  the usual paths for the specified wave file.
/*
/**********************************************************************/
BOOL playSoundWave(INT_PTR szWaveFile)
{
    if (PlaySound((const char *)szWaveFile, (HMODULE)hDllInstance,
                  SND_ASYNC | SND_RESOURCE | SND_NODEFAULT)) {
        //played from a WAVE resource
        return TRUE;
    }
    else if (szWaveFile != 0) {
        //try to locate the original wave file...
        return PlaySound(FmSearchFile((const char *)szWaveFile), NULL,
                         SND_ASYNC | SND_FILENAME);
    }
    return TRUE;
} /*fm_playSoundWave*/

/**********************************************************************/
/* BOOL playSoundWaveLoop(szWaveFile);
/*  Plays the specified wave file repeatedly.  Looks first for
/*  an embedded WAVE resource with the specified name; else searches
/*  the usual paths for the specified wave file.
/*
/**********************************************************************/
BOOL playSoundWaveLoop(INT_PTR szWaveFile)
{
    if (PlaySound((const char *)szWaveFile, (HMODULE)hDllInstance,
        SND_LOOP | SND_ASYNC | SND_RESOURCE | SND_NODEFAULT)) {
        //played from a WAVE resource
        return TRUE;
    }
    else if (szWaveFile != 0) {
        //try to locate the original wave file...
        return PlaySound(FmSearchFile((const char *)szWaveFile), NULL,
                         SND_LOOP | SND_ASYNC | SND_FILENAME);
    }
    return TRUE;
} /*fm_playSoundWaveLoop*/

/**********************************************************************/
/* BOOL playSoundWaveSync(szWaveFile);
/*  Plays the specified wave file synchronously.  Looks first for
/*  an embedded WAVE resource with the specified name; else searches
/*  the usual paths for the specified wave file.
/*
/**********************************************************************/
BOOL playSoundWaveSync(INT_PTR szWaveFile)
{
    if (PlaySound((const char *)szWaveFile, (HMODULE)hDllInstance,
                  SND_SYNC | SND_RESOURCE | SND_NODEFAULT)) {
        //played from a WAVE resource
        return TRUE;
    }
    else if (szWaveFile != 0) {
        //try to locate the original wave file...
        return PlaySound(FmSearchFile((const char *)szWaveFile), NULL,
                         SND_SYNC | SND_FILENAME);
    }
    return TRUE;
} /*fm_playSoundWaveSync*/

int fm_sleep(int msecs)
{
    // interface to the __stdcall Sleep service
    Sleep(msecs);
    return 0;
} /*fm_sleep*/

int fm_RGB(int r, int g, int b) {
    return ((b & 0xff) << 8 | (g & 0xff)) << 8 | (r & 0xff);
} /*fm_RGB*/

int RGBA(int r, int g, int b, int a) {
    return (((a & 0xff) << 8 | (b & 0xff)) << 8 | (g & 0xff)) << 8 | (r & 0xff);
} /*fm_RGBA*/

int Rval(int rgba) {
    return GetRValue(rgba);
} /*fm_Rval*/

int Gval(int rgba) {
    return GetGValue(rgba);
} /*fm_Gval*/

int Bval(int rgba) {
    return GetBValue(rgba);
} /*fm_Bval*/

int Aval(int rgba) {
    return (BYTE)(rgba >> 24);
} /*fm_Aval*/

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
int getOpenFileName( int flags,
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
                   )
{
    OPENFILENAME ofn;
    BOOL ok;
    int iError;

    ofn.lStructSize       = sizeof(ofn);
    ofn.hwndOwner         = fmc.doingProxy?fmc.hDlg:NULL;   // handle of owner window
    ofn.hInstance         = NULL;
    ofn.lpstrFilter       = lpstrFilter;
    ofn.lpstrCustomFilter = lpstrCustomFilter;
    ofn.nMaxCustFilter    = nMaxCustFilter;
    ofn.nFilterIndex      = pnFilterIndex ? *pnFilterIndex : 1;
    ofn.nMaxFile          = nMaxFile;
    ofn.nMaxFileTitle     = nMaxFileTitle;
    ofn.lpstrInitialDir   = lpstrInitialDir;
    ofn.lpstrTitle        = lpstrDialogTitle;
    ofn.nFileOffset       = 0;
    ofn.nFileExtension    = 0;
    ofn.lpstrDefExt       = lpstrDefExt;
    ofn.lCustData         = 0;
    ofn.lpfnHook          = NULL;
    ofn.lpTemplateName    = NULL;
    ofn.lpstrFile         = lpstrFile;
    ofn.lpstrFileTitle    = lpstrFileTitle;
    ofn.Flags             = flags | OFN_EXPLORER;   //force Explorer-style dialog
    // Don't allow user to set following flags...
    ofn.Flags &= ~(OFN_ENABLEHOOK | OFN_ENABLETEMPLATE | OFN_ENABLETEMPLATEHANDLE);

    ok = GetOpenFileName(&ofn);

#if 1
    if (pOflags) *pOflags = ofn.Flags;
#else
    //screen out non-return bits?
    if (pOflags) {
        *pOflags = ofn.Flags & (OFN_EXTENSIONDIFFERENT | OFN_NOREADONLYRETURN | OFN_READONLY);
    }
#endif
    if (pnFileOffset) *pnFileOffset = ofn.nFileOffset;
    if (pnFileExtension) *pnFileExtension = ofn.nFileExtension;
    if (pnFilterIndex) *pnFilterIndex = ofn.nFilterIndex;
    if (ok) {
        iError = 0;   //success
    }
    else {
        //error...
        iError = CommDlgExtendedError();
        if (iError == 0) {
            //User closed or canceled the dialog box...
            iError = -1;
        }
    }
    return iError;
} //fm_getOpenFileName

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
int getSaveFileName( int flags,
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
                   )
{
    OPENFILENAME ofn;
    BOOL ok;
    int iError;

    ofn.lStructSize       = sizeof(ofn);
    ofn.hwndOwner         = fmc.doingProxy?fmc.hDlg:NULL;   // handle of owner window
    ofn.hInstance         = NULL;
    ofn.lpstrFilter       = lpstrFilter;
    ofn.lpstrCustomFilter = lpstrCustomFilter;
    ofn.nMaxCustFilter    = nMaxCustFilter;
    ofn.nFilterIndex      = pnFilterIndex ? *pnFilterIndex : 1;
    ofn.nMaxFile          = nMaxFile;
    ofn.nMaxFileTitle     = nMaxFileTitle;
    ofn.lpstrInitialDir   = lpstrInitialDir;
    ofn.lpstrTitle        = lpstrDialogTitle;
    ofn.nFileOffset       = 0;
    ofn.nFileExtension    = 0;
    ofn.lpstrDefExt       = lpstrDefExt;
    ofn.lCustData         = 0;
    ofn.lpfnHook          = NULL;
    ofn.lpTemplateName    = NULL;
    ofn.lpstrFile         = lpstrFile;
    ofn.lpstrFileTitle    = lpstrFileTitle;
    ofn.Flags             = flags | OFN_EXPLORER;   //force Explorer-style dialog
    // Don't allow user to set following flags...
    ofn.Flags &= ~(OFN_ENABLEHOOK | OFN_ENABLETEMPLATE | OFN_ENABLETEMPLATEHANDLE | OFN_ALLOWMULTISELECT);

    ok = GetSaveFileName(&ofn);

#if 1
    if (pOflags) *pOflags = ofn.Flags;
#else
    //screen out non-return bits?
    if (pOflags) {
        *pOflags = ofn.Flags & (OFN_EXTENSIONDIFFERENT | OFN_NOREADONLYRETURN | OFN_READONLY);
    }
#endif
    if (pnFileOffset) *pnFileOffset = ofn.nFileOffset;
    if (pnFileExtension) *pnFileExtension = ofn.nFileExtension;
    if (pnFilterIndex) *pnFilterIndex = ofn.nFilterIndex;
    if (ok) {
        iError = 0;   //success
    }
    else {
        //error...
        iError = CommDlgExtendedError();
        if (iError == 0) {
            //User closed or canceled the dialog box...
            iError = -1;
        }
    }
    return iError;
} //fm_getSaveFileName



int ctlColorBox;
int ctlColorDialogPreview;
int cdLanguage = 0;
int cdInitialColor;
int cdOldcolor = 0;
//char * strColorTitle;
HWND foundWindow = 0;

#include <ColorDlg.h>
#define COLOR_PREVIEW 740


BOOL CALLBACK EnumChildProc(__in  HWND hwnd,__in  LPARAM lParam){
	char tempString[256];
	GetWindowText(hwnd,tempString,256);
	if (strcmp(tempString,(char *)lParam)==0){
		foundWindow = hwnd;
		return FALSE;
	}
	return TRUE;
}

void findChildWindow (HWND window, char * text){
	foundWindow = 0;
	EnumChildWindows(window,EnumChildProc,(LPARAM)text);
}

void cdUpdatePreview(HWND hdlg){

	if (SendMessage(GetDlgItem(hdlg,COLOR_PREVIEW),BM_GETCHECK, 0, 0)){ //Is Preview check box activated?
		BOOL fTranslated;
		int rVal = (int)GetDlgItemInt(hdlg, COLOR_RED, &fTranslated, TRUE);
		int gVal = (int)GetDlgItemInt(hdlg, COLOR_GREEN, &fTranslated, TRUE);
		int bVal = (int)GetDlgItemInt(hdlg, COLOR_BLUE, &fTranslated, TRUE);
		int color = RGB(rVal,gVal,bVal);
		
		if (fTranslated && color != cdOldcolor) {
			//setCtlColor(ctlColorBox,color);  //Avoid redraw problem on some systems
			setCtlVal(ctlColorBox,color);
			Sleep(30); //Avoid preview flicker
			doAction(CA_PREVIEW);
			cdOldcolor = RGB(rVal,gVal,bVal);
		}
	}

}

UINT_PTR CALLBACK CCHookProc(__in  HWND hdlg,__in  UINT uiMsg,__in  WPARAM wParam,__in  LPARAM lParam){

	if(uiMsg == WM_INITDIALOG){
		
		char * controlTextEN[12]={"&Basic colors:","&Custom colors:","Cancel","Color","|S&olid","Hu&e:","&Sat:","&Lum:","&Red:","&Green:","Bl&ue:","&Add to Custom Colors"};
		char * controlTextDE[12]={"&Grundfarben:","Benut&zerdefinierte Farben:","Abbruch","Farbe","|&Basis","&Farbt.:","&Sätt.:","H&ell.:","&Rot:","Grü&n:","B&lau:","Farben hinzuf&ügen"};
		char ** controlText = &controlTextEN[0];
		int controlID[12]={-1,COLOR_CUSTOM1,IDCANCEL,COLOR_SOLID_LEFT,COLOR_SOLID_RIGHT,COLOR_HUEACCEL,COLOR_SATACCEL,COLOR_LUMACCEL,COLOR_REDACCEL,COLOR_GREENACCEL,COLOR_BLUEACCEL,COLOR_ADD};
		RECT r;
		HWND hCtl;
		int i;

		if (cdLanguage==1) controlText = &controlTextDE[0]; //Switch to German

		//Set window title text
		SetWindowText(hdlg, cdLanguage?"Bitte wählen Sie eine Farbe:":"Please choose a color:" ); //strColorTitle 

		//Localize
		findChildWindow(hdlg,"&Basic colors:");
		if (foundWindow==0) findChildWindow(hdlg,"&Grundfarben:");
		if (foundWindow!=0) SetWindowText(foundWindow,controlText[0]);
		findChildWindow(hdlg,"&Custom colors:");
		if (foundWindow==0) findChildWindow(hdlg,"Benut&zerdefinierte Farben:");
		if (foundWindow!=0) SetWindowText(foundWindow,controlText[1]);
		for (i=2;i<12;i++) SetWindowText(GetDlgItem(hdlg,controlID[i]),controlText[i]);

		//Hide button
		EnableWindow(GetDlgItem(hdlg,COLOR_MIX), FALSE);
		ShowWindow(GetDlgItem(hdlg,COLOR_MIX), SW_HIDE);

		//Add Preview check box
		if (ctlColorDialogPreview>0){
			GetWindowRect(GetDlgItem(hdlg,COLOR_MIX), &r); //Position where hidden button was
			ScreenToClient (hdlg, (LPPOINT)&r);

			hCtl = CreateWindowEx(  
					0,//gParams->ctl[n].styleEx,        // extended styles
					"BUTTON",                       // predefined class 
					cdLanguage?"Vorschau":"Preview",	// control text
					WS_VISIBLE | WS_CHILD | WS_TABSTOP | BS_AUTOCHECKBOX,          // styles
					r.left+3,                         // starting x position 
					r.top,                          // starting y position 
					80,//r.right,                        // control width 
					20,//r.bottom,                       // control height 
					hdlg,							// parent window 
					(HMENU)COLOR_PREVIEW,        // control ID
					(HINSTANCE)hDllInstance,
					NULL);                          // pointer not needed 

			// Set control font to same as dialog font
			if (hCtl) {
				HFONT hDlgFont;
				if ((hDlgFont = (HFONT)SendMessage(hdlg, WM_GETFONT, 0, 0L)) != NULL){
					SendMessage(hCtl, WM_SETFONT, (WPARAM)hDlgFont, 0L);  //redraw=FALSE
				}
			}
			//Activate check box ???
			if (ctl(ctlColorDialogPreview)) SendMessage(hCtl, BM_SETCHECK, TRUE, 0);
		}


		//Center dialog
		{
			RECT rc,mrc;
			GetWindowRect(hdlg, &rc);
			if (fmc.hDlg!=NULL){
				//GetWindowRect(fmc.hDlg, &mrc);
				GetWindowRect(gParams->ctl[ctlColorBox].hCtl, &mrc);
				SetWindowPos(hdlg, NULL, mrc.left + ((mrc.right-mrc.left)-(rc.right-rc.left))/2, mrc.top+((mrc.bottom-mrc.top)-(rc.bottom-rc.top))/2, 0, 0,SWP_NOSIZE|SWP_NOZORDER|SWP_NOACTIVATE|SWP_SHOWWINDOW);
			} else
				SetWindowPos(hdlg, NULL,(getDisplaySettings(1)-(rc.right-rc.left))/2, (getDisplaySettings(2)-(rc.bottom-rc.top))/2, 0, 0,SWP_NOSIZE|SWP_NOZORDER|SWP_NOACTIVATE|SWP_SHOWWINDOW);
		}

	} else if (uiMsg == WM_COMMAND){

		int idd = LOWORD(wParam);
        int iCmd = HIWORD(wParam);
		
		if (idd==COLOR_PREVIEW){ //Preview check
			int val = (int)SendMessage(GetDlgItem(hdlg,idd),BM_GETCHECK, 0, 0);
			setCtlVal(ctlColorDialogPreview, val);

			if (val){
				cdOldcolor=-1;
				cdUpdatePreview(hdlg);
			} else {
				BOOL fTranslated;
				int rVal = (int)GetDlgItemInt(hdlg, COLOR_RED, &fTranslated, TRUE);
				int gVal = (int)GetDlgItemInt(hdlg, COLOR_GREEN, &fTranslated, TRUE);
				int bVal = (int)GetDlgItemInt(hdlg, COLOR_BLUE, &fTranslated, TRUE);
				int color = RGB(rVal,gVal,bVal);
				
				if (cdInitialColor != color){ //Restore initial color
					//setCtlColor(ctlColorBox,cdInitialColor); //Avoid redraw problem on some systems
					setCtlVal(ctlColorBox,cdInitialColor);
					doAction(CA_PREVIEW);
				}
			}

		} else if (iCmd == EN_CHANGE){ //Color change
			if (idd==COLOR_RED||idd==COLOR_GREEN||idd==COLOR_BLUE){
				if (ctlColorDialogPreview>0) cdUpdatePreview(hdlg);	
			}
		}

	}

	return false;
}

int chooseColor2(int n, int initialColor, int ctlPreview, int language){ //char * title, 
	
	CHOOSECOLOR cc;
    static COLORREF acrCustClr[16];
	//char language[256];

	//Get language of color dialog
	//getLocaleInfo (LOCALE_USER_DEFAULT, LOCALE_SENGLANGUAGE, language, 255 );
	//if (strcmp (language, "German") ==0 ) cdLanguage = 1;
    
    // Initialize CHOOSECOLOR 
    ZeroMemory(&cc, sizeof(cc));
    cc.lStructSize = sizeof(cc);
    cc.hwndOwner = fmc.hDlg;
    cc.lpCustColors = (LPDWORD) acrCustClr;
    cc.rgbResult = (DWORD) initialColor;
    cc.Flags = CC_FULLOPEN | CC_RGBINIT | CC_ENABLEHOOK;
	cc.lpfnHook = CCHookProc;

	//Set stuff for hook
	ctlColorBox = n;
	ctlColorDialogPreview = ctlPreview;
	//strColorTitle = title;
	cdLanguage = language;
	cdInitialColor = initialColor;
	
    if (ChooseColor(&cc))
        return cc.rgbResult;
	else {  //Canceled
		//Restore color
		//if (getCtlColor(n) != initialColor){
		if (ctl(n) != initialColor){
			//setCtlColor(n,initialColor);  //Avoid redraw problem on some systems
			setCtlVal(n,initialColor);
			doAction(CA_PREVIEW);
		}
		return -1;
	}

} //fm_chooseColor




/**********************************************************************/
/* chooseColor(initialColor, szPrompt, ...)
/*
/*  Invokes the host app's color picker to choose a color.
/*  Returns the chosen color, or -1 if an error occurred.
/*
/**********************************************************************/
int chooseColor(int initialColor, const char *szPrompt, ...)
{

#ifndef APP

	if (gStuff->colorServices) {

		ColorServicesInfo csi;
		OSErr err;
		Str255 pickerPrompt;
		char szBuffer[1024];
		va_list ap;

		/* Interpret and interpolate the prompt string */
		va_start(ap, szPrompt);
		_vsnprintf(szBuffer, sizeof(szBuffer), szPrompt, ap);
		szBuffer[sizeof(szBuffer)-1] = 0;   //ensure NUL-terminated
		va_end(ap);
		strncpy(szBuffer, formatString(szBuffer), sizeof(szBuffer));
		szBuffer[sizeof(szBuffer)-1] = 0;   //ensure NUL-terminated
		// convert to Pascal (byte-counted) string, 255 chars max
		strncpy((char *)&pickerPrompt[1], szBuffer, 255);
		pickerPrompt[0] = min(strlen(szBuffer), 255);

		csi.infoSize = sizeof(csi);
		csi.selector = plugIncolorServicesChooseColor;
		csi.sourceSpace = plugIncolorServicesRGBSpace;
		csi.resultSpace = plugIncolorServicesRGBSpace;
		csi.reservedSourceSpaceInfo = NULL;
		csi.reservedResultSpaceInfo = NULL;
		csi.reserved = NULL;
		csi.colorComponents[0] = Rval(initialColor);
		csi.colorComponents[1] = Gval(initialColor);
		csi.colorComponents[2] = Bval(initialColor);
		csi.colorComponents[3] = 0;
		csi.selectorParameter.pickerPrompt = &pickerPrompt;

        //call the host's color picker
        err = gStuff->colorServices(&csi);
        if (err == noErr) {
            return fm_RGB(csi.colorComponents[0], csi.colorComponents[1], csi.colorComponents[2]);
        } else {
			if (err == userCanceledErr) return -1;  //error occurred
        }
    }

#endif //APP

	//Display out own color dialog if host does not do it
    {
        //No host color service. Call the Windows native color picker.

        //Added by Harald Heim, January 2002
        CHOOSECOLOR cc;                 // common dialog box structure 
        static COLORREF acrCustClr[16]; // array of custom colors  NOT THREAD-SAFE!! (Should it be?? or just MT lock?)
        //HWND hwnd ;                      // owner window
        //static DWORD rgbCurrent;        // initial color selection

        // Initialize CHOOSECOLOR 
        ZeroMemory(&cc, sizeof(cc));
        cc.lStructSize = sizeof(cc);
        cc.hwndOwner = fmc.hDlg;
        cc.lpCustColors = (LPDWORD) acrCustClr;
        cc.rgbResult = (DWORD) initialColor;
        cc.Flags = CC_FULLOPEN | CC_RGBINIT;
		
        if (ChooseColor(&cc))
            return cc.rgbResult;
        else 
            return -1;

    }
} //fm_chooseColor


/**********************************************************************/
/* setRegRoot(int hkey)
/*  Sets the current registry root key.
/*
/**********************************************************************/
int setRegRoot(HKEY hkey)
{
    gParams->gRegRoot = hkey;
    return ERROR_SUCCESS;
} /*fm_setRegRoot*/

/**********************************************************************/
/* getRegRoot(int *hkey)
/*  Retrieves the current registry root key.
/*
/**********************************************************************/
int getRegRoot(HKEY *phkey)
{
    *phkey = gParams->gRegRoot;
    return ERROR_SUCCESS;
} /*fm_getRegRoot*/

/**********************************************************************/
/* setRegPath(lpsz szPath[, varargs]...)
/*  Sets the current registry path, with printf-style formatting.
/*  Returns ERROR_INVALID_DATA if expanded path string is too long;
/*  else returns ERROR_SUCCESS.
/*
/**********************************************************************/
int setRegPath(LPCSTR szPath,...)
{
    va_list ap;
    int cb;

    va_start(ap, szPath);
    cb = _vsnprintf(gParams->gRegPath, sizeof(gParams->gRegPath), szPath, ap);
    gParams->gRegPath[sizeof(gParams->gRegPath)-1] = 0;   //ensure NUL-terminated
    va_end(ap);
    return (cb < 0) ? ERROR_INVALID_DATA : ERROR_SUCCESS;
} /*fm_setRegPath*/

/**********************************************************************/
/* getRegPath(lpsz szPath, int maxPathLen)
/*  Retrieves the current (not yet interpolated) registry path.
/*  Returns ERROR_INVALID_DATA if caller's buffer is too small;
/*  else returns ERROR_SUCCESS.
/*
/**********************************************************************/
int getRegPath(LPSTR szPath, int maxPathLen)
{
    strncpy(szPath, gParams->gRegPath, maxPathLen);
    if (szPath[maxPathLen-1]) {
        //caller's buffer was too short...
        szPath[maxPathLen-1] = 0;   //force NUL-terminated string;
        return ERROR_INVALID_DATA;  //return error status
    }
    return ERROR_SUCCESS;
} /*fm_getRegPath*/

/**********************************************************************/
/* putRegInt(int iValue, lpsz szValueName[, varargs]...)
/*  Stores an integer value into the registry, with printf-style formatting
/*  for the value name.
/*
/**********************************************************************/
int putRegInt(int iValue, LPCSTR szValueName,...)
{
    va_list ap;
    char szName[1024];
    int cb;
    HKEY hkey;

    va_start(ap, szValueName);
    cb = _vsnprintf(szName, sizeof(szName), szValueName, ap);
    szName[sizeof(szName)-1] = 0;   //ensure NUL-terminated
    va_end(ap);
    if (cb < 0) {
        //oops... expanded string is too long
        return ERROR_INVALID_DATA;
    }
    hkey = RCK(gParams->gRegRoot, formatString(gParams->gRegPath));
    return RSD(hkey, formatString(szName), (DWORD)iValue);
} /*fm_putRegInt*/

/**********************************************************************/
/* getRegInt(int *iValue, lpsz szValueName[, varargs]...)
/*  Gets an integer value from the registry, with printf-style formatting
/*  for the value name.
/*
/**********************************************************************/
int getRegInt(int *iValue, LPCSTR szValueName,...)
{
    va_list ap;
    char szName[1024];
    int cb;
    HKEY hkey;

    va_start(ap, szValueName);
    cb = _vsnprintf(szName, sizeof(szName), szValueName, ap);
    szName[sizeof(szName)-1] = 0;   //ensure NUL-terminated
    va_end(ap);
    if (cb < 0) {
        //oops... expanded string is too long
        return ERROR_INVALID_DATA;
    }
    hkey = ROK(gParams->gRegRoot, formatString(gParams->gRegPath));
    if (hkey!=NULL)
        return RQD(hkey, formatString(szName), (DWORD *)iValue);
    else
        return ERROR_INVALID_ACCESS;
} /*fm_getRegInt*/

/**********************************************************************/
/* putRegString(lpsz szString, lpsz szValueName[, vargargs]...)
/*  Stores a string value into the registry, with printf-style formatting
/*  for the value name.
/*
/**********************************************************************/
int putRegString(LPCSTR szString, LPCSTR szValueName,...)
{
    va_list ap;
    char szName[1024];
    int cb;
    HKEY hkey;

    va_start(ap, szValueName);
    cb = _vsnprintf(szName, sizeof(szName), szValueName, ap);
    szName[sizeof(szName)-1] = 0;   //ensure NUL-terminated
    va_end(ap);
    if (cb < 0) {
        //oops... expanded string is too long
        return ERROR_INVALID_DATA;
    }
#ifdef REG_DATA_LIMIT
    //check for data value too long...
    if (strlen(szString) > REG_DATA_LIMIT) {
        //limit amount of data user is allowed to store in registry
        return ERROR_INVALID_DATA;
    }
#endif //REG_DATA_LIMIT
    hkey = RCK(gParams->gRegRoot, formatString(gParams->gRegPath));
    return RSS(hkey, formatString(szName), szString);
} /*fm_putRegString*/

/**********************************************************************/
/* getRegString(lpsz szString, int iMaxlen, lpsz szValueName[, varargs]...)
/*  Gets a string value from the registry, with printf-style formatting
/*  for the value name.
/*
/**********************************************************************/
int getRegString(LPSTR szString, int iMaxlen, LPCSTR szValueName,...)
{
    va_list ap;
    char szName[1024];
    int cb;
    HKEY hkey;

    va_start(ap, szValueName);
    cb = _vsnprintf(szName, sizeof(szName), szValueName, ap);
    szName[sizeof(szName)-1] = 0;   //ensure NUL-terminated
    va_end(ap);
    if (cb < 0) {
        //oops... expanded string is too long
        return ERROR_INVALID_DATA;
    }
    hkey = ROK(gParams->gRegRoot, formatString(gParams->gRegPath));
    if (hkey!=NULL)
        return RQS(hkey, formatString(szName), szString, iMaxlen);
    else
        return ERROR_INVALID_ACCESS;
} /*fm_getRegString*/

/**********************************************************************/
/* putRegData(void *data, int dataLen, lpsz szValueName[, vargargs]...)
/*  Stores a set of binary data into the registry, with printf-style
/*  formatting for the value name.
/*
/**********************************************************************/
int putRegData(const void *pData, int dataLen, LPCSTR szValueName,...)
{
    va_list ap;
    char szName[1024];
    int cb;
    HKEY hkey;

    va_start(ap, szValueName);
    cb = _vsnprintf(szName, sizeof(szName), szValueName, ap);
    szName[sizeof(szName)-1] = 0;   //ensure NUL-terminated
    va_end(ap);
    if (cb < 0) {
        //oops... expanded string is too long
        return ERROR_INVALID_DATA;
    }
#ifdef REG_DATA_LIMIT
    //check for data value too long...
    if (dataLen > REG_DATA_LIMIT) {
        //limit amount of data user is allowed to store in registry
        return ERROR_INVALID_DATA;
    }
#endif //REG_DATA_LIMIT
    hkey = RCK(gParams->gRegRoot, formatString(gParams->gRegPath));
    return RSB(hkey, formatString(szName), pData, dataLen);
} /*fm_putRegData*/

/**********************************************************************/
/* getRegData(void *data, int dataLen, lpsz szValueName[, vargargs]...)
/*  Gets a set of binary data from the registry, with printf-style
/*  formatting for the value name.
/*
/**********************************************************************/
int getRegData(void *pData, int dataLen, LPCSTR szValueName,...)
{
    va_list ap;
    char szName[1024];
    int cb;
    HKEY hkey;

    va_start(ap, szValueName);
    cb = _vsnprintf(szName, sizeof(szName), szValueName, ap);
    szName[sizeof(szName)-1] = 0;   //ensure NUL-terminated
    va_end(ap);
    if (cb < 0) {
        //oops... expanded string is too long
        return ERROR_INVALID_DATA;
    }
    hkey = ROK(gParams->gRegRoot, formatString(gParams->gRegPath));
    return RQB(hkey, formatString(szName), pData, dataLen);
} /*fm_getRegData*/

/**********************************************************************/
/* enumRegSubKey(int index, lpsz szSubKey, int maxSubKeyLen)
/*  Gets name of n-th subkey under current registry key.
/*
/**********************************************************************/
int enumRegSubKey(int index, LPSTR szSubKey, int maxSubKeyLen)
{
    HKEY hkey;

    hkey = ROK(gParams->gRegRoot, formatString(gParams->gRegPath));
    return REK(hkey, index, szSubKey, maxSubKeyLen);
} /*fm_enumRegSubKey*/

/**********************************************************************/
/* enumRegValue(int index, lpsz szValueName, int maxValueNameLen,
/*              int *iType, int *cbData)
/*  Gets name, type, and data size of n-th value under current registry
/*  key.
/*
/**********************************************************************/
int enumRegValue(int index, LPSTR szValueName, int maxValueNameLen,
                    int *iType, int *cbData)
{
    HKEY hkey;

    hkey = ROK(gParams->gRegRoot, formatString(gParams->gRegPath));
    return REV(hkey, index, szValueName, (LPDWORD)&maxValueNameLen, (LPDWORD)iType, NULL, (LPDWORD)cbData);
} /*fm_enumRegValue*/

/**********************************************************************/
/* deleteRegSubKey(lpsz szSubKey[, varargs]...)
/*  Deletes specified subkey (and all its values) below the current
/*  key.  The subkey name is specified with printf-style formatting.
/*  The specified subkey must not have sub-subkeys (at least on NT;
/*  Win95 may allow deletion of an entire key subtree?).
/*
/**********************************************************************/
int deleteRegSubKey(LPCSTR szSubKey,...)
{
    va_list ap;
    char szName[1024];
    int cb;
    HKEY hkey;

    va_start(ap, szSubKey);
    cb = _vsnprintf(szName, sizeof(szName), szSubKey, ap);
    szName[sizeof(szName)-1] = 0;   //ensure NUL-terminated
    va_end(ap);
    if (cb < 0) {
        //oops... expanded string is too long
        return ERROR_INVALID_DATA;
    }
    hkey = RCK(gParams->gRegRoot, formatString(gParams->gRegPath));
    return RDK(hkey, formatString(szName));
} /*fm_deleteRegSubKey*/

/**********************************************************************/
/* deleteRegValue(lpsz szValueName[, vargargs]...)
/*  Deletes specified value under the current key.  The value name is
/*  specified with printf-style formatting.
/*
/**********************************************************************/
int deleteRegValue(LPCSTR szValueName,...)
{
    va_list ap;
    char szName[1024];
    int cb;
    HKEY hkey;

    va_start(ap, szValueName);
    cb = _vsnprintf(szName, sizeof(szName), szValueName, ap);
    szName[sizeof(szName)-1] = 0;   //ensure NUL-terminated
    va_end(ap);
    if (cb < 0) {
        //oops... expanded string is too long
        return ERROR_INVALID_DATA;
    }
    hkey = RCK(gParams->gRegRoot, formatString(gParams->gRegPath));
    return RDV(hkey, formatString(szName));
} /*fm_deleteRegValue*/



int shellExec(char* szVerb, char* szFilename, char* szParams, char* szDefDir)
{
    HINSTANCE ret = ShellExecute(fmc.hDlg, (char *)szVerb,
                                              (char *)szFilename,
                                              (char *)szParams,
                                              (char *)szDefDir,
                                              SW_SHOWNORMAL);
    if ((INT_PTR)ret <= 32) {
#ifdef VERBOSE
        ErrorOk("shellExec failed: %d", ret);
#endif
        return 0; //ret;   //failed
    }
    return -1;    //success
} /*fm_shellExec*/

int msgBox(UINT uType, LPCTSTR lpCaption, LPCTSTR lpFmt,...)
{
    char szBuffer[1024];
    va_list ap;

    va_start(ap, lpFmt); //(LPCTSTR)
    _vsnprintf(szBuffer, sizeof(szBuffer), lpFmt, ap);
    szBuffer[sizeof(szBuffer)-1] = 0;   //ensure NUL-terminated
    va_end(ap);
    return MessageBox(
        fmc.doingProxy?fmc.hDlg:NULL,   // handle of owner window
        formatString(szBuffer),      // interpolated text for message box
        lpCaption,                      // address of title of message box  
        uType                           // style of message box
        | MB_TASKMODAL                  // Force task modal style.
       );
} /*msgBox*/

int fm_printf(char* lpFmt,...)
{
    char szBuffer[1024];
    va_list ap;
    int nchars;

    va_start(ap, lpFmt);
    nchars = _vsnprintf(szBuffer, sizeof(szBuffer), (LPCTSTR)lpFmt, ap);
    szBuffer[sizeof(szBuffer)-1] = 0;   //ensure NUL-terminated
    va_end(ap);
    MessageBox(
        fmc.doingProxy?fmc.hDlg:NULL,   // handle of owner window
        formatString(szBuffer),      // interpolated text for message box
        
        //Added by Harald Heim, January 2003
        "stdout",           // address of title of message box  
        //"FilterMeister stdout",           // address of title of message box  

        0                               // style of message box
        | MB_TASKMODAL                  // Force task modal style.
       );
    return nchars;
} /*fm_printf*/

int Info(char* lpFmt,...)
{
    char szBuffer[1024];
    va_list ap;

    va_start(ap, lpFmt);
    _vsnprintf(szBuffer, sizeof(szBuffer), (LPCTSTR)lpFmt, ap);
    szBuffer[sizeof(szBuffer)-1] = 0;   //ensure NUL-terminated
    va_end(ap);
    return MessageBox(
        fmc.doingProxy?fmc.hDlg:NULL,   // handle of owner window
        formatString(szBuffer),      // interpolated text for message box
        
        //Added by Harald Heim, January 12, 2003
         fmc.filterTitleText,
        //"FilterMeister Info",         // address of title of message box  

        MB_ICONINFORMATION              // style of message box
        | MB_TASKMODAL                  // Force task modal style.
        //| MB_TOPMOST                    // Force topmost window.
        //| MB_SETFOREGROUND              // Force foreground window.
       );
} /*Info*/

int Warn(char* lpFmt,...)
{
    char szBuffer[1024];   
	va_list ap;

	va_start(ap, lpFmt);
    _vsnprintf(szBuffer, sizeof(szBuffer), (LPCTSTR)lpFmt, ap);
    szBuffer[sizeof(szBuffer)-1] = 0;   //ensure NUL-terminated
    va_end(ap);
    return MessageBox(
        fmc.doingProxy?fmc.hDlg:NULL,   // handle of owner window
        formatString(szBuffer),      // interpolated text for message box
        
        //Added by Harald Heim, January 12, 2003
         "Warning",
        //"FilterMeister Warning",      // address of title of message box  
        MB_ICONWARNING|MB_OKCANCEL      // style of message box
        | MB_TASKMODAL                  // Force task modal style.
       );
} /*Warn*/

int Error(char* lpFmt,...)
{
    char szBuffer[1024];
    va_list ap;

    va_start(ap, lpFmt);
    _vsnprintf(szBuffer, sizeof(szBuffer), (LPCTSTR)lpFmt, ap);
    szBuffer[sizeof(szBuffer)-1] = 0;   //ensure NUL-terminated
    va_end(ap);
    return MessageBox(
        fmc.doingProxy?fmc.hDlg:NULL,   // handle of owner window
        formatString(szBuffer),      // interpolated text for message box
        
        //Added by Harald Heim, January 12, 2003        
        "Error",
        //"FilterMeister Error",            // address of title of message box  
        
        MB_ICONERROR|MB_ABORTRETRYIGNORE // style of message box
        | MB_TASKMODAL                  // Force task modal style.
       );
} /*Error*/

int ErrorOk(char* lpFmt,...)
{
    char szBuffer[1024];
    va_list ap;

    va_start(ap, lpFmt);
    _vsnprintf(szBuffer, sizeof(szBuffer), (LPCTSTR)lpFmt, ap);
    szBuffer[sizeof(szBuffer)-1] = 0;   //ensure NUL-terminated
    va_end(ap);
    return MessageBox(
        fmc.doingProxy?fmc.hDlg:NULL,   // handle of owner window
        formatString(szBuffer),      // interpolated text for message box
        
        //Added by Harald Heim, January 12, 2003
        "Error",
        //"FilterMeister Error",            // address of title of message box  

        MB_ICONERROR|MB_OK              // style of message box
        | MB_TASKMODAL                  // Force task modal style.
       );
} /*ErrorOk*/

int YesNo(char* lpFmt,...)
{
    char szBuffer[1024];
    va_list ap;

    va_start(ap, lpFmt);
    _vsnprintf(szBuffer, sizeof(szBuffer), (LPCTSTR)lpFmt, ap);
    szBuffer[sizeof(szBuffer)-1] = 0;   //ensure NUL-terminated
    va_end(ap);
    return MessageBox(
        fmc.doingProxy?fmc.hDlg:NULL,   // handle of owner window
        formatString(szBuffer),      // interpolated text for message box
        
        //Added by Harald Heim, January 12, 2003
         fmc.filterTitleText,
        //"FilterMeister",              // address of title of message box  

        MB_ICONQUESTION|MB_YESNO        // style of message box
        | MB_TASKMODAL                  // Force task modal style.
       );
} /*YesNo*/

int YesNoCancel(char* lpFmt,...)
{
    char szBuffer[1024];
    va_list ap;

    va_start(ap, lpFmt);
    _vsnprintf(szBuffer, sizeof(szBuffer), (LPCTSTR)lpFmt, ap);
    szBuffer[sizeof(szBuffer)-1] = 0;   //ensure NUL-terminated
    va_end(ap);
    return MessageBox(
        fmc.doingProxy?fmc.hDlg:NULL,   // handle of owner window
        formatString(szBuffer),      // interpolated text for message box
        
        //Added by Harald Heim, January 12, 2003
         fmc.filterTitleText,
        //"FilterMeister",              // address of title of message box  

        MB_ICONQUESTION|MB_YESNOCANCEL  // style of message box
        | MB_TASKMODAL                  // Force task modal style.
       );
} /*YesNoCancel*/




LARGE_INTEGER start;
clock_t startclock;

int startClock(){

    if (!QueryPerformanceCounter(&start)) startclock = clock();

    return true;
}

int stopClock(){

    LARGE_INTEGER stop;
    LARGE_INTEGER proc_freq;
    double diff;

    if (QueryPerformanceCounter(&stop)){
        if (QueryPerformanceFrequency(&proc_freq)){
            diff = ((stop.QuadPart - start.QuadPart) / (proc_freq.QuadPart/1000.0) );
            return (int)diff;
        }
    } else {
        int stopclock = clock();
        return (int)(stopclock-startclock);
    }

    return 0;
}


int convertColor(int16 sourceSpace, int resultSpace, FilterColor color)
{
    //0=plugIncolorServicesRGBSpace
    //1=plugIncolorServicesHSBSpace
    //2=plugIncolorServicesCMYKSpace
    //3=plugIncolorServicesLabSpace
    //4=plugIncolorServicesGraySpace
    //5=plugIncolorServicesHSLSpace
    //6=plugIncolorServicesXYZSpace
   
    int16 a;

	ColorServicesInfo csInfo;

	csInfo.selector = plugIncolorServicesConvertColor;

	csInfo.sourceSpace = sourceSpace; //CSModeToSpace((int16)sourceMode); //plugIncolorServicesRGBSpace;
    csInfo.resultSpace = resultSpace; //CSModeToSpace((int16)resultMode); //CSModeToSpace(gFilterRecord->imageMode);

	csInfo.reservedSourceSpaceInfo = NULL;
	csInfo.reservedResultSpaceInfo = NULL;
	csInfo.reserved = NULL;
	csInfo.selectorParameter.pickerPrompt = NULL;
	csInfo.infoSize = sizeof(csInfo);
	
    for (a = 0; a < 4; a++)
		csInfo.colorComponents[a] = color[a];

    if (!gStuff->colorServices(&csInfo)){
		for (a = 0; a < 4; a++)
			color[a] = (int8)csInfo.colorComponents[a];
        return true;
    }

    return false;

}



int getProperty(int property, int notsupported){

#ifndef APP
    GetPropertyProc pGPP;   //pointer to getPropertyProc
    OSErr err;
    intptr_t simpleProperty;
    Handle complexProperty;

    //Check if available
    pGPP = gStuff->getPropertyObsolete;
    if (gStuff->propertyProcs) {
        if (gStuff->propertyProcs->propertyProcsVersion >= kCurrentPropertyProcsVersion &&
            gStuff->propertyProcs->numPropertyProcs >= 1 &&
            gStuff->propertyProcs->getPropertyProc) {
            //prefer this version of getPropertyProc over the obsolete one
            pGPP = gStuff->propertyProcs->getPropertyProc;
        }
    } //propertyProcs != 0


    if (pGPP) {
        err = pGPP('8BIM', property, 0, &simpleProperty, &complexProperty);
        if (err == noErr){
            return simpleProperty;
        }
    } 
#endif

    return notsupported; //Not Supported

}


int setCtlClass(int iCtl, int iClass, int absy, int height, int thumbSize){

    int  val       = gParams->ctl[iCtl].val;
    int  initVal   = gParams->ctl[iCtl].initVal;
    int  minval    = gParams->ctl[iCtl].minval;
    int  maxval    = gParams->ctl[iCtl].maxval;
    int  lineSize  = gParams->ctl[iCtl].lineSize;
    int  pageSize  = gParams->ctl[iCtl].pageSize;
	//int  thumbSize  = gParams->ctl[iCtl].thumbSize;
    int  imageType = gParams->ctl[iCtl].imageType;
    int  divisor   = gParams->ctl[iCtl].divisor;
    int  ticFreq   = gParams->ctl[iCtl].tb.ticFreq;
    CTLACTION action = gParams->ctl[iCtl].action;
    COLORREF textColor = gParams->ctl[iCtl].textColor;
    COLORREF bkColor = gParams->ctl[iCtl].bkColor;
    char tooltip[MAX_TOOLTIP_SIZE+1];
    char image[_MAX_PATH+1];
	int noeditborder=false;
	int anchor = gParams->ctl[iCtl].anchor;
	int tabControl = gParams->ctl[iCtl].tabControl;
	int tabSheet = gParams->ctl[iCtl].tabSheet;
    int scripted = gParams->ctl[iCtl].scripted;
    
    strcpy(tooltip, gParams->ctl[iCtl].tooltip);
    strcpy(image, gParams->ctl[iCtl].image);

	if (!(gParams->ctl[iCtl].buddy1Style & WS_BORDER)) noeditborder=true;
	

    deleteCtl(iCtl);

    gParams->ctl[iCtl].ctlclass = (CTLCLASS)iClass;
    if (height>-1) gParams->ctl[iCtl].height = height;
    //gParams->ctl[iCtl].thumbSize = thumbSize;
    if (absy!=0) gParams->ctl[iCtl].yPos += absy;
    
    createCtl(iCtl,
                 gParams->ctl[iCtl].ctlclass,         //class
                 gParams->ctl[iCtl].label,    //text
                 gParams->ctl[iCtl].xPos,          //x
                 gParams->ctl[iCtl].yPos,          //y
                 gParams->ctl[iCtl].width,         //w
                 gParams->ctl[iCtl].height,        //h
                 0,//gParams->ctl[iCtl].style,         //styles
                 0,//gParams->ctl[iCtl].styleEx,       //extended styles
                 0,//gParams->ctl[iCtl].properties,    //properties
                 gParams->ctl[iCtl].enabled);      //visible/enabled
    
    setCtlRange(iCtl, minval, maxval);
    // setCtlTheme(iCtl, gParams->ctl[iCtl].theme); // Overwrites the background color if theme enabled
	setCtlTab(iCtl, tabControl, tabSheet);		// Overwrites the background color if theme enabled
    gParams->ctl[iCtl].initVal = initVal;
    setCtlLineSize(iCtl, lineSize);
    setCtlPageSize(iCtl, pageSize);
	//setCtlThumbSize(iCtl, thumbSize);
    setCtlAction(iCtl, action);
    setCtlVal(iCtl, initVal);
    setCtlFontColor(iCtl, textColor);
    //setCtlColor(iCtl, bkColor);
    setCtlToolTip(iCtl, tooltip, 0);
    setCtlImage(iCtl, image, imageType);
    setCtlDivisor(iCtl, divisor);
	setCtlAnchor(iCtl, anchor);
    setCtlScripting(iCtl, scripted);
    setCtlTab(iCtl,tabControl,tabSheet);

	//Set buddy styles
	if (gParams->ctl[iCtl].ctlclass == CC_STANDARD || gParams->ctl[iCtl].ctlclass == CC_SLIDER){
		setCtlBuddyStyle (iCtl,0,gParams->ctl[iCtl].buddy2Style);
		setCtlBuddyStyle (iCtl,1,gParams->ctl[iCtl].buddy1Style);
		setCtlBuddyStyleEx (iCtl,0,gParams->ctl[iCtl].buddy2StyleEx);
		setCtlBuddyStyleEx (iCtl,1,gParams->ctl[iCtl].buddy1StyleEx);
		if (noeditborder) clearCtlBuddyStyle (iCtl,1,WS_BORDER);
	}

    switch (gParams->ctl[iCtl].ctlclass) {
    case CC_COMBOBOX:
	case CC_LISTBAR:
        setCtlLabel(iCtl,gParams->ctl[iCtl].label2);
        break;
    case CC_TRACKBAR:
	case CC_SLIDER:
        setCtlTicFreq(iCtl, ticFreq);
        break;
    default:
        break;
    } //switch class


    return true;

}

int isValidFilename (char * filename){

    if (filename == NULL) return false;

    // '\', '/', ':', '*', '?', '"', '<', '>', '|'
    if (strchr(filename,'\\') != NULL) return false;
    if (strchr(filename,'/') != NULL) return false;
    if (strchr(filename,':') != NULL) return false;
    if (strchr(filename,'*') != NULL) return false;
    if (strchr(filename,'?') != NULL) return false;
    if (strchr(filename,'"') != NULL) return false;
    if (strchr(filename,'<') != NULL) return false;
    if (strchr(filename,'>') != NULL) return false;
    if (strchr(filename,'|') != NULL) return false;

    return true;
}

int convertToFilename (char * filename){

    int i=0; //j,length;
    char letter;

    if (filename == NULL) return false;

    while (i<(int)strlen(filename)){   
        letter = filename[i];
        if (letter == '\\' || letter == '/' || letter == ':' || letter == '*' || letter == '?' || letter == '"' || letter == '<' || letter == '>' || letter == '|'){
            memmove(filename+i,filename+i+1,strlen(filename)-i);
            //length = strlen(filename)-i;
            //for(j=0;j<length;j++) *(filename+i+j) = *(filename+i+1+j);
        }else
            i++;
    }
    
    return true;
}

int stripEndSpaces(char * string){

    int i; 
    int length = (int)strlen(string);

    for (i=length-1; i>=0; i--){
        if (string[i] != 32){
            if (i!=length-1) *(string+i+1) = 0;
            break;
        }
    }

    return true;
}

int keepFirstWord(char * string){

    int i; 
    int length = (int)strlen(string);

    for (i=0; i<length; i++){
        if (string[i] == 32){
            *(string+i) = 0;
            break;
        }
    }

    return true;
}




int toLowerCase(char * str) 
{   
    int i;

    for (i=0;i<(int)strlen(str);i++){        
        if (str[i] >= 65 && str[i] <= 90) str[i] += 32; 
        //if (str[i] >= 97 && str[i] <= 122) str[i] -= 32; //ToUpper
    }

    return true;
}

int retrieveFilename(char * path, char * filename)
{
	char * pch;

	if (strlen(path)==0) return false;
					        
	pch=strrchr(path,0x5C);

	if (pch != NULL){
		strcpy (filename, pch+1);
	} else 
		strcpy (filename, path);

	return true;
}

int retrieveFilenameNoExt(char * path, char * filename)
{
	char * pch;

	if (strlen(path)==0) return false;
					        
	pch=strrchr(path,0x5C);

	if (pch != NULL){
		strcpy (filename, pch+1);
	} else 
		strcpy (filename, path);

	pch=strrchr(filename,0x2E);
	
	if (pch == NULL) return false;
	
	memset(filename+(pch-filename),0,1);

	return true;
}

int retrieveFolder(char * path, char * folder)
{
	char * pch;
	
	if (strlen(path)==0) return false;

	pch=strrchr(path,0x5C);

	if (pch == NULL) return false;

	if (pch == path+strlen(path)-1){
		strcpy (folder, path);
	} else {
		memcpy(folder,path,pch-path+1);
		memset(folder+(pch-path)+1,0,1);
	}

	if (strlen(folder)>0 && folder[strlen(folder)-1] != 0x5C) strcat(folder,"\\");
		
	return true;
}



int fileExists(char *fileName){    
	DWORD	fileAttr;    
	
	fileAttr = GetFileAttributes(fileName);    
	
	//return (fileAttr != INVALID_FILE_ATTRIBUTES &&  !(fileAttr & FILE_ATTRIBUTE_DIRECTORY));

	if (fileAttr == 0xFFFFFFFF) return false;
	return true;

}

int shellExecuteAndWait(char * lpVerb, char * lpFile){

	BOOL retval=0;
	SHELLEXECUTEINFO ExecInfo;

	ExecInfo.cbSize = sizeof(SHELLEXECUTEINFO);
	ExecInfo.fMask = SEE_MASK_NOCLOSEPROCESS; //NULL;
	ExecInfo.hwnd = fmc.doingProxy?fmc.hDlg:NULL;
	ExecInfo.lpVerb = (LPCTSTR)lpVerb;
	ExecInfo.lpFile = (LPCTSTR)lpFile;
	ExecInfo.lpParameters = NULL;
    ExecInfo.lpDirectory = NULL;
    ExecInfo.nShow = SW_SHOWNORMAL;
    ExecInfo.hInstApp = NULL;

	retval = ShellExecuteEx(&ExecInfo);

	//Wait
	if (ExecInfo.hProcess){
		WaitForSingleObject(ExecInfo.hProcess,INFINITE);
		CloseHandle(ExecInfo.hProcess);
	}

	return retval;
	

	/*
	STARTUPINFO si;
    PROCESS_INFORMATION pi;
	char * strTemp = calloc(512,1);

	sprintf(strTemp,"explorer.exe %s",lpFile);

    ZeroMemory( &si, sizeof(si) );
    si.cb = sizeof(si);
    ZeroMemory( &pi, sizeof(pi) );

    // Start the child process. 
    if(CreateProcess( NULL,   // No module name (use command line)
        (LPSTR)strTemp,        // Command line
        NULL,           // Process handle not inheritable
        NULL,           // Thread handle not inheritable
        FALSE,          // Set handle inheritance to FALSE
        0,              // No creation flags
        NULL,           // Use parent's environment block
        NULL,           // Use parent's starting directory 
        &si,            // Pointer to STARTUPINFO structure
        &pi )           // Pointer to PROCESS_INFORMATION structure
    ){
        // Wait until child process exits.
		WaitForSingleObject( pi.hProcess, INFINITE );

		// Close process and thread handles. 
		CloseHandle( pi.hProcess );
		CloseHandle( pi.hThread );

        return true;
    }*/

    return false;

}



/*
HACCEL hAcceleratorTable = 0;
LPACCEL acceleratorArray = 0;
int acceleratorCount = 0;

int addKeyShortcut (int n, int key, int fVirt){

	hAcceleratorTable = NULL;
	if (hAcceleratorTable) DestroyAcceleratorTable(hAcceleratorTable);

	acceleratorCount++;
	acceleratorArray = realloc(acceleratorArray,acceleratorCount*sizeof(ACCEL));
	
	acceleratorArray[acceleratorCount-1].cmd = n;
	acceleratorArray[acceleratorCount-1].key = key;
	acceleratorArray[acceleratorCount-1].fVirt = fVirt;

	hAcceleratorTable = CreateAcceleratorTable(acceleratorArray,acceleratorCount);

	//Info ("acc: %d",hAcceleratorTable);

	return (int)hAcceleratorTable;
}*/

int getCtlItemString(int n, int item, char * string){

	char *p, *q, q_save;
    char* copy;
	int count = 0;
    
    copy = (char *)malloc(strlen(gParams->ctl[n].label)+1);
    strcpy(copy,gParams->ctl[n].label);

    p = copy; 
    while (*p) {
        q = p; while(*q != '\0' && *q != '\n') q++;

        q_save = *q;
        *q = '\0';

		if (count == item) {
			if (string) strcpy(string,p);
			break;
		}
		count++;

        *q = q_save;
        p = q;
        if (*p) p++;
    } //while 
	
    free (copy);

	return count;
}


int gammaCtlVal(int iCtl, int val, int invert){

	int gamma = gParams->ctl[iCtl].gamma;
	double r, absRange;

	if (gamma == 100) return val; //Not necessary

	r = gamma/100.0;
	if (!invert) r = 1.0/r;  //Inverse Gamma?


    if (gParams->ctl[iCtl].minval >= 0){
        absRange = (double) abs (gParams->ctl[iCtl].maxval - gParams->ctl[iCtl].minval) - 1.0;
        return gParams->ctl[iCtl].minval + (int)(pow( (val - gParams->ctl[iCtl].minval) / absRange, r) * absRange + 0.5);
    } else {
        if (val >= 0){
            absRange = (double) gParams->ctl[iCtl].maxval - 1.0;
            return (int)(pow( val / absRange, r) * absRange + 0.5);
        } else {
            absRange = (double) abs (gParams->ctl[iCtl].minval) - 1.0;
            return -(int)(pow( abs(val) / absRange, r) * absRange + 0.5);
        }
    }
}

void drawPreviewColor(HDC hDC){

	if (gParams->ctl[CTL_PREVIEW].bkColor != -1){

		if (hDC ==(HDC)-1) hDC = GetDC(fmc.hDlg);

		//Only if image does not fill the preview
		if ( (gFmcp->x_start == 0 && gFmcp->x_end == gFmcp->X) || (gFmcp->y_start == 0 && gFmcp->y_end == gFmcp->Y) ){
			HBRUSH hBrush;
			RECT rect;
			//HDC hDC = (HDC) wParam;

			GetWindowRect( GetDlgItem(fmc.hDlg, 101), &rect );
			ScreenToClient (fmc.hDlg, (LPPOINT)&rect);
			ScreenToClient (fmc.hDlg, (LPPOINT)&(rect.right));
			hBrush = CreateSolidBrush(gParams->ctl[CTL_PREVIEW].bkColor);
			FillRect(hDC, &rect, hBrush);
			DeleteObject(hBrush);
		}
	}

}

void drawSizeGrip(){

	RECT rc;
	HDC  hDC;

	GetClientRect(fmc.hDlg, &rc ); 
	rc.left = rc.right - GetSystemMetrics(SM_CXHSCROLL); 
	rc.top = rc.bottom - GetSystemMetrics(SM_CYVSCROLL);

#if SKIN
	//Delete previous grip
	/*if (!isSkinActive() || gParams->skinStyle==0){
		static RECT rcOld;
		if (rcOld.right - rcOld.left > 0 && !(EqualRect(&rcOld, &rc)==1) ) {
			HBRUSH hBr;
			hBr = GetSysColorBrush(COLOR_BTNFACE);
			FillRect (hDC, &rcOld, hBr);
			DeleteObject (hBr);
		}
		rcOld = rc;
	}*/
#endif

	/*if (getAppTheme()) drawThemePart(-1,&rc,L"SCROLLBAR",10,1);*/
#if SKIN		
	else if (isSkinActive() && gParams->skinStyle>0){ //Simulate handle
		int i; 
		int hicolor = GetSysColor(COLOR_BTNHILIGHT);
		int locolor = GetSysColor(COLOR_BTNSHADOW);
		startSetPixel(-1);
			for (i=0;i<3;i++){
				setPenWidth(2);
				setLine(rc.left+6+i*4,rc.bottom,rc.right,rc.top+6+i*4,locolor);
				setLine(rc.left+8+i*4,rc.bottom,rc.right,rc.top+8+i*4,hicolor);
				setPenWidth(1);
			}
		endSetPixel(-1);
	} 
#endif	
	// else {
		hDC = GetDC(fmc.hDlg);
		DrawFrameControl(hDC, &rc, DFC_SCROLL, DFCS_SCROLLSIZEGRIP );
		ReleaseDC(fmc.hDlg, hDC);
	// }
	
}

void resetTabCtls(int iCtl){

	int i,oldvalue;
	int previewUpdate = false;
	int val = fmc.pre_ctl[iCtl];
	int subtab;

	for (i = 0; i < N_CTLS; ++i) {

		//Get sub tab
		if (gParams->ctl[i].tabControl > 0) 
			subtab = gParams->ctl[gParams->ctl[i].tabControl].tabControl; else subtab = -1;

		if (gParams->ctl[i].inuse && gParams->ctl[i].defval != -1 && 
			((gParams->ctl[i].enabled == 3 && gParams->ctl[i].tabControl == iCtl &&  gParams->ctl[i].tabSheet == val) || subtab == iCtl)
		){
		
			oldvalue = gParams->ctl[i].val;
			setCtlVal(i,gParams->ctl[i].defval);
			if (gParams->ctl[i].ctlclass == CC_OWNERDRAW)
				setCtlColor(i,gParams->ctl[i].defval);
			else if (gParams->ctl[i].ctlclass == CC_CHECKBOX || gParams->ctl[i].ctlclass == CC_COMBOBOX)
				triggerEvent(i,FME_CLICKED,oldvalue);

			if (gParams->ctl[i].action == CA_PREVIEW) previewUpdate = true;
		}	
	}
	
	triggerEvent(iCtl,FME_TABRESET,oldvalue);
	
	if (previewUpdate) doAction(CA_PREVIEW);

}

void removeChar(char * str, char garbage) {

    char *src, *dst;
    for (src = dst = str; *src != '\0'; src++) {
        *dst = *src;
        if (*dst != garbage) dst++;
    }
    *dst = '\0';
}

int fm_isnan(double x)
{
   return x != x;
}

int fm_isinf(double x)
{
   if (/*(x == x) &&*/ ((x - x) != 0.0)) return (x < 0.0 ? -1 : 1);
   else return 0;
}




/*
char hostName[32] = "";
int hostVersion = 0;
int hostVersionMinor = 0;
BOOL isPhotoshop = true;
BOOL isElements = false;

#ifndef APP

BOOL getHostApp(){

	char appPath[512];
	GetModuleFileName(GetModuleHandle(NULL),appPath,512);
	toLowerCase(appPath);
	
	//Get Host Application
	if (strcmp(&appPath[strlen(appPath)-13],"photoshop.exe") == 0 || 
		strcmp(&appPath[strlen(appPath)-12],"photoshp.exe") == 0
	){
		strcpy_s(hostName,32,"Photoshop");
		isPhotoshop = true;
		isElements = false;
	} else {
		strcpy_s(hostName,32,"Elements");
		isPhotoshop = false;
		isElements = true;
	}
	
	{
		//Get Version
		DWORD  verHandle = 0;
		UINT   size      = 0;
		LPBYTE lpBuffer  = NULL;
		DWORD  verSize   = GetFileVersionInfoSize( appPath, &verHandle);

		if (verSize){
			LPSTR verData = malloc(verSize);
			if (GetFileVersionInfo( appPath, verHandle, verSize, verData)){
				if (VerQueryValue(verData,"\\",(VOID FAR* FAR*)&lpBuffer,&size)){
					if (size){
						VS_FIXEDFILEINFO *verInfo = (VS_FIXEDFILEINFO *)lpBuffer;
						if (verInfo->dwSignature == 0xfeef04bd){
							//int major = HIWORD(verInfo->dwFileVersionMS);
							//int minor = LOWORD(verInfo->dwFileVersionMS);
							//int build = verInfo->dwFileVersionLS;
							//sprintf_s(appString,128,"%s %d",appString,major);
							//*appVersion = HIWORD(verInfo->dwFileVersionMS);
							hostVersion = HIWORD(verInfo->dwFileVersionMS);
							hostVersionMinor = LOWORD(verInfo->dwFileVersionMS);
							//if (isElements && hostVersion==13)
							//	hostVersionMinor = checkPSE13MinorVersion(appPath);
						}
					}
				}
			}
			free(verData);
		}
	}

	return true;
}

#endif

BOOL getRegistryData(HKEY hRootKey, char *subKey, char *value, LPBYTE data, DWORD cbData, int WowAccess)
{
	HKEY hKey;
	LONG error;
	REGSAM regSam = KEY_QUERY_VALUE;
	if (WowAccess==1) regSam |= KEY_WOW64_64KEY;
	else if (WowAccess==2) regSam |= KEY_WOW64_32KEY;
	error = RegOpenKeyEx(hRootKey, subKey, 0, regSam, &hKey);
	if (error) return false;

	error = RegQueryValueEx(hKey, value, NULL, NULL, data, &cbData);
	RegCloseKey(hKey);
	if (error) return false;
	
	return true;
}


BOOL checkUIScaling(){

	BOOL UIScaling = false;

#ifndef APP

	if (hostVersion==0) getHostApp();

	if ((isElements && hostVersion>=13) || (isPhotoshop && hostVersion>=15)){
		
		double factorX = 1.0, factorY = 1.0;
		DWORD dScale = -1;
		char registryStr[256];
		BOOL retval = false;

		getSysDPI(&factorX,&factorY);
		
		if (isElements)
			sprintf_s(registryStr,256,"Software\\Adobe\\Photoshop Elements\\%d.0",hostVersion);
		else {
			int versionVal = 60 + (hostVersion-13)*10;
			sprintf_s(registryStr,256,"Software\\Adobe\\Photoshop\\%d.0",versionVal);
		}

		
		if (isPhotoshop && hostVersion>=16)
			retval = getRegistryData(HKEY_CURRENT_USER,registryStr,"UIScale",(LPBYTE)&dScale,4,128);
		else
			retval = getRegistryData(HKEY_CURRENT_USER,registryStr,"UIScalingSize",(LPBYTE)&dScale,4,128);
		
		if (!retval) //UI scaling Works even without reg key
			dScale = 1;

		if (dScale == 2){
			UIScaling = true;
		} else if (dScale == 1){
			int width  = GetSystemMetrics(SM_CXSCREEN);
			int height = GetSystemMetrics(SM_CYSCREEN);
			if (width>=2560 && height>=1600 && max(factorX,factorY)>= 1.5) //3840 x 2160
				UIScaling = true;
		}
	}

#endif

	return UIScaling;

}
*/

int getSysDPI(double * factorX, double * factorY){

	int dpiX,dpiY;

	HDC hdc = GetDC(NULL);
    if (hdc){
        if (factorX) dpiX = GetDeviceCaps(hdc, LOGPIXELSX);
        if (factorY) dpiY = GetDeviceCaps(hdc, LOGPIXELSY);
        ReleaseDC(NULL, hdc);

		if (factorX) *factorX =  dpiX / 96.0;
		if (factorY) *factorY =  dpiY / 96.0;
		return max(dpiX,dpiY);
	}
	
	return 0;
}

BOOL isHDPIAware()
{

#ifndef APP

    GetPropertyProc pGPP;
    OSErr err;
    intptr_t simpleProperty;

    pGPP = gStuff->getPropertyObsolete;
    if (gStuff->propertyProcs) {
        if (gStuff->propertyProcs->propertyProcsVersion >= kCurrentPropertyProcsVersion &&
            gStuff->propertyProcs->numPropertyProcs >= 1 &&
            gStuff->propertyProcs->getPropertyProc) {
            //prefer this version of getPropertyProc over the obsolete one
            pGPP = gStuff->propertyProcs->getPropertyProc;
        }
    } //propertyProcs != 0

    if (pGPP) {
        err = pGPP('8BIM', 'HDPI', 0, &simpleProperty, NULL);
        if (!err) {
            if (simpleProperty) 
				return true;
        }
    }   

#endif

    return false;

}

//#include <tpcshrd.h>
#define MICROSOFT_TABLETPENSERVICE_PROPERTY "MicrosoftTabletPenServiceProperty"
#define TABLET_DISABLE_PRESSANDHOLD        0x00000001
#define TABLET_DISABLE_PENTAPFEEDBACK      0x00000008
#define TABLET_DISABLE_PENBARRELFEEDBACK   0x00000010
#define TABLET_DISABLE_TOUCHUIFORCEON      0x00000100
#define TABLET_DISABLE_TOUCHUIFORCEOFF     0x00000200
#define TABLET_DISABLE_TOUCHSWITCH         0x00008000
#define TABLET_DISABLE_FLICKS              0x00010000
#define TABLET_ENABLE_FLICKSONCONTEXT      0x00020000
#define TABLET_ENABLE_FLICKLEARNINGMODE    0x00040000
#define TABLET_DISABLE_SMOOTHSCROLLING     0x00080000
#define TABLET_DISABLE_FLICKFALLBACKKEYS   0x00100000
#define TABLET_ENABLE_MULTITOUCHDATA       0x01000000

void deactivateFlicks(int n) {

	HWND hwnd;
	const DWORD_PTR dwHwndTabletProperty =
		TABLET_DISABLE_PRESSANDHOLD | // disables press and hold (right-click) gesture          
		TABLET_DISABLE_PENTAPFEEDBACK | // disables UI feedback on pen up (waves)          
		TABLET_DISABLE_PENBARRELFEEDBACK | // disables UI feedback on pen button down          
		TABLET_DISABLE_FLICKS; // disables pen flicks (back, forward, drag down, drag up)      
	ATOM atom = GlobalAddAtom(MICROSOFT_TABLETPENSERVICE_PROPERTY); //GlobalAddAtom(MICROSOFT_TABLETPENSERVICE_PROPERTY);

	if (n<0) hwnd = fmc.hDlg;
	else if (n == CTL_PREVIEW) GetDlgItem(fmc.hDlg, 101);
	else if (n == CTL_ZOOM) hwnd = GetDlgItem(fmc.hDlg, IDC_BUTTON1);
	else if (n == CTL_FRAME) hwnd = GetDlgItem(fmc.hDlg, IDC_PROXY_GROUPBOX);
	else if (n == CTL_PROGRESS) hwnd = GetDlgItem(fmc.hDlg, IDC_PROGRESS1);
	else hwnd = gParams->ctl[n].hCtl;

	SetProp(hwnd, MICROSOFT_TABLETPENSERVICE_PROPERTY, (HANDLE)dwHwndTabletProperty); //reinterpret_cast(dwHwndTabletProperty));
	GlobalDeleteAtom(atom);
}

void setDialogTitle(char * title){

	strcpy_s(fmc.filterTitleText, 256, title);

}


int evalZoomCombo(int n, int ctlPlus, int ctlMinus, int previous){

    int NewZoom=0;
    int NewLarge=1;
    int forceUpdate=0;
    int retval;
    int oldscale=fmc.scaleFactor;

    
#ifdef HIGHZOOM

    /*if (fmc.pre_ctl[n]==27){ //Softproof
        NewZoom = SoftProof();
        if (fmc.enlargeFactor==1 && scaleFactor==NewZoom && previous!=24) forceUpdate=true;
    }else*/ if (fmc.pre_ctl[n]==26){ //Fit
		NewZoom=-888;
    }else if (fmc.pre_ctl[n]>=0 && fmc.pre_ctl[n]<=9){ //800-200
		NewZoom=1;
        if (fmc.pre_ctl[n]==0) NewLarge=32;
        else if (fmc.pre_ctl[n]==1) NewLarge=16;
        else if (fmc.pre_ctl[n]==2) NewLarge=12;
        else NewLarge=11-fmc.pre_ctl[n];
    }else if (fmc.pre_ctl[n]==10 && previous<fmc.pre_ctl[n]){ //previous==fmc.pre_ctl[n]-1
		setCtlVal (n,11);
		NewZoom=1;
	}else if (fmc.pre_ctl[n]==10 && previous>fmc.pre_ctl[n]){//previous==fmc.pre_ctl[n]+1
		setCtlVal (n,9);
		NewZoom=1;
        NewLarge=2;
    }else if (fmc.pre_ctl[n]==25 && previous<fmc.pre_ctl[n]){ //previous==fmc.pre_ctl[n]-1
		setCtlVal (n,26);
		NewZoom=-888;
	}else if (fmc.pre_ctl[n]==25 && previous>fmc.pre_ctl[n]){//previous==fmc.pre_ctl[n]+1
		setCtlVal (n,24);
		NewZoom=16;
	} else {
		NewZoom= fmc.pre_ctl[n]-10;//+1
		if (NewZoom>=13) NewZoom=NewZoom+1;
		if (NewZoom==15) NewZoom=NewZoom+1;
		if (NewZoom<1) NewZoom=1;
		if (NewZoom>16) NewZoom=16;
	}

#else

	/*if (fmc.pre_ctl[n]==16){ //Softproof
        NewZoom = SoftProof();
        if (fmc.enlargeFactor==1 && fmc.scaleFactor==NewZoom && previous!=16) forceUpdate=true;
    }else*/ if (fmc.pre_ctl[n]==15){ //Fit
		NewZoom=-888;
	}else if (fmc.pre_ctl[n]==14 && previous<fmc.pre_ctl[n]){ //previous==fmc.pre_ctl[n]-1
		setCtlVal (n,15);
		NewZoom=-888;
	}else if (fmc.pre_ctl[n]==14 && previous>fmc.pre_ctl[n]){//previous==fmc.pre_ctl[n]+1
		setCtlVal (n,13);
		NewZoom=16;
	} else {
		NewZoom= fmc.pre_ctl[n]+1;
		if (NewZoom>=13) NewZoom=NewZoom+1;
		if (NewZoom==15) NewZoom=NewZoom+1;
		if (NewZoom<1) NewZoom=1;
		if (NewZoom>16) NewZoom=16;
	}
#endif

    #ifdef HIGHZOOM
		if (NewZoom==1 && NewLarge==32)  enableCtl(ctlPlus,1);  else enableCtl(ctlPlus,-1);
    #else
        if (NewZoom==1 )  enableCtl(ctlPlus,1);  else enableCtl(ctlPlus,-1);
    #endif

    if (NewZoom==16 )  enableCtl(ctlMinus,1);  else enableCtl(ctlMinus,-1);
	
	
    //if (NewZoom != scaleFactor) 
    #ifdef HIGHZOOM 
        retval = setZoom(NewZoom,NewLarge);
        updatePreview(0);
    #else
        retval = setZoom(NewZoom,1);
    #endif
	
	setCtlFocus(n); //Focus Back to the zoom combo box

    //Force Preview Update
    #ifdef HIGHZOOM
        if (retval==false && (forceUpdate || (fmc.pre_ctl[n]!=27 && previous==27)) ){
            doAction(CA_PREVIEW);
            retval=true;
        }
    #else
        if (retval==false && (forceUpdate || (fmc.pre_ctl[n]!=16 && previous==16)) ) {
            doAction(CA_PREVIEW);
            retval=true;
        }
    #endif


	triggerEvent(CTL_PREVIEW,FME_ZOOMCHANGED,0);
	
	return retval; //Return Preview Update
	
}


int evalZoomButtons(int n, int ctlCombo, int ctlPlus, int ctlMinus){

    int NewZoom=0;
    int NewLarge=1;
	
	if (n==ctlMinus){ //Minus
		
#ifndef CREATEDIALOGPARAM
		if (getAsyncKeyStateF(VK_CONTROL)<0){
			if (fmc.enlargeFactor==1)
                NewZoom=16;
            else
                NewZoom=1;
		} else if (getAsyncKeyStateF(VK_SHIFT)<0 ){
			if (fmc.enlargeFactor==1)
                NewZoom=-888;
            else {
                NewZoom=1;
                NewLarge=1;
            }
		} else 
#endif		
		{
            if (fmc.enlargeFactor==1){
                NewZoom=fmc.scaleFactor+1;
				if (NewZoom==13 || NewZoom==15) NewZoom=NewZoom+1;
				if (NewZoom<1) NewZoom=1;
				if (NewZoom>16) NewZoom=16;
            } else {
                NewZoom=1;
                if (fmc.enlargeFactor==32) NewLarge=fmc.enlargeFactor=16;
                else if (fmc.enlargeFactor==16) NewLarge=fmc.enlargeFactor=12;
                else if (fmc.enlargeFactor==12) NewLarge=fmc.enlargeFactor=8;
                else 
                    NewLarge=fmc.enlargeFactor-1;
            }
		}
		
		if (NewZoom==16) enableCtl(ctlMinus,1);
		enableCtl(ctlPlus,-1);
		
		if (NewZoom != -888){
			#ifdef HIGHZOOM
                if (NewZoom==14)
					setCtlVal(ctlCombo, 23);
				else if (NewZoom==16)
					setCtlVal(ctlCombo, 24);
                else if (NewLarge==1 && NewZoom>=1)
                    setCtlVal(ctlCombo, 10+NewZoom);
                else if (NewLarge>=1){
                    if (NewLarge==32) setCtlVal(ctlCombo, 0);
                    else if (NewLarge==16) setCtlVal(ctlCombo, 1);
                    else if (NewLarge==12) setCtlVal(ctlCombo, 2);
                    else 
                        setCtlVal(ctlCombo, 11-NewLarge);
                }
            #else
                if (NewZoom==14)
					setCtlVal(ctlCombo, NewZoom-2);
				else if (NewZoom==16)
					setCtlVal(ctlCombo, NewZoom-3);
				else
					setCtlVal(ctlCombo, NewZoom-1);
            #endif
		} else {
			#ifdef HIGHZOOM
                setCtlVal(ctlCombo, 26);
            #else
                setCtlVal(ctlCombo, 15);
            #endif
		}
				
		#ifdef HIGHZOOM 
            setZoom(NewZoom,NewLarge);
            updatePreview(0);
        #else
			setZoom(NewZoom,1);
        #endif
			
	} else if  (n==ctlPlus ) { //Plus

#ifndef CREATEDIALOGPARAM
		if (getAsyncKeyStateF(VK_CONTROL)<0 || getAsyncKeyStateF(VK_SHIFT)<0 ){
			if (fmc.enlargeFactor==1 && fmc.scaleFactor>1)
                NewZoom=1;
            else {
                NewZoom=1;
                NewLarge=32;
            }
		} else 
#endif		
		{
            if (fmc.enlargeFactor==1){
                NewZoom=fmc.scaleFactor-1;
				if (NewZoom==13 || NewZoom==15) NewZoom=NewZoom-1;
				
                if (NewZoom<1) {
                    NewZoom=1;
                    #ifdef HIGHZOOM
                        NewLarge=2;
                    #endif
                }
				if (NewZoom>16) NewZoom=16;
            } else {
                NewZoom=1;
				if (fmc.enlargeFactor>=16) NewLarge=fmc.enlargeFactor=32;
                else if (fmc.enlargeFactor==12) NewLarge=fmc.enlargeFactor=16;
                else if (fmc.enlargeFactor==8) NewLarge=fmc.enlargeFactor=12;
                else 
                    NewLarge=fmc.enlargeFactor+1;
            }
		}
		
		
        #ifdef HIGHZOOM
			if (NewZoom==1 && NewLarge==32)  enableCtl(ctlPlus,1);  else enableCtl(ctlPlus,-1);
        #else
            if (NewZoom==1 )  enableCtl(ctlPlus,1);  else enableCtl(ctlPlus,-1);
        #endif
		enableCtl(ctlMinus,-1); 
		
        #ifdef HIGHZOOM
            if (NewZoom==14)
				setCtlVal(ctlCombo, 23);
			else if (NewZoom==16)
				setCtlVal(ctlCombo, 24);
            else if (NewLarge==1 && NewZoom>=1)
                setCtlVal(ctlCombo, 10+NewZoom);
            else if (NewLarge>=1){
                if (NewLarge==32) setCtlVal(ctlCombo, 0);
                else if (NewLarge==16) setCtlVal(ctlCombo, 1);
                else if (NewLarge==12) setCtlVal(ctlCombo, 2);
                else 
                    setCtlVal(ctlCombo, 11-NewLarge);
            }
        #else
			if (NewZoom==14)
				setCtlVal(ctlCombo, NewZoom-2);
			else if (NewZoom==16)
				setCtlVal(ctlCombo, NewZoom-3);
			else
				setCtlVal(ctlCombo, NewZoom-1);
        #endif

        #ifdef HIGHZOOM 
            setZoom(NewZoom,NewLarge);
            updatePreview(0);
        #else
			setZoom(NewZoom,1);
        #endif
	}
	
    triggerEvent(CTL_PREVIEW,FME_ZOOMCHANGED,0);

	return true; //Preview Update already done by setZoom

}

// Functions added by Ognen Genchev
double fclamp (double x, double minVal, double maxVal)
{
    double clamp = x <= minVal ? minVal : x > maxVal ? maxVal : x;
    return clamp;
}

double fsmoothstep (double edge0, double edge1, double x)
{
    // Scale, bias and saturate x to 0..1 range
    x = fclamp((x - edge0) / (edge1 - edge0), 0.0, 1.0); 
    // Evaluate polynomial
    return x * x * (3.0 - 2.0 * x);
}
// end