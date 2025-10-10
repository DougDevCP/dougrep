
		//Get sub tab
		if (gParams->ctl[i].tabControl > 0) 
			subtab = gParams->ctl[gParams->ctl[i].tabControl].tabControl; else subtab = -1;

		if (gParams->ctl[i].inuse && gParams->ctl[i].defval != -1 && 
			((gParams->ctl[i].enabled == 3 && gParams->ctl[i].tabControl == iCtl &&  gParams->ctl[i].tabSheet == val) || subtab == iCtl)
		){
		
			oldvalue = gParams->ctl[i].val;
			setCtlVal(i,gParams->ctl[i].defval);
			if (gParams->ctl[i].ctlclass == CC_OWNERDRAW)
				setCtlColor(i,gParams->ctl[i].defval);
			else if (gParams->ctl[i].ctlclass == CC_CHECKBOX || gParams->ctl[i].ctlclass == CC_COMBOBOX)
				triggerEvent(i,FME_CLICKED,oldvalue);

			if (gParams->ctl[i].action == CA_PREVIEW) previewUpdate = true;
		}	
	}
	
	triggerEvent(iCtl,FME_TABRESET,oldvalue);
	
	if (previewUpdate) doAction(CA_PREVIEW);

}

void removeChar(char * str, char garbage) {

    char *src, *dst;
    for (src = dst = str; *src != '\0'; src++) {
        *dst = *src;
        if (*dst != garbage) dst++;
    }
    *dst = '\0';
}

int fm_isnan(double x)
{
   return x != x;
}

int fm_isinf(double x)
{
   if (/*(x == x) &&*/ ((x - x) != 0.0)) return (x < 0.0 ? -1 : 1);
   else return 0;
}




/*
char hostName[32] = "";
int hostVersion = 0;
int hostVersionMinor = 0;
BOOL isPhotoshop = true;
BOOL isElements = false;

#ifndef APP

BOOL getHostApp(){

	char appPath[512];
	GetModuleFileName(GetModuleHandle(NULL),appPath,512);
	toLowerCase(appPath);
	
	//Get Host Application
	if (strcmp(&appPath[strlen(appPath)-13],"photoshop.exe") == 0 || 
		strcmp(&appPath[strlen(appPath)-12],"photoshp.exe") == 0
	){
		strcpy_s(hostName,32,"Photoshop");
		isPhotoshop = true;
		isElements = false;
	} else {
		strcpy_s(hostName,32,"Elements");
		isPhotoshop = false;
		isElements = true;
	}
	
	{
		//Get Version
		DWORD  verHandle = 0;
		UINT   size      = 0;
		LPBYTE lpBuffer  = NULL;
		DWORD  verSize   = GetFileVersionInfoSize( appPath, &verHandle);

		if (verSize){
			LPSTR verData = malloc(verSize);
			if (GetFileVersionInfo( appPath, verHandle, verSize, verData)){
				if (VerQueryValue(verData,"\\",(VOID FAR* FAR*)&lpBuffer,&size)){
					if (size){
						VS_FIXEDFILEINFO *verInfo = (VS_FIXEDFILEINFO *)lpBuffer;
						if (verInfo->dwSignature == 0xfeef04bd){
							//int major = HIWORD(verInfo->dwFileVersionMS);
							//int minor = LOWORD(verInfo->dwFileVersionMS);
							//int build = verInfo->dwFileVersionLS;
							//sprintf_s(appString,128,"%s %d",appString,major);
							//*appVersion = HIWORD(verInfo->dwFileVersionMS);
							hostVersion = HIWORD(verInfo->dwFileVersionMS);
							hostVersionMinor = LOWORD(verInfo->dwFileVersionMS);
							//if (isElements && hostVersion==13)
							//	hostVersionMinor = checkPSE13MinorVersion(appPath);
						}
					}
				}
			}
			free(verData);
		}
	}

	return true;
}

#endif

BOOL getRegistryData(HKEY hRootKey, char *subKey, char *value, LPBYTE data, DWORD cbData, int WowAccess)
{
	HKEY hKey;
	LONG error;
	REGSAM regSam = KEY_QUERY_VALUE;
	if (WowAccess==1) regSam |= KEY_WOW64_64KEY;
	else if (WowAccess==2) regSam |= KEY_WOW64_32KEY;
	error = RegOpenKeyEx(hRootKey, subKey, 0, regSam, &hKey);
	if (error) return false;

	error = RegQueryValueEx(hKey, value, NULL, NULL, data, &cbData);
	RegCloseKey(hKey);
	if (error) return false;
	
	return true;
}


BOOL checkUIScaling(){

	BOOL UIScaling = false;

#ifndef APP

	if (hostVersion==0) getHostApp();

	if ((isElements && hostVersion>=13) || (isPhotoshop && hostVersion>=15)){
		
		double factorX = 1.0, factorY = 1.0;
		DWORD dScale = -1;
		char registryStr[256];
		BOOL retval = false;

		getSysDPI(&factorX,&factorY);
		
		if (isElements)
			sprintf_s(registryStr,256,"Software\\Adobe\\Photoshop Elements\\%d.0",hostVersion);
		else {
			int versionVal = 60 + (hostVersion-13)*10;
			sprintf_s(registryStr,256,"Software\\Adobe\\Photoshop\\%d.0",versionVal);
		}

		
		if (isPhotoshop && hostVersion>=16)
			retval = getRegistryData(HKEY_CURRENT_USER,registryStr,"UIScale",(LPBYTE)&dScale,4,128);
		else
			retval = getRegistryData(HKEY_CURRENT_USER,registryStr,"UIScalingSize",(LPBYTE)&dScale,4,128);
		
		if (!retval) //UI scaling Works even without reg key
			dScale = 1;

		if (dScale == 2){
			UIScaling = true;
		} else if (dScale == 1){
			int width  = GetSystemMetrics(SM_CXSCREEN);
			int height = GetSystemMetrics(SM_CYSCREEN);
			if (width>=2560 && height>=1600 && max(factorX,factorY)>= 1.5) //3840 x 2160
				UIScaling = true;
		}
	}

#endif

	return UIScaling;

}
*/

