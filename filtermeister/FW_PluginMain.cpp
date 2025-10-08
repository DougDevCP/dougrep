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

/*****************************************************************************/

/* All calls to the plug-in module come through this routine. It must be
   placed first in the resource. To achieve this, most development systems
   require that this be the first routine in the source. */

#ifdef __cplusplus
   #define DllExport   extern "C" __declspec( dllexport )
#else
   #define DllExport   __declspec( dllexport )
#endif



extern "C" DllExport void ENTRYPOINT (short selector,
                        FilterRecord *filterParamBlock,
                        intptr_t *data,
                        short *result)              
    {

#ifdef NDEBUG
    __try {
#endif
  
    Globals globalValues;
    GPtr globals = &globalValues;

#ifdef __cplusplus
    if (!hDllInstance) {
        /*MEMORY_BASIC_INFORMATION mbi;
		static int dummy;
		VirtualQuery( &dummy, &mbi, sizeof(mbi) );
		hDllInstance = (HINSTANCE)mbi.AllocationBase;*/
        hDllInstance = GetDLLInstance();
    }
#endif    

    if (!*data)
        {
        
            InitGlobals (globals);

            *data = (intptr_t) NewHandle (sizeof (Globals));
        
            if (!*data)
            {
                *result = memFullErr;
                return;
            }
        
            ** (GHdl) *data = globalValues;
        
        }
        
    globalValues = ** (GHdl) *data;
    //Should be the only ptr to globals that we need!!!
    *(GPtr *)&MyGlobals = globals;    //cast away const on MyGlobals this one time!
        
    gStuff = filterParamBlock;
    gResult = noErr;


	//if (selector == filterSelectorAbout){
	//	sSPBasic = ((AboutRecord*)gStuff)->sSPBasic;
	//} else {
		sSPBasic = gStuff->sSPBasic;
	//}

#if BIGDOCUMENT
    //Tell Photoshop that we support bigDocument stuff
//    if (gStuff->bigDocumentData != NULL)
//		    gStuff->bigDocumentData->PluginUsing32BitCoordinates = true;
#endif

    switch (selector)
        {

        case filterSelectorAbout:
            DoAbout (globals, selector);
            break;

        case filterSelectorParameters:
            DoParameters (globals);
            break;

        case filterSelectorPrepare:
            DoPrepare (globals);
            break;

        case filterSelectorStart:
            DoStart (globals);
            break;

        case filterSelectorContinue:
            DoContinue (globals);
            break;

        case filterSelectorFinish:
            DoFinish (globals);
            break;

        default:
            gResult = filterBadParameters;  // NFG for PSP???
            break;

    }

    if (gResult == filterBadParameters)
    {   // PSP and others don't recognize the filterBadParameters error,
        // so put out our own message box...
        ShowAlert(16502);
        gResult = 1;    // indicates error message already displayed.
    }


    *result = gResult;
    
    ** (GHdl) *data = globalValues;


#ifdef NDEBUG
    }
    __except (EXCEPTION_EXECUTE_HANDLER) {
        ErrorOk ("Sorry, a framework error occured."); //,(int)_exception_code()
        //gResult = userCanceledErr;
    }
#endif

        
}

/*
DllExport void ENTRYPOINT00 (short selector, FilterRecord *filterParamBlock, intptr_t *data, short *result)
{
	fmc.entryPoint = 0;
	ENTRYPOINT (selector, filterParamBlock, data, result);
}
DllExport void ENTRYPOINT01 (short selector, FilterRecord *filterParamBlock, intptr_t *data, short *result)
{
	fmc.entryPoint = 1;
	ENTRYPOINT (selector, filterParamBlock, data, result);
}
DllExport void ENTRYPOINT02 (short selector, FilterRecord *filterParamBlock, intptr_t *data, short *result)
{
	fmc.entryPoint = 2;
	ENTRYPOINT (selector, filterParamBlock, data, result);
}
DllExport void ENTRYPOINT03 (short selector, FilterRecord *filterParamBlock, intptr_t *data, short *result)
{
	fmc.entryPoint = 3;
	ENTRYPOINT (selector, filterParamBlock, data, result);
}
DllExport void ENTRYPOINT04 (short selector, FilterRecord *filterParamBlock, intptr_t *data, short *result)
{
	fmc.entryPoint = 4;
	ENTRYPOINT (selector, filterParamBlock, data, result);
}
DllExport void ENTRYPOINT05 (short selector, FilterRecord *filterParamBlock, intptr_t *data, short *result)
{
	fmc.entryPoint = 5;
	ENTRYPOINT (selector, filterParamBlock, data, result);
}
DllExport void ENTRYPOINT06 (short selector, FilterRecord *filterParamBlock, intptr_t *data, short *result)
{
	fmc.entryPoint = 6;
	ENTRYPOINT (selector, filterParamBlock, data, result);
}
DllExport void ENTRYPOINT07 (short selector, FilterRecord *filterParamBlock, intptr_t *data, short *result)
{
	fmc.entryPoint = 7;
	ENTRYPOINT (selector, filterParamBlock, data, result);
}
DllExport void ENTRYPOINT08 (short selector, FilterRecord *filterParamBlock, intptr_t *data, short *result)
{
	fmc.entryPoint = 8;
	ENTRYPOINT (selector, filterParamBlock, data, result);
}
DllExport void ENTRYPOINT09 (short selector, FilterRecord *filterParamBlock, intptr_t *data, short *result)
{
	fmc.entryPoint = 9;
	ENTRYPOINT (selector, filterParamBlock, data, result);
}
DllExport void ENTRYPOINT10 (short selector, FilterRecord *filterParamBlock, intptr_t *data, short *result)
{
	fmc.entryPoint = 10;
	ENTRYPOINT (selector, filterParamBlock, data, result);
}
DllExport void ENTRYPOINT11 (short selector, FilterRecord *filterParamBlock, intptr_t *data, short *result)
{
	fmc.entryPoint = 11;
	ENTRYPOINT (selector, filterParamBlock, data, result);
}
DllExport void ENTRYPOINT12 (short selector, FilterRecord *filterParamBlock, intptr_t *data, short *result)
{
	fmc.entryPoint = 12;
	ENTRYPOINT (selector, filterParamBlock, data, result);
}
DllExport void ENTRYPOINT13 (short selector, FilterRecord *filterParamBlock, intptr_t *data, short *result)
{
	fmc.entryPoint = 13;
	ENTRYPOINT (selector, filterParamBlock, data, result);
}
DllExport void ENTRYPOINT14 (short selector, FilterRecord *filterParamBlock, intptr_t *data, short *result)
{
	fmc.entryPoint = 14;
	ENTRYPOINT (selector, filterParamBlock, data, result);
}
DllExport void ENTRYPOINT15 (short selector, FilterRecord *filterParamBlock, intptr_t *data, short *result)
{
	fmc.entryPoint = 15;
	ENTRYPOINT (selector, filterParamBlock, data, result);
}
DllExport void ENTRYPOINT16 (short selector, FilterRecord *filterParamBlock, intptr_t *data, short *result)
{
	fmc.entryPoint = 16;
	ENTRYPOINT (selector, filterParamBlock, data, result);
}
DllExport void ENTRYPOINT17 (short selector, FilterRecord *filterParamBlock, intptr_t *data, short *result)
{
	fmc.entryPoint = 17;
	ENTRYPOINT (selector, filterParamBlock, data, result);
}
DllExport void ENTRYPOINT18 (short selector, FilterRecord *filterParamBlock, intptr_t *data, short *result)
{
	fmc.entryPoint = 18;
	ENTRYPOINT (selector, filterParamBlock, data, result);
}
DllExport void ENTRYPOINT19 (short selector, FilterRecord *filterParamBlock, intptr_t *data, short *result)
{
	fmc.entryPoint = 19;
	ENTRYPOINT (selector, filterParamBlock, data, result);
}
DllExport void ENTRYPOINT20 (short selector, FilterRecord *filterParamBlock, intptr_t *data, short *result)
{
	fmc.entryPoint = 20;
	ENTRYPOINT (selector, filterParamBlock, data, result);
}
DllExport void ENTRYPOINT21 (short selector, FilterRecord *filterParamBlock, intptr_t *data, short *result)
{
	fmc.entryPoint = 21;
	ENTRYPOINT (selector, filterParamBlock, data, result);
}
DllExport void ENTRYPOINT22 (short selector, FilterRecord *filterParamBlock, intptr_t *data, short *result)
{
	fmc.entryPoint = 22;
	ENTRYPOINT (selector, filterParamBlock, data, result);
}
DllExport void ENTRYPOINT23 (short selector, FilterRecord *filterParamBlock, intptr_t *data, short *result)
{
	fmc.entryPoint = 23;
	ENTRYPOINT (selector, filterParamBlock, data, result);
}
DllExport void ENTRYPOINT24 (short selector, FilterRecord *filterParamBlock, intptr_t *data, short *result)
{
	fmc.entryPoint = 24;
	ENTRYPOINT (selector, filterParamBlock, data, result);
}
DllExport void ENTRYPOINT25 (short selector, FilterRecord *filterParamBlock, intptr_t *data, short *result)
{
	fmc.entryPoint = 25;
	ENTRYPOINT (selector, filterParamBlock, data, result);
}
DllExport void ENTRYPOINT26 (short selector, FilterRecord *filterParamBlock, intptr_t *data, short *result)
{
	fmc.entryPoint = 26;
	ENTRYPOINT (selector, filterParamBlock, data, result);
}
DllExport void ENTRYPOINT27 (short selector, FilterRecord *filterParamBlock, intptr_t *data, short *result)
{
	fmc.entryPoint = 27;
	ENTRYPOINT (selector, filterParamBlock, data, result);
}
DllExport void ENTRYPOINT28 (short selector, FilterRecord *filterParamBlock, intptr_t *data, short *result)
{
	fmc.entryPoint = 28;
	ENTRYPOINT (selector, filterParamBlock, data, result);
}
DllExport void ENTRYPOINT29 (short selector, FilterRecord *filterParamBlock, intptr_t *data, short *result)
{
	fmc.entryPoint = 29;
	ENTRYPOINT (selector, filterParamBlock, data, result);
}
DllExport void ENTRYPOINT30 (short selector, FilterRecord *filterParamBlock, intptr_t *data, short *result)
{
	fmc.entryPoint = 30;
	ENTRYPOINT (selector, filterParamBlock, data, result);
}
DllExport void ENTRYPOINT31 (short selector, FilterRecord *filterParamBlock, intptr_t *data, short *result)
{
	fmc.entryPoint = 31;
	ENTRYPOINT (selector, filterParamBlock, data, result);
}
*/


/*****************************************************************************/

void InitGlobals (GPtr globals)
    {
    
    /* None of the globals requires initialization. */
    
}

//-------------------------------------------------------------------------------
//
//  ValidateParameters
//
//  Initialize parameters to default values.
//
//  Inputs:
//      GPtr globals        Pointer to global structure.
//
//  Outputs:
//
//-------------------------------------------------------------------------------

