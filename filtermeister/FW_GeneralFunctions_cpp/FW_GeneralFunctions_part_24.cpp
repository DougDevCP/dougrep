
    } else { //Calculate
        
        Cr=r;
        Cg=g;
        Cb=b;

        rVal = (double) r;
        gVal = (double) g;
        bVal = (double) b;

        xVal = 0.412453 * rVal + 0.357580 * gVal + 0.180423 * bVal;
        xVal /= (255.0 * 0.950456);
        yVal = 0.212671 * rVal + 0.715160 * gVal + 0.072169 * bVal;
        yVal /=  255.0;
        zVal = 0.019334 * rVal + 0.119193 * gVal + 0.950227 * bVal;
        zVal /= (255.0 * 1.088754);
        

        /*
        //Provided by Bill MacBeth
        xVal = 0.00170178 *rVal + 0.00147537 * gVal + 0.000744423 * bVal;
        yVal = 0.000834004 * rVal + 0.00280455 * gVal + 0.000283016 * bVal;
        zVal = 0.0000696389 * rVal + 0.000429320 * gVal + 0.00342261 * bVal;
        */

        if (yVal > 0.008856){
          fY = pow(yVal, 1.0 / 3.0);
          lVal = 116.0 * fY - 16.0;
        } else {
          fY = 7.787 * yVal + 16.0 / 116.0;
          lVal = 903.3 * yVal;
        }
        
        if (xVal > 0.008856)
            fX = pow(xVal, 1.0 / 3.0);
        else
            fX = 7.787 * xVal + 16.0 / 116.0;
                
        if (zVal > 0.008856)
            fZ = pow(zVal, 1.0 / 3.0);
        else
            fZ = 7.787 * zVal + 16.0 / 116.0;


        lVal = lVal * 2.56;
        aVal = 500.0 * (fX - fY)+128.0;
        bVal = 200.0 * (fY - fZ)+128.0;

        Cl=(int) lVal;
        Ca=(int) aVal;
        Cb2=(int) bVal;

        if (z==0)
            return Cl;
        else if (z==1)
            return Ca;
        else
            return Cb2;   
    }


}


//function added by Harald Heim on Dec 7, 2002
int lab2rgb(int l, int a, int b, int z)
{
    double rVal, gVal, bVal;
    double xVal, yVal, zVal;
    double lVal, aVal;
    double fX, fY, fZ;

    THREAD_LOCAL static int Cr,Cg,Cb;
    THREAD_LOCAL static int Cl,Ca,Cb2;


    if (l==Cl && a==Ca && b==Cb2){ //Read from Cache

        if (z==0) {         
            return Cr;
        } else if (z==1) {
            return Cg;
        } else {
            return Cb;
        }

    } else { //Calculate

        Cl=l;
        Ca=a;
        Cb2=b;

        lVal = (double) l;
        aVal = (double) a;
        bVal = (double) b;


        lVal= lVal / 2.56;
        aVal = aVal - 128;
        bVal = bVal - 128;


        fY = pow((lVal + 16.0) / 116.0, 3.0);
        if (fY < 0.008856)
            fY = lVal / 903.3;
        yVal = fY;

        if (fY > 0.008856)
        fY = pow(fY, 1.0 / 3.0);
        else
        fY = 7.787 * fY + 16.0 / 116.0;

        fX = aVal / 500.0 + fY;
        if (fX > 0.206893)
            xVal = pow(fX, 3.0);
        else
            xVal = (fX - 16.0 / 116.0) / 7.787;

        fZ = fY - bVal /200.0;
        if (fZ > 0.206893)
            zVal = pow(fZ, 3.0);
        else
            zVal = (fZ - 16.0 / 116.0) / 7.787;

        
        xVal *= (0.950456 * 255.0);
        yVal *=             255.0;
        zVal *= (1.088754 * 255.0);
        rVal = xVal *  3.240479 - yVal * 1.537150 - zVal * 0.498535;
        gVal = xVal * -0.969256 + yVal * 1.875992 + zVal * 0.041556;
        bVal = xVal *  0.055648 - yVal * 0.204043 + zVal * 1.057311;
        
        /*
        //Provided by Bill MacBeth
        rVal =   xVal * 7.853828E+02 - yVal * 6.342260E-03 - zVal * 2.056949E-03;
        gVal = - xVal * 2.471603E+02 + yVal * 7.356831E-03 + zVal * 1.629647E-04;
        bVal =   xVal * 1.544968E+01 - yVal * 7.349398E-04 + zVal * 3.808315E-03;
        */

        Cr=(int) (rVal + 0.5);
        Cg=(int) (gVal + 0.5);
        Cb=(int) (bVal + 0.5);

        if (z==0) {         
            return Cr;
        } else if (z==1) {
            return Cg;
        } else {
            return Cb;
        }

    }


}

