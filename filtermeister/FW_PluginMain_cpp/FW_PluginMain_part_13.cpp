#else
	*scaleFactor = max(fraction.h, fraction.v);
#endif
					   
	
//possibly the simplest implementation??
//*scaleFactor = min(*scaleFactor, min(filterHeight, filterWidth));

#if BIGDOCUMENT   
	if (filterHeight/(*scaleFactor)==0 || filterWidth/(*scaleFactor)==0){
		int c;
		for (c=*scaleFactor; c>=1; c--){
			if (filterHeight/c>0 && filterWidth/c>0) {
				*scaleFactor = c;
				break;
			}
		}
	}
#else
	//For very thin images
	if (filterHeight/(short)scaleFactor==0 || filterWidth/(short)scaleFactor==0){
	//if (filterHeight/(short)*scaleFactor==0 || filterWidth/(short)*scaleFactor==0){ //Bug Fix
		short c;
		for (c=(short)*scaleFactor; c>=1; c--){
			if (filterHeight/c>0 && filterWidth/c>0) {
				*scaleFactor = (long) c;
				break;
			}
		}
	}
#endif


    //copyRect (proxyRect, filterRect); 
    //scaleRect (proxyRect, 1, *scaleFactor);

}

