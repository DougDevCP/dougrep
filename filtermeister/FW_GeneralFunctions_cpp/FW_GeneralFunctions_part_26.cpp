          if (rf==cmax) {
				h = (gf-bf)/diff;
          } else if (gf==cmax) {
				h = 2.0 + (bf-rf)/diff;
          } else {
				h = 4.0 + (rf-gf)/diff;
          }
          h = h / 6.0;
          if (h < 0) h += 1.0;
    }   

    *r =(int)(h*32768.0);
    *g =(int)(s*32768.0);
    *b =(int)(l*32768.0);
}

void hsl2rgb_fast16(int * h, int * s, int * l)
{
    double r,g,b, h2,m1,m2;
	double h1=(double) *h / 32768.0;
    double s1=(double) *s / 32768.0;
    double l1=(double) *l / 32768.0;
	
    if (s == 0) {
        r = l1;
        g = l1;
        b = l1;
    } else {
        if (l1 <= 0.5) m2 = l1 * (1.0+s1);
		else m2 = (l1+s1) - (l1 * s1);
        m1 = 2.0 * l1 - m2;

        //Calculate Red
        h2=h1+1.0/3.0;
        if (h2 < 0) h2 = h2 + 1.0;
        if (h2 > 1) h2 = h2 - 1.0;
		if (6.0 * h2 < 1) r = (m1 + (m2-m1)*h2*6.0);
        else if (2.0 * h2 < 1) r = m2;
        else if (3.0 * h2 < 2.0) r = (m1 + (m2-m1)*((2.0/3.0)-h2)*6.0);
		else r = m1;
        
        //Calculate Green               
        if (h1 < 0) h1 = h1 + 1.0;
        if (h1 > 1) h1 = h1 - 1.0;
        if (6.0 * h1 < 1) g = (m1+(m2-m1)* h1 *6.0);
        else if (2.0 * h1 < 1) g = m2;
        else if (3.0 * h1 < 2.0) g = (m1+(m2-m1)*((2.0/3.0)-h1)*6.0);
		else g = m1;
        
        //Calculate Blue
        h2=h1-1.0/3.0;
        if (h2 < 0) h2 = h2 + 1.0;
        if (h2 > 1) h2 = h2 - 1.0;
        if (6.0 * h2 < 1) b = (m1+(m2-m1)*h2*6.0);
        else if (2.0 * h2 < 1) b = m2;
        else if (3.0 * h2 < 2.0) b = (m1+(m2-m1)*((2.0/3.0)-h2)*6.0);
		else b = m1;
    }

    *h=(int)(r*32768.0);
    *s=(int)(g*32768.0);
    *l=(int)(b*32768.0);
}



int rgb2hsl_safe(int r, int g, int b, int z)
{

    double rf,gf,bf,hf,sf,lf, cmax,cmin,diff;
    
	int Cr,Cg,Cb;
    int Ch,Cs,Cl;


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


int hsl2rgb_safe(int h, int s, int l, int z)
{

    //int r,g,b;
    double rf,gf,bf,hf,sf,lf, hf2, m1,m2;
    
    int Cr,Cg,Cb;
    int Ch,Cs,Cl;


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






//function added by Harald Heim on Feb 6, 2002
int rgb2ycbcr(int r, int g, int b, int z)
{
    // [  Y'  601 ] [ 0.299     0.587     0.114    ] [ R' ] 
    // [  PB  601 ]=[-0.168736 -0.331264  0.5      ]*[ G' ] 
    // [  PR  601 ] [ 0.5      -0.418688 -0.081312 ] [ B' ]   
    
    if (z == 0)
        return (299*r + 587*g + 114*b)/1000;
    else if (z == 1)
        return (-169*r - 331*g + 500*b)/1000 + 128*mode_bitMultiply;
    else if (z == 2)
        return (500*r - 419*g - 81*b)/1000 + 128*mode_bitMultiply;
    else
        return 0;
}

//function added by Harald Heim on Feb 8, 2002
int ycbcr2rgb(int y, int cb, int cr, int z)
{
    cb = cb - 128*mode_bitMultiply;
    cr = cr - 128*mode_bitMultiply;
    
    //[ R' ] [ 1.        0.        1.402    ] [  Y'  601 ] 
    //[ G' ]=[ 1.       -0.344136 -0.714136 ]*[  PB  601 ] 
    //[ B' ] [ 1.        1.772     0.       ] [  PR  601 ]  

    if (z == 0)
        return y + 1402*cr/1000; 
    else if (z == 1)
        return y - (344*cb + 714*cr)/1000; 
    else if (z == 2)
        return y + 1772*cb/1000;
    else
        return 0;
}


//function added by Harald Heim on Jun 17, 2003
int rgb2ypbpr(int r, int g, int b, int z)
{
    //Y=  0.2122*Red+0.7013*Green+0.0865*Blue    
    //Pb=-0.1162*Red-0.3838*Green+0.5000*Blue    
    //Pr= 0.5000*Red-0.4451*Green-0.0549*Blue
    
    if (z == 0)
        return (212*r + 701*g + 87*b)/1000;
    else if (z == 1)
        return (-116*r - 384*g + 500*b)/1000 + 128*mode_bitMultiply;
    else if (z == 2)
        return (500*r - 445*g - 55*b)/1000 + 128*mode_bitMultiply;
    else
        return 0;
}

//function added by Harald Heim on Jun 17, 2003
int ypbpr2rgb(int y, int pb, int pr, int z)
{
    pb = pb - 128*mode_bitMultiply;
    pr = pr - 128*mode_bitMultiply;
    
    //Red  =1*Y+0.0000*Pb+1.5756*Pr
    //Green=1*Y-0.2253*Pb+0.5000*Pr
    //Blue =1*Y+1.8270*Pb+0.0000*Pr

    if (z == 0)
        return y + 1576*pr/1000; 
    else if (z == 1)