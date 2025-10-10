                //???? Could just pre-fill the ctls array with all the
                //???? right parameters, and let "Initialize user controls"
                //???? do all the dirty work?  But would also have to do
                //???? that in resetAllCtls...
                // Ognen Genchev. Initialize custom control's state.
                iCtl = NULL;
                setCustomCtl(iCtl, gParams->ctl[iCtl].state);
                // end
                createPredefinedCtls();

// #ifndef to #ifdef modified by Ognen Genchev for proper initialization.
#ifdef ONWINDOWSTART
            } else {
                //On subsequent invocations, use coordinates that were
                //saved at previous exit...
            
                //if (gParams->gDialogState == 0){

					SetWindowPos(hDlg, NULL,
                             gParams->gDialogRect.left,
                             gParams->gDialogRect.top,
                             gParams->gDialogRect.right - gParams->gDialogRect.left,
                             gParams->gDialogRect.bottom - gParams->gDialogRect.top,
                             SWP_NOZORDER | SWP_NOACTIVATE);

                /*} else { //Dialog was maximized before
                
					SetWindowPos(hDlg, NULL,
                             gParams->gDialogRectMin.left,
                             gParams->gDialogRectMin.top,
                             gParams->gDialogRectMin.right - gParams->gDialogRectMin.left,
                             gParams->gDialogRectMin.bottom - gParams->gDialogRectMin.top,
                             SWP_NOZORDER | SWP_NOACTIVATE);

                    ShowWindow (hDlg, SW_MAXIMIZE); //Maximize Window

                }*/
            	
				// Restore/refresh dialog theme...
				//Info ("%d",gParams->gDialogTheme);
				//setDialogTheme((int) gParams->gDialogTheme);
				
                //Avoid that the XP theme is reactivated on second invocation
				//setDefaultWindowTheme(GetDlgItem(hDlg, IDC_PROGRESS1));
                //setDefaultWindowTheme(GetDlgItem(hDlg, IDC_BUTTON1));
                //setDefaultWindowTheme(GetDlgItem(hDlg, IDC_BUTTON2));
                //setDefaultWindowTheme(GetDlgItem(hDlg, IDC_PROXY_GROUPBOX));
            }

			// setDialogTheme((int) gParams->gDialogTheme);

			
			// Restore any dialog background image.
            //Need to make this pointer relative to gParams????
            gDialogImage = gParams->gDialogImage_save;

            // Restore any dialog clipping region.
            if (gParams->gDialogRegion != NULL) {
                SetWindowRgn(
                    hDlg,   // handle to window whose window region is to be set
                    gParams->gDialogRegion, // handle to region 
                    FALSE   // window redraw flag 
                       );
            }
#endif
// end
            hcHand = LoadCursor((HINSTANCE)hDllInstance, MAKEINTRESOURCE(IDC_CURSOR1));
			cursorResource = IDC_CURSOR1;


//#ifndef NOISECONTROL
            // Do this first, to compute scaleFactor...
            GetProxyItemRect (hDlg, &gProxyRect); 
            SetupFilterRecordForProxy (globals, &scaleFactor, scaleFactor, &imageOffset);
            // But don't run the filter until all other params are computed...
            //DoFilterRect (globals);

            // Set focus to OK button.
            SetFocus (GetDlgItem (hDlg, IDX_OK));

			#if OLDZOOMLABEL || TESTSHELL
				// Give the zoom buttons a more readable font...
				// (But then the focus rectangles overwrite the font????)
				SendDlgItemMessage(hDlg, IDC_BUTTON1, WM_SETFONT,
								   (WPARAM) GetStockObject(SYSTEM_FIXED_FONT),
								   FALSE /*redraw flag*/);
				SendDlgItemMessage(hDlg, IDC_BUTTON2, WM_SETFONT,
								   (WPARAM) GetStockObject(SYSTEM_FIXED_FONT),
								   FALSE /*redraw flag*/);

                // Enable/disable zoom buttons...
                EnableWindow(GetDlgItem(hDlg, IDC_BUTTON1), scaleFactor > 1);
                EnableWindow(GetDlgItem(hDlg, IDC_BUTTON2), scaleFactor < MAX_SCALE_FACTOR);
            #endif
//#endif

// ------------------------------------------------------------------------------------------------
// ------------------------------------------------------------------------------------------------

