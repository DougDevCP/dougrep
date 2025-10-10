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
    //fmc.Z = fmc.zmax = fmc.pmax = (gStuff->planes > 4)?4:gStuff->planes; //What about CMYK + alpha???
    fmc.Z = fmc.zmax = fmc.pmax = gStuff->planes;
    fmc.M = fmc.mmax = (int)( sqrt((double)(fmc.X*fmc.X + fmc.Y*fmc.Y)) / 2 + 0.0 ); // don't round per Jens???
    fmc.dmax = fmc.D = 1024;
    fmc.xmin = fmc.ymin = fmc.zmin = fmc.pmin = fmc.mmin = fmc.dmin = 0;
    fmc.rmax = fmc.gmax = fmc.bmax = fmc.amax = fmc.cmax = 255;	//should change with bit-depth??
    fmc.imax = fmc.umax = fmc.vmax = 255;
    fmc.rmin = fmc.gmin = fmc.bmin = fmc.amin = fmc.cmin = 0;
    fmc.imin = fmc.umin = fmc.vmin = 0;
    fmc.R = fmc.G = fmc.B = fmc.A = fmc.C = 255;    //conflict between FF and FF+???
    fmc.I = fmc.U = fmc.V = 255;
    fmc.t = fmc.tmin = 0;
    fmc.total = fmc.tmax = 1;

#if 0   //already done in InitBeforeFilter.
    // Cache current control values in fmc.pre_ctl[].
    for (i=0; i < N_CTLS; i++) {
        fmc.pre_ctl[i] = gParams->ctl[i].val;
    }
#endif

    fmc.hDlg = MyHDlg; //already done in WM_INITDIALOG!!!

    // Compute min/max values for column and row indices that still lie
    // within the (scaled) filterRect (used to simulate padding of accesses
    // outside the bounds of filterRect).
#if BIGDOCUMENT
	/*fmc.outRect32 = GetOutRect();	//needed??
    minCol = scaledFilterRect.left   - fmc.outRect32.left;
    maxCol = scaledFilterRect.right  - fmc.outRect32.left - 1;
    minRow = scaledFilterRect.top    - fmc.outRect32.top;
    maxRow = scaledFilterRect.bottom - fmc.outRect32.top - 1;*/
#else
    /*minCol = scaledFilterRect.left   - gStuff->outRect.left;
    maxCol = scaledFilterRect.right  - gStuff->outRect.left - 1;
    minRow = scaledFilterRect.top    - gStuff->outRect.top;
    maxRow = scaledFilterRect.bottom - gStuff->outRect.top - 1;*/
