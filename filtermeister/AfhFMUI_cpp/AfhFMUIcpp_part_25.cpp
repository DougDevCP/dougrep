			DispatchMessage(&msg);
		}

	}


	if (hDlg) {
		DestroyWindow(hDlg);
		hDlg = 0;
	}
	
#else
	
	INT_PTR nResult;
	nResult = DialogBoxParam(
		               hDllInstance,
                       (LPSTR)"FILTERPARAM",
                       hParent, //platform,//(HWND)platform->hwnd,
                       (DLGPROC)FilterDialogProc,
                       (LPARAM)globals);

    if  (nResult == IDCANCEL || nResult == -1)
        gResult = 1; // don't continue

#endif

}   

/*
void PromptUserForInputNoProxy (GPtr globals)
{

    int    nResult;
    PlatformData *platform;

    platform = ((FilterRecordPtr) gStuff)->platformData;

    // Query the user for parameters
    nResult = DialogBoxParam(
		               hDllInstance,
                       (LPSTR)"FILTERPARAM",
                       (HWND)platform->hwnd,
                       (DLGPROC)FilterDialogProc,
                       (LPARAM)globals);
}   
*/

/****************************************************************************/
/* Example for ShowAlert() function which takes a string ID as parameter    */
/* and displays a message box                                               */
/****************************************************************************/

short ShowAlert (short stringID)
{
    char szMessage[256];
    char szTitle[128];

    LoadString((HINSTANCE)hDllInstance, stringID, szMessage, sizeof szMessage);
    LoadString((HINSTANCE)hDllInstance, 2, szTitle, sizeof szTitle);
    return  MessageBox(MyHDlg, szMessage, szTitle, MB_OK | MB_ICONHAND | MB_TASKMODAL);

}

/*****************************************************************************/

#ifndef NOTRACE
void Trace (GPtr globals, const char *szFormat, ...)
{
    char szBuffer[1024];
    char *pArgs;
    int nResult=0; //Added to fix compiler error
//#if 0
    if ((gParams->flags & (AFH_TRACE|AFH_DRAGGING_PROXY)) == AFH_TRACE)
//#endif
    {   // Trace if Trace flag set but not while dragging proxy...

    if  (nResult == IDCANCEL || nResult == -1)
        gResult = 1; // don't continue

        pArgs = (char *) &szFormat + sizeof(szFormat);
        wvsprintf(szBuffer, szFormat, pArgs);
        MessageBox(MyHDlg, szBuffer, "Filter trace", MB_OK | MB_TASKMODAL);
    }
} /*Trace*/
#endif



#ifndef APP

/*****************************************************************************/

/* Initialization and termination code for window's dlls. */
// Every 32-Bit DLL has an entry point DLLInit (It is actually called DllMain()...)

#ifndef __cplusplus

#ifdef _MT 
    //Needed for static linking of the C Runtime
    BOOL WINAPI _CRT_INIT(HANDLE, DWORD, LPVOID);
#endif

BOOL APIENTRY DLLInit(HANDLE hInstance, DWORD fdwReason, LPVOID lpReserved)
{

    if (fdwReason == DLL_PROCESS_ATTACH) 
        hDllInstance = hInstance;
    
#ifdef _MT    
    //Needed for static linking of the C Runtime
    if (fdwReason == DLL_PROCESS_ATTACH || fdwReason == DLL_THREAD_ATTACH || fdwReason == DLL_PROCESS_DETACH || fdwReason == DLL_THREAD_DETACH){
        if (!_CRT_INIT(hInstance, fdwReason, lpReserved)) return(FALSE);
    }
#endif

    return TRUE;   // Indicate that the DLL was initialized successfully.
}

#endif

/*****************************************************************************/

#endif
