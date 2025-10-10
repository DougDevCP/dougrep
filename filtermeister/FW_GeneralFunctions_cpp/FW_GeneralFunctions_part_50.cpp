                //update tooltips for buddies
                ti.uId = (UINT_PTR) gParams->ctl[n].hBuddy1;
                iss = (int)SendMessage(ghTT, TTM_UPDATETIPTEXT, 0, (LPARAM)&ti);
#if 1
                ti.uId = (UINT_PTR) gParams->ctl[n].hBuddy2;
                iss = (int)SendMessage(ghTT, TTM_UPDATETIPTEXT, 0, (LPARAM)&ti);
#endif
            }
        }
        else {
            //delete this tool tip
            ti.cbSize = sizeof(TOOLINFO);
            ti.uFlags = safe_style | TTF_IDISHWND;
            ti.hwnd = MyHDlg;
            ti.uId = (UINT_PTR) gParams->ctl[n].hCtl;
            //ti.rect = xxx;
            ti.hinst = (HINSTANCE)hDllInstance;
            ti.lpszText = NULL;
            iss = (int)SendMessage(ghTT, TTM_DELTOOL, 0, (LPARAM)&ti);
            if (gParams->ctl[n].ctlclass == CC_STANDARD  || gParams->ctl[n].ctlclass == CC_SLIDER) {
                //delete tooltips for buddies
                ti.uId = (UINT_PTR) gParams->ctl[n].hBuddy1;
                iss = (int)SendMessage(ghTT, TTM_DELTOOL, 0, (LPARAM)&ti);
                ti.uId = (UINT_PTR) gParams->ctl[n].hBuddy2;
                iss = (int)SendMessage(ghTT, TTM_DELTOOL, 0, (LPARAM)&ti);
            }
        }
    }

    //save tool tip text for this control
    if (iText == 0) {
        gParams->ctl[n].tooltip[0] = '\0';   //null string
    }
    else {
        strncpy(gParams->ctl[n].tooltip, (char *)iText, MAX_TOOLTIP_SIZE+1);
        gParams->ctl[n].tooltip[MAX_TOOLTIP_SIZE] = '\0';   //ensure null-terminated
    }

    return iss;    //success/failure
} /*fm_setCtlToolTip*/


int setToolTipDelay(int m, int iTime)
{

	WPARAM w;

	if (m==1)
		w = TTDT_INITIAL;
	else if (m==2)
		w = TTDT_AUTOPOP;
	else if (m==3)
		w = TTDT_RESHOW;
	else //m==0
		w = TTDT_AUTOMATIC;

	if (iTime==-1){
		// TTDT_AUTOMATIC with initial < 0 sets all values to default...
        SendMessage(ghTT, TTM_SETDELAYTIME, w, MAKELONG(-1, 0));

	} else {
		// TTDT_AUTOMATIC sets autopop = initial*10, reshow = initial/5...
        SendMessage(ghTT, TTM_SETDELAYTIME, w, MAKELONG(iTime, 0));
	}

	return true;
}


char * getCtlImage(int n)
{

	if (n < 0 || n >= N_CTLS || !gParams->ctl[n].inuse) {
        return FALSE;
    }

	//if (gParams->ctl[n].image[0] == '\0')

	return (char *)((unsigned8 *)gParams->ctl[n].image);

}


