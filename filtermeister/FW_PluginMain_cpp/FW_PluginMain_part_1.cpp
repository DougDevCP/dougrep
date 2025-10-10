///////////////////////////////////////////////////////////////////////////////////
//  File: FW_PluginMain.c
//
//  Source file of FilterMeisterVS which handles the plugin execution and preview
//  drawing
//
//  Copyright (c) 1997-2017 by Alex Hunter, Harald Heim & Martijn Van der Lee
//  
//  Derived from Photoshop SDK source files which are: Copyright by Adobe Systems, Inc.
// 
//  FilterMeisterVS is free software. You can redistribute and modify it
//  under the terms of the MPL2 license. See https://www.mozilla.org/en-US/MPL/2.0/
//  Any changes to the FilterMeisterVS files in the /components/filtermeister/
//  sub folder have to be published or sent to harry@thepluginsite.com. 
//  The individual filter code itself does not need to be disclosed.
//  
///////////////////////////////////////////////////////////////////////////////////


#ifdef _DEBUG
#define VERBOSE 1
#endif

#define REG_DATA_LIMIT 2048



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

#include <SHLOBJ.H> //For SHGetSpecialFolderLocation

#include "AfhFM.h"
#include "AfhFMcontext.h"

#include "PIUtilities.h"
#include <commctrl.h>   //PBM_SETPOS

#include "resource.h"

//#include <windows.h>

#include "AfhFMScripting.h"

#include "FW_FFP.h"



#define mskVal(row,rowOff,col,colOff) \
    gMskVal(row,rowOff,col,colOff)


// FilterMeister context record.
//static FMcontext fmc; //Now defined in AfhFMContext.h
//FMcontext * const gFmcp = &fmc;
FMcontext * gFmcp = &fmc;

//SPBasicSuite * sSPBasic = NULL;


#if BIGDOCUMENT
/*****************************************************************************/
/*  Following is a modifed version of FilterBigDocument.cpp
/*****************************************************************************/
//maybe these should always update the fmc fields???
// E.g., GetFilterRect() and SetFilterRect() would copy also to fmc.filterRect32???
VPoint GetImageSize(void)
{
	VPoint returnPoint;

	gFilterRecord = gStuff;
	gBigDocumentData = gStuff->bigDocumentData;

	if (gBigDocumentData != NULL)
	{
		returnPoint = gBigDocumentData->imageSize32;
	}
	else
	{
		returnPoint.h = gFilterRecord->imageSize.h;
		returnPoint.v = gFilterRecord->imageSize.v;
	}

	return returnPoint;
}

VRect GetFilterRect(void)
{
	VRect returnRect;

	gFilterRecord = gStuff;
	gBigDocumentData = gStuff->bigDocumentData;

	if (gBigDocumentData != NULL)
	{
		returnRect = gBigDocumentData->filterRect32;
	}
	else
	{
		returnRect.right = gFilterRecord->filterRect.right;
		returnRect.top = gFilterRecord->filterRect.top;
		returnRect.left = gFilterRecord->filterRect.left;
		returnRect.bottom = gFilterRecord->filterRect.bottom;
	}
	
	return returnRect;
}

VRect GetInRect(void)
{
	VRect returnRect;

	gFilterRecord = gStuff;
	gBigDocumentData = gStuff->bigDocumentData;

	if (gBigDocumentData != NULL)
	{
		returnRect = gBigDocumentData->inRect32;
	}
	else
	{
		returnRect.right = gFilterRecord->inRect.right;
		returnRect.top = gFilterRecord->inRect.top;
		returnRect.left = gFilterRecord->inRect.left;
		returnRect.bottom = gFilterRecord->inRect.bottom;
	}
	
	return returnRect;
}

VRect GetOutRect(void)
{
	VRect returnRect;

	gFilterRecord = gStuff;
	gBigDocumentData = gStuff->bigDocumentData;

	if (gBigDocumentData != NULL)
	{
		returnRect = gBigDocumentData->outRect32;
	}
	else
	{
		returnRect.right = gFilterRecord->outRect.right;
		returnRect.top = gFilterRecord->outRect.top;
		returnRect.left = gFilterRecord->outRect.left;
		returnRect.bottom = gFilterRecord->outRect.bottom;
	}
	
	return returnRect;
}

