							v3 = fmc.t2buf[(y_int*fmc.X + x_int1)*fmc.Z + z];
							v4 = fmc.t2buf[(y_int1*fmc.X + x_int1)*fmc.Z + z];
						} else {
							v1 = *(unsigned16 *)(((unsigned8 *)fmc.t2buf) + 2*((y_int*fmc.X + x_int)*fmc.Z + z));
							v2 = *(unsigned16 *)(((unsigned8 *)fmc.t2buf) + 2*((y_int1*fmc.X + x_int)*fmc.Z + z));
							v3 = *(unsigned16 *)(((unsigned8 *)fmc.t2buf) + 2*((y_int*fmc.X + x_int1)*fmc.Z + z));
							v4 = *(unsigned16 *)(((unsigned8 *)fmc.t2buf) + 2*((y_int1*fmc.X + x_int1)*fmc.Z + z));
						}
                        break;

                    case 3: //output buffer
                        if (gStuff->imageMode < 10){
                            v1 = ((unsigned8 *)gStuff->outData)
                                [(x_int-fmc.x_start)*fmc.dstColDelta + (y_int-fmc.y_start)*fmc.dstRowDelta + z];
                            v2 = ((unsigned8 *)gStuff->outData)
                                [(x_int-fmc.x_start)*fmc.dstColDelta + (y_int1-fmc.y_start)*fmc.dstRowDelta + z];
                            v3 = ((unsigned8 *)gStuff->outData)
                                [(x_int1-fmc.x_start)*fmc.dstColDelta + (y_int-fmc.y_start)*fmc.dstRowDelta + z];
                            v4 = ((unsigned8 *)gStuff->outData)
                                [(x_int1-fmc.x_start)*fmc.dstColDelta + (y_int1-fmc.y_start)*fmc.dstRowDelta + z];
                        } else {
                            v1 = *(unsigned16 *)  ( ((unsigned8 *)gStuff->outData) +
                                (((x_int-fmc.x_start)*fmc.dstColDelta + z)<<1 ) + (y_int-fmc.y_start)*fmc.dstRowDelta  );
                            v2 = *(unsigned16 *)  ( ((unsigned8 *)gStuff->outData) +
                                (((x_int-fmc.x_start)*fmc.dstColDelta + z)<<1 ) + (y_int1-fmc.y_start)*fmc.dstRowDelta  );
                            v3 = *(unsigned16 *)  ( ((unsigned8 *)gStuff->outData) +
                                (((x_int1-fmc.x_start)*fmc.dstColDelta + z)<<1 ) + (y_int-fmc.y_start)*fmc.dstRowDelta  );
                            v4 = *(unsigned16 *)  ( ((unsigned8 *)gStuff->outData) +
                                (((x_int1-fmc.x_start)*fmc.dstColDelta + z)<<1 ) + (y_int1-fmc.y_start)*fmc.dstRowDelta  );
                        }
						break;
					
					case 4: //temp buffer 3
						if (gStuff->imageMode < 10){
							v1 = fmc.t3buf[(y_int*fmc.X + x_int)*fmc.Z + z];
							v2 = fmc.t3buf[(y_int1*fmc.X + x_int)*fmc.Z + z];
							v3 = fmc.t3buf[(y_int*fmc.X + x_int1)*fmc.Z + z];
							v4 = fmc.t3buf[(y_int1*fmc.X + x_int1)*fmc.Z + z];
						} else {
							v1 = *(unsigned16 *)(((unsigned8 *)fmc.t3buf) + 2*((y_int*fmc.X + x_int)*fmc.Z + z));
							v2 = *(unsigned16 *)(((unsigned8 *)fmc.t3buf) + 2*((y_int1*fmc.X + x_int)*fmc.Z + z));
							v3 = *(unsigned16 *)(((unsigned8 *)fmc.t3buf) + 2*((y_int*fmc.X + x_int1)*fmc.Z + z));
							v4 = *(unsigned16 *)(((unsigned8 *)fmc.t3buf) + 2*((y_int1*fmc.X + x_int1)*fmc.Z + z));
						}
                        break;
					case 5: //temp buffer 4
						if (gStuff->imageMode < 10){
							v1 = fmc.t4buf[(y_int*fmc.X + x_int)*fmc.Z + z];
							v2 = fmc.t4buf[(y_int1*fmc.X + x_int)*fmc.Z + z];
							v3 = fmc.t4buf[(y_int*fmc.X + x_int1)*fmc.Z + z];
							v4 = fmc.t4buf[(y_int1*fmc.X + x_int1)*fmc.Z + z];
						} else {
							v1 = *(unsigned16 *)(((unsigned8 *)fmc.t4buf) + 2*((y_int*fmc.X + x_int)*fmc.Z + z));
							v2 = *(unsigned16 *)(((unsigned8 *)fmc.t4buf) + 2*((y_int1*fmc.X + x_int)*fmc.Z + z));
							v3 = *(unsigned16 *)(((unsigned8 *)fmc.t4buf) + 2*((y_int*fmc.X + x_int1)*fmc.Z + z));
							v4 = *(unsigned16 *)(((unsigned8 *)fmc.t4buf) + 2*((y_int1*fmc.X + x_int1)*fmc.Z + z));
						}
                        break;
                }
                

