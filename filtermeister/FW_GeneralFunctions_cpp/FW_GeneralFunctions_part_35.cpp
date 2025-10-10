                            &dwThread);

	if (hThread) { //Success
		ThreadCount++;
		ThreadHandle = realloc(ThreadHandle, ThreadCount*sizeof(int));
		ThreadHandle[ThreadCount-1]	= (INT_PTR)hThread;
		ThreadMem = realloc(ThreadMem, ThreadCount*sizeof(int));
		ThreadMem[ThreadCount-1] = (INT_PTR)pfmc_copy;
		return (INT_PTR)hThread;
	} else //Failure
		free(pfmc_copy);


    return false;
}


int waitForThread(INT_PTR hThread, int ms, int userinput){

	int retval = 0; 
	
	if (ThreadCount==0) return false; // No Threads available

	if (hThread==0){ //Wait for all threads
		
		if (ms==0){

			retval = WaitForMultipleObjects( 
						ThreadCount, 
						(HANDLE *)ThreadHandle, 
						TRUE, 
						0); // 0 => immediate return

		} else {

			MSG msg;
			int count=0;
		
			while (WaitForMultipleObjects( 
						ThreadCount, 
						(HANDLE *)ThreadHandle, 
						TRUE, 
						10) == WAIT_TIMEOUT){ //Use a 10 ms time-out

				if (ms!=INFINITE){
					count = count + 10;
					if (count >= ms) {
						retval = WAIT_TIMEOUT;
						break;
					}
				}
			
                if (userinput != -1){ //Use -1 to deactivate the message pump
				    //Flush key and mouse messages
				    if (userinput==0){
					    while( PeekMessage(&msg, NULL, WM_KEYFIRST, WM_KEYLAST, PM_REMOVE ) );
					    while( PeekMessage(&msg, NULL, WM_MOUSEFIRST, WM_MOUSELAST, PM_REMOVE ) );
				    }

				    //Message Pump
				    while (PeekMessage (&msg, NULL, 0, 0, PM_REMOVE)) {
					    TranslateMessage(&msg);
					    DispatchMessage(&msg);
				    }
                }
                
			}

		}

		if (retval != WAIT_TIMEOUT) {
			int i;

			for (i=0; i<ThreadCount; i++) 
				if (ThreadHandle[i]!=0) CloseHandle((HANDLE)ThreadHandle[i]);

			for (i=0; i<ThreadCount; i++) 
				if (ThreadMem[i]!=0) free ((void *)ThreadMem[i]);

			free(ThreadHandle);
			ThreadHandle = NULL;

			free(ThreadMem);
			ThreadMem = NULL;

			ThreadCount = 0;
		}

	} /*else { //Wait for specific thread
		
		retval = WaitForSingleObject( 
						(HANDLE)hThread, 
						(DWORD) ms );
		CloseHandle((HANDLE)hThread);
	}*/	


	if (retval==WAIT_TIMEOUT)
		return false;
	else
		return true;
}


int isThreadActive(INT_PTR hThread){

	LPDWORD lpExitCode=0;

	if (ThreadCount==0) return false;

	if (hThread==0){
		int i;
		for (i=0; i<ThreadCount; i++){ 
			if (GetExitCodeThread((HANDLE)ThreadHandle[i],lpExitCode)){	
				if (*lpExitCode == STILL_ACTIVE) return true;
			}
		}

		// Cleanup Threads
		for (i=0; i<ThreadCount; i++) 
			if (ThreadHandle[i]!=0) CloseHandle((HANDLE)ThreadHandle[i]);

		for (i=0; i<ThreadCount; i++) 
			if (ThreadMem[i]!=0) free ((void *)ThreadMem[i]);

		free(ThreadHandle);
		ThreadHandle = NULL;

		free(ThreadMem);
		ThreadMem = NULL;

		ThreadCount = 0;

	} else {
		if (GetExitCodeThread((HANDLE)hThread,lpExitCode)){	
			if (*lpExitCode == STILL_ACTIVE) return true;
		}
	}

	return false;
}


int getThreadRetVal(INT_PTR hThread){

	DWORD dwExitCode=0;

	if (ThreadCount==0) return false;

	if (hThread!=0){
		if (GetExitCodeThread((HANDLE)hThread,&dwExitCode)){	
			//if (dwExitCode != STILL_ACTIVE) 
				return dwExitCode;
		}
	}

	return 0;

}