void ValidateParameters (GPtr globals)
{
    int i;

    // Should add a version # to gParams, and ignore old params if version #
    // mismatch!!!

    if (!gStuff->parameters)
    {

        gStuff->parameters = NewHandle ((long) sizeof (TParameters));

        if (!gStuff->parameters)
        {
            gResult = memFullErr;
            return;
        }


#if 0
        // Note that NewHandle zeroes the allocated memory.  Otherwise,         /// DOES IT ZERO????? yes, sets GMEM_ZEROINIT
        // we should zero out the gParams here to make sure the ptrs in
        // the CB code buffer structures are set to NULL, to avoid
        // problems when calling X86_emit_free_resources() on an
        // otherwise uninitialized CB.
        memset(gParams, 0, sizeof(*gParams));
#endif

        // Better query for parameters, since they didn't exist!!
        gParams->queryForParameters = TRUE;

        gParams->version_major = 1;
        gParams->version_minor = 0;

        gParams->flags     = AFH_USE_ADVANCESTATE|AFH_FLUSH_ADVANCESTATE|
                             AFH_INIT_OPTIMAL_ZOOM|AFH_OPTIMIZE_BEVEL|
                             AFH_ZERO_OUTPUT_DRAG| 
                             AFH_USE_INLINES|
                             AFH_SUPPORT_RGB; //|AFH_QUICK_TOGGLE ; /*|AFH_SUPPORT_GRAY| AFH_PROXY_BIG_GULP; */ 

        gParams->tileHeight = 100; //gParams->bandWidth = 100;    // Slow FM down for now...
        gParams->tileWidth = 0;

        for (i = 0;  i < N_CTLS;  i++)
        {   //default control settings
            //gParams->ctl[i].id = 0;
            gParams->ctl[i].initVal = 0;
            gParams->ctl[i].val = 0;
            gParams->ctl[i].defval = 0xffffffff;	//or -1 for X64???
            gParams->ctl[i].minval = 0;
            gParams->ctl[i].maxval = 255;
            gParams->ctl[i].lineSize = 1;
            gParams->ctl[i].pageSize = 10;
			gParams->ctl[i].thumbSize = 0;
            gParams->ctl[i].properties = 0;
            gParams->ctl[i].action = CA_NONE;
            gParams->ctl[i].label[0] = '\0';
            gParams->ctl[i].enabled = 3; //enabled and visible
            gParams->ctl[i].inuse = FALSE;
            gParams->ctl[i].tooltip[0] = '\0';
            gParams->ctl[i].image[0] = '\0';
            gParams->ctl[i].imageType = 0;
            
            gParams->ctl[i].editwidth = 24;
            gParams->ctl[i].editheight = 10;
			gParams->ctl[i].editXpos = 90; // Added by Ognen Genchev
			gParams->ctl[i].editYpos = 0; // Added by Ognen Genchev

			gParams->ctl[i].tabControl = -1;
            gParams->ctl[i].tabSheet = -1;
            gParams->ctl[i].mateControl = -1;
			gParams->ctl[i].anchor = 0;
			gParams->ctl[i].distWidth = 0;
			gParams->ctl[i].distHeight = 0;
			
            gParams->ctl[i].divisor = 1;
            gParams->ctl[i].gamma = 100;
			gParams->ctl[i].theme = -1;

            gParams->ctl[i].scripted = -1;
			gParams->ctl[i].data = -1;
        }

		//Preview is always used
        gParams->ctl[CTL_PREVIEW].inuse = TRUE;
        gParams->ctl[CTL_PREVIEW].properties = CTP_MOUSEMOVE | CTP_MOUSEOVER | CTP_PREVIEWDRAG; //Activate the preview events by default for now until the problem with Photoshop is solved
		gParams->ctl[CTL_PREVIEW].bkColor = -1; //No preview color
		gParams->ctl[CTL_PREVIEW].xPos = -1;
		gParams->ctl[CTL_PREVIEW].yPos = -1;
		gParams->ctl[CTL_PREVIEW].width = -1;
		gParams->ctl[CTL_PREVIEW].height = -1;
		gParams->ctl[CTL_PREVIEW].enabled = 3;
		
		gParams->ctl[CTL_FRAME].inuse = TRUE;
		gParams->ctl[CTL_FRAME].xPos = -1;
		gParams->ctl[CTL_FRAME].yPos = -1;
		gParams->ctl[CTL_FRAME].width = -1;
		gParams->ctl[CTL_FRAME].height = -1;
		gParams->ctl[CTL_FRAME].enabled = 3;
		
		gParams->ctl[CTL_PROGRESS].inuse = TRUE;
		gParams->ctl[CTL_PROGRESS].xPos = -1;
		gParams->ctl[CTL_PROGRESS].yPos = -1;
		gParams->ctl[CTL_PROGRESS].width = -1;
		gParams->ctl[CTL_PROGRESS].height = -1;
		gParams->ctl[CTL_PROGRESS].enabled = 3;
		gParams->ctl[CTL_PROGRESS].ctlclass = CC_PROGRESSBAR;
		gParams->ctl[CTL_PROGRESS].style = 0;
		gParams->ctl[CTL_PROGRESS].styleEx = 0;
		
		gParams->ctl[CTL_ZOOM].inuse = TRUE;
		gParams->ctl[CTL_ZOOM].xPos = -1;
		gParams->ctl[CTL_ZOOM].yPos = -1;
		gParams->ctl[CTL_ZOOM].width = -1;
		gParams->ctl[CTL_ZOOM].height = -1;
		gParams->ctl[CTL_ZOOM].enabled = 3;

	
        //Default dialog settings
		gParams->skinActive = 0;
		gParams->skinStyle = -1;
        gParams->gInitialInvocation = TRUE;
        gParams->gDialogRegion = NULL;
#if 0
        gParams->gDialogGradientColor1 = RGB(0,0,0);        //black
        gParams->gDialogGradientColor2 = RGB(0,0,0);        //black
#else
        //Changed by Harald Heim, March 9, 2003
        gParams->gDialogGradientColor1 = GetSysColor(COLOR_BTNFACE);//RGB(0xD4,0xD0,0xC8);   //yellow grey
        gParams->gDialogGradientColor2 = GetSysColor(COLOR_BTNFACE);//RGB(0xD4,0xD0,0xC8);   //yellow grey
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

		//gParams->gDialogEvent = 0; //Default value: no event


        //initialize registry access...
        //setRegRoot(HKEY_CURRENT_USER);
        //setRegPath("Software\\!O\\!C\\!t");

        // Initialize built-in strings...
        gParams->filterAuthorText[0] = 0;
        gParams->filterCategoryText[0] = 0;
        gParams->filterTitleText[0] = 0;
        gParams->filterFilenameText[0] = 0;
        gParams->filterCopyrightText[0] = 0;
        gParams->filterDescriptionText[0] = 0;
        gParams->filterOrganizationText[0] = 0;
        gParams->filterURLText[0] = 0;
        gParams->filterVersionText[0] = 0;
        gParams->filterAboutText[0] = 0;    //temporary
        gParams->filterUniqueID[0] = 0;

        //gParams->renderTime = 0;

    } //!gStuff->parameters
} // end ValidateParameters


/*****************************************************************************/

/* Asks the user for the plug-in filter module's parameters. Note that
   the image size information is not yet defined at this point. Also, do
   not assume that the calling program will call this routine every time the
   filter is run (it may save the data held by the parameters handle in
   a macro file). */

void DoParameters (GPtr globals)
{
    ValidateParameters (globals);

    if (gResult != noErr) return;

    gParams->queryForParameters = TRUE; //redundant!!!

} /*DoParameters*/

/*****************************************************************************/
static int topPad, bottomPad, leftPad, rightPad;	//THREAD-SAFE??
#if BIGDOCUMENT
	static VRect *mskRectPtr;
#else
    static Rect *mskRectPtr;
#endif

void addPadding (GPtr globals)
{
#if 0   //OBSOLETE???
#ifdef STATIC_FILTER
    int padding = 20000;  // try to grab the entire rect for now
#else
    int padding = 103;  // upper bound for now... (100 pixel bevel + 3 pixel radius max blur)
#endif
#endif

#if BIGDOCUMENT	 
	VRect scaledFilterRect;
#else
    Rect scaledFilterRect;
#endif
    long  scaleFactor;
    int padding;

    if (fmc.isTileable)
        padding = fmc.needPadding;//0;        // for now, assume no padding needed
    else
        padding = 20000;    // try to grab the entire rect

#if BIGDOCUMENT
	// Grab maskRect and inRect from host (necessary??)
	fmc.maskRect32 = GetMaskRect();
	fmc.inRect32 = GetInRect();

  #ifndef DONT_USE_MASK
    if (fmc.maskRect32.right > fmc.maskRect32.left)
        {   // mask is nonempty
        mskRectPtr = &fmc.maskRect32;
        }
        else
  #endif
        {   // using alpha (layer transparency mask) as mask
        mskRectPtr = &fmc.inRect32;
        }
#else
  #ifndef DONT_USE_MASK
        if (gStuff->maskRect.right > gStuff->maskRect.left)
        {   // mask is nonempty
            mskRectPtr = &gStuff->maskRect;
        }
        else
  #endif
        {   // using alpha (layer transparency mask) as mask
            mskRectPtr = &gStuff->inRect;
        }
    //}
#endif


    if (gParams->flags & AFH_USE_HOST_PADDING)
    {
        topPad = bottomPad = leftPad = rightPad = padding;
    }
    else
    {
		// Get filterRect from host and scale it down
        scaleFactor = gStuff->inputRate >> 16;      

#if BIGDOCUMENT
		scaledFilterRect = GetFilterRect();
		scaleRect32 (&scaledFilterRect, 1, scaleFactor);
#else
        copyRect (&scaledFilterRect, &gStuff->filterRect);
        scaleRect (&scaledFilterRect, 1, scaleFactor);
#endif
		// Calculate padding at each edge
        topPad    = min(padding, mskRectPtr->top - scaledFilterRect.top);
        bottomPad = min(padding, scaledFilterRect.bottom - mskRectPtr->bottom);
        leftPad   = min(padding, mskRectPtr->left - scaledFilterRect.left);
        rightPad  = min(padding, scaledFilterRect.right - mskRectPtr->right);
    }
	// Add padding to either fmc.maskRect32 or fmc.inRect32
    mskRectPtr->top    -= topPad;
    mskRectPtr->bottom += bottomPad;
    mskRectPtr->left   -= leftPad;
    mskRectPtr->right  += rightPad;

#ifndef DONT_USE_MASK
#if BIGDOCUMENT  
    if (fmc.maskRect32.right > fmc.maskRect32.left)
    {   
		// Update maskRect for host
		SetMaskRect(fmc.maskRect32);
		// Mask was nonempty, so adjust input rectangle also
        fmc.inRect32.top    -= topPad;
        fmc.inRect32.bottom += bottomPad;
        fmc.inRect32.left   -= leftPad;
        fmc.inRect32.right  += rightPad;
    }
#else
    if (gStuff->maskRect.right > gStuff->maskRect.left)
    {   // mask is nonempty, so adjust input rectangle also
        gStuff->inRect.top    -= topPad;
        gStuff->inRect.bottom += bottomPad;
        gStuff->inRect.left   -= leftPad;
        gStuff->inRect.right  += rightPad;
    }
#endif
#endif	 

#if BIGDOCUMENT
	// Update inRect for host
	SetInRect(fmc.inRect32);
#endif

} /*addPadding*/

void subtractPadding (GPtr globals)
{
	// Remove padding from either fmc.maskRect32 or fmc.inRect32
    mskRectPtr->top    += topPad;
    mskRectPtr->bottom -= bottomPad;
    mskRectPtr->left   += leftPad;
    mskRectPtr->right  -= rightPad;

#ifndef DONT_USE_MASK
#if BIGDOCUMENT
	if (fmc.maskRect32.right > fmc.maskRect32.left)
#else
    if (gStuff->maskRect.right > gStuff->maskRect.left)
#endif
    {   	
#if BIGDOCUMENT
        // Update maskRect for host
		SetMaskRect(fmc.maskRect32);	 
		// Mask was nonempty, so adjust input rectangle also
        fmc.inRect32.top    += topPad;
        fmc.inRect32.bottom -= bottomPad;
        fmc.inRect32.left   += leftPad;
        fmc.inRect32.right  -= rightPad;   
#else	
        // mask is nonempty...
        gStuff->inRect.top    += topPad;
        gStuff->inRect.bottom -= bottomPad;
        gStuff->inRect.left   += leftPad;
        gStuff->inRect.right  -= rightPad;
#endif
        // PSP 4.12 treats maskData as read-only, so we need to make
        // our own (adjusted) copy.
        MyAdjustedMaskData = (unsigned8 *) gStuff->maskData + (topPad*gStuff->maskRowBytes + leftPad);
                                                            // note: maskColumnBytes=1
    }
#endif

		//Some hosts (e.g., PSP X3) set inColumnBytes incorrectly, so we set it correctly here
	    if (gStuff->imageMode > 9) //16-bit image data
			gStuff->inColumnBytes = 2 * ((gStuff->inHiPlane - gStuff->inLoPlane) + 1);	
		else
			gStuff->inColumnBytes = (gStuff->inHiPlane - gStuff->inLoPlane) + 1;
		
		// PSP 4.12 treats inData as read-only, so we need to make
        // our own (adjusted) copy.
		MyAdjustedInData = (unsigned8 *) gStuff->inData + (topPad*gStuff->inRowBytes + leftPad*gStuff->inColumnBytes);
#if BIGDOCUMENT
	// Update inRect for host
	SetInRect(fmc.inRect32);
#endif

} /*subtractPadding*/

/*****************************************************************************/
//
//  Stuff used to select "desirable" Proxy Preview scale factors...
//
/// 100%, 50%, 33.3%, 25%, 20%, 16.7%, 14.3%, 12.5%, 11.1%, 10%, 9.09%, 8.33%, (7.69%),
/// 7.14%, (6.67%), 6.25%, (5.88%), (5.55%), (5.26%), 5%, (4.76%),...,4%,...,3.33%,...,2.5%,
/// ...,2%,...,1.49%,...,1.33%,...,1%,...0.50%,...,0.33%,...,0.25%,...0.20%,...,0.17%,
/// ...,0.14%,...,0.12%,...,0.11%,...,0.10%,...0.05%,...0.03%,...0.02%,...,0.01%,...,
/// .005%, .002%, .001%, .0005%, .0002%, .0001%, .00005%, .00002%, .00001% <== brings 2G down to 200

