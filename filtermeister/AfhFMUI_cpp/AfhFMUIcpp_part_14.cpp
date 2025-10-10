                if (hBmpOld && hdcMem) {
                    SelectObject(hdcMem, hBmpOld);
                }
                // Need to DeleteObject() the bitmap that was loaded.
                /*if (hBmpDialog) {
                    if (!DeleteObject(hBmpDialog)) {
                        //ErrorOk("DeleteObject #5 failed: %8.8x", hBmpDialog);
                    }
                }*/
                // Delete the memory DC.
                if (hdcMem) {
                    DeleteDC(hdcMem);
                }
            }

#define CHECK(what) (what)

            //Delete any acquired resources in user controls...
            {//scope
                int n;
                for (n = 0;  n < N_CTLS;  n++)
                {
                    if (gParams->ctl[n].inuse) {
                        //what about hCtl, hBuddy1, hBuddy2????
                        if (gParams->ctl[n].hBrush) {
                            CHECK(DeleteObject(gParams->ctl[n].hBrush));
                            gParams->ctl[n].hBrush = NULL;
                        }
#if 0
                        //NO! Fonts are shared among controls,
                        //and NT (but not Win9x) generates a check
                        //and vanishes Photoshop!
                        if (gParams->ctl[n].hFont) {
                            CHECK(DeleteObject(gParams->ctl[n].hFont));
                            gParams->ctl[n].hFont = NULL;
                        }
#endif
                        if (gParams->ctl[n].ctlclass == CC_IMAGE) {
                            //select old bitmaps back into DC's, then
                            //delete all image bitmaps and DC's.

                            if (gParams->ctl[n].im.hbmOld) {
                                SelectObject(gParams->ctl[n].im.hdcMem, gParams->ctl[n].im.hbmOld);
                                gParams->ctl[n].im.hbmOld = 0;
                            }
                            if (gParams->ctl[n].im.hbm) {
                                CHECK(DeleteObject(gParams->ctl[n].im.hbm));
                                gParams->ctl[n].im.hbm = 0;
                            }
                            if (gParams->ctl[n].im.hdcMem) {
                                CHECK(DeleteDC(gParams->ctl[n].im.hdcMem));
                                gParams->ctl[n].im.hdcMem = 0;
                            }

                            if (gParams->ctl[n].im.hbmOldAnd) {
                                SelectObject(gParams->ctl[n].im.hdcAnd, gParams->ctl[n].im.hbmOldAnd);
                                gParams->ctl[n].im.hbmOldAnd = 0;
                            }
                            if (gParams->ctl[n].im.hbmAnd) {
                                CHECK(DeleteObject(gParams->ctl[n].im.hbmAnd));
                                gParams->ctl[n].im.hbmAnd = 0;
                            }
                            if (gParams->ctl[n].im.hdcAnd) {
                                CHECK(DeleteDC(gParams->ctl[n].im.hdcAnd));
                                gParams->ctl[n].im.hdcAnd = 0;
                            }
                        }//if CC_IMAGE
                        else if (gParams->ctl[n].ctlclass == CC_METAFILE) {
                            //set no image, and retrieve previous image
                            HENHMETAFILE hPrevImage;
                            hPrevImage = (HENHMETAFILE)SendMessage(gParams->ctl[n].hCtl,
                                                                   STM_SETIMAGE,
                                                                   IMAGE_ENHMETAFILE,
                                                                   (LPARAM)NULL);
                            //delete previous image, if any
                            if (hPrevImage) {
                                CHECK(DeleteEnhMetaFile(hPrevImage));
                            }
                        }//if CC_METAFILE
                        else if (gParams->ctl[n].ctlclass == CC_BITMAP) {
                            //set no image, and retrieve previous bitmap
                            HBITMAP hPrevImage;
                            hPrevImage = (HBITMAP)SendMessage(gParams->ctl[n].hCtl,
                                                              STM_SETIMAGE,
                                                              IMAGE_BITMAP,
                                                              (LPARAM)NULL);
                            //delete previous bitmap, if any
                            if (hPrevImage) {
                                CHECK(DeleteObject(hPrevImage));
                            }
                        }//if CC_BITMAP
                    }//if inuse
                } //for n
            }//scope

			
			//Deactivate skin
			gParams->skinActive = 0;
			gParams->skinStyle = -1;
			gParams->gDialogGradientColor1 = GetSysColor(COLOR_BTNFACE);
#if SKIN
			freeSkin();//Workaround for Serif PhotoPlus
#endif
			//Info ("Exit");

			
            // Delete the proxy RGB buffer, if any.
            if (pRGBbuffer) {
#ifndef APP
                if (pRGBbufferID != 0){
			        UnlockBuffer(pRGBbufferID);
			        FreeBuffer(pRGBbufferID); 
		        } else
#endif
			        free(pRGBbuffer);
		        pRGBbufferID = 0;
                pRGBbuffer = NULL;
				pRGBbufferSize = 0;
            }

			#ifdef GDIPLUS
			{
				void exitGraphicsPlus();
				exitGraphicsPlus();
			}
			#endif

            return FALSE;   // Not processed.


        case WM_NCHITTEST:
