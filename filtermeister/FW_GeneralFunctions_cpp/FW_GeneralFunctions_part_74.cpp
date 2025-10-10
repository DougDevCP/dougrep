		NewZoom=1;
	}else if (fmc.pre_ctl[n]==10 && previous>fmc.pre_ctl[n]){//previous==fmc.pre_ctl[n]+1
		setCtlVal (n,9);
		NewZoom=1;
        NewLarge=2;
    }else if (fmc.pre_ctl[n]==25 && previous<fmc.pre_ctl[n]){ //previous==fmc.pre_ctl[n]-1
		setCtlVal (n,26);
		NewZoom=-888;
	}else if (fmc.pre_ctl[n]==25 && previous>fmc.pre_ctl[n]){//previous==fmc.pre_ctl[n]+1
		setCtlVal (n,24);
		NewZoom=16;
	} else {
		NewZoom= fmc.pre_ctl[n]-10;//+1
		if (NewZoom>=13) NewZoom=NewZoom+1;
		if (NewZoom==15) NewZoom=NewZoom+1;
		if (NewZoom<1) NewZoom=1;
		if (NewZoom>16) NewZoom=16;
	}

#else

	/*if (fmc.pre_ctl[n]==16){ //Softproof
        NewZoom = SoftProof();
        if (fmc.enlargeFactor==1 && fmc.scaleFactor==NewZoom && previous!=16) forceUpdate=true;
    }else*/ if (fmc.pre_ctl[n]==15){ //Fit
		NewZoom=-888;
	}else if (fmc.pre_ctl[n]==14 && previous<fmc.pre_ctl[n]){ //previous==fmc.pre_ctl[n]-1
		setCtlVal (n,15);
		NewZoom=-888;
	}else if (fmc.pre_ctl[n]==14 && previous>fmc.pre_ctl[n]){//previous==fmc.pre_ctl[n]+1
		setCtlVal (n,13);
		NewZoom=16;
	} else {
		NewZoom= fmc.pre_ctl[n]+1;
		if (NewZoom>=13) NewZoom=NewZoom+1;
		if (NewZoom==15) NewZoom=NewZoom+1;
		if (NewZoom<1) NewZoom=1;
		if (NewZoom>16) NewZoom=16;
	}
#endif

    #ifdef HIGHZOOM
		if (NewZoom==1 && NewLarge==32)  enableCtl(ctlPlus,1);  else enableCtl(ctlPlus,-1);
    #else
        if (NewZoom==1 )  enableCtl(ctlPlus,1);  else enableCtl(ctlPlus,-1);
    #endif

    if (NewZoom==16 )  enableCtl(ctlMinus,1);  else enableCtl(ctlMinus,-1);
	
	
    //if (NewZoom != scaleFactor) 
    #ifdef HIGHZOOM 
        retval = setZoom(NewZoom,NewLarge);
        updatePreview(0);
    #else
        retval = setZoom(NewZoom,1);
    #endif
	
	setCtlFocus(n); //Focus Back to the zoom combo box

    //Force Preview Update
    #ifdef HIGHZOOM
        if (retval==false && (forceUpdate || (fmc.pre_ctl[n]!=27 && previous==27)) ){
            doAction(CA_PREVIEW);
            retval=true;
        }
    #else
        if (retval==false && (forceUpdate || (fmc.pre_ctl[n]!=16 && previous==16)) ) {
            doAction(CA_PREVIEW);
            retval=true;
        }
    #endif


	triggerEvent(CTL_PREVIEW,FME_ZOOMCHANGED,0);
	
	return retval; //Return Preview Update
	
}


