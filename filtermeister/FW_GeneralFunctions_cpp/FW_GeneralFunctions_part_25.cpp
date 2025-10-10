

    } else { //Calculate
        
        Cr=r;
        Cg=g;
        Cb=b;

        //Scale rgb to float
        rf=(double) r / (255.0*mode_bitMultiply);
        gf=(double) g / (255.0*mode_bitMultiply);
        bf=(double) b / (255.0*mode_bitMultiply);


        //Convert RGB to HSL
        cmax = bf;
        cmin = bf;
        if (rf > cmax) cmax = rf;
        if (gf > cmax) cmax = gf;
        if (rf < cmin) cmin = rf;
        if (gf < cmin) cmin = gf;


        lf = (cmax + cmin)/2.0;

        if (cmax==cmin){
            sf = 0.0;
            hf = 0.0;
        } else {
              diff = cmax-cmin;
			  if (diff==0) {
                    sf = 0;
			  } else if (lf < 0.5) {
					sf = diff/(cmax+cmin); 
              } else {
					sf = diff/(2.0-cmax-cmin);
              }

              if (rf==cmax) {
					hf = (gf-bf)/diff;
              } else if (gf==cmax) {
					hf = 2.0 + (bf-rf)/diff;
              } else {
					hf = 4.0+(rf-gf)/diff;
              }
              hf = hf/6.0;
              if (hf < 0.0) { hf = hf+1.0;}
        }   

        
        //Scale hsl to int
        Ch=(int) (hf*255*mode_bitMultiply);
        Cs=(int) (sf*255*mode_bitMultiply);
        Cl=(int) (lf*255*mode_bitMultiply);


        if (z==0)
            return Ch;
        else if (z==1)
            return Cs;
        else
            return Cl;      

    }

}




//function added by Harald Heim on Dec 7, 2002
int hsl2rgb(int h, int s, int l, int z)
{

    //int r,g,b;
    double rf,gf,bf,hf,sf,lf, hf2, m1,m2;
    
    THREAD_LOCAL static int Cr,Cg,Cb;
    THREAD_LOCAL static int Ch,Cs,Cl;


    if (h==Ch && s==Cs && l==Cl){ //Read from Cache

        if (z==0)
            return Cr;
        else if (z==1)
            return Cg;
        else
            return Cb;    


    } else { //Calculate

        Ch=h;
        Cs=s;
        Cl=l;

        //Scale to float
        hf=(double) h / (255.0*mode_bitMultiply);
        sf=(double) s / (255.0*mode_bitMultiply);
        lf=(double) l / (255.0*mode_bitMultiply);
        

        //Convert HSL to RGB
        if (sf == 0.0) {
            rf = lf;
            gf = lf;
            bf = lf;
        } else {
                if (lf <= 0.5) {m2 = lf*(1.0+sf);} else {m2 = lf+sf-(lf*sf);}
                m1 = 2.0 * lf - m2;

            //Calculate Red
            hf2=hf+1.0/3.0;             
            if (hf2 < 0) {hf2 = hf2 + 1.0;}
            if (hf2 > 1) {hf2 = hf2 - 1.0;}
            if (6.0 * hf2 < 1){
                rf = (m1+(m2-m1)*hf2*6.0);}
            else{
                if (2.0 * hf2 < 1){
                   rf = m2;
                }
                else {
                   if (3.0*hf2 < 2.0) {rf = (m1+(m2-m1)*((2.0/3.0)-hf2)*6.0);} else {rf = m1;}
                } 
            }

            //Calculate Green               
            if (hf < 0) {hf = hf + 1.0;}
            if (hf > 1) {hf = hf - 1.0;}
            if (6.0 * hf < 1){
                gf = (m1+(m2-m1)*hf*6.0);}
            else {
                if (2.0 * hf < 1){
                   gf = m2;
                }
                else {
                   if (3.0*hf < 2.0) {gf = (m1+(m2-m1)*((2.0/3.0)-hf)*6.0);} else {gf = m1;}
                } 
            }

            //Calculate Blue
            hf2=hf-1.0/3.0;             
            if (hf2 < 0) {hf2 = hf2 + 1.0;}
            if (hf2 > 1) {hf2 = hf2 - 1.0;}
            if (6.0 * hf2 < 1) {
                bf = (m1+(m2-m1)*hf2*6.0);}
            else {
                if (2.0 * hf2 < 1){
                   bf = m2;
                }
                else {
                   if (3.0*hf2 < 2.0) {bf = (m1+(m2-m1)*((2.0/3.0)-hf2)*6.0);} else {bf = m1;}
                } 
            }

        }


        //Scale rgb to int
        Cr=(int) (rf*255*mode_bitMultiply);
        Cg=(int) (gf*255*mode_bitMultiply);
        Cb=(int) (bf*255*mode_bitMultiply);

        if (z==0)
            return Cr;
        else if (z==1)
            return Cg;
        else
            return Cb;

    }
}



