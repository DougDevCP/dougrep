fmf1dd math_cos = cos;
fmf1dd math_sin = sin;
fmf1dd math_tan = tan;

//Functions that collide with C
#if defined(HARRYSFILTERS) | defined (PLUGINGALAXY)
    #define sin(n) fm_sin(n)
    #define cos(n) fm_cos(n)
    #define tan(n) fm_tan(n)
	#define fsin(n) fm_fsin(n)
    #define fcos(n) fm_fcos(n)
	#define ftan(n) fm_ftan(n)
	#define ffsin(n) math_sin(n)
    #define ffcos(n) math_cos(n)
	#define fftan(n) math_tan(n)
	#define fc2d(x,y) fm_fc2d(x,y)
	#define fc2m(x,y) fm_fc2m(x,y)
	#define fr2x(d,m) fm_fr2x(d,m)
	#define fr2y(d,m) fm_fr2y(d,m)
#else
	#define fsin(n) sin(n)
	#define fcos(n) cos(n)
	#define ftan(n) tan(n)
#endif


#define abort() {/*fm_abort();*/return true;} //fm_abort()
#define snprintf _snprintf
#define sqr(a) fm_sqr(a)


//Faster access for get and put
//#define get(a) fmc->cell[a & (N_CELLS-1)]
//#define put(a,b) fmc->cell[b & (N_CELLS-1)] = a
#define get(a) fmc->cell[a]
#define put(a,b) fmc->cell[b]=a


#define _eval_FFP for (y= y_start; y < y_end; y++){ \
		          if(updateProgress(y_end*stepCount*currentPass+y, y_end*stepCount*totalPasses)) abort(); \
                  for (x = x_start; x < x_end; x++) {\
                        r = src(x,y,0); \
                        g = src(x,y,1); \
                        b = src(x,y,2); \

#define _eval_FFP_end pset(x,y,0, r); \
                      pset(x,y,1, g); \
                      pset(x,y,2, b); \
                      }}


#define _eval_FFP_A for (y= y_start; y < y_end; y++){ \
		          if(updateProgress(y_end*stepCount*currentPass+y, y_end*stepCount*totalPasses)) abort(); \
                  for (x = x_start; x < x_end; x++) {\
                        r = src(x,y,0); \
                        g = src(x,y,1); \
                        b = src(x,y,2); \
                        if(alphaChannel) a = src(x,y,3);

#define _eval_FFP_A_end pset(x,y,0, r); \
                      pset(x,y,1, g); \
                      pset(x,y,2, b); \
                      if(alphaChannel) pset(x,y,3, a); \
                      }}

#define R r
#define G g
#define B b
#define A a

//C macro collision
/* #undef INVALID_HANDLE_VALUE
#define INVALID_HANDLE_VALUE -1
#undef NULL
#define NULL 0 */

#define setCtlImage(a,b,c) setCtlImage(a,b,0x42)
#define strcpy(a,b) strcpy((LPTSTR)a,(LPCTSTR)b)

#define createPopupMenu (INT_PTR)CreatePopupMenu
#define createMenu (INT_PTR)CreateMenu
#define mkdir _mkdir


#define VK_0 0x30
#define VK_1 0x31
#define VK_2 0x32
#define VK_3 0x33
#define VK_4 0x34
#define VK_5 0x35
#define VK_6 0x36
#define VK_7 0x37
#define VK_8 0x38
#define VK_9 0x39
#define VK_A 0x41
#define VK_B 0x42
#define VK_C 0x43
#define VK_D 0x44
#define VK_E 0x45
#define VK_F 0x46
#define VK_G 0x47
#define VK_H 0x48
#define VK_I 0x49
#define VK_J 0x4a
#define VK_K 0x4b
#define VK_L 0x4c
#define VK_M 0x4d
#define VK_N 0x4e
#define VK_O 0x4f
#define VK_P 0x50
#define VK_Q 0x51
#define VK_R 0x52
#define VK_S 0x53
#define VK_T 0x54
#define VK_U 0x55
#define VK_V 0x56
#define VK_W 0x57
#define VK_X 0x58
#define VK_Y 0x59
#define VK_Z 0x5a


#define CSIDL_APPDATA 26


#define To8Bit(a) (imageMode>9?(int)(a/128.5):a)
//#define To8Bit(a) imageMode>9?(a>>7)|(a>>15):a
//#define To8BitIf(a) if(imageMode>9)a=(a>>7)|(a>>15);
//#define To8BitVal(a) (a>>7)|(a>>15)