#if 0
static int desirableScaleFactor[] = {
	1,		// 100%
	2,		// 50%
	3,		// 33.3%
	4,		// 25%
	5,		// 20%
	6,		// 16.7%
	7,		// 14.3%
	8,		// 12.5%
	9,		// 11.1%
	10,		// 10%
	11,		// 9.09%
	12,		// 8.33%
///	13,		//			7.69%	*skip*
	14,		// 7.14%
///	15,		//			6.67%	*skip*
	16,		// 6.25%
///	17,		//			5.88%	*skip*
///	18,		//			5.55%	*skip*
///	19,		//			5.26%	*skip*
	20,		// 5%
	25,		// 4%
	30,		// 3.33%
///	33,		//			3.03%	*skip*
	40,		// 2.5%
	50,		// 2%
	57,		// 1.75%
///	60,		//			1.67%	*skip*
	67,		// 1.49%
///	70,		//			1.43%	*skip*
	75,		// 1.33%
	80,		// 1.25%
///	90,		//			1.11%	*skip*
	91,		// 1.1%
	100,	// 1%
///	111,	//			0.901%	*skip*
	125,	// 0.8%
///	133,	//			0.752	*skip*
///	143,	//			0.699%	*skip*
///	167,	//			0.599	*skip*
	200,	// 0.5%
	250,	// 0.4%
///	300,	//			0.333%	*skip*
	333,	// 0.3%
	400,	// 0.25%
	500,	// 0.2%
///	600,	//			0.167%	*skip*
	625,	// 0.16%
	667,	// 0.15%
///	700,	//			0.143%	*skip*
	800,	// 0.125%
///	900,	//			0.111%	*skip*
	909,	// 0.11%
	1000,	// 0.1%
	1111,	// 0.09%
	1250,	// 0.08%
///	1333,	//			0.075	*skip*
	1429,	// 0.07%
	1667,	// 0.06%
	2000,	// 0.05%
	2500,	// 0.04%
///	3000,	//			0.0333%	*skip*
	3333,	// 0.03%
	5000,	// 0.02%
	10000,	// 0.01%
	11111,	// 0.009
	12500,	// 0.008%
///	13333,	//			0.0075%	*skip*
	14286,	// 0.007%
	16667,	// 0.006%
	20000,	// 0.005%
	25000,	// 0.004%
	33333,	// 0.003%
	50000,	// 0.002%
	100000,	// 0.001%
	111111,	// 0.0009%
	125000,	// 0.0008%
///	133333,	//			0.00075%	*skip*
	142857,	// 0.0007%
	166667, // 0.0006%
	200000,	// 0.0005%
	250000,	// 0.0004%
	333333,	// 0.0003%
	500000,	// 0.0002%
	1000000,// 0.0001%
	1111111,// 0.00009%
	1250000,// 0.00008%
///	1333333,//			0.000075%	*skip*
	1428571,// 0.00007%
	1666667,// 0.00006%
	2000000,// 0.00005%
	2500000,// 0.00004%
	3333333,// 0.00003%
	5000000,// 0.00002%
	10000000,// 0.00001%	//scales 2Gpx to 200px, so probably max reduction ever needed!
	INT_MAX	// ~infinity (stopper)
}; /**desirableScaleFactor*/
#else
//Using a roughly logarithmic series, with a step of 10**0.1 ~= 1.25 (1/0.8),
//which descends in one decade something like:
//100, 80, 64, 50, 40, 32, 25, 20, 16, 12.5, 10
static int desirableScaleFactor[] = {
	1,		// 100%
	//		// 80%		//no can do
	//		// 64%		//no can do
	2,		// 50%
	//		// 40%		//no can do
	3,		// 33.3%
	4,		// 25%
	5,		// 20%
	6,		// 16.7%
	8,		// 12.5%

	10,		// 10%
	12,		// 8.33%
	16,		// 6.25%
	20,		// 5%
	25,		// 4%
	30,		// 3.33%	//or 33 //3.03%??
	40,		// 2.5%
	50,		// 2%
	60,		// 1.67%
	80,		// 1.25%

	100,	// 1%
	125,	// 0.8%
	160,	// 0.625%	// or 156,	// 0.641%
	200,	// 0.5%
	250,	// 0.4%
	300,	// 0.333%
	400,	// 0.25%
	500,	// 0.2%
	625,	// 0.16%
	800,	// 0.125%

	1000,	// 0.1%
	1250,	// 0.08%
	1562,	// 0.064%
	2000,	// 0.05%
	2500,	// 0.04%
	3000,	// 0.0333%	//or 3125, // 0.032
	4000,	// 0.025%
	5000,	// 0.02%
	6250,	// 0.016%
	8000,	// 0.0125%

	10000,	// 0.01%
	12500,	// 0.008%
	15625,	// 0.0064%
	20000,	// 0.005%
	25000,	// 0.004%
	30000,	// 0.00333%
#if 0	///////////////// OOPS. For now scaleFactor can't exceed 30,000 ( or 32767).
		/////////////////       This limit is imposed because inputRate and MaskRate
		/////////////////		Are fixed point 16.16 numbers, and scaleFactor must
		/////////////////		fit in the high (signed) 16 bits!  This is a PS limitation,
		/////////////////		even in CS5.1 X64.  So if scaleFactor is <= 30000 and
		/////////////////		we want to scale an image to fit entirely within a 300 x 300 px
		/////////////////		frame, then no image dimension should exceeed 300px * 30000 =
		/////////////////		9,000,000 px, or approxmately 10Mpx.  Of course, this is well
		/////////////////		within the (published) CS5 limit of 300,000 x 300,000 px.
	40000,	// 0.0025%
	50000,	// 0.002%
	62500,	// 0.0016%
	80000,	// 0.00125%

	100000,	// 0.001%
	125000,	// 0.0008%
	156250,	// 0.00064%
	200000,	// 0.0005%
	250000,	// 0.0004%
	300000,	// 0.000333%
	400000,	// 0.00025%
	500000,	// 0.0002%
	625000,	// 0.00016%
	800000,	// 0.00125%

	1000000,// 0.0001%
	1250000,// 0.00008%
	1562500,// 0.000064%
	2000000,// 0.00005%
	2500000,// 0.00004%
	3000000,// 0.0000333%
	4000000,// 0.000025%
	5000000,// 0.00002%
	6250000,// 0.000016%
	8000000,// 0.000125%
	10000000,// 0.00001%	//scales 2Gpx to 200px, so probably max reduction ever needed!
#endif
	INT_MAX	// ~infinity (stopper)
}; /**desirableScaleFactor*/
#endif

/* Round up to a desirable scale factor */
int roundUpScaleFactor(int sf) {
	int i;
	for (i = 0; sf > desirableScaleFactor[i]; ++i) {
	}
	assert (sf <= desirableScaleFactor[i]);
	return desirableScaleFactor[i];
} /*roundUpScaleFactor*/

/* Round down to a desirable scale factor */
int roundDownScaleFactor(int sf) {
	int i;
	for (i = 0; sf > desirableScaleFactor[i]; ++i) {
	}
	assert (sf <= desirableScaleFactor[i]);
	if (sf < desirableScaleFactor[i])
		return desirableScaleFactor[i-1];
	return desirableScaleFactor[i];
} /*roundDownScaleFactor*/

#if 0
/* Round down to a desirable scale factor */
int roundDownScaleFactor2(int sf)
{
	int i = sizeof(desirableScaleFactor)/sizeof(desirableScaleFactor[0]) - 1;
	for ( ; i >= 0; --i) {
		if (sf >= desirableScaleFactor[i])
			return desirableScaleFactor[i];
	}
	return desirableScaleFactor[i];
} /*roundDownScaleFactor2*/
#endif

/*****************************************************************************/

void SetupFilterRecordForProxy (GPtr globals, long *scaleFactor, long prevScaleFactor,
                                POINT *offset)
{
    long    maxScaleFactor;
#if BIGDOCUMENT					 
	int     proxW, proxH, inW, inH;
    VRect   scaledImage;
    VRect   savedImage, savedOut; 
    #ifndef DONT_USE_MASK
       VRect   savedMask;
    #endif
#else
    short   proxW, proxH, inW, inH;
    Rect    scaledImage;
    Rect    savedImage, savedOut;
#ifndef DONT_USE_MASK
    Rect    savedMask;
#endif
#endif
    int     oops;
    static long oldScaleFactor = 9999;
    static BOOL scaleFactorChanged = FALSE;
    BOOL    proxyWasTrimmed;
	
    fmc.doingProxy = TRUE;

#if BIGDOCUMENT		
    fmc.filterRect32 = GetFilterRect();
	CalcProxyScaleFactor (&fmc.filterRect32, &gProxyRect, &maxScaleFactor);
#else
    CalcProxyScaleFactor (&gStuff->filterRect, &gProxyRect, &maxScaleFactor);
#endif

    if (*scaleFactor == -888)
	{   // -888 means use calculated optimal scale factor...
        *scaleFactor = maxScaleFactor;
    }

#ifdef LIMIT_MAX_SCALEFACTOR
    // Optionally limit scalefactor to <= maxScaleFactor.
    if (*scaleFactor > maxScaleFactor) *scaleFactor = maxScaleFactor;
#endif

    // Make sure scale factor is in range [1,MAX_SCALE_FACTOR].
    if (*scaleFactor < 1) *scaleFactor = 1;
    if (*scaleFactor > MAX_SCALE_FACTOR) *scaleFactor = MAX_SCALE_FACTOR;  //Allow -888 to work fine for 30k+ images

#if LOWZOOM
	*scaleFactor = roundUpScaleFactor(*scaleFactor);
#else
    // Skip scalefactor 13 or 15...
    if (*scaleFactor == 13 || *scaleFactor == 15)
        (*scaleFactor)++;  //use next higher scaleFactor (14 or 16)
	//Info ("%d",*scaleFactor);
	//*scaleFactor = roundUpScaleFactor(*scaleFactor);
	//Info ("%d",*scaleFactor);
#endif
    

    if (*scaleFactor != oldScaleFactor)
    {
        scaleFactorChanged = TRUE;
        oldScaleFactor = *scaleFactor;
        fmc.zoomFactor = *scaleFactor;
    }

#if BIGDOCUMENT
	fmc.inRect32 = fmc.filterRect32;
	scaleRect32(&fmc.inRect32, 1, *scaleFactor);
	scaledImage = fmc.inRect32;
#else
    copyRect (&gStuff->inRect, &gStuff->filterRect);
    scaleRect (&gStuff->inRect, 1, *scaleFactor);
    copyRect (&scaledImage, &gStuff->inRect);
#endif

    // Trim (scaled) inRect to fit within proxyRect if necessary...
    proxyWasTrimmed = FALSE;
    proxW = gProxyRect.right - gProxyRect.left;
    proxH = gProxyRect.bottom - gProxyRect.top;

	//Scale down the processed image area to make the whole image scrollable at >100% zoom
	proxW /= fmc.enlargeFactor;
	proxH /= fmc.enlargeFactor;

#if BIGDOCUMENT
    inW = fmc.inRect32.right - fmc.inRect32.left;
    inH = fmc.inRect32.bottom - fmc.inRect32.top;
    if (inW > proxW) {
        fmc.inRect32.left += (inW - proxW)/2;
        fmc.inRect32.right = fmc.inRect32.left + proxW;
        proxyWasTrimmed = TRUE;
    }
    if (inH > proxH) {
        fmc.inRect32.top += (inH - proxH)/2;
        fmc.inRect32.bottom = fmc.inRect32.top + proxH;
        proxyWasTrimmed = TRUE;
    }
#else
    inW = gStuff->inRect.right - gStuff->inRect.left;
    inH = gStuff->inRect.bottom - gStuff->inRect.top;
    if (inW > proxW) {
        gStuff->inRect.left += (inW - proxW)/2;
        gStuff->inRect.right = gStuff->inRect.left + proxW;
        proxyWasTrimmed = TRUE;
    }
    if (inH > proxH) {
        gStuff->inRect.top += (inH - proxH)/2;
        gStuff->inRect.bottom = gStuff->inRect.top + proxH;
        proxyWasTrimmed = TRUE;
    }
#endif

    // Apply (scaled) offset...
/*#ifdef HIGHZOOM
    //This code causes image cut off in the preview
#if BIGDOCUMENT
    fmc.inRect32.left   += (offset->x / (*scaleFactor) / fmc.enlargeFactor);
    fmc.inRect32.right  += (offset->x / (*scaleFactor) / fmc.enlargeFactor);
    fmc.inRect32.top    += (offset->y / (*scaleFactor) / fmc.enlargeFactor);
    fmc.inRect32.bottom += (offset->y / (*scaleFactor) / fmc.enlargeFactor);
#else
    gStuff->inRect.left   += (short) (offset->x / (*scaleFactor) / fmc.enlargeFactor);
    gStuff->inRect.right  += (short) (offset->x / (*scaleFactor) / fmc.enlargeFactor);
    gStuff->inRect.top    += (short) (offset->y / (*scaleFactor) / fmc.enlargeFactor);
    gStuff->inRect.bottom += (short) (offset->y / (*scaleFactor) / fmc.enlargeFactor);
#endif
#else */

#if BIGDOCUMENT
	fmc.inRect32.left   += (offset->x / *scaleFactor);
    fmc.inRect32.right  += (offset->x / *scaleFactor);
    fmc.inRect32.top    += (offset->y / *scaleFactor);
    fmc.inRect32.bottom += (offset->y / *scaleFactor);		   
#else
	gStuff->inRect.left   += (short) (offset->x / *scaleFactor);
    gStuff->inRect.right  += (short) (offset->x / *scaleFactor);
    gStuff->inRect.top    += (short) (offset->y / *scaleFactor);
    gStuff->inRect.bottom += (short) (offset->y / *scaleFactor);
#endif
//#endif

	//setCtlTextv(0, "%d, %d - %d, %d", gStuff->inRect.left,gStuff->inRect.right,gStuff->inRect.top,gStuff->inRect.bottom ); 

#if BIGDOCUMENT
    if (fmc.inRect32.left < scaledImage.left)
    {   // Oops.  Too far left...
        oops = scaledImage.left - fmc.inRect32.left;
        fmc.inRect32.left  += oops;
        fmc.inRect32.right += oops;
        offset->x += oops * (*scaleFactor);
    }
    else if (fmc.inRect32.right > scaledImage.right)
    {   // Oops.  Too far right...
        oops = fmc.inRect32.right - scaledImage.right;
        fmc.inRect32.left  -= oops;
        fmc.inRect32.right -= oops;
        offset->x -= oops * (*scaleFactor);
    }
    if (fmc.inRect32.top < scaledImage.top)
    {   // Oops.  Too far up...
        oops = scaledImage.top - fmc.inRect32.top;
        fmc.inRect32.top    += oops;
        fmc.inRect32.bottom += oops;
        offset->y += oops * (*scaleFactor);
    }
    else if (fmc.inRect32.bottom > scaledImage.bottom)
    {   // Oops.  Too far down...
        oops = fmc.inRect32.bottom - scaledImage.bottom;
        fmc.inRect32.top    -= oops;
        fmc.inRect32.bottom -= oops;
        offset->y -= oops * (*scaleFactor);
    }
	SetInRect(fmc.inRect32);

    if ((gParams->flags & (AFH_DRAGGING_PROXY|AFH_ZERO_OUTPUT_DRAG|AFH_DRAG_FILTERED_OUTPUT))
                       == (AFH_DRAGGING_PROXY|AFH_ZERO_OUTPUT_DRAG))
    {   // Don't need to set up output Rect.  Could save some time.
        PISetRect32(&fmc.outRect32, 0, 0, 0, 0);
    }
    else
    {
		fmc.outRect32 = fmc.inRect32;
    }
	SetOutRect(fmc.outRect32);
#else
    if (gStuff->inRect.left < scaledImage.left)
    {   // Oops.  Too far left...
        oops = scaledImage.left - gStuff->inRect.left;
        gStuff->inRect.left  += oops;
        gStuff->inRect.right += oops;
        offset->x += oops * (*scaleFactor);
    }
    else if (gStuff->inRect.right > scaledImage.right)
    {   // Oops.  Too far right...
        oops = gStuff->inRect.right - scaledImage.right;
        gStuff->inRect.left  -= oops;
        gStuff->inRect.right -= oops;
        offset->x -= oops * (*scaleFactor);
    }
    if (gStuff->inRect.top < scaledImage.top)
    {   // Oops.  Too far up...
        oops = scaledImage.top - gStuff->inRect.top;
        gStuff->inRect.top    += oops;
        gStuff->inRect.bottom += oops;
        offset->y += oops * (*scaleFactor);
    }
    else if (gStuff->inRect.bottom > scaledImage.bottom)
    {   // Oops.  Too far down...
        oops = gStuff->inRect.bottom - scaledImage.bottom;
        gStuff->inRect.top    -= oops;
        gStuff->inRect.bottom -= oops;
        offset->y -= oops * (*scaleFactor);
    }

    if ((gParams->flags & (AFH_DRAGGING_PROXY|AFH_ZERO_OUTPUT_DRAG|AFH_DRAG_FILTERED_OUTPUT))
                       == (AFH_DRAGGING_PROXY|AFH_ZERO_OUTPUT_DRAG))
    {   // Don't need to set up output Rect.  Could save some time.
        PISetRect (&gStuff->outRect, 0, 0, 0, 0);
    }
    else
    {
        copyRect (&gStuff->outRect, &gStuff->inRect);
    }
#endif	 

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

    gStuff->inputRate = gStuff->maskRate = long2fixed(*scaleFactor);
 
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



    // If the Big Gulp option is set, try to speed up proxy
    // dragging by requesting the entire (scaled) image whenever
    // the scale factor first changes (and the scaled image
    // needs to be trimmed to fit in the proxy window).
    if ((gParams->flags & AFH_PROXY_BIG_GULP) && scaleFactorChanged && proxyWasTrimmed)
    {
        short tempResult;

        scaleFactorChanged = FALSE;
        // Save the current inRect, outRect, and (maybe) maskRect,
        // and set them to the full (scaled) image rectangle.
#if BIGDOCUMENT
		savedImage = fmc.inRect32;
		fmc.inRect32 = scaledImage;
		SetInRect(fmc.inRect32);

		savedOut = fmc.outRect32;

        if ((gParams->flags & (AFH_DRAGGING_PROXY|AFH_ZERO_OUTPUT_DRAG|AFH_DRAG_FILTERED_OUTPUT))
                           == (AFH_DRAGGING_PROXY|AFH_ZERO_OUTPUT_DRAG))
            // Don't need to set up output Rect.  Could save some time.
            PISetRect32(&fmc.outRect32, 0, 0, 0, 0);
        else
			fmc.outRect32 = scaledImage;
		SetOutRect(fmc.outRect32);

    #ifndef DONT_USE_MASK
        if (gStuff->haveMask)
        {
			savedMask = fmc.maskRect32;
			fmc.maskRect32 = scaledImage;
			SetMaskRect(fmc.maskRect32);
        }
    #endif
#else
        copyRect (&savedImage, &gStuff->inRect);
        copyRect (&gStuff->inRect, &scaledImage);
        copyRect (&savedOut, &gStuff->outRect);

        if ((gParams->flags & (AFH_DRAGGING_PROXY|AFH_ZERO_OUTPUT_DRAG|AFH_DRAG_FILTERED_OUTPUT))
                           == (AFH_DRAGGING_PROXY|AFH_ZERO_OUTPUT_DRAG))
        {   // Don't need to set up output Rect.  Could save some time.
            PISetRect (&gStuff->outRect, 0, 0, 0, 0);
        }
        else
        {
            copyRect (&gStuff->outRect, &scaledImage);
        }

   #ifndef DONT_USE_MASK
        if (gStuff->haveMask)
        {
            copyRect (&savedMask, &gStuff->maskRect);
            copyRect (&gStuff->maskRect, &scaledImage);
        }
   #endif
#endif

        addPadding(globals);



        // Call AdvanceState() to try to gulp in the entire (scaled) image.
        // The result is ignored, because we don't care if it fails (due to
        // insufficient memory).
        tempResult = AdvanceState();


#if BIGDOCUMENT
        // Restore the current values of inRect, outRect, and maskRect.
		fmc.inRect32 = savedImage;
		SetInRect(fmc.inRect32);
		fmc.outRect32 = savedOut;
		SetOutRect(fmc.outRect32);
   #ifndef DONT_USE_MASK
        if (gStuff->haveMask)
        {
			fmc.maskRect32 = savedMask;
			SetMaskRect(fmc.maskRect32);
        }
   #endif
#else
        // Restore the current values of inRect, outRect, and maskRect.
        copyRect (&gStuff->inRect, &savedImage);
        copyRect (&gStuff->outRect, &savedOut);
   #ifndef DONT_USE_MASK
        if (gStuff->haveMask)
        
                copyRect (&gStuff->maskRect, &savedMask);
   #endif
#endif
    } /*if Big Gulp*/

    

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
                    "Not enough memory to generate a preview image at this zoom level."
                    );
        }
        else {
            msgBox( MB_ICONHAND|MB_SYSTEMMODAL,
                    fmc.filterTitleText,//"FilterMeister",
                    "Unknown error (%d) while trying to generate a preview image.",
                    gResult
                    );
        }
    }
    
    subtractPadding(globals);

    // Need to set outRect correctly for ShowOutput();
