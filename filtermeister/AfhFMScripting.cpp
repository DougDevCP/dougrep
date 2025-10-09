///////////////////////////////////////////////////////////////////////////////////
//  File: AfhFMScripting.c
//
//  Source file of FilterMeisterVS for scripting filter parameters.
//
//  Copyright (c) 1997-2017 by Alex Hunter, Harald Heim & Martijn Van der Lee
//  
//  Derived from Photoshop SDK source files which are: Copyright by Adobe Systems, Inc.
// 
//  FilterMeisterVS is free software. You can redistribute and modify it
//  under the terms of the MPL2 license. See https://www.mozilla.org/en-US/MPL/2.0/
//  Any changes to the FilterMeisterVS files in the /components/filtermeister/
//  sub folder have to be published or sent to harry@thepluginsite.com. 
//  The individual filter code itself does not need to be disclosed.
//  
///////////////////////////////////////////////////////////////////////////////////

#ifdef SCRIPTABLE

#include "AfhFMScripting.h"
#include "AfhFM.h"
#include "AfhFMcontext.h"
#include <stdio.h>
//#include <stdlib.h>

//FMcontext fmc;


int ScriptParamRead = 0;
int ScriptParam[] = {0,0,0,0,0,0,0,0};
int ScriptParamUsed[] = {true,true,true,true,true,true,true,true};
int ScriptProp[] = {0,0,0};//version,width,height
char * ScriptStrings = NULL;
int N_ScriptStrings = 0;
int maxStrings = 0;


OSErr ReadScriptParameters(Boolean* displayDialog)
{
	OSErr err = noErr;
	PIReadDescriptor token = 0;//NULL;
	DescriptorKeyID key = 0;//NULL;
	DescriptorTypeID type = 0;//NULL;
	//DescriptorUnitID units;
	int32 flags = 0;
	//double percent;
	int value;
	//DescriptorEnumID disposition;
	//Boolean ignoreSelection;
	DescriptorKeyIDArray array = {'ctl0','ctl1','ctl2'}; //= { keyAmount, keyDisposition, 0 }; //NULL
	PIDescriptorParameters* descParams;
	ReadDescriptorProcs* readProcs;

    int iKey;
    char * pKey;
    char swap;
    

	if (displayDialog != NULL)
		*displayDialog = (unsigned char)gParams->queryForParameters;
	else
		return errMissingParameter;

	descParams = gStuff->descriptorParameters;
	if (descParams == NULL) return err;
	
	readProcs = gStuff->descriptorParameters->readDescriptorProcs;
	if (readProcs == NULL) return err;

    //Info ("%d", descParams->playInfo); //plugInDialogDisplay

#if HYPERTYLE
	if (ScriptStrings!=NULL) free(ScriptStrings); 
	ScriptStrings = calloc( 256, 1);
#elif defined(COLORSTYLER) || defined(BWSTYLER2)
	if (ScriptStrings!=NULL) free(ScriptStrings); 
	ScriptStrings = calloc( (7+9+9+1)*256, 1);
	maxStrings = 26;
#elif defined(PLUGINGALAXY3)
	if (ScriptStrings!=NULL) free(ScriptStrings); 
	ScriptStrings = calloc( 32*256, 1);
	maxStrings = 32;
#elif defined(LIGHTMACHINE)
	if (ScriptStrings!=NULL) free(ScriptStrings); 
	ScriptStrings = calloc( 100*256, 1);
	maxStrings = 101;
#elif defined(COLORWASHER)
	if (ScriptStrings!=NULL) free(ScriptStrings); 
	ScriptStrings = calloc( 1*256, 1);
	maxStrings = 1;
#endif

	
	if (descParams->descriptor != NULL) // || gStuff->hostSig==0
	{
		token = readProcs->openReadDescriptorProc(descParams->descriptor, array);
		if (token != NULL)
		{
			
            //Info ("ReadScriptParameters2");
            
            while(readProcs->getKeyProc(token, &key, &type, &flags) && !err) {
			 
				pKey = (char *)&key;
				swap = *pKey;
				*pKey = *(pKey+3);
				*(pKey+3) = swap;
				swap = *(pKey+1);
				*(pKey+1) = *(pKey+2);
				*(pKey+2) = swap;
/*				
#if defined(COLORSTYLER) || defined(BWSTYLER2)
				if (strcmp(pKey,"xver")==0) {
					err = readProcs->getIntegerProc(token, &value);
					if (!err) ScriptProp[0]=value;
				}
				else if (strcmp(pKey,"xwid")==0) {
					err = readProcs->getIntegerProc(token, &value);
					if (!err) ScriptProp[1]=value;
				}
				else if (strcmp(pKey,"xhei")==0) {
					err = readProcs->getIntegerProc(token, &value);
					if (!err) ScriptProp[2]=value;
				}
				else 
#endif
*/
#if defined(PLUGINGALAXY3)
				//Info("%s",pKey);
				if (strncmp(pKey,"xver",4)==0) {
					err = readProcs->getIntegerProc(token, &value);
					if (!err) ScriptProp[0]=value;
				}
#endif

				if (type==typeInteger){
					iKey = atoi(pKey);

					if (iKey < N_SCRIPTITEMS){
						//Info ("%s\n%d",(char *)&key,iKey);
						err = readProcs->getIntegerProc(token, &value);
						if (!err){
							ScriptParam[iKey] = value;
							ScriptParamUsed[iKey] = true;
							ScriptParamRead++;
						}
					}
				}


#if HYPERTYLE

				else if (type==typeChar){
					char string[256];
					BYTE strLen;
					readProcs->getStringProc(token, &string);
					strLen = string[0];
					memcpy(ScriptStrings,&string[1],strLen);
					memset(ScriptStrings+strLen+1,0,1);
				}

#elif defined(COLORSTYLER) || defined(BWSTYLER2) || defined(PLUGINGALAXY3) || defined(LIGHTMACHINE) || defined(COLORWASHER)

				else if (type==typeChar){

					if (ScriptStrings){
						int count = -1;
						char string[256];

						iKey = atoi(pKey+1); //Ignore 't'
						#if defined(COLORSTYLER) || defined(BWSTYLER2) 
							if (iKey<7) count=iKey;
							else if (iKey<19) count=iKey-3; //else if (iKey<16) count=iKey-3;
							else if (iKey<29) count=iKey-4; 
							else count=iKey-5;
						#elif defined(PLUGINGALAXY3)
							if (ScriptProp[0]<310)
								count=pKey[3] - 0x30;
							else
								count=iKey;
						#else
							count=iKey;
						#endif

						if (count<maxStrings){
							readProcs->getStringProc(token, &string);
							//if (string[0]!=0)
								//strcpy(ScriptStrings+count*256,&string[1]);
								memcpy(ScriptStrings+count*256,string,strlen(&string[1])+1); //Avoid problem if first char is 0x0d

							// //Info ("%d: %s",count,ScriptStrings+count*256);
							//Info ("%d: %s",count,&string[1]);
						}
					}
				}
#endif

			}

#if defined(COLORSTYLER) || defined(BWSTYLER2) || defined(PLUGINGALAXY3) || defined(LIGHTMACHINE) || defined(COLORWASHER)
			readScriptStrings();
#endif

			err = readProcs->closeReadDescriptorProc(token);
			gStuff->handleProcs->disposeProc(descParams->descriptor);
			descParams->descriptor = NULL;

		}
		*displayDialog = descParams->playInfo == plugInDialogDisplay;
	}

	//Info ("%d,%d,%d",ScriptProp[0],ScriptProp[1],ScriptProp[2]);

    //For Fireworks we needs to use playInfo even if descriptor == NULL
    if (gStuff->hostSig==0) {
        *displayDialog = !(descParams->playInfo == plugInDialogDisplay); //Only display dialog on second run
    }

	return err;
}



