								// Shift - means optimal zoom
								scaleFactor = -888; //magic # for SetupFilterRecordForProxy.
								gFmcp->enlargeFactor = 1;
							#ifdef HIGHZOOM 
							}
							#endif
                        }
                    }
                    else if (GetKeyState(VK_CONTROL) < 0) {
                        //CONTROL modifies meaning of +/-
                        if (scaleFactor < prevScaleFactor) {
                            // Ctrl + means zoom 100 %
                            scaleFactor = 1;
							gFmcp->enlargeFactor = 1;
                        }
                        else {
                            #ifdef HIGHZOOM 
							if (scaleFactor == 1 && gFmcp->enlargeFactor == 15){
								gFmcp->enlargeFactor = 1;
							} else if (scaleFactor == 1){
								gFmcp->enlargeFactor = 16;
							} else {
							#endif
								// Ctrl + means zoom 6 %
								scaleFactor = MAX_SCALE_FACTOR;
								gFmcp->enlargeFactor = 1;
							#ifdef HIGHZOOM
							}
							#endif
                        }
                    }
                    else if (scaleFactor == 13 || scaleFactor == 15)
                    {
                        // Skip scalefactor 13 or 15...
                        if (scaleFactor > prevScaleFactor)
                            scaleFactor++;  //zooming out
                        else
                            scaleFactor--;  //zooming in
                    }
                    GetProxyItemRect (hDlg, &gProxyRect); 
                    //Should really do this in WM_TIMER, once final scale factor is set???
                    SetupFilterRecordForProxy (globals, &scaleFactor, prevScaleFactor, &imageOffset);
					#ifdef HIGHZOOM
						EnableWindow(GetDlgItem(hDlg, IDC_BUTTON1), scaleFactor > 1 || gFmcp->enlargeFactor < 16 );
					#else
						EnableWindow(GetDlgItem(hDlg, IDC_BUTTON1), scaleFactor > 1);
					#endif
                    EnableWindow(GetDlgItem(hDlg, IDC_BUTTON2), scaleFactor < MAX_SCALE_FACTOR);
                    // Should switch input focus to other button when a button
                    // gets disabled ... otherwise, can't TAB way from disabled
                    // button using keyboard only.
                    #ifdef HIGHZOOM
						if (scaleFactor == 1 && gFmcp->enlargeFactor==16) SetFocus(GetDlgItem(hDlg, IDC_BUTTON2));
					#else
						if (scaleFactor == 1) SetFocus(GetDlgItem(hDlg, IDC_BUTTON2));
					#endif
                    if (scaleFactor == MAX_SCALE_FACTOR) SetFocus(GetDlgItem(hDlg, IDC_BUTTON1));
                              
                    #if OLDZOOMLABEL || TESTSHELL
                    {
                        //Update old zoom label
                        char buff[20];
                        wsprintf(buff, "%d%%", 100*gFmcp->enlargeFactor/(gStuff->inputRate>>16));
                        SetDlgItemText(MyHDlg, 498, buff);
                    }
                    #endif

                    if (gResult == noErr) {
                        // Set a timer to trigger a delayed proxy update,
                        // giving the user a chance to click several times
                        // on the zoom button first...
                        KillTimer(hDlg, 1); //Cancel any previous trigger.
                        if (!SetTimer(hDlg, 1, 500, NULL)) {
                            // If SetTimer() fails, update the proxy immediately.
                            if (!fDragging) {
                                // Do Filtering operation
                                DoFilterRect (globals);
                                // Invalidate Proxy Item (and repaint background if scale factor
                                // increased; i.e., if proxy image may have shrunk)...
                                // (To eliminate MORE flashing, only repaint BG if shrunk, and
                                // view width or height is less than Proxy width or height.)
                                UpdateProxy(hDlg, scaleFactor > prevScaleFactor);
                            }
                        }

                    }//noErr
                
                    break;

#endif //#if OLDZOOMLABEL || TESTSHELL

              case  IDOK:
                    // Added by Ognen Genchev. Fixes not remembering last values after hitting enter key.
                    for (iCtl = 0;  iCtl < N_CTLS;  iCtl++) {
                        gParams->ctl[iCtl].initVal = gParams->ctl[iCtl].val;
                        gParams->ctl[iCtl].cancelVal = gParams->ctl[iCtl].val;
                        ScriptParam[iCtl] = gFmcp->pre_ctl[iCtl];
                        ScriptParamUsed[iCtl] = gParams->ctl[iCtl].inuse && gParams->ctl[iCtl].scripted;
                        // end
                    }
