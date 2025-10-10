                }
                //DeleteObject(hBmp);    //do this at end of dialog???
*/
#if 0
                return TRUE;        //unless need to force window update below...
#endif
            }
        }
    }//if CC_BITMAP

    else if (gParams->ctl[n].ctlclass == CC_ICON) {
        if (gParams->ctl[n].image[0] != '\0') {
            HICON   hIcon;

            // First try loading the icon from an embedded RT_ICON resource...
            hIcon = (HICON)LoadImage((HINSTANCE)hDllInstance,
                              gParams->ctl[n].image,
                              IMAGE_ICON,
                              0, 0,
                              LR_DEFAULTCOLOR);
            //if (hIcon) Info("Loaded icon from resource: %s", gParams->ctl[n].image);
            //Info("Loaded icon from resource: %8.8x", hIcon);

            // If it's not embedded, try searching for it as an
            // external icon file...
            if (!hIcon) {
                // Load icon from file...
                hIcon = (HICON)LoadImage((HINSTANCE)hDllInstance,
                                 FmSearchFile(gParams->ctl[n].image),
                                 IMAGE_ICON,
                                 0, 0,
                                 LR_LOADFROMFILE);
                //Info("Loaded icon from file: %8.8x", hIcon);
            }
            if (!hIcon) {
                ErrorOk("Could not load icon %s", gParams->ctl[n].image);
                return FALSE;
            }
            else {
                HICON hPrevImage;
                //Info("hIcon = %x", hIcon);
                hPrevImage = (HICON)SendMessage(gParams->ctl[n].hCtl,
                                                STM_SETIMAGE,
                                                IMAGE_ICON,
                                                (LPARAM)hIcon);
                //no need to delete previous icon (in fact, can't!)
#if 0
                return TRUE;        //unless need to force window update below...
#endif
            }
        }
    }//if CC_ICON

    else if (gParams->ctl[n].ctlclass == CC_METAFILE) {
      if (gParams->ctl[n].image[0] != '\0') {
        //enhanced metafile static control....
        HENHMETAFILE hImage = NULL;
        FILE *file;

        // Read from resource
        // Could also use LoadIMage (  IMAGE_ENHMETAFILE... ) ???
        HRSRC hRes;         // handle/ptr. to res. info.
        HRSRC hResLoad;     // handle to loaded resource  
        char *pFmMf;        // pointer to FMMETAFILE resource data 
        int cb;             // Size of resource (bytes)

        // Try to locate FMMETAFILE resource first...
        hRes = FindResource((HMODULE)hDllInstance, gParams->ctl[n].image, "FMMETAFILE"); 
        //Info("hRes = %x", hRes);
        if (hRes != NULL) { 
            // Get size of resource.
            cb = SizeofResource((HMODULE)hDllInstance, hRes);
            if (cb < 100) {
                //Implausible size
                ErrorOk("FMMETAFILE resource is too short (%d bytes)", cb);
            }
            else {
                // Load the FMMETAFILE resource into global memory. 
                hResLoad = (HRSRC)LoadResource((HMODULE)hDllInstance, hRes);
                //Info("hResLoad = %x", hResLoad);
                if (hResLoad == NULL) { 
                    ErrorOk("Could not load FMMETAFILE resource.");
                } 
                else {
                    // Lock the FMMETAFILE resource into global memory. 
                    pFmMf = (char *)LockResource(hResLoad); 
                    //Info("pFmMf = %x", pFmMf);
                    if (pFmMf == NULL) { 
                        ErrorOk("Could not lock FMMETAFILE resource.");
                    }
                    else {
                        // Determine what kind of metafile..,
                        if (((LPENHMETAHEADER)pFmMf)->iType == EMR_HEADER &&
                            ((LPENHMETAHEADER)pFmMf)->dSignature == ENHMETA_SIGNATURE) {
                            //It's an enhanced metafile
                            hImage = SetEnhMetaFileBits(cb, (BYTE *)pFmMf);
                        }
                        else {
                            //Probably a Windows metafile
                            if (*(DWORD *)pFmMf == 0x9AC6CDD7) {
                                //metafile has an Aldus placeable header
                                //--skip over it...
                                pFmMf += 22;
                                cb -= 22;
                                //Info("WMF adjusted 1st dword = %x", *(DWORD *)pFmMf);
                            }
#if 0
                            if (((LPMETAHEADER)pFmMf)->mtSize * 2 != (size_t)cb) {
                                ErrorOk("FMMETAFILE resource is wrong size (%d instead of %d)",
                                    cb, ((LPMETAHEADER)pFmMf)->mtSize * 2);
                            }
#else
                            if (((LPMETAHEADER)pFmMf)->mtSize * 2 > (size_t)cb) {
                                ErrorOk("FMMETAFILE resource is too small (%d instead of %d)",
                                    cb, ((LPMETAHEADER)pFmMf)->mtSize * 2);
                            }
#endif
                            else {
                                hImage = SetWinMetaFileBits(cb, (BYTE *)pFmMf,
                                                            NULL /*hdcRef*/,
                                                            NULL /*lpmfp*/);
                            }
                        }
                    }
                }
            }
        } //if hRes != NULL
        else {
            /* Search for metafile file in PATH and FM_PATH environment variables: */
            if ((file = fopen(FmSearchFile(gParams->ctl[n].image), "rb"))) {
                // Read from *.wmf (Plain Windows or Aldus Placeable) file
                // or *.emf (enhanced metafile).

                // Determine file length...
                fseek(file, 0, SEEK_END);
                cb = ftell(file);
                rewind(file);

                if (!(pFmMf = (char *)malloc(cb)))
                {
                    fclose(file);
                    ErrorOk("Could not allocate %d bytes for metafile", cb);
                }
                else {
                    //Now get the actual data bits...
                    cb = (int)fread(pFmMf, 1, cb, file);
                    //check for correct read length???
                    if (cb < 100) {
                        //implausible file size
                        ErrorOk("Metafile is too short (%d bytes)", cb);
                    }
                    // Determine what kind of metafile..,
                    else if (((LPENHMETAHEADER)pFmMf)->iType == EMR_HEADER &&
                        ((LPENHMETAHEADER)pFmMf)->dSignature == ENHMETA_SIGNATURE) {
                        //It's an enhanced metafile
                        hImage = SetEnhMetaFileBits(cb, (BYTE *)pFmMf);
                    }
                    else {
                        //Info("WMF 1st dword = %x", *(DWORD *)pFmMf);
                        if (*(DWORD *)pFmMf == 0x9AC6CDD7) {
                            //file has an Aldus placeable header
                            //--skip over it...
                            pFmMf += 22;
                            cb -= 22;
                            //Info("WMF adjusted 1st dword = %x", *(DWORD *)pFmMf);
                        }
#if 0
                        if (((LPMETAHEADER)pFmMf)->mtSize * 2 != (size_t)cb) {
                            ErrorOk("Metafile is wrong size (%d instead of %d)",
                                cb, ((LPMETAHEADER)pFmMf)->mtSize * 2);
                        }
#elif 1
                        if (((LPMETAHEADER)pFmMf)->mtSize * 2 > (size_t)cb) {
                            ErrorOk("Metafile is too small (%d instead of %d)",
                                cb, ((LPMETAHEADER)pFmMf)->mtSize * 2);
                        }
#else
                        if (((LPMETAHEADER)pFmMf)->mtSize * 2 != (size_t)cb &&
                            IDOK != Warn("Metafile is wrong size (%d instead of %d)",
                                         cb, ((LPMETAHEADER)pFmMf)->mtSize * 2)) {
                            //bad size and user pressed Cancel...
                            ; //do nothiing (so hImage == NULL)
                        }
#endif
                        else {
                            hImage = SetWinMetaFileBits(cb, (BYTE *)pFmMf,
                                                        NULL /*hdcRef*/,
                                                        NULL /*lpmfp*/);
                        }
                    }

                    free(pFmMf);
                }//if malloc

                fclose(file);
            }//if fopen
        }

        if (!hImage) {
            ErrorOk("Could not load metafile %s", gParams->ctl[n].image);
            return FALSE;
        }
        else {
            HENHMETAFILE hPrevImage;
            //Info("hImage = %x", hImage);
            //Info("EMF 1st dword = %x", *(DWORD *)hImage);
            hPrevImage = (HENHMETAFILE)SendMessage(gParams->ctl[n].hCtl,
                                                   STM_SETIMAGE,
                                                   IMAGE_ENHMETAFILE,
                                                   (LPARAM)hImage);
            //delete previous image
/*
            if (hPrevImage) {
                if (!DeleteEnhMetaFile(hPrevImage)) {
                    //ErrorOk("DeleteEnhMetaFile failed, image = %8.8x", hPrevImage);
                }
                else {
                    //Info("Deleted metafile image = %8.8x", hPrevImage);
                }
            }
*/
            //DeleteEnhMetaFile(hImage);    //do this at end of dialog???
        }
      }//if image not null
      ///////return TRUE;  //unless need to force refresh below...
      //fall through to update control...
    }//if CC_METAFILE


    //FORCE CONTROL UPDATE IF NEEDED!!!
