
                                // Choose the transparent color to be the
                                // color of the pixel at (0,0)
                                crTransparent = GetPixel(gParams->ctl[n].im.hdcMem, 0, 0);

                                // Set the background color of the image
                                // to the transparent color chosen above
                                SetBkColor(gParams->ctl[n].im.hdcMem, crTransparent);

                                // Create another memory DC for the AND mask
                                gParams->ctl[n].im.hdcAnd = CreateCompatibleDC(pdis->hDC);
                                if (!gParams->ctl[n].im.hdcAnd) {
                                        return FALSE;   //failed
                                }
                                SetMapMode(gParams->ctl[n].im.hdcAnd, GetMapMode(pdis->hDC));

                                // Create a monochrome bitmap
                                gParams->ctl[n].im.hbmAnd =
                                    CreateBitmap(gParams->ctl[n].im.bmInfo.bmWidth,
                                                 gParams->ctl[n].im.bmInfo.bmHeight,
                                                 1, 1, NULL);

                                // Select the monochrome bitmap into the AND DC
                                gParams->ctl[n].im.hbmOldAnd =
                                    (HBITMAP)SelectObject(gParams->ctl[n].im.hdcAnd,
                                                 gParams->ctl[n].im.hbmAnd);

                                // Copy the image into the monochrome DC, setting
                                // all background-colored pixels to 1, and all
                                // other pixels to 0
                                BitBlt(gParams->ctl[n].im.hdcAnd,
                                       0, 0,
                                       gParams->ctl[n].im.bmInfo.bmWidth,
                                       gParams->ctl[n].im.bmInfo.bmHeight,
                                       gParams->ctl[n].im.hdcMem,
                                       0, 0,
                                       SRCCOPY);

                                // Set transparent bits in the image to black
                                BitBlt(gParams->ctl[n].im.hdcMem,
                                       0, 0,
                                       gParams->ctl[n].im.bmInfo.bmWidth,
                                       gParams->ctl[n].im.bmInfo.bmHeight,
                                       gParams->ctl[n].im.hdcAnd,
                                       0, 0,
                                       0x220326); //dest<-dest AND NOT src
                            }
                        }
                        if ((DWORD_PTR)gParams->ctl[n].im.hbm > 1) {
                            int oldMode = SetStretchBltMode(pdis->hDC, COLORONCOLOR);
                            //Set foreground color to black and background color to
                            //white in the display DC to get desired monchrome
                            //conversion for the AND mask...
                            COLORREF oldTextColor = SetTextColor(pdis->hDC, RGB(0,0,0));
                            COLORREF oldBkColor = SetBkColor(pdis->hDC, RGB(255,255,255));
                            StretchBlt(pdis->hDC,
                                       pdis->rcItem.left,
                                       pdis->rcItem.top,
                                       pdis->rcItem.right - pdis->rcItem.left,
                                       pdis->rcItem.bottom - pdis->rcItem.top,
                                       gParams->ctl[n].im.hdcAnd,
                                       0, 0, 
                                       gParams->ctl[n].im.bmInfo.bmWidth, 
                                       gParams->ctl[n].im.bmInfo.bmHeight,
                                       SRCAND);
                            //Restore the display DC's previous foreground and background
                            //colors...
                            SetTextColor(pdis->hDC, oldTextColor);
                            SetBkColor(pdis->hDC, oldBkColor);
#if 1
                            StretchBlt(pdis->hDC,
                                       pdis->rcItem.left,
                                       pdis->rcItem.top,
                                       pdis->rcItem.right - pdis->rcItem.left,
                                       pdis->rcItem.bottom - pdis->rcItem.top,
                                       gParams->ctl[n].im.hdcMem,
                                       0, 0, 
                                       gParams->ctl[n].im.bmInfo.bmWidth, 
                                       gParams->ctl[n].im.bmInfo.bmHeight,
#if 1
                                       SRCPAINT);   //or SRCINVERT
#else
                                       SRCINVERT);   //or SRCINVERT
#endif
#endif
                            SetStretchBltMode(pdis->hDC, oldMode);
                        }
                    } //if ODA_DRAWENTIRE
                    if (pdis->itemAction & ODA_SELECT) {
                    }
                    if (pdis->itemAction & ODA_FOCUS) {
                    }
                    return TRUE;    //handled
                }//if user control
            }//scope
            return FALSE;   //not handled

