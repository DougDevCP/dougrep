#define COMMANDID(X)			LOWORD(X)
#define COMMANDWND(X)			(HWND)(UINT)lParam

#ifdef WIN32
#define COMMANDCMD(X, Y)	HIWORD(X)
#define huge
#else
#define COMMANDCMD(X, Y)	HIWORD(Y)
#endif


