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
typedef enum {
    CA_NONE,
    CA_CANCEL,
    CA_APPLY,
    CA_PREVIEW,
    CA_EDIT,
    CA_ABOUT,
    CA_RESET,
    CA_SIZE
} CTLACTION;

// FM Event codes...
typedef enum {
    FME_UNKNOWN,
    FME_ZERODIVIDE,
    FME_DIVIDEOVERFLOW,
    FME_CLICKED,
    FME_DBLCLK,
    FME_PAGEUP,
    FME_PAGEDOWN,
    FME_LINEUP,
    FME_LINEDOWN,
    FME_MOUSEOVER,
    FME_MOUSEOUT,
    FME_MOUSEMOVE,
    FME_LEFTCLICKED_DOWN,
    FME_LEFTCLICKED_UP,
    FME_RIGHTCLICKED_DOWN,
    FME_RIGHTCLICKED_UP,
    FME_TIMER,
    FME_ZOOMCHANGED,
    FME_KEYDOWN,
    FME_KEYUP,
    FME_CHANGED,//FME_VALUECHANGED,
    FME_SIZE,
    FME_ENTERSIZE,
    FME_EXITSIZE,
    FME_DRAWITEM,
    FME_CUSTOMEVENT,
    FME_COMBO_DROPDOWN,
    FME_COMBO_CLOSEUP,
    FME_CONTEXTMENU,
    FME_SETFOCUS,//FME_SETEDITFOCUS,
    FME_KILLFOCUS,//FME_KILLEDITFOCUS,
    FME_PREVIEWDRAG,
	FME_CANCEL,
	FME_INIT,
    FME_READSCRIPT,
    FME_WRITESCRIPT,
    FME_MENUSELECT,
	FME_TABRESET,
	FME_MENUCLICKED,
	FME_DIALOGDISPLAYED,
    FME_RECTANGLE // Added by Ognen Genchev
} FME;


// Dialog background image modes...
typedef enum {
    DIM_EXACT=0,
    DIM_TILED=1,
    DIM_STRETCHED=2
} DLGIMAGEMODE;


typedef enum {
    FFP_CtlDef,
    FFP_OnCtl,
    FFP_OnWindowStart,
    FFP_OnFilterStart,
    FFP_ForEveryTile,
    FFP_OnFilterEnd
    //FFP_ForEveryRow
} FFP_HANDLER;

EXTERNC int call_FFP(FFP_HANDLER h, INT_PTR p1);


#define ANCHOR_LEFT		1
#define ANCHOR_RIGHT	2
#define ANCHOR_TOP		4
#define ANCHOR_BOTTOM	8
#define ANCHOR_HCENTER	16
#define ANCHOR_VCENTER	32


    /* Predefined global variables, saved across filter invocations */
typedef struct tagPREDEF_GLOBALS
{
    int i0, i1, i2, i3, i4, i5, i6, i7, i8, i9;    //WARNING: i0, i1 are Premiere vars!
    int j0, j1, j2, j3, j4, j5, j6, j7, j8, j9;
    int k0, k1, k2, k3, k4, k5, k6, k7, k8, k9;
    double x0, x1, x2, x3, x4, x5, x6, x7, x8, x9;
    double y0, y1, y2, y3, y4, y5, y6, y7, y8, y9;
    double z0, z1, z2, z3, z4, z5, z6, z7, z8, z9;
    char str0[512];
    char str1[512];
    char str2[512];
    char str3[512];
    char str4[512];
    char str5[512];
    char str6[512];
    char str7[512];
    char str8[512];
    char str9[512];
    char str10[512];
    char str11[512];
    char str12[512];
    char str13[512];
    char str14[512];
    char str15[512];
    char str16[512];
    char str17[512];
    char str18[512];
    char str19[512];
} PREDEF_GLOBALS;

