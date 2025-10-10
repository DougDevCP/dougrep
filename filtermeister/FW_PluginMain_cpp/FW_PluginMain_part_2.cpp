
/*****************************************************************************/

/* All calls to the plug-in module come through this routine. It must be
   placed first in the resource. To achieve this, most development systems
   require that this be the first routine in the source. */

#ifdef __cplusplus
   #define DllExport   extern "C" __declspec( dllexport )
#else
   #define DllExport   __declspec( dllexport )
#endif



extern "C" DllExport void ENTRYPOINT (short selector,
                        FilterRecord *filterParamBlock,
                        intptr_t *data,
                        short *result)              
    {

#ifdef NDEBUG
    __try {
#endif
  
    Globals globalValues;
    GPtr globals = &globalValues;

#ifdef __cplusplus
    if (!hDllInstance) {
        /*MEMORY_BASIC_INFORMATION mbi;
		static int dummy;
		VirtualQuery( &dummy, &mbi, sizeof(mbi) );
		hDllInstance = (HINSTANCE)mbi.AllocationBase;*/
        hDllInstance = GetDLLInstance();
    }
#endif    

    if (!*data)
        {
        
            InitGlobals (globals);

            *data = (intptr_t) NewHandle (sizeof (Globals));
        
            if (!*data)
            {
                *result = memFullErr;
                return;
            }
        
            ** (GHdl) *data = globalValues;
        
        }
        
    globalValues = ** (GHdl) *data;
    //Should be the only ptr to globals that we need!!!
    *(GPtr *)&MyGlobals = globals;    //cast away const on MyGlobals this one time!
        
    gStuff = filterParamBlock;
    gResult = noErr;


	//if (selector == filterSelectorAbout){
	//	sSPBasic = ((AboutRecord*)gStuff)->sSPBasic;
	//} else {
		sSPBasic = gStuff->sSPBasic;
	//}

#if BIGDOCUMENT
    //Tell Photoshop that we support bigDocument stuff
//    if (gStuff->bigDocumentData != NULL)
//		    gStuff->bigDocumentData->PluginUsing32BitCoordinates = true;
#endif

    switch (selector)
        {

        case filterSelectorAbout:
            DoAbout (globals, selector);
            break;

        case filterSelectorParameters:
            DoParameters (globals);
            break;

        case filterSelectorPrepare:
            DoPrepare (globals);
            break;

        case filterSelectorStart:
            DoStart (globals);
            break;

        case filterSelectorContinue:
            DoContinue (globals);
            break;

        case filterSelectorFinish:
            DoFinish (globals);
            break;

        default:
            gResult = filterBadParameters;  // NFG for PSP???
            break;

    }

    if (gResult == filterBadParameters)
    {   // PSP and others don't recognize the filterBadParameters error,
        // so put out our own message box...
        ShowAlert(16502);
        gResult = 1;    // indicates error message already displayed.
    }


    *result = gResult;
    
    ** (GHdl) *data = globalValues;


#ifdef NDEBUG
    }
    __except (EXCEPTION_EXECUTE_HANDLER) {
        ErrorOk ("Sorry, a framework error occured."); //,(int)_exception_code()
        //gResult = userCanceledErr;
    }
#endif

        
}

