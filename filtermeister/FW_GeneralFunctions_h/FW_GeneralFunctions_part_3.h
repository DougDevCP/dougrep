
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