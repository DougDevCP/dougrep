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