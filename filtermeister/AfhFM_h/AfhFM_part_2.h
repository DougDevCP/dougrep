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