void rgb2hsl_fast8(int * r, int * g, int * b)
{
    int h,s,l,cmax,cmin,diff;
    
	if (*r<0) *r=0; else if (*r>255) *r=255;
	if (*g<0) *g=0; else if (*g>255) *g=255;
	if (*b<0) *b=0; else if (*b>255) *b=255;

    cmax = *b;
    cmin = *b;
    if (*r > cmax) cmax = *r;
    if (*g > cmax) cmax = *g;
    if (*r < cmin) cmin = *r;
    if (*g < cmin) cmin = *g;

	l = (cmax + cmin)/2;
    if (cmax==cmin){
        s = 0;
        h = 0;
    } else {
		  diff = cmax-cmin;
		  if (diff==0) {
                s = 0;
		  } else if (l < mode_midVal) {
				s = diff*mode_maxVal/(cmax+cmin); 
          } else {
				s = diff*mode_maxVal/(2*mode_maxVal-cmax-cmin);
          }

          if (*r==cmax) {
				h = (*g-*b)*mode_maxVal/diff;
          } else if (*g==cmax) {
				h = 2*mode_maxVal + (*b-*r)*mode_maxVal/diff;
          } else {
				h = 4*mode_maxVal + (*r-*g)*mode_maxVal/diff;
          }
          h = h / 6;
          if (h < 0) h += mode_maxVal;
    }   

    *r =h;
    *g =s;
    *b =l;
}

void hsl2rgb_fast8(int * h, int * s, int * l)
{
    int r,g,b, h2,m1,m2;
	int h1 = *h;
	int s1 = *s;
	int l1 = *l;

    if (s == 0) {
        r = l1;
        g = l1;
        b = l1;
    } else {
        if (l1 <= mode_midVal) m2 = l1 * (mode_maxVal+s1) / mode_maxVal; 
		else m2 = (l1+s1) - (l1 * s1)/mode_maxVal;
        m1 = 2 * l1 - m2;

        //Calculate Red
        h2=h1+mode_maxVal/3;
        if (h2 < 0) h2 = h2 + mode_maxVal;
        if (h2 > mode_maxVal) h2 = h2 - mode_maxVal;
		if (6 * h2 < mode_maxVal) r = (m1 + (m2-m1)*h2*6 / mode_maxVal);
        else if (2 * h2 < mode_maxVal) r = m2;
        else if (3 * h2 < 2*mode_maxVal) r = (m1 + (m2-m1)*((2*mode_maxVal/3)-h2)*6 / mode_maxVal);
		else r = m1;
        
        //Calculate Green               
        if (h1 < 0) h1 = h1 + mode_maxVal;
        if (h1 > mode_maxVal) h1 = h1 - mode_maxVal;
        if (6 * h1 < mode_maxVal) g = (m1+(m2-m1)* h1 *6 / mode_maxVal);
        else if (2 * h1 < mode_maxVal) g = m2;
        else if (3 * h1 < 2*mode_maxVal) g = (m1+(m2-m1)*((2*mode_maxVal/3)-h1)*6 / mode_maxVal);
		else g = m1;
        
        //Calculate Blue
        h2=h1-mode_maxVal/3;
        if (h2 < 0) h2 = h2 + mode_maxVal;
        if (h2 > mode_maxVal) h2 = h2 - mode_maxVal;
        if (6 * h2 < mode_maxVal) b = (m1+(m2-m1)*h2*6 / mode_maxVal);
        else if (2 * h2 < mode_maxVal) b = m2;
        else if (3 * h2 < 2*mode_maxVal) b = (m1+(m2-m1)*((2*mode_maxVal/3)-h2)*6 / mode_maxVal);
		else b = m1;
    }

    *h=r;
    *s=g;
    *l=b;
}


void rgb2hsl_fast16(int * r, int * g, int * b)
{
    double h,s,l,cmax,cmin,diff;
	double rf,gf,bf;
 
	rf=(double) *r / 32768.0;
    gf=(double) *g / 32768.0;
    bf=(double) *b / 32768.0;

    cmax = bf;
    cmin = bf;
    if (rf > cmax) cmax = rf;
    if (gf > cmax) cmax = gf;
    if (rf < cmin) cmin = rf;
    if (gf < cmin) cmin = gf;

	l = (cmax + cmin)/2.0;
    if (cmax==cmin){
        s = 0.0;
        h = 0.0;
    } else {
		  diff = cmax-cmin;
		  if (diff==0) {
                s = 0;
		  } else if (l < 0.5) {
				s = diff/(cmax+cmin); 
          } else {
				s = diff/(2.0-cmax-cmin);
          }
