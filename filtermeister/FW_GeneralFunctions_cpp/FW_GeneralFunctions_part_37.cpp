    int timeelapsed=false;
    int starttime = clock(); 
    
    
    if (NumberOfThreads<2 || lpThreadArray==NULL) return false; //In case Sync was not initialized correctly


    //search for an empty slot (or one we already own)...

    for (i=0;i<NumberOfThreads;i++){

        //fetching, testing, and claiming slot i
        //must be atomic...
        val = (int)InterlockedCompareExchange((PVOID *)&lpThreadArray[i*2], (PVOID)ThreadID, 0);
        //val is the *previous* value of this slot
        if (val==0){
            //we took ownership of this slot
            //now no one else can claim slot i
            //record our current sync number
            lpThreadArray[i*2+1] = syncnr;
            success = true;
            break;
    
        }

        //someone already owns slot i, but who?
        //(once slot i is claimed, it never gets
        //changed until deletion time)
        if (val==ThreadID){
            //we already own this slot...
            //record our current sync number
            lpThreadArray[i*2+1] = syncnr;
            success = true;
            break;
        }

    }//for i

    if (!success) return false; //because an unknown thread entered waitForSync



    while (true){
        count=0;
        for (i=0;i<NumberOfThreads;i++){
            if (lpThreadArray[i*2]!=0 && lpThreadArray[i*2+1] >= syncnr) count++;
        }

        if (count>=NumberOfThreads){
            timeelapsed=false;
            break;
        }

        if (timeout!=0xffffffff && clock()-starttime >= timeout) {
            timeelapsed=true; 
            break;
        }

        Sleep(0);
    }  


    if (timeelapsed)
        return false;
    else
        return true;
}


int deleteSync(void){

    NumberOfThreads = 0;

    if (lpThreadArray!=NULL){
        free (lpThreadArray);
        lpThreadArray=NULL;
    }

    return true;
}

#elif 1 //#4: Tighten up the While-loop ******************************

int NumberOfThreads=0;
int *lpThreadArray=NULL;


int createSync(int number){

    if (number<2) return false;
        
    NumberOfThreads = number;
    lpThreadArray = (int *)calloc(number*2,4);
    if (!lpThreadArray) return false;

    return true;
}//createSync


int waitForSync(int syncnr, int timeout){

    int val, i;
    BOOL success=false;
    int ThreadID = (int)GetCurrentThreadId();
    BOOL timeelapsed=false;
    int starttime = clock(); 
    
    if (NumberOfThreads<2 || lpThreadArray==NULL) return false; //In case Sync was not initialized correctly

    //search for an empty slot (or one we already own)...

    for (i=0; i<NumberOfThreads; i++){

        //fetching, testing, and claiming slot i
        //must be atomic...
        #if defined(_WIN64) || _MSC_VER >= 1200
            val = (int)InterlockedCompareExchange((LONG*)&lpThreadArray[i*2], (LONG)ThreadID, 0);
        #else
            val = (int)InterlockedCompareExchange((PVOID *)&lpThreadArray[i*2], (PVOID)ThreadID, 0);
        #endif
        //val is the *previous* value of this slot
        if (val==0){
            //we took ownership of this slot
            //now no one else can claim slot i
            //record our current sync number
            lpThreadArray[i*2+1] = syncnr;
            success = true;
            break;
    
        }

        //someone already owns slot i, but who?
        //(once slot i is claimed, it never gets
        //changed until deletion time)
        if (val==ThreadID){
            //we already own this slot...
            //record our current sync number
            lpThreadArray[i*2+1] = syncnr;
            success = true;
            break;
        }

    }//for i

    if (!success) return false; //because an unknown thread entered waitForSync


    // Wait until all threads are at the current syncnr or greater.

    for (i=0; i<NumberOfThreads && !timeelapsed; i++) {
        //wait until thread i reaches syncnr or greater...
        while(lpThreadArray[i*2]==0 || lpThreadArray[i*2+1] < syncnr) {
            //check for timeout
            if ((timeout!=INFINITE && clock()-starttime >= timeout) || mtAbortFlag) {
                timeelapsed=true; 
                break;
            }//if timeout
            //yield this timeslice
            Sleep(0);
        }//while
    }//for

    //Either we timed out, or all threads are at current syncnr or greater...
    return !timeelapsed;

}//waitForSync