// Added by Ognen Genchev. OKdone jump has the role to skip part of the IDCANCEL code in order to prevent
// the applying operation to conflict with the canceling operation. By adding this jump, now OK button and
// Enter key have individual function from ESC key, titlebar button x and cancel button.
goto OKdone;
              case  IDCANCEL:
				  	//Trigger cancel event on ESC key or x titlebar button, but not on OK or Cancel button
					if (idd == IDCANCEL && cancelEventPerformed==0){
                        // Added by Ognen Genchev. Loop through controls to remember last values after canceling operation is performed.
                        for (iCtl = 0;  iCtl < N_CTLS;  iCtl++) {
                            gParams->ctl[iCtl].initVal = gParams->ctl[iCtl].cancelVal;
                        }
                        // end
						triggerEvent (CTL_CANCEL,FME_CLICKED,0);
						//cancelEventPerformed=1; //Do not set it to 1
					} 
/*                    else if (idd == IDOK && okEventPerformed==0){ //Trigger OK on Enter key, but not on OK or Cancel button
                        for (iCtl = 0;  iCtl < N_CTLS;  iCtl++) {
                            gParams->ctl[iCtl].initVal = gParams->ctl[iCtl].val;
                            gParams->ctl[iCtl].cancelVal = gParams->ctl[iCtl].val;
                            ScriptParam[iCtl] = gFmcp->pre_ctl[iCtl];
                            ScriptParamUsed[iCtl] = gParams->ctl[iCtl].inuse && gParams->ctl[iCtl].scripted;
                            // end
                        }
						triggerEvent (CTL_OK,FME_CLICKED,0);
					}*/
OKdone:
                    if (fDragging)
                    {
                        ReleaseCapture();
                        //SetCursor(LoadCursor(NULL, IDC_ARROW));  //???
                        fDragging = FALSE;
                        gParams->flags &= ~AFH_DRAGGING_PROXY;
                    }

                    // Preserve values in FM context record across invocations...
                    gParams->need_iuv = gFmcp->need_iuv;
                    gParams->need_d = gFmcp->need_d;
                    gParams->need_m = gFmcp->need_m;
                    gParams->need_tbuf = gFmcp->need_tbuf;
                    gParams->need_t2buf = gFmcp->need_t2buf;
					gParams->need_t3buf = gFmcp->need_t3buf;
					gParams->need_t4buf = gFmcp->need_t4buf;
                    gParams->needPremiere = gFmcp->needPremiere;
                    gParams->needPadding = gFmcp->needPadding;
                    gParams->tileHeight = gFmcp->tileHeight;//gParams->bandWidth = gFmcp->bandWidth;
                    gParams->tileWidth = gFmcp->tileWidth;
#ifndef DONT_SAVE_GLOBALS
                    /* Save the following predeclared global variables */
                    //// NO!!! Should be done AFTER the filter has run on the host image!!!!!
                    gParams->globvars = gFmcp->globvars;
#endif //DONT_SAVE_GLOBALS
                    // !!!Too bad C doesn't allow direct array assignments, e.g.:
                    ///// gParams->filterAuthorText = gFmcp->filterAuthorText;
                    memcpy(gParams->filterAuthorText, gFmcp->filterAuthorText, sizeof(gParams->filterAuthorText));
                    memcpy(gParams->filterCategoryText, gFmcp->filterCategoryText, sizeof(gParams->filterCategoryText));
                    memcpy(gParams->filterTitleText, gFmcp->filterTitleText, sizeof(gParams->filterTitleText));
                    memcpy(gParams->filterFilenameText, gFmcp->filterFilenameText, sizeof(gParams->filterFilenameText));
                    memcpy(gParams->filterCopyrightText, gFmcp->filterCopyrightText, sizeof(gParams->filterCopyrightText));
                    memcpy(gParams->filterDescriptionText, gFmcp->filterDescriptionText, sizeof(gParams->filterDescriptionText));
                    memcpy(gParams->filterOrganizationText, gFmcp->filterOrganizationText, sizeof(gParams->filterOrganizationText));
                    memcpy(gParams->filterURLText, gFmcp->filterURLText, sizeof(gParams->filterURLText));
                    memcpy(gParams->filterVersionText, gFmcp->filterVersionText, sizeof(gParams->filterVersionText));
                    memcpy(gParams->filterUniqueID, gFmcp->filterUniqueID, sizeof(gParams->filterUniqueID));

                    //save current position and size of dialog
                    GetWindowRect(hDlg, &gParams->gDialogRect);
                    
                    if (gParams->gDialogState == 0){
                        //save current position and size of dialog
                        GetWindowRect(hDlg, &gParams->gDialogRectMin);
                    }

                    //save current dialog styles
                    gParams->gDialogStyle = (int)GetWindowLong(hDlg, GWL_STYLE);
                    gParams->gDialogStyleEx = (int)GetWindowLong(hDlg, GWL_EXSTYLE);
                    gParams->gInitialInvocation = FALSE;

                    //save current dialog clipping region, if any
                    // create a dummy region first...
                    gParams->gDialogRegion = CreateRectRgn(0, 0, 0, 0);
                    if (GetWindowRgn(hDlg, gParams->gDialogRegion) == ERROR) {
                        //no clipping region
                        DeleteObject(gParams->gDialogRegion);
                        gParams->gDialogRegion = NULL;
                    }


                    gFmcp->hDlg = NULL;    //So msgBox et al. will use global handle.
                    MyHDlg = NULL;          //ditto
					
                    #if CREATEDIALOGPARAM
						//if CreateDialogParam was used to create the window
						if (idd == IDCANCEL) gResult = 1;
						PostQuitMessage(0);
						#if STANDALONE
							EndDialog(hDlg, idd); //Avoids problem with File > Exit
						#endif
					#else
						//if DialogBoxParam was used to create the window
						EndDialog(hDlg, idd);
					#endif

					fDialogDisplayed = 0; //Reset in case the plugin is not removed from memory by host

                    break;

              default:
                    return FALSE;   // not handled
            }
            break;

        default:
            return  FALSE;  // not handled
    }

    return  TRUE;   // handled; do not call defProc
} /*FilterDialogProc*/

/*****************************************************************************/


void setScaleFactor(int n){
    
    scaleFactor = n;

}



//function added by Harald Heim on Apr 2, 2002
int sub_setZoom (HWND hDlg, int n, int m)
{
    
    //static GPtr globals = NULL;         /* needs to be static */    // EH????
    long   prevScaleFactor = scaleFactor;
	static int prevEnlargeFactor;
	int retval = false;
    
    scaleFactor = n;
	gFmcp->enlargeFactor = m;

    if (scaleFactor != prevScaleFactor || gFmcp->enlargeFactor != prevEnlargeFactor){ // 
    
        GetProxyItemRect (hDlg, &gProxyRect); 
        SetupFilterRecordForProxy (globals, &scaleFactor, prevScaleFactor, &imageOffset);
		#ifdef HIGHZOOM
			EnableWindow(GetDlgItem(hDlg, IDC_BUTTON1), scaleFactor > 1 || gFmcp->enlargeFactor < 16 );
		#else
			EnableWindow(GetDlgItem(hDlg, IDC_BUTTON1), scaleFactor > 1);
		#endif
        EnableWindow(GetDlgItem(hDlg, IDC_BUTTON2), scaleFactor < 16);
        #ifdef HIGHZOOM
			if (scaleFactor == 1 && gFmcp->enlargeFactor==16) SetFocus(GetDlgItem(hDlg, IDC_BUTTON2));
		#else
			if (scaleFactor == 1) SetFocus(GetDlgItem(hDlg, IDC_BUTTON2));
		#endif
        if (scaleFactor == MAX_SCALE_FACTOR) SetFocus(GetDlgItem(hDlg, IDC_BUTTON1));

        #if OLDZOOMLABEL || TESTSHELL
        {   //Update old zoom label
            char buff[20];
            wsprintf(buff, "%d%%", 100*gFmcp->enlargeFactor/(gStuff->inputRate>>16));
            SetDlgItemText(MyHDlg, 498, buff);
        }
        #endif

        if (!fDragging) {
            DoFilterRect (MyGlobals);
            UpdateProxy(hDlg, scaleFactor > prevScaleFactor || gFmcp->enlargeFactor != prevEnlargeFactor);
        }
        
		retval = true;
    }

	prevEnlargeFactor = gFmcp->enlargeFactor;

    return retval;
}



//function added by Harald Heim on Dec 14, 2002
int sub_scrollPreview (HWND hDlg, int mode, int offsetX, int offsetY, int update)
{

    //GPtr globals = NULL;        // EH???
    POINT tempOffset;

    //gParams->flags |= AFH_DRAGGING_PROXY;
    //UpdateProxy(hDlg, FALSE);  // To display raw input.


    if (mode==0){
        imageOffset.x += offsetX * scaleFactor;
        imageOffset.y += offsetY * scaleFactor;
    } else if (mode==1){
        imageOffset.x = offsetX * scaleFactor;
        imageOffset.y = offsetY * scaleFactor;
    }