typedef struct TParameters
{
    short version_major, version_minor;
    unsigned long flags;
    ///short bandWidth;
    short queryForParameters;
    struct
    {
        CTLCLASS ctlclass; //class not allowed in C++
        HWND hCtl;
        HWND hBuddy1, hBuddy2;
		HWND hSkin;
        // Added by Ognen Genchev
        int editXpos, editYpos, state;
        COLORREF shapeColor;
        // end
        int xPos, yPos;
		int defXPos, defYPos;        
        int width, height;
		int defwidth, defheight;
        int editwidth, editheight;
        // cancelVal added by Ognen Genchev
        int initVal, val, defval, minval, maxval, lineSize, pageSize, thumbSize, cancelVal;
        int selStart, selEnd;
        int style, styleEx;
		int buddy1Style, buddy1StyleEx;
		int buddy2Style, buddy2StyleEx;
        int properties;

        int divisor;
        int gamma;

        CTLACTION action;
        HBRUSH hBrush;     //background brush
        COLORREF bkColor;   //background color; -1=transparent
        COLORREF textColor;
		COLORREF buddyBkColor;
		COLORREF editColor;
		COLORREF editTextColor;
        HFONT hFont;
        int fontSize;
        int iParent, leftBuddy, rightBuddy;
        //int id;
#if PLUGINGALAXY3
		char label[4096];
#else
        char label[MAX_LABEL_SIZE+1];
#endif
        char label2[32];
        char tooltip[MAX_TOOLTIP_SIZE+1];
        char image[_MAX_PATH+1];
        int  imageType; // 'B' = bitmap, 'W' = Windows metafile, etc.
        char enabled;   //0=invisible/disabled, 1=visible/disabled, 3=visible/enabled
        char inuse;

		int theme;
		int tabControl;
		int tabSheet;
		int defTabControl;
		int defTabSheet;
        int mateControl;
		int anchor;
		int distWidth;
		int distHeight;
		
        int scripted;
		int data;

        union { //class-specific variants
            struct {    //standard scrollbar-specific
                int w2, h2;     //width and height of static text buddy (in pixels)
				int maxw2, maxh2; //maximum width and height
            } sb;
            struct {    //pushbutton-specific
                char pushState; //pushed...
            } pb;
            struct {    //trackbar-specific
                int ticFreq;
                int tics[8];        //trackbar
            } tb;
            struct {    //up-down-specific
                int nBase;      // for buddy
                int nInc;
                UDACCEL aAccel[4];  //up-down
            } ud;
            struct {    //tooltip-specific
                int iDelay;     //tooltip
            } tt;
            struct {    //image-specific
                        BITMAP bmInfo;
                        HDC hdcMem;
                        HDC hdcAnd;
                        HBITMAP hbm;
                        HBITMAP hbmOld;
                        HBITMAP hbmAnd;
                        HBITMAP hbmOldAnd;
            } im;
            // Added by Ognen Genchev
            // ======================== New Dial control settings ========================
            DialConfig dial;
            // end
        };
    } ctl[N_CTLS];

	int skinActive;
	int skinStyle;

    // Preserve dialog configuration
    BOOL     gInitialInvocation;
    BOOL     gCtlImagesSet;
    int      gImagesEmbedded;
    BOOL     gWavesSpecified;
    int      gWavesEmbedded;
    HRGN     gDialogRegion;
    int      gDialogStyle;
    int      gDialogInitialStyle;
    int      gDialogStyleEx;
    int      gDialogInitialStyleEx;
    int      gDialogDragMode;
    RECT     gDialogRect;
    RECT     gDialogRectMin;
    int      gDialogState;
    COLORREF gDialogGradientColor1;
    COLORREF gDialogGradientColor2;
    int      gDialogGradientDirection;
    int      gDialogImageMode;
    int      gDialogImageStretchMode;
    char    *gDialogImage_save;
    char     gDialogImageFileName[_MAX_PATH+1];
    char     gDialogTitle[MAX_LABEL_SIZE+1];
	int      gDialogTheme;
	//int      gDialogEvent;
    RECT     gEditorRect;
    HKEY     gRegRoot;
    char     gRegPath[MAX_PATH+1];

    int scaleLock;

    // Preserve the following initialized storage from the context record...
    int need_iuv;
    int need_d;
    int need_m;
    int need_tbuf;
    int need_t2buf;
	int need_t3buf;
	int need_t4buf;
    int needPremiere;
    int needPadding;
    int tileHeight;//bandWidth;
    int tileWidth;
#ifndef DONT_SAVE_GLOBALS
    /* Save the following predeclared global and user-declared global/static variables */
    PREDEF_GLOBALS globvars;     //renamed from 'globals' to 'globvars' to avoid lexical ambiguity with other 'globals'!!!
#endif //DONT_SAVE_GLOBALS
    /* Public FF+ built-in strings and arrays */
    char filterAuthorText[256];
    char filterCategoryText[256];
    char filterTitleText[256];
    char filterFilenameText[256];
    char filterCopyrightText[256];
    char filterDescriptionText[256];
    char filterOrganizationText[256];
    char filterURLText[256];
    char filterVersionText[256];
    char filterAboutText[256];      //temporary
    char filterUniqueID[37];

} TParameters, *PParameters, **HParameters;

