__declspec(naked)
#endif
void __cdecl fldcw(int cw)
{
#ifndef _WIN64
    __asm {
        fldcw word ptr[esp+4]   ; load control word from low 16 bits of argument
        ret
    }
#endif
}

// restore "no return value" warnings
#ifdef _WIN64
    #pragma warning (default: 4716)
#else
    #pragma warning (default: 4035)
#endif


/****************** End DLL Access Routines *************************/

//#endif //#if 0


int setPreviewDragMode (int mode)
{
    //0
    //1 = ZeroDrag //Default!
    //2 = OutputDrag
    //3 = Zero + Output
    
    if (mode==0){
        gParams->flags &= ~AFH_ZERO_OUTPUT_DRAG;
        gParams->flags &= ~AFH_DRAG_FILTERED_OUTPUT;
		//gParams->flags |= AFH_QUICK_TOGGLE;
    } else if (mode==1){
        gParams->flags |= AFH_ZERO_OUTPUT_DRAG;
        gParams->flags &= ~AFH_DRAG_FILTERED_OUTPUT;
		//gParams->flags |= AFH_QUICK_TOGGLE;
    } else if (mode==2){
        gParams->flags &= ~AFH_ZERO_OUTPUT_DRAG;
        gParams->flags |= AFH_DRAG_FILTERED_OUTPUT;
		//gParams->flags |= AFH_QUICK_TOGGLE;
    } else if (mode==3){
        gParams->flags |= AFH_ZERO_OUTPUT_DRAG;
        gParams->flags |= AFH_DRAG_FILTERED_OUTPUT;
		//gParams->flags |= AFH_QUICK_TOGGLE;

	}/*else if (mode==4){
        gParams->flags &= ~AFH_ZERO_OUTPUT_DRAG;
        gParams->flags &= ~AFH_DRAG_FILTERED_OUTPUT;
		gParams->flags &= ~AFH_QUICK_TOGGLE;
    } else if (mode==5){
        gParams->flags |= AFH_ZERO_OUTPUT_DRAG;
        gParams->flags &= ~AFH_DRAG_FILTERED_OUTPUT;
		gParams->flags &= ~AFH_QUICK_TOGGLE;
    } else if (mode==6){
        gParams->flags &= ~AFH_ZERO_OUTPUT_DRAG;
        gParams->flags |= AFH_DRAG_FILTERED_OUTPUT;
		gParams->flags &= ~AFH_QUICK_TOGGLE;
    } else if (mode==7){
        gParams->flags |= AFH_ZERO_OUTPUT_DRAG;
        gParams->flags |= AFH_DRAG_FILTERED_OUTPUT;
		gParams->flags &= ~AFH_QUICK_TOGGLE;
    }*/

    return true;
    
}



int linearInterpolate (int v1,int v2, double x)
{

    return  (int)(v1*(1.0-x) + v2*x);
}

int cosineInterpolate (int v1,int v2, double x)
{

    double ft = x * 3.1415927;
	double f = (1.0 - cos(ft)) * 0.5;

	return  (int) (v1*(1.0-f) + v2*f);

}

int cubicInterpolate (int v0,int v1,int v2,int v3,double x)
{

    int P = (v3 - v2) - (v0 - v1);
	int Q = (v0 - v1) - P;
	int R = v2 - v0;
	int S = v1;

    return (int)(P*x*x*x + Q*x*x + R*x + S);

}


int hermiteInterpolate (int T1,int P1,int P2,int T2, double s)
{
  double s2 = s*s;
  double s3 = s2*s;

  double h1 =  2*s3 - 3*s2 + 1;          // calculate basis function 1
  double h2 = -2*s3 + 3*s2;              // calculate basis function 2
  double h3 =   s3 - 2*s2 + s;         // calculate basis function 3
  double h4 =   s3 -  s2;              // calculate basis function 4

  return (int)(h1*P1 +                  // multiply and sum all funtions
             h2*P2 +                    // together to build the interpolated
             h3*T1 +                    // point along the curve.
             h4*T2);
}