#if 0
    //This doesn't work...
    InvalidateRect(gParams->ctl[n].hCtl, NULL, TRUE /*erase background*/);
    return iss && UpdateWindow(gParams->ctl[n].hCtl);
#elif 0
    //Try updating entire dialog!
    InvalidateRect(fmc.hDlg, NULL, TRUE /*erase background*/);
    return iss && UpdateWindow(fmc.hDlg);
#elif 0
    //This doesn't do it either...
    return iss && setCtlPos(n, -1, -1, -1, -1);
#elif 1
    //This works, but is pretty heavy-handed!!!
    setCtlPos(n, gParams->ctl[n].xPos+1, -1, -1, -1);
    setCtlPos(n, gParams->ctl[n].xPos-1, -1, -1, -1);
    InvalidateRect(gParams->ctl[n].hCtl, NULL, TRUE /*erase background*/);
    return iss && UpdateWindow(gParams->ctl[n].hCtl);
#elif 0
    //This works, but is pretty heavy-handed!!!
    setCtlPos(n, gParams->ctl[n].xPos+1, -1, -1, -1);
    setCtlPos(n, gParams->ctl[n].xPos-1, -1, -1, -1);
    return iss;
#else
    return iss;    //success/failure
#endif
} /*fm_setCtlImage*/