/**********************************************************************/
/* setCtlImage(n, "Image Name", 'X')
/*
/*  Sets the image (if any) associated with control n to the specified
/*  filename or embedded resource name, with image type 'X', where
/*  'X' is one of the following character constants:
/*
/*      'B' -   bitmap file (.bmp)
/*      'W' -   Windows (old-style) metafile (.wmf)
/*      'E' -   enhanced metafile (.emf)
/*      'I' -   icon file (.ico)
/*      'C' -   cursor file (.cur)
/*      'J' -   JPEG file (.jpg)
/*      'G' -   GIF file (.gif)
/*      'M' -   MIG (mouse-ivo graphics) file (.mig)
/*       0  -   unspecified file type
/*
/**********************************************************************/
int setCtlImage(int n, char * iName, int iType)
{
    int iss = TRUE;

    if (n < 0 || n >= N_CTLS || !gParams->ctl[n].inuse) {
        return FALSE;
    }

    if (gParams->ctl[n].image[0] == '\0') {
        //no current image for this control
        if (iName != 0 && ((char *)iName)[0] != '\0') {
            //set image for this control
        }
    }
    else {
        //already have an image for this control
        
        if (gParams->ctl[n].ctlclass == CC_PUSHBUTTON) {
            //set no image, and delete previous bitmap
            HBITMAP hPrevImage = (HBITMAP)SendMessage(gParams->ctl[n].hCtl,
                                              BM_SETIMAGE,
                                              IMAGE_BITMAP,
                                              (LPARAM)NULL);

        } else if (gParams->ctl[n].ctlclass == CC_IMAGE) {
            //select old bitmaps back into DC's, then
            //delete all image bitmaps and DC's.

            if (gParams->ctl[n].im.hbmOld) {
                SelectObject(gParams->ctl[n].im.hdcMem, gParams->ctl[n].im.hbmOld);
                gParams->ctl[n].im.hbmOld = 0;
            }
            if (gParams->ctl[n].im.hbm) {
                DeleteObject(gParams->ctl[n].im.hbm);
                gParams->ctl[n].im.hbm = 0;
            }
            if (gParams->ctl[n].im.hdcMem) {
                DeleteDC(gParams->ctl[n].im.hdcMem);
                gParams->ctl[n].im.hdcMem = 0;
            }

            if (gParams->ctl[n].im.hbmOldAnd) {
                SelectObject(gParams->ctl[n].im.hdcAnd, gParams->ctl[n].im.hbmOldAnd);
                gParams->ctl[n].im.hbmOldAnd = 0;
            }
            if (gParams->ctl[n].im.hbmAnd) {
                DeleteObject(gParams->ctl[n].im.hbmAnd);
                gParams->ctl[n].im.hbmAnd = 0;
            }
            if (gParams->ctl[n].im.hdcAnd) {
                DeleteDC(gParams->ctl[n].im.hdcAnd);
                gParams->ctl[n].im.hdcAnd = 0;
            }
#if 0
            //force update to reset background????
            InvalidateRect(gParams->ctl[n].hCtl, NULL, TRUE /*erase background*/);
            UpdateWindow(gParams->ctl[n].hCtl);
            //Sleep(1000);
#endif
        }
        else if (gParams->ctl[n].ctlclass == CC_METAFILE) {
            //set no image, and delete previous image
            HENHMETAFILE hPrevImage;
            hPrevImage = (HENHMETAFILE)SendMessage(gParams->ctl[n].hCtl,
                                                   STM_SETIMAGE,
                                                   IMAGE_ENHMETAFILE,
                                                   (LPARAM)NULL);
            //delete previous image, if any
/*            if (hPrevImage) {
                if (!DeleteEnhMetaFile(hPrevImage)) {
                    //ErrorOk("DeleteEnhMetaFile failed, image = %8.8x", hPrevImage);
                }
                else {
                    //Info("Deleted metafile image = %8.8x", hPrevImage);
                }
            }
*/
        }
        else if (gParams->ctl[n].ctlclass == CC_BITMAP) {
            //set no image, and delete previous bitmap
            HBITMAP hPrevImage;
            hPrevImage = (HBITMAP)SendMessage(gParams->ctl[n].hCtl,
                                              STM_SETIMAGE,
                                              IMAGE_BITMAP,
                                              (LPARAM)NULL);
            //delete previous bitmap, if any
/*            
            if (hPrevImage) {
                if (!DeleteObject(hPrevImage)) {
                    //ErrorOk("DeleteObject failed, bitmap = %8.8x", hPrevImage);
                }
                else {
                    //Info("Deleted bitmap = %8.8x", hPrevImage);
                }
            }
*/
        }
        else if (gParams->ctl[n].ctlclass == CC_ICON) {
            //set no image (icons don't get deleted)
            HICON hPrevImage;
            hPrevImage = (HICON)SendMessage(gParams->ctl[n].hCtl,
                                            STM_SETIMAGE,
                                            IMAGE_ICON,
                                            (LPARAM)NULL);
            //no need to delete previous icon (in fact, can't!)
        }
        gParams->ctl[n].image[0] = '\0';   //set no image
    }

    //save new image name for this control
    if (iName == 0) {
        gParams->ctl[n].image[0] = '\0';   //null string
    }
    else {
        strncpy(gParams->ctl[n].image, (char *)iName, _MAX_PATH+1);
        gParams->ctl[n].image[_MAX_PATH] = '\0';   //ensure null-terminated
    }
    gParams->ctl[n].imageType = iType;



    if (gParams->ctl[n].ctlclass == CC_PUSHBUTTON) {

        if (gParams->ctl[n].image[0] != '\0') {
            HBITMAP hBmp;

            // First try loading the bitmap from an embedded BITMAP resource...
            hBmp = (HBITMAP)LoadImage((HINSTANCE)hDllInstance,
                             gParams->ctl[n].image,
                             IMAGE_BITMAP,
                             0, 0,
                             LR_DEFAULTCOLOR);
            //if (hBmp) Info("Loaded bitmap from resource: %s", gParams->ctl[n].image);

            // If it's not embedded, try searching for it as an
            // external bitmap file...
            if (!hBmp) {
                // Load bitmap from file...
                hBmp = (HBITMAP)LoadImage((HINSTANCE)hDllInstance,
                                FmSearchFile(gParams->ctl[n].image),
                                IMAGE_BITMAP,
                                0, 0,
                                LR_LOADFROMFILE);
            }
            if (!hBmp) {
                ErrorOk("Could not load bitmap %s", gParams->ctl[n].image);
                return FALSE;
            }
            else {
                HBITMAP hPrevImage;
                hPrevImage = (HBITMAP)SendMessage(gParams->ctl[n].hCtl,
                                                  BM_SETIMAGE,
                                                  IMAGE_BITMAP,
                                                  (LPARAM)hBmp);
            }
        }

    } else if (gParams->ctl[n].ctlclass == CC_BITMAP) {
        if (gParams->ctl[n].image[0] != '\0') {
            HBITMAP hBmp;

            // First try loading the bitmap from an embedded BITMAP resource...
            hBmp = (HBITMAP)LoadImage((HINSTANCE)hDllInstance,
                             gParams->ctl[n].image,
                             IMAGE_BITMAP,
                             0, 0,
                             LR_DEFAULTCOLOR);
            //if (hBmp) Info("Loaded bitmap from resource: %s", gParams->ctl[n].image);

            // If it's not embedded, try searching for it as an
            // external bitmap file...
            if (!hBmp) {
                // Load bitmap from file...
                hBmp = (HBITMAP)LoadImage((HINSTANCE)hDllInstance,
                                FmSearchFile(gParams->ctl[n].image),
                                IMAGE_BITMAP,
                                0, 0,
                                LR_LOADFROMFILE);
            }
            if (!hBmp) {
                ErrorOk("Could not load bitmap %s", gParams->ctl[n].image);
                return FALSE;
            }
            else {
                HBITMAP hPrevImage;
                //Info("hBmp = %x", hBmp);
                hPrevImage = (HBITMAP)SendMessage(gParams->ctl[n].hCtl,
                                                  STM_SETIMAGE,
                                                  IMAGE_BITMAP,
                                                  (LPARAM)hBmp);
                //delete previous bitmap
/*
                if (hPrevImage) {
                    if (!DeleteObject(hPrevImage)) {
                        //ErrorOk("DeleteObject failed, bitmap = %8.8x", hPrevImage);
                    }
                    else {
                        //Info("Deleted bitmap = %8.8x", hPrevImage);
                    }