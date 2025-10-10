                if (y < 0) y = 0; else if (y >= fmc.Y) y = fmc.Y - 1;
                break;

            case 1:
                if (x < 0 || y < 0 || x >= fmc.X || y >= fmc.Y)
                    return 0;
        
           case 2:
                //if (x < 0) x = (fmc.X - 1) + x;				/// or use MOD ???
                //else if (x >= fmc.X) x = (x -(fmc.X-1)) % fmc.X;
                //if (y < 0) y = (fmc.Y - 1) + y;
                //else if (y >= fmc.Y) y = (y -(fmc.Y-1)) % fmc.Y;
				if (x < 0) x = (fmc.X-1) + x%(fmc.X-1);
                else if (x >= fmc.X) x = x % (fmc.X-1);
                if (y < 0) y = (fmc.Y-1) + y%(fmc.Y-1);
                else if (y >= fmc.Y) y = y % (fmc.Y-1);
                break;
            
            case 3: //two reflection modes, one of period 2N, other of period 2N-2 (as here)?
                //if (x < 0) x = - x;
                //else if (x >= fmc.X) x = (fmc.X - 1) - (x - (fmc.X - 1));       //optimize?
                //if (y < 0) y = - y;
                //else if (y >= fmc.Y) y = (fmc.Y - 1) - (y - (fmc.Y - 1));
				if (x < 0) x = - x%(fmc.X-1);
                else if (x >= fmc.X) x = (fmc.X - 1) -x%(fmc.X-1);
				if (y < 0) y = - y%(fmc.Y-1);
                else if (y >= fmc.Y) y = (fmc.Y - 1) -y%(fmc.Y-1);
                break;

        }//end switch
    #endif

        return *(unsigned16 *)(((unsigned8 *)MyAdjustedInData) +
                               (((x-fmc.x_start)*fmc.srcColDelta + z) << 1) +
                               (y-fmc.y_start)*fmc.srcRowDelta);
    }


    // Must be upated for Photoshop CS where we have transparency in 16bit mode!!!

    else if (z == 3 || z < fmc.planes) //==3 //gStuff->planes
    {   // get a fictitious alpha channel pixel
        // alpha outside image/selection is assumed 0...
        //if (x < 0 || x >= fmc.X || y < 0 || y >= fmc.Y)
        //{   // pin the coordinates (return 0 if outside, NOT replicate edges!!!)
        //    return 0;
        //}
        //else {//still needs to be tested
        //#ifndef HYPERTYLE
		#if !defined(HYPERTYLE) && !defined(PLUGINGALAXY)
            if (x < 0) x = 0; else if (x >= fmc.X) x = fmc.X - 1;
		    if (y < 0) y = 0; else if (y >= fmc.Y) y = fmc.Y - 1;
        #else
            switch (edgeMode)
			{
				case 0:
					if (x < 0) x = 0; else if (x >= fmc.X) x = fmc.X - 1;
					if (y < 0) y = 0; else if (y >= fmc.Y) y = fmc.Y - 1;
					break;

				case 1:
					if (x < 0 || y < 0 || x >= fmc.X || y >= fmc.Y)
						return 0;
        
				case 2:
					//if (x < 0) x = (fmc.X - 1) + x;				/// or use MOD ???
					//else if (x >= fmc.X) x = (x -(fmc.X-1)) % fmc.X;
					//if (y < 0) y = (fmc.Y - 1) + y;
					//else if (y >= fmc.Y) y = (y -(fmc.Y-1)) % fmc.Y;
					if (x < 0) x = (fmc.X-1) + x%(fmc.X-1);
					else if (x >= fmc.X) x = x % (fmc.X-1);
					if (y < 0) y = (fmc.Y-1) + y%(fmc.Y-1);
					else if (y >= fmc.Y) y = y % (fmc.Y-1);
					break;
	            
				case 3: //two reflection modes, one of period 2N, other of period 2N-2 (as here)?
					//if (x < 0) x = - x;
					//else if (x >= fmc.X) x = (fmc.X - 1) - (x - (fmc.X - 1));       //optimize?
					//if (y < 0) y = - y;
					//else if (y >= fmc.Y) y = (fmc.Y - 1) - (y - (fmc.Y - 1));
					if (x < 0) x = - x%(fmc.X-1);
					else if (x >= fmc.X) x = (fmc.X - 1) -x%(fmc.X-1);
					if (y < 0) y = - y%(fmc.Y-1);
					else if (y >= fmc.Y) y = (fmc.Y - 1) -y%(fmc.Y-1);
					break;

			}//end switch
        #endif

			//Causes problems when reading the alpha of greyscale images, so z==3 was added
			if (gNoMask && z==3)
			{   // entire filter rect is presumed selected
				return 32768;
			}
			
			return *(unsigned16 *)(((unsigned8 *)MyAdjustedInData) +
                               (((x-fmc.x_start)*fmc.srcColDelta + z) << 1) + //3 //+gStuff->planes
                               (y-fmc.y_start)*fmc.srcRowDelta);
            //return  65535; //0x8000;  // assume max opacity
        //}
    }
    else
    {   // invalid channel number
        return 0;
    }
} /*fm_src16*/