//function added by Harald Heim on Dec 13, 2002
int setClickDrag(int b)
{
    //b=0 for left click drag
    //b=1 for right click drag
    //b=2 for no click drag at all
    ClickDrag=b;
    return 1;
}


/*
VOID CALLBACK MyTimerProc( 
    HWND hwnd,        // handle to window for timer messages 
    UINT message,     // WM_TIMER message 
    UINT idTimer,     // timer identifier 
    DWORD dwTime)     // current system time 
{ 

        fmc.n = idTimer-3;
        fmc.e = FME_TIMER;
        fmc.previous = 0;//previous;
        fmc.ctlMsg = WM_TIMER;//event;//WM_SIZE;
        fmc.ctlCode = 0;
        fmc.ctlClass = 0;//gParams->ctl[n].ctlclass;
        
        X86_call_fmf1(gParams->CB_onCtl, (int)&fmc);

}*/


//function added by Harald Heim on Dec 13, 2002
int setTimerEvent(int nr, int t, int state)
{
    //nr -> timer number 0 - 9
    //t -> trigger time
    //state==0 -> kill timer
    //state==1 -> set timer
    unsigned int Retval;

    if (nr<0 && nr>9) return 0;

    KillTimer(fmc.hDlg, 3 + nr); //Cancel any previous trigger.
    
    if (state==1){
        //Retval = SetTimer(fmc.hDlg, 3 + nr, t, (TIMERPROC) MyTimerProc); //Don't use message cue
        Retval = (int)SetTimer(fmc.hDlg, 3 + nr, t, NULL);
    }

    return (int) Retval;
}



int getDisplaySettings(int s)
{
   DEVMODE dm;
   dm.dmSize = sizeof(DEVMODE);
   dm.dmDriverExtra = 0;

   EnumDisplaySettings(NULL,ENUM_CURRENT_SETTINGS,&dm);
   
   switch (s) {   
        case 0:
            return (int) dm.dmBitsPerPel;
        case 1:
            return (int) dm.dmPelsWidth;
        case 2:
            return (int) dm.dmPelsHeight;
        case 3:
            return (int) dm.dmDisplayFrequency; 
   }
    return 0;

}

//function added by Harald Heim on Dec 14, 2002
int scrollPreview(int mode, int ox, int oy, int update)
{

    int RetVal;
#if BIGDOCUMENT
	VRect scaledFilterRect;
#else
    Rect scaledFilterRect;
#endif
    
    if (mode==1){
        ox = ox - (fmc.X-1)/2;
        oy = oy - (fmc.Y-1)/2;
    }

    RetVal = sub_scrollPreview(fmc.hDlg, mode, ox, oy, update);

    //Setup values
        
#if BIGDOCUMENT
	fmc.filterRect32 = GetFilterRect();	//needed?
	scaledFilterRect = fmc.filterRect32;
    scaleRect32(&scaledFilterRect, 1, fmc.scaleFactor);
    X0 = scaledFilterRect.left;
    Y0 = scaledFilterRect.top;
	fmc.inRect32 = GetInRect();	//needed??
    fmc.x_start = fmc.inRect32.left - X0;
    fmc.y_start = fmc.inRect32.top - Y0;
#else
    copyRect (&scaledFilterRect, &gStuff->filterRect);
    scaleRect (&scaledFilterRect, 1, fmc.scaleFactor);
    X0 = scaledFilterRect.left;
    Y0 = scaledFilterRect.top;
    fmc.x_start = gStuff->inRect.left - X0;
    fmc.y_start = gStuff->inRect.top - Y0;
#endif    

    fmc.X = fmc.xmax = scaledFilterRect.right - X0;
    fmc.Y = fmc.ymax = scaledFilterRect.bottom - Y0;
    fmc.x_end = fmc.x_start + fmc.columns;
    fmc.y_end = fmc.y_start + fmc.rows;

    return RetVal;

}


//function added by Harald Heim on Dec 23, 2002
int rgb2hsl(int r, int g, int b, int z)
{

    //int h,s,l;
    double rf,gf,bf,hf,sf,lf, cmax,cmin,diff;
    
    THREAD_LOCAL static int Cr,Cg,Cb;
    THREAD_LOCAL static int Ch,Cs,Cl;


    if (r==Cr && g==Cg && b==Cb){ //Read from Cache

        if (z==0)
            return Ch;
        else if (z==1)
            return Cs;
        else
            return Cl;    