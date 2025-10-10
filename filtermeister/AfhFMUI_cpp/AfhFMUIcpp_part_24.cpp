    GetProxyItemRect (hDlg, &gProxyRect);
    tempOffset = imageOffset;
	
    SetupFilterRecordForProxy (globals, &scaleFactor, scaleFactor, &tempOffset ); //&tempOffset
    /*if ( ((gParams->flags & AFH_ZERO_OUTPUT_DRAG) == AFH_ZERO_OUTPUT_DRAG) && gFmcp->x_start == 0 && gFmcp->y_start == 0 && gFmcp->x_end == gFmcp->X && gFmcp->y_end == gFmcp->Y ){
        
         //Temporarily deactivate Zero drag to avoid redraw problem
         gParams->flags &= ~AFH_ZERO_OUTPUT_DRAG;
         SetupFilterRecordForProxy (globals, &scaleFactor, scaleFactor, &tempOffset);
         //Activate Zero drag again
         gParams->flags |= AFH_ZERO_OUTPUT_DRAG;

    }else {
        SetupFilterRecordForProxy (globals, &scaleFactor, scaleFactor, &tempOffset);
    }*/


    //GetProxyItemRect (hDlg, &gProxyRect);
    //gParams->flags &= ~AFH_DRAGGING_PROXY;
    
    if (update && gResult == noErr) {
        // Run the filter only if want to drag the filtered output
        // rather than the raw input...
        if (gParams->flags & AFH_DRAG_FILTERED_OUTPUT)
        {
            DoFilterRect (globals);
        }
        UpdateProxy(hDlg, FALSE);
    }//noErr
    

    return 1;
}



//function added by Harald Heim on January, 2003
int sub_setPreviewCursor (INT_PTR res)
{
    HCURSOR hcHand_old = hcHand;

	// How about loading a named resource????

	//HCURSOR
	if (res >= 32700){
		//Info ("cursor set: %d", res);
		hcHand = (HCURSOR)(INT_PTR)res;
    //Standard Windows Cursors
	} else if (res >= 32512)
        hcHand = LoadCursor(NULL,  MAKEINTRESOURCE(res));
    //Load from Resource
    else
        hcHand = LoadCursor((HINSTANCE)hDllInstance, MAKEINTRESOURCE(res));

    if (hcHand != NULL){
            DestroyCursor(hcHand_old);
            cursorResource = res;
			SetCursor(hcHand);
            return true;
    }

    return false;
}

INT_PTR sub_getPreviewCursor (void)
{

	return cursorResource;

}


/*****
Similarly, by default the TAB key moves the input focus to the next control in the tabbing sequence. 
However, it might be useful for the user to be able to enter TAB characters in an edit control. By 
default, multiple-line edit controls accept CTRL+TAB to enter TAB characters in an edit control.
The following sections describe four methods that can be used to implement alternative behavior for 
the ENTER and TAB keys. It should be noted that some users may object if the ENTER and TAB keys no 
longer act in the default fashion.

Use ES_WANTRETURN in Windows version 3.1

Windows version 3.1 supplies a new edit control style, ES_WANTRETURN, which causes an ENTER key to 
advance to a new line and not be passed to the dialog manager. ES_WANTRETURN does not affect TAB keys.

Subclass the edit control

An edit control subclass procedure could respond to the WM_GETDLGCODE message with DLGC_WANTALLKEYS 
to receive all keyboard input. This causes the edit control to be sent all key input, including ENTER 
and TAB keys. The following subclass procedure could be used:
  
LONG FAR PASCAL AllKeyFunc(HWND hWnd, WORD Message, WORD wParam,
   LONG lParam)
{
   //
   // This subclass function makes pressing an ENTER key
   // create a new line when the focus is in an edit control.
   //

   if ( Message == WM_GETDLGCODE )
      return DLGC_WANTALLKEYS;

   return CallWindowProc(lpfnOldClassProc, hWnd, Message, wParam,
      lParam);
}

  ****/

