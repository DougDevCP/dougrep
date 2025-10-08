///////////////////////////////////////////////////////////////////////////////////
//  File: fmdib_core.h
//
//  Source file with the FilterMeisterVS GDI drawing functions
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

#ifndef FMDIB_CORE_H
#define FMDIB_CORE_H

// Contains the init/exit code and support functions

HINSTANCE hInst_msimg32 = NULL;

// Generic
HDC hdcSource=0;
int GDIctlWidth, GDIctlHeight;
int GDIcanvasSize, GDIcanvasWidth, GDIcanvasHeight;
int GDIsuperSampling = 1;

// DIB
UINT * dibPixels = NULL;
HDC hdcDIB = NULL;
HBITMAP hbmDIB = NULL;
HBITMAP hbmDIBOld = NULL;

// GDI
BOOL modeGDI;
HDC hdcGDI = NULL;
HBITMAP hbmGDI = NULL;
HBITMAP hbmGDIOld = NULL;

//User passed DC
BOOL modeDirect;

//---------------------------------------------------------------------------

__inline UINT GDI_dibGetPixel (int x, int y)
{
	int i = x + (y * GDIcanvasWidth);

	if (i >= 0 && i < GDIcanvasSize) {
		UINT value = dibPixels[i];
		return (value >> 16) | (value & 0x0000FF00) | ( (value << 16) & 0x00FF0000);
	}

	return 0;
}

//---------------------------------------------------------------------------

__inline int GDI_dibSetPixel (int x, int y, UINT color)
{
	int i = x + (y * GDIcanvasWidth);

	if (i >= 0 && i < GDIcanvasSize) {
		dibPixels[i] = (color >> 16) | (color & 0x0000FF00) | ( (color << 16) & 0x00FF0000);
		return true;
	}

	return false;
}

//---------------------------------------------------------------------------

__inline UINT GDI_blendRGBA(UINT ca, UINT cb, double fraction)
{
	const double beta = 1. - fraction;

	UINT red = (UINT)(fraction * (ca & 0x000000FF) + beta * (cb & 0x000000FF));
	UINT grn = (UINT)(fraction * (ca & 0x0000FF00) + beta * (cb & 0x0000FF00));
	UINT blu = (UINT)(fraction * (ca & 0x00FF0000) + beta * (cb & 0x00FF0000));
	UINT alp = (UINT)(fraction * (ca & 0xFF000000) + beta * (cb & 0xFF000000));

	return red | (grn & 0x0000FF00) | (blu & 0x00FF0000) | (alp & 0xFF000000);
}

UINT blendRGBA(UINT ca, UINT cb, double fraction)
{
	return GDI_blendRGBA(ca, cb, fraction);
}

//---------------------------------------------------------------------------

__inline UINT GDI_dibMixPixel_BGR(int x, int y, UINT bgr, double alpha)
{
	if (x > 0 && x < GDIcanvasWidth && y > 0 && y < GDIcanvasHeight) {
		int i = x + (y * GDIcanvasWidth);

		dibPixels[i] = GDI_blendRGBA(bgr, dibPixels[i], alpha);

		return true;
	}

	return false;
}

//---------------------------------------------------------------------------

#endif
// FMDIB_CORE_H