                    // Get bitmap information.
                    GetObject(hBmpDialog, sizeof(BITMAP), &bmInfo);

                    // Create a compatible memory DC using the dialog's DC.
                    hDC = GetDC(hDlg);  // release later!
                    hdcMem = CreateCompatibleDC(hDC);
                    if (!hdcMem) {
                        //failed
                        // Release the client window DC.
                        ReleaseDC(hDlg, hDC);
                        DeleteObject(hBmpDialog);
                        hBmpDialog = NULL;
                        gParams->gDialogImage_save = gDialogImage = NULL;//first, to prevent reentry
                        ErrorOk("Could not create memory DC for background image");
                        return TRUE;    //otherwise background paints too much????
                    }
                    else {
                        // Select the bitmap into the memory DC.
                        hBmpOld = (HBITMAP)SelectObject(hdcMem, hBmpDialog);
                        SetMapMode(hdcMem, GetMapMode(hDC));    //needed???

                        // Set gDialogImage to null so we don't open this file again
                        // during this invocation.
                        gDialogImage = NULL;
                    }
                    // Release the client window DC.
                    ReleaseDC(hDlg, hDC);
                }
            }
            if (hBmpDialog && gParams->gDialogImage_save)
            {   // Display a bitmap as the dialog background.
                // MUST use the DC passed in wParam, not the DC of hDlg --
                // The clipping/update regions are different!!!
                HDC hDC = (HDC) wParam; // handle of device context

                if (gParams->gDialogImageMode == DIM_STRETCHED)
                //mode=stretched
                {   // Compress/stretch the bitmap as necessary...
                    // See BDLG32 (msdn sample 4544)
                    RECT rectClient;
                    POINT pt;
                    int oldMode;

                    GetClientRect(hDlg, &rectClient);
                    size.cx = rectClient.right;     // zero-based
                    size.cy = rectClient.bottom;    // zero-based
                    pt.x = rectClient.left;
                    pt.y = rectClient.top;
                    oldMode = SetStretchBltMode(hDC, gParams->gDialogImageStretchMode);
                    StretchBlt(hDC, pt.x, pt.y, size.cx, size.cy,
                               hdcMem, 0, 0, bmInfo.bmWidth, bmInfo.bmHeight,
                               SRCCOPY);
                    SetStretchBltMode(hDC, oldMode);
                }
                else if (gParams->gDialogImageMode == DIM_TILED)
                //mode=tiled
                {   // Use multiple straight BitBlts... (a la Prosise)
                    int x, y;
                    POINT ptSize, ptOrg, ptTotalSize;
                    RECT rectClient;

                    GetClientRect(hDlg, &rectClient);
                    size.cx = rectClient.right;     // zero-based
                    size.cy = rectClient.bottom;    // zero-based
                    ptSize.x = bmInfo.bmWidth;
                    ptSize.y = bmInfo.bmHeight;
                    DPtoLP(hDC, &ptSize, 1);
                    //need to convert size.cx, size.cy to LP??
                    ptTotalSize.x = size.cx;
                    ptTotalSize.y = size.cy;
                    DPtoLP(hDC, &ptTotalSize, 1);
                    ptOrg.x = 0;
                    ptOrg.y = 0;
                    DPtoLP(hdcMem, &ptOrg, 1);
                    for (y = 0; y < ptTotalSize.y; y += ptSize.y) {
                        for (x = 0; x < ptTotalSize.x; x += ptSize.x) {
                            BitBlt(hDC, x, y, ptSize.x, ptSize.y,
                                   hdcMem, ptOrg.x, ptOrg.y, SRCCOPY);
                        }
                    }
                }
                else
                //mode=exact
                {   // Use straight BitBlt... (a la Petzold)
                    POINT   ptSize, ptOrg;
                    ptSize.x = bmInfo.bmWidth;
                    ptSize.y = bmInfo.bmHeight;
                    DPtoLP(hDC, &ptSize, 1);
                    ptOrg.x = 0;
                    ptOrg.y = 0;
                    DPtoLP(hdcMem, &ptOrg, 1);
                    BitBlt(hDC, 0, 0, ptSize.x, ptSize.y,
                           hdcMem, ptOrg.x, ptOrg.y, SRCCOPY);
                }
            }
            else
            {   // Do a gradient fill a la Prosise BitmapDemo.cpp...
                HBRUSH hBrush[64];
                int i;
                int nWidth;
                int nHeight;
                RECT rect;
                // MUST use the DC passed in wParam, not the DC of hDlg --
                // The clipping/update regions are different!!!
                HDC hDC = (HDC) wParam; // handle of device context

                //COLORREF color1 = RGB(255,0,0);
                //COLORREF color2 = RGB(0,255,0);
                //COLORREF color1 =  GetSysColor(COLOR_BTNFACE);//RGB(0xD4,0xD0,0xC8);  //yellow grey
                //COLORREF color2 =  GetSysColor(COLOR_BTNFACE);//RGB(0xD4,0xD0,0xC8);   //yellow grey

                COLORREF color1 = gParams->gDialogGradientColor1;
                COLORREF color2 = gParams->gDialogGradientColor2;
                int r1 = GetRValue(color1);
                int g1 = GetGValue(color1);
                int b1 = GetBValue(color1);
                int r2 = GetRValue(color2);
                int g2 = GetGValue(color2);
                int b2 = GetBValue(color2);

                GetClientRect(hDlg, &rect);
                nWidth = rect.right - rect.left;
                nHeight = rect.bottom - rect.top;

#ifndef NOTRACE
                Trace (globals, "WM_ERASEBKGND\n"
                                " nWidth = %d\n"
                                " nHeight = %d\n"
                                " hDC = %#x",
                                nWidth,
                                nHeight,
                                hDC);
#endif
                
                if (color1 == color2) {
                    
                    // optimize this if color1 == color2
                    hBrush[0] = CreateSolidBrush(color1);
                    FillRect(hDC, &rect, hBrush[0]);
                    
                    DeleteObject(hBrush[0]);
                    //SetTimer(hDlg, 1002, 100, NULL);
                }
                else
                {
                    // create a 64-step linear gradient
                    for (i=0; i < 64; i++)
                    {
                        hBrush[i] = CreateSolidBrush(
                            RGB( (r1*(63-i)+r2*i)/63, (g1*(63-i)+g2*i)/63, (b1*(63-i)+b2*i)/63 )
                            );
                    }

                    if (gParams->gDialogGradientDirection == 0) {
                        // vertical gradient
#if 0
                        for (i=0; i < nHeight; i++) {
                            SetRect(&rect, 0, i, nWidth, i+1);
                            FillRect(hDC, &rect, hBrush[(i*64)/nHeight]);
                        }
#else
                        for (i=0; i < 64; i++) {
                            SetRect(&rect, 0, i*nHeight/64, nWidth, (i+1)*nHeight/64);
                            FillRect(hDC, &rect, hBrush[i]);
                        }
#endif
                    }
                    else {
                        // horizontal gradient
#if 0
                        for (i=0; i < nWidth; i++) {
                            SetRect(&rect, i, 0, i+1, nHeight);
                            FillRect(hDC, &rect, hBrush[(i*64)/nWidth]);
                        }
#else
                        for (i=0; i < 64; i++) {
                            SetRect(&rect, i*nWidth/64, 0, (i+1)*nWidth/64, nHeight);
                            FillRect(hDC, &rect, hBrush[i]);
                        }
#endif
                    }

                    for (i=0; i < 64; i++)
                        DeleteObject(hBrush[i]);
                } /* color1 != color2 */
                
            }