#if BIGDOCUMENT
    fmc.inRect32 = GetInRect();	//needed??
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

} /*SetupFilterRecordForProxy*/

/*****************************************************************************/

/* Prepare to filter an image.  If the plug-in filter needs a large amount
   of buffer memory, this routine should set the bufferSpace field to the
   number of bytes required. */

void DoPrepare (GPtr globals)
{

	int maxSpace = gStuff->maxSpace;

    ValidateParameters (globals);

    if (gResult != noErr) return;

#ifndef NOTRACE
    Trace (globals, "DoPrepare:\n"
        " gStuff->bufferSpace = %d\n"
        " (before setting it to 0)",
        gStuff->bufferSpace);
#endif
    
    
    //Added by Harald Heim, February 2003
    //Suppress dialog when there is no filter parameter block
    //For Plugin Commander Pro
    if (gStuff == NULL)
        gParams->queryForParameters = FALSE; 


/*
	InitContextRecord();
	if (!triggerEvent(0,FME_DOPREPARE,0)){
		//Restore values in case false was returned;
		gStuff->maxSpace = maxSpace;	
		gStuff->bufferSpace = 0;
	}
*/

    gStuff->bufferSpace = 0;


    
    //Added by Harald Heim, Sep 26, 2003
    //Make Photoshop free up all the memory ??? -> Doesn't work
    //gStuff->maxSpace = 0;



    //Added by Harald Heim, Jul 6, 2002
    //if (gStuff->isFloating==FALSE && gStuff->haveMask==TRUE) 
    //      gStuff->autoMask = FALSE;

    
}

/*****************************************************************************/

//do initialization at start of each filter run...

static void InitBeforeFilter(void)
{
    int i;

    //initialize random number generator for the proxy image
    srand(1);

    // Set gamma = 1.0...
    //fm_setGamma(1.0);

    //initialize anonymous cells
    if (Dont_Initialize_Cells == 0)
    {
        for (i = 0; i < N_CELLS; i++) {
            fmc.cell[i] = 0;
        }
    }

    // Cache current control values in fmc.pre_ctl[].
    for (i=0; i < N_CTLS; i++) {
        fmc.pre_ctl[i] = gParams->ctl[i].val;
    }

    //init fmc.scaleFactor, etc. ?????

    call_FFP(FFP_OnFilterStart,(INT_PTR)&fmc);//OnFilterStart(&fmc);


} /*InitBeforeFilter*/

/*****************************************************************************/