VRect GetMaskRect(void)
{
	VRect returnRect;

	gFilterRecord = gStuff;
	gBigDocumentData = gStuff->bigDocumentData;

	if (gBigDocumentData != NULL)
	{
		returnRect = gBigDocumentData->maskRect32;
	}
	else
	{
		returnRect.right = gFilterRecord->maskRect.right;
		returnRect.top = gFilterRecord->maskRect.top;
		returnRect.left = gFilterRecord->maskRect.left;
		returnRect.bottom = gFilterRecord->maskRect.bottom;
	}
	
	return returnRect;
}

VPoint GetFloatCoord(void)
{
	VPoint returnPoint;

	gFilterRecord = gStuff;
	gBigDocumentData = gStuff->bigDocumentData;

	if (gBigDocumentData != NULL)
	{
		returnPoint = gBigDocumentData->floatCoord32;
	}
	else
	{
		returnPoint.h = gFilterRecord->floatCoord.h;
		returnPoint.v = gFilterRecord->floatCoord.v;
	}

	return returnPoint;
}

VPoint GetWholeSize(void)
{
	VPoint returnPoint;

	gFilterRecord = gStuff;
	gBigDocumentData = gStuff->bigDocumentData;

	if (gBigDocumentData != NULL)
	{
		returnPoint = gBigDocumentData->wholeSize32;
	}
	else
	{
		returnPoint.h = gFilterRecord->wholeSize.h;
		returnPoint.v = gFilterRecord->wholeSize.v;
	}

	return returnPoint;
}

void SetInRect(VRect inRect)
{
	gFilterRecord = gStuff;
	gBigDocumentData = gStuff->bigDocumentData;

	if (gBigDocumentData != NULL)
	{
		gBigDocumentData->PluginUsing32BitCoordinates = 1;	//temporary kludge!!!
		gBigDocumentData->inRect32 = inRect;
	}
	else
	{
		gFilterRecord->inRect.right = (int16)inRect.right;
		gFilterRecord->inRect.top = (int16)inRect.top;
		gFilterRecord->inRect.left = (int16)inRect.left;
		gFilterRecord->inRect.bottom = (int16)inRect.bottom;
	}
}

void SetOutRect(VRect inRect)
{
	gFilterRecord = gStuff;
	gBigDocumentData = gStuff->bigDocumentData;

	if (gBigDocumentData != NULL)
	{
		gBigDocumentData->PluginUsing32BitCoordinates = 1;	//temporary kludge!!!
		gBigDocumentData->outRect32 = inRect;
	}
	else
	{
		gFilterRecord->outRect.right = (int16)inRect.right;
		gFilterRecord->outRect.top = (int16)inRect.top;
		gFilterRecord->outRect.left = (int16)inRect.left;
		gFilterRecord->outRect.bottom = (int16)inRect.bottom;
	}
}

void SetMaskRect(VRect inRect)
{
	gFilterRecord = gStuff;
	gBigDocumentData = gStuff->bigDocumentData;

	if (gBigDocumentData != NULL)
	{
		gBigDocumentData->PluginUsing32BitCoordinates = 1;	//temporary kludge!!!
		gBigDocumentData->maskRect32 = inRect;
	}
	else
	{
		gFilterRecord->maskRect.right = (int16)inRect.right;
		gFilterRecord->maskRect.top = (int16)inRect.top;
		gFilterRecord->maskRect.left = (int16)inRect.left;
		gFilterRecord->maskRect.bottom = (int16)inRect.bottom;
	}
}
/*****************************************************************************/
// end of modified FilterBigDocument.cpp
/*****************************************************************************/
#endif

    /* Forward prototypes */

void InitGlobals (GPtr globals);
void ValidateParameters (GPtr globals);

void DoParameters (GPtr globals);
void DoPrepare (GPtr globals);
void DoStart (GPtr globals);
void DoContinue (GPtr globals);
void DoFinish (GPtr globals);

//static int fm_doForEveryRow(void);
void InitContextRecord(void);

void finalCleanup(void);