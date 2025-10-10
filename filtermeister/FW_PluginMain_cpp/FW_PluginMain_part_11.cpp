                    goto egress;
                }
#if 0
                else {
                    Info("Allocated tile buffer 3 (%d bytes).",
                            fmc.X * fmc.Y * fmc.Z * sizeof (fmc.t3buf[0]));
                }
#endif
            }//if need t3buf
			if (fmc.need_t4buf) {
                //allocate a (temporary) tile buffer 4...
                //Should use AllocateBuffer() ????
                do {
                    int bufferSize = fmc.imageMode>9 ? 2:1;
					res = IDOK;

                    //some kind of dword padding (+ 3) added by Harald Heim, February 2003
					
					if (gStuff->bufferProcs && gStuff->bufferProcs->allocateProc && gStuff->bufferProcs->lockProc && gStuff->bufferProcs->unlockProc && gStuff->bufferProcs->freeProc){ //Photoshop allocation
						if (AllocateBuffer( (fmc.X * fmc.Y * fmc.Z + 3) * bufferSize, &fmc.t4bufID) == noErr){
							fmc.t4buf = (unsigned char *)LockBuffer(fmc.t4bufID, true);
						} else { //Fall back to Windows allocation
							fmc.t4buf = (unsigned char *)calloc(fmc.X * fmc.Y * fmc.Z + 3, bufferSize);
							fmc.t4bufID=0;
						}
					}else { //Windows allocation
						fmc.t4buf = (unsigned char *)calloc(fmc.X * fmc.Y * fmc.Z + 3, bufferSize); //sizeof (fmc.tbuf[0])
						fmc.t4bufID=0;
					}

                    if (!fmc.t4buf) {
                        res = Error("Could not allocate tile buffer 4 (%d bytes).",
                                fmc.X * fmc.Y * fmc.Z * bufferSize); //sizeof (fmc.t4buf[0])
                    }
                } while (res == IDRETRY);
                if (res == IDABORT) {
                    gResult = memFullErr;
                    goto egress;
                }
#if 0
                else {
                    Info("Allocated tile buffer 4 (%d bytes).",
                            fmc.X * fmc.Y * fmc.Z * sizeof (fmc.t4buf[0]));
                }
#endif
            }//if need t4buf
#endif

            //if (gParams->gInitialInvocation) scaleLock = false; //Activate Control Scaling again

            //startclock = clock();
			
            res = call_FFP(FFP_ForEveryTile,(INT_PTR)&fmc); //res = ForEveryTile(&fmc);
            //if (!res) {
            //    res = call_FFP(FFP_ForEveryRow,(INT_PTR)&fmc);//res = ForEveryRow(&fmc);
            //}

            //fmc.renderTime = clock() - startclock;

        }
    }//scope


egress:
    // Free the temporary tile buffers if allocated...
    if (fmc.tbuf) {
        if (fmc.tbufID != 0){ //was allocated with Buffer Proc
			UnlockBuffer(fmc.tbufID);
			FreeBuffer(fmc.tbufID); 
		} else
			free (fmc.tbuf);

		fmc.tbufID = 0;
        fmc.tbuf = NULL;
#if 0
        Info("Freed the tile buffer.");
#endif
    }
    if (fmc.t2buf) {
        if (fmc.t2bufID != 0){ //was allocated with Buffer Proc
			UnlockBuffer(fmc.t2bufID);
			FreeBuffer(fmc.t2bufID); 
		} else
			free (fmc.t2buf);

		fmc.t2bufID = 0;
        fmc.t2buf = NULL;
#if 0
        Info("Freed tile buffer 2.");
#endif
    }
	if (fmc.t3buf) {
        if (fmc.t3bufID != 0){ //was allocated with Buffer Proc
			UnlockBuffer(fmc.t3bufID);
			FreeBuffer(fmc.t3bufID); 
		} else
			free (fmc.t3buf);

		fmc.t3bufID = 0;
        fmc.t3buf = NULL;
#if 0
        Info("Freed tile buffer 3.");
#endif
    }
	if (fmc.t4buf) {
        if (fmc.t4bufID != 0){ //was allocated with Buffer Proc
			UnlockBuffer(fmc.t4bufID);
			FreeBuffer(fmc.t4bufID); 
		} else
			free (fmc.t4buf);

		fmc.t4bufID = 0;
        fmc.t4buf = NULL;
#if 0
        Info("Freed tile buffer 4.");
#endif
    }
    } //scope
} //scope

if (fmc.doingProxy) {
    //Only one tile when doing proxy, so we can call the OnFilterEnd
    //handler here.

    int res = call_FFP(FFP_OnFilterEnd,(INT_PTR)&fmc);//OnFilterEnd(&fmc);

}//doingProxy

reentry_depth--;
} /*DoFilterRect*/

