    /*fmc.x_start = gStuff->inRect.left;
    fmc.y_start = gStuff->inRect.top;
    fmc.x_end = gStuff->inRect.right;
    fmc.y_end = gStuff->inRect.bottom;*/
    fmc.x_start = 0;
    fmc.y_start = 0;
    
#if BIGDOCUMENT		   
        fmc.x_end = fmc.inRect32.right - fmc.inRect32.left;
        fmc.y_end = fmc.inRect32.bottom - fmc.inRect32.top;
#else    
        fmc.x_end = gStuff->inRect.right - gStuff->inRect.left;
        fmc.y_end = gStuff->inRect.bottom - gStuff->inRect.top;
#endif

    fmc.X = fmc.xmax = fmc.x_end - fmc.x_start;
    fmc.Y = fmc.ymax = fmc.y_end - fmc.y_start;

    return true;

} /*fm_requestRect*/


#endif



//#if NSSOFTENER2

/********************************************************************/
/*                 DLL Access Routines                              */
/*                                                                  */
/*  1. Original code contributed by Florian Xhumari, as transmitted */
/*     by Harald Heim.                                              */
/*  2. Remove commented-out code (Alex Hunter).                     */
/*                                                                  */
/********************************************************************/


typedef void (*FMLIB_FN)();


INT_PTR loadLib(INT_PTR libname) 
{
    HINSTANCE retval;
	
	if (!strrchr((char *)libname, 0x5C)){
		char string[512];

		strcpy(string, fmc.filterInstallDir); //Always has an ending \ sign
		strcat(string,(char *)libname);
		retval = LoadLibrary((LPCTSTR)string);
		if (retval != 0) return (INT_PTR) retval;
	}

	return (INT_PTR)LoadLibrary((char *)libname); 
}


int freeLib(INT_PTR hinstLib)
{
	return FreeLibrary((HINSTANCE)hinstLib);
}


INT_PTR getLibFn(INT_PTR hinstLib, INT_PTR fnName)
{
	return (INT_PTR)GetProcAddress((HINSTANCE)hinstLib, (char *)fnName);
}


// Disable "no return value" warnings
#ifdef _WIN64
    #pragma warning(disable:4716)
#else
    #pragma warning(disable:4035)
#endif

