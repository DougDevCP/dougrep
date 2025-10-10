			        UnlockBuffer(pRGBbufferID);
			        FreeBuffer(pRGBbufferID); 
		        } else
			        free(pRGBbuffer);
		        pRGBbufferID = 0;
                pRGBbuffer = NULL;
				pRGBbufferSize = 0;
            }

            // Allocate a Duotone buffer large enough to cover the entire proxy window.
            if (AllocateBuffer( proxyWidth*proxyHeight * (gStuff->inHiPlane+1), &pRGBbufferID) == noErr){
				pRGBbuffer = (unsigned8 *)LockBuffer(pRGBbufferID, true);
			} else { //Fall back to Windows allocation
				pRGBbuffer = (unsigned8 *)calloc(proxyWidth*proxyHeight, (gStuff->inHiPlane+1)); //3
				pRGBbufferID=0;
			}

            if (!pRGBbuffer) {
#if 1
                ErrorOk("Can't allocate proxy preview buffer for Duotone16 image!");
#endif
                goto egress;

            } else {

                pRGBbufferSize = proxyWidth*proxyHeight*(gStuff->inHiPlane+1);//2;
            }
        }
        outPtr = pRGBbuffer;
        while (iRows--) {
            int16 *inPtr = (int16 *)inPtr1;
            int j = iCols;
            while (j--) {
                // Unroll 2 planes...
                int d = inPtr[0];
                int u = inPtr[1];
                outPtr[0] = (d >> 7) | (d >> 15);
                outPtr[1] = (u >> 7) | (u >> 15);
                if (gStuff->inHiPlane==2) {
                    int a = inPtr[3];
                    outPtr[2] = (a >> 7) | (a >> 15);
                }
                inPtr += pixels.colBytes;
                outPtr += (gStuff->inHiPlane+1); //2;
            }//while j
            inPtr1 += pixels.rowBytes;
        }//while iRows
        // Point pixel map to the Duotone buffer...
        pixels.baseAddr = pRGBbuffer;
        pixels.rowBytes = iCols*(gStuff->inHiPlane+1);//2;
        pixels.colBytes = (gStuff->inHiPlane+1);//2;
        pixels.planeBytes = 1;
        pixels.imageMode = plugInModeDuotone;      //data is Duotone
    }
    