#ifdef APP
#ifndef LIGHTROOM

        case WM_DROPFILES:
        {
            HDROP hdrop = (HDROP)wParam;
            char szNextFile [MAX_PATH];

            UINT uNumFiles = DragQueryFile ( hdrop, 0xFFFFFFFF, 0, 0);
            UINT uFile;
			
            if (uNumFiles==1){

                if ( DragQueryFile ( hdrop, 0, szNextFile, MAX_PATH ) > 0 ){
                    openImage(szNextFile,0,true,false);
                }

            } else {

                int openIT = (app.inputTotal==0);
				
                for (uFile = 0; uFile < uNumFiles; uFile++){
                    // Get the next filename from the HDROP info.
                    if ( DragQueryFile ( hdrop, uFile, szNextFile, MAX_PATH ) > 0 ){
                        addImageFile(szNextFile);
                    }
                }
                updateImageList();

                if (openIT)
                    openImage(app.strInputFiles,1,true,false);
                else {
                    SetupFilterRecordForProxy (globals, &fmc.scaleFactor, fmc.scaleFactor, &imageOffset);
                    //UpdateProxy(MyHDlg, TRUE);
                    //ShowOutputBuffer(globals, MyHDlg, FALSE /*fDragging*/);
                    doAction(CA_PREVIEW);
                }
 
            }
            

            // Free up memory.
            DragFinish ( hdrop );

            return 0; //processed

        }
#endif
#endif

        case WM_CTLCOLORSTATIC:
        case WM_CTLCOLORBTN:
        case WM_CTLCOLORLISTBOX:
        case WM_CTLCOLORSCROLLBAR:
//#if 0
            // Yes, this controls nonreadonly edit box colors;
            // Readonly edit boxes seem to use WM_CTLCOLORSTATIC instead.
        case WM_CTLCOLOREDIT:
//#endif

            {
                HDC hdc = (HDC) wParam;   // handle of display context
                HWND hwnd = (HWND) lParam; // handle of static control
                int iCtl;

                iCtl = GetDlgCtrlID(hwnd);    // ID of control

/*                
				if (// iCtl >= IDC_BUDDY2 && iCtl < IDC_BUDDY2+N_CTLS   // Scrollbar labels
                    //|| iCtl >= 600 && iCtl < 600+N_CHECKBOXES   // Checkboxes--NFG, not 600!
                    // || iCtl == IDC_SLIDER1          // trackbar -- NFG!!!
                       (iCtl >= 803 && iCtl <= 899) // Static text but NOT Group boxes
                       //(iCtl >= 800 && iCtl <= 899) // Static text and [NFG]Group boxes
                    || (iCtl >= IDC_TRACE && iCtl <= IDC_OPTBEV)
                    || (iCtl >= IDC_DISASSEMB && iCtl <= IDC_USE_CMOV)
                    || (iCtl == IDC_USE_INLINES)
                    //|| iCtl == 410                  // Slider1 label
                    || iCtl == 499                  // Host Name
                    || iCtl == 500                  // Filter Title
                    || iCtl == 501                  // Filter Case
                    || iCtl == 3987                 // Custom control above
                    || iCtl == 3988                 // Custom control above
                    || iCtl == 3989                 // Custom control above
                    // || iCtl == 3990                 // NFG for trackbar!!
#if 0
                    || iCtl == IDC_EMF
                    //|| iCtl == IDC_ICON1
#endif
#ifdef ALLOW_DRAGOUT
                    || iCtl == IDC_DRAGOUT
#endif
#ifdef USE_BITMAP
                    || iCtl == IDX_OK || iCtl == IDX_CANCEL
#ifdef FM_LIFTCOV
                    || iCtl == IDC_BUTTON1 || iCtl == IDC_BUTTON2   // Zoom buttons
#endif
#endif
                    )
                {   // We want these controls to be transparent...
                    // Note that these controls are NOT updated dynamically
                    // (which doesn't work right, since the backgrounds don't
                    // get erased properly).
                    SetBkMode(hdc, TRANSPARENT);
#if defined(xxxxx)
                    SetTextColor(hdc, RGB(  0,  0,  0));    //black text
#elif defined(FM_LIFTCOV)
                    SetTextColor(hdc, RGB(255,255,  0));    //yellow text
#else
                    SetTextColor(hdc, RGB(255,255,255));    //white text
#endif
                    //should really use SetDlgMsgResult...
                    return (INT_PTR) (HBRUSH) GetStockObject(HOLLOW_BRUSH); // create statically?
                }
*/
                if (iCtl >= IDC_CTLBASE && iCtl < IDC_CTLBASE+N_CTLS) {
                    // user control...
                    int n = iCtl - IDC_CTLBASE;
                    if (!gParams->ctl[n].inuse) return FALSE; //not processed
                    if (gParams->ctl[n].bkColor == -1) {
                        //transparent background
                        if (gParams->ctl[n].ctlclass == CC_STANDARD ||
							gParams->ctl[n].ctlclass == CC_SLIDER ||
                            gParams->ctl[n].ctlclass == CC_SCROLLBAR) {
                            //scrollbars are NFG with transparent body,
                            //so bail out to the Windows default handling
                            //in this case...
                            return FALSE;   //not processed
                        /*} else if (gParams->ctl[n].ctlclass == CC_SLIDER){
                            //SetBkMode(hdc, TRANSPARENT); 
                            //return (INT_PTR)GetStockObject(HOLLOW_BRUSH);
                            if (!getAppTheme()){
                                return false;
                            } else {
                                RECT clientRectangle = {0};
                                GetClientRect(gParams->ctl[n].hCtl, &clientRectangle);

                                {
                                    typedef HRESULT WINAPI ISDRAWTHEME(HWND hwnd, HDC hdc,const RECT *prc);
                                    ISDRAWTHEME* pISDRAWTHEME = NULL;
                                    HMODULE hMod = LoadLibrary("uxtheme.dll");
                                    if(hMod){
                                        pISDRAWTHEME = (ISDRAWTHEME*) GetProcAddress(hMod, "DrawThemeParentBackground");
                                        pISDRAWTHEME(gParams->ctl[n].hCtl, (HDC) wParam, &clientRectangle);
                                        FreeLibrary(hMod);
                                    }
                                }
                                return (LRESULT) GetStockObject(NULL_BRUSH);
                            }*/
                        }   
                        //else, go ahead and set transparent background mode
                        SetBkMode(hdc, TRANSPARENT);
                    }
                    else {
                        //solid background color
                        SetBkMode(hdc, OPAQUE);
                        SetBkColor(hdc, gParams->ctl[n].bkColor);
                    }
                    SetTextColor(hdc, gParams->ctl[n].textColor);
                    return (INT_PTR)gParams->ctl[n].hBrush;         //background brush
                

				} else if (iCtl >= IDC_BUDDY2 && iCtl < IDC_BUDDY2+N_CTLS) {
                    // static text buddy for a scrollbar...
                    int n = iCtl - IDC_BUDDY2;
                    if (!gParams->ctl[n].inuse) return FALSE; //not processed
                    
					/*if (gParams->ctl[n].ctlclass == CC_TOOLBAR){
						SetBkMode(hdc, OPAQUE);
                        SetBkColor(hdc, gParams->ctl[n].bkColor);
						SetTextColor(hdc, gParams->ctl[n].textColor);
						return (INT_PTR)gParams->ctl[n].hBrush;         //background brush
					}*/

					if (gParams->ctl[n].buddyBkColor == -1) {
						//for now, we always want static text buddies to have
						//a transparent background
						SetBkMode(hdc, TRANSPARENT);
						SetTextColor(hdc, gParams->ctl[n].textColor);
						return (INT_PTR)GetStockObject(HOLLOW_BRUSH);         //background brush
					} else {
						//solid background color
                        SetBkMode(hdc, OPAQUE);
                        SetBkColor(hdc, gParams->ctl[n].buddyBkColor);
						SetTextColor(hdc, gParams->ctl[n].textColor);
						return (INT_PTR)gParams->ctl[n].hBrush;         //background brush
					}
                

				} else if (iCtl >= IDC_BUDDY1 && iCtl < IDC_BUDDY1+N_CTLS) {
                    // static text buddy for a scrollbar...
                    int n = iCtl - IDC_BUDDY1;
                    if (!gParams->ctl[n].inuse) return FALSE; //not processed
                    
					if (gParams->ctl[n].editColor != -1) {
						//solid background color
                        SetBkMode(hdc, OPAQUE);
                        SetBkColor(hdc, gParams->ctl[n].editColor);
						SetTextColor(hdc, gParams->ctl[n].editTextColor);
						return (INT_PTR)gParams->ctl[n].hBrush;         //background brush
					} else
						return FALSE;
                }
