                    r = ((int16 *)inBase)[iPos + 0];
                    outPtr[0] = (r >> 7) | (r >> 15);
                    if (gStuff->inHiPlane==1) {
					    a = ((int16 *)inBase)[iPos + 1];
					    outPtr[1] = (a >> 7) | (a >> 15);
				    }
                } else if (gray8){
                    outPtr[0] = inBase[iPos + 0];
                    if (gStuff->inHiPlane==1) {
					    outPtr[1] = inBase[iPos + 1];
				    }
                } else  if (cmyk8){
                    outPtr[0] = inBase[iPos + 0];
                    outPtr[1] = inBase[iPos + 1];
                    outPtr[2] = inBase[iPos + 2];
                    outPtr[3] = inBase[iPos + 3];
                    if (gStuff->inHiPlane==4) {
					    outPtr[4] = inBase[iPos + 4];
				    }
                } else if (cmyk16){
                    r = ((int16 *)inBase)[iPos + 0];
                    g = ((int16 *)inBase)[iPos + 1];
                    b = ((int16 *)inBase)[iPos + 2];
                    k = ((int16 *)inBase)[iPos + 3];
                    outPtr[0] = (r >> 7) | (r >> 15);
                    outPtr[1] = (g >> 7) | (g >> 15);
                    outPtr[2] = (b >> 7) | (b >> 15);
                    outPtr[3] = (k >> 7) | (k >> 15);
                    if (gStuff->inHiPlane==4) {
					    a = ((int16 *)inBase)[iPos + 4];
					    outPtr[4] = (a >> 7) | (a >> 15);
				    }
                }
                outPtr += (gStuff->inHiPlane+1); //3
            }//while j
            

        }//while iRows

        // Point pixel map to the RGB buffer...
        pixels.baseAddr = pRGBbuffer;
        pixels.rowBytes = iCols*(gStuff->inHiPlane+1); //3
        pixels.colBytes = (gStuff->inHiPlane+1); //3
        pixels.planeBytes = 1;

        if (pixels.imageMode == plugInModeRGB48)
            pixels.imageMode = plugInModeRGBColor;      //data is RGB
        else if (pixels.imageMode == plugInModeLab48)
            pixels.imageMode = plugInModeLabColor; //data is Lab
        else if (pixels.imageMode == plugInModeDeepMultichannel)
            pixels.imageMode = plugInModeMultichannel; //data is Multichannel
        else if (pixels.imageMode == plugInModeGray16)
            pixels.imageMode = plugInModeGrayScale;
        else if (pixels.imageMode == plugInModeCMYK64)
            pixels.imageMode = plugInModeCMYKColor;
    }
#endif




