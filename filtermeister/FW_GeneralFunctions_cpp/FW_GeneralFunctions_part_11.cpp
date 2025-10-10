	}

}

//Used by Plugin Galaxy

#include <math.h> 

#define rnd(a,b) (rand()%(b-a)+a)
#define dif(a,b) (((a)-(b)>=0)?(a)-(b):(b)-(a))
#define sqr(x) ( x > 0 ? (int)(sqrt((double)x) + 0.0) : 0 )

void clipColor(int * red, int *green, int * blue){
	int l = (int)(0.3 * *red + 0.59 * *green + 0.11 * *blue);
	int n = min(*red,min(*green,*blue));
	int x = max(*red,max(*green,*blue));
	if (n < 0){
		*red = l + (((*red-l)*l)/(l-n));
		*green = l + (((*green-l)*l)/(l-n));
		*blue = l + (((*blue-l)*l)/(l-n));
	}
	if (x > mode_maxVal){
		*red = l + (((*red-l)*(mode_maxVal-l))/(x-l));
		*green = l + (((*green-l)*(mode_maxVal-l))/(x-l));
		*blue = l + (((*blue-l)*(mode_maxVal-l))/(x-l));
	}
}

void setLum(int * red, int *green, int * blue, int l){
	int d = l - (int)(0.3 * *red + 0.59 * *green + 0.11 * *blue);
	*red += d;
	*green += d;
	*blue += d;
	clipColor(red,green,blue);
}

#define PSWAP(x,y) {int* t;t=x;x=y;y=t;}

void setSat(int * red, int *green, int * blue, int s){
	int * cmax = red;
	int * cmid = green;
	int * cmin = blue;
	
	//Sort
	if (*cmax < *cmin) PSWAP(cmax,cmin)
	if (*cmin > *cmid) PSWAP(cmin,cmid)
	if (*cmax < *cmid) PSWAP(cmax,cmid)

	if (*cmax > *cmin){
		*cmid = (((*cmid - *cmin)*s)/(*cmax - *cmin));
		*cmax = s;
	} else 
		*cmid = *cmax = 0;
	*cmin = 0;
}

#undef PSWAP
//#define SWAP(x,y) x^=y;y^=x;x^=y