void DoStart (GPtr globals)
{

    //Added by Harald Heim
    int i;
	Boolean displayDialog = true;
	//OSErr err;


    ValidateParameters (globals);

    if (gResult != noErr) return;

#ifndef NOTRACE
    Trace (globals, "DoStart:\n"
        " gParams->queryForParameters = %#x",
        gParams->queryForParameters);
#endif
    
    // Initialize static parts of the FM context record.
    InitContextRecord();

    // Synchronize FM context flags to global (parameter) flags...
    fmc.isTileable = (gParams->flags & AFH_IS_TILEABLE) != 0;

    // Copy info from host's filter record...
    
    fmc.hostSerialNumber = gStuff->serialNumber;
    fmc.displayDialog = gParams->queryForParameters;
    fmc.doingScripting = 0;
	fmc.planes = gStuff->planes;

#if BIGDOCUMENT
	// Following routines check at runtime whether to use 16- or 32-bit host fields...
	fmc.imageSize32	= GetImageSize();
	fmc.filterRect32 = GetFilterRect();
	fmc.inRect32 = GetInRect();
	fmc.outRect32 = GetOutRect();
	fmc.maskRect32 = GetMaskRect();
	fmc.floatCoord32 = GetFloatCoord();
	fmc.wholeSize32 = GetWholeSize();	

    // Following two fields are only exposed in the FD's dictionary...
	fmc.filterRectWidth32   = fmc.filterRect32.right - fmc.filterRect32.left;
	fmc.filterRectHeight32  = fmc.filterRect32.bottom - fmc.filterRect32.top;

	//Fill anyway -> in case used by filter code
	fmc.imageHeight = fmc.imageSize32.v;
    fmc.imageWidth = fmc.imageSize32.h;
	fmc.filterRectWidth = fmc.filterRect32.right - fmc.filterRect32.left;
    fmc.filterRectHeight = gStuff->filterRect.bottom - gStuff->filterRect.top;
    fmc.filterRectLeft = fmc.filterRect32.left;
    fmc.filterRectTop = fmc.filterRect32.top;
    fmc.filterRectRight = fmc.filterRect32.right;
    fmc.filterRectBottom = fmc.filterRect32.bottom;
    fmc.wholeWidth = fmc.wholeSize32.h;
    fmc.wholeHeight = fmc.wholeSize32.v;
#else
    fmc.imageHeight = gStuff->imageSize.v;
    fmc.imageWidth = gStuff->imageSize.h;
    fmc.filterRectLeft = gStuff->filterRect.left;
    fmc.filterRectTop = gStuff->filterRect.top;
    fmc.filterRectRight = gStuff->filterRect.right;
    fmc.filterRectBottom = gStuff->filterRect.bottom;
    fmc.filterRectWidth = gStuff->filterRect.right - gStuff->filterRect.left;
    fmc.filterRectHeight = gStuff->filterRect.bottom - gStuff->filterRect.top;
    fmc.wholeWidth = gStuff->wholeSize.h;
    fmc.wholeHeight = gStuff->wholeSize.v;
#endif

    fmc.planes = gStuff->planes; 
    
    if (gStuff->imageMode == plugInModeCMYKColor || gStuff->imageMode == plugInModeCMYK64)
        fmc.planesWithoutAlpha = 4;
    else if (gStuff->imageMode == plugInModeGrayScale || gStuff->imageMode == plugInModeGray16 
        || gStuff->imageMode == plugInModeDuotone || gStuff->imageMode == plugInModeDuotone16
        || gStuff->imageMode == plugInModeBitmap || gStuff->imageMode == plugInModeIndexedColor)
        fmc.planesWithoutAlpha = 1;
    else /*if (gStuff->imageMode ==  plugInModeMultichannel || gStuff->imageMode == plugInModeDeepMultichannel
        || gStuff->imageMode == plugInModeRGBColor || gStuff->imageMode == plugInModeLabColor
        || gStuff->imageMode == plugInModeHSLColor || gStuff->imageMode == plugInModeHSBColor
        || gStuff->imageMode == plugInModeRGB48 || gStuff->imageMode == plugInModeLab48)*/
        fmc.planesWithoutAlpha = 3;
    

    fmc.maxSpace = gStuff->maxSpace;
	fmc.bufferSpace = gStuff->bufferSpace;
    fmc.isFloating = gStuff->isFloating;
    fmc.haveMask = gStuff->haveMask;

    //Added by Harald Heim, Jul 5, 2002
    fmc.autoMask = gStuff->autoMask;
    
    fmc.backColor = RGBA(gStuff->backColor[0],
                            gStuff->backColor[1],
                            gStuff->backColor[2],
                            gStuff->backColor[3]);
    fmc.foreColor = RGBA(gStuff->foreColor[0],
                            gStuff->foreColor[1],
                            gStuff->foreColor[2],
                            gStuff->foreColor[3]);
    // Need to mask out garbage in unused bytes of back/foreColor...
    {   int mask = 0xffffffff;
        switch (gStuff->imageMode)
        {
        case plugInModeBitmap:
        case plugInModeGrayScale:
        case plugInModeIndexedColor:    //can this happen???
        case plugInModeGray16:          //correct???
            mask = 0x000000ff;
            break;

        case plugInModeDuotone:
        case plugInModeDuotone16:

            //mask = 0x0000ffff;          //is duotone always 2 channels???
            mask = 0x00ffffff;          //duotone can have RGB fore and background colors!
            break;

        case plugInModeRGBColor:
        case plugInModeLabColor:
        case plugInModeHSLColor:        //can this happen???
        case plugInModeHSBColor:        //can this happen???
        case plugInModeRGB48:           //correct???      
        case plugInModeLab48:   

            mask = 0x00ffffff;
            break;

        case plugInModeMultichannel:     
        case plugInModeDeepMultichannel:
            
            mask = 0x00ffffff;
            break;

        case plugInModeCMYKColor:
        case plugInModeCMYK64:

        default:
            mask = 0xffffffff;
            break;
        } /*switch*/
        fmc.backColor &= mask;
        fmc.foreColor &= mask;
    } //scope

    //scale 16-bit back/foreground values down to 8 bits...     //Is this correct ???
    fmc.bgColor = fm_RGB(gStuff->background.red >> 8,
                         gStuff->background.green >> 8,
                         gStuff->background.blue >> 8);
    fmc.fgColor = RGB(gStuff->foreground.red >> 8,
                         gStuff->foreground.green >> 8,
                         gStuff->foreground.blue >> 8);
    fmc.hostSig = gStuff->hostSig;

    
    
    fmc.platformData = (intptr_t) gStuff->platformData;
    fmc.imageHRes = (gStuff->imageHRes >> 16) + (gStuff->imageHRes & 0xffff) / 65536.0; //(int) gStuff->imageHRes; //>> 16
    fmc.imageVRes = (gStuff->imageVRes >> 16) + (gStuff->imageVRes & 0xffff) / 65536.0; //(int) gStuff->imageVRes; //>> 16


    fmc.imageMode = gStuff->imageMode;

/*
#if BIGDOCUMENT
#else
     fmc.wholeWidth = gStuff->wholeSize.h;
     fmc.wholeHeight = gStuff->wholeSize.v;
#endif
*/

    /*if (gStuff->filterCase == 0){
		if (gStuff->haveMask==0 && gStuff->isFloating==0) gStuff->filterCase = 1;
		if (gStuff->haveMask==1 && gStuff->isFloating==0) gStuff->filterCase = 2;
		if (gStuff->isFloating==1) gStuff->filterCase = 3;
	}*/
	fmc.filterCase = gStuff->filterCase;
	fmc.samplingSupport = gStuff->samplingSupport;
    fmc.zoomFactor = 0;     //not yet set
	fmc.enlargeFactor = 1;
	fmc.scrollFactor = 1;

    // Restore values in FM context record across invocations...
    fmc.need_iuv = gParams->need_iuv;
    fmc.need_d = gParams->need_d;
    fmc.need_m = gParams->need_m;
    fmc.need_tbuf = gParams->need_tbuf;
    fmc.need_t2buf = gParams->need_t2buf;
	fmc.need_t3buf = gParams->need_t3buf;
	fmc.need_t4buf = gParams->need_t4buf;
    fmc.needPremiere = gParams->needPremiere;
    fmc.needPadding = gParams->needPadding;
    fmc.tileHeight = gParams->tileHeight; //fmc.bandWidth = gParams->bandWidth;
    fmc.tileWidth = gParams->tileWidth;
#ifndef DONT_SAVE_GLOBALS
    /* Restore the following predeclared global and user-declared global/static variables */
    fmc.globvars = gParams->globvars;
#endif //DONT_SAVE_GLOBALS
    memcpy(fmc.filterAuthorText, gParams->filterAuthorText, sizeof(fmc.filterAuthorText));
    memcpy(fmc.filterCategoryText, gParams->filterCategoryText, sizeof(fmc.filterCategoryText));
    memcpy(fmc.filterTitleText, gParams->filterTitleText, sizeof(fmc.filterTitleText));
    memcpy(fmc.filterFilenameText, gParams->filterFilenameText, sizeof(fmc.filterFilenameText));
    memcpy(fmc.filterCopyrightText, gParams->filterCopyrightText, sizeof(fmc.filterCopyrightText));
    memcpy(fmc.filterDescriptionText, gParams->filterDescriptionText, sizeof(fmc.filterDescriptionText));
    memcpy(fmc.filterOrganizationText, gParams->filterOrganizationText, sizeof(fmc.filterOrganizationText));
    memcpy(fmc.filterURLText, gParams->filterURLText, sizeof(fmc.filterURLText));
    memcpy(fmc.filterVersionText, gParams->filterVersionText, sizeof(fmc.filterVersionText));
    memcpy(fmc.filterUniqueID, gParams->filterUniqueID, sizeof(fmc.filterUniqueID));
    
    // Get filter installation directory...
    getFilterInstallDir();

    //fmc.DESIGNTIME=false;

    // Check for valid mode...
#if 1
    ///// Temp kludge!!!!  Allow all modes for now...
    gResult = noErr;
#else
    gResult = filterBadMode;    // PSP 4.0 doesn't recognize this!!
#endif

    switch (gStuff->imageMode)
    {
    case plugInModeBitmap:
        strcpy(fmc.filterImageModeText, "Bitmap");
        if (gParams->flags & AFH_SUPPORT_BITMAP)
            gResult = noErr;
        break;

    case plugInModeGrayScale:
        strcpy(fmc.filterImageModeText, "Grayscale");
        if (gParams->flags & AFH_SUPPORT_GRAY)
            gResult = noErr;
        break;

    case plugInModeDuotone:
        strcpy(fmc.filterImageModeText, "Duotone");
        if (gParams->flags & AFH_SUPPORT_DUOTONE)
            gResult = noErr;
        break;

    case plugInModeIndexedColor:
        strcpy(fmc.filterImageModeText, "Indexed Color");
        if (gParams->flags & AFH_SUPPORT_INDEXED)
            gResult = noErr;
        break;

    case plugInModeRGBColor:
        strcpy(fmc.filterImageModeText, "RGB Color");
        if (gParams->flags & AFH_SUPPORT_RGB)
            gResult = noErr;
        break;

    case plugInModeCMYKColor:
        strcpy(fmc.filterImageModeText, "CMYK Color");
        if (gParams->flags & AFH_SUPPORT_CMYK)
            gResult = noErr;
        break;

    case plugInModeLabColor:
        strcpy(fmc.filterImageModeText, "Lab Color");
        if (gParams->flags & AFH_SUPPORT_LAB)
            gResult = noErr;
        break;

    case plugInModeMultichannel:
        strcpy(fmc.filterImageModeText, "Multichannel");
        if (gParams->flags & AFH_SUPPORT_MULTI)
            gResult = noErr;
        break;

    case plugInModeHSLColor:
        strcpy(fmc.filterImageModeText, "HSL Color");
        break;

    case plugInModeHSBColor:
        strcpy(fmc.filterImageModeText, "HSB Color");
        break;

    case plugInModeGray16:
        strcpy(fmc.filterImageModeText, "16-bit Grayscale");
        break;

    case plugInModeRGB48:
        strcpy(fmc.filterImageModeText, "48-bit RGB Color");
        break;

//Added by Harald Heim, December 6, 2002

    case plugInModeLab48:
        strcpy(fmc.filterImageModeText, "48-bit Lab Color");
        break;
    
    case plugInModeCMYK64:
        strcpy(fmc.filterImageModeText, "64-bit CMYK Color");
        break;

    case plugInModeDeepMultichannel:
        strcpy(fmc.filterImageModeText, "Deep Multichannel");
        break;

    case plugInModeDuotone16:
        strcpy(fmc.filterImageModeText, "16-bit Duotone");
        break;


    
    default:
        strcpy(fmc.filterImageModeText, "Unknown Image Mode");
        break;
    } /*switch*/


    if (gResult != noErr)
    {
        if (gResult == filterBadMode && gStuff->hostSig != '8BIM')
        {   // PSP and others don't recognize the filterBadMode error,
            // so put out our own message box...
            ShowAlert(16501);
            gResult = 1;    // indicates error message already displayed.
        }
        return;
    }

    
	//set default to allocation from the host
	set_array_mode(0);

    //Initialize Arrays
    for (i=0; i<N_ARRAYS; i++){
                Array[i] = 0;
				ArrayID[i] = 0;
                ArrayPadding[i]=0;
				ArraySize[i]=0;
				ArrayBytes[i]=0;
				ArrayX[i]=0;
				ArrayY[i]=0; 
				ArrayZ[i]=0;
    }

    //Initialize Font Array
    for (i=0; i<N_FONTS; i++){
        Font[i] = 0;		
    }



#ifdef SCRIPTABLE

    //Initialize Script Array
	for (i=0; i<N_SCRIPTITEMS; i++){
        ScriptParam[i] = 0;
        ScriptParamUsed[i] = false;
    }

	ReadScriptParameters(&displayDialog); //err = ReadScriptParameters(&displayDialog);
    fmc.displayDialog = displayDialog;
    fmc.doingScripting = ScriptParamRead;

#endif

	mtAbortFlag = false; //For aborting multiple threads


	if (fmc.displayDialog)
    {
        //Display Dialog
		PlatformData *platform = (PlatformData *)((FilterRecordPtr) gStuff)->platformData;
        
        /*if (CheckProxiesSupported (gStuff->displayPixels) && WarnAdvanceStateAvailable())
		    PromptUserForInput (globals);   // Show the UI
        else
            PromptUserForInputNoProxy (globals);*/

		CheckProxiesSupported (gStuff->displayPixels);
        PromptUserForInput (globals);   // Show the UI
		

		//Unhook Key Capture
		if (KeyHook != NULL) UnhookWindowsHookEx(KeyHook);



        if (gResult == noErr){

            //user hit OK

            gParams->queryForParameters = FALSE;
			
		#if 0
			//Clean up threads
			terminateThread(0);
		#endif
		#if STYLER
			deleteThreads();
		#endif

        } else {

            //user hit Cancel, exit button or some other error

			//if (gParams->gDialogEvent & 2) triggerEvent(0,FME_CANCEL,0);

			finalCleanup();
            
            // Maybe should also copy the global vars to the param block here????

            return;

        }


    }
    else
    {
        // No Dialog will be displayed
        // don't need to ask for params
        // ...but may need to adjust passed-in params; e.g.,
        //    if the filterCase has changed...

    #ifdef SCRIPTABLE
        //Write the script params to the controls
        if (ScriptParamRead){
            int n;
			for (n=0;n<N_SCRIPTITEMS;n++) {
                if (ScriptParamUsed[n]) gFmcp->pre_ctl[n] = gParams->ctl[n].val = ScriptParam[n]; //setCtlVal(n, ScriptParam[n]);
            }
        }
    #endif

    }


    //Finally apply filter to image...

    //set doingProxy FALSE again, since PromptUserForInput may have
    //set it TRUE...
    fmc.doingProxy = FALSE;

	//Perform pre-filter initializations...
    InitBeforeFilter();

    if (WarnAdvanceStateAvailable())
        StartWithAdvanceState(globals);
    else
        StartNoAdvanceState(globals);

    call_FFP(FFP_OnFilterEnd,(INT_PTR)&fmc);//OnFilterEnd(&fmc);

    // Maybe here is a good place to copy the global vars to the param block????
#ifndef DONT_SAVE_GLOBALS
    /* Save the following predeclared global and user-declared global/static variables */
    gParams->globvars = gFmcp->globvars;
#endif //DONT_SAVE_GLOBALS


} /*DoStart*/

/******************************************************************************/
/* Does all of the filtering with no advance state and proxies. */



void StartNoAdvanceState (GPtr globals)
{
    DoInitialRect(globals);

} /*StartNoAdvanceState*/
    


/*****************************************************************************/
/* Does all of the filtering. */


void StartWithAdvanceState (GPtr globals)
{
    
    //int total;
    int row, rowDelta = gParams->tileHeight; //bandWidth;
    int col, colDelta = gParams->tileWidth;
    //int top,bottom,left,right;

    // Do the actual filtering operation on the original image
#if BIGDOCUMENT      
	fmc.filterRect32 = GetFilterRect();
    //total = fmc.filterRect32.bottom - fmc.filterRect32.top;  
#else
    //total = gStuff->filterRect.bottom - gStuff->filterRect.top;
#endif	 

    // If the algorithm isn't tileable, grab the entire image...
    //if (!fmc.isTileable) rowDelta = total;


    gStuff->inLoPlane = gStuff->outLoPlane = 0;
    gStuff->inHiPlane = gStuff->outHiPlane = gStuff->planes - 1;

  #if BIGDOCUMENT
	  // Set left and right edges of inRect and outRect to the full width of the filterRect.
	   fmc.inRect32 = fmc.outRect32 = fmc.filterRect32;
	   SetInRect(fmc.inRect32);
	   SetOutRect(fmc.outRect32);
																										 
       if (!fmc.isTileable || rowDelta==0) rowDelta = fmc.filterRect32.bottom - fmc.filterRect32.top;
       if (!fmc.isTileable || colDelta==0) colDelta = fmc.filterRect32.right - fmc.filterRect32.left;
	   
       PISetRect32(&fmc.maskRect32, 0, 0, 0, 0);
	   SetMaskRect(fmc.maskRect32);
#else
        gStuff->filterRect.top = gStuff->filterRect.top;
        gStuff->filterRect.bottom = gStuff->filterRect.bottom;
        gStuff->filterRect.left = gStuff->filterRect.left;
        gStuff->filterRect.right = gStuff->filterRect.right;
        if (!fmc.isTileable || rowDelta==0) rowDelta = gStuff->filterRect.bottom - gStuff->filterRect.top;
        if (!fmc.isTileable || colDelta==0) colDelta = gStuff->filterRect.right - gStuff->filterRect.left;
        gStuff->inRect.left = gStuff->outRect.left = gStuff->filterRect.left;
        gStuff->inRect.right = gStuff->outRect.right = gStuff->filterRect.right;
        PISetRect (&gStuff->maskRect, 0, 0, 0, 0);
#endif
    //}



    gStuff->inputRate = long2fixed (1L);
    gStuff->maskRate = long2fixed (1L);
    
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
    
#if BIGDOCUMENT      
	for (row = fmc.filterRect32.top; row < fmc.filterRect32.bottom; row += rowDelta)
    //for (col = fmc.filterRect32.left; col < fmc.filterRect32.right; col += colDelta)
#else    
    for (row = gStuff->filterRect.top; row < gStuff->filterRect.bottom; row += rowDelta)
    for (col = gStuff->filterRect.left; col < gStuff->filterRect.right; col += colDelta)
#endif
    {
        
        if (TestAbort ())
        {
            gResult = userCanceledErr;
            goto done;
        }
        
#if BIGDOCUMENT      
        fmc.inRect32.top = fmc.outRect32.top = row;
        if (rowDelta > fmc.filterRect32.bottom - row)
            rowDelta = fmc.filterRect32.bottom - row;
        fmc.inRect32.bottom = fmc.outRect32.bottom = row + rowDelta;
		SetInRect(fmc.inRect32);
		SetOutRect(fmc.outRect32); 
#else
        gStuff->inRect.top = gStuff->outRect.top = row;
        //if (rowDelta > gStuff->filterRect.bottom - row)
        //    rowDelta = gStuff->filterRect.bottom - row;
        //gStuff->inRect.bottom = gStuff->outRect.bottom = row + rowDelta;
        if (rowDelta > gStuff->filterRect.bottom - row)
            gStuff->inRect.bottom = gStuff->outRect.bottom = gStuff->filterRect.bottom;
        else
            gStuff->inRect.bottom = gStuff->outRect.bottom = row + rowDelta;

        gStuff->inRect.left = gStuff->outRect.left = col;
        if (colDelta > gStuff->filterRect.right - col)
            gStuff->inRect.right = gStuff->outRect.right= gStuff->filterRect.right;
        else
            gStuff->inRect.right = gStuff->outRect.right= col + colDelta;
#endif

        
#ifndef DONT_USE_MASK
        if (gStuff->haveMask){
#if BIGDOCUMENT
			fmc.maskRect32 = fmc.inRect32;
			SetMaskRect(fmc.maskRect32);
#else
                gStuff->maskRect = gStuff->inRect;
#endif
        }
#endif

        addPadding(globals);


        gResult = AdvanceState ();
        

        subtractPadding(globals);

        if (gResult != noErr)
        {
#if 0
            ShowAlert (16500); //debug PSP 4.12
#endif
            goto done;
        }
        
        DoFilterRect (globals);
        
        if (gResult != noErr)
        {
            goto done;
        }
        
    } //for
    
done:
        
#if BIGDOCUMENT
    // Zero the request Rectangles to signal a stop
    PISetRect32(&fmc.inRect32, 0, 0, 0, 0);
    PISetRect32(&fmc.outRect32, 0, 0, 0, 0);
    PISetRect32(&fmc.maskRect32, 0, 0, 0, 0);
	SetInRect(fmc.inRect32);
	SetOutRect(fmc.outRect32);
	SetMaskRect(fmc.maskRect32);
#else
    PISetRect (&gStuff->inRect, 0, 0, 0, 0);
    PISetRect (&gStuff->outRect, 0, 0, 0, 0);
    PISetRect (&gStuff->maskRect, 0, 0, 0, 0);
#endif

    if ((gParams->flags & AFH_FLUSH_ADVANCESTATE) && (gResult == noErr))
    {

        // Call AdvanceState() one more time to flush out PSP 4.12...
        gResult = AdvanceState ();
    
    } //if
} /*StartWithAdvanceState*/