int getImageTitle (char * n) //LPSTR
{

#ifdef APP

	if (app.strInputFiles+app.inputIndex*(MAX_PATH+1) != NULL)
		strcpy(n,app.strInputFiles+app.inputIndex*(MAX_PATH+1));

	return true;

#else

    GetPropertyProc pGPP;
    OSErr err;
    intptr_t simpleProperty;
    Handle complexProperty;
    int success = false;


    pGPP = gStuff->getPropertyObsolete;
    if (gStuff->propertyProcs) {
        if (gStuff->propertyProcs->propertyProcsVersion >= kCurrentPropertyProcsVersion &&
            gStuff->propertyProcs->numPropertyProcs >= 1 &&
            gStuff->propertyProcs->getPropertyProc) {
            //prefer this version of getPropertyProc over the obsolete one
            pGPP = gStuff->propertyProcs->getPropertyProc;
        }
    } //propertyProcs != 0

    if (pGPP) {

        err = pGPP('8BIM', 'titl', 0, &simpleProperty, &complexProperty);
        if (err == noErr) {
            int size;
            LPSTR ptr;
            size = HostGetHandleSize(gStuff->handleProcs, complexProperty);
            
            if (size>0){
                size = min(size, 256);  //limit to avoid buff overflow
                ptr = HostLockHandle(gStuff->handleProcs, complexProperty, FALSE);
           
                if (ptr != NULL){
                
                    //strcpy (n, ptr); //Problem with PSP 7 when using strcpy
                    memcpy (n, ptr, size);
                    memset (n+size,0,1);
                
                    success = true;
                }

                HostUnlockHandle(gStuff->handleProcs, complexProperty);
                HostDisposeHandle(gStuff->handleProcs, complexProperty);

                if (success)
                    return true;
                else
                    return false;
            
            } 

        }

    }   

#endif

    return false;

}

int getEXIFSize (){

    int size = 0;

#ifndef APP

    GetPropertyProc pGPP;
    OSErr err;
    intptr_t simpleProperty;
    Handle complexProperty;
    int success = false;

    pGPP = gStuff->getPropertyObsolete;
    if (gStuff->propertyProcs) {
        if (gStuff->propertyProcs->propertyProcsVersion >= kCurrentPropertyProcsVersion &&
            gStuff->propertyProcs->numPropertyProcs >= 1 &&
            gStuff->propertyProcs->getPropertyProc) {
            //prefer this version of getPropertyProc over the obsolete one
            pGPP = gStuff->propertyProcs->getPropertyProc;
        }
    }

    if (pGPP) {
        err = pGPP('8BIM', 'EXIF', 0, &simpleProperty, &complexProperty);
        if (err == noErr) {
            size = HostGetHandleSize(gStuff->handleProcs, complexProperty);
        }
    } else {
        size = -1; //Not supported by host
    }

#endif

    return size;
}



int getEXIFData (int * buffer){

//Memory buffer n is allocated by function itself
//Return value is buffer size

#ifndef APP

    GetPropertyProc pGPP;
    OSErr err;
    intptr_t simpleProperty;
    Handle complexProperty;
    int success = false;

    pGPP = gStuff->getPropertyObsolete;
    if (gStuff->propertyProcs) {
        if (gStuff->propertyProcs->propertyProcsVersion >= kCurrentPropertyProcsVersion &&
            gStuff->propertyProcs->numPropertyProcs >= 1 &&
            gStuff->propertyProcs->getPropertyProc) {
            //prefer this version of getPropertyProc over the obsolete one
            pGPP = gStuff->propertyProcs->getPropertyProc;
        }
    }

    if (pGPP) {

        err = pGPP('8BIM', 'EXIF', 0, &simpleProperty, &complexProperty);
        if (err == noErr) {
            int size;
            LPSTR ptr;
            size = HostGetHandleSize(gStuff->handleProcs, complexProperty);
            
            if (size>0){
                ptr = HostLockHandle(gStuff->handleProcs, complexProperty, FALSE);

                if (ptr != NULL){
                    //buffer = malloc(size);
                    if (buffer){
                        memcpy (buffer, ptr, size);
                        success = true;
                    }
                }

                HostUnlockHandle(gStuff->handleProcs, complexProperty);
                HostDisposeHandle(gStuff->handleProcs, complexProperty);

                if (success) return true;//size;
            } 

        }

    }   

#endif

    return 0;
}



int destroyMenu(INT_PTR hMenu)
{
    // cdecl -> stdcall thunk
    return DestroyMenu((HMENU)hMenu);
}


int insertMenuItem (INT_PTR hMenu, int uItem, char* itemName, int fState, INT_PTR subMenu)
{
    return insertMenuItemEx (hMenu, uItem, itemName, fState, false, subMenu);
}


int insertMenuItemEx (INT_PTR hMenu, int uItem, char* itemName, int fState, int bullet, INT_PTR subMenu)