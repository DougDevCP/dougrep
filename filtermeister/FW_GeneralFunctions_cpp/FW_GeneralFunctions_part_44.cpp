		return 0;
}

INT_PTR getBufferAddress(int nr){

	if (nr == 0) //Source Buffer
		return (INT_PTR)MyAdjustedInData;
	else if (nr == 1) //T1
		return (INT_PTR)fmc.tbuf;
	else if (nr == 2) //T2
		return (INT_PTR)fmc.t2buf;
	else if (nr == 3) //Output Buffer
		return (INT_PTR)gStuff->outData;
	else if (nr == 4) //T3
		return (INT_PTR)fmc.t3buf;
	else if (nr == 5) //T4
		return (INT_PTR)fmc.t4buf;
	else
		return 0;
}


int setCtlScripting (int n, int state)
{
    
    if ( (n < 0 || n >= N_CTLS || !gParams->ctl[n].inuse) ) {
        return FALSE;
    }

    if (state != 0) state = -1;
    gParams->ctl[n].scripted = state;

    return true;
}


int checkScriptVal(int n)
{
    
    if (n < 0 || n >= N_SCRIPTITEMS) {
        return false;
    }
#ifdef SCRIPTABLE
    return (int)ScriptParamUsed[n];
#else
    return 0;
#endif

}

int enableScriptVal(int n, int state)
{
    
    if (n < 0 || n >= N_SCRIPTITEMS) {
        return false;
    }

    if (state !=0 ) state = -1;
#ifdef SCRIPTABLE
    ScriptParamUsed[n] = state;
#endif

    return true;
}

int getScriptVal(int n)
{
    
    if (n < 0 || n >= N_SCRIPTITEMS) {
        return 0;
    }
#ifdef SCRIPTABLE
    return ScriptParam[n];
#else
    return 0;
#endif

}

int setScriptVal(int n, int val)
{
    
    if (n < 0 || n >= N_SCRIPTITEMS) {
        return false;
    }
#ifdef SCRIPTABLE
    ScriptParam[n] = val;
#endif
    return true;
}


INT_PTR getDialogHandle(void)
{
    return (INT_PTR)fmc.hDlg;
}

INT_PTR getCtlHandle(int n)
{
    if ( (n < 0 || n >= N_CTLS || !gParams->ctl[n].inuse) ) {
        return 0;
    }

    return (INT_PTR)gParams->ctl[n].hCtl;
}



#if IMGSOURCE6
	#include "../../ImgSource6.h"
#endif


#ifdef FMDIB
    //Martijn's setPixel functions
    #include "fmdib.h"
#elif defined(GDIPLUS)
	#include "graphicsplus_fml.h"
#endif











//*******************************************************



int cnvX(int k, int off, int d,
                   INT_PTR pGetf, int x, int y, int z)
{
    int i;
    int sum = 0;
    int n = k*2 + 1;

#if 0
    //check for valid indexing into cell[]...
    if (n + off > N_CELLS) {
        return -1;  //invalid
    }
    else if (n + off < 0) {
        return -1;  //invalid
    }
#endif

#if 0
    if (d == 0)
    {   // default divisor is sum of weights...
        for (i = 0; i < n; i++) {
            d += fmc.cell[(off + i) & (N_CELLS-1)];
        }
        if (d == 0)
            return -1; // give up if divisor still 0
    }
#endif

    for (i = 0; i < n; i++) {
        sum += fmc.cell[(off + i) & (N_CELLS-1)] * ((fmf3)pGetf)(x - k + i, y, z);
    }
    return sum / d;
} /*fm_cnvX*/

int cnvY(int k, int off, int d,
                   INT_PTR pGetf, int x, int y, int z)
{
    int i;
    int sum = 0;
    int n = k*2 + 1;

#if 0
    //check for valid indexing into cell[]...
    if (n + off > N_CELLS) {
        return -1;  //invalid
    }
    else if (n + off < 0) {
        return -1;  //invalid
    }
#endif

#if 0
    if (d == 0)
    {   // default divisor is sum of weights...
        for (i = 0; i < n; i++) {
            d += fmc.cell[(off + i) & (N_CELLS-1)];
        }
        if (d == 0)
            return -1; // give up if divisor still 0
    }
#endif

    for (i = 0; i < n; i++) {
        sum += fmc.cell[(off + i) & (N_CELLS-1)] * ((fmf3)pGetf)(x, y - k + i, z);
    }
    return sum / d;
} /*fm_cnvY*/



int fm_abort(void) {
    
    //RaiseException(STATUS_CONTROL_C_EXIT,         // exception code 
    //    0,                    // continuable exception 
    //    0, NULL);             // no arguments 

    gResult = userCanceledErr;

    return 0;  //no return expected
} //fm_abort



int testAbort(void) {
    if (fmc.doingProxy) {
        //check for any character typed...
        MSG msg;
        if (PeekMessage(
            &msg,   // pointer to structure for message
            NULL,   // handle to window (NULL => any window belonging to this thread)
            WM_KEYDOWN,     // first message
            WM_KEYDOWN,     // last message
            PM_REMOVE       // removal flags
           ))
        { 
            if (msg.wParam == VK_ESCAPE) {
#if 0
                // User pressed escape -- ask if he really wants
                // to abort.
                if (YesNo("Abort filter operation?") == IDYES)
                    return userCanceledErr;
                else
                    return 0;
#else
                // Unconditional abort.
                return userCanceledErr;
#endif
            }
            else if (msg.wParam == VK_PAUSE) {
                // User pressed pause -- ask if he wants
                // to continue or abort.
#if 0
                if (Warn("Filter operation paused...\n\n"
                         "Select OK to continue\n"
                         "or Cancel to abort.") == IDCANCEL)
#else
                if (msgBox(MB_ICONWARNING|MB_YESNO,
                           "Pause",//"FilterMeister Pause",
                           "Continue?") == IDNO)
#endif
                    return userCanceledErr;
                else
                    return 0;
            }
#if 0
            else if (msg.wParam == VK_APPS) {
                // Context menu key ==> unconditional abort...
                return userCanceledErr;
            }
#endif
            // Ignore any other key-down
            return 0;
        };
    }

#ifndef APP
    else {
        // Running filter on main image.
        if (TestAbort ())
        {
            gResult = userCanceledErr;
            //longjmp()
        }
    }
#else
    
    if (gResult == userCanceledErr) return userCanceledErr;

#endif

    return gResult;
} //fm_testAbort



void setProgressMain(int val){
#if SKIN
	if (isSkinActive() && isSkinCtl(CTL_PROGRESS)){
		gParams->ctl[CTL_PROGRESS].val = val;
		skinDraw(CTL_PROGRESS,1);
	} else 
#endif
		SendMessage(GetDlgItem(MyHDlg, IDC_PROGRESS1), PBM_SETPOS, val, 0);
}

