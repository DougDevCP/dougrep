    double fracx, fracy;
/*
	//Does not work with multi-threading
    //Cached values
    THREAD_LOCAL static int x_int_old, y_int_old, x_int1_old, y_int1_old,mode_old, edgeMode_old;
    THREAD_LOCAL static double fracx_old, fracy_old, x_old, y_old;
*/

    //Info ("x: %f - y: %f",x,y);

    if (z >= 0 && z < fmc.Z)
    {
        
        if (mode==0) //Nearest Neighbor
        {
            
			//x_int=(int) x;
            //y_int=(int) y;
			//x_int=(int)(x>INT_MAX?INT_MAX:x); 
            //y_int=(int)(y>INT_MAX?INT_MAX:y); 
			x_int=(int)(unsigned int)x; //Avoid crash with too large double value becoming negative int
            y_int=(int)(unsigned int)y;

        //#ifndef HYPERTYLE
		#if !defined(HYPERTYLE) && !defined(PLUGINGALAXY)
            if (x_int < 0) x_int = 0; else if (x_int >= fmc.X) x_int = fmc.X - 1;
            if (y_int < 0) y_int = 0; else if (y_int >= fmc.Y) y_int = fmc.Y - 1;
        #else
            switch(edgeMode)
            {
                case 0:
                    if (x_int < 0) x_int = 0; else if (x_int >= fmc.X) x_int = fmc.X - 1;
                    if (y_int < 0) y_int = 0; else if (y_int >= fmc.Y) y_int = fmc.Y - 1;
                    break;

                case 1:
                    if (x_int < 0 || y_int < 0 || x_int >= fmc.X || y_int >= fmc.Y)
                        return 0;
					break;				//was missing!!
                
                case 2:
                    //if (x_int < 0) x_int = (fmc.X - 1) + x_int;
					//else if (x_int >= fmc.X) x_int = (x_int - (fmc.X - 1)) % fmc.X;
                    //if (y_int < 0) y_int = (fmc.Y - 1) + y_int;
                    //else if (y_int >= fmc.Y) y_int = (y_int - (fmc.Y - 1)) % fmc.Y;
					if (x_int < 0) x_int = (fmc.X-1) + x_int%(fmc.X-1);
					else if (x_int >= fmc.X) x_int = x_int % (fmc.X-1);
					if (y_int < 0) y_int = (fmc.Y-1) + y_int%(fmc.Y-1);
					else if (y_int >= fmc.Y) y_int = y_int % (fmc.Y-1);
                    break;

                case 3:
                    //if (x_int < 0) x_int = - x_int;
                    //else if (x_int >= fmc.X) x_int = (fmc.X - 1) - (x_int - (fmc.X - 1));
                    //if (y_int < 0) y_int = - y_int;
                    //else if (y_int >= fmc.Y) y_int = (fmc.Y - 1) - (y_int - (fmc.Y - 1));
					if (x_int < 0) x_int = - x_int%(fmc.X-1);
                    else if (x_int >= fmc.X) x_int = (fmc.X - 1) -x_int%(fmc.X-1);
					if (y_int < 0) y_int = - y_int%(fmc.Y-1);
                    else if (y_int >= fmc.Y) y_int = (fmc.Y - 1) -y_int%(fmc.Y-1);
					break;	//good practice

				default:	//no special handling
					;	//afh 01-may-2008
            }//end switch 
        #endif

            switch (buffer)
            {
				default:	//do something reasonable even if "buffer" isn't.
                case 0: //input buffer
                    if (gStuff->imageMode < 10)
                        return ((unsigned8 *)MyAdjustedInData)
                            [(x_int-fmc.x_start)*fmc.srcColDelta + (y_int-fmc.y_start)*fmc.srcRowDelta + z];
                    else
                        return *(unsigned16 *)(((unsigned8 *)MyAdjustedInData) +
                               (((x_int-fmc.x_start)*fmc.srcColDelta + z) << 1) +
                               (y_int-fmc.y_start)*fmc.srcRowDelta);
                case 1: //t buffer 1
                    if (gStuff->imageMode < 10)
						return fmc.tbuf[(y_int*fmc.X + x_int)*fmc.Z + z];
					else
						return *(unsigned16 *)(((unsigned8 *)fmc.tbuf) + 2*((y_int*fmc.X + x_int)*fmc.Z + z));
                case 2: //t buffer 2
					if (gStuff->imageMode < 10)
						return fmc.t2buf[(y_int*fmc.X + x_int)*fmc.Z + z];
					else
						return *(unsigned16 *)(((unsigned8 *)fmc.t2buf) + 2*((y_int*fmc.X + x_int)*fmc.Z + z));
                case 3: //output buffer
                    if (gStuff->imageMode < 10)
                        return ((unsigned8 *)gStuff->outData)
                            [(x_int-fmc.x_start)*fmc.dstColDelta + (y_int-fmc.y_start)*fmc.dstRowDelta + z];
                    else
                        return *(unsigned16 *)  ( ((unsigned8 *)gStuff->outData) +
                            (((x_int-fmc.x_start)*fmc.dstColDelta + z)<<1 ) + (y_int-fmc.y_start)*fmc.dstRowDelta  );
				case 4: //t buffer 3
					if (gStuff->imageMode < 10)
						return fmc.t3buf[(y_int*fmc.X + x_int)*fmc.Z + z];
					else
						return *(unsigned16 *)(((unsigned8 *)fmc.t3buf) + 2*((y_int*fmc.X + x_int)*fmc.Z + z));
				case 5: //t buffer 4
					if (gStuff->imageMode < 10)
						return fmc.t4buf[(y_int*fmc.X + x_int)*fmc.Z + z];
					else
						return *(unsigned16 *)(((unsigned8 *)fmc.t4buf) + 2*((y_int*fmc.X + x_int)*fmc.Z + z));
            }


        }

        else 
        {
            
            if (mode<4) 
            {
				int v1,v2,v3,v4;

/*
                //Does not work with multi-threading
				if (x==x_old && y==y_old && mode==mode_old && edgeMode==edgeMode_old)
                {
                    //Set cached values
                    fracx=fracx_old;
                    fracy=fracy_old;
                    x_int=x_int_old;
                    y_int=y_int_old; 
                    x_int1=x_int1_old;
                    y_int1=y_int1_old;
                }

                else //Calculate the values
*/
                {

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
                            //if (x < 0) x = fmc.X + x;
                            //else if (x >= fmc.X) x = fmod(x - fmc.X , fmc.X) ;
                            //if (y < 0) y = fmc.Y + y;
                            //else if (y >= fmc.Y) y = fmod(y - fmc.Y , fmc.Y);
							if (x < 0) x = (fmc.X-1) + fmod(x,(fmc.X-1));
							else if (x >= fmc.X) x = fmod(x,(fmc.X-1));
							if (y < 0) y = (fmc.Y-1) + fmod(y,(fmc.Y-1));
							else if (y >= fmc.Y) y = fmod(y,(fmc.Y-1));
                            break;

                        case 3:
                            //if (x < 0) x = - x;
                            //else if (x >= fmc.X) x = (fmc.X - 1) - (x - (fmc.X - 1));
                            //if (y < 0) y = - y;
                            //else if (y >= fmc.Y) y = (fmc.Y - 1) - (y - (fmc.Y - 1));
							if (x < 0) x = - fmod(x,fmc.X - 1);
							else if (x >= fmc.X) x = (fmc.X - 1) - fmod(x,fmc.X - 1);
							if (y < 0) y = - fmod(y,fmc.Y - 1);
							else if (y >= fmc.Y) y = (fmc.Y - 1) - fmod(y,fmc.Y - 1);
							break;

						default:	//no special handling
							;	//afh 01-may-2008
                    }//end switch
                #endif

                    x_int=(int) x;
                    y_int=(int) y;
					
                    x_int1=x_int+1;
                    y_int1=y_int+1;

                //#ifndef HYPERTYLE
				#if !defined(HYPERTYLE) && !defined(PLUGINGALAXY)
                    if (x_int1 >= fmc.X) x_int1 = (fmc.X - 1);
                    if (y_int1 >= fmc.Y) y_int1 = (fmc.Y - 1);
                #else
                    switch (edgeMode)
                    {
						default:
                        case 0: 
                            if (x_int1 >= fmc.X) x_int1 = (fmc.X - 1);
                            if (y_int1 >= fmc.Y) y_int1 = (fmc.Y - 1);
                            break;
                        case 1: 
                            if (x_int1 >= fmc.X) x_int1 = (fmc.X - 1);
                            if (y_int1 >= fmc.Y) y_int1 = (fmc.Y - 1);
                            break;
                        case 2:
                            //if (x_int1 >= fmc.X) x_int1 = (x_int1  - fmc.X) % fmc.X;
                            //if (y_int1 >= fmc.Y) y_int1 = (y_int1  - fmc.Y) % fmc.Y;
							if (x_int1 >= fmc.X) x_int1 = x_int1 % (fmc.X-1);
							if (y_int1 >= fmc.Y) y_int1 = y_int1 % (fmc.Y-1);
                            break;
                        case 3:
                            //if (x_int1 >= fmc.X) x_int1 = (fmc.X - 1) - (x_int1 - (fmc.X - 1));
                            //if (y_int1 >= fmc.Y) y_int1 = (fmc.Y - 1)- (y_int1 - (fmc.Y - 1));
							if (x_int1 >= fmc.X) x_int1 = (fmc.X - 1) -x_int1%(fmc.X-1);
							if (y_int1 >= fmc.Y) y_int1 = (fmc.Y - 1) -y_int1%(fmc.Y-1);
							break;	//afh 01-may-2008
                    }//end switch
                #endif

                    //Info ("x_int: %d - y_ing: %d",x_int,y_int);

                    fracx=x-x_int;
                    fracy=y-y_int;

                    //Info ("fracx: %f - fracy: %f",fracx,fracy);
                    

                    if (mode==2) //Bicosine
                    {
                        if (fracx==fracy)
                        {
                            fracx= (1.0-cos(fracx*3.1415927))*0.5;
                            fracy=fracx;
                        }
                        else
                        {
                            fracx= (1.0-cos(fracx*3.1415927))*0.5;
                            fracy= (1.0-cos(fracy*3.1415927))*0.5;
                        }
                    }
                    else if (mode == 1) //Bisquare
                    {
                        fracx *= fracx;
                        fracy *= fracy;
                    }
                
                }//Calculate not cached

                
                switch (buffer)
                {
					default:
                    case 0: //input buffer
                        if (gStuff->imageMode < 10){
							v1 = ((unsigned8 *)MyAdjustedInData)
                                [(x_int-fmc.x_start)*fmc.srcColDelta + (y_int-fmc.y_start)*fmc.srcRowDelta + z];
                            v2 = ((unsigned8 *)MyAdjustedInData)
                                [(x_int-fmc.x_start)*fmc.srcColDelta + (y_int1-fmc.y_start)*fmc.srcRowDelta + z];
                            v3 = ((unsigned8 *)MyAdjustedInData)
                                [(x_int1-fmc.x_start)*fmc.srcColDelta + (y_int-fmc.y_start)*fmc.srcRowDelta + z];
                            v4 = ((unsigned8 *)MyAdjustedInData)
                                [(x_int1-fmc.x_start)*fmc.srcColDelta + (y_int1-fmc.y_start)*fmc.srcRowDelta + z];
							/*int xval = (x_int-fmc.x_start)*fmc.srcColDelta ;
							int xval1 = (x_int1-fmc.x_start)*fmc.srcColDelta ;
							int yval = (y_int-fmc.y_start)*fmc.srcRowDelta + z;
							int yval1 = (y_int1-fmc.y_start)*fmc.srcRowDelta + z;
							v1 = ((unsigned8 *)MyAdjustedInData)[xval + yval];
                            v2 = ((unsigned8 *)MyAdjustedInData)[xval + yval1];
                            v3 = ((unsigned8 *)MyAdjustedInData)[xval1 + yval];
                            v4 = ((unsigned8 *)MyAdjustedInData)[xval1 + yval1];*/
                        } else {
                            v1 = *(unsigned16 *)(((unsigned8 *)MyAdjustedInData) +
                               (((x_int-fmc.x_start)*fmc.srcColDelta + z) << 1) +
                               (y_int-fmc.y_start)*fmc.srcRowDelta);
                            v2 = *(unsigned16 *)(((unsigned8 *)MyAdjustedInData) +
                               (((x_int-fmc.x_start)*fmc.srcColDelta + z) << 1) +
                               (y_int1-fmc.y_start)*fmc.srcRowDelta);
                            v3 = *(unsigned16 *)(((unsigned8 *)MyAdjustedInData) +
                               (((x_int1-fmc.x_start)*fmc.srcColDelta + z) << 1) +
                               (y_int-fmc.y_start)*fmc.srcRowDelta);
                            v4 = *(unsigned16 *)(((unsigned8 *)MyAdjustedInData) +
                               (((x_int1-fmc.x_start)*fmc.srcColDelta + z) << 1) +
                               (y_int1-fmc.y_start)*fmc.srcRowDelta);
                        }
                        break;

                    case 1: //temp buffer 1
						if (gStuff->imageMode < 10){
							v1 = fmc.tbuf[(y_int*fmc.X + x_int)*fmc.Z + z];
							v2 = fmc.tbuf[(y_int1*fmc.X + x_int)*fmc.Z + z];
							v3 = fmc.tbuf[(y_int*fmc.X + x_int1)*fmc.Z + z];
							v4 = fmc.tbuf[(y_int1*fmc.X + x_int1)*fmc.Z + z]; 
						} else {
							v1 = *(unsigned16 *)(((unsigned8 *)fmc.tbuf) + 2*((y_int*fmc.X + x_int)*fmc.Z + z));
							v2 = *(unsigned16 *)(((unsigned8 *)fmc.tbuf) + 2*((y_int1*fmc.X + x_int)*fmc.Z + z));
							v3 = *(unsigned16 *)(((unsigned8 *)fmc.tbuf) + 2*((y_int*fmc.X + x_int1)*fmc.Z + z));
							v4 = *(unsigned16 *)(((unsigned8 *)fmc.tbuf) + 2*((y_int1*fmc.X + x_int1)*fmc.Z + z));
						}
                        break;

                    case 2: //temp buffer 2
						if (gStuff->imageMode < 10){
							v1 = fmc.t2buf[(y_int*fmc.X + x_int)*fmc.Z + z];
							v2 = fmc.t2buf[(y_int1*fmc.X + x_int)*fmc.Z + z];