#else

	#if 0
			//Doube Buffering - Causes slower drawing under Win7
			{
				HDC hDC = (HDC) wParam; // handle of device context
				HBRUSH hBrush;
				RECT rect;
				HDC offDC;
				HBITMAP hOldBitmap, hBitmap;
				
				offDC = CreateCompatibleDC(hDC);
				
				GetClientRect(hDlg, &rect);
				hBitmap = CreateCompatibleBitmap(hDC, rect.right-rect.left, rect.bottom-rect.top);
				hOldBitmap = SelectObject(offDC, hBitmap);
				
				hBrush = CreateSolidBrush(gParams->gDialogGradientColor1);
				SelectObject(offDC,hBrush);

				//Background color 
				FillRect(offDC, &rect, hBrush);
				DeleteObject(hBrush);

				//Apply Preview Color
				if (gParams->ctl[CTL_PREVIEW].bkColor != -1){
					//Only if image does not fill the preview
					if ( (gFmcp->x_start == 0 && gFmcp->x_end == gFmcp->X) || (gFmcp->y_start == 0 && gFmcp->y_end == gFmcp->Y) ){
						RECT rect;
						
						GetWindowRect( GetDlgItem(hDlg, 101), &rect );
						ScreenToClient (hDlg, (LPPOINT)&rect);
						ScreenToClient (hDlg, (LPPOINT)&(rect.right));

						hBrush = CreateSolidBrush(gParams->ctl[CTL_PREVIEW].bkColor);
						FillRect(offDC, &rect, hBrush);
						DeleteObject(hBrush);
					}
				}

#if SKIN
				//Erase tab background
				if (isSkinActive()) {
					int i;
					for (i = 0; i < N_CTLS; ++i) {
						if (gParams->ctl[i].ctlclass == CC_TAB){
							if (gParams->ctl[i].inuse && gParams->ctl[i].enabled & 3){
								//skinDraw(i,2);
								RECT rect;
								
								int screenX = getCtlPixelPos(i,0); 
								int screenY = getCtlPixelPos(i,1);
								int width = HDBUsToPixels(gParams->ctl[i].defwidth);
								int height = VDBUsToPixels(gParams->ctl[i].defheight);
								
								rect.left = screenX;
								rect.top = screenY;
								rect.right = screenX+width-1;
								rect.bottom = screenY+height;

								hBrush = CreateSolidBrush(skinGetColor(i));
								FillRect(offDC, &rect, hBrush);
								DeleteObject(hBrush);
							}
						}	
					}
				}
#endif

				//Output to screen	
				BitBlt(hDC, 0, 0, rect.right, rect.bottom, offDC, 0, 0, SRCCOPY);

				DeleteObject (hBitmap);
				SelectObject (offDC, hOldBitmap);
				DeleteDC (offDC);
			}

	#else


			//Clip controls to avoid flickering under Vista and 7
			if (getWindowsVersion() >= 11){ //getAppTheme() &&
				int i;
				HDC hDC = (HDC) wParam;
				RECT rect;//r,rC;
				for (i = 0; i < N_CTLS; ++i) {
					if (gParams->ctl[i].inuse && ctlEnabledAs(i)){
						if (i != CTL_FRAME && gParams->ctl[i].ctlclass != CC_IMAGE && gParams->ctl[i].ctlclass != CC_TOOLBAR && //gParams->ctl[i].ctlclass != CC_EDIT &&
							!(gParams->ctl[i].ctlclass == CC_STATICTEXT && gParams->ctl[i].tabControl<0) &&
							!(!getAppTheme() && gParams->ctl[i].ctlclass == CC_CHECKBOX) &&  //check boxes cause redraw problems with classic theme
							!(gParams->ctl[i].ctlclass == CC_GROUPBOX && (gParams->ctl[i].tabControl==-1 && gParams->ctl[i].enabled & 3))
							#if SKIN
							#ifndef SKINFLICKERFREE
								&& !(isSkinActive() && gParams->skinStyle>0 && (gParams->ctl[i].tabControl>=0 || gParams->ctl[i].ctlclass == CC_TAB))
							#endif
							#endif
						){
							GetWindowRect( gParams->ctl[i].hCtl, &rect );
							ScreenToClient (hDlg, (LPPOINT)&rect);
							ScreenToClient (hDlg, (LPPOINT)&(rect.right));
							ExcludeClipRect(hDC,rect.left,rect.top,rect.right-1,rect.bottom-1);
							/*if (gParams->ctl[i].hBuddy2 && gParams->ctl[i].buddyBkColor != -1){ //Non-transparent labels
								GetWindowRect( gParams->ctl[i].hBuddy2, &rect );
								ScreenToClient (hDlg, (LPPOINT)&rect);
								ScreenToClient (hDlg, (LPPOINT)&(rect.right));
								ExcludeClipRect(hDC,rect.left,rect.top,rect.right-1,rect.bottom-1);
							}
							if (gParams->ctl[i].hBuddy1){ //edit boxes
								GetWindowRect( gParams->ctl[i].hBuddy1, &rect );
								ScreenToClient (hDlg, (LPPOINT)&rect);
								ScreenToClient (hDlg, (LPPOINT)&(rect.right));