int deleteSync(void){

    NumberOfThreads = 0;

    if (lpThreadArray!=NULL){
        free (lpThreadArray);
        lpThreadArray=NULL;
    }

    return true;
}//deleteSync

#elif 0 //#5: Cache mySlot ******************************

int NumberOfThreads=0;
int *lpThreadArray=NULL;


int createSync(int number){

    if (number<2) return false;
        
    NumberOfThreads = number;
    lpThreadArray = calloc(number*2,4);
    if (!lpThreadArray) return false;

    return true;
}//createSync


int waitForSync(int syncnr, int timeout){

    THREAD_LOCAL static int mySlot = -1;
    int val, i;
    BOOL success=false;
    int ThreadID = (int)GetCurrentThreadId();
    BOOL timeelapsed=false;
    int starttime = clock(); 
    
    if (NumberOfThreads<2 || lpThreadArray==NULL) return false; //In case Sync was not initialized correctly

    //search for an empty slot (if we don't already own one)...

    if (mySlot == -1) {
        //not yet assigned a slot...
        for (i=0; i<NumberOfThreads; i++){
            //fetching, testing, and claiming slot i
            //must be atomic...
            val = (int)InterlockedCompareExchange((PVOID *)&lpThreadArray[i*2], (PVOID)ThreadID, 0);
            //val is the *previous* value of this slot
            if (val==0){
                //we took ownership of this slot
                //now no one else can claim slot i
                //cache our assigned slot#
                mySlot = i;
                success = true;
                break;
            }//if
        }//for i
        if (!success) return false; //because an unknown thread entered waitForSync
    }//if

    lpThreadArray[mySlot*2+1] = syncnr;  //record our current syncnr


    // Wait until all threads are at the current syncnr or greater.

    for (i=0; i<NumberOfThreads && !timeelapsed; i++) {
        //wait until thread i reaches syncnr or greater...
        while(lpThreadArray[i*2]==0 || lpThreadArray[i*2+1] < syncnr) {
            //check for timeout
            if (timeout!=INFINITE && clock()-starttime >= timeout) {
                timeelapsed=true; 
                break;
            }//if timeout
            //yield this timeslice
            Sleep(0);
        }//while
    }//for

    //Either we timed out, or all threads are at current syncnr or greater...
    return !timeelapsed;

}//waitForSync


int deleteSync(void){

    NumberOfThreads = 0;

    if (lpThreadArray!=NULL){
        free (lpThreadArray);
        lpThreadArray=NULL;
    }

    return true;
}//deleteSync

#elif 0 //#6: Assign mySlot directly ******************************

int NumberOfThreads=0;
int *lpThreadArray=NULL;
int NextSlot=0;


int createSync(int number){

    if (number<2) return false;
        
    NumberOfThreads = number;
    NextSlot = 0;
    lpThreadArray = calloc(number,4);
    if (!lpThreadArray) return false;

    return true;
}//createSync


int waitForSync(int syncnr, int timeout){

    THREAD_LOCAL static int mySlot = -1;
    int i;
    BOOL timeelapsed=false;
    int starttime = clock(); 
    
    if (NumberOfThreads<2 || lpThreadArray==NULL) return false; //In case Sync was not initialized correctly
    if (syncnr < 1) return false; //syncnr must be 1 or greater

    //first time around, assign next slot in ThreadArray to this thread...

    if (mySlot == -1) {
        //not yet assigned a slot...