/*****************************************************************************/

typedef struct Globals
{
    short result;
    Rect proxyRect;
    FilterRecord *filterParamBlock;
} Globals, *GPtr, **GHdl;

    /* Globals */
extern GPtr globals;
    
#define gResult ((*MyGlobals).result)

#ifndef APP
    #define gStuff  ((*MyGlobals).filterParamBlock)
#else
    FilterRecord *gStuff;
#endif

#define gProxyRect  ((*MyGlobals).proxyRect)

#ifndef APP
    #define gParams  ((PParameters) *gStuff->parameters)
#else
    TParameters *gParams;
#endif


/* Global variables */
EXTERNC GPtr /*const*/ MyGlobals;
extern void *MyAdjustedMaskData;
extern void *MyAdjustedInData;
extern HWND MyHDlg;    // For use with MessageBox() -- will default to NULL before dialog is init'ed.
extern HWND ghTT;      //tooltip control handle
extern HANDLE hDllInstance;
extern char *MDP_looking_for;

extern char    *gDialogImage;

extern char szTitleName[_MAX_FNAME + _MAX_EXT + 1];

extern int FM_is_busy;

#if STYLER
	#define N_SCRIPTITEMS N_FXCTLS 
#else
	#define N_SCRIPTITEMS 256
#endif

#ifdef SCRIPTABLE
    extern int ScriptParamRead;
	extern int ScriptParam[N_SCRIPTITEMS];
	extern BOOL ScriptParamUsed[N_SCRIPTITEMS];
	extern int ScriptProp[3];
	
	#if defined(HYPERTYLE) || defined(STYLER) || defined(PLUGINGALAXY3) || defined(LIGHTMACHINE) || defined(COLORWASHER)
		extern char * ScriptStrings;
		extern int N_ScriptStrings; 
	#endif
	#if defined(STYLER) || defined(PLUGINGALAXY3) || defined(LIGHTMACHINE) || defined(COLORWASHER)
		EXTERNC int writeScriptStrings();
		EXTERNC int readScriptStrings();
	#endif
#endif

extern int edgeMode;
extern int EditBoxUpdate;
extern int MouseOverWhenInvisible;
extern int ClickDrag;
extern POINT imageOffset;

extern BOOL fDragging;
extern BOOL fDrawing;
extern BOOL fSizing;
extern BOOL fScaling;
extern BOOL fPaint;
extern BOOL fMaximized;
extern BOOL fMinimized;
//extern BOOL fMoving;
extern BOOL fDialogDisplayed;



extern MINMAXINFO MaxMinInfo;
extern int sizeGrip;

extern int unlockedPreviewFrame;

extern HHOOK KeyHook;

typedef union UIF 
{
	unsigned int	i;
	float		f;
} UIF; 

//extern int cancelEventPerformed;


/* Function prototypes */
void DoAbout (GPtr globals, short selector);
void scaleRect(Rect *l, long n, long d);
void shrinkRect(Rect *l, long x, long y);
void copyRect(Rect *l, Rect *r);
void copyVRect(VRect *l, const VRect *r);
void PromptUserForInput (GPtr globals);
//void PromptUserForInputNoProxy (GPtr globals);

