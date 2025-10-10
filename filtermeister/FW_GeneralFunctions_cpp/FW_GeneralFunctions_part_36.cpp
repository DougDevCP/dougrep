/****** Interlocked Variable API's ******/

int interlockedIncrement(INT_PTR pvar) {
    // Atomically increments a variable,
    // returning the resulting incremented value.
    // 486+ only? Win98+/NT4.0+?
    return InterlockedIncrement((LONG *)pvar);
}

int interlockedDecrement(INT_PTR pvar) {
    // Atomically decrements a variable,
    // returning the resulting decremented value.
    // 486+ only? Win98+/NT4.0+?
    return InterlockedDecrement((LONG *)pvar);
}

int interlockedExchange(INT_PTR ptarget, int value) {
    // Atomically sets the target int variable to the 
    // specified value, and returns the previous value
    // of the target variable.
    return InterlockedExchange((LONG *)ptarget, value);
}

int interlockedExchangeAdd(INT_PTR ptarget, int value) {
    // Atomically adds the specified value to 
    // the target int variable, and returns the previous
    // value of the target variable.
    return InterlockedExchangeAdd((LONG *)ptarget, value);
}

int interlockedCompareExchange(INT_PTR pdest, int exchange, int comparand) {
    // Atomically compares the value of the destination variable
    // with the comparand value.  If equal, then the exchange
    // value is stored in the destination variable;
    // otherwise the destination variable is not modified.
    // Returns the initial value of the destination variable.
#if defined(_WIN64) || _MSC_VER >= 1200
    return (int)InterlockedCompareExchange((LONG*)pdest, (LONG)exchange, (LONG)comparand);
#else
    return (int)InterlockedCompareExchange((PVOID)pdest, (PVOID)exchange, (PVOID)comparand);
#endif
}


/****** Thread Synchronization API's ******/

#if 0   //#1: Harry's first version (+ Alex's mods)

int NumberOfThreads=0;
int *lpThreadArray=NULL;
LPCRITICAL_SECTION lpcs1=NULL;


int createSync(int number){

    if (number<2) return false;
        
    NumberOfThreads = number;
    lpThreadArray = calloc(number*2,4);
    if (!lpThreadArray) return false;

    lpcs1 = malloc(sizeof(CRITICAL_SECTION));
    if (!lpcs1) {
        free(lpThreadArray);
        lpThreadArray = NULL;
        return false;
    }
    InitializeCriticalSection(lpcs1);

    return true;
}


int waitForSync(int syncnr, int timeout){

    int val,i, count;
    BOOL success=false;
    int ThreadID = (int)GetCurrentThreadId();
    int timeelapsed=false;
    int starttime = clock(); 
    
    
    if (NumberOfThreads<2 || lpThreadArray==NULL || lpcs1==NULL) return false; //In case Sync was not initialized correctly


    EnterCriticalSection(lpcs1);
        for (i=0;i<NumberOfThreads;i++){

            val = lpThreadArray[i*2];

            if (val==0){
                lpThreadArray[i*2] = ThreadID; 
                lpThreadArray[i*2+1] = syncnr;
                success = true;
                break;
        
            } else if (val==ThreadID){
                lpThreadArray[i*2+1] = syncnr;
                success = true;
                break;
            }

        }
    LeaveCriticalSection(lpcs1);

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

    if (lpcs1 != NULL){
        DeleteCriticalSection(lpcs1);
        free (lpcs1);
        lpcs1=NULL;
    }

    return true;
}

#elif 0 //#2: Tighten up the Critical Section *******************************************

int NumberOfThreads=0;
int *lpThreadArray=NULL;
LPCRITICAL_SECTION lpcs1=NULL;


int createSync(int number){

    if (number<2) return false;
        
    NumberOfThreads = number;
    lpThreadArray = calloc(number*2,4);
    if (!lpThreadArray) return false;

    lpcs1 = malloc(sizeof(CRITICAL_SECTION));
    if (!lpcs1) {
        free(lpThreadArray);
        lpThreadArray = NULL;
        return false;
    }
    InitializeCriticalSection(lpcs1);

    return true;
}


int waitForSync(int syncnr, int timeout){

    int val,i, count;
    BOOL success=false;
    int ThreadID = (int)GetCurrentThreadId();
    int timeelapsed=false;
    int starttime = clock(); 
    
    
    if (NumberOfThreads<2 || lpThreadArray==NULL || lpcs1==NULL) return false; //In case Sync was not initialized correctly


    //search for an empty slot (or one we already own)...

    for (i=0;i<NumberOfThreads;i++){

        //fetching, testing, and claiming slot i
        //must be atomic...
        EnterCriticalSection(lpcs1);        /****** enter CS ******/
        val = lpThreadArray[i*2];
        if (val==0){
            //take ownership of this slot
            lpThreadArray[i*2] = ThreadID;
            //now no one else can claim slot i
            LeaveCriticalSection(lpcs1);    /****** exit CS ******/

            //record our current sync number
            lpThreadArray[i*2+1] = syncnr;
            success = true;
            break;
    
        }
        LeaveCriticalSection(lpcs1);         /****** exit CS ******/

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

    if (lpcs1 != NULL){
        DeleteCriticalSection(lpcs1);
        free (lpcs1);
        lpcs1=NULL;
    }

    return true;
}

#elif 0 //#3: Using Interlocked Instruction instead of Critical Section ******************************

int NumberOfThreads=0;
int *lpThreadArray=NULL;


int createSync(int number){

    if (number<2) return false;
        
    NumberOfThreads = number;
    lpThreadArray = calloc(number*2,4);
    if (!lpThreadArray) return false;

    return true;
}


int waitForSync(int syncnr, int timeout){

    int val,i, count;
    BOOL success=false;
    int ThreadID = (int)GetCurrentThreadId();