OSErr WriteScriptParameters(void)
{
	OSErr err = noErr;
	PIWriteDescriptor token = NULL;
	PIDescriptorHandle h;
	//const double percent = gParams->percent;
	PIDescriptorParameters*	descParams;
	WriteDescriptorProcs* writeProcs;
    
    char sNumber[5]; 
    char swap;
    unsigned long * lnum = (unsigned long *)&sNumber;
    int i;

    //Info ("WriteScriptParameters1");
	//return false;

	descParams = gStuff->descriptorParameters;
	if (descParams == NULL) return err;
	
	writeProcs = gStuff->descriptorParameters->writeDescriptorProcs;
	if (writeProcs == NULL) return err;

	token = writeProcs->openWriteDescriptorProc();
	if (token != NULL)
	{

#if defined(COLORSTYLER) || defined(BWSTYLER2)

		//Write version number
		sNumber[0] = 'r';
		sNumber[1] = 'e';
		sNumber[2] = 'v';
		sNumber[3] = 'x';
		sNumber[4] = 0;
		writeProcs->putIntegerProc(token,*lnum, 100); //100 = Version 1.00

		//Image width
		sNumber[0] = 'd';
		sNumber[1] = 'i';
		sNumber[2] = 'w';
		sNumber[3] = 'x';
		sNumber[4] = 0;
		writeProcs->putIntegerProc(token,*lnum, fmc.imageWidth);

		//Image height
		sNumber[0] = 'i';
		sNumber[1] = 'e';
		sNumber[2] = 'h';
		sNumber[3] = 'x';
		sNumber[4] = 0;
		writeProcs->putIntegerProc(token,*lnum, fmc.imageHeight);

#elif defined(PLUGINGALAXY3)
		//Write version number
		sNumber[0] = 'r';
		sNumber[1] = 'e';
		sNumber[2] = 'v';
		sNumber[3] = 'x';
		sNumber[4] = 0;
		writeProcs->putIntegerProc(token,*lnum, 310); //300 = Version 3.00 //301 = 3.01 // 310 == 3.10
#endif

        for (i=0;i<N_SCRIPTITEMS;i++)
        { 
			if (ScriptParamUsed[i]){
                
                _itoa( i, sNumber, 10 );
                if (i<10){
                    sNumber[0] = sNumber[0];
                    sNumber[1] = 0x30;
                    sNumber[2] = 0x30;
                } else if (i<100) {
                    swap = sNumber[0];
                    sNumber[0] = sNumber[1];
                    sNumber[1] = swap;
                    sNumber[2] = 0x30;
                } else {
                    swap = sNumber[0];
                    sNumber[0] = sNumber[2];
                    sNumber[1] = sNumber[1];
                    sNumber[2] = swap;
                }
				sNumber[3] = 0x30;
                sNumber[4] = 0;
                
                //lnum = (unsigned long *)&sNumber; //Already done above
                //Info ("%d - %d - %d",i,*lnum, '0217');
			
                writeProcs->putIntegerProc(token,*lnum,ScriptParam[i]);
            }
        }

#if HYPERTYLE

		sNumber[4] = 0;
		sNumber[3] = 't';
        sNumber[2] = '0';
		sNumber[1] = '0';
		sNumber[0] = '0'; //0x30 + 0;
		{
			char string[256];
			int strLen = strlen(fmc.globvars.str0);
			if (strLen>255) strLen = 255;
			memset(string,strLen,1);
			memcpy(string+1,fmc.globvars.str0,strLen);
			writeProcs->putStringProc(token,*lnum,string);
		}


#elif defined(COLORWASHER)

		if (writeScriptStrings()){
			sNumber[4] = 0;
			sNumber[3] = 't';
			sNumber[2] = '0';    
			sNumber[1] = '0';
			sNumber[0] = '0';
			writeProcs->putStringProc(token,*lnum,ScriptStrings);
		}

#elif defined(LIGHTMACHINE)

		if (writeScriptStrings()){
			unsigned char * pch;
			int count=0;
			sNumber[4] = 0;
			sNumber[3] = 't';
			sNumber[2] = '0';    
			sNumber[1] = '0';
			sNumber[0] = '0';            
			for (i=0;i<N_ScriptStrings;i++){
				if (i>0){
					if (i%100==0) {
						sNumber[2]++;
						sNumber[1] = '0';
						sNumber[0] = '0';
					} else if (i%10==0) {
						sNumber[1]++; 
						sNumber[0] = '0';
					} else 
						sNumber[0]++;
				}
				pch = ScriptStrings+count*256;
				if (pch[0]==0) break;
				writeProcs->putStringProc(token,*lnum,pch);
				count++;
			}
		}

#elif defined(COLORSTYLER) || defined(BWSTYLER2)

		if (writeScriptStrings()){
			
			int count=0;

			//char text[256];
			//memset(text,(BYTE)strlen("Copyright (c) 2012 by Harald Heim"),1);
			//strcpy(text+1,"Copyright (c) 2012 by Harald Heim");
			//writeProcs->putStringProc(token,'t000',&text[0]);

			sNumber[4] = 0;
			sNumber[3] = 't';
            sNumber[2] = '0';
            
			sNumber[1] = '0';
			for (i=0;i<7;i++){
				sNumber[0] = 0x30 + i;
                writeProcs->putStringProc(token,*lnum,ScriptStrings+count*256);
				count++;
			}

			sNumber[1] = '1';
			for (i=0;i<9;i++){
				sNumber[0] = 0x30 + i;
				writeProcs->putStringProc(token,*lnum,ScriptStrings+count*256);
				count++;
			}

			sNumber[1] = '2';
			for (i=0;i<9;i++){
				sNumber[0] = 0x30 + i;
				writeProcs->putStringProc(token,*lnum,ScriptStrings+count*256);
				count++;
			}

			sNumber[1] = '3';
			sNumber[0] = '0';
			writeProcs->putStringProc(token,*lnum,ScriptStrings+count*256);
			count++;			
		}

#elif defined(PLUGINGALAXY3)

		if (writeScriptStrings()){
			unsigned char * pch;
			int count=0;
			sNumber[4] = 0;
			sNumber[3] = 't';
            sNumber[2] = '0';
			for (i=0;i<32;i++){
				if (i==0) sNumber[1] = '0';
				else if (i==10) sNumber[1] = '1';
				else if (i==20) sNumber[1] = '2';
				else if (i==30) sNumber[1] = '3';
				sNumber[0] = 0x30 + i%10; //Plugin Galaxy 3.09: sNumber[0] = 0x30 + i;
				pch = ScriptStrings+count*256;
				if (pch[0] ==0) break;
				writeProcs->putStringProc(token,*lnum,pch);
				count++;
			}
		}

#endif

                
		gStuff->handleProcs->disposeProc(descParams->descriptor);
		writeProcs->closeWriteDescriptorProc(token, &h);
		descParams->descriptor = h;
		descParams->recordInfo = plugInDialogOptional;
	}
	else
	{
		return errMissingParameter;
	}
	return err;
}


#endif

