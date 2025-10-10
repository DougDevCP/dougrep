/*  Note: The formatting is nonrecursive, to prevent inadvertent
/*        infinite loops.
/*
/*  Caution: The string returned by formatString() has a maximum
/*           length of 1024 chars (not including the terminating
/*           NUL), and will be truncated if necessary.  It resides 
/*           in (thread-local) static storage and will be overwritten
/*           by the next call to formatString() in the current thread.
/*           If you need the formatted string to persist, copy it to 
/*           your own storage.
/*
/**********************************************************************/

char *formatString(const char *s)
{
    THREAD_LOCAL static char f[1024];        //Warning: static!!!! NOT THREAD-SAFE!!
    unsigned int cbLeft = sizeof(f) - 1;
    unsigned int cbCopied;
	int n;
    char *p = f;
    char *q;
    char buffer[32];

    while (*s) {
        if (*s == '!') {
            s++;
            switch (*s) {
            case 'A':
                q = fmc.filterAuthorText;
                break;
            case 'a':
                q = gParams->filterAboutText;
                break;
            case 'C':
                q = fmc.filterCategoryText;
                break;
            case 'c':
                q = fmc.filterCopyrightText;
                break;
            case 'D':
                q = fmc.filterDescriptionText;
                break;
            case 'F':
                q = fmc.filterFilenameText;
                break;
            case 'f':
                q = fmc.filterCaseText;
                break;
            case 'H':
                q = fmc.filterHostText;
                break;
            case 'h':
                q = _itoa(fmc.imageHeight, buffer, 10);
                break;
            case 'M':
                q = fmc.filterImageModeText;
                break;
            case 'm':
                q = _itoa(fmc.imageMode, buffer, 10);
                break;
            case 'O':
                q = fmc.filterOrganizationText;
                break;
            case 'T':
                q = fmc.filterTitleText;
                break;
            case 't':
                q = stripEllipsis(fmc.filterTitleText);
                break;
            case 'U':
                q = fmc.filterURLText;
                break;
            case 'V':
                q = fmc.filterVersionText;
                break;
            case 'w':
                q = _itoa(fmc.imageWidth, buffer, 10);
                break;
            case 'Y':
                {   time_t tm = time(NULL);
                    q = _itoa(localtime(&tm)->tm_year + 1900, buffer, 10);
                }
                break;
            case 'z':
                q = _itoa(fmc.zoomFactor, buffer, 10);
                break;
            case '!':
                q = "!";
                break;
            default:
                //copy a '!' to the formatted string
                q = "!";
                s--;    //back up one char to rescan the char after the '!'
                break;
            } //switch
            //copy the portion of the interpolated string
            //that will fit in the formatted string...
            cbCopied = (unsigned int)min(cbLeft, strlen(q));
            strncpy(p, q, cbCopied);
            p += cbCopied;
            cbLeft -= cbCopied;
        }
		else if (*s == '&') {
			//possible HTML entity...

			//define a macro to compare against a named entity
			#define ENTITY(entity, subst) \
				if (!strncmp(s, (entity), n = sizeof(entity)-1)) {\
					q = (subst); goto subst_entity;\
				}

            /* Cases are sorted by (first char of) entity name for faster lookup */
            switch (s[1]) {
			case '#':
				/* Numeric character reference */
				//we don't accept UNICODE numeric entities above 0x00ff for now...
				#define MAX_NUMERIC_ENTITY_VAL	255
				{
					unsigned int iVal = 0;
					const char *s2 = s + 2; // point past '#'
					n = 2;	// chars eaten so far
					if (*s2 == 'x' || *s2 == 'X') {
						/* hexadecimal numeric entity */
						s2++;
						n++;
						if (!isxdigit(*s2)) break;	// error, must be at least one digit
						while (isxdigit(*s2)) {
							iVal = iVal * 16 + ( isdigit(*s2) ? *s2 - '0' : (*s2 & 7) + 9 );
							// check for overflow
							if (iVal > MAX_NUMERIC_ENTITY_VAL) break;	// error, value too large
							s2++;
							n++;
						}//while hex digit
					}
					else {
						/* decimal numeric entity */
						if (!isdigit(*s2)) break;	// error, must be at least one digit
						while (isdigit(*s2)) {
							iVal = iVal * 10 + (*s2 - '0');
							// check for overflow
							if (iVal > MAX_NUMERIC_ENTITY_VAL) break;	// error, value too large
							s2++;
							n++;
						}//while decimal digit
					}
					if (*s2 != ';') break;	// Error, no final ';'
					n++;
					// disallow certain graphic values
					if (iVal < 0x00 ||		// NUL allowed?
						iVal == 0x7f ||		// 0x7f reserved for multibyte leader?
						iVal == 0x81 ||		// 0x81 reserved for multibyte????
						iVal > 0xff )		// only allow 8-bit codes? TRANSLATE other ISO-xxxx to Windows charset???
											// Could normalize by stripping leading 0's, then lookup e.g. "&#8084;"
											//  -- nahh, might as well lookup binary val directly.
					{
						iVal = 0x81;		// graphic to represent non-displayable coe?
					}
					q = (char *)&iVal;		// treat as NUL-terminated string. (tricky cast!!!)
				}
				cbCopied = 1;				// copy exactly 1 byte, which may be a NUL byte
				goto subst_entity2;
				#undef MAX_NUMERIC_ENTITY_VAL
				break;
            case '&':
			    ENTITY("&&",		"&");		// & (&& => verbatim &)
                break;
            case 'A':
			    ENTITY("&AElig;",	"\xC6");	// �
			    ENTITY("&Aacute;",	"\xC1");	// �
			    ENTITY("&Acirc;",	"\xC2");	// �
			    ENTITY("&Agrave;",	"\xC0");	// �
			    ENTITY("&Alpha;",	"A");		// A (Greek Alpha, use Latin A)
			    ENTITY("&Aring;",	"\xC5");	// �
			    ENTITY("&Atilde;",	"\xC3");	// �
			    ENTITY("&Auml;",	"\xC4");	// �
                break;
            case 'B':
				ENTITY("&BEL;",		"\x07");	// BEL (ASCII Control code, not ISO)
				ENTITY("&BS;",		"\x08");	// BS (ASCII Control code, not ISO)
			    ENTITY("&Beta;",	"B");		// B (Greek Beta, use Latin B)
                break;
            case 'C':
				ENTITY("&CR;",		"\x0D");	// CR (ASCII Control code, not ISO)
				ENTITY("&CRLF;",	"\r\n");	// CR/LF (ASCII Control code sequence, not ISO)
			    ENTITY("&Ccedil;",	"\xC7");	// �
			    ENTITY("&Chi;",	    "X");		// X (Greek Chi, use Latin X)
			    ENTITY("&Colon;",	"::");		// :: (double colon)
                break;
            case 'D':
			    ENTITY("&Dagger;",	"\x87");	// � (double dagger)
                break;
            case 'E':
			    ENTITY("&ESC;",		"\x1B");	// ESC (ASCII control code, not ISO entity)
			    ENTITY("&ETH;",		"\xD0");	// �
			    ENTITY("&Eacute;",	"\xC9");	// �
			    ENTITY("&Ecirc;",	"\xCA");	// �
			    ENTITY("&Egrave;",	"\xC8");	// �
			    ENTITY("&Epsilon;",	"E");		// E (Greek Epsilon, use Latin E)
			    ENTITY("&Eta;",	    "H");		// H (Greek Eta, use Latin H)
			    ENTITY("&Euml;",	"\xCB");	// �
			    ENTITY("&Exclam;",	"\x13");	// OEM (double exclamation mark !!)
                break;
			case 'F':
				ENTITY("&FF;",		"\x0c");	// FF (ASCII Control code, not ISO)
                break;
			case 'H':
				ENTITY("&HT;",		"\x09");	// HT (ASCII Control code, not ISO)
				break;
            case 'I':
			    ENTITY("&Iacute;",	"\xCD");	// �
			    ENTITY("&Icirc;",	"\xCE");	// �
			    ENTITY("&Igrave;",	"\xCC");	// �
			    ENTITY("&Iota;",	"I");		// I (Greek Iota, use Latin I)
			    ENTITY("&Iuml;",	"\xCF");	// �
                break;
            case 'K':
			    ENTITY("&Kappa;",	"K");		// K (Greek Kappa, use Latin K)
                break;
            case 'L':
				ENTITY("&LF;",		"\x0A");	// LF (ASCII Control code, not ISO)
                break;
            case 'M':
			    ENTITY("&Mu;",	    "M");		// M (Greek Mu, use Latin M)
                break;
            case 'N':
				///ENTITY("&NUL;",		"\x00");	// NUL (ASCII Control code, not ISO)	/// NFG!!!
			    ENTITY("&Ntilde;",	"\xD1");	// �
			    ENTITY("&Nu;",	    "N");		// N (Greek Nu, use Latin N)
                break;
            case 'O':
			    ENTITY("&OElig;",	"\x8C");	// � (capital ligature OE)
			    ENTITY("&Oacute;",	"\xD3");	// �
			    ENTITY("&Ocirc;",	"\xD4");	// �
			    ENTITY("&Ograve;",	"\xD2");	// �
			    ENTITY("&Omicron;",	"O");		// O (Greek Omicron, use Latin O)
			    ENTITY("&Oslash;",	"\xD8");	// �
			    ENTITY("&Otilde;",	"\xD5");	// �
			    ENTITY("&Ouml;",	"\xD6");	// �
                break;
            case 'P':
			    ENTITY("&Prime;",	"\"");		// " (seconds, inches; use &quot;)
                break;
            case 'R':
			    ENTITY("&Rho;",	    "P");		// P (Greek Rho, use Latin P)
                break;
            case 'S':
			    ENTITY("&Scaron;",	"\x8A");	// � (capital S with caron)
                break;
            case 'T':
			    ENTITY("&THORN;",	"\xDE");	// �
			    ENTITY("&Tau;",	    "T");		// T (Greek Tau, use Latin T)
                break;
            case 'U':
			    ENTITY("&Uacute;",	"\xDA");	// �
			    ENTITY("&Ucirc;",	"\xDB");	// �
			    ENTITY("&Ugrave;",	"\xD9");	// �
			    ENTITY("&Upsilon;",	"Y");		// Y (Greek Upsilon, use Latin Y)
			    ENTITY("&Uuml;",	"\xDC");	// �
                break;
            case 'V':
				ENTITY("&VT;",		"\x0B");	// VT (ASCII Control code, not ISO)
                break;
            case 'Y':
			    ENTITY("&Yacute;",	"\xDD");	// �
			    ENTITY("&Yuml;",	"\x9F");	// � (capital Y with umlaut/diaeresis)
                break;
            case 'Z':
			    ENTITY("&Zcaron;",	"\x8E");	// � (capital Z with caron, not official?)
			    ENTITY("&Zeta;",	"Z");		// Z (Greek Zeta, use Latin Z)
                break;
            case 'a':
			    ENTITY("&aacute;",	"\xE1");	// �
			    ENTITY("&acirc;",	"\xE2");	// �
			    ENTITY("&acute;",	"\xB4");	// �
			    ENTITY("&aelig;",	"\xE6");	// �
			    ENTITY("&agrave;",	"\xE0");	// �
			    ENTITY("&amp;",		"\x26");	// &
			    ENTITY("&apos;",	"\x27");	// ' (not official HTML)
			    ENTITY("&aring;",	"\xE5");	// �
			    ENTITY("&atilde;",	"\xE3");	// �
			    ENTITY("&auml;",	"\xE4");	// �
                break;
            case 'b':
			    ENTITY("&bdquo;",	"\x84");	// � (double low-9 quotation mark)
#if 0
			    ENTITY("&beamedsixteenthnotes;", "\x0E");   // OEM (beamed 16th notes)
#endif
			    ENTITY("&beta;",	"\xDF");	// � (Greek beta, use &szlig; -- yecch!)
			    ENTITY("&blk14;",	"\x0F");	// OEM LIGHT SHADE  
			    ENTITY("&boxdl;",	"\x02");	// OEM BOX DRAWINGS LIGHT DOWN AND LEFT  
			    ENTITY("&boxdr;",	"\x01");	// OEM BOX DRAWINGS LIGHT DOWN AND RIGHT  
			    ENTITY("&boxh;",	"\x06");	// OEM BOX DRAWINGS LIGHT HORIZONTAL  
			    ENTITY("&boxhd;",	"\x16");	// OEM BOX DRAWINGS LIGHT HORIZONTAL AND DOWN
			    ENTITY("&boxhu;",	"\x15");	// OEM BOX DRAWINGS LIGHT HORIZONTAL AND UP
			    ENTITY("&boxul;",	"\x04");	// OEM BOX DRAWINGS LIGHT UP AND LEFT  
			    ENTITY("&boxur;",	"\x03");	// OEM BOX DRAWINGS LIGHT UP AND RIGHT  
			    ENTITY("&boxv;",	"\x05");	// OEM BOX DRAWINGS LIGHT VERTICAL  
			    ENTITY("&boxvh;",	"\x10");	// OEM BOX DRAWINGS LIGHT VERTICAL AND HORIZONTAL  
			    ENTITY("&boxvl;",	"\x17");	// OEM BOX DRAWINGS LIGHT VERTICAL AND LEFT  
			    ENTITY("&boxvr;",	"\x19");	// OEM BOX DRAWINGS LIGHT VERTICAL AND RIGHT  