/*****************************************************************************/
void InitContextRecord(void)
{
    // Initialize static portion of the FM context record.
    // (Needs to be done only once per DLL invocation.)
    int i;

    // Following private C-RTL routine converts float on top
    // of FPU stack to long int in EAX, but is not declared
    // in any public header file (it's only referenced by
    // compiler-generated code).
#ifndef _WIN64
    extern long _ftol(void);
#endif

    // Initialize the info in the "frozen" section of the FM
    // context record.  This section should never be altered.
    // Any additions to the FM context record must *follow*
    // the frozen section.

    // initialize fmc.undefined with a signaling NaN.
    *(__int64 *)&fmc.undefined = 0x7ff7ffffffffffff; /* largest positive double SNaN */
    fmc.FMC_major_version = FMC_MAJOR_VERSION;
    fmc.FMC_minor_version = FMC_MINOR_VERSION;
    fmc.FMC_struct_size = sizeof(fmc);

#ifndef TRIG_ON_FLY
    // Precompute the trig tables...
    for (i=0; i < 1024; i++)
        fmc.pre_sin[i] = (int)(sin( (twopi/1024.0)*(double)i ) * 512.0 + 0.0); // round??

    for (i=0; i < 512; i++) {
        if ( i == 256)
            fmc.pre_tan[i] = (int)(tan( (twopi/1024.0)*(-255.5) ) * 1024.0 + 0.0);    // per Jens???
        else
            fmc.pre_tan[i] = (int)(tan( (twopi/1024.0)*(double)i ) * 1024.0 + 0.0);   // round?? can be neg!!
    }
#endif

    // Initialize constants

    // NDP control word constants, used for various rounding modes
    *(int *)&fmc.CW_ROUND = 0x133F;  //IC=affine, RC=00(round), PC=11(64bits), all 6 exceptions masked
    *(int *)&fmc.CW_FLOOR = 0x173F;  //IC=affine, RC=01(floor), PC=11(64bits), all 6 exceptions masked
    *(int *)&fmc.CW_CEIL  = 0x1B3F;  //IC=affine, RC=10(ceil),  PC=11(64bits), all 6 exceptions masked
    *(int *)&fmc.CW_CHOP  = 0x1F3F;  //IC=affine, RC=11(chop),  PC=11(64bits), all 6 exceptions masked

    // Save current control word (not a constant, but presumably 027F for MSVC).
    //fmc.current_cw = fstcw();

    // The following "magic" constant has been carefully tuned for
    // use in rounding down when computing an integer square root,
    // over most (if not all) the domain of sqr().  See the code
    // for OP_SQR in the FM compiler module.
    fmc.magic5 = 0.49999999999999;

    // Following approximation to 512.0/Pi is used to compute d...
    fmc.d512byPi = 256.0 * 0.63661977236758134308;

    // Following 32-bit floating-point constant 1.0F is used by
    // the in-line code generated for powi().  We specifically
    // want a 32-bit float, not a 64-bit double, because (at least
    // on the 387), FDIVR_F 1.0F is faster than FDIVR_D 1.0.
    fmc.float1p0 = 1.0F;

    // Initialize built-in strings...
    fmc.filterAuthorText[0] = 0;
    fmc.filterCategoryText[0] = 0;
    fmc.filterTitleText[0] = 0;
    fmc.filterFilenameText[0] = 0;
    fmc.filterCopyrightText[0] = 0;
    fmc.filterDescriptionText[0] = 0;
    fmc.filterOrganizationText[0] = 0;
    fmc.filterURLText[0] = 0;
    fmc.filterVersionText[0] = 0;
    fmc.filterCaseText[0] = 0;
    fmc.filterImageModeText[0] = 0;
    fmc.filterHostText[0] = 0;
    fmc.filterInstallDir[0] = 0;
    fmc.filterUniqueID[0] = 0;

#ifndef DONT_SAVE_GLOBALS
    //Zero out the predefined globals (harmless, but probably unnecessary) */
    memset(&fmc.globvars, 0, sizeof(fmc.globvars));
#endif //DONT_SAVE_GLOBALS


    // Precompute the gamma table for gamma = 1.0...
    //setGamma(1.0);

    // Initialize vectors to FM built-in routines...


    // Standard FF built-ins.
    if (gStuff->imageMode == plugInModeRGB48 ||
        gStuff->imageMode == plugInModeGray16 ||
        gStuff->imageMode == plugInModeLab48 || 
        gStuff->imageMode == plugInModeCMYK64 ||
        gStuff->imageMode == plugInModeDeepMultichannel ||
        gStuff->imageMode == plugInModeDuotone16
        ) {
       
        // pixel depth = 16
        fmc.src = src16;
        fmc.pset = pset16;
        fmc.pget = pget16;
        //fmc.pgetr = pgetr16;

		fmc.tget = tget16;
	    fmc.t2get = t2get16;
		fmc.t3get = t3get16;
		fmc.t4get = t4get16;
		fmc.tset = tset16;
		fmc.t2set = t2set16;
		fmc.t3set = t3set16;
		fmc.t4set = t4set16;
    }
    else {
        
        // pixel depth = 8
        fmc.src = src;
        fmc.pset = pset;
        fmc.pget = pget;
        //fmc.pgetr = pgetr;

		fmc.tget = tget;
	    fmc.t2get = t2get;
		fmc.t3get = t3get;
		fmc.t4get = t4get;
		fmc.tset = tset;
		fmc.t2set = t2set;
		fmc.t3set = t3set;
		fmc.t4set = t4set;
    }



    //Following are not necessarily static, but we initialize
    //them here anyway for good hygiene...
    //These also get reinitialized later at the start of each tile.
    fmc.dmax = fmc.D = 1024;
    fmc.xmin = fmc.ymin = fmc.zmin = fmc.pmin = fmc.mmin = fmc.dmin = 0;
    fmc.rmax = fmc.gmax = fmc.bmax = fmc.amax = fmc.cmax = 255;	//should change with bit-depth??
    fmc.imax = fmc.umax = fmc.vmax = 255;
    fmc.rmin = fmc.gmin = fmc.bmin = fmc.amin = fmc.cmin = 0;
    fmc.imin = fmc.umin = fmc.vmin = 0;
    fmc.R = fmc.G = fmc.B = fmc.A = fmc.C = 255;    //conflict between FF and FF+???
    fmc.I = fmc.U = fmc.V = 255;
    fmc.t = fmc.tmin = 0;
    fmc.total = fmc.tmax = 1;

    //Initialize some other variables for good hygiene.
    fmc.n = 0;
    fmc.ctlMsg = 0;
    fmc.ctlCode = 0;
    fmc.ctlClass = CC_UNUSED;
    fmc.dividend = 0;
    fmc.divisor = 1;

    fmc.doingProxy = FALSE;
    fmc.doingScripting = 0;

    //initialize random number generator 
    srand(1);

    //initialize anonymous cells
    if (Dont_Initialize_Cells == 0)
    {
        for (i = 0; i < N_CELLS; i++) {
            fmc.cell[i] = 0;
        }
    }

    // Cache current control values in fmc.pre_ctl[].
    for (i=0; i < N_CTLS; i++) {
        fmc.pre_ctl[i] = gParams->ctl[i].val;
    }

} /*InitContextRecord*/

/*****************************************************************************/
/* Filter the area. */


//Commented out by Harald Heim, Jun 4, 2002
/*
#define maxBlurRadius 3

#if 0
#define mskVal(row,rowOff,col,colOff) \
            fmc.mskPtr[(rowOff)*fmc.mskRowDelta+(colOff)*fmc.mskColDelta]
#elif 0
#define mskVal(row,rowOff,col,colOff) \
            fmc.mskData[((row)+(rowOff))*fmc.mskRowDelta+((col)+(colOff))*fmc.mskColDelta]
#elif 1
static unsigned8 gMskVal(int row, int rowOff, int col, int colOff)
{
    // alert if out of bounds, for debugging...
#ifdef CHECK_GMSKVAL
#ifndef NOTRACE
    if ( row + rowOff <            -gBevWidth-maxBlurRadius   ||
         row + rowOff >    fmc.rows+gBevWidth+maxBlurRadius-1 ||
         col + colOff <            -gBevWidth-maxBlurRadius   ||
         col + colOff > fmc.columns+gBevWidth+maxBlurRadius-1 )
    {
        Trace (globals, "Bad maskData subscripts:\n %d, %d, %d, %d",
               row, rowOff, col, colOff);
    }
#endif
#endif

    if ((gNoMask || !(gParams->flags & AFH_USE_HOST_PADDING)) &&
        (row+rowOff < minRow || row+rowOff > maxRow ||
         col+colOff < minCol || col+colOff > maxCol) )
        return 0;   // Simulate the padding...
    else if (gNoMask)
        return 255; // Entire filterRect is inside the mask.
    else
        return fmc.mskPtr[rowOff*fmc.mskRowDelta + colOff*fmc.mskColDelta];
} //gMskVal
#define mskVal(row,rowOff,col,colOff) \
    gMskVal(row,rowOff,col,colOff)
#endif
*/


////////////////////////////////////////
int copyInputToOutput();