#ifndef _WIN64
__declspec(naked)
#endif
int callLibFmc(int fn, ...)
{
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
     *      #define callLibraryDoubleFast   ((dfn1v)callLibraryFast)
	 */

#ifndef _WIN64

    __asm {
#if 0
        mov eax, [esp+4]        ; eax <- fn
        mov [esp+4], ebx        ; replace fn with ebx (fmcp) in arg list
        jmp eax                 ; jmp to fn, which returns to caller
#elif 1
        mov eax, [esp+4]        ; eax <- fn
        mov ebx, gFmcp          ; (In case we weren't called from FM-generated code!)
        mov [esp+4], ebx        ; replace fn with ebx (fmcp) in arg list
        jmp eax                 ; jmp to fn, which returns to caller
#elif 0
        mov eax, ebx            ; eax <- ebx (=fmcp)
        xchg eax, [esp+4]       ; swap with fn in arg list; eax now has fn  //don't use XCHG per Fog!!!
        jmp eax                 ; jmp to fn, which returns to caller
#else
        ; old version, no fmcp argument...
        jmp dword ptr [esp+4]   ; jump to the function; it will return to caller of current function
#endif
        //
        // return result will be in eax (for callLibraryIntFast) or
        // on top of NDP stack ST(0) (for callLIbraryDoubleFast) --
        // only the caller cares which.
	}
#endif//Win64
}

#if 0
////////////// TEST PATCH /////////////////////
     
           typedef int (*ifn1v) (int, ...);
           typedef double (*dfn1v) (int, ...);
           #define fm_callLibraryIntFast      ((ifn1v)fm_callLibraryFast)
           #define fm_callLibraryDoubleFast   ((dfn1v)fm_callLibraryFast)

void xyxxy() {
           int g_hMyDll = gFmcp->loadLibrary((INT_PTR)"MyDll");
           if (!g_hMyDll) MessageBeep(MB_OK);
           {
           int g_pfnMyForEveryTile = gFmcp->getLibraryFunction(g_hMyDll, (INT_PTR)"MyForEveryTile");
           if (!g_pfnMyForEveryTile) MessageBeep(MB_OK);
           {    
           int g_pfnMyFloat = gFmcp->getLibraryFunction(g_hMyDll, (INT_PTR)"MyFloat");
           if (!g_pfnMyFloat) MessageBeep(MB_OK);
           {
           int iRes = fm_callLibraryIntFast(g_pfnMyForEveryTile, 100, 200);
           double x1,x2,x3,x4,x5,x6,x7,x8,x9;
           double fRes = fm_callLibraryDoubleFast(g_pfnMyFloat, 10, 1.2, 3.14);

           x1 = sin(x1)+cos(x2)+sin(x3+x4)*cos(x5-x6);
           }}}
}
///////////////////////////////////
#endif

#ifndef _WIN64
__declspec(naked)
#endif
int callLib(int fn, ...)
{
	/**
	 * Calls a function that takes the same parameters as used
	 * to call this function with the exception of the first one, fn
	 * Example of called function: int myFunction(double a, double b) { return a + b; }
	 */

	/**
	 * Use all data in previous function's frame (including all arguments passed
	 * to the current function) as arguments to the function fn. Actually,
	 * not all data, but all data excluding last argument (fn).
	 *
	 * Then, call function fn and remove added arguments.
	 *
	 * It is assumed that called function follows CDECL (or STDCALL) conventions, in particular
	 * it preserves register ebx.
	 */

#ifndef _WIN64
	__asm {
        ; prolog
        push ebp            ; save must-save registers
        ;;push ebx          ; not used
        push esi
        push edi
        mov ebp, esp        ; our new stack frame

        dec ecx             ; upon entry, ecx contains # of actual args (including fn)
        shl ecx, 3          ; subtract the fn argument and convert to (worst case) arg byte count
        lea esi, [ebp+5*4]	; skip 5 dwords (pushed edi, esi, ebp, eip and first argument (fn))

		sub esp, ecx		; allocate space on stack
		mov edi, esp		; destination: end of added space in stack
		shr	ecx, 2			; will be moving dwords, so divide count by 4
		rep movs dword ptr [edi]	; copy argument list, excluding fn
		call dword ptr [ebp+4*4] ; call the function

		;;;add esp, XXX		; remove arguments (but STDCALL may have already removed them!!!)

        ; epilog
        mov esp, ebp        ; restore our stack frame
        pop edi             ; restore callee-save registers
        pop esi
        ;;pop ebx           ; wasn't used
        pop ebp             ; restore caller's stack frame
        ret                 ; caller will pop actual args

	}
#endif

}


#ifndef _WIN64
__declspec(naked)
#endif
int __cdecl getCpuReg(int nr)
{
    // Get value of specified CPU register:
    // nr: 0 = edi, 1 = esi, 2 = ebp, 3 = esp, 4 = ebx, 5 = edx, 6 = ecx, 7 = eax, 8 = eip
#ifndef _WIN64
    __asm {                 ;        +0    1    2        3    4    5    6    7    8    9
        pushad              ; esp -> edi, esi, ebp, old esp, ebx, edx, ecx, eax, eip, nr
        add dword ptr [esp+3*4], 2*4  ; correct saved esp for push nr, push eip
        mov eax, [esp+9*4]  ; eax <- nr
        and eax, 0x0f       ; prevent GPF on out-of-range arg.
        mov eax, [esp+eax*4]    ; load desired register
        add esp, 8*4        ; restore esp
        ret
    }
#endif
}

#ifndef _WIN64
__declspec(naked)
#endif
void __cdecl finit(void)
{
#ifndef _WIN64
    __asm {
        finit               ; init cw to 0x037f, sw to 0, tag word to 0xffff, rest to 0
        ret
    }
#endif
}

#ifndef _WIN64
__declspec(naked)
#endif
int __cdecl fstsw(void)
{
#ifndef _WIN64
    __asm {
        xor eax, eax        ; clear 32-bit eax
        fstsw ax            ; return 16-bit status word in ax
        ret
    }
#endif
}

#ifndef _WIN64
__declspec(naked)
#endif
int __cdecl fstcw(void)
{
#ifndef _WIN64
    __asm {
        push 0              ; clear a dword on the stack
        fstcw word ptr[esp] ; store control word in low 16 bits or dword
        pop eax             ; pop zero-extended control word into eax
        ret
    }
#endif
}

#ifndef _WIN64