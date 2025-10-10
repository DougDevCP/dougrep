///////////////////////////////////////////////////////////////////////////////////
//  File: FW_FFP.h
//
//  Header file of FilterMeisterVS which is required for linking the image processing
//  code with the FilterMeister framework
//
//  Copyright (c) 1997-2017 by Alex Hunter, Harald Heim & Martijn Van der Lee
// 
//  FilterMeisterVS is free software. You can redistribute and modify it
//  under the terms of the MPL2 license. See https://www.mozilla.org/en-US/MPL/2.0/
//  Any changes to the FilterMeisterVS files in the /components/filtermeister/
//  sub folder have to be published or sent to harry@thepluginsite.com. 
//  The individual filter code itself does not need to be disclosed.
//  
///////////////////////////////////////////////////////////////////////////////////

#ifndef FWFFP_H_DEF
#define FWFFP_H_DEF

#ifdef __cplusplus
#define EXTERNC extern "C"
#else
#define EXTERNC
#endif


EXTERNC char * AboutMessage (void);
char * getDefaultPresets (void);

EXTERNC int call_FFP(FFP_HANDLER h, INT_PTR p1);

int CtlDef (FMcontext * const fmc);
int OnCtl(FMcontext * const fmc, int n, int e, int previous);
int OnWindowStart(FMcontext * const fmc);
int OnFilterStart(FMcontext * const fmc);
int ForEveryTile(FMcontext * const fmc);
//int ForEveryRow(FMcontext * const fmc);
int OnFilterEnd(FMcontext * const fmc);
void OnFilterExit();


#endif // close ifndef FWFFP_H_DEF