/**********************************************************************/
/* setCtlTicFreq(n, m)
/*
/*  Sets the frequency with which tick marks are displayed for slider
/*  control n. For example, if the frequency is set to 2, a tick mark
/*  is displayed for every other increment in the slider�s range. The
/*  default setting for the frequency is 1 (that is, every increment 
/*  in the range is associated with a tick mark).
/*
/**********************************************************************/
int setCtlTicFreq(int n, int m)
{
    if (n < 0 || n >= N_CTLS || !gParams->ctl[n].inuse) {
        return -1;
    }

    if (gParams->ctl[n].ctlclass == CC_TRACKBAR || gParams->ctl[n].ctlclass == CC_SLIDER) {
        gParams->ctl[n].tb.ticFreq = m;
        SendMessage(gParams->ctl[n].hCtl, TBM_SETTICFREQ, m /*freq*/, 0);
        return TRUE;
    }
    else {
        //not a TRACKBAR
        return FALSE;
    }
} /*fm_setCtlTicFreq*/

/**********************************************************************/
/* setWindowTheme(hwnd, pszSubAppName, pszSubIdList)
/*
/*  Sets the Visual Theme for a given window to the specified theme.
/*
/*  Calls the following XP API function (if it exists):
/*
/*  THEMEAPI SetWindowTheme(HWND hwnd, LPCWSTR pszSubAppName, 
/*    LPCWSTR pszSubIdList);
/*
/*  N.B.  The 2nd and 3rd args must be UNICODE strings!
/**********************************************************************/
//#include <Uxtheme.h>

/*typedef HRESULT (STDAPICALLTYPE *LPSETWINDOWTHEME) (HWND, LPCWSTR, LPCWSTR);
