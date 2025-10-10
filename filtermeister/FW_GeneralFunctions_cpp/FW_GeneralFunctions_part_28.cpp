    /// WHY CLAMP COORDINATES?  Shouldn't the setp be ignored if out of bounds????
    if (x < fmc.x_start) x = fmc.x_start;
    else if (x >= fmc.x_end) x = fmc.x_end - 1;
    if (y < fmc.y_start) y = fmc.y_start;
    else if (y >= fmc.y_end) y = fmc.y_end - 1;


    //if ((x == fmc.X-1 && y == fmc.Y-1) || (fmc.isTileable && x == fmc.x_end-1 && y == fmc.y_end-1)) //Second condition to avoid problems in PSP8
    //{

        //// Don't need to mask bytes with 0xff (does VC optimize it away?).
        //// if Z > 3, just do 32bit store?
    
        r = val & 0xff;
        g = val >> 8 & 0xff;
        b = val >> 16 & 0xff;
        
        pos = (x-fmc.x_start)*fmc.dstColDelta + (y-fmc.y_start)*fmc.dstRowDelta;

        ((unsigned8 *)gStuff->outData) [pos] = r;
        ((unsigned8 *)gStuff->outData) [pos + 1] = g;
        ((unsigned8 *)gStuff->outData) [pos + 2] = b;

        if (fmc.Z>3){
            a =  val >> 24 & 0xff;

            if (psetpMode || a>0){ //Avoid messup of alpha channel and being forced to read it out, only if set_psetp_mode(1) was used.
                ((unsigned8 *)gStuff->outData)[pos + 3] = a;
            }
        }


    /*} else {


        *(unsigned32 *)( ((unsigned8 *)gStuff->outData) +
                                (x-fmc.x_start)*fmc.dstColDelta  +
                                (y-fmc.y_start)*fmc.dstRowDelta    )
                        =  val;
    }*/

    return 1;

}


/*
#include <Lm.h> //Requires Netapi32.lib

bool GetNewerWindowsVersion(DWORD& major, DWORD& minor)
{	

#ifndef FILTERHUB

	LPBYTE pinfoRawData;
	if (NERR_Success == NetWkstaGetInfo(NULL, 100, &pinfoRawData))
	{
		WKSTA_INFO_100 * pworkstationInfo = (WKSTA_INFO_100 *)pinfoRawData;
		major = pworkstationInfo->wki100_ver_major;
		minor = pworkstationInfo->wki100_ver_minor;
		NetApiBufferFree(pinfoRawData);
		return true;
	}

#endif

	return false;
}
*/


