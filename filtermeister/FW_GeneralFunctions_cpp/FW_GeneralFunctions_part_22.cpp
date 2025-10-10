			case 2: val = gParams->ctl[n].width; break;
			case 3: val = gParams->ctl[n].height; break;
		}
	}

    return val;
}


int getCtlCoord (int n, int w)
{

    RECT rcCtl;
    POINT pt;
    int val;

        
    if (n == CTL_PREVIEW){

#if BIGDOCUMENT
		int proxW, inW, proxH, inH;
#else
        short proxW, inW, proxH, inH;
#endif
        
        if ( GetWindowRect( GetDlgItem(fmc.hDlg, 101), &rcCtl) )
        {   
            GetCursorPos(&pt);

            if (pt.x >= rcCtl.left && pt.x < rcCtl.right &&
                pt.y >= rcCtl.top  && pt.y < rcCtl.bottom) 
            {

#if BIGDOCUMENT
				fmc.inRect32 = GetInRect();	//needed??
                if (w==0){ //X-Coordinate
                    proxW = gProxyRect.right - gProxyRect.left;
                    inW = fmc.inRect32.right - fmc.inRect32.left;
    
                    if (inW < proxW) //Proxy full used
                        return  pt.x - rcCtl.left - (proxW - inW)/2 - 1; 
                    else //Proxy partially used
                        return  fmc.x_start + pt.x - rcCtl.left - 1;

                } else { //Y-Coordinate
                    proxH = gProxyRect.bottom - gProxyRect.top;
                    inH = fmc.inRect32.bottom - fmc.inRect32.top;
                
                    if (inH < proxH)
                        return  pt.y - rcCtl.top - (proxH - inH)/2 - 1; 
                    else
                        return  fmc.y_start + pt.y - rcCtl.top - 1;
                }

#else
                if (w==0){ //X-Coordinate
                    proxW = gProxyRect.right - gProxyRect.left;
                    inW = gStuff->inRect.right - gStuff->inRect.left;
    
                    if (inW*fmc.enlargeFactor < proxW-fmc.enlargeFactor){ //Proxy full used
                        val = pt.x - rcCtl.left - (proxW - inW*fmc.enlargeFactor)/2 - 1; 
                        #ifdef HIGHZOOM
                            val /= fmc.enlargeFactor;
						#endif
                    }else { //Proxy partialy used
                        #ifdef HIGHZOOM
                            val = fmc.x_start + (pt.x - rcCtl.left - 1)/fmc.enlargeFactor;
                        #else
                            val = fmc.x_start + pt.x - rcCtl.left - 1;
                        #endif
                    }

                } else { //Y-Coordinate
                    proxH = gProxyRect.bottom - gProxyRect.top;
                    inH = gStuff->inRect.bottom - gStuff->inRect.top;
                
                    if (inH*fmc.enlargeFactor < proxH-fmc.enlargeFactor){
                        val = pt.y - rcCtl.top - (proxH - inH*fmc.enlargeFactor)/2 - 1; 
                        #ifdef HIGHZOOM
                            val /= fmc.enlargeFactor;
                        #endif
                    } else {
                        #ifdef HIGHZOOM
                            val = fmc.y_start + (pt.y - rcCtl.top - 1)/fmc.enlargeFactor;
                        #else
                            val = fmc.y_start + pt.y - rcCtl.top - 1;
                        #endif
                    }
                }
#endif                
                return val;

            } else {
                
                return -1;
            }
        }
        return -1; //error from GetWindowRect afh 01-may2008
    
    } else { //Other Controls

        if ( GetWindowRect(gParams->ctl[n].hCtl, &rcCtl) ) //GetWindowRect( GetDlgItem(fmc.hDlg, 101), &rcCtl) )
        {   
            GetCursorPos(&pt);
            if (pt.x >= rcCtl.left && pt.x < rcCtl.right &&
                pt.y >= rcCtl.top  && pt.y < rcCtl.bottom) 
            {
                     if (w==0){ //X-Coordinate
                        return  pt.x - rcCtl.left;// - 1;

                     } else { 
                        return  pt.y - rcCtl.top;// -1;

                     }
            }
            else
            {
                return -1;
            }
            
        }
        return -1;	//error from GetWindowRect afh 01-may2008
    }
}