#if BIGDOCUMENT
void scaleRect32(VRect *l, long n, long d);
void PISetRect32 (VRect *rect, int left, int top, int right, int bottom);
void shrinkRect32(VRect *l, long x, long y);
void copyRect32(VRect *l, const VRect *r);
void CalcProxyScaleFactor (const VRect *filterRect, Rect *proxyRect, long *scaleFactor);
#else
void CalcProxyScaleFactor (const Rect *filterRect, Rect *proxyRect, long *scaleFactor);
#endif

void DoFilterRect (GPtr globals);
void UpdateProxy (HWND hDlg, BOOL bEraseBackground);
void ShowOutputBuffer( GPtr globals, HWND hDlg, BOOL fDragging);
void SetupFilterRecordForProxy (GPtr globals, long *scaleFactor, long prevScaleFactor,
                                POINT *offset);   
void StartWithAdvanceState (GPtr globals);

void StartNoAdvanceState (GPtr globals);
void DoInitialRect (GPtr globals);

void DoFilterRectNoAdvanceState (GPtr globals);

void subtractPadding (GPtr globals);
void addPadding (GPtr globals);

void createPredefinedCtls(void);
EXTERNC int doAction(CTLACTION action);

int isImageOpen();
int nameButtons(int locale);
int displayAppHelp(int n, int ctl, int locale);


void setScaleFactor(int n);
int sub_setZoom (HWND hDlg, int n, int m);

void GetProxyItemRect (HWND hDlg, Rect *proxyItemRect);
int sub_scrollPreview (HWND hDlg, int mode, int offsetX, int offsetY, int update);
int sub_setPreviewCursor (INT_PTR res);
INT_PTR sub_getPreviewCursor (void);
HRESULT setDefaultWindowTheme(HWND hwnd);

const char *FmSearchFile(const char *szFileName);


/* Windows Registry routines */
HKEY ROK(HKEY hKey, LPCTSTR lpSubKeyName);
HKEY RCK(HKEY hKey, LPCTSTR lpSubKeyName);
LONG RQV(HKEY hKey, LPCTSTR lpValueName, LPBYTE dataBuffer, DWORD cbData);
LONG RQB(HKEY hKey, LPCTSTR lpValueName, VOID *dataBuffer, DWORD cbData);
LONG RQD(HKEY hKey, LPCTSTR lpValueName, DWORD *pdwValue);
LONG RQS(HKEY hKey, LPCTSTR lpValueName, CHAR *szBuffer, DWORD cbMaxLen);
LONG RSB(HKEY hKey, LPCTSTR lpValueName, CONST VOID *dataBuffer, DWORD cbData);
LONG RSD(HKEY hKey, LPCTSTR lpValueName, DWORD dwValue);
LONG RSS(HKEY hKey, LPCTSTR lpValueName, LPCTSTR szBuffer);
LONG REK(HKEY hKey, DWORD dwIndex, LPTSTR lpName, DWORD cbName);
LONG REV(HKEY hKey, DWORD dwIndex, LPTSTR lpValueName, LPDWORD lpcbValueName,
         LPDWORD lpType, LPBYTE lpData, LPDWORD lpcbData);
LONG RDK(HKEY hKey, LPCTSTR lpSubKey);
LONG RDV(HKEY hKey, LPCTSTR lpValueName);


void Trace (GPtr globals, const char *szFormat, ...);

//long long2fixed(long value);

Boolean CheckProxiesSupported (DisplayPixelsProc displayPixels);
Boolean DoNextRect (GPtr globals);



LRESULT CALLBACK KeyHookProc( int code, WPARAM wParam, LPARAM lParam);
LRESULT CALLBACK LLKeyHookProc( int code, WPARAM wParam, LPARAM lParam);


#if BIGDOCUMENT
/*****************************************************************************/
/*  Following is a modifed version of FilterBigDocument.h
/*****************************************************************************/
//#ifndef __FilterBigDocument_H__
//#define __FilterBigDocument_H__

//#include "PIDefines.h"
//#include "PITypes.h"
//#include "PIFilter.h"

VPoint GetImageSize(void);
VRect GetFilterRect(void);
VRect GetInRect(void);
VRect GetOutRect(void);
VRect GetMaskRect(void);
VPoint GetFloatCoord(void);
VPoint GetWholeSize(void);

