				}
                break;

			case 36: //Hard Mood -> GIMP's Grain Merge (Similar to Overlay, but harder)
				for (i=0; i<planes; i++){
					calc[i] = a[i] + b[i] - mode_midVal;
					if (calc[i] < 0) calc[i] = 0; else if (calc[i] > mode_maxVal) calc[i] = mode_maxVal;
				}
				break;

            case 37://Add
				for (i=0; i<planes; i++)
					calc[i] = a[i]+b[i];
                break;

			case 38://Negative Difference 1
				for (i=0; i<planes; i++)
					calc[i] = mode_maxVal-dif(a[i],mode_maxVal-b[i]);
                break;

            case 39://Negative Difference 2
				for (i=0; i<planes; i++)
					calc[i] = dif(mode_maxVal-a[i],b[i]);
                break;

            case 40: //Threshold
				for (i=0; i<planes; i++)
					calc[i] = a[0] < (255-ratio)*mode_bitMultiply ? a[i] : b[i];
				break;

            case 41: //Threshold 2
				for (i=0; i<planes; i++)
                	calc[i] = b[0] < (255-ratio)*mode_bitMultiply ? a[i] : b[i];
				break;
			
			/*case 41: //Reflect -> Similar to Hard Mix, but softer
				for (i=0; i<planes; i++){
					if (b[i] == mode_maxVal)
					  calc[i] = mode_maxVal;
					else {
					  calc[i] = a[i]*a[i] /(mode_maxVal-b[i]);
					  if (calc[i] > mode_maxVal) calc[i] = mode_maxVal;
					}
				}
				break;*/
			/*case 41: //Phoenix -> colorful
				for (i=0; i<planes; i++)
					calc[i] = min(a[i],b[i]) - max(a[i],b[i]) + mode_maxVal;
				break;*/
			/*case 41: //Stamp -> Similar to Hard Light, but stronger
				for (i=0; i<planes; i++){
					calc[i] = a[i] + 2*b[i] - (mode_maxVal+1);
					if (calc[i] < 0) calc[i] = 0; else if (calc[i] > mode_maxVal) calc[i] = mode_maxVal;
				}
				break;*/
			
        }//end switch


		//Opacity
		if (ratio<255){
			for (i=0; i<planes; i++)
				calc[i] = ((255-ratio)*a[i] + ratio*calc[i])/255;
		}
	}
	
	return true;

}

#undef dif
#undef rnd
#undef sqr



//function added by Harald Heim on Feb 5, 2002
int contrast(int a, int b) 
    //a=Color Value, b=Contrast Value from -127 to 128
{
    if (b < -127) b=-127;	//afh 01-may-2008
    else if (b > 128) b=128;	//afh 01-may-2008

    return scl(a,b,257-b,0,255);
}

//function added by Harald Heim on Feb 5, 2002
int posterize(int a, int b) 
    //a=Color Value, b=Posterize Value/Number of Colors from 0 to 255
{ 
    if (b < 0) b=0;
    else if (b > 255) b=255;

    //Bad formula
    /*
    if (b==0)
        return 0;
    if (b==255)
        return a;
    else
        return scl( scl(a,0,255,0,b) ,0,b,0,255 );
    */

    if (b==0)
        return a;
    else
        return a - (a % b);
}


//function added by Harald Heim on Feb 5, 2002
int saturation(int r, int g, int b, int z, int sat) 
    // r,g,b = color values, z = returned channel, sat = saturation value between -500 and 500
{
    int gray,sat2,channel;

    //Which channel to return?
    if (z==0) channel=r;
    else if (z==1) channel=g;
    else if (z==2) channel=b;

    if (sat==0)
    {
        return channel;
    }
    else
    {
        //test limits
        if (sat < -500) b=-500;
        else if (sat > 500) b=500;

        //Gray value
        //gray= (r+b+g)/3;
        gray =( max(max(r,g),b) + min(min(r,g),b) ) /2;

        if (sat==-500)
        {
            return gray;
        }
        else
        {           
            //Saturate or desaturate?
            if (sat < 0) sat2=(sat/5)+100;
            else sat2=sat+100;

            return ( (100-sat2)*gray + sat2*channel )/100;
        }

    }

}