#ifndef APP
// #ifndef to #ifdef modified by Ognen Genchev for proper reinitialization.
#ifdef ONWINDOWSTART
// end
            // Initialize user controls
            if (!gParams->gInitialInvocation){       //does this break standalones???

                //Create Tab controls first
                for (iCtl = 0;  iCtl < N_CTLS;  iCtl++){
                    
                    if (gParams->ctl[iCtl].ctlclass == CC_TAB){
                    
                        int  val       = gParams->ctl[iCtl].val;
                        int  initVal   = gParams->ctl[iCtl].initVal;
                        int  minval    = gParams->ctl[iCtl].minval;
                        int  maxval    = gParams->ctl[iCtl].maxval;
                        int  lineSize  = gParams->ctl[iCtl].lineSize;
                        int  pageSize  = gParams->ctl[iCtl].pageSize;
					    int  thumbSize  = gParams->ctl[iCtl].thumbSize;
                        int  imageType = gParams->ctl[iCtl].imageType;
                        int  state = gParams->ctl[iCtl].state;
                        int  divisor   = gParams->ctl[iCtl].divisor;
                        int  ticFreq   = gParams->ctl[iCtl].tb.ticFreq;
                        CTLACTION action = gParams->ctl[iCtl].action;
                        COLORREF textColor = gParams->ctl[iCtl].textColor;
                        COLORREF bkColor = gParams->ctl[iCtl].bkColor;
                        COLORREF shapeColor = gParams->ctl[iCtl].shapeColor; // Added by Ognen Genchev
                        char tooltip[MAX_TOOLTIP_SIZE+1];
                        char image[_MAX_PATH+1];
					    int noeditborder=false;
					    int anchor = gParams->ctl[iCtl].anchor;
					    int tabControl = gParams->ctl[iCtl].tabControl;
					    int tabSheet = gParams->ctl[iCtl].tabSheet;
                        int scripted = gParams->ctl[iCtl].scripted;

                        strcpy(tooltip, gParams->ctl[iCtl].tooltip);
                        //strcpy(image, gParams->ctl[iCtl].image);

					    //if (!(gParams->ctl[iCtl].buddy1Style & WS_BORDER)) noeditborder=true;
					    
                        gParams->ctl[iCtl].inuse = FALSE;  //so fm_createCtl can work ???
                                                            //without trying to delete
                                                            //stale handles, etc. ???
                        createCtl(iCtl,
                                     gParams->ctl[iCtl].ctlclass,         //class
                                     gParams->ctl[iCtl].label,    //text
                                     gParams->ctl[iCtl].xPos,          //x
                                     gParams->ctl[iCtl].yPos,          //y
                                     gParams->ctl[iCtl].width,         //w
                                     gParams->ctl[iCtl].height,        //h
                                     gParams->ctl[iCtl].style,         //styles
                                     gParams->ctl[iCtl].styleEx,       //extended styles
                                     gParams->ctl[iCtl].properties,    //properties
                                     gParams->ctl[iCtl].enabled);      //visible/enabled
                    
                        //restore clobbered attributes
                        // setCtlTheme(iCtl, gParams->ctl[iCtl].theme); // Overwrites the background color if theme enabled
					    setCtlTab(iCtl, tabControl, tabSheet);		// Overwrites the background color if theme enabled
                        gParams->ctl[iCtl].initVal = initVal;
                        setCtlRange(iCtl, minval, maxval);
                        setCtlLineSize(iCtl, lineSize);
                        setCtlPageSize(iCtl, pageSize);
					    // setCtlThumbSize(iCtl, thumbSize);
                        setCtlAction(iCtl, action);
                        /******************************************************/
                        /*  MODIFICATION by Ognen Genchev
                        /*  fm_setCtlVal() receives value from initVal to solve
                        /*  the problem with writing value modification on
                        /*  cancelling opertion.
                        /*  See case CA_APPLY comment bellow
                        /******************************************************/
                        setCtlVal(iCtl, initVal);
                        // end
                        setCtlFontColor(iCtl, textColor);
                        setCtlColor(iCtl, bkColor);
                        setFrameColor(iCtl, shapeColor); // Added by Ognen Genchev
                        setCtlToolTip(iCtl, tooltip, 0);
                        setCtlImage(iCtl, image, imageType);
                        setCustomCtl(iCtl, state); // Added by Ognen Genchev
                        setCtlDivisor(iCtl, divisor);
					    setCtlAnchor(iCtl, anchor);
                        setCtlScripting(iCtl, scripted);
                    }
                }

			 

              //.. then create others...
              for (iCtl = 0;  iCtl < N_CTLS;  iCtl++)
              {
                if (gParams->ctl[iCtl].inuse && gParams->ctl[iCtl].ctlclass != CC_TAB)
                {
#if 1
                    //createCtl clobbers these, so save them for later
                    int  val       = gParams->ctl[iCtl].val;
                    int  initVal   = gParams->ctl[iCtl].initVal;
                    int  minval    = gParams->ctl[iCtl].minval;
                    int  maxval    = gParams->ctl[iCtl].maxval;
                    int  lineSize  = gParams->ctl[iCtl].lineSize;
                    int  pageSize  = gParams->ctl[iCtl].pageSize;
					int  thumbSize  = gParams->ctl[iCtl].thumbSize;
                    int  imageType = gParams->ctl[iCtl].imageType;
                    int  state = gParams->ctl[iCtl].state;
                    int  divisor   = gParams->ctl[iCtl].divisor;
                    int  ticFreq   = gParams->ctl[iCtl].tb.ticFreq;
                    CTLACTION action = gParams->ctl[iCtl].action;
                    COLORREF textColor = gParams->ctl[iCtl].textColor;
                    COLORREF bkColor = gParams->ctl[iCtl].bkColor;
                    char tooltip[MAX_TOOLTIP_SIZE+1];
                    char image[_MAX_PATH+1];
					int noeditborder=false;
					int anchor = gParams->ctl[iCtl].anchor;
					int tabControl = gParams->ctl[iCtl].tabControl;
					int tabSheet = gParams->ctl[iCtl].tabSheet;
                    int scripted = gParams->ctl[iCtl].scripted;

                    strcpy(tooltip, gParams->ctl[iCtl].tooltip);
                    strcpy(image, gParams->ctl[iCtl].image);

					if (!(gParams->ctl[iCtl].buddy1Style & WS_BORDER)) noeditborder=true;
					
                    gParams->ctl[iCtl].inuse = FALSE;  //so fm_createCtl can work ???
                                                        //without trying to delete
                                                        //stale handles, etc. ???
                    
					createCtl(iCtl,
                                 gParams->ctl[iCtl].ctlclass,         //class
                                 gParams->ctl[iCtl].label,    //text
                                 gParams->ctl[iCtl].xPos,          //x
                                 gParams->ctl[iCtl].yPos,          //y
                                 gParams->ctl[iCtl].width,         //w
                                 gParams->ctl[iCtl].height,        //h
                                 gParams->ctl[iCtl].style,         //styles
                                 gParams->ctl[iCtl].styleEx,       //extended styles
                                 gParams->ctl[iCtl].properties,    //properties
                                 gParams->ctl[iCtl].enabled);      //visible/enabled
					
                    //restore clobbered attributes
                    // setCtlTheme(iCtl, gParams->ctl[iCtl].theme); // Overwrites the background color if theme enabled
					setCtlTab(iCtl, tabControl, tabSheet);		// Overwrites the background color if theme enabled
                    gParams->ctl[iCtl].initVal = initVal;
                    setCtlRange(iCtl, minval, maxval);
                    setCtlLineSize(iCtl, lineSize);
                    setCtlPageSize(iCtl, pageSize);
					// setCtlThumbSize(iCtl, thumbSize);
                    setCtlAction(iCtl, action);
                    /******************************************************/
                    /*  MODIFICATION by Ognen Genchev
                    /*  fm_setCtlVal() receives value from initVal to solve
                    /*  the problem with writing value modification on
                    /*  cancelling opertion.
                    /*  See case CA_APPLY comment bellow
                    /******************************************************/
                    setCtlVal(iCtl, initVal);
                    // end
                    setCtlFontColor(iCtl, textColor);
                    setCtlColor(iCtl, bkColor);
                    setCtlToolTip(iCtl, tooltip, 0);
                    setCtlImage(iCtl, image, imageType);
                    setCustomCtl(iCtl, state); // Added by Ognen Genchev
                    setCtlDivisor(iCtl, divisor);
					setCtlAnchor(iCtl, anchor);
                    setCtlScripting(iCtl, scripted);

					//Set buddy styles
					if (gParams->ctl[iCtl].ctlclass == CC_STANDARD || gParams->ctl[iCtl].ctlclass == CC_SLIDER){
						setCtlBuddyStyle (iCtl,0,gParams->ctl[iCtl].buddy2Style);
						setCtlBuddyStyle (iCtl,1,gParams->ctl[iCtl].buddy1Style);
						setCtlBuddyStyleEx (iCtl,0,gParams->ctl[iCtl].buddy2StyleEx);
						setCtlBuddyStyleEx (iCtl,1,gParams->ctl[iCtl].buddy1StyleEx);
						if (noeditborder) clearCtlBuddyStyle (iCtl,1,WS_BORDER);
					}

                    switch (gParams->ctl[iCtl].ctlclass) {
						case CC_COMBOBOX:
						case CC_LISTBAR:
							setCtlLabel(iCtl,gParams->ctl[iCtl].label2);
							break;
						case CC_TRACKBAR:
						case CC_SLIDER:
							setCtlTicFreq(iCtl, ticFreq);
							break;
						default:
							break;
                    } //switch class


#else
                    hCtrl = GetDlgItem(hDlg, IDC_CTLBASE+iCtl);
                    if (gParams->ctl[iCtl].minval > gParams->ctl[iCtl].maxval) {
                        //range is inverted...
                        SetScrollRange(hCtrl, SB_CTL,
                                       gParams->ctl[iCtl].maxval,
                                       gParams->ctl[iCtl].minval,
                                       FALSE ); 
                        SetScrollPos(hCtrl, SB_CTL,
                                     gParams->ctl[iCtl].minval + gParams->ctl[iCtl].maxval -
                                        gParams->ctl[iCtl].val,
                                        TRUE ); //redraw
                    }
                    else {