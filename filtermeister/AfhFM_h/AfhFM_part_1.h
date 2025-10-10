///////////////////////////////////////////////////////////////////////////////////
//  File: AfhFM.h
//
//  Main header file with general FilterMeisterVS definitions
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

#ifndef AFHFM_H_DEF
#define AFHFM_H_DEF



#ifdef __cplusplus
#define EXTERNC extern "C"
#else
#define EXTERNC
#endif


//#define _WIN32_WINNT 0x0400 //need Windows 4.0 or greater (for TryEnterCriticalSection, etc.)

//#define SKINFLICKERFREE 1
#include <windows.h>
#include <commctrl.h>       //for UDACCEL
// Note the two possible file name string formats.
#include "PIFilter.h"

//#include "FW_SpecialFunctions.h"
#include "FW_GeneralFunctions.h"

int copyInputToOutput();

extern int popupOffsetX, popupOffsetY;

#define MAXCPU 16

//Multithreading
extern struct MULTITHREAD
{
   int totalcpu;
   int progressCount;
   int progressTotal;
   int * buffer[6];
   int param[10];
   int width;
   int height;
   int bitDepth;
   LPTHREAD_START_ROUTINE threadFunction;
} multithread;

EXTERNC struct MULTITHREAD mt; //for doMultiThreading()

//struct MULTITHREAD mt2; //for createThreads()
//int * threadState;


//Key shortcuts
//HACCEL hAcceleratorTable;
//LPACCEL acceleratorArray;
//int acceleratorCount;


//Preset Dialog
extern struct PRESETSETTINGS
{
	char name[256];
	char folder[512];
	int group[27];
} presetsettings;
extern struct PRESETSETTINGS presetSettings;



/*****************************************************************************/

#if 0	//thread-safe
#define THREAD_LOCAL __declspec(thread)
#else
#define THREAD_LOCAL
#endif




/*****************************************************************************/

// Flag bit definitions for gParams->flags.

#define AFH_TRACE                   0x00000001
#define AFH_USE_MMX                 0x00000002
#define AFH_USE_ADVANCESTATE        0x00000004
#define AFH_DRAGGING_PROXY          0x00000008
#define AFH_USE_HOST_PADDING        0x00000010
#define AFH_INIT_OPTIMAL_ZOOM       0x00000020
#define AFH_PROXY_BIG_GULP          0x00000040
#define AFH_DRAG_FILTERED_OUTPUT    0x00000080
#define AFH_FLUSH_ADVANCESTATE      0x00000100
#define AFH_ZERO_OUTPUT_DRAG        0x00000200
#define AFH_SUPPORT_RGB             0x00000400
#define AFH_SUPPORT_GRAY            0x00000800
#define AFH_SUPPORT_CMYK            0x00001000
#define AFH_SUPPORT_LAB             0x00002000
#define AFH_SUPPORT_INDEXED         0x00004000
#define AFH_SUPPORT_BITMAP          0x00008000
#define AFH_SUPPORT_DUOTONE         0x00010000
#define AFH_SUPPORT_MULTI           0x00020000
#define AFH_OPTIMIZE_BEVEL          0x00040000
#define AFH_DISASSEMBLE             0x00080000
#define AFH_DUMP_SDL                0x00100000
#define AFH_PSEUDOCODE              0x00200000
#define AFH_USE_CMOV                0x00400000
#define AFH_USE_INLINES             0x00800000
#define AFH_IS_TILEABLE             0x01000000
#define AFH_EDITMODE                0x02000000
#define AFH_ADVANCED                0x04000000
//#define AFH_QUICK_TOGGLE            0x08000000

// Flag bit definitions for control properties.

#define CTP_TRACK                   0x00000001
#define CTP_READONLY                0x00000002
#define CTP_HORZ                    0x00000004
#define CTP_VERT                    0x00000008
#define CTP_ORIENT_MASK             (CTP_HORZ|CTP_VERT)
#define CTP_TOP                     0x00000010
#define CTP_BOTTOM                  0x00000020
#define CTP_LEFT                    0x00000040
#define CTP_RIGHT                   0x00000080
#define CTP_SIDE_MASK               (CTP_TOP|CTP_BOTTOM|CTP_LEFT|CTP_RIGHT)
#define CTP_EXTENDEDUI              0x00000100
#define CTP_MOUSEOVER               0x00000200
#define CTP_CONTEXTMENU             0x00000400
#define CTP_DRAWITEM                0x00000800
// Added by Ognen Genchev
#define CTP_RECTANGLE               0x00000801
#define CTP_ELLIPSE                 0x00000802
// end
#define CTP_MOUSEMOVE               0x00001000
#define CTP_PREVIEWDRAG             0x00002000


// Should add a version # to Parameters, and ignore old params if version #
// mismatch!!!

#define MAX_LABEL_SIZE 1023
#define MAX_TOOLTIP_SIZE 127

//following must be a power of 2 for index masking...
#define IS_POWER_OF_TWO(n) (((n) & -(n)) == (n))

