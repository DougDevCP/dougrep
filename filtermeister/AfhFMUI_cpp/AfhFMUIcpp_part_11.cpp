								ExcludeClipRect(hDC,rect.left,rect.top,rect.right-1,rect.bottom-1);
							}*/
						}
					}
				}


			#if SKINFLICKERFREE
				/*for (i = 0; i < N_CTLS; ++i) {
					if (gParams->ctl[i].inuse && gParams->ctl[i].tabControl>=0){
						if (gParams->ctl[i].ctlclass == CC_IMAGE && ctlEnabledAs(i)){
							HRGN hClipRegion=0, hCtlRegion;
							GetWindowRect( gParams->ctl[i].hCtl, &rect );
							ScreenToClient (hDlg, (LPPOINT)&rect);
							ScreenToClient (hDlg, (LPPOINT)&(rect.right));
							//Add back
							hCtlRegion = CreateRectRgn(rect.left,rect.top,rect.right-1,rect.bottom-1);
							GetClipRgn(hDC,hClipRegion);
							CombineRgn (hClipRegion,hCtlRegion,hClipRegion,RGN_XOR);
							SelectClipRgn(hDC,hClipRegion);
						}
					}
				}*/
			#endif

			}


			/*{
				static int cycle=0;
				char string[256] = "";
				cycle++;
				sprintf(string,"%d: WM_ERASE\n",cycle);
				OutputDebugString(string);
			}*/

			//Background color
			{
				HDC hDC = (HDC) wParam;
				HBRUSH hBrush;
				RECT rect;
				GetClientRect(hDlg, &rect);
				hBrush = CreateSolidBrush(gParams->gDialogGradientColor1);
                FillRect(hDC, &rect, hBrush);
                DeleteObject(hBrush);
			}

			//Apply Preview Color
			drawPreviewColor((HDC)wParam);
			/*
			if (gParams->ctl[CTL_PREVIEW].bkColor != -1){
				//Only if image does not fill the preview
				if ( (gFmcp->x_start == 0 && gFmcp->x_end == gFmcp->X) || (gFmcp->y_start == 0 && gFmcp->y_end == gFmcp->Y) ){
					HBRUSH hBrush;
					RECT rect;
					HDC hDC = (HDC) wParam;

					GetWindowRect( GetDlgItem(hDlg, 101), &rect );
					ScreenToClient (hDlg, (LPPOINT)&rect);
					ScreenToClient (hDlg, (LPPOINT)&(rect.right));
					hBrush = CreateSolidBrush(gParams->ctl[CTL_PREVIEW].bkColor);
					FillRect(hDC, &rect, hBrush);
					DeleteObject(hBrush);
				}
			}*/


#if SKIN && !defined(GDIPLUS)

			//Erase tab background
			if (isSkinActive() && gParams->skinStyle>0) {
				int i;

#ifndef SKINFLICKERFREE
				for (i = 0; i < N_CTLS; ++i) {
					if (gParams->ctl[i].ctlclass == CC_TAB){
						if (gParams->ctl[i].inuse && gParams->ctl[i].enabled & 3){
							/*HBRUSH hBrush;
							RECT rect;
							HDC hDC = (HDC) wParam;

							int screenX = getCtlPixelPos(i,0); 
							int screenY = getCtlPixelPos(i,1);
							int width = HDBUsToPixels(gParams->ctl[i].width); //defwidth
							int height = VDBUsToPixels(gParams->ctl[i].height); //defheight
							
							rect.left = screenX;
							rect.top = screenY + skinGetTabButtonHeight();
							rect.right = screenX+width-1;
							rect.bottom = screenY+height;

							hBrush = CreateSolidBrush(skinGetColor(i));
							FillRect(hDC, &rect, hBrush);
							DeleteObject(hBrush);*/

							skinSetDC((HDC)wParam);
							skinDraw(i,2);
							
						}
					}
				}


				for (i = 0; i < N_CTLS; ++i) {
					if (gParams->ctl[i].ctlclass == CC_GROUPBOX){
						if ( gParams->ctl[i].inuse && 
							 ( ctlEnabledAs(i) || (gParams->ctl[i].tabControl==-1 && gParams->ctl[i].enabled & 3)) 
						){
						//if (IsWindowVisible(gParams->ctl[i].hCtl)){
							
							/*HDC hDC = (HDC) wParam;
							HBRUSH hBrush;
							RECT rect;
							
							int screenX = getCtlPixelPos(i,0); 
							int screenY = getCtlPixelPos(i,1);
							int width = HDBUsToPixels(gParams->ctl[i].width); //defwidth
							int height = VDBUsToPixels(gParams->ctl[i].height); //defheight
							
							rect.left = screenX;
							rect.top = screenY;
							rect.right = screenX+width;
							rect.bottom = screenY+height;

							hBrush = CreateSolidBrush(0xffffff);
							FrameRect(hDC, &rect, hBrush);
							DeleteObject(hBrush);*/

							skinSetDC((HDC)wParam);
							skinDraw(i,2);

						}
					}
				}
#endif

			}

