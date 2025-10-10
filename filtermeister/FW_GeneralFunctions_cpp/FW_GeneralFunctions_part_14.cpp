
}

/*
//function added by Harald Heim on Feb 10, 2002
int tbuffer_initialize(int i)
{
    if (i==0) 
        Dont_Initialize_Tbuffer = 0; //Tbuffer isn't initialized
    else
        Dont_Initialize_Tbuffer = 1; //Tbuffer is initialized
    
    return true;

}

//function added by Harald Heim on Feb 10, 2002
int t2buffer_initialize(int i)
{
    if (i==0) 
        Dont_Initialize_T2buffer = 0; //T2buffer isn't initialized
    else
        Dont_Initialize_T2buffer = 1; //T2buffer is initialized
    
    return true;

}
*/

//// Use an enum for edge mode, expose to FD???

//function added by Harald Heim on Feb 13, 2002
int set_edge_mode(int mode)
{ 
//mode=0 for regular behaviour
//mode=1 for black border
//mode=2 for edge wrap
//mode=3 for edge mirror
    if (mode < 0 && mode > 3) return false;

    edgeMode = mode;
    return true;
}


int set_bitdepth_mode(int mode)
{ 
    bitDepthMode = mode;

	if (bitDepthMode==16){ 
		mode_maxVal=32768;
		mode_midVal=16384;
		mode_quarVal=8192;
		mode_bitMultiply=128;
	} else {
		mode_maxVal=255;
		mode_midVal=128;
		mode_quarVal=64;
		mode_bitMultiply=1;
	}

    return true;
}

int get_bitdepth_mode()
{ 
    return bitDepthMode;
}

/// Use pluggable variants of psetp() instead of testing a flag at run-time!!!

int set_psetp_mode(int mode)
{ 
//mode=0 for regular behaviour (not possible to set alpha channel to zero) for more performance and avoiding bugs
//mode=1 for always setting the alpha channel value

    if (mode < 0 && mode > 1) return false;

    psetpMode = mode;
    return true;
}