#endif


    pixels.mat = NULL;
    pixels.masks = NULL;
    pixels.maskPhaseRow = 0;
    pixels.maskPhaseCol = 0;


	if (gStuff->isFloating || //gStuff->depth == 16 || //Temp preview fix on 16bit layer -> but no transparency chess pattern
        (gStuff->haveMask && gStuff->filterCase == filterCaseProtectedTransparencyWithSelection)) {
       
		mask.next = NULL;

		 if (gFmcp->enlargeFactor != 1){
            //This avoid crash but does not display preview correctly. 
			//We probably need to copy the MyAdjustedMaskData into pRGBbuffer on channel 4!
			mask.maskData = pRGBbuffer+gStuff->inHiPlane;
            mask.rowBytes = pixels.rowBytes;
            mask.colBytes = pixels.colBytes;
		 } else {
			mask.maskData = MyAdjustedMaskData;
			mask.rowBytes = gStuff->maskRowBytes;
			mask.colBytes = 1;
		 }
        mask.maskDescription = kSimplePSMask;
    
        pixels.masks = &mask;

    } else if ((gStuff->inLayerPlanes != 0) && (gStuff->inTransparencyMask != 0) &&
               (gStuff->filterCase != filterCaseProtectedTransparencyNoSelection) &&
               (gStuff->filterCase != filterCaseProtectedTransparencyWithSelection)) {
        // Was wrong for PS Protected Transparency cases, in which
        // case inLayerPlanes = 3 and inTransparencyMask=1, but inColumnBytes = 3,
        // not 4, and the transparency mask is NOT interleaved with the RGB data.
        // In fact, I don't know where the transparency mask data is for this case!
        
        //int multiply=1;
        //if (gStuff->depth == 16) multiply=2

	    mask.next = NULL;
        
        if (gStuff->depth == 16 || gFmcp->enlargeFactor != 1){
            
            mask.maskData = pRGBbuffer+gStuff->inHiPlane;

            mask.rowBytes = pixels.rowBytes;
            mask.colBytes = pixels.colBytes;

        } else {
            mask.maskData = fDragging ?
                        ((char *) MyAdjustedInData) + (gStuff->inHiPlane - gStuff->inLoPlane) :
                        ((char *) gStuff->outData) + (gStuff->outHiPlane - gStuff->outLoPlane);

            mask.rowBytes = fDragging ? gStuff->inRowBytes : gStuff->outRowBytes;
            mask.colBytes = fDragging ? gStuff->inHiPlane - gStuff->inLoPlane + 1
                                  : gStuff->outHiPlane - gStuff->outLoPlane + 1;
        }
        mask.maskDescription = kSimplePSMask;
    
        pixels.masks = &mask;
    }

	//if ( !fDragging) Info ("%d,%d - %d,%d     %d, %d", pixels.bounds.left, pixels.bounds.top, pixels.bounds.right, pixels.bounds.bottom,   itemBounds.left, itemBounds.top);

	//Info ("UIScaling: %d",UIScaling);
	//fmc.hostSig=='PffA' ||
	//&& fmc.hostSig!='PffA'

	if (UIScaling && (pixels.imageMode==plugInModeRGBColor || pixels.imageMode==plugInModeGrayScale) ) { //Compensate UIScaling bug

        BITMAPINFO info;
		int w = pixels.bounds.right - pixels.bounds.left;
		int h = pixels.bounds.bottom - pixels.bounds.top;
		int size;
		BYTE * buffer;
		BufferID bufferID = 0;
		BYTE * buffer2 = (BYTE *)pixels.baseAddr;
		int count=0,count2,x,y;

        //startClock();


		//Info ("Here");

		//Only width and heights with a multiple of 4 are displayed correctly 
		if (w%4!=0) w = (w/4)*4;
		if (h%4!=0) h = (h/4)*4;
		size = w*h*3;

		wRect.right -= (pixels.bounds.right - pixels.bounds.left - w);
		wRect.bottom -= (pixels.bounds.bottom - pixels.bounds.top - h);
		FrameRect(hDC, &wRect, (HBRUSH)GetStockObject(BLACK_BRUSH));


		/*SPErr error;
		PSColorSpaceSuite2 *sPSColorSpace;
		error = sSPBasic->AcquireSuite(kPSColorSpaceSuite, kPSColorSpaceSuiteVersion2, (const void**)&sPSColorSpace);
		if (!error){
			buffer2 = malloc(size);
			sPSColorSpace->ConvertToMonitorRGB(plugIncolorServicesRGBSpace,pixels.baseAddr,buffer2,size);
			sSPBasic->ReleaseSuite(kPSColorSpaceSuite,kPSColorSpaceSuiteVersion2);
		}*/
		
		if (AllocateBuffer(size, &bufferID) == noErr){
			buffer = (BYTE *)LockBuffer(bufferID, true);
		} else { //Fall back to Windows allocation
			buffer = (BYTE *)malloc(size);
		}
		
		//Swap color channels for StretchDIBits //Mirror vertically and 
		if (pixels.imageMode==plugInModeGrayScale){

			//for (y=h-1;y>=0;y--){
			for (y=0;y<h;y++){
				count2 = y*pixels.rowBytes; //w*pixels.colBytes;
			for (x=0;x<w;x++){
				if (gStuff->inHiPlane!=1 || buffer2[count2-1]==255){
					buffer[count] = buffer[count+1] = buffer[count+2] = 
						buffer2[count2];
				} else { //Add checker board
					int m = buffer2[count2-1];
					int xx = x%16;
					int yy = y%16;
					int c = (xx<8 && yy<8) || (xx>=8 && yy>=8) ? 255 : 192;
					if (m==0){
						buffer[count] = buffer[count+1] = buffer[count+2] = c;
					} else {
						buffer[count] = buffer[count+1] = buffer[count+2] = 
							((255-m)*c + m* buffer2[count2])/255;
					}
				}
				count+=3;
				count2+=pixels.colBytes;
			}}

		} else {

			//for (y=h-1;y>=0;y--){
			for (y=0;y<h;y++){
				count2 = y*pixels.rowBytes; //w*pixels.colBytes;
			for (x=0;x<w;x++){
				if (gStuff->inHiPlane!=3 || buffer2[count2-1]==255){
					buffer[count] = buffer2[count2+2];
					buffer[count+1] = buffer2[count2+1];
					buffer[count+2] = buffer2[count2];
				} else { //Add checker board
					int m = buffer2[count2-1];
					int xx = x%16;
					int yy = y%16;
					int c = (xx<8 && yy<8) || (xx>=8 && yy>=8) ? 255 : 192;
					if (m==0){
						buffer[count] = buffer[count+1] = buffer[count+2] = c;
					} else {
						buffer[count] = ((255-m)*c + m* buffer2[count2+2])/255;
						buffer[count+1] = ((255-m)*c + m* buffer2[count2+1])/255;
						buffer[count+2] = ((255-m)*c + m* buffer2[count2])/255;
					}
				}
				count+=3;
				count2+=pixels.colBytes;
			}}
		}

		ZeroMemory(&info, sizeof(BITMAPINFO));
		info.bmiHeader.biBitCount = 24;
		info.bmiHeader.biWidth = w;
		info.bmiHeader.biHeight = -h; //top down image
		info.bmiHeader.biPlanes = 1;
		info.bmiHeader.biSize = sizeof(BITMAPINFOHEADER);
		info.bmiHeader.biSizeImage = 0; //size;
		info.bmiHeader.biCompression = BI_RGB;
                
		StretchDIBits(hDC, itemBounds.left, itemBounds.top, w, h, 0, 0, w, h, buffer, &info, DIB_RGB_COLORS, SRCCOPY);

        //DebugLogVal(stopClock());

		
		if (buffer){
			if (bufferID != 0){
				UnlockBuffer(bufferID);
				FreeBuffer(bufferID); 
			} else
				free(buffer);
			bufferID = 0;
			buffer = NULL;
		}

		//if (buffer2!=pixels.baseAddr)
		//	free(buffer2);

	} else {

		FrameRect(hDC, &wRect, (HBRUSH)GetStockObject(BLACK_BRUSH));

        //startClock();

		(gStuff->displayPixels)(&pixels, &pixels.bounds, itemBounds.top, itemBounds.left, (void *)hDC);

        //DebugLogVal(stopClock());

	}


#if 1
    }//if intersects
#endif
egress:
    EndPaint(hDlg, (LPPAINTSTRUCT) &ps);

    fDrawing = false;
	
	//DRAWITEM event for preview
	if (gParams->ctl[CTL_PREVIEW].properties & CTP_DRAWITEM){            
        PerformControlAction(
                CTL_PREVIEW,             //control index
                FME_DRAWITEM,           //FM event code
                0,                      //previous value
                WM_DRAWITEM,            //message type
                0,                      //notification code
                FALSE,                  //fActionable
                TRUE,0);                  //fProcessed
    }




} /*ShowOutputBuffer*/

#endif //APP




void UpdateProxy (HWND hDlg, BOOL bEraseBackground)
{
    HWND hProxy;
    RECT imageRect;

    static long previousScaleFactor = -1; //force initial background erase. //Thread Safe????
	static long prevEnlargeFactor = -1;
#ifdef APP
    static int image = 0;
#endif