//function added by Harald Heim on Feb 6, 2002
int msk(int x, int y)
{
        if (x < 0 || x >= fmc.X || y < 0 || y >= fmc.Y)
        {   // pin the coordinates (return 0 if outside, NOT replicate edges!!!)
            return 0;
        }
        else if (gStuff->haveMask)
        { // get value from selection mask

            //if (gStuff->imageMode > 9
            //) {
            //    return *(unsigned16 *)(((unsigned8 *)MyAdjustedMaskData) +
            //                   (((x-fmc.x_start)*fmc.mskColDelta)) +  // <<1
            //                   (y-fmc.y_start)*fmc.mskRowDelta);    
            //} else {    
           
                return ((unsigned8 *)MyAdjustedMaskData)
                    [(x-fmc.x_start)*fmc.mskColDelta + (y-fmc.y_start)*fmc.mskRowDelta];    
            //}
        }
        /*else if (!gNoMask)
        {   // get value from alpha channel
            return ((unsigned8 *)MyAdjustedInData)
                [(x-fmc.x_start)*fmc.srcColDelta + (y-fmc.y_start)*fmc.srcRowDelta + 3];
        }*/
        else
        {
            //if (gStuff->imageMode > 9
            //  ) {
            //    return 65535; //32768;
            //}else {
                return 255;
            //}

        }
}


/**
//function added by Harald Heim on Feb 6, 2002
int egw(int b, int t, int v)
{
    int swapvalue;

    if (t==0 && b==0)
        return 0;
    if (b==t)
        return b;

    //added by Harald Heim on Mar 8, 2002
    b=abs(b);	//afh 01-may-2008
    t=abs(t);	//afh 01-may-2008
    if (b>t)
    {
        swapvalue = t;
        t = b;
        b = swapvalue;
    }

    if (v > t)
        return b + (v-b) % (t-b) ;
    else if (v < b)
        return t - (b - v) % (t-b);
    else
        return v;

}/**/

//------
// routines
//------
// int egw(int edge_a, int edge_b, int value)
//		Arguments
//			edge_a
//				edge_a edge value
//			edge_b
//				edge_b edge value
//			value
//				value that will be edge wrapped.
//		Return
//			The edge-wrapped value will be returned.
//		Description
//			This function returns the value untouched if it lies between edge_a and edge_b.
//			If it lies outside edge_a and edge_b, the value will be edge wrapped to lie between
//			edge_a and edge_b as if the image is continuous.
//		Examples:
//			Input value of 21 will be returned as 11 if edge_a = 10 and edge_b = 20
//			Input value of 21 will be returned as 12 if edge_a = 10 and edge_b = 19
//			Input value of  8 will be returned as 18 if edge_a = 10 and edge_b = 20
//			Input value of  5 will be returned as 14 if edge_a = 10 and edge_b = 19.
//------
int egw(int edge_a, int edge_b, int value)
{
	int swapvalue;

	//RC    if (edge_b == 0 && edge_a == 0)  return 0;   redundant with next statement??
	if (edge_a == edge_b) return edge_a;

	if (edge_a < 0) edge_a = -edge_a;
	if (edge_b < 0) edge_b = -edge_b;
	if (edge_a > edge_b)
	{
		swapvalue 	= edge_b;
		edge_b 		= edge_a;
		edge_a 		= swapvalue;
	}

	if (value > edge_b)
		return edge_a + (value - edge_a) % (edge_b - edge_a) ;
	else if (value < edge_a)
		return edge_b - (edge_a - value) % (edge_b - edge_a);
	else
		return value;
}


//------
// int egm(int edge_a, int edge_b, int value)
//
//		Arguments
//			edge_a
//				edge_a edge value
//			edge_b
//				edge_b edge value
//			value
//				value that will be edge mirrored.
//		Return
//			edge-mirrored value will be returned
//			-1 if after mirroring one time the computed value is outside the range.
//		Description
//			This function returns the value untouched if it lies between edge_a and edge_b.
//			If it lies outside edge_a and edge_b, the value will be edge mirrored to lie between
//			edge_a and edge_b as if the image is reflected back at each edge.
//		Examples:
//			Input value of 21 will be returned as 19 if edge_a = 10 and edge_b = 20
//			Input value of 21 will be returned as 18 if edge_a = 10 and edge_b = 19.
//			Input value of  8 will be returned as 12 if edge_a = 10 and edge_b = 20
//			Input value of  5 will be returned as 15 if edge_a = 10 and edge_b = 19.
//------
int egm(int edge_a, int edge_b, int value)
{
	int swapvalue;

	if (edge_a == edge_b) return edge_a;

	if (edge_a < 0) edge_a = -edge_a;
	if (edge_b < 0) edge_b = -edge_b;