int copyInputToOutput()
{

	//return true;

#if BIGDOCUMENT
	fmc.outRect32 = GetOutRect();	//needed??
    fmc.rows = fmc.outRect32.bottom - fmc.outRect32.top;
    fmc.columns = fmc.outRect32.right - fmc.outRect32.left;		   
#else
	fmc.rows = gStuff->outRect.bottom - gStuff->outRect.top;
    fmc.columns = gStuff->outRect.right - gStuff->outRect.left;
#endif

    fmc.srcRowDelta = gStuff->inRowBytes;
    fmc.srcColDelta = gStuff->inHiPlane - gStuff->inLoPlane + 1;
	
    
			fmc.writeAlpha = 
#ifndef DONT_USE_MASK
                !gStuff->haveMask &&
#endif
                !gNoMask;


	//Unconditionally copy input tile to output...
        //Do this with memcpy instead????
        fmc.y = fmc.y_start;
        for (fmc.row = 0; fmc.row < fmc.rows; fmc.row++)
        {
            // No progress indicator -- but do testAbort()
            if (testAbort()) return false; //goto egress;
            /*else if (fmc.doingProxy) {
                if (updateProgress(fmc.row, fmc.rows)) goto egress;
            }
            else {
                if (updateProgress(fmc.y - Y0, fmc.Y - Y0)) goto egress;
                fmc.y++;
            }*/

            fmc.srcPtr = (unsigned8 *) MyAdjustedInData + (fmc.row * fmc.srcRowDelta);
            fmc.dstPtr = (unsigned8 *) gStuff->outData + (fmc.row * fmc.dstRowDelta);
            fmc.mskPtr = fmc.mskData + (fmc.row * fmc.mskRowDelta);

            //Copy over all planes and possibly the alpha...

            /*if (gStuff->imageMode == plugInModeRGB48 ||
                gStuff->imageMode == plugInModeGray16|| 
                gStuff->imageMode == plugInModeLab48 || 
                gStuff->imageMode == plugInModeCMYK64 ||
                gStuff->imageMode == plugInModeDeepMultichannel ||
                gStuff->imageMode == plugInModeDuotone16){ */
            
            if (gStuff->imageMode > 9) {

                // pixel depth = 16
#define srcPtr16 ((unsigned16 *)fmc.srcPtr)
#define dstPtr16 ((unsigned16 *)fmc.dstPtr)
#define mskPtr16 ((unsigned16 *)fmc.mskPtr)

                for (fmc.col = 0; fmc.col < fmc.columns; fmc.col++)
                {
                    int plane;
                    for (plane = 0; plane < gStuff->planes; plane++)
                       dstPtr16[plane] = srcPtr16[plane];

#if 0
                    // No alpha in 16-bit mode?
                    if (fmc.writeAlpha) {   //redundant???
                        dstPtr16[3] = mskVal(fmc.row, 0, fmc.col, 0);
                    }
#endif

                    //srcPtr16 += fmc.srcColDelta;
                    //dstPtr16 += fmc.dstColDelta;
                    fmc.srcPtr = fmc.srcPtr + 2 * fmc.srcColDelta;
                    fmc.dstPtr = fmc.dstPtr + 2 * fmc.dstColDelta;

                    ///mskPtr16 += fmc.mskColDelta;    //wrong????

                } /* for fmc.col */
            }
            else {
                // pixel depth = 8
                for (fmc.col = 0; fmc.col < fmc.columns; fmc.col++)
                {
                    int plane;
                    for (plane = 0; plane < gStuff->planes; plane++)
                       fmc.dstPtr[plane] = fmc.srcPtr[plane];
#if 0
                    if (fmc.writeAlpha) {   //redundant???
                        fmc.dstPtr[3] = mskVal(fmc.row, 0, fmc.col, 0);
                    }
#endif
                    fmc.srcPtr += fmc.srcColDelta;
                    fmc.dstPtr += fmc.dstColDelta;
                    fmc.mskPtr += fmc.mskColDelta;  //wrong????
                } /* for fmc.col */
            } /* if 8-bit depth */
        } /* for fmc.row */

		return true;
}




/******************************************************/

#if 0

static int fm_doForEveryRow(void) {
    int res;

    // re-initialize in case ForEveryTile() altered these values...
    fmc.y = fmc.y_start;

    for (fmc.row = 0; fmc.row < fmc.rows; fmc.row++, fmc.y++)
    {
        if (fmc.doingProxy) {
            if (res = updateProgress(fmc.row, fmc.rows)) return res;
        }
        else {
            if (res = updateProgress(fmc.y - Y0, fmc.Y - Y0)) return res;
        }

        fmc.srcPtr = (unsigned8 *) MyAdjustedInData + (fmc.row * fmc.srcRowDelta);
        fmc.dstPtr = (unsigned8 *) gStuff->outData + (fmc.row * fmc.dstRowDelta);
        fmc.mskPtr = fmc.mskData + (fmc.row * fmc.mskRowDelta);

        fmc.x = fmc.x_start;
        fmc.col = 0;        //redundant??? DoOneRow sets it also.
        fmc.writeAlpha = 
#ifndef DONT_USE_MASK
                !gStuff->haveMask &&
#endif
            !gNoMask;
        // call ForEveryRow() here.
        // or call it from within DoOneRow???
        // Then call DoOneRow to drive the ForEveryPixel code...
        //execute the compiled code...
        call_FFP(FFP_ForEveryRow,(INT_PTR)gFmcp); //ForEveryRow(gFmcp);

    } /* for fmc.row */
    return 0;   //success
} /*fm_doForEveryRow*/

#endif

/******************************************************/

/* DoFilterRect for non-advanceState */


void DoFilterRectNoAdvanceState (GPtr globals)
{
#ifndef NOTRACE
    Trace (globals, "DoFilterRectNoAdvanceState:\n"
                    " Didn't expect to come here!");
#endif
}


/*****************************************************************************/

/* If we do all of the filtering during the start phase, the continue
   phase is negligible. */

/* Otherwise (no AdvanceState) Filters the area and requests the next area. */