int terminateThread(INT_PTR hThread){

	if (ThreadCount==0) return true;

	if (hThread==0){
		int i;
				
		for (i=0; i<ThreadCount; i++){ 
			if (ThreadHandle[i]!=0) {
				TerminateThread ((HANDLE)ThreadHandle[i], 0);
				CloseHandle((HANDLE)ThreadHandle[i]);
			}
		}

		for (i=0; i<ThreadCount; i++) 
			if (ThreadMem[i]!=0) free ((void *)ThreadMem[i]);

		free(ThreadHandle);
		ThreadHandle = NULL;

		free(ThreadMem);
		ThreadMem = NULL;

		ThreadCount = 0;
	
	} /*else {

		TerminateThread (hThread, 0);
		CloseHandle(hThread);

	}*/

	return true;

}

#endif


/***** Critical Section APIs *****/

INT_PTR createCriticalSection(void) {
    //returns handle to CS, or 0 if failed
    LPCRITICAL_SECTION lpcs = (LPCRITICAL_SECTION)malloc(sizeof(CRITICAL_SECTION));
    if (lpcs) {
        // CS was successfully allocated, now initialize it
        InitializeCriticalSection(lpcs);
    }
    return (INT_PTR)lpcs;    //returns 0 if couldn't be allocated
}/*createCriticalSection*/


#ifdef HAVE_XP  //requires XP or later
int createCriticalSectionAndSpinCount(DWORD dwSpinCount) {
    //returns handle to CS, or 0 if failed
    LPCRITICAL_SECTION lpcs = malloc(sizeof(CRITICAL_SECTION));
    if (lpcs) {
        // CS was successfully allocated, now initialize it..
        // Returns 0 if error.
        if (InitializeCriticalSectionAndSpinCount(lpcs, dwSpinCount))
            return (int)lpcs;   //success -- return handle to CS
        else
            return 0;           //failure -- return 0
    }
    return (int)lpcs;    //returns 0 if couldn't be allocated
}/*fm_createCriticalSection*/
#endif //XP


#ifdef HAVE_VISTA //requires Vista or later
int createCriticalSectionEx(DWORD dwSpinCount, DWORD flags) {
    // flags should be 0 or CRITICAL_SECTION_NO_DEBUG_INFO
    //returns handle to CS, or 0 if failed
    LPCRITICAL_SECTION lpcs = malloc(sizeof(CRITICAL_SECTION));
    if (lpcs) {
        // CS was successfully allocated, now initialize it..
        // Returns 0 if error.
        if (InitializeCriticalSectionEx(lpcs, dwSpinCount, flags))
            return (int)lpcs;   //success -- return handle to CS
        else
            return 0;           //failure -- return 0
    }
    return (int)lpcs;    //returns 0 if couldn't be allocated
}/*fm_createCriticalSection*/
#endif //Vista


#ifdef HAVE_XP  //requires XP or later
int setCriticalSectionSpinCount(int hCS, DWORD spinCount) {
    //returns previous spin count, or 0 if hCS is 0
    //if hCS is 0, SetCriticalSectionSpinCount will GP fault!
    if (!hCS) return 0;
    return SetCriticalSectionSpinCount((LPCRITICAL_SECTION)hCS, spinCount);
}/*setCriticalSectionSpinCount*/
#endif //XP


BOOL enterCriticalSection(INT_PTR hCS) {
    //returns true upon successful entry, false if hCS is 0.
    //if hCS is 0, EnterCriticalSection will GP fault!
    if (!hCS) return false;
    EnterCriticalSection((LPCRITICAL_SECTION)hCS);
    return true;
}/*fm_enterCriticalSection*/


#if _WIN32_WINNT >= 0x0400 
BOOL tryEnterCriticalSection(INT_PTR hCS) {
    //returns true if entered CS, false if not (or hCS is 0)
    //if hCS is 0, TryEnterCriticalSection will GP fault!
    if (!hCS) return false;
    return TryEnterCriticalSection((LPCRITICAL_SECTION)hCS);
}/*fm_tryEnterCriticalSection*/
#endif


BOOL leaveCriticalSection(INT_PTR hCS) {
    //returns true upon successful exit, false if hCS is 0.
    //if hCS is 0, LeaveCriticalSection will GP fault!
    if (!hCS) return false;
    LeaveCriticalSection((LPCRITICAL_SECTION)hCS);
    return true;
}/*fm_leaveCriticalSection*/


BOOL deleteCriticalSection(INT_PTR hCS) {
    // DE-initialize the CS...
    //returns true if successful, false if hCS is 0.
    //if hCS is 0, DeleteCriticalSection will GP fault!
    if (!hCS) return false;
    DeleteCriticalSection((LPCRITICAL_SECTION)hCS);
    //now delete the CS object itself
    free((void *)hCS);
    hCS = 0;    //Mark the CS handle no longer valid
    return true;
}/*fm_deleteCriticalSection*/