//#if defined(USE_BITMAP) || defined(FML)
            {
                POINT   pt;
                UINT    nHitTest = HTCLIENT;

                pt.x = LOWORD(lParam);
                pt.y = HIWORD(lParam);
                
                
                //Test if Size grip
                if (sizeGrip){
                    nHitTest = (int)DefWindowProc(hDlg, wMsg, wParam, lParam);
                    if( nHitTest == HTCLIENT ) 
                    { 
                            RECT rc; 
                            GetWindowRect(hDlg, &rc ); 
                            rc.left = rc.right - GetSystemMetrics(SM_CXHSCROLL); 
                            rc.top = rc.bottom - GetSystemMetrics(SM_CYVSCROLL); 
                        
                            if( PtInRect(&rc, pt) ){ 
                                nHitTest = HTBOTTOMRIGHT;
                                return SetDlgMsgResult(hDlg, wMsg, nHitTest);
                            }
                    }
                }


                ScreenToClient(hDlg, &pt);
                if (pt.x >= gProxyRect.left && pt.x < gProxyRect.right &&
                    pt.y >= gProxyRect.top  && pt.y < gProxyRect.bottom)
                {   // It's in our proxy preview window...
                    nHitTest = HTCLIENT;
                }
//#ifdef FML
                else if (gParams->gDialogDragMode == 2)  //Drag=None
                {
                    // Call default window proc to perform hit test...
                    nHitTest = (int)DefWindowProc(hDlg, wMsg, wParam, lParam);
                    // Change title bar hit to client hit to prevent dragging...
                    if (nHitTest == HTCAPTION)
                        nHitTest = HTCLIENT;
                }
//#endif

                else if (gParams->gDialogDragMode > 2)  //Drag=Any Control
                {
                    nHitTest = (int)DefWindowProc(hDlg, wMsg, wParam, lParam);

                    if (nHitTest == HTCLIENT){
                        
                        RECT rcCtl;

                        GetWindowRect(gParams->ctl[gParams->gDialogDragMode].hCtl, &rcCtl);
                        ScreenToClient (hDlg, (LPPOINT)&rcCtl);
                        ScreenToClient (hDlg, (LPPOINT)&(rcCtl.right));

                        //if (pt.x >= rcCtl.left && pt.x < rcCtl.right &&
                        //    pt.y >= rcCtl.top  && pt.y < rcCtl.bottom) 
                        if (pt.y < rcCtl.bottom)
                        {    
                                nHitTest = HTCAPTION;
                                //fMoving = TRUE;
                        }
                    }
                    
                }  
                else
                {
                    

#ifndef NO_TITLE_BAR
                    // Call default window proc to perform hit test...
                    nHitTest = (int)DefWindowProc(hDlg, wMsg, wParam, lParam);
                    // Change client hit to title bar hit to force dragging...
                    if (nHitTest == HTCLIENT){

#endif
//#ifdef FML
                        if (gParams->gDialogDragMode == 1){   //Drag=Background
//#endif
                              if (getAsyncKeyState(VK_LBUTTON) & 0x8000){ //Take swapped mouse buttons into account
                                    nHitTest = HTCAPTION;
                                    //fMoving = TRUE;
                              }

                        }
                    }

                }
                return SetDlgMsgResult(hDlg, wMsg, nHitTest);
            }
//#endif //USE_BITMAP||FML
            return FALSE;   // Not processed -- pass to DefWindowProc().

        case WM_SIZE:
                      
            if (fPaint) //make sure that it isn't executed before the dialog appears, otherwise Memory Error
            {
                int fwSizeType = (int)wParam;      // resizing flag 
                int nWidth = LOWORD(lParam);  // width of client area 
                int nHeight = HIWORD(lParam); // height of client area 

				int dialogWidth = PixelsToHDBUs(nWidth);
				int dialogHeight = PixelsToVDBUs(nHeight);

				if (fwSizeType == SIZE_MAXIMIZED){
                    gParams->gDialogState = 2;
				} else {
                    gParams->gDialogState = 0;
                    //save current position and size of dialog
                    GetWindowRect(hDlg, &gParams->gDialogRectMin);
                }

                //if (!fMoving) setDialogStyleEx( 0x02000000L);

				fScaling = true;
                if(!gParams->scaleLock) {
                    //Info ("Scaling Controls");
					scaleCtls(dialogWidth, dialogHeight);
                }
                fScaling = false;

				//Repaint windows
				{
					/*int i;
					for (i=0;i<N_CTLS;i++){
						if (gParams->ctl[i].inuse && gParams->ctl[i].enabled){
							//InvalidateRect(gParams->ctl[i].hCtl, NULL, FALSE);
							//InvalidateRect(gParams->ctl[i].hCtl, NULL, TRUE);
							refreshCtl(i);
						}
					}*/
				}


                fSizing=true;

                PerformControlAction(
                            fwSizeType,             //control index
                            FME_SIZE,               //FM event code
                            fwSizeType,             //previous value
                            wMsg,                   //message type
                            0,                      //notification code
                            FALSE,                  //fActionable
                            TRUE,0);                  //fProcessed

				
                //When maximizing or restoring window there is no WM_EXITSIZEMOVE event, so we have to do this now
                //Because there is no way to distinguish a restore, we have to use fMaximized
                if (fwSizeType == SIZE_MAXIMIZED || fMaximized || fwSizeType == SIZE_MINIMIZED || fMinimized){ // || fwSizeType == SIZE_RESTORED
                    
                    if (fwSizeType == SIZE_MAXIMIZED) fMaximized = TRUE;
                    else fMaximized = FALSE;

					if (fwSizeType == SIZE_MINIMIZED) fMinimized = TRUE;
                    else fMinimized = FALSE;

                    //Make sure that the preview is displayed correctly
                    GetProxyItemRect (hDlg, &gProxyRect); 
                    SetupFilterRecordForProxy (globals, &scaleFactor, scaleFactor, &imageOffset);

                    PerformControlAction(
                                0,                      //control index
                                FME_EXITSIZE,           //FM event code
                                0,                      //previous value
                                WM_EXITSIZEMOVE,        //message type
                                0,                      //notification code
                                FALSE,                  //fActionable
                                TRUE,0);                  //fProcessed