int fast_src (int x, int y, int z){

    if (x < 0) x = 0; else if (x >= fmc.X) x = fmc.X - 1;
    if (y < 0) y = 0; else if (y >= fmc.Y) y = fmc.Y - 1;
    
    if (fmc.imageMode<10)
        return ((unsigned8 *)MyAdjustedInData) [(x-fmc.x_start)*fmc.srcColDelta + (y-fmc.y_start)*fmc.srcRowDelta + z];
    else
        return *(unsigned16 *)(((unsigned8 *)MyAdjustedInData) + (((x-fmc.x_start)*fmc.srcColDelta + z) << 1) + (y-fmc.y_start)*fmc.srcRowDelta);
}


int fast_src3 (int x, int y, int z){

    if (x < 0) x = - x%(fmc.X-1);
	else if (x >= fmc.X) x = (fmc.X - 1) - x%(fmc.X-1);
	if (y < 0) y = - y%(fmc.Y-1);
    else if (y >= fmc.Y) y = (fmc.Y - 1) - y%(fmc.Y-1);
    
    if (fmc.imageMode<10)
        return ((unsigned8 *)MyAdjustedInData) [(x-fmc.x_start)*fmc.srcColDelta + (y-fmc.y_start)*fmc.srcRowDelta + z];
    else
        return *(unsigned16 *)(((unsigned8 *)MyAdjustedInData) + (((x-fmc.x_start)*fmc.srcColDelta + z) << 1) + (y-fmc.y_start)*fmc.srcRowDelta);
}

int fast_pget3 (int x, int y, int z){

    if (x < 0) x = - x%(fmc.X-1);
	else if (x >= fmc.X) x = (fmc.X - 1) - x%(fmc.X-1);
	if (y < 0) y = - y%(fmc.Y-1);
    else if (y >= fmc.Y) y = (fmc.Y - 1) - y%(fmc.Y-1);

	if (fmc.imageMode<10)
		return ((unsigned8 *)gStuff->outData)
			[(x-fmc.x_start)*fmc.dstColDelta + (y-fmc.y_start)*fmc.dstRowDelta + z];
	else
		return *(unsigned16 *)  ( ((unsigned8 *)gStuff->outData) +
			(((x-fmc.x_start)*fmc.dstColDelta + z)<<1 ) + (y-fmc.y_start)*fmc.dstRowDelta  );
}

int fast_t3get3 (int x, int y, int z){

    if (x < 0) x = - x%(fmc.X-1);
	else if (x >= fmc.X) x = (fmc.X - 1) - x%(fmc.X-1);
	if (y < 0) y = - y%(fmc.Y-1);
    else if (y >= fmc.Y) y = (fmc.Y - 1) - y%(fmc.Y-1);

	if (fmc.imageMode<10)
		return fmc.t3buf[(y*fmc.X + x)*fmc.Z + z];
	else
		return *(unsigned16 *)(((unsigned8 *)fmc.t3buf) + 2*((y*fmc.X + x)*fmc.Z + z));
}