//a=Bottom Color Value, b=Top Color Value, m=Blend Mode, r = blending ratio from 0 (bottom) to 255 (top)
int blend2(int a[4], int b[4], int calc[4], int planes, int mode, int ratio)
{   
	int i, value;

	for (i=0; i<planes; i++){
		if (a[i] < 0) a[i]=0; else if (a[i] > mode_maxVal) a[i]=mode_maxVal;
		if (b[i] < 0) b[i]=0; else if (b[i] > mode_maxVal) b[i]=mode_maxVal;
	}
	
	if (ratio==0){
		
		return true; //No need to do anything

	} else {

        switch (mode){

            case 0://Normal
				for (i=0; i<planes; i++) 
					calc[i] = b[i];
				break;

            case 1: //Dissolve
                value = rand()%255;
                for (i=0; i<planes; i++) 
					calc[i] = ratio < value ? a[i] : b[i];
				break;


			case 3://Darken
                for (i=0; i<planes; i++) 
					calc[i] = min(a[i],b[i]);
                break;

            case 4: //Multiply
                for (i=0; i<planes; i++) 
					calc[i] = a[i]*b[i]/mode_maxVal;
                break;

			case 5: //Color Burn
				for (i=0; i<planes; i++) 
					calc[i] = b[i] <= 0? 0 : max(mode_maxVal - ((mode_maxVal - a[i]) * mode_maxVal / b[i]), 0);
				break;
			
			case 6: //Linear Burn
				for (i=0; i<planes; i++){
					calc[i] = a[i] + b[i] - mode_maxVal;
					if (calc[i] < 0) calc[i] = 0;
				}
				break;
			
			case 7: //Darker Color
				if (a[0]+a[1]+a[2] > b[0]+b[1]+b[2])
					for (i=0; i<planes; i++) 
						calc[i] = b[i];
				else
					for (i=0; i<planes; i++) 
						calc[i] = a[i];
				break;


            case 9: //Lighten
                for (i=0; i<planes; i++) 
					calc[i] = max(a[i],b[i]);
                break;

			case 10: //Screen
                for (i=0; i<planes; i++) 
					//calc[i] = mode_maxVal-((mode_maxVal-a[i])*(mode_maxVal-b[i]))/mode_maxVal;
					calc[i] = a[i] + b[i] - (a[i] * b[i])/mode_maxVal;
                break;

			case 11: //Color Dodge
				for (i=0; i<planes; i++) 
					calc[i] = b[i] == mode_maxVal? mode_maxVal : min(a[i] * mode_maxVal / (mode_maxVal - b[i]), mode_maxVal);
				break;

			case 12: //Linear Dodge (Add)
				for (i=0; i<planes; i++){
					calc[i] = a[i] + b[i];
					if (calc[i] > mode_maxVal) calc[i] = mode_maxVal;
				}
				break;

			case 13: //Lighter Color
				if (a[0]+a[1]+a[2] > b[0]+b[1]+b[2])
					for (i=0; i<planes; i++) 
						calc[i] = a[i];
				else
					for (i=0; i<planes; i++) 
						calc[i] = b[i];
				break;


            case 15: //Overlay
                for (i=0; i<planes; i++) 
					calc[i] = a[i]<mode_midVal ? (2*a[i]*b[i])/mode_maxVal : 
					                              mode_maxVal-(2*(mode_maxVal-a[i])*(mode_maxVal-b[i]))/mode_maxVal;
                break;

            case 16: //Soft Light
			{
				double value,cb,cs;
				for (i=0; i<planes; i++){ 
					//Almost identical to Photoshop's Soft Light
					//calc[i] = b[i] > mode_midVal ? mode_maxVal - (mode_maxVal-a[i]) * (mode_maxVal-(b[i]-mode_midVal)) / mode_maxVal :
					//					          ((2*b[i]-mode_maxVal) * (a[i]-a[i]*a[i]/mode_maxVal))/ mode_maxVal  + a[i];
					
					cb = (double)a[i] / mode_maxVal;
					cs = (double)b[i] / mode_maxVal;
					if (cs <= .5) {
						value = cb - (1-2*cs) * cb * (1-cb);
					} else {
						value = cb <= .25 ? ((16*cb-12)*cb+4)*cb : sqrt(cb);
						value = cb + (2*cs-1) * (value-cb);
					}
					calc[i] = (int)(value * mode_maxVal);
				}				
				break;
			}
            case 17: //Hard Light -> Very similar to overlay!
                for (i=0; i<planes; i++) 
					calc[i] = b[i]<mode_midVal ? (2*a[i]*b[i])/mode_maxVal : 
												  mode_maxVal-(2*(mode_maxVal-a[i])*(mode_maxVal-b[i]))/mode_maxVal;
												  //a[i] + (2*b[i]-mode_maxVal) - (a[i] * (2*b[i]-mode_maxVal))/mode_maxVal;
                break;

			case 18: //Vivid Light
				for (i=0; i<planes; i++){
					if (b[i] <= mode_midVal){
						if (b[i]==0) 
							calc[i] = 0;
						else 
							calc[i] = mode_maxVal - (mode_maxVal-a[i])*mode_maxVal / (2*b[i]);
					} else {
						if (b[i]==mode_maxVal)
							calc[i] = mode_maxVal;
						else
							calc[i] = a[i]*mode_maxVal / (2*(mode_maxVal-b[i]));
					}
					if (calc[i] < 0) calc[i] = 0;
					else if (calc[i] > mode_maxVal) calc[i] = mode_maxVal;
				}
				break;

			case 19: //Linear Light
				for (i=0; i<planes; i++){
					calc[i] = a[i] + 2*b[i] - mode_maxVal;
					if (calc[i] < 0) calc[i] = 0;
					else if (calc[i] > mode_maxVal) calc[i] = mode_maxVal;
				}
				break;

			case 20: //Pin Light
				for (i=0; i<planes; i++){
					calc[i] = a[i] < 2*b[i] - mode_maxVal ? 2*b[i] - mode_maxVal: a[i] > 2*b[i] ? 2*b[i] : a[i];
					if (calc[i] < 0) calc[i] = 0;
					else if (calc[i] > mode_maxVal) calc[i] = mode_maxVal;
				}
				break;

			case 21: //Hard Mix
				for (i=0; i<planes; i++) 
					calc[i] = a[i] < mode_maxVal-b[i] ? 0: mode_maxVal;
				break;

			
			case 23: //Difference
                for (i=0; i<planes; i++) 
					calc[i] = dif(a[i],b[i]);
                break;
		
            case 24: //Exclusion
                for (i=0; i<planes; i++) 
					calc[i] = mode_maxVal-(((mode_maxVal-a[i])*(mode_maxVal-b[i])/mode_maxVal)+(a[i]*b[i]/mode_maxVal));
                break;

			case 25: //Subtract
				for (i=0; i<planes; i++){
					calc[i] = a[i]-b[i];
					if (calc[i] < 0) calc[i] = 0;
				}
				break;
		
            case 26: //Divide
				for (i=0; i<planes; i++){
					calc[i] = b[i] == 0? 0: a[i] * mode_maxVal / b[i];
					if (calc[i] > mode_maxVal) calc[i] = mode_maxVal;
				}
                break;


			case 28: //Hue
				for (i=0; i<planes; i++)
					calc[i] = b[i];
				setSat(&calc[0], &calc[1], &calc[2], max(a[0],max(a[1],a[2])) - min(a[0],min(a[1],a[2])) );
				setLum(&calc[0], &calc[1], &calc[2], (int)(0.3*a[0]+0.59*a[1]+0.11*a[2]));				
				break;

			case 29: //Saturation
				for (i=0; i<planes; i++)
					calc[i] = a[i];
				value = (int)(0.3*a[0]+0.59*a[1]+0.11*a[2]);
				setSat(&calc[0], &calc[1], &calc[2], max(b[0],max(b[1],b[2])) - min(b[0],min(b[1],b[2])) );
				setLum(&calc[0], &calc[1], &calc[2], value);
				break;

			case 30: //Color
				for (i=0; i<planes; i++)
					calc[i] = b[i];
				setLum(&calc[0], &calc[1], &calc[2], (int)(0.3*a[0]+0.59*a[1]+0.11*a[2]));
				break;

			case 31: //Luminosity
				for (i=0; i<planes; i++)
					calc[i] = a[i];
				setLum(&calc[0], &calc[1], &calc[2], (int)(0.3*b[0]+0.59*b[1]+0.11*b[2]));
				break;

							
			case 33://Exposure
				for (i=0; i<planes; i++){
					calc[i] = a[i]+a[i]-b[i];
					if (calc[i] < 0) calc[i] = 0;
					else if (calc[i] > mode_maxVal) calc[i] = mode_maxVal;
				}
				break;

			case 34://Overexposure
				for (i=0; i<planes; i++){
					calc[i] = b[i]+b[i]-a[i];
					if (calc[i] < 0) calc[i] = 0;
					else if (calc[i] > mode_maxVal) calc[i] = mode_maxVal;
				}
				break;

			case 35: //Soft Mood -> previously Soft Light
                for (i=0; i<planes; i++){
					value = a[i]/2 + mode_quarVal;
					calc[i] = a[i]<mode_midVal ? (2*b[i]*value)/mode_maxVal : mode_maxVal-2*(mode_maxVal-value)*(mode_maxVal-b[i])/mode_maxVal;