//function added by Harald Heim on Feb 11, 2002
int igetArrayEx(int nr, double x, double y, int z, int mode)
{ 
    int x_int, y_int, x_int1, y_int1;
    double fracx, fracy;
    int pos;

	if (z < 0 || z > ArrayZ[nr]) return 0;
        
    if (mode==0) //Nearest Neighbor
    {
        
        x_int=(int) x;
        y_int=(int) y;

        if (x_int < 0) x_int = 0; else if (x_int >= ArrayX[nr]) x_int = ArrayX[nr] - 1;
        if (y_int < 0) y_int = 0; else if (y_int >= ArrayY[nr]) y_int = ArrayY[nr] - 1;

		pos = (( (y_int+ArrayPadding[nr])*ArrayX[nr] + (x_int+ArrayPadding[nr]) )*ArrayZ[nr] + z);//*ArrayBytes[nr];
		switch(ArrayBytes[nr]) {
			case 1:
				return ((unsigned8 *)Array[nr]) [pos];
			case 2:
				return  ((unsigned16 *)Array[nr]) [pos];
			case 4:
				return  ((unsigned32 *)Array[nr]) [pos];
		}

    } else {
        
        if (mode<4){

			int v1,v2,v3,v4;
			//Cached Values
			static int x_int_old, y_int_old, x_int1_old, y_int1_old,mode_old, edgeMode_old;
			static double fracx_old, fracy_old, x_old, y_old;

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
            {

                if (x < 0) x = 0; else if (x >= ArrayX[nr]) x = ArrayX[nr] - 1;
				if (y < 0) y = 0; else if (y >= ArrayY[nr]) y = ArrayY[nr] - 1;

                x_int=(int) x;
                y_int=(int) y;

                x_int1=x_int+1;
                y_int1=y_int+1;

                if (x_int1 >= ArrayX[nr]) x_int1 = (ArrayX[nr] - 1);
                if (y_int1 >= ArrayY[nr]) y_int1 = (ArrayY[nr]- 1);

                fracx=x-x_int;
                fracy=y-y_int;


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

			switch(ArrayBytes[nr]) {
				case 1:
					pos = (( (y_int+ArrayPadding[nr])*ArrayX[nr] + (x_int+ArrayPadding[nr]) )*ArrayZ[nr] + z);
					v1 = ((unsigned8 *)Array[nr]) [pos];
					pos = (( (y_int1+ArrayPadding[nr])*ArrayX[nr] + (x_int+ArrayPadding[nr]) )*ArrayZ[nr] + z);
					v2 = ((unsigned8 *)Array[nr]) [pos];
					pos = (( (y_int+ArrayPadding[nr])*ArrayX[nr] + (x_int1+ArrayPadding[nr]) )*ArrayZ[nr] + z);
					v3 = ((unsigned8 *)Array[nr]) [pos];
					pos = (( (y_int1+ArrayPadding[nr])*ArrayX[nr] + (x_int1+ArrayPadding[nr]) )*ArrayZ[nr] + z);
					v4 = ((unsigned8 *)Array[nr]) [pos];
					break;
				case 2:
					pos = (( (y_int+ArrayPadding[nr])*ArrayX[nr] + (x_int+ArrayPadding[nr]) )*ArrayZ[nr] + z);
					v1 = ((unsigned16 *)Array[nr]) [pos];
					pos = (( (y_int1+ArrayPadding[nr])*ArrayX[nr] + (x_int+ArrayPadding[nr]) )*ArrayZ[nr] + z);
					v2 = ((unsigned16 *)Array[nr]) [pos];
					pos = (( (y_int+ArrayPadding[nr])*ArrayX[nr] + (x_int1+ArrayPadding[nr]) )*ArrayZ[nr] + z);
					v3 = ((unsigned16 *)Array[nr]) [pos];
					pos = (( (y_int1+ArrayPadding[nr])*ArrayX[nr] + (x_int1+ArrayPadding[nr]) )*ArrayZ[nr] + z);
					v4 = ((unsigned16 *)Array[nr]) [pos];
					break;
				case 4:
					pos = (( (y_int+ArrayPadding[nr])*ArrayX[nr] + (x_int+ArrayPadding[nr]) )*ArrayZ[nr] + z);
					v1 = ((unsigned32 *)Array[nr]) [pos];
					pos = (( (y_int1+ArrayPadding[nr])*ArrayX[nr] + (x_int+ArrayPadding[nr]) )*ArrayZ[nr] + z);
					v2 = ((unsigned32 *)Array[nr]) [pos];
					pos = (( (y_int+ArrayPadding[nr])*ArrayX[nr] + (x_int1+ArrayPadding[nr]) )*ArrayZ[nr] + z);
					v3 = ((unsigned32 *)Array[nr]) [pos];
					pos = (( (y_int1+ArrayPadding[nr])*ArrayX[nr] + (x_int1+ArrayPadding[nr]) )*ArrayZ[nr] + z);
					v4 = ((unsigned32 *)Array[nr]) [pos];
					break;  
			}

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

            return (int) ((1.0-fracx)* ((1.0-fracy)*v1 + fracy*v2)  + fracx* ((1.0-fracy)*v3 + fracy*v4));
    

        } else if (mode==4) {//bicubic interpolation
        
            //For Bicubic Interpolation 
           double nx,ny,ox3,oy3,nx3,ny3;
           double ix[4];
           int j,k;
           int i[4];
           double x1,x2,x3,x4,y1,y2,y3,y4,y9;
        
           if (x < 0) x = 0; else if (x >= ArrayX[nr]) x = (double)(ArrayX[nr] - 1);
		   if (y < 0) y = 0; else if (y >= ArrayY[nr]) y = (double)(ArrayY[nr] - 1);

           y_int = (int)y;
           x_int = (int)x;

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

               if (y_int1 < 0) y_int1 = 0;
               if (y_int1 >= ArrayY[nr]) y_int1 = (ArrayY[nr] - 1);
            
               for (k=-1;k<3;k++) {
                    
                    x_int1 = x_int + k;

                    if (x_int1 < 0) x_int1 = 0;
                    if (x_int1 >= ArrayX[nr]) x_int1 = (ArrayX[nr] - 1);
               
					pos = (( (y_int1+ArrayPadding[nr])*ArrayX[nr] + (x_int1+ArrayPadding[nr]) )*ArrayZ[nr] + z);
					switch(ArrayBytes[nr]) {
						case 1: i[k+1] = ((unsigned8 *)Array[nr]) [pos]; break;
						case 2: i[k+1] = ((unsigned16 *)Array[nr]) [pos]; break;
						case 4: i[k+1] = ((unsigned32 *)Array[nr]) [pos];
					}

                }//for k

               
               x1 = (i[2] - i[1]) - (i[1] - i[0]);
               x2 = (i[3] - i[2]) - (i[2] - i[1]);
               x3 = x1 - x2 / 4.;
               x4 = x2 - x1 / 4.;
               ix[j+1] = (fracx * i[2]) + (nx * i[1])  + (((x4 * ox3) + (x3 * nx3)) / 3.75);
           }//for j


           // Y
           y1 = (ix[2] - ix[1]) - (ix[1] - ix[0]);
           y2 = (ix[3] - ix[2]) - (ix[2] - ix[1]);
           y3 = y1 - y2 / 4.;
           y4 = y2 - y1 / 4.;
           y9 = (fracy * ix[2]) + (ny * ix[1])  + (((y4 * oy3) + (y3 * ny3)) / 3.75);

           return (int)y9;

        }//end bicubic

    }// end interpolation methods

    return 0;

} /*fm_iget*/


int iget(double x, double y, int z, int buffer, int mode)
{ 
    int x_int, y_int, x_int1, y_int1;