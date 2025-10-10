        //char saveFolder[MAX_PATH];

		char EXIFMake [256];
		char EXIFCamera [256];
		unsigned short EXIFISO;

        int imageOnStartup;
        
    } APPCONTEXT;

    APPCONTEXT app;


#endif

#if FOCALBLADE

    //Cache Values
    struct CACHEVAL
    {
       int cx;
       int cy;
       int cctl;
       int cctl2;
       int cscl;
    } cache_values;

    struct CACHEVAL cache[12];

#endif


struct FFFILTER {
	char category[256];
	char title[256];
	char author[256];
	char copyright[256];
	char filename[256];

	char relpath[256];

	char code[4][1024];
	char map[4][256];
	char ctl[8][256];
	int val[8];
	int def[8];

	BOOL supports16Bit;
	BOOL maxCompatibility;
	void * tree[4];
	//void * pcode[4];
};

extern BOOL mtAbortFlag;

extern BOOL UIScaling;

extern SPBasicSuite * sSPBasic;

EXTERNC void OnFilterExit();




#endif  // Closed ifndef AFHFM_H_DEF