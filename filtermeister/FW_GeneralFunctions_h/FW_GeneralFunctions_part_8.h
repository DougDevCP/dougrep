    EXTERNC int drawText (int x, int y, char* text);
#endif



#include <time.h>


EXTERNC int startClock();
EXTERNC int stopClock();

EXTERNC int convertColor(int16 sourceSpace, int resultSpace, FilterColor color);

EXTERNC int getProperty(int property, int notsupported);

EXTERNC int setCtlClass(int iCtl, int iClass, int absy, int height, int thumbSize);

EXTERNC int isValidFilename (char * filename);

EXTERNC int convertToFilename (char * filename);

EXTERNC int stripEndSpaces(char * string);

EXTERNC int toLowerCase(char * str);

EXTERNC int retrieveFilename(char * path, char * filename);
EXTERNC int retrieveFilenameNoExt(char * path, char * filename);
EXTERNC int retrieveFolder(char * path, char * folder);

EXTERNC int fileExists(char *fileName);

//int addKeyShortcut (int n, int key, int fVirt);

EXTERNC int getCtlItemString(int n, int item, char * string);

EXTERNC int gammaCtlVal(int iCtl, int val, int invert);

EXTERNC void drawPreviewColor(HDC hDC);
EXTERNC void drawSizeGrip();

EXTERNC void resetTabCtls(int iCtl);

EXTERNC void removeChar(char * str, char garbage);

EXTERNC int fm_isnan(double x);
EXTERNC int fm_isinf(double x);

EXTERNC int getSysDPI(double * factorX, double * factorY);
EXTERNC BOOL checkUIScaling();
EXTERNC BOOL isHDPIAware();

EXTERNC void deactivateFlicks(int n);

EXTERNC void setDialogTitle(char * title);

EXTERNC int evalZoomCombo(int n, int ctlPlus, int ctlMinus, int previous);
EXTERNC int evalZoomButtons(int n, int ctlCombo, int ctlPlus, int ctlMinus);

// Added by Ognen Genchev
EXTERNC int setCtlEditPos(int n, int x, int y);
EXTERNC int setCustomCtl(int n, int iName);
EXTERNC int setCtlInitVal(int n, int val);
EXTERNC int setFrameColor(int n, int color);
// EXTERNC struct DialConfig;  // forward declaration
// Create a custom dial control inside a dialog
EXTERNC double fclamp (double x, double minVal, double maxVal);
EXTERNC double fsmoothstep (double edge0, double edge1, double x);
// end