#if 1


    //Added by Harald Heim, December 6, 2002
    if (gFmcp->enlargeFactor==1 && (pixels.imageMode == plugInModeRGB48 || pixels.imageMode == plugInModeLab48 || pixels.imageMode == plugInModeDeepMultichannel)) {

        // Convert outData from 48-bit to 24-bit for display...
        unsigned8 *inPtr1 = (unsigned8 *)pixels.baseAddr;
        unsigned8 *outPtr;
        int iRows = pixels.bounds.bottom - pixels.bounds.top;
        int iCols = pixels.bounds.right - pixels.bounds.left;

        //We need to reallocate the pRGBbuffer if the preview has been resized in the meantime.
        //This was the problem that caused the crash. The buffer wasn't reallocated, so sometimes it was too small.
        //if (!pRGBbuffer) {
        if (pRGBbufferSize != proxyWidth*proxyHeight*(gStuff->inHiPlane+1)){ //3 //Only reallocate if size has to be changed
            
            if (pRGBbuffer) {
                if (pRGBbufferID != 0){
			        UnlockBuffer(pRGBbufferID);
			        FreeBuffer(pRGBbufferID); 
		        } else
			        free(pRGBbuffer);
		        pRGBbufferID = 0;
                pRGBbuffer = NULL;
				pRGBbufferSize = 0;
            }
            
            // Allocate an RGB buffer large enough to cover the entire proxy window.
            if (AllocateBuffer( proxyWidth*proxyHeight * (gStuff->inHiPlane+1), &pRGBbufferID) == noErr){
				pRGBbuffer = (unsigned8 *)LockBuffer(pRGBbufferID, true);
			} else { //Fall back to Windows allocation
				pRGBbuffer = (unsigned8 *)calloc(proxyWidth*proxyHeight, (gStuff->inHiPlane+1)); //3
				pRGBbufferID=0;
			}
            
            if (!pRGBbuffer) {
#if 1
                ErrorOk("Can't allocate proxy preview buffer for 48-bit image!");
#endif
                pRGBbufferSize = 0;
                goto egress;
            
            } else {

                pRGBbufferSize = proxyWidth*proxyHeight*(gStuff->inHiPlane+1); //3
            }
        }

        outPtr = pRGBbuffer;
        while (iRows--) {
            int16 *inPtr = (int16 *)inPtr1;
            int j = iCols;
            while (j--) {
                // Unroll 3 planes...
                int r = inPtr[0];
                int g = inPtr[1];
                int b = inPtr[2];
                outPtr[0] = (r >> 7) | (r >> 15);
                outPtr[1] = (g >> 7) | (g >> 15);
                outPtr[2] = (b >> 7) | (b >> 15);
                if (gStuff->inHiPlane==3) {
                    int a = inPtr[3];
                    outPtr[3] = (a >> 7) | (a >> 15);
                }
                inPtr += pixels.colBytes;
                outPtr += (gStuff->inHiPlane+1); //3
            }//while j
            inPtr1 += pixels.rowBytes;
        }//while iRows
        // Point pixel map to the RGB buffer...
        pixels.baseAddr = pRGBbuffer;
        pixels.rowBytes = iCols*(gStuff->inHiPlane+1); //3
        pixels.colBytes = (gStuff->inHiPlane+1); //3
        pixels.planeBytes = 1;

        if (pixels.imageMode == plugInModeRGB48)
            pixels.imageMode = plugInModeRGBColor;      //data is RGB
        else if (pixels.imageMode == plugInModeLab48)
            pixels.imageMode = plugInModeLabColor; //data is Lab
        else if (pixels.imageMode == plugInModeDeepMultichannel)
            pixels.imageMode = plugInModeMultichannel; //data is Multichannel
    }

    else if (gFmcp->enlargeFactor==1 && pixels.imageMode == plugInModeGray16) {
        // Convert outData from Gray16 to Grayscale for display...
        unsigned8 *inPtr1 = (unsigned8 *)pixels.baseAddr;
        unsigned8 *outPtr;
        int iRows = pixels.bounds.bottom - pixels.bounds.top;
        int iCols = pixels.bounds.right - pixels.bounds.left;
        
        //if (!pRGBbuffer) {
        if (pRGBbufferSize != proxyWidth*proxyHeight*(gStuff->inHiPlane+1)){ //Only reallocate if size has to be changed

            if (pRGBbuffer) {
                if (pRGBbufferID != 0){
			        UnlockBuffer(pRGBbufferID);
			        FreeBuffer(pRGBbufferID); 
		        } else
			        free(pRGBbuffer);
		        pRGBbufferID = 0;
                pRGBbuffer = NULL;
				pRGBbufferSize = 0;
            }

            // Allocate a grayscale buffer large enough to cover the entire proxy window.
            if (AllocateBuffer( proxyWidth*proxyHeight * (gStuff->inHiPlane+1), &pRGBbufferID) == noErr){
				pRGBbuffer = (unsigned8 *)LockBuffer(pRGBbufferID, true);
			} else { //Fall back to Windows allocation
				pRGBbuffer = (unsigned8 *)calloc(proxyWidth*proxyHeight, (gStuff->inHiPlane+1)); //3
				pRGBbufferID=0;
			}


            if (!pRGBbuffer) {
#if 1
                ErrorOk("Can't allocate proxy preview buffer for Gray16 image!");
#endif
                goto egress;
            
            } else {

                pRGBbufferSize = proxyWidth*proxyHeight;
            }
        }
        outPtr = pRGBbuffer;
        while (iRows--) {
            int16 *inPtr = (int16 *)inPtr1;
            int j = iCols;
            while (j--) {
                //int gray = *inPtr;
                int gray = inPtr[0];
                outPtr[0] = (gray >> 7) | (gray >> 15);
                if (gStuff->inHiPlane==1){
                    int a = inPtr[1];
                    outPtr[1] = (a >> 7) | (a >> 15);
                }
                //*outPtr++ = (gray >> 7) | (gray >> 15);
                inPtr += pixels.colBytes;
                outPtr += (gStuff->inHiPlane+1); //2;
            }//while j
            inPtr1 += pixels.rowBytes;
        }//while iRows
        // Point pixel map to the grayscale buffer...
        pixels.baseAddr = pRGBbuffer;
        pixels.rowBytes = iCols*(gStuff->inHiPlane+1);
        pixels.colBytes = (gStuff->inHiPlane+1); //1;
        pixels.planeBytes = 1;
        pixels.imageMode = plugInModeGrayScale;      //data is GrayScale
    }

    //Added by Harald Heim, December 6, 2002
    else if (gFmcp->enlargeFactor==1 && pixels.imageMode == plugInModeCMYK64) {

        // Convert outData from CMYK64 to CMYK for display...
        unsigned8 *inPtr1 = (unsigned8 *)pixels.baseAddr;
        unsigned8 *outPtr;
        int iRows = pixels.bounds.bottom - pixels.bounds.top;
        int iCols = pixels.bounds.right - pixels.bounds.left;
        
        //if (!pRGBbuffer) {
        if (pRGBbufferSize != proxyWidth*proxyHeight*(gStuff->inHiPlane+1)){ //4 //Only reallocate if size has to be changed

            if (pRGBbuffer) {
                if (pRGBbufferID != 0){
			        UnlockBuffer(pRGBbufferID);
			        FreeBuffer(pRGBbufferID); 
		        } else
			        free(pRGBbuffer);
		        pRGBbufferID = 0;
                pRGBbuffer = NULL;
				pRGBbufferSize = 0;
            }

            // Allocate a CMYK buffer large enough to cover the entire proxy window.
            if (AllocateBuffer( proxyWidth*proxyHeight * (gStuff->inHiPlane+1), &pRGBbufferID) == noErr){
				pRGBbuffer = (unsigned8 *)LockBuffer(pRGBbufferID, true);
			} else { //Fall back to Windows allocation
				pRGBbuffer = (unsigned8 *)calloc(proxyWidth*proxyHeight, (gStuff->inHiPlane+1)); //3
				pRGBbufferID=0;
			}

            if (!pRGBbuffer) {
#if 1
                ErrorOk("Can't allocate proxy preview buffer for CMYK64 image!");
#endif
                goto egress;

            } else {

                pRGBbufferSize = proxyWidth*proxyHeight*(gStuff->inHiPlane+1);//4
            }
        }
        outPtr = pRGBbuffer;
        while (iRows--) {
            int16 *inPtr = (int16 *)inPtr1;
            int j = iCols;
            while (j--) {
                // Unroll 4 planes...
                int c = inPtr[0];
                int m = inPtr[1];
                int y = inPtr[2];
                int k = inPtr[3];
                outPtr[0] = (c >> 7) | (c >> 15);
                outPtr[1] = (m >> 7) | (m >> 15);
                outPtr[2] = (y >> 7) | (y >> 15);
                outPtr[3] = (k >> 7) | (k >> 15);
                if (gStuff->inHiPlane==4) {
                    int a = inPtr[4];
                    outPtr[4] = (a >> 7) | (a >> 15);
                }
                inPtr += pixels.colBytes;
                outPtr += (gStuff->inHiPlane+1);//4;
            }//while j
            inPtr1 += pixels.rowBytes;
        }//while iRows
        // Point pixel map to the CMYK buffer...
        pixels.baseAddr = pRGBbuffer;
        pixels.rowBytes = iCols*(gStuff->inHiPlane+1);//4;
        pixels.colBytes = (gStuff->inHiPlane+1);//4;
        pixels.planeBytes = 1;
        pixels.imageMode = plugInModeCMYKColor;      //data is CMYK
    }

    
    //Added by Harald Heim, December 6, 2002
    else if (pixels.imageMode == plugInModeDuotone16) {

        // Convert outData from Duotone16 to Duotone for display...
        unsigned8 *inPtr1 = (unsigned8 *)pixels.baseAddr;
        unsigned8 *outPtr;
        int iRows = pixels.bounds.bottom - pixels.bounds.top;
        int iCols = pixels.bounds.right - pixels.bounds.left;
        
        //if (!pRGBbuffer) {
        if (pRGBbufferSize != proxyWidth*proxyHeight*(gStuff->inHiPlane+1)){ //2 //Only reallocate if size has to be changed

            if (pRGBbuffer) {
                if (pRGBbufferID != 0){