// function added by Harald Heim on Aug 9, 2002
int getPreviewCoordX (void)
{
    
    //Redirect to new function
    return getCtlCoord (CTL_PREVIEW, 0);
    
    /*RECT rcCtl;
    POINT pt;
    short   proxW, inW;


        if ( GetWindowRect( GetDlgItem(fmc.hDlg, 101), &rcCtl) )
        {   
            GetCursorPos(&pt);

            if (pt.x >= rcCtl.left && pt.x < rcCtl.right &&
                pt.y >= rcCtl.top  && pt.y < rcCtl.bottom) 
            {
                
                proxW = gProxyRect.right - gProxyRect.left;
                inW = gStuff->inRect.right - gStuff->inRect.left;
    
                if (inW < proxW) //Proxy full used
                    return  pt.x - rcCtl.left - (proxW - inW)/2 - 1; 
                else //Proxy partialy used
                    return  fmc.x_start + pt.x - rcCtl.left - 1;
                
            }
            else
            {
                return -1;
            }
        }*/
    
}

// function added by Harald Heim on Aug 9, 2002
int getPreviewCoordY (void)
{

    //Redirect to new function
    return getCtlCoord (CTL_PREVIEW, 1);
    
    /*RECT rcCtl;
    POINT pt;
    short   proxH, inH;

        if ( GetWindowRect(GetDlgItem(fmc.hDlg, 101), &rcCtl) )
        {   
            GetCursorPos(&pt);

            if (pt.x >= rcCtl.left && pt.x < rcCtl.right &&
                pt.y >= rcCtl.top  && pt.y < rcCtl.bottom) 
            {
                
                proxH = gProxyRect.bottom - gProxyRect.top;
                inH = gStuff->inRect.bottom - gStuff->inRect.top;
                
                if (inH < proxH)
                    return  pt.y - rcCtl.top - (proxH - inH)/2 - 1; 
                else
                    return  fmc.y_start + pt.y - rcCtl.top - 1;

            }
            else
            {
                return -1;
            }
        }*/

}




// function added by Harald Heim on Aug 12, 2002
int getAsyncKeyState (int t)
{
	//Swap mouse buttons for left hand setting
	if (t==VK_LBUTTON || t==VK_RBUTTON){
		if (GetSystemMetrics(SM_SWAPBUTTON)){
			if (t==VK_LBUTTON) t = VK_RBUTTON;
			else if (t==VK_RBUTTON) t = VK_LBUTTON;
		}
	}

   return GetAsyncKeyState (t);
}


// function added by Harald Heim on Aug 12, 2002
int getAsyncKeyStateF (int t)
{
    if (GetForegroundWindow() == MyHDlg)
        return getAsyncKeyState (t);
    else
        return 0;
}


int getAsyncKeyStateFC (int t)
{
    if (GetForegroundWindow() == MyHDlg){
		int n;
		HWND focWindow = GetFocus();

		//Check if edit box is focused
		for (n = 0; n < N_CTLS; n++) {
			if (gParams->ctl[n].ctlclass == CC_STANDARD || gParams->ctl[n].ctlclass == CC_SLIDER){
				if (focWindow == gParams->ctl[n].hBuddy1) return 0;
			}
		}

        if (getCtlClass(getCtlFocus()) == CC_COMBOBOX || getCtlClass(getCtlFocus()) == CC_LISTBOX){
            if (getAsyncKeyStateF(VK_MENU) <= -32767)
                return getAsyncKeyState (t);
        } else
             return getAsyncKeyState (t);
    } 

    return 0;

}


// function added by Harald Heim on Jun 6, 2002
int mouseOverWhenInvisible (int t)
{
    MouseOverWhenInvisible = t;
    
    return 1;
}


int arrayExists (int nr){

	if (nr<0 || nr>99) return false;

	if (Array[nr]!=NULL) return true;

	return false;
}


int getArrayDim (int nr, int dim)
{
	
	if (nr<0 || nr>99) return 0;

	if (Array[nr]!=NULL){
		if (dim==0)
			return ArrayX[nr];
		else if (dim==1)
			return ArrayY[nr];
		else if (dim==2)
			return ArrayZ[nr];
        else if (dim==3)
            return ArrayBytes[nr];

#if 0   //extended return values...
        else if (dim==-1)
            return (int)Array[nr];
        else if (dim==-2)
            return (int)ArrayID[nr];
        else if (dim==-3)
            return ArraySize[nr];
        else if (dim==-4)
            return ArrayBytes[nr];
        else if (dim==-5)
            return ArrayPadding[nr];
#endif
	}