/*
			    //Does not work with multi-threading
                //Set static variables for caching
                x_old = x;
                y_old = y;
                mode_old = mode;
                edgeMode_old=edgeMode;
                fracx_old=fracx;
                fracy_old=fracy;
                x_int_old=x_int;
                y_int_old=y_int; 
                x_int1_old=x_int1;
                y_int1_old=y_int1;
*/

                return (int) ((1.0-fracx)* ((1.0-fracy)*v1 + fracy*v2)  + fracx* ((1.0-fracy)*v3 + fracy*v4));
        
            }

            else if (mode==4) //bicubic interpolation
            {

                //For Bicubic Interpolation 
               double nx,ny,ox3,oy3,nx3,ny3;
               double ix[4];//ix1, ix2, ix3, ix4;
               int j,k;
               int i[4];//i1,i2,i3,i4;
               double x1,x2,x3,x4,y1,y2,y3,y4,y9;
            
             //#ifndef HYPERTYLE
			 #if !defined(HYPERTYLE) && !defined(PLUGINGALAXY)
                if (x < 0) x = 0; else if (x >= fmc.X) x = (double)(fmc.X - 1);
                if (y < 0) y = 0; else if (y >= fmc.Y) y = (double)(fmc.Y - 1);
             #else
                switch (edgeMode)
                {
                    case 0:
                        if (x < 0) x = 0; else if (x >= fmc.X) x = (double)(fmc.X - 1);
                        if (y < 0) y = 0; else if (y >= fmc.Y) y = (double)(fmc.Y - 1);
                        break;
                    case 1:
                        if (x < 0 || y < 0 || x >= (fmc.X - 1)|| y >= (fmc.Y - 1))
                            return 0;
						break;				//was missing!!
                    case 2:
                        //if (x < 0) x = fmod(fmc.X + x , fmc.X);
                        //else if (x >= fmc.X) x = fmod(x - fmc.X , fmc.X);
                        //if (y < 0) y = fmod(fmc.Y + y , fmc.Y);
                        //else if (y >= fmc.Y) y = fmod(y - fmc.Y , fmc.Y);
						if (x < 0) x = (fmc.X-1) + fmod(x,(fmc.X-1));
						else if (x >= fmc.X) x = fmod(x,(fmc.X-1));
						if (y < 0) y = (fmc.Y-1) + fmod(y,(fmc.Y-1));
						else if (y >= fmc.Y) y = fmod(y,(fmc.Y-1));
                        break;
                    case 3:
                        //else if (x >= fmc.X) x = (fmc.X - 1) - (x - (fmc.X - 1));
						//else if (y >= fmc.Y) y = (fmc.Y - 1) - (y - (fmc.Y - 1));
						if (x < 0) x = - fmod(x,fmc.X - 1);
                        else if (x >= fmc.X) x = (fmc.X - 1) - fmod(x,fmc.X - 1);
                        if (y < 0) y = - fmod(y,fmc.Y - 1);
                        else if (y >= fmc.Y) y = (fmc.Y - 1) - fmod(y,fmc.Y - 1);
						break;
					default:
						;	//afh 01-may-2008
                }//end switch
            #endif

               y_int = (int)y; //yy
               x_int = (int)x; //xx

               fracx=x-x_int;
               fracy=y-y_int;

               nx = 1. - fracx;
               ny = 1. - fracy;
               ox3 = (fracx * fracx * fracx) - fracx;
               oy3 = (fracy * fracy * fracy) - fracy;
               nx3 = (nx * nx * nx) - nx;
               ny3 = (ny * ny * ny) - ny;

                
               for (j=-1;j<3;j++){

                   y_int1 = y_int + j;

                //#ifndef HYPERTYLE
			    #if !defined(HYPERTYLE) && !defined(PLUGINGALAXY)
                    if (y_int1 < 0) y_int1 = 0;
                    if (y_int1 >= fmc.Y) y_int1 = (fmc.Y - 1);
                #else
                   switch (edgeMode)
                    {
                    case 0: 
                        if (y_int1 < 0) y_int1 = 0;
                        else if (y_int1 >= fmc.Y) y_int1 = (fmc.Y - 1);
                        break;
                    case 1: 
						if (y_int1 >= fmc.Y) y_int1 = (fmc.Y - 1);
                        break;
                    case 2:
                        //if (y_int1 >= fmc.Y) y_int1 = (y_int1  - fmc.Y)% fmc.Y;
						if (y_int1 >= fmc.Y) y_int1 = y_int1 % (fmc.Y-1);
                        break;
                    case 3:
						//if (y_int1 < 0) y_int1 = - y_int1;
						//else if (y_int1 >= fmc.Y) y_int1 = (fmc.Y - 1)- (y_int1 - (fmc.Y - 1));
						if (y_int1 < 0) y_int1 = - y_int1%(fmc.Y-1);
						else if (y_int1 >= fmc.Y) y_int1 = (fmc.Y - 1) -y_int1%(fmc.Y-1);
						break;
					default:
						break;	//afh 01-may-2008
                    }//end switch
                #endif
               
                   for (k=-1;k<3;k++) {
                        
                        x_int1 = x_int + k;

                    //#ifndef HYPERTYLE
					#if !defined(HYPERTYLE) && !defined(PLUGINGALAXY)
                        if (x_int1 < 0) x_int1 = 0;
                        if (x_int1 >= fmc.X) x_int1 = (fmc.X - 1);
                    #else
                        switch (edgeMode)
                        {
                            case 0: 
                                if (x_int1 < 0) x_int1 = 0; //Required for PSP X3 and older
                                if (x_int1 >= fmc.X) x_int1 = (fmc.X - 1);
                                break;
                            case 1: 
								if (x_int1 < 0) x_int1 = 0; //Required for PSP X3 and older
                                if (x_int1 >= fmc.X) x_int1 = (fmc.X - 1);
                                break;
                            case 2:
								if (x_int1 < 0) x_int1 = 0; //Required for PSP X3 and older
                                //if (x_int1 >= fmc.X) x_int1 = (x_int1 - fmc.X) % fmc.X;
								if (x_int1 >= fmc.X) x_int1 = x_int1 % (fmc.X-1);
                                break;                        
							case 3:
								if (x_int1 < 0) x_int1 = 0; //Required for PSP X3 and older
								//if (x_int1 >= fmc.X) x_int1 = (fmc.X - 1) - (x_int1 - (fmc.X - 1));
								if (x_int1 >= fmc.X) x_int1 = (fmc.X - 1) - x_int1%(fmc.X-1);
								break;
							default:
								break;	//afh 01-may-2008
                        }//end switch
                    #endif

                       switch (buffer){
							default:
                            case 0: //input buffer
                                if (gStuff->imageMode < 10)
                                    i[k+1] = ((unsigned8 *)MyAdjustedInData)
                                        [(x_int1 -fmc.x_start)*fmc.srcColDelta + (y_int1 -fmc.y_start)*fmc.srcRowDelta + z];
                                else
                                    i[k+1] = *(unsigned16 *)(((unsigned8 *)MyAdjustedInData) +
                                        (((x_int1-fmc.x_start)*fmc.srcColDelta + z) << 1) +
                                        (y_int1-fmc.y_start)*fmc.srcRowDelta);
                                break;
                            case 1: //temp buffer 1
								if (gStuff->imageMode < 10)
									i[k+1] = fmc.tbuf[( y_int1 *fmc.X + x_int1 )*fmc.Z + z];
								else
									i[k+1] = *(unsigned16 *)(((unsigned8 *)fmc.tbuf) + 2*((y_int1*fmc.X + x_int1)*fmc.Z + z));
                                break;
                            case 2: //temp buffer 2
								if (gStuff->imageMode < 10)
									i[k+1] = fmc.t2buf[( y_int1 *fmc.X + x_int1 )*fmc.Z + z];
								else
									i[k+1] = *(unsigned16 *)(((unsigned8 *)fmc.t2buf) + 2*((y_int1*fmc.X + x_int1)*fmc.Z + z));
                                break;
                            case 3: //output buffer
                                if (gStuff->imageMode < 10)
                                    i[k+1] = ((unsigned8 *)gStuff->outData)
                                        [(x_int1 -fmc.x_start)*fmc.dstColDelta + (y_int1 -fmc.y_start)*fmc.dstRowDelta + z];
                                else
                                    i[k+1] = *(unsigned16 *)  ( ((unsigned8 *)gStuff->outData) +
                                        (((x_int1-fmc.x_start)*fmc.dstColDelta + z)<<1 ) + (y_int1-fmc.y_start)*fmc.dstRowDelta  );
								break;
							case 4: //temp buffer 3
								if (gStuff->imageMode < 10)
									i[k+1] = fmc.t3buf[( y_int1 *fmc.X + x_int1 )*fmc.Z + z];
								else
									i[k+1] = *(unsigned16 *)(((unsigned8 *)fmc.t3buf) + 2*((y_int1*fmc.X + x_int1)*fmc.Z + z));
                                break;
							case 5: //temp buffer 4
								if (gStuff->imageMode < 10)
									i[k+1] = fmc.t4buf[( y_int1 *fmc.X + x_int1 )*fmc.Z + z];
								else
									i[k+1] = *(unsigned16 *)(((unsigned8 *)fmc.t4buf) + 2*((y_int1*fmc.X + x_int1)*fmc.Z + z));
                                break;
                        }

                    }//for k

                   
                   x1 = (i[2] - i[1]) - (i[1] - i[0]);
                   x2 = (i[3] - i[2]) - (i[2] - i[1]);
                   x3 = x1 - x2 / 4.;
                   x4 = x2 - x1 / 4.;
                   ix[j+1] = (fracx * i[2]) + (nx * i[1])
                    + (((x4 * ox3) + (x3 * nx3)) / 3.75);
               }//for j


               // Y
               y1 = (ix[2] - ix[1]) - (ix[1] - ix[0]);
               y2 = (ix[3] - ix[2]) - (ix[2] - ix[1]);
               y3 = y1 - y2 / 4.;
               y4 = y2 - y1 / 4.;
               y9 = (fracy * ix[2]) + (ny * ix[1])
                + (((y4 * oy3) + (y3 * ny3)) / 3.75);


                //Set static variables for caching
                //nx,ny,ox3,oy3,nx3,ny3

               return (int)y9;

            }//end bicubic



        }// end interpolation methods


    }
    // bad channel number??
    return 0; 	//afh 01-may-2008
} /*fm_iget*/




//function added by Harald Heim on Feb 13, 2002
int bCircle(int x, int y, int centerx, int centery, int radius)
{
    return c2m(x-centerx,y-centery) <= radius ? 1: 0;
    