int getSysDPI(double * factorX, double * factorY){

	int dpiX,dpiY;

	HDC hdc = GetDC(NULL);
    if (hdc){
        if (factorX) dpiX = GetDeviceCaps(hdc, LOGPIXELSX);
        if (factorY) dpiY = GetDeviceCaps(hdc, LOGPIXELSY);
        ReleaseDC(NULL, hdc);

		if (factorX) *factorX =  dpiX / 96.0;
		if (factorY) *factorY =  dpiY / 96.0;
		return max(dpiX,dpiY);
	}
	
	return 0;
}

BOOL isHDPIAware()
{

#ifndef APP

    GetPropertyProc pGPP;
    OSErr err;
    intptr_t simpleProperty;

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
        err = pGPP('8BIM', 'HDPI', 0, &simpleProperty, NULL);
        if (!err) {
            if (simpleProperty) 
				return true;
        }
    }   

#endif

    return false;

}

//#include <tpcshrd.h>
#define MICROSOFT_TABLETPENSERVICE_PROPERTY "MicrosoftTabletPenServiceProperty"
#define TABLET_DISABLE_PRESSANDHOLD        0x00000001
#define TABLET_DISABLE_PENTAPFEEDBACK      0x00000008
#define TABLET_DISABLE_PENBARRELFEEDBACK   0x00000010
#define TABLET_DISABLE_TOUCHUIFORCEON      0x00000100
#define TABLET_DISABLE_TOUCHUIFORCEOFF     0x00000200
#define TABLET_DISABLE_TOUCHSWITCH         0x00008000
#define TABLET_DISABLE_FLICKS              0x00010000
#define TABLET_ENABLE_FLICKSONCONTEXT      0x00020000
#define TABLET_ENABLE_FLICKLEARNINGMODE    0x00040000
#define TABLET_DISABLE_SMOOTHSCROLLING     0x00080000
#define TABLET_DISABLE_FLICKFALLBACKKEYS   0x00100000
#define TABLET_ENABLE_MULTITOUCHDATA       0x01000000

void deactivateFlicks(int n) {

	HWND hwnd;
	const DWORD_PTR dwHwndTabletProperty =
		TABLET_DISABLE_PRESSANDHOLD | // disables press and hold (right-click) gesture          
		TABLET_DISABLE_PENTAPFEEDBACK | // disables UI feedback on pen up (waves)          
		TABLET_DISABLE_PENBARRELFEEDBACK | // disables UI feedback on pen button down          
		TABLET_DISABLE_FLICKS; // disables pen flicks (back, forward, drag down, drag up)      
	ATOM atom = GlobalAddAtom(MICROSOFT_TABLETPENSERVICE_PROPERTY); //GlobalAddAtom(MICROSOFT_TABLETPENSERVICE_PROPERTY);

	if (n<0) hwnd = fmc.hDlg;
	else if (n == CTL_PREVIEW) GetDlgItem(fmc.hDlg, 101);
	else if (n == CTL_ZOOM) hwnd = GetDlgItem(fmc.hDlg, IDC_BUTTON1);
	else if (n == CTL_FRAME) hwnd = GetDlgItem(fmc.hDlg, IDC_PROXY_GROUPBOX);
	else if (n == CTL_PROGRESS) hwnd = GetDlgItem(fmc.hDlg, IDC_PROGRESS1);
	else hwnd = gParams->ctl[n].hCtl;

	SetProp(hwnd, MICROSOFT_TABLETPENSERVICE_PROPERTY, (HANDLE)dwHwndTabletProperty); //reinterpret_cast(dwHwndTabletProperty));
	GlobalDeleteAtom(atom);
}

void setDialogTitle(char * title){

	strcpy_s(fmc.filterTitleText, 256, title);

}


int evalZoomCombo(int n, int ctlPlus, int ctlMinus, int previous){

    int NewZoom=0;
    int NewLarge=1;
    int forceUpdate=0;
    int retval;
    int oldscale=fmc.scaleFactor;

    
#ifdef HIGHZOOM

    /*if (fmc.pre_ctl[n]==27){ //Softproof
        NewZoom = SoftProof();
        if (fmc.enlargeFactor==1 && scaleFactor==NewZoom && previous!=24) forceUpdate=true;
    }else*/ if (fmc.pre_ctl[n]==26){ //Fit
		NewZoom=-888;
    }else if (fmc.pre_ctl[n]>=0 && fmc.pre_ctl[n]<=9){ //800-200
		NewZoom=1;
        if (fmc.pre_ctl[n]==0) NewLarge=32;
        else if (fmc.pre_ctl[n]==1) NewLarge=16;
        else if (fmc.pre_ctl[n]==2) NewLarge=12;
        else NewLarge=11-fmc.pre_ctl[n];
    }else if (fmc.pre_ctl[n]==10 && previous<fmc.pre_ctl[n]){ //previous==fmc.pre_ctl[n]-1
		setCtlVal (n,11);