int fast_t4get3 (int x, int y, int z){

    if (x < 0) x = - x%(fmc.X-1);
	else if (x >= fmc.X) x = (fmc.X - 1) - x%(fmc.X-1);
	if (y < 0) y = - y%(fmc.Y-1);
    else if (y >= fmc.Y) y = (fmc.Y - 1) - y%(fmc.Y-1);

	if (fmc.imageMode<10)
		return fmc.t4buf[(y*fmc.X + x)*fmc.Z + z];
	else
		return *(unsigned16 *)(((unsigned8 *)fmc.t4buf) + 2*((y*fmc.X + x)*fmc.Z + z));
}

int fast_frad3(double d, double m, int z)
{
    return fast_src3((int)fm_fr2x(d,m) + (fmc.X/2),
                   (int)fm_fr2y(d,m) + (fmc.Y/2),
                   z);
}
int fast_fpgetr3(double d, double m, int z)
{
    return fast_pget3((int)fm_fr2x(d,m) + (fmc.X/2),
                   (int)fm_fr2y(d,m) + (fmc.Y/2),
                   z);
}
int fast_ft3getr3(double d, double m, int z)
{
    return fast_t3get3((int)fm_fr2x(d,m) + (fmc.X/2),
                 (int)fm_fr2y(d,m) + (fmc.Y/2),
                    z);
}
int fast_ft4getr3(double d, double m, int z)
{
    return fast_t4get3((int)fm_fr2x(d,m) + (fmc.X/2),
                 (int)fm_fr2y(d,m) + (fmc.Y/2),
                    z);
}





int r2x(int d, int m)
{
    return (int)((double)m * cos( (twopi/1024.0)*(double)d ) + 0.0); // trunc or round???
}

int r2y(int d, int m)
{
    return (int)((double)m * sin( (twopi/1024.0)*(double)d ) + 0.0); // trunc or round???
}

double fm_fr2x(double d, double m)
{
    return m * cos( (twopi/1024.0) * d );
}
double fm_fr2y(double d, double m)
{
	return m * sin( (twopi/1024.0) * d );
}


int rad(int d, int m, int z)
{
    return fmc.src(r2x(d,m) + (fmc.X/2),
                   r2y(d,m) + (fmc.Y/2),
                   z);
}

int pgetr(int d, int m, int z)
{
    return fmc.pget(r2x(d,m) + (fmc.X/2),
                   r2y(d,m) + (fmc.Y/2),
                   z);
}

int frad(double d, double m, int z)
{
    return fmc.src((int)fm_fr2x(d,m) + (fmc.X/2),
                   (int)fm_fr2y(d,m) + (fmc.Y/2),
                   z);
}
int fpgetr(double d, double m, int z)
{
    return fmc.pget((int)fm_fr2x(d,m) + (fmc.X/2),
                   (int)fm_fr2y(d,m) + (fmc.Y/2),
                   z);
}

int psetr(int d, int m, int z, int val)
{
    return fmc.pset(r2x(d,m) + (fmc.X/2),
                   r2y(d,m) + (fmc.Y/2),
                   z, val);
}

int tgetr(int d, int m, int z)
{
    return fmc.tget(r2x(d,m) + (fmc.X/2),
                   r2y(d,m) + (fmc.Y/2),
                   z);
}

int tsetr(int d, int m, int z, int val)
{
    return fmc.tset(r2x(d,m) + (fmc.X/2),
                   r2y(d,m) + (fmc.Y/2),
                   z, val);
}

int t2getr(int d, int m, int z)
{
    return fmc.t2get(r2x(d,m) + (fmc.X/2),
                    r2y(d,m) + (fmc.Y/2),
                    z);
}

int t2setr(int d, int m, int z, int val)
{
    return fmc.t2set(r2x(d,m) + (fmc.X/2),
                    r2y(d,m) + (fmc.Y/2),
                    z, val);
}

int t3getr(int d, int m, int z)
{
    return fmc.t3get(r2x(d,m) + (fmc.X/2),
                    r2y(d,m) + (fmc.Y/2),
                    z);
}

int t3setr(int d, int m, int z, int val)
{
    return fmc.t3set(r2x(d,m) + (fmc.X/2),
                    r2y(d,m) + (fmc.Y/2),
                    z, val);
}