int evalZoomButtons(int n, int ctlCombo, int ctlPlus, int ctlMinus){

    int NewZoom=0;
    int NewLarge=1;
	
	if (n==ctlMinus){ //Minus
		
#ifndef CREATEDIALOGPARAM
		if (getAsyncKeyStateF(VK_CONTROL)<0){
			if (fmc.enlargeFactor==1)
                NewZoom=16;
            else
                NewZoom=1;
		} else if (getAsyncKeyStateF(VK_SHIFT)<0 ){
			if (fmc.enlargeFactor==1)
                NewZoom=-888;
            else {
                NewZoom=1;
                NewLarge=1;
            }
		} else 
#endif		
		{
            if (fmc.enlargeFactor==1){
                NewZoom=fmc.scaleFactor+1;
				if (NewZoom==13 || NewZoom==15) NewZoom=NewZoom+1;
				if (NewZoom<1) NewZoom=1;
				if (NewZoom>16) NewZoom=16;
            } else {
                NewZoom=1;
                if (fmc.enlargeFactor==32) NewLarge=fmc.enlargeFactor=16;
                else if (fmc.enlargeFactor==16) NewLarge=fmc.enlargeFactor=12;
                else if (fmc.enlargeFactor==12) NewLarge=fmc.enlargeFactor=8;
                else 
                    NewLarge=fmc.enlargeFactor-1;
            }
		}
		
		if (NewZoom==16) enableCtl(ctlMinus,1);
		enableCtl(ctlPlus,-1);
		
		if (NewZoom != -888){
			#ifdef HIGHZOOM
                if (NewZoom==14)
					setCtlVal(ctlCombo, 23);
				else if (NewZoom==16)
					setCtlVal(ctlCombo, 24);
                else if (NewLarge==1 && NewZoom>=1)
                    setCtlVal(ctlCombo, 10+NewZoom);
                else if (NewLarge>=1){
                    if (NewLarge==32) setCtlVal(ctlCombo, 0);
                    else if (NewLarge==16) setCtlVal(ctlCombo, 1);
                    else if (NewLarge==12) setCtlVal(ctlCombo, 2);
                    else 
                        setCtlVal(ctlCombo, 11-NewLarge);
                }
            #else
                if (NewZoom==14)
					setCtlVal(ctlCombo, NewZoom-2);
				else if (NewZoom==16)
					setCtlVal(ctlCombo, NewZoom-3);
				else
					setCtlVal(ctlCombo, NewZoom-1);
            #endif
		} else {
			#ifdef HIGHZOOM
                setCtlVal(ctlCombo, 26);
            #else
                setCtlVal(ctlCombo, 15);
            #endif
		}
				
		#ifdef HIGHZOOM 
            setZoom(NewZoom,NewLarge);
            updatePreview(0);
        #else
			setZoom(NewZoom,1);
        #endif
			
	} else if  (n==ctlPlus ) { //Plus

#ifndef CREATEDIALOGPARAM
		if (getAsyncKeyStateF(VK_CONTROL)<0 || getAsyncKeyStateF(VK_SHIFT)<0 ){
			if (fmc.enlargeFactor==1 && fmc.scaleFactor>1)
                NewZoom=1;
            else {
                NewZoom=1;
                NewLarge=32;
            }
		} else 
#endif		
		{
            if (fmc.enlargeFactor==1){
                NewZoom=fmc.scaleFactor-1;
				if (NewZoom==13 || NewZoom==15) NewZoom=NewZoom-1;
				
                if (NewZoom<1) {
                    NewZoom=1;
                    #ifdef HIGHZOOM
                        NewLarge=2;
                    #endif
                }
				if (NewZoom>16) NewZoom=16;
            } else {
                NewZoom=1;
				if (fmc.enlargeFactor>=16) NewLarge=fmc.enlargeFactor=32;
                else if (fmc.enlargeFactor==12) NewLarge=fmc.enlargeFactor=16;
                else if (fmc.enlargeFactor==8) NewLarge=fmc.enlargeFactor=12;
                else 
                    NewLarge=fmc.enlargeFactor+1;
            }
		}
		
		
        #ifdef HIGHZOOM
			if (NewZoom==1 && NewLarge==32)  enableCtl(ctlPlus,1);  else enableCtl(ctlPlus,-1);
        #else
            if (NewZoom==1 )  enableCtl(ctlPlus,1);  else enableCtl(ctlPlus,-1);
        #endif
		enableCtl(ctlMinus,-1); 
		
        #ifdef HIGHZOOM
            if (NewZoom==14)
				setCtlVal(ctlCombo, 23);
			else if (NewZoom==16)
				setCtlVal(ctlCombo, 24);
            else if (NewLarge==1 && NewZoom>=1)
                setCtlVal(ctlCombo, 10+NewZoom);
            else if (NewLarge>=1){
                if (NewLarge==32) setCtlVal(ctlCombo, 0);
                else if (NewLarge==16) setCtlVal(ctlCombo, 1);
                else if (NewLarge==12) setCtlVal(ctlCombo, 2);
                else 
                    setCtlVal(ctlCombo, 11-NewLarge);
            }
        #else
			if (NewZoom==14)
				setCtlVal(ctlCombo, NewZoom-2);
			else if (NewZoom==16)
				setCtlVal(ctlCombo, NewZoom-3);
			else
				setCtlVal(ctlCombo, NewZoom-1);
        #endif

        #ifdef HIGHZOOM 
            setZoom(NewZoom,NewLarge);
            updatePreview(0);
        #else
			setZoom(NewZoom,1);
        #endif
	}
	
    triggerEvent(CTL_PREVIEW,FME_ZOOMCHANGED,0);

	return true; //Preview Update already done by setZoom

}

// Functions added by Ognen Genchev
double fclamp (double x, double minVal, double maxVal)
{
    double clamp = x <= minVal ? minVal : x > maxVal ? maxVal : x;
    return clamp;
}

double fsmoothstep (double edge0, double edge1, double x)
{
    // Scale, bias and saturate x to 0..1 range
    x = fclamp((x - edge0) / (edge1 - edge0), 0.0, 1.0); 
    // Evaluate polynomial
    return x * x * (3.0 - 2.0 * x);
}
// end