int getWindowsVersion(void)
{
    static int version = 0;
    
    OSVERSIONINFO osvi;
    BOOL bOsVersionInfoEx;

	if (version != 0) return version; //Use cached value


    osvi.dwOSVersionInfoSize = sizeof(OSVERSIONINFO);
    if( !(bOsVersionInfoEx = GetVersionEx ((OSVERSIONINFO *) &osvi)) ){
      osvi.dwOSVersionInfoSize = sizeof (OSVERSIONINFO);
      if (! GetVersionEx ( (OSVERSIONINFO *) &osvi) ) return FALSE;
    }

    switch (osvi.dwPlatformId) {

      //Windows NT product family.
      case VER_PLATFORM_WIN32_NT:

		
		 if ( osvi.dwMajorVersion >= 7 || (osvi.dwMajorVersion == 6 && osvi.dwMinorVersion >= 2) ){
			//For newer Windows version GetVersionEx() does not work
			DWORD major = 0; 
			DWORD minor = 0;
			
			version = 13; //Windows 8 - 10

/*
			if (GetNewerWindowsVersion(major,minor)){
				//Info("%d %d",major,minor);
				if (major==6 && minor==3)
					version = 14; //Windows 8.1
				else if ((major==6 && minor>=4) || major>=7) //major>=10
					version = 15; //Windows 10 -> version number 10.0, Win10 betas may return 6.4
			}
*/
		 } else if ( osvi.dwMajorVersion == 6 && osvi.dwMinorVersion == 1 ) //&& osvi.dwMinorVersion == 0
            version = 12; //Windows 7
         else if ( osvi.dwMajorVersion == 6) //&& osvi.dwMinorVersion == 0
            version = 11; //Windows Vista
         else if ( osvi.dwMajorVersion == 5 && osvi.dwMinorVersion == 2 )
            version = 10; //Windows Server 2003
         else if ( osvi.dwMajorVersion == 5 && osvi.dwMinorVersion == 1 )
            version = 9; //Windows XP 
         else if ( osvi.dwMajorVersion == 5 && osvi.dwMinorVersion == 0 )
            version = 8;//Windows 2000 
         else if ( osvi.dwMajorVersion <= 4 )
            version = 7;//Windows NT 

         /*
         // Test for specific product on Windows NT 4.0 SP6 and later.
         if( bOsVersionInfoEx )
         {
            // Test for the workstation type.
            if ( osvi.wProductType == VER_NT_WORKSTATION )
            {
               if( osvi.dwMajorVersion == 4 )
                  printf ( "Workstation 4.0 " );
               else if( osvi.wSuiteMask & VER_SUITE_PERSONAL )
                  printf ( "Home Edition " );
               else
                  printf ( "Professional " );
            }
            
            // Test for the server type.
            else if ( osvi.wProductType == VER_NT_SERVER )
            {
               if( osvi.dwMajorVersion == 5 && osvi.dwMinorVersion == 2 )
               {
                  if( osvi.wSuiteMask & VER_SUITE_DATACENTER )
                     printf ( "Datacenter Edition " );
                  else if( osvi.wSuiteMask & VER_SUITE_ENTERPRISE )
                     printf ( "Enterprise Edition " );
                  else if ( osvi.wSuiteMask == VER_SUITE_BLADE )
                     printf ( "Web Edition " );
                  else
                     printf ( "Standard Edition " );
               }

               else if( osvi.dwMajorVersion == 5 && osvi.dwMinorVersion == 0 )
               {
                  if( osvi.wSuiteMask & VER_SUITE_DATACENTER )
                     printf ( "Datacenter Server " );
                  else if( osvi.wSuiteMask & VER_SUITE_ENTERPRISE )
                     printf ( "Advanced Server " );
                  else
                     printf ( "Server " );
               }

               else  // Windows NT 4.0 
               {
                  if( osvi.wSuiteMask & VER_SUITE_ENTERPRISE )
                     printf ("Server 4.0, Enterprise Edition " );
                  else
                     printf ( "Server 4.0 " );
               }
            }
         }
         else  // Test for specific product on Windows NT 4.0 SP5 and earlier
         {
            HKEY hKey;
            char szProductType[BUFSIZE];
            DWORD dwBufLen=BUFSIZE;
            LONG lRet;

            lRet = RegOpenKeyEx( HKEY_LOCAL_MACHINE,
               "SYSTEM\\CurrentControlSet\\Control\\ProductOptions",
               0, KEY_QUERY_VALUE, &hKey );
            if( lRet != ERROR_SUCCESS )
               return FALSE;

            lRet = RegQueryValueEx( hKey, "ProductType", NULL, NULL,
               (LPBYTE) szProductType, &dwBufLen);
            if( (lRet != ERROR_SUCCESS) || (dwBufLen > BUFSIZE) )
               return FALSE;

            RegCloseKey( hKey );

            if ( lstrcmpi( "WINNT", szProductType) == 0 )
               printf( "Workstation " );
            if ( lstrcmpi( "LANMANNT", szProductType) == 0 )
               printf( "Server " );
            if ( lstrcmpi( "SERVERNT", szProductType) == 0 )
               printf( "Advanced Server " );

            printf( "%d.%d ", osvi.dwMajorVersion, osvi.dwMinorVersion );
         }

      // Display service pack (if any) and build number.

         if( osvi.dwMajorVersion == 4 && 
             lstrcmpi( osvi.szCSDVersion, "Service Pack 6" ) == 0 )
         {
            HKEY hKey;
            LONG lRet;

            // Test for SP6 versus SP6a.
            lRet = RegOpenKeyEx( HKEY_LOCAL_MACHINE,
               "SOFTWARE\\Microsoft\\Windows NT\\CurrentVersion\\Hotfix\\Q246009",
               0, KEY_QUERY_VALUE, &hKey );
            if( lRet == ERROR_SUCCESS )
               printf( "Service Pack 6a (Build %d)\n", osvi.dwBuildNumber & 0xFFFF );         
            else // Windows NT 4.0 prior to SP6a
            {
               printf( "%s (Build %d)\n",
                  osvi.szCSDVersion,
                  osvi.dwBuildNumber & 0xFFFF);
            }

            RegCloseKey( hKey );
         }
         else // Windows NT 3.51 and earlier or Windows 2000 and later
         {
            printf( "%s (Build %d)\n",
               osvi.szCSDVersion,
               osvi.dwBuildNumber & 0xFFFF);
         }
        */

         break;

      // Test for the Windows 95 product family.
      case VER_PLATFORM_WIN32_WINDOWS:

         if (osvi.dwMajorVersion == 4 && osvi.dwMinorVersion == 0){
             version = 2; //Windows 95
             if ( osvi.szCSDVersion[1] == 'C' || osvi.szCSDVersion[1] == 'B' )
                version = 3; //Windows 95 OSR2
         } else if (osvi.dwMajorVersion == 4 && osvi.dwMinorVersion == 10) {
             version = 4; //Windows 98 
             if ( osvi.szCSDVersion[1] == 'A' )
                version = 5;//Windows 98 SE
         } else if (osvi.dwMajorVersion == 4 && osvi.dwMinorVersion == 90){
             version = 6;//Windows ME
         } 
         break;

      //Windows 3
	  case VER_PLATFORM_WIN32s:

         version = 1; //Win32s
         break;
    }

	return version;

}


int setPreviewCursor(INT_PTR res)
{

    if (GetForegroundWindow() == MyHDlg)
        return sub_setPreviewCursor(res);
    else
        return 0;
        
}

int getPreviewCursor(void)
{

    return (int)sub_getPreviewCursor();

}


//Added by Harald Heim, March 12, 2003
int deleteRgn(INT_PTR Rgn)
{

    return DeleteObject ((HGDIOBJ) Rgn);

}


//Added by Harald Heim, May 17, 2003
/*
int comp(const void *a, const void *b )
{
    return *(int *)a - * (int *)b;
}

int quickSort(int first, int last)
{

    if (first<0 || first> N_CELLS-1) return 0;