void SetInRect(VRect inRect);
void SetOutRect(VRect inRect);
void SetMaskRect(VRect inRect);

/* Globals */
extern FilterRecord *gFilterRecord;
extern BigDocumentStruct *gBigDocumentData;

//#endif
/*****************************************************************************/
/* End of modified FilterBigDocument.h
/*****************************************************************************/
#endif


#ifdef APP
    void addAppButtons(void);
	void addMenuBar(void);
    int openImage(char * file, int doNotAdd, int saveSettings, int loadSettings);
	void refreshPreview();
    int saveImage();
    int openListImage(void);
    void batchFiles(void);
    void addImageFile(char * file);
    void updateImageList(void);
    void removeAllImages(void);
    void removeImageList(int n, int openNew);
    void exitIS(void);
    void storeSettings(int index);
    void eraseAllSettings(void);
    int displayOptions(int onlyRAW, int override);
    int isRAWExt(char * filename);
    void addFilesTab(void);
    int convertTo8bit(int source, int dest);
	int displayProgressWindow (int nodelay);
    int performAppAction(int n, FME e, int previous, int ctlMsg, int ctlCode, BOOL fActionable, BOOL fProcessed, int overrideBusy);
	void populateFMC(int source, int dest);

    #define TEMPIMAGE 93
    #define TEMPIMAGE2 94
    #define IMAGE 95
    #define IMAGEOUT 96
    #define PREVIEW 97
    #define PREVIEWOUT 98
    
    typedef struct APPCONTEXT
    {
        HWND hProgress;
        HWND hBatch;
        
        LPTSTR strInputFiles;
        int inputTotal;
        int inputIndex;
        LPTSTR strRequestedFile;

        LPTSTR strOutputFiles;
        int outputTotal;

        int doOutput;
        int saveOption;
        int batchFormat;
        int batchQuality;
        int batchCodec;
        int batch8bit;
        char batchFolder[MAX_PATH];
        char batchSuffix[32];
        int batchUseSource;
        int batchNoEffect;
		int batchSRGB;
		int batchNoMeta;

        int filterIndividual;
        int * filterSettings;
        int * previewtabSettings;
        int ** spotSettings;
        int * spotSettingsSize;
        int * colormaskSettings;

        int * previewCoord;

        int optionsMode;

        int colorManagementOff;
        int colorMonitorProfile;
        int colorIntent;
        int colorBPC;

        int rawSizeCurrent;
        int rawQualityCurrent;
        int rawSizePreview;
        int rawSizeOutput;
        int rawQualityPreview;
        int rawQualityOutput;
        int rawLook;
        int rawWhiteBalance;
        int rawColorSpace;
        int rawFourColor;
        int rawDontDisplay;

        clock_t progressStartTime;
        int progressDisplayed;
		int doProgress;
        char progressMessage[255];

        int progressCurrent;
        int progressTotal;
        int progressTaskCurrent;
        int progressTaskTotal;
        int progressStartClock;

        char openFolder[MAX_PATH];
        //char saveFolder[MAX_PATH];

		char EXIFMake [256];
		char EXIFCamera [256];
		unsigned short EXIFISO;

        int imageOnStartup;
        
    } APPCONTEXT;

    APPCONTEXT app;


#endif

#if FOCALBLADE

    //Cache Values
    struct CACHEVAL
    {
       int cx;
       int cy;
       int cctl;
       int cctl2;
       int cscl;
    } cache_values;

    struct CACHEVAL cache[12];

#endif


struct FFFILTER {
	char category[256];
	char title[256];
	char author[256];
	char copyright[256];
	char filename[256];

	char relpath[256];

	char code[4][1024];
	char map[4][256];
	char ctl[8][256];
	int val[8];
	int def[8];

	BOOL supports16Bit;
	BOOL maxCompatibility;
	void * tree[4];
	//void * pcode[4];
};

extern BOOL mtAbortFlag;

extern BOOL UIScaling;

extern SPBasicSuite * sSPBasic;

EXTERNC void OnFilterExit();




#endif  // Closed ifndef AFHFM_H_DEF