#define MAX_SCALE_FACTOR 30000 //16

// Number of FM Arrays
#define N_ARRAYS 100
#define IS_OK_ARRAY_NR(nr) ((UINT_PTR)(nr) < N_ARRAYS)			//note uint also catches < 0
#define IS_ALLOCATED_ARRAY(nr) (IS_OK_ARRAY_NR(nr) && Array[nr])

//number of anonymous put/get cells
#define N_CELLS 1024

#if !IS_POWER_OF_TWO(N_CELLS)
#error N_CELLS must be a power of two!
#endif

//number of dialog controls
#if STYLER
	#define N_CTLS 400 //370
	#define N_FXCTLS 310 //280
#else
	#define N_CTLS 256
	#define N_FXCTLS 200
#endif

#define MAX_CURVEPOINTS 16

//#if !IS_POWER_OF_TWO(N_CTLS)
//#error N_CTLS must be a power of two!
//#endif

//predefined control indices
#define CTL_OK          (N_CTLS - 1)
#define CTL_CANCEL      (N_CTLS - 2)
#define CTL_EDIT        (N_CTLS - 3)
#define CTL_LOGO        (N_CTLS - 4)
#define CTL_PREVIEW     (N_CTLS - 5)
#define CTL_PROGRESS    (N_CTLS - 6)
#define CTL_HOST        (N_CTLS - 7)
#define CTL_ZOOM        (N_CTLS - 8)
#define CTL_FRAME       (N_CTLS - 9)
#define CTL_BACKGROUND  (N_CTLS - 10)

//last available user control index (with room to spare...)
#define CTL_LAST_USER   (N_CTLS - 11)

#if APP
	#define CTL_TOPMENU	(N_CTLS - 11)
	#define CTL_CLOSE	(N_CTLS - 12)
	#define CTL_BATCH	(N_CTLS - 13)
	#define CTL_INDIVIDUAL	(N_CTLS - 14)	
#endif


//predefined control IDs
#define IDX_OK          (IDC_CTLBASE + CTL_OK)
#define IDX_CANCEL      (IDC_CTLBASE + CTL_CANCEL)
#define IDX_EDIT        (IDC_CTLBASE + CTL_EDIT)
#define IDX_LOGO        (IDC_CTLBASE + CTL_LOGO)
#define IDX_PREVIEW     (IDC_CTLBASE + CTL_PREVIEW)
#define IDX_PROGRESS    (IDC_CTLBASE + CTL_PROGRESS)
#define IDX_HOST        (IDC_CTLBASE + CTL_HOST)
#define IDX_ZOOM        (IDC_CTLBASE + CTL_ZOOM)
#define IDX_FRAME        (IDC_CTLBASE + CTL_FRAME)

#ifndef BS_TYPEMASK
#define BS_TYPEMASK 0x0000000F  //missing from some versions of <Winuser.h>!!!
#endif
#define CBS_TYPEMASK 0x00000003

#ifndef TCS_FLATBUTTONS
#define TCS_FLATBUTTONS 0x0008  //missing from some versions of <commctrl.h>
#endif

// Added by Ognen Genchev
EXTERNC WNDPROC defWndProc;

EXTERNC HBITMAP hBitmapThumb[2];
EXTERNC HBITMAP hBitmapChannel[2];
EXTERNC BITMAP bm;

// ======================== DialConfig (for ctl[] and CreateCustomDialControl) ========================
typedef struct DialConfig {
    HWND hwnd;
    int index;
    float angle;
    BOOL dragging;

    int x, y, size;

    int dialRadius;
    int tickLength;
    int ellipseThickness;
    int tickThickness;
    int dotRadius;

    COLORREF dotColor;
    COLORREF tickColor;
    COLORREF ellipseColor;
    COLORREF backgroundColor;

    WCHAR fontName[32];
} DialConfig;

// Create a custom dial control inside a dialog
EXTERNC HWND CreateCustomDialControl(int x, int y, int size, const DialConfig* cfg);
// end

// Control classes...
typedef enum {
    CC_UNUSED,
    CC_STANDARD,
    CC_SCROLLBAR,
    CC_TRACKBAR,
    CC_SPINNER,
    CC_UPDOWN,
    CC_PUSHBUTTON,
    CC_CHECKBOX,
    CC_RADIOBUTTON,
    CC_GROUPBOX,
    CC_OWNERDRAW,
    CC_LISTBOX,
    CC_COMBOBOX,
    CC_SLIDER,
    CC_PROGRESSBAR,
    CC_EDIT,
    CC_STATICTEXT,
    CC_FRAME,
    CC_RECT,
    CC_IMAGE,
    CC_BITMAP,
    CC_ICON,
    CC_METAFILE,
    CC_TOOLTIP,
    CC_ANIMATION,
    CC_PREVIEW,
    CC_ZOOM,
	CC_TAB,
    CC_LISTBAR,
	CC_LISTVIEW,
	CC_TOOLBAR,
} CTLCLASS;

// Default control actions...