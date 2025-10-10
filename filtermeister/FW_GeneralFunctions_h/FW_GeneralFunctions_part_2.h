

EXTERNC int fm_sqr(int x);

/***********************************************************************/
/* from: http://www.research.microsoft.com/~hollasch/cgindex/math/introot.html
/* Integer Square Root
/* 
/* Ben Discoe (rodent@netcom.COM), comp.graphics, 6 Feb 92 
/*
/***********************************************************************/
EXTERNC unsigned int isqrt(unsigned int v);




EXTERNC int fm_sin(int x);
EXTERNC int fm_cos(int x);

EXTERNC double fm_fcos(double x);
EXTERNC double fm_fsin(double x);
EXTERNC double fm_ftan(double x);

EXTERNC int fm_tan(int x);

EXTERNC int c2d(int x, int y);
EXTERNC int c2m(int x, int y);

EXTERNC double fm_fc2d(double x, double y);
EXTERNC double fm_fc2m(double x, double y);

EXTERNC int get(int i);

EXTERNC int put(int v, int i);

EXTERNC int cnv(int m11, int m12, int m13,
                  int m21, int m22, int m23,
                  int m31, int m32, int m33,
                  int d);


/********* FF+ built-in functions ***********/


EXTERNC INT_PTR pointer_to_buffer(int a, int x, int y, int z);


EXTERNC int phaseshift(int a, int b) ;

EXTERNC int blend(int a, int b, int z, int m, int r) ;

EXTERNC int contrast(int a, int b) ;

EXTERNC int posterize(int a, int b) ;
    
EXTERNC int saturation(int r, int g, int b, int z, int sat) ;

EXTERNC int msk(int x, int y);



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
EXTERNC int egw(int edge_a, int edge_b, int value);


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
EXTERNC int egm(int edge_a, int edge_b, int value);



//function added by Harald Heim on Feb 6, 2002
EXTERNC int gray(int r, int g, int b, int rweight, int gweight, int bweight);



//function added by Harald Heim on Feb 6, 2002
EXTERNC int rgb2iuv(int r, int g, int b, int z);

EXTERNC int iuv2rgb(int i, int u, int v, int z);

EXTERNC int rgb2cmyk(int r, int g, int b, int z);

EXTERNC int cmyk2rgb(int c, int m, int y, int k, int z);

EXTERNC int solarize(int a, int s);

EXTERNC int tricos(int x);

EXTERNC int tri(int x);

EXTERNC int sinbell(int x);

EXTERNC int grad2D(int x, int y, int X, int Y,int grad, int dist, int repeat);

EXTERNC int tone(int a, int h, int m, int d);

EXTERNC int xyzcnv(int x, int y, int z, int m11, int m12, int m13,
                  int m21, int m22, int m23, int m31, int m32, int m33,
                  int d);


EXTERNC int cell_initialize(int i);			//is sense reversed???


EXTERNC int set_edge_mode(int mode);

EXTERNC int set_bitdepth_mode(int mode);

EXTERNC int get_bitdepth_mode();

/// Use pluggable variants of psetp() instead of testing a flag at run-time!!!

EXTERNC int set_psetp_mode(int mode);

EXTERNC int iget(double x, double y, int z, int buffer, int mode);
EXTERNC int igetArrayEx(int nr, double x, double y, int z, int mode);

EXTERNC int bCircle(int x, int y, int centerx, int centery, int radius);
 

EXTERNC int bRect(int x, int y, int centerx, int centery, int radiusx, int radiusy);

EXTERNC int bRect2(int x, int y, int topx, int topy, int bottomx, int bottomy);

EXTERNC int bTriangle(int x, int y, int centerx, int centery, int radius);

EXTERNC int refreshWindow (void);
EXTERNC int refreshWindowNoErase (void);

EXTERNC int refreshRgn (INT_PTR Rgn);

EXTERNC int setCtlOrder (int n, int order);

EXTERNC int refreshCtl (int n, ... );

EXTERNC int refreshItems(int n);

EXTERNC int lockCtl (int n);
EXTERNC int unlockCtl (int n);

EXTERNC int lockWindow (int c);

EXTERNC int setZoom (int n, int m);

EXTERNC int setZoomEx (int n, int m);



EXTERNC int set_array_mode(int mode);

EXTERNC int freeHost (INT_PTR bufferid);

EXTERNC int allocArray (int nr, int X, int Y, int Z, int bytes);

EXTERNC int allocArrayPad (int nr, int X, int Y, int Z, int bytes, int padding);

EXTERNC int freeArray (int nr);

EXTERNC void swapArray(int nr, int nr2);

EXTERNC int getArray (int nr, int x, int y, int z);

EXTERNC int fast_getArray (int nr, int x, int y, int z);

EXTERNC int UNSAFE_getArray (int nr, int x, int y, int z);

EXTERNC int UNSAFE_getArrayNext8 (int nr);

EXTERNC int UNSAFE_getArrayNext16 (int nr);

EXTERNC int UNSAFE_getArrayNext32 (int nr);

EXTERNC double fgetArray (int nr, int x, int y, int z);

EXTERNC int putArray (int nr, int x, int y, int z, int val);

EXTERNC void fast_putArray (int nr, int x, int y, int z, int val);

EXTERNC int UNSAFE_putArray (int nr, int x, int y, int z, int val);

EXTERNC int fputArray (int nr, int x, int y, int z, double val);

EXTERNC int UNSAFE_fputArray (int nr, int x, int y, int z, double val);

EXTERNC int fputArrayLE (int nr, int x, int y, int z, double val);

EXTERNC int putArrayString (int nr, int x, LPSTR string);

EXTERNC char* getArrayString (int nr, int x);

EXTERNC unsigned8 gMskVal(int row, int rowOff, int col, int colOff);

EXTERNC int calcSBD(int bevWidth);

EXTERNC int freeSBD(void);

EXTERNC int getSBD(int j, int i);

EXTERNC int getSBDangle(int j, int i, int a10);

EXTERNC int getSBDX(int j, int i);

EXTERNC int getSBDY(int j, int i);



EXTERNC int getCtlPos (int n, int w);

EXTERNC int getCtlCoord (int n, int w);

EXTERNC int getPreviewCoordX (void);

EXTERNC int getPreviewCoordY (void);

EXTERNC int getAsyncKeyState (int t);

EXTERNC int getAsyncKeyStateF (int t);

EXTERNC int getAsyncKeyStateFC (int t);

EXTERNC int mouseOverWhenInvisible (int t);


EXTERNC int arrayExists (int nr);

EXTERNC int getArrayDim (int nr, int dim);

EXTERNC int copyArray (int src, int dest);

EXTERNC int fillArray (int nr, int val);

EXTERNC int ffillArray (int nr, double dval);



EXTERNC int ctlEnabled(int n);

EXTERNC int sub_ctlEnabledAs(int n, int depth);

EXTERNC int ctlEnabledAs(int n);

EXTERNC int rgb2lab(int r, int g, int b, int z);

EXTERNC int lab2rgb(int l, int a, int b, int z);


EXTERNC int setClickDrag(int b);

EXTERNC int setTimerEvent(int nr, int t, int state);

EXTERNC int getDisplaySettings(int s);

EXTERNC int scrollPreview(int mode, int ox, int oy, int update);

EXTERNC int rgb2hsl(int r, int g, int b, int z);
EXTERNC int hsl2rgb(int h, int s, int l, int z);