/****
FOLLOWING SHOWS HOW TO SUBCLASS:

Here is how to make an edit control read-only in Windows version 3.0:
  
// Declare the subclass function.
long FAR PASCAL SubEditWndProc(HWND hWnd, unsigned wMessage,
                WORD wParam, LONG lParam);

//
// Create an edit control.
//
hWndEdit=CreateWindow("EDIT",
                  "Edit Control Line Numbers",
                  WS_OVERLAPPEDWINDOW,
                  35, 35, 400, 150,
                  NULL, NULL, hInstance, NULL);
//
// Subclass the edit control.
//
static FARPROC     lpSubClassProc;

lpSubClassProc=MakeProcInstance((FARPROC)SubEditWndProc,hInstance);
     SetWindowLong(hWndEdit, GWL_WNDPROC, (LONG)lpSubClassProc);
.
.
.

//
// Read-only subclass function for an edit control.
//
long FAR PASCAL SubEditWndProc(HWND hWnd, unsigned wMessage, WORD
   wParam, LONG lParam)
{
   switch ( wMessage ) {
      case WM_CUT:
      case WM_KEYDOWN:
      case WM_PASTE:
      case WM_CHAR:
         // Do nothing here.
      break;

      default:
         CallWindowProc (lpPrevWndFunc, hWnd, wMessage, wParam,
                         lParam );
   }
}

****/