#endif


    {   

#if NOISECONTROL
#ifndef NOOUTPUTCOPY
    if (ctl(fmc.previewCheckbox)==0){
#endif            
#endif

        // pass 2: ...
		if (!copyInputToOutput()) goto egress;

#if NOISECONTROL
#ifndef NOOUTPUTCOPY
    }
#endif
#endif

        //call ForEveryTile...
        fmc.x = fmc.x_start;
        fmc.y = fmc.y_start;
        fmc.x_end = fmc.x_start + fmc.columns;
        fmc.y_end = fmc.y_start + fmc.rows;
        fmc.row = 0;
        fmc.col = 0;
        fmc.tbuf = NULL;
        fmc.t2buf = NULL;
        fmc.t3buf = NULL;
        fmc.t4buf = NULL;

        {
            //need to handle longjmp back???
            int res;
#if 0
            //Min version of tile buffers (same size as output tile)
            if (fmc.need_tbuf) {
                //allocate a (temporary) tile buffer...
                //Should use AllocateBuffer() ????
                do {
                    res = IDOK;

                    //some kind of dword padding (+ 3) added by Harald Heim, February 2003

					fmc.tbuf = calloc( fmc.rows * fmc.columns * fmc.Z + 3 , sizeof (fmc.tbuf[0]) );

                    if (!fmc.tbuf) {
                        res = Error("Could not allocate a tile buffer (%d bytes).",
                                fmc.rows * fmc.columns * fmc.Z * sizeof (fmc.tbuf[0]));
                    }
                } while (res == IDRETRY);
                if (res == IDABORT) {
                    gResult = memFullErr;
                    goto egress;
                }
#if 0
                else {
                    Info("Allocated a tile buffer (%d bytes).",
                            fmc.rows * fmc.columns * fmc.Z * sizeof (fmc.tbuf[0]));
                }
#endif
            }//if need tbuf
            if (fmc.need_t2buf) {
                //allocate a (temporary) tile buffer 2...
                //Should use AllocateBuffer() ????
                do {
                    res = IDOK;

                    //some kind of dword padding (+ 3) added by Harald Heim, February 2003
 
                    fmc.t2buf = calloc(fmc.rows * fmc.columns * fmc.Z + 3, sizeof (fmc.t2buf[0]));

                    if (!fmc.t2buf) {
                        res = Error("Could not allocate tile buffer 2 (%d bytes).",
                                fmc.rows * fmc.columns * fmc.Z * sizeof (fmc.t2buf[0]));
                    }
                } while (res == IDRETRY);
                if (res == IDABORT) {
                    gResult = memFullErr;
                    goto egress;
                }
#if 0
                else {
                    Info("Allocated tile buffer 2 (%d bytes).",
                            fmc.rows * fmc.columns * fmc.Z * sizeof (fmc.t2buf[0]));
                }
#endif
            }//if need t2buf
			if (fmc.need_t3buf) {
                //allocate a (temporary) tile buffer 3...
                //Should use AllocateBuffer() ????
                do {
                    res = IDOK;

                    //some kind of dword padding (+ 3) added by Harald Heim, February 2003
 
                    fmc.t3buf = calloc(fmc.rows * fmc.columns * fmc.Z + 3, sizeof (fmc.t3buf[0]));

                    if (!fmc.t3buf) {
                        res = Error("Could not allocate tile buffer 3 (%d bytes).",
                                fmc.rows * fmc.columns * fmc.Z * sizeof (fmc.t3buf[0]));
                    }
                } while (res == IDRETRY);
                if (res == IDABORT) {
                    gResult = memFullErr;
                    goto egress;
                }
#if 0
                else {
                    Info("Allocated tile buffer 3 (%d bytes).",
                            fmc.rows * fmc.columns * fmc.Z * sizeof (fmc.t3buf[0]));
                }
#endif
            }//if need t3buf
			if (fmc.need_t4buf) {
                //allocate a (temporary) tile buffer 4...
                //Should use AllocateBuffer() ????
                do {
                    res = IDOK;

                    //some kind of dword padding (+ 3) added by Harald Heim, February 2003
 
                    fmc.t4buf = calloc(fmc.rows * fmc.columns * fmc.Z + 3, sizeof (fmc.t3buf[0]));

                    if (!fmc.t4buf) {
                        res = Error("Could not allocate tile buffer 4 (%d bytes).",
                                fmc.rows * fmc.columns * fmc.Z * sizeof (fmc.t4buf[0]));
                    }
                } while (res == IDRETRY);
                if (res == IDABORT) {
                    gResult = memFullErr;
                    goto egress;
                }
#if 0
                else {
                    Info("Allocated tile buffer 4 (%d bytes).",
                            fmc.rows * fmc.columns * fmc.Z * sizeof (fmc.t4buf[0]));
                }
#endif
            }//if need t4buf
#else
            //Max version of tile buffers (same size as input tile)
            if (fmc.need_tbuf) {
                //allocate a (temporary) tile buffer...
                //Should use AllocateBuffer() ????
				
                do {
                    int bufferSize = fmc.imageMode>9 ? 2:1;
					
					res = IDOK;

                    //some kind of dword padding (+ 3) added by Harald Heim, February 2003
					
					if (gStuff->bufferProcs && gStuff->bufferProcs->allocateProc && gStuff->bufferProcs->lockProc && gStuff->bufferProcs->unlockProc && gStuff->bufferProcs->freeProc){ //Photoshop allocation
						if (AllocateBuffer( (fmc.X * fmc.Y * fmc.Z + 3) * bufferSize, &fmc.tbufID) == noErr){
							fmc.tbuf = (unsigned char *)LockBuffer(fmc.tbufID, true);
						} else { //Fall back to Windows allocation
							fmc.tbuf = (unsigned char *)calloc(fmc.X * fmc.Y * fmc.Z + 3, bufferSize);
							fmc.tbufID=0;
						}
					}else { //Windows allocation
						fmc.tbuf = (unsigned char *)calloc(fmc.X * fmc.Y * fmc.Z + 3, bufferSize); //sizeof (fmc.tbuf[0])
						fmc.tbufID=0;
					}


                    if (!fmc.tbuf) {
                        res = Error("Could not allocate a tile buffer (%d bytes).",
                                fmc.X * fmc.Y * fmc.Z * bufferSize); //sizeof (fmc.tbuf[0])
                    }
                } while (res == IDRETRY);
                if (res == IDABORT) {
                    gResult = memFullErr;
                    goto egress;
                }
#if 0
                else {
                    Info("Allocated a tile buffer (%d bytes).",
                            fmc.X * fmc.Y * fmc.Z * sizeof (fmc.tbuf[0]));
                }
#endif
            }//if need tbuf
            if (fmc.need_t2buf) {
                //allocate a (temporary) tile buffer 2...
                //Should use AllocateBuffer() ????
                do {
                    int bufferSize = fmc.imageMode>9 ? 2:1;
					res = IDOK;

                    //some kind of dword padding (+ 3) added by Harald Heim, February 2003

                    if (gStuff->bufferProcs && gStuff->bufferProcs->allocateProc && gStuff->bufferProcs->lockProc && gStuff->bufferProcs->unlockProc && gStuff->bufferProcs->freeProc){ //Photoshop allocation
						if (AllocateBuffer( (fmc.X * fmc.Y * fmc.Z + 3) * bufferSize, &fmc.t2bufID) == noErr){
							fmc.t2buf = (unsigned char *)LockBuffer(fmc.t2bufID, true);
						} else { //Fall back to Windows allocation
							fmc.t2buf = (unsigned char *)calloc(fmc.X * fmc.Y * fmc.Z + 3, bufferSize);
							fmc.t2bufID=0;
						}
					}else { //Windows allocation
						fmc.t2buf = (unsigned char *)calloc(fmc.X * fmc.Y * fmc.Z + 3, bufferSize); //sizeof (fmc.t2buf[0])
						fmc.t2bufID=0;
					}


                    if (!fmc.t2buf) {
                        res = Error("Could not allocate tile buffer 2 (%d bytes).",
                                fmc.X * fmc.Y * fmc.Z * bufferSize); //sizeof (fmc.t2buf[0])
                    }
                } while (res == IDRETRY);
                if (res == IDABORT) {
                    gResult = memFullErr;
                    goto egress;
                }
#if 0
                else {
                    Info("Allocated tile buffer 2 (%d bytes).",
                            fmc.X * fmc.Y * fmc.Z * sizeof (fmc.t2buf[0]));
                }
#endif
            }//if need t2buf
			if (fmc.need_t3buf) {
                //allocate a (temporary) tile buffer 3...
                //Should use AllocateBuffer() ????
                do {
                    int bufferSize = fmc.imageMode>9 ? 2:1;
					res = IDOK;

                    //some kind of dword padding (+ 3) added by Harald Heim, February 2003
					
					if (gStuff->bufferProcs && gStuff->bufferProcs->allocateProc && gStuff->bufferProcs->lockProc && gStuff->bufferProcs->unlockProc && gStuff->bufferProcs->freeProc){ //Photoshop allocation
						if (AllocateBuffer( (fmc.X * fmc.Y * fmc.Z + 3) * bufferSize, &fmc.t3bufID) == noErr){
							fmc.t3buf = (unsigned char *)LockBuffer(fmc.t3bufID, true);
						} else { //Fall back to Windows allocation
							fmc.t3buf = (unsigned char *)calloc(fmc.X * fmc.Y * fmc.Z + 3, bufferSize);
							fmc.t3bufID=0;
						}
					}else { //Windows allocation
						fmc.t3buf = (unsigned char *)calloc(fmc.X * fmc.Y * fmc.Z + 3, bufferSize); //sizeof (fmc.tbuf[0])
						fmc.t3bufID=0;
					}

                    if (!fmc.t3buf) {
                        res = Error("Could not allocate tile buffer 3 (%d bytes).",
                                fmc.X * fmc.Y * fmc.Z * bufferSize); //sizeof (fmc.t3buf[0])
                    }
                } while (res == IDRETRY);
                if (res == IDABORT) {
                    gResult = memFullErr;