#endif


	#endif

#endif




#if 1
            return TRUE;   // No more background painting needed.
#else
            // Seems to make things worse...
            return SetDlgMsgResult(hDlg, wMsg, TRUE);   // No more background painting needed.
#endif


#ifdef FOCALBLADE

        case WM_MENUSELECT:
            {
                int n = LOWORD(wParam);   // menu item or submenu index 
                int fuFlags = HIWORD(wParam); // menu flags 
                HMENU hmenu = (HMENU) lParam;          // handle of menu clicked 
                int previous = 0;

                if (GetSubMenu(hmenu,n) != NULL) previous = 1;

                PerformControlAction(
                                    n,                      //control index
                                    FME_MENUSELECT,           //FM event code
                                    previous,                  //previous value -> is sub menu if nonzero
                                    WM_MENUSELECT,            //message type
                                    0,                      //notification code
                                    FALSE,                  //fActionable
                                    TRUE,1);                  //fProcessed

                return false;
            }
#endif

        case WM_DRAWITEM:
            {//scope
                LPDRAWITEMSTRUCT pdis = (LPDRAWITEMSTRUCT)lParam;
                if (pdis->CtlID >= IDC_CTLBASE && pdis->CtlID < IDC_CTLBASE+N_CTLS) {
                    int n = pdis->CtlID - IDC_CTLBASE;
                    
                    // Added by Ognen Genchev
                    COLORREF shapeColor = gParams->ctl[n].shapeColor;
                    // end

					//if (n==CTL_PREVIEW) Info ("%d",gParams->ctl[n].inuse,!(gParams->ctl[n].enabled != 3));
					//if (n==CTL_PROGRESS) Info ("Progress"); 

#if SKIN
					if (isSkinActive() && isSkinCtl(n)){ // && gParams->ctl[n].ctlclass != CC_TAB // && gParams->ctl[n].ctlclass != CC_GROUPBOX
						
						skinDraw(n,0);

					} else 
#endif					
                        // Added by Ognen Genchev
                        if (gParams->ctl[n].inuse && (gParams->ctl[n].properties & CTP_RECTANGLE) && !(gParams->ctl[n].enabled != 3)) {

                            // Draw rectangular frame
                            FrameRect(pdis->hDC, &pdis->rcItem, CreateSolidBrush(shapeColor));
                            
                                // Draw triangle
    /*                      cx = pdis->rcItem.right  - pdis->rcItem.left;
                            cy = pdis->rcItem.bottom - pdis->rcItem.top;

                            ptt[0].x = 3 * cx / 8 ;  ptt[0].y = 1 * cy / 8;
                            ptt[1].x = 5 * cx / 8 ;  ptt[1].y = 1 * cy / 8;
                            ptt[2].x = 4 * cx / 8 ;  ptt[2].y = 3 * cy / 8;

                            Triangle (pdis->hDC, ptt);*/

                            PerformControlAction(
                                    n,                      //control index
                                    FME_RECTANGLE,           //FM event code
                                    0,                      //previous value
                                    WM_DRAWITEM,            //message type
                                    0,                      //notification code
                                    FALSE,                  //fActionable
                                    TRUE,0);                  //fProcessed
                        }
                        // end

						if (gParams->ctl[n].inuse && (gParams->ctl[n].properties & CTP_DRAWITEM) && !(gParams->ctl[n].enabled != 3) ){
                    
                        PerformControlAction(
                                n,                      //control index
                                FME_DRAWITEM,           //FM event code
                                0,                      //previous value
                                WM_DRAWITEM,            //message type
                                0,                      //notification code
                                FALSE,                  //fActionable
                                TRUE,0);                  //fProcessed
                    }
                    
                    //It's a user-defined control
                    if (gParams->ctl[n].ctlclass != CC_IMAGE) {
                        return FALSE;       //for now
                    }
#if 1
                    //Info("WM_DRAWITEM: ctl = %d", n);
                    //InvalidateRect(pdis->hwndItem, NULL, TRUE); //Infinite loop!
#endif
                    if (gParams->ctl[n].image[0] == '\0') {
                        //no image
                        return FALSE;       //not handled
                    }
                    if (pdis->itemAction & ODA_DRAWENTIRE) {
                        //stretch a bitmap

                        if (gParams->ctl[n].im.hbm == 0) {
                            //first time through, load bitmap into a memory DC
                            //first try to load from RT_BITMAP resource...
                            gParams->ctl[n].im.hbm = (HBITMAP)LoadImage((HINSTANCE)hDllInstance,
                                                               gParams->ctl[n].image,
                                                               IMAGE_BITMAP,
                                                               0, 0,
                                                               LR_DEFAULTCOLOR);
                            if (gParams->ctl[n].im.hbm == 0) {
                                //if not in resources, load from file...
                                gParams->ctl[n].im.hbm = (HBITMAP)LoadImage((HINSTANCE)hDllInstance,
                                                                   FmSearchFile(gParams->ctl[n].image),
                                                                   IMAGE_BITMAP,
                                                                   0, 0,
                                                                   LR_LOADFROMFILE);
                            }
                            if (gParams->ctl[n].im.hbm == 0) {
                                gParams->ctl[n].im.hbm = (HBITMAP) 1; //1-shot stopper

                                return FALSE;   //failed
                            }
                            else {
                                COLORREF crTransparent;

#if SKIN
								//Invert Bitmpap?
								if (isSkinActive() && skinGetInvertBitmaps()){
									if (strstr(gParams->ctl[n].image,"_off") || strstr(gParams->ctl[n].image,"_grey") || strstr(gParams->ctl[n].image,"arrow_"))
										gParams->ctl[n].im.hbm = (HBITMAP)(INT_PTR)invertBitmap(gParams->ctl[n].im.hbm,NULL);
								}
#endif

                                // Get bitmap information.
                                GetObject(gParams->ctl[n].im.hbm, sizeof(BITMAP),
                                          &gParams->ctl[n].im.bmInfo);

                                // Create a compatible memory DC.
                                gParams->ctl[n].im.hdcMem = CreateCompatibleDC(pdis->hDC);
                                if (!gParams->ctl[n].im.hdcMem) {
                                        return FALSE;   //failed
                                }
                                SetMapMode(gParams->ctl[n].im.hdcMem, GetMapMode(pdis->hDC));

                                // Select the bitmap into the memory DC.
                                gParams->ctl[n].im.hbmOld = (HBITMAP)SelectObject(gParams->ctl[n].im.hdcMem,
                                                                         gParams->ctl[n].im.hbm);