/*
DllExport void ENTRYPOINT00 (short selector, FilterRecord *filterParamBlock, intptr_t *data, short *result)
{
	fmc.entryPoint = 0;
	ENTRYPOINT (selector, filterParamBlock, data, result);
}
DllExport void ENTRYPOINT01 (short selector, FilterRecord *filterParamBlock, intptr_t *data, short *result)
{
	fmc.entryPoint = 1;
	ENTRYPOINT (selector, filterParamBlock, data, result);
}
DllExport void ENTRYPOINT02 (short selector, FilterRecord *filterParamBlock, intptr_t *data, short *result)
{
	fmc.entryPoint = 2;
	ENTRYPOINT (selector, filterParamBlock, data, result);
}
DllExport void ENTRYPOINT03 (short selector, FilterRecord *filterParamBlock, intptr_t *data, short *result)
{
	fmc.entryPoint = 3;
	ENTRYPOINT (selector, filterParamBlock, data, result);
}
DllExport void ENTRYPOINT04 (short selector, FilterRecord *filterParamBlock, intptr_t *data, short *result)
{
	fmc.entryPoint = 4;
	ENTRYPOINT (selector, filterParamBlock, data, result);
}
DllExport void ENTRYPOINT05 (short selector, FilterRecord *filterParamBlock, intptr_t *data, short *result)
{
	fmc.entryPoint = 5;
	ENTRYPOINT (selector, filterParamBlock, data, result);
}
DllExport void ENTRYPOINT06 (short selector, FilterRecord *filterParamBlock, intptr_t *data, short *result)
{
	fmc.entryPoint = 6;
	ENTRYPOINT (selector, filterParamBlock, data, result);
}
DllExport void ENTRYPOINT07 (short selector, FilterRecord *filterParamBlock, intptr_t *data, short *result)
{
	fmc.entryPoint = 7;
	ENTRYPOINT (selector, filterParamBlock, data, result);
}
DllExport void ENTRYPOINT08 (short selector, FilterRecord *filterParamBlock, intptr_t *data, short *result)
{
	fmc.entryPoint = 8;
	ENTRYPOINT (selector, filterParamBlock, data, result);
}
DllExport void ENTRYPOINT09 (short selector, FilterRecord *filterParamBlock, intptr_t *data, short *result)
{
	fmc.entryPoint = 9;
	ENTRYPOINT (selector, filterParamBlock, data, result);
}
DllExport void ENTRYPOINT10 (short selector, FilterRecord *filterParamBlock, intptr_t *data, short *result)
{
	fmc.entryPoint = 10;
	ENTRYPOINT (selector, filterParamBlock, data, result);
}
DllExport void ENTRYPOINT11 (short selector, FilterRecord *filterParamBlock, intptr_t *data, short *result)
{
	fmc.entryPoint = 11;
	ENTRYPOINT (selector, filterParamBlock, data, result);
}
DllExport void ENTRYPOINT12 (short selector, FilterRecord *filterParamBlock, intptr_t *data, short *result)
{
	fmc.entryPoint = 12;
	ENTRYPOINT (selector, filterParamBlock, data, result);
}
DllExport void ENTRYPOINT13 (short selector, FilterRecord *filterParamBlock, intptr_t *data, short *result)
{
	fmc.entryPoint = 13;
	ENTRYPOINT (selector, filterParamBlock, data, result);
}
DllExport void ENTRYPOINT14 (short selector, FilterRecord *filterParamBlock, intptr_t *data, short *result)
{
	fmc.entryPoint = 14;
	ENTRYPOINT (selector, filterParamBlock, data, result);
}
DllExport void ENTRYPOINT15 (short selector, FilterRecord *filterParamBlock, intptr_t *data, short *result)
{
	fmc.entryPoint = 15;
	ENTRYPOINT (selector, filterParamBlock, data, result);
}
DllExport void ENTRYPOINT16 (short selector, FilterRecord *filterParamBlock, intptr_t *data, short *result)
{
	fmc.entryPoint = 16;
	ENTRYPOINT (selector, filterParamBlock, data, result);
}
DllExport void ENTRYPOINT17 (short selector, FilterRecord *filterParamBlock, intptr_t *data, short *result)
{
	fmc.entryPoint = 17;
	ENTRYPOINT (selector, filterParamBlock, data, result);
}
DllExport void ENTRYPOINT18 (short selector, FilterRecord *filterParamBlock, intptr_t *data, short *result)
{
	fmc.entryPoint = 18;
	ENTRYPOINT (selector, filterParamBlock, data, result);
}
DllExport void ENTRYPOINT19 (short selector, FilterRecord *filterParamBlock, intptr_t *data, short *result)
{
	fmc.entryPoint = 19;
	ENTRYPOINT (selector, filterParamBlock, data, result);
}
DllExport void ENTRYPOINT20 (short selector, FilterRecord *filterParamBlock, intptr_t *data, short *result)
{
	fmc.entryPoint = 20;
	ENTRYPOINT (selector, filterParamBlock, data, result);
}
DllExport void ENTRYPOINT21 (short selector, FilterRecord *filterParamBlock, intptr_t *data, short *result)
{
	fmc.entryPoint = 21;
	ENTRYPOINT (selector, filterParamBlock, data, result);
}
DllExport void ENTRYPOINT22 (short selector, FilterRecord *filterParamBlock, intptr_t *data, short *result)
{
	fmc.entryPoint = 22;
	ENTRYPOINT (selector, filterParamBlock, data, result);
}
DllExport void ENTRYPOINT23 (short selector, FilterRecord *filterParamBlock, intptr_t *data, short *result)
{
	fmc.entryPoint = 23;
	ENTRYPOINT (selector, filterParamBlock, data, result);
}
DllExport void ENTRYPOINT24 (short selector, FilterRecord *filterParamBlock, intptr_t *data, short *result)
{
	fmc.entryPoint = 24;
	ENTRYPOINT (selector, filterParamBlock, data, result);
}
DllExport void ENTRYPOINT25 (short selector, FilterRecord *filterParamBlock, intptr_t *data, short *result)
{
	fmc.entryPoint = 25;
	ENTRYPOINT (selector, filterParamBlock, data, result);
}
DllExport void ENTRYPOINT26 (short selector, FilterRecord *filterParamBlock, intptr_t *data, short *result)
{
	fmc.entryPoint = 26;
	ENTRYPOINT (selector, filterParamBlock, data, result);
}
DllExport void ENTRYPOINT27 (short selector, FilterRecord *filterParamBlock, intptr_t *data, short *result)
{
	fmc.entryPoint = 27;
	ENTRYPOINT (selector, filterParamBlock, data, result);
}
DllExport void ENTRYPOINT28 (short selector, FilterRecord *filterParamBlock, intptr_t *data, short *result)
{
	fmc.entryPoint = 28;
	ENTRYPOINT (selector, filterParamBlock, data, result);
}
DllExport void ENTRYPOINT29 (short selector, FilterRecord *filterParamBlock, intptr_t *data, short *result)
{
	fmc.entryPoint = 29;
	ENTRYPOINT (selector, filterParamBlock, data, result);
}
DllExport void ENTRYPOINT30 (short selector, FilterRecord *filterParamBlock, intptr_t *data, short *result)
{
	fmc.entryPoint = 30;
	ENTRYPOINT (selector, filterParamBlock, data, result);
}
DllExport void ENTRYPOINT31 (short selector, FilterRecord *filterParamBlock, intptr_t *data, short *result)
{
	fmc.entryPoint = 31;
	ENTRYPOINT (selector, filterParamBlock, data, result);
}
*/


/*****************************************************************************/

void InitGlobals (GPtr globals)
    {