/*****************************************************************************/
void PromptUserForInput (GPtr globals)
{

#if APP
	HWND hParent = NULL;
#else
	PlatformData *platform = (PlatformData *)((FilterRecordPtr) gStuff)->platformData;
	HWND hParent = (HWND)platform->hwnd;
#endif

    /* Query the user for parameters. */

#if CREATEDIALOGPARAM
	
	HWND hDlg;
	MSG msg;
	int retval;

	hDlg = CreateDialogParam(
					   (HINSTANCE)hDllInstance,
					   (LPSTR)"FILTERPARAM",
					   hParent,
					   (DLGPROC)FilterDialogProc,
					   (LPARAM)globals);


	//ShowWindow(hDlg, SW_HIDE);
	ShowWindow(hDlg, SW_SHOW);

	//Message Loop - process keyboard input
	while (GetMessage(&msg, NULL, 0, 0) 
#ifndef APP
		&& (fmc.hostSig!='PSPI' || (fmc.hostSig=='PSPI' && fmc.hDlg))
#endif
	){ //&& fmc.hDlg = PSP 6 Fix
		//if (hAcceleratorTable) if (TranslateAccelerator(hDlg, hAcceleratorTable, &msg)) { Info("translated"); continue;}
		if (msg.message == WM_KEYDOWN || msg.message == WM_SYSKEYDOWN) {
			//if (msg.wParam != VK_SHIFT && msg.wParam != VK_CONTROL){ 
				
#if SKIN
				if (!isSkinActive())
#endif				
				{
					if ( (msg.wParam == '0' || msg.wParam == VK_NUMPAD0) && getAsyncKeyStateF(VK_CONTROL)>=0){
						int iCtl = getCtlFocus();
						if (iCtl!=-1){
							if (gParams->ctl[iCtl].ctlclass == CC_STANDARD){
								int oldvalue = gParams->ctl[iCtl].val;
								setCtlVal(iCtl,0);
								triggerEvent(iCtl,FME_CHANGED,oldvalue);
								doAction(gParams->ctl[iCtl].action);
								continue; //No further processing
							}
						}
					}

				} 
#if SKIN
				else { //active skin

					if (msg.wParam == VK_LEFT || msg.wParam == VK_RIGHT || msg.wParam == VK_UP || msg.wParam == VK_DOWN || 
						msg.wParam == VK_NEXT|| msg.wParam == VK_PRIOR || 
						//msg.wParam == '0' || msg.wParam == VK_NUMPAD0 || 
						((msg.wParam == '0' || msg.wParam == VK_NUMPAD0) && getAsyncKeyStateF(VK_CONTROL)>=0) ||
						msg.wParam == VK_HOME|| msg.wParam == VK_END
					){ 
						//POINT pt;
						//HWND hCtl;
						//Info ("%d",getCtlFocus());
						int iCtl = getCtlFocus();
						//GetCursorPos(&pt);
						//hCtl = WindowFromPoint(pt); //int n = getCtlFocus(); //hCtl = ChildWindowFromPointEx(hDlg,pt,CWP_SKIPDISABLED|CWP_SKIPINVISIBLE|CWP_SKIPTRANSPARENT);
						//if (hCtl!=NULL){
							//int i,iCtl=-1;
							//for (i=0;i<N_CTLS;i++) if (hCtl == gParams->ctl[i].hCtl) {iCtl=i; break;}
							if (iCtl!=-1){
								if (gParams->ctl[iCtl].ctlclass == CC_STANDARD){
									int oldvalue = gParams->ctl[iCtl].val;
									if (msg.wParam==VK_LEFT||msg.wParam==VK_UP) setCtlVal(iCtl,ctl(iCtl)-gParams->ctl[iCtl].lineSize);
									else if (msg.wParam==VK_RIGHT||msg.wParam==VK_DOWN) setCtlVal(iCtl,ctl(iCtl)+gParams->ctl[iCtl].lineSize);
									else if (msg.wParam==VK_NEXT) setCtlVal(iCtl,ctl(iCtl)+gParams->ctl[iCtl].pageSize);
									else if (msg.wParam==VK_PRIOR) setCtlVal(iCtl,ctl(iCtl)-gParams->ctl[iCtl].pageSize);
									else if (msg.wParam==VK_HOME) setCtlVal(iCtl,gParams->ctl[iCtl].minval );
									else if (msg.wParam==VK_END) setCtlVal(iCtl,gParams->ctl[iCtl].maxval);
									else if (msg.wParam==0x30 || msg.wParam==VK_NUMPAD0) setCtlVal(iCtl,0); //VK_0
									triggerEvent(iCtl,FME_CHANGED,oldvalue);
									doAction(gParams->ctl[iCtl].action);
									continue; //No further processing
								} else if (gParams->ctl[iCtl].ctlclass == CC_COMBOBOX){
									int oldvalue = gParams->ctl[iCtl].val;
									if (msg.wParam==VK_UP) setCtlVal(iCtl,max(0,ctl(iCtl)-1));
									else if (msg.wParam==VK_DOWN) setCtlVal(iCtl,min(getCtlItemString(iCtl,-1,NULL)-1,ctl(iCtl)+1));
									else if (msg.wParam==VK_HOME) setCtlVal(iCtl,0 );
									else if (msg.wParam==VK_END) setCtlVal(iCtl,getCtlItemString(iCtl,-1,NULL)-1);
									triggerEvent(iCtl,FME_CLICKED,oldvalue);
									doAction(gParams->ctl[iCtl].action);
									continue; //No further processing
								}
							}
						//}
					}
				} 
#endif

				//Do FME_KEYDOWN event
				retval = PerformControlAction(
						(int)msg.wParam,  //key code //control index
						FME_KEYDOWN,  //FM event code
						(int)msg.lParam,	//Other info //previous value
						msg.message,        //message type
						1,               //notification code
						FALSE, //no action
						FALSE, //return false if key was not processed by plugin
						0);

				if (retval) continue; //No further processing
			//}
		
		} else if (msg.message == WM_KEYUP || msg.message == WM_SYSKEYUP) {
		
			if (msg.wParam == VK_SHIFT || msg.wParam == VK_CONTROL){ 

				//Do FME_KEYDOWN event
				retval = PerformControlAction(
						(int)msg.wParam,  //key code //control index
						FME_KEYUP,  //FM event code
						(int)msg.lParam,	//Other info //previous value
						msg.message,        //message type
						1,               //notification code
						FALSE, //no action
						FALSE, //return false if key was not processed by plugin
						0);

				if (retval) continue; //No further processing

			}
		
		}

		if(!IsDialogMessage(hDlg, &msg)){
			TranslateMessage(&msg);