void DoFilterRect (GPtr localGlobals)
{
    static int reentry_depth = 0;
    static int reentry_count = 0;
    //int startclock;


    if (gResult != noErr) {
        // Got an error somewhere along the way...
        // Get out quick in case the error was returned by
        // AdvanceState(), in which case the input/output data
        // pointers are bogus and can cause severe dyspepsia.
        return;
    }

    //check for reentry...
    if (reentry_depth++ > 0) {
        //reentered!!! Log the fact, and exit quickly...
        reentry_count++;
        reentry_depth--;
        return; //get out quick!!!
    }

    globals = localGlobals;

    if (!gStuff->outData) {
        // I think I've caught all cases where zero-dragging
        // can cause a GP in the filter code because outData
        // is set to 0 while dragging.  This is just a last-
        // minute sanity check to try to avoid  GP-ing on the
        // end user!  (Record any failures for later analysis.)
        static int unexpected_null_outData_aborts = 0;
        unexpected_null_outData_aborts++;
#if 1
        Info("Unexpected NULL outData, count = %d", unexpected_null_outData_aborts);
#endif
        reentry_depth--;
        return; //get out quick!!!
    }

    if (fmc.doingProxy) {
        //proxy is never tiled, so we can do this here...
        InitBeforeFilter();
    }

    { //scope
#if BIGDOCUMENT
	VRect scaledFilterRect;  
#else
    Rect scaledFilterRect;
#endif
    OSErr err = noErr;

    fmc.scaleFactor = gStuff->inputRate >> 16;
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

    { //new scope

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
    //fmc.Z = fmc.zmax = fmc.pmax = (gStuff->planes > 4)?4:gStuff->planes; //What about CMYK + alpha???
    fmc.Z = fmc.zmax = fmc.pmax = gStuff->planes;
    fmc.M = fmc.mmax = (int)( sqrt((double)(fmc.X*fmc.X + fmc.Y*fmc.Y)) / 2 + 0.0 ); // don't round per Jens???
    fmc.dmax = fmc.D = 1024;
    fmc.xmin = fmc.ymin = fmc.zmin = fmc.pmin = fmc.mmin = fmc.dmin = 0;
    fmc.rmax = fmc.gmax = fmc.bmax = fmc.amax = fmc.cmax = 255;	//should change with bit-depth??
    fmc.imax = fmc.umax = fmc.vmax = 255;
    fmc.rmin = fmc.gmin = fmc.bmin = fmc.amin = fmc.cmin = 0;
    fmc.imin = fmc.umin = fmc.vmin = 0;
    fmc.R = fmc.G = fmc.B = fmc.A = fmc.C = 255;    //conflict between FF and FF+???
    fmc.I = fmc.U = fmc.V = 255;
    fmc.t = fmc.tmin = 0;
    fmc.total = fmc.tmax = 1;

#if 0   //already done in InitBeforeFilter.
    // Cache current control values in fmc.pre_ctl[].
    for (i=0; i < N_CTLS; i++) {
        fmc.pre_ctl[i] = gParams->ctl[i].val;
    }
#endif

    fmc.hDlg = MyHDlg; //already done in WM_INITDIALOG!!!

    // Compute min/max values for column and row indices that still lie
    // within the (scaled) filterRect (used to simulate padding of accesses
    // outside the bounds of filterRect).
#if BIGDOCUMENT
	/*fmc.outRect32 = GetOutRect();	//needed??
    minCol = scaledFilterRect.left   - fmc.outRect32.left;
    maxCol = scaledFilterRect.right  - fmc.outRect32.left - 1;
    minRow = scaledFilterRect.top    - fmc.outRect32.top;
    maxRow = scaledFilterRect.bottom - fmc.outRect32.top - 1;*/
#else
    /*minCol = scaledFilterRect.left   - gStuff->outRect.left;
    maxCol = scaledFilterRect.right  - gStuff->outRect.left - 1;
    minRow = scaledFilterRect.top    - gStuff->outRect.top;
    maxRow = scaledFilterRect.bottom - gStuff->outRect.top - 1;*/
#endif


    {   

#if NOISECONTROL
#ifndef NOOUTPUTCOPY
    if (ctl(fmc.previewCheckbox)==0){
#endif            
#endif

        // pass 2: ...
		if (!copyInputToOutput()) goto egress;

#if NOISECONTROL
#ifndef NOOUTPUTCOPY
    }
#endif
#endif

        //call ForEveryTile...
        fmc.x = fmc.x_start;
        fmc.y = fmc.y_start;
        fmc.x_end = fmc.x_start + fmc.columns;
        fmc.y_end = fmc.y_start + fmc.rows;
        fmc.row = 0;
        fmc.col = 0;
        fmc.tbuf = NULL;
        fmc.t2buf = NULL;
        fmc.t3buf = NULL;
        fmc.t4buf = NULL;

        {
            //need to handle longjmp back???
            int res;
#if 0
            //Min version of tile buffers (same size as output tile)
            if (fmc.need_tbuf) {
                //allocate a (temporary) tile buffer...
                //Should use AllocateBuffer() ????
                do {
                    res = IDOK;

                    //some kind of dword padding (+ 3) added by Harald Heim, February 2003

					fmc.tbuf = calloc( fmc.rows * fmc.columns * fmc.Z + 3 , sizeof (fmc.tbuf[0]) );

                    if (!fmc.tbuf) {
                        res = Error("Could not allocate a tile buffer (%d bytes).",
                                fmc.rows * fmc.columns * fmc.Z * sizeof (fmc.tbuf[0]));
                    }
                } while (res == IDRETRY);
                if (res == IDABORT) {
                    gResult = memFullErr;
                    goto egress;
                }
#if 0
                else {
                    Info("Allocated a tile buffer (%d bytes).",
                            fmc.rows * fmc.columns * fmc.Z * sizeof (fmc.tbuf[0]));
                }
#endif
            }//if need tbuf
            if (fmc.need_t2buf) {
                //allocate a (temporary) tile buffer 2...
                //Should use AllocateBuffer() ????
                do {
                    res = IDOK;

                    //some kind of dword padding (+ 3) added by Harald Heim, February 2003
 
                    fmc.t2buf = calloc(fmc.rows * fmc.columns * fmc.Z + 3, sizeof (fmc.t2buf[0]));

                    if (!fmc.t2buf) {
                        res = Error("Could not allocate tile buffer 2 (%d bytes).",
                                fmc.rows * fmc.columns * fmc.Z * sizeof (fmc.t2buf[0]));
                    }
                } while (res == IDRETRY);
                if (res == IDABORT) {
                    gResult = memFullErr;
                    goto egress;
                }
#if 0
                else {
                    Info("Allocated tile buffer 2 (%d bytes).",
                            fmc.rows * fmc.columns * fmc.Z * sizeof (fmc.t2buf[0]));
                }
#endif
            }//if need t2buf
			if (fmc.need_t3buf) {
                //allocate a (temporary) tile buffer 3...
                //Should use AllocateBuffer() ????
                do {
                    res = IDOK;

                    //some kind of dword padding (+ 3) added by Harald Heim, February 2003
 
                    fmc.t3buf = calloc(fmc.rows * fmc.columns * fmc.Z + 3, sizeof (fmc.t3buf[0]));

                    if (!fmc.t3buf) {
                        res = Error("Could not allocate tile buffer 3 (%d bytes).",
                                fmc.rows * fmc.columns * fmc.Z * sizeof (fmc.t3buf[0]));
                    }
                } while (res == IDRETRY);
                if (res == IDABORT) {
                    gResult = memFullErr;
                    goto egress;
                }
#if 0
                else {
                    Info("Allocated tile buffer 3 (%d bytes).",
                            fmc.rows * fmc.columns * fmc.Z * sizeof (fmc.t3buf[0]));
                }
#endif
            }//if need t3buf
			if (fmc.need_t4buf) {
                //allocate a (temporary) tile buffer 4...
                //Should use AllocateBuffer() ????
                do {
                    res = IDOK;

                    //some kind of dword padding (+ 3) added by Harald Heim, February 2003
 
                    fmc.t4buf = calloc(fmc.rows * fmc.columns * fmc.Z + 3, sizeof (fmc.t3buf[0]));

                    if (!fmc.t4buf) {
                        res = Error("Could not allocate tile buffer 4 (%d bytes).",
                                fmc.rows * fmc.columns * fmc.Z * sizeof (fmc.t4buf[0]));
                    }
                } while (res == IDRETRY);
                if (res == IDABORT) {
                    gResult = memFullErr;
                    goto egress;
                }
#if 0
                else {
                    Info("Allocated tile buffer 4 (%d bytes).",
                            fmc.rows * fmc.columns * fmc.Z * sizeof (fmc.t4buf[0]));
                }
#endif
            }//if need t4buf
#else
            //Max version of tile buffers (same size as input tile)
            if (fmc.need_tbuf) {
                //allocate a (temporary) tile buffer...
                //Should use AllocateBuffer() ????
				
                do {
                    int bufferSize = fmc.imageMode>9 ? 2:1;
					
					res = IDOK;

                    //some kind of dword padding (+ 3) added by Harald Heim, February 2003
					
					if (gStuff->bufferProcs && gStuff->bufferProcs->allocateProc && gStuff->bufferProcs->lockProc && gStuff->bufferProcs->unlockProc && gStuff->bufferProcs->freeProc){ //Photoshop allocation
						if (AllocateBuffer( (fmc.X * fmc.Y * fmc.Z + 3) * bufferSize, &fmc.tbufID) == noErr){
							fmc.tbuf = (unsigned char *)LockBuffer(fmc.tbufID, true);
						} else { //Fall back to Windows allocation
							fmc.tbuf = (unsigned char *)calloc(fmc.X * fmc.Y * fmc.Z + 3, bufferSize);
							fmc.tbufID=0;
						}
					}else { //Windows allocation
						fmc.tbuf = (unsigned char *)calloc(fmc.X * fmc.Y * fmc.Z + 3, bufferSize); //sizeof (fmc.tbuf[0])
						fmc.tbufID=0;
					}


                    if (!fmc.tbuf) {
                        res = Error("Could not allocate a tile buffer (%d bytes).",
                                fmc.X * fmc.Y * fmc.Z * bufferSize); //sizeof (fmc.tbuf[0])
                    }
                } while (res == IDRETRY);
                if (res == IDABORT) {
                    gResult = memFullErr;
                    goto egress;
                }
#if 0
                else {
                    Info("Allocated a tile buffer (%d bytes).",
                            fmc.X * fmc.Y * fmc.Z * sizeof (fmc.tbuf[0]));
                }
#endif
            }//if need tbuf
            if (fmc.need_t2buf) {
                //allocate a (temporary) tile buffer 2...
                //Should use AllocateBuffer() ????
                do {
                    int bufferSize = fmc.imageMode>9 ? 2:1;
					res = IDOK;

                    //some kind of dword padding (+ 3) added by Harald Heim, February 2003

                    if (gStuff->bufferProcs && gStuff->bufferProcs->allocateProc && gStuff->bufferProcs->lockProc && gStuff->bufferProcs->unlockProc && gStuff->bufferProcs->freeProc){ //Photoshop allocation
						if (AllocateBuffer( (fmc.X * fmc.Y * fmc.Z + 3) * bufferSize, &fmc.t2bufID) == noErr){
							fmc.t2buf = (unsigned char *)LockBuffer(fmc.t2bufID, true);
						} else { //Fall back to Windows allocation
							fmc.t2buf = (unsigned char *)calloc(fmc.X * fmc.Y * fmc.Z + 3, bufferSize);
							fmc.t2bufID=0;
						}
					}else { //Windows allocation
						fmc.t2buf = (unsigned char *)calloc(fmc.X * fmc.Y * fmc.Z + 3, bufferSize); //sizeof (fmc.t2buf[0])
						fmc.t2bufID=0;
					}


                    if (!fmc.t2buf) {
                        res = Error("Could not allocate tile buffer 2 (%d bytes).",
                                fmc.X * fmc.Y * fmc.Z * bufferSize); //sizeof (fmc.t2buf[0])
                    }
                } while (res == IDRETRY);
                if (res == IDABORT) {
                    gResult = memFullErr;
                    goto egress;
                }
#if 0
                else {
                    Info("Allocated tile buffer 2 (%d bytes).",
                            fmc.X * fmc.Y * fmc.Z * sizeof (fmc.t2buf[0]));
                }
#endif
            }//if need t2buf
			if (fmc.need_t3buf) {
                //allocate a (temporary) tile buffer 3...
                //Should use AllocateBuffer() ????
                do {
                    int bufferSize = fmc.imageMode>9 ? 2:1;
					res = IDOK;

                    //some kind of dword padding (+ 3) added by Harald Heim, February 2003
					
					if (gStuff->bufferProcs && gStuff->bufferProcs->allocateProc && gStuff->bufferProcs->lockProc && gStuff->bufferProcs->unlockProc && gStuff->bufferProcs->freeProc){ //Photoshop allocation
						if (AllocateBuffer( (fmc.X * fmc.Y * fmc.Z + 3) * bufferSize, &fmc.t3bufID) == noErr){
							fmc.t3buf = (unsigned char *)LockBuffer(fmc.t3bufID, true);
						} else { //Fall back to Windows allocation
							fmc.t3buf = (unsigned char *)calloc(fmc.X * fmc.Y * fmc.Z + 3, bufferSize);
							fmc.t3bufID=0;
						}
					}else { //Windows allocation
						fmc.t3buf = (unsigned char *)calloc(fmc.X * fmc.Y * fmc.Z + 3, bufferSize); //sizeof (fmc.tbuf[0])
						fmc.t3bufID=0;
					}

                    if (!fmc.t3buf) {
                        res = Error("Could not allocate tile buffer 3 (%d bytes).",
                                fmc.X * fmc.Y * fmc.Z * bufferSize); //sizeof (fmc.t3buf[0])
                    }
                } while (res == IDRETRY);
                if (res == IDABORT) {
                    gResult = memFullErr;
                    goto egress;
                }
#if 0
                else {
                    Info("Allocated tile buffer 3 (%d bytes).",
                            fmc.X * fmc.Y * fmc.Z * sizeof (fmc.t3buf[0]));
                }
#endif
            }//if need t3buf
			if (fmc.need_t4buf) {
                //allocate a (temporary) tile buffer 4...
                //Should use AllocateBuffer() ????
                do {
                    int bufferSize = fmc.imageMode>9 ? 2:1;
					res = IDOK;

                    //some kind of dword padding (+ 3) added by Harald Heim, February 2003
					
					if (gStuff->bufferProcs && gStuff->bufferProcs->allocateProc && gStuff->bufferProcs->lockProc && gStuff->bufferProcs->unlockProc && gStuff->bufferProcs->freeProc){ //Photoshop allocation
						if (AllocateBuffer( (fmc.X * fmc.Y * fmc.Z + 3) * bufferSize, &fmc.t4bufID) == noErr){
							fmc.t4buf = (unsigned char *)LockBuffer(fmc.t4bufID, true);
						} else { //Fall back to Windows allocation
							fmc.t4buf = (unsigned char *)calloc(fmc.X * fmc.Y * fmc.Z + 3, bufferSize);
							fmc.t4bufID=0;
						}
					}else { //Windows allocation
						fmc.t4buf = (unsigned char *)calloc(fmc.X * fmc.Y * fmc.Z + 3, bufferSize); //sizeof (fmc.tbuf[0])
						fmc.t4bufID=0;
					}

                    if (!fmc.t4buf) {
                        res = Error("Could not allocate tile buffer 4 (%d bytes).",
                                fmc.X * fmc.Y * fmc.Z * bufferSize); //sizeof (fmc.t4buf[0])
                    }
                } while (res == IDRETRY);
                if (res == IDABORT) {
                    gResult = memFullErr;
                    goto egress;
                }
#if 0
                else {
                    Info("Allocated tile buffer 4 (%d bytes).",
                            fmc.X * fmc.Y * fmc.Z * sizeof (fmc.t4buf[0]));
                }
#endif
            }//if need t4buf
#endif

            //if (gParams->gInitialInvocation) scaleLock = false; //Activate Control Scaling again

            //startclock = clock();
			
            res = call_FFP(FFP_ForEveryTile,(INT_PTR)&fmc); //res = ForEveryTile(&fmc);
            //if (!res) {
            //    res = call_FFP(FFP_ForEveryRow,(INT_PTR)&fmc);//res = ForEveryRow(&fmc);
            //}

            //fmc.renderTime = clock() - startclock;

        }
    }//scope


egress:
    // Free the temporary tile buffers if allocated...
    if (fmc.tbuf) {
        if (fmc.tbufID != 0){ //was allocated with Buffer Proc
			UnlockBuffer(fmc.tbufID);
			FreeBuffer(fmc.tbufID); 
		} else
			free (fmc.tbuf);

		fmc.tbufID = 0;
        fmc.tbuf = NULL;
#if 0
        Info("Freed the tile buffer.");
#endif
    }
    if (fmc.t2buf) {
        if (fmc.t2bufID != 0){ //was allocated with Buffer Proc
			UnlockBuffer(fmc.t2bufID);
			FreeBuffer(fmc.t2bufID); 
		} else
			free (fmc.t2buf);

		fmc.t2bufID = 0;
        fmc.t2buf = NULL;
#if 0
        Info("Freed tile buffer 2.");
#endif
    }
	if (fmc.t3buf) {
        if (fmc.t3bufID != 0){ //was allocated with Buffer Proc
			UnlockBuffer(fmc.t3bufID);
			FreeBuffer(fmc.t3bufID); 
		} else
			free (fmc.t3buf);

		fmc.t3bufID = 0;
        fmc.t3buf = NULL;
#if 0
        Info("Freed tile buffer 3.");
#endif
    }
	if (fmc.t4buf) {
        if (fmc.t4bufID != 0){ //was allocated with Buffer Proc
			UnlockBuffer(fmc.t4bufID);
			FreeBuffer(fmc.t4bufID); 
		} else
			free (fmc.t4buf);

		fmc.t4bufID = 0;
        fmc.t4buf = NULL;
#if 0
        Info("Freed tile buffer 4.");
#endif
    }
    } //scope
} //scope

if (fmc.doingProxy) {
    //Only one tile when doing proxy, so we can call the OnFilterEnd
    //handler here.

    int res = call_FFP(FFP_OnFilterEnd,(INT_PTR)&fmc);//OnFilterEnd(&fmc);

}//doingProxy

reentry_depth--;
} /*DoFilterRect*/

int copyInputToOutput()
{

	//return true;

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
	
    
			fmc.writeAlpha = 
#ifndef DONT_USE_MASK
                !gStuff->haveMask &&
#endif
                !gNoMask;


	//Unconditionally copy input tile to output...
        //Do this with memcpy instead????
        fmc.y = fmc.y_start;
        for (fmc.row = 0; fmc.row < fmc.rows; fmc.row++)
        {
            // No progress indicator -- but do testAbort()
            if (testAbort()) return false; //goto egress;
            /*else if (fmc.doingProxy) {
                if (updateProgress(fmc.row, fmc.rows)) goto egress;
            }
            else {
                if (updateProgress(fmc.y - Y0, fmc.Y - Y0)) goto egress;
                fmc.y++;
            }*/

            fmc.srcPtr = (unsigned8 *) MyAdjustedInData + (fmc.row * fmc.srcRowDelta);
            fmc.dstPtr = (unsigned8 *) gStuff->outData + (fmc.row * fmc.dstRowDelta);
            fmc.mskPtr = fmc.mskData + (fmc.row * fmc.mskRowDelta);

            //Copy over all planes and possibly the alpha...

            /*if (gStuff->imageMode == plugInModeRGB48 ||
                gStuff->imageMode == plugInModeGray16|| 
                gStuff->imageMode == plugInModeLab48 || 
                gStuff->imageMode == plugInModeCMYK64 ||
                gStuff->imageMode == plugInModeDeepMultichannel ||
                gStuff->imageMode == plugInModeDuotone16){ */
            
            if (gStuff->imageMode > 9) {

                // pixel depth = 16
#define srcPtr16 ((unsigned16 *)fmc.srcPtr)
#define dstPtr16 ((unsigned16 *)fmc.dstPtr)
#define mskPtr16 ((unsigned16 *)fmc.mskPtr)

                for (fmc.col = 0; fmc.col < fmc.columns; fmc.col++)
                {
                    int plane;
                    for (plane = 0; plane < gStuff->planes; plane++)
                       dstPtr16[plane] = srcPtr16[plane];

#if 0
                    // No alpha in 16-bit mode?
                    if (fmc.writeAlpha) {   //redundant???
                        dstPtr16[3] = mskVal(fmc.row, 0, fmc.col, 0);
                    }
#endif

                    //srcPtr16 += fmc.srcColDelta;
                    //dstPtr16 += fmc.dstColDelta;
                    fmc.srcPtr = fmc.srcPtr + 2 * fmc.srcColDelta;
                    fmc.dstPtr = fmc.dstPtr + 2 * fmc.dstColDelta;

                    ///mskPtr16 += fmc.mskColDelta;    //wrong????

                } /* for fmc.col */
            }
            else {
                // pixel depth = 8
                for (fmc.col = 0; fmc.col < fmc.columns; fmc.col++)
                {
                    int plane;
                    for (plane = 0; plane < gStuff->planes; plane++)
                       fmc.dstPtr[plane] = fmc.srcPtr[plane];
#if 0
                    if (fmc.writeAlpha) {   //redundant???
                        fmc.dstPtr[3] = mskVal(fmc.row, 0, fmc.col, 0);
                    }
#endif
                    fmc.srcPtr += fmc.srcColDelta;
                    fmc.dstPtr += fmc.dstColDelta;
                    fmc.mskPtr += fmc.mskColDelta;  //wrong????
                } /* for fmc.col */
            } /* if 8-bit depth */
        } /* for fmc.row */

		return true;
}




/******************************************************/

#if 0

static int fm_doForEveryRow(void) {
    int res;

    // re-initialize in case ForEveryTile() altered these values...
    fmc.y = fmc.y_start;

    for (fmc.row = 0; fmc.row < fmc.rows; fmc.row++, fmc.y++)
    {
        if (fmc.doingProxy) {
            if (res = updateProgress(fmc.row, fmc.rows)) return res;
        }
        else {
            if (res = updateProgress(fmc.y - Y0, fmc.Y - Y0)) return res;
        }

        fmc.srcPtr = (unsigned8 *) MyAdjustedInData + (fmc.row * fmc.srcRowDelta);
        fmc.dstPtr = (unsigned8 *) gStuff->outData + (fmc.row * fmc.dstRowDelta);
        fmc.mskPtr = fmc.mskData + (fmc.row * fmc.mskRowDelta);

        fmc.x = fmc.x_start;
        fmc.col = 0;        //redundant??? DoOneRow sets it also.
        fmc.writeAlpha = 
#ifndef DONT_USE_MASK
                !gStuff->haveMask &&
#endif
            !gNoMask;
        // call ForEveryRow() here.
        // or call it from within DoOneRow???
        // Then call DoOneRow to drive the ForEveryPixel code...
        //execute the compiled code...
        call_FFP(FFP_ForEveryRow,(INT_PTR)gFmcp); //ForEveryRow(gFmcp);

    } /* for fmc.row */
    return 0;   //success
} /*fm_doForEveryRow*/

#endif

/******************************************************/

/* DoFilterRect for non-advanceState */


void DoFilterRectNoAdvanceState (GPtr globals)
{
#ifndef NOTRACE
    Trace (globals, "DoFilterRectNoAdvanceState:\n"
                    " Didn't expect to come here!");
#endif
}


/*****************************************************************************/

/* If we do all of the filtering during the start phase, the continue
   phase is negligible. */

/* Otherwise (no AdvanceState) Filters the area and requests the next area. */


void DoContinue (GPtr globals)
{
    
    if (TestAbort ())
    {
        gResult = 1;
        return;
    }
        
    if (!WarnAdvanceStateAvailable())
    {
        DoFilterRectNoAdvanceState (globals);
    
        if (!DoNextRect (globals))
            {
#if BIGDOCUMENT
				PISetRect32(&fmc.inRect32, 0, 0, 0, 0);
                PISetRect32(&fmc.outRect32, 0, 0, 0, 0);
				SetOutRect(fmc.inRect32);
				SetOutRect(fmc.outRect32);
#else
                PISetRect (&gStuff->inRect, 0, 0, 0, 0);
                PISetRect (&gStuff->outRect, 0, 0, 0, 0);
#endif
            }
    }
    else
    {
#if BIGDOCUMENT
		PISetRect32(&fmc.inRect32, 0, 0, 0, 0);
        PISetRect32(&fmc.outRect32, 0, 0, 0, 0);
		SetOutRect(fmc.inRect32);
		SetOutRect(fmc.outRect32);
#else
        PISetRect (&gStuff->inRect, 0, 0, 0, 0);
        PISetRect (&gStuff->outRect, 0, 0, 0, 0);
#endif
    }
}


/*****************************************************************************/

/* Requests first part of the image to be filtered. */

void DoInitialRect (GPtr globals)
{

#if BIGDOCUMENT
	fmc.filterRect32 = GetFilterRect();	//needed?
    fmc.inRect32        = fmc.filterRect32;
    fmc.inRect32.bottom = fmc.inRect32.top + 1;
	SetInRect(fmc.inRect32);		 
    fmc.outRect32 = fmc.inRect32;
	SetOutRect(fmc.outRect32);
#else
    gStuff->inRect        = gStuff->filterRect;
    gStuff->inRect.bottom = gStuff->inRect.top + 1;
    gStuff->outRect = gStuff->inRect;
#endif

    gStuff->inLoPlane = 0;
    gStuff->inHiPlane = gStuff->planes - 1;
 	   

    gStuff->outLoPlane = gStuff->inLoPlane;
    gStuff->outHiPlane = gStuff->inHiPlane;

}
/*****************************************************************************/

/* Request the next area. */

Boolean DoNextRect (GPtr globals)
{

#if BIGDOCUMENT
	fmc.inRect32 = GetInRect();
    fmc.inRect32.top    = fmc.inRect32.top    + 1;
    fmc.inRect32.bottom = fmc.inRect32.bottom + 1;
	SetOutRect(fmc.inRect32);
    
    fmc.outRect32 = fmc.inRect32;
	SetOutRect(fmc.outRect32);

    return fmc.inRect32.top < fmc.filterRect32.bottom;
#else
    gStuff->inRect.top    = gStuff->inRect.top    + 1;
    gStuff->inRect.bottom = gStuff->inRect.bottom + 1;   
    gStuff->outRect = gStuff->inRect;
    return gStuff->inRect.top < gStuff->filterRect.bottom;
#endif

}

/*****************************************************************************/

/* Do any necessary clean-up. */


void DoFinish (GPtr globals)
{

#ifndef NOTRACE
    Trace (globals, "DoFinish:\n"
                    " Nothing much to do!");
#endif

#ifdef SCRIPTABLE
    WriteScriptParameters();
#endif

	finalCleanup();

}

void finalCleanup(void){

	int i;

	OnFilterExit();

	
#if 0
	//Clean up threads
	terminateThread(0);
#endif

    // Kill any async sound that might still be running...
    //playSoundWave(0);

    //Added by Harald Heim
    //Delete Array if available
    for (i=0; i<N_ARRAYS; i++) freeArray(i);

#if defined(STYLER)
	//Delete image lists
	for (i=0; i<10; i++) freeImageList(i);
#endif    

    //Delete Fonts
    for (i=0; i<N_FONTS; i++){
            if (Font[i] != 0) DeleteObject(Font[i]);
    }

    //Added by Harald Heim
    //free distance buffer if necessary
    if (bufferID != 0){
        UnlockBuffer(bufferID);
        FreeBuffer(bufferID);
        bufferID = 0;
    }

#if defined(IMGSOURCE5) || defined(IMGSOURCE6) //|| definded(IMGSOURCE)
	exitIS();
#endif
	
}




/*****************************************************************************/
/* Some utility functions to support proxies in dialog */

//long long2fixed(long value) {
//    return value << 16;
//}


/*****************************************************************************/

void scaleRect(Rect *l, long n, long d) {					//What if coords are negative???
    // Carefully control rounding so scaled Rect is always
    // a (scaled) subset of the original Rect (we assume the
    // original Rect has no negative coordinates -- otherwise
    // we need to be even more careful!).
    l->left = (int16)((l->left * n + d - 1) / d);   // round up
    l->top = (int16)((l->top * n + d - 1) / d);     // round up
    l->right = (int16)((l->right * n) / d);         // round down
    l->bottom = (int16)((l->bottom * n) / d);       // round down
}

void scaleRect32(VRect *l, long n, long d) {				//What if coords are negative???
    // Carefully control rounding so scaled VRect is always
    // a (scaled) subset of the original VRect (we assume the
    // original VRect has no negative coordinates -- otherwise
    // we need to be even more careful!).
    l->left = ((l->left * n + d - 1) / d);   // round up
    l->top = ((l->top * n + d - 1) / d);     // round up
    l->right = ((l->right * n) / d);         // round down
    l->bottom = ((l->bottom * n) / d);       // round down
}

void shrinkRect(Rect *l, long x, long y) {
    l->left += (int16)x;
    l->top += (int16)y;
    l->right -= (int16)x;
    l->bottom -= (int16)y;
}

void shrinkRect32(VRect *l, long x, long y) {
    l->left += x;
    l->top += y;
    l->right -= x;
    l->bottom -= y;
}
void copyRect(Rect *l, Rect *r) { //make this a macro???
    l->left = r->left;
    l->top = r->top;
    l->right = r->right;
    l->bottom = r->bottom;
}

void copyVRect(VRect *l, const VRect *r)
{
	l->left = r->left;
    l->top = r->top;
    l->right = r->right;
    l->bottom = r->bottom;
}

void copyRect32(VRect *l, const VRect *r) {	//make this a macro???
	*l = *r;
}

void PISetRect32 (VRect *rect, int left, int top, int right, int bottom)
{
	rect->left = left;
	rect->top = top;
	rect->right = right;
	rect->bottom = bottom;
}


/*****************************************************************************/

/* Test for the display pixels proc and if it isn't available, show an alert. */

Boolean CheckProxiesSupported (DisplayPixelsProc displayPixels)
    {
    
    Boolean available = (displayPixels != NULL);
    
    if (!available)
        ShowAlert (16999);
        
    return available;
    
    }

/***********************************************************************************/

/* Computes the scaled down rectangle and the scale factor for the proxy */

#if BIGDOCUMENT
void CalcProxyScaleFactor (const VRect *filterRect, Rect *proxyRect, long *scaleFactor)
#else
void CalcProxyScaleFactor (const Rect *filterRect, Rect *proxyRect, long *scaleFactor)
#endif
{

#if BIGDOCUMENT
    int filterHeight;
    int filterWidth;
    int itemHeight;
    int itemWidth;
    VPoint fraction;
#else
    short filterHeight;
    short filterWidth;
    short itemHeight;
    short itemWidth;
    Point fraction;
#endif

    shrinkRect (proxyRect, 1, 1);        // for the border

    filterHeight = filterRect->bottom - filterRect->top;
    filterWidth  = filterRect->right  - filterRect->left;
    
    itemHeight = proxyRect->bottom - proxyRect->top;
    itemWidth  = proxyRect->right  - proxyRect->left;
    
    if (itemHeight > filterHeight)
        itemHeight = filterHeight;
        
    if (itemWidth > filterWidth)
        itemWidth = filterWidth;
    
    fraction.h = (filterWidth + itemWidth - 1) / itemWidth;
    fraction.v = (filterHeight + itemHeight - 1) / itemHeight;

#if 1
    if (fraction.h > fraction.v) 
        *scaleFactor = fraction.h;
    else
        *scaleFactor = fraction.v;
#else
	*scaleFactor = max(fraction.h, fraction.v);
#endif
					   
	
//possibly the simplest implementation??
//*scaleFactor = min(*scaleFactor, min(filterHeight, filterWidth));

#if BIGDOCUMENT   
	if (filterHeight/(*scaleFactor)==0 || filterWidth/(*scaleFactor)==0){
		int c;
		for (c=*scaleFactor; c>=1; c--){
			if (filterHeight/c>0 && filterWidth/c>0) {
				*scaleFactor = c;
				break;
			}
		}
	}
#else
	//For very thin images
	if (filterHeight/(short)scaleFactor==0 || filterWidth/(short)scaleFactor==0){
	//if (filterHeight/(short)*scaleFactor==0 || filterWidth/(short)*scaleFactor==0){ //Bug Fix
		short c;
		for (c=(short)*scaleFactor; c>=1; c--){
			if (filterHeight/c>0 && filterWidth/c>0) {
				*scaleFactor = (long) c;
				break;
			}
		}
	}
#endif


    //copyRect (proxyRect, filterRect); 
    //scaleRect (proxyRect, 1, *scaleFactor);

}

