///////////////////////////////////////////////////////////////////////////////////
//  File: AfhFMScripting.h
//
//  Header file of FilterMeisterVS for scripting filter parameters.
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

//#ifndef _DISSOLVESCRIPTING_H
//#define _DISSOLVESCRIPTING_H

#include "PIDefines.h"
#include "PITypes.h"
//#include "PIActions.h"
//#include "PITerminology.h"

//#ifndef Rez
//#include "Dissolve.h"
//#endif


//#define keyDisposition 		'disP'
//#define keyIgnoreSelection	'ignS'
//#define typeMood		'mooD'
//#define dispositionClear	'moD0'
//#define dispositionCool		'moD1'
//#define dispositionHot		'moD2'
//#define dispositionSick		'moD3'

#ifndef Rez
OSErr ReadScriptParameters(Boolean* displayDialog);
OSErr WriteScriptParameters(void);
//int16 ScriptToDialog(int32 script);
//int32 DialogToScript(int16 dialog);
#endif
//#endif
