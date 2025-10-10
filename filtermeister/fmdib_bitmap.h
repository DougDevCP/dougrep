///////////////////////////////////////////////////////////////////////////////////
//  File: fmdib_bitmap.h
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

#ifndef FMDIB_BITMAP_H
#define FMDIB_BITMAP_H

#include "fmdib_core.h"

typedef HRESULT (STDAPICALLTYPE *LPTRANSPARENTBLT) (HDC, int, int, int, int, HDC, int, int, int, int, UINT);
LPTRANSPARENTBLT lpTransparentBlt = NULL;

/*
int setBitmapTile(int x, int y, int iName, int tiles_x, int tiles_y, int tile)
int setBitmapTileStretch(int x, int y, int iName, int tiles_x, int tiles_y, int tile, int width, int height)
int setBitmapTileTransparent(int x, int y, int iName, int tiles_x, int tiles_y, int tile, UINT transparent)
int setBitmapTileStretchTransparent(int x, int y, int iName, int tiles_x, int tiles_y, int tile, int width, int height, UINT transparent)
int setBitmapTransparent(int x, int y, int iName, UINT transparent)
int setBitmapStretchTransparent(int x, int y, int iName, int width, int height, UINT transparent)

  BOOL TransparentBlt(
  HDC hdcDest,        // handle to destination DC
  int nXOriginDest,   // x-coord of destination upper-left corner
  int nYOriginDest,   // y-coord of destination upper-left corner
  int nWidthDest,     // width of destination rectangle
  int hHeightDest,    // height of destination rectangle
  HDC hdcSrc,         // handle to source DC
  int nXOriginSrc,    // x-coord of source upper-left corner
  int nYOriginSrc,    // y-coord of source upper-left corner
  int nWidthSrc,      // width of source rectangle
  int nHeightSrc,     // height of source rectangle
  UINT crTransparent  // color to make transparent
); 

Parameters
hdcDest 
[in] Handle to the destination device context. 
nXOriginDest 
[in] Specifies the x-coordinate, in logical units, of the upper-left corner of the destination rectangle. 
nYOriginDest 
[in] Specifies the y-coordinate, in logical units, of the upper-left corner of the destination rectangle. 
nWidthDest 
[in] Specifies the width, in logical units, of the destination rectangle. 
hHeightDest 
[in] Handle to the height, in logical units, of the destination rectangle. 
hdcSrc 
[in] Handle to the source device context. 
nXOriginSrc 
[in] Specifies the x-coordinate, in logical units, of the source rectangle. 
nYOriginSrc 
[in] Specifies the y-coordinate, in logical units, of the source rectangle. 
nWidthSrc 
[in] Specifies the width, in logical units, of the source rectangle. 
nHeightSrc 
[in] Specifies the height, in logical units, of the source rectangle. 
crTransparent 
[in] The RGB color in the source bitmap to treat as transparent. 


stretch mode?!*/

static HBITMAP GDI_CreateHBitmapFromImage(INT_PTR iName) {
	HBITMAP hBmp;

	if (iName == 0) {
		return NULL;
	}

    // First try loading the bitmap from an embedded BITMAP resource...
    hBmp = (HBITMAP)LoadImage((HINSTANCE)hDllInstance,
                     (char *)iName,
                     IMAGE_BITMAP,
                     0, 0,
                     LR_DEFAULTCOLOR);

    // If it's not embedded, try searching for it as an external bitmap file...
    if (!hBmp) {
        hBmp = (HBITMAP)LoadImage((HINSTANCE)hDllInstance,
                         FmSearchFile((char *)iName),
                         IMAGE_BITMAP,
                         0, 0,
                         LR_LOADFROMFILE);
    }

    if (!hBmp) {
        ErrorOk("Could not load bitmap %s", (char *)iName);
        return NULL;
    }

	return hBmp;
}

//---------------------------------------------------------------------------

int setBitmap(int x, int y, int iName)
{
	int result = false;
	
	HBITMAP hBmp = GDI_CreateHBitmapFromImage(iName);

    if (hBmp) {
		HDC hDC, hMemDC;
		BITMAP bm;
		HBITMAP hOldBitmap;

		hDC = modeGDI? hdcGDI : hdcDIB;

		GetObject(hBmp, sizeof(BITMAP), &bm);
		hMemDC		= CreateCompatibleDC(hDC);
		hOldBitmap	= (HBITMAP)SelectObject(hMemDC, hBmp);
		
		result = BitBlt(hDC, x, y, bm.bmWidth, bm.bmHeight, hMemDC, 0, 0, SRCCOPY);

		SelectObject(hMemDC, hOldBitmap);
		DeleteObject(hBmp);
	}

	return result;
};

int setBitmapTile(int x, int y, int iName, int tileWidth, int tileHeight, int tileIndex)
{
	int result = false;

	if (tileWidth > 0 && tileHeight > 0) {
		HBITMAP hBmp = GDI_CreateHBitmapFromImage(iName);

		if (hBmp) {
			int xSrc, ySrc;
			int columns;

			HDC hDC, hMemDC;
			BITMAP bm;
			HBITMAP hOldBitmap;

			hDC = modeGDI? hdcGDI : hdcDIB;

			GetObject(hBmp, sizeof(BITMAP), &bm);
			hMemDC		= CreateCompatibleDC(hDC);
			hOldBitmap	= (HBITMAP)SelectObject(hMemDC, hBmp);

			columns = (bm.bmWidth / tileWidth);

			xSrc = (tileIndex % columns) * tileWidth;
			ySrc = (tileIndex / columns) * tileHeight;
			
			result = BitBlt(hDC, x, y, tileWidth, tileHeight, hMemDC, xSrc, ySrc, SRCCOPY);

			SelectObject(hMemDC, hOldBitmap);
			DeleteObject(hBmp);
		}
	}

	return result;
};

int setBitmapTransparent(int x, int y, int iName, UINT color)
{
	int result = false;

	HBITMAP hBmp = GDI_CreateHBitmapFromImage(iName);

    if (hBmp) {
		HDC hDC, hMemDC;
		BITMAP bm;
		HBITMAP hOldBitmap;

		hDC = modeGDI? hdcGDI : hdcDIB;

		GetObject(hBmp, sizeof(BITMAP), &bm);
		hMemDC		= CreateCompatibleDC(hDC);
		hOldBitmap	= (HBITMAP)SelectObject(hMemDC, hBmp);
		
		// load lpTransparentBlt function
		if (!lpTransparentBlt) {
			if (!hInst_msimg32) {
				hInst_msimg32 = LoadLibrary("msimg32.dll");
			}
			lpTransparentBlt = ((LPTRANSPARENTBLT) GetProcAddress(hInst_msimg32, "TransparentBlt"));
		}

		if (lpTransparentBlt) {
		result = lpTransparentBlt(hDC, x, y, bm.bmWidth, bm.bmHeight, hMemDC, 0, 0, bm.bmWidth, bm.bmHeight, color);
		}

		SelectObject(hMemDC, hOldBitmap);
		DeleteObject(hBmp);
	}

	return result;
};

int setBitmapStretch(int x, int y, int iName, int width, int height)
{
	int result = false;
	
	HBITMAP hBmp = GDI_CreateHBitmapFromImage(iName);

    if (hBmp) {
		HDC hDC, hMemDC;
		BITMAP bm;
		HBITMAP hOldBitmap;

		hDC = modeGDI? hdcGDI : hdcDIB;

		GetObject(hBmp, sizeof(BITMAP), &bm);
		hMemDC		= CreateCompatibleDC(hDC);
		hOldBitmap	= (HBITMAP)SelectObject(hMemDC, hBmp);
		
		SetStretchBltMode(hDC, HALFTONE);
		SetBrushOrgEx(hDC, 0, 0, NULL);
		result = StretchBlt(hDC, x, y, width, height, hMemDC, 0, 0, bm.bmWidth, bm.bmWidth, SRCCOPY);

		SelectObject(hMemDC, hOldBitmap);
		DeleteObject(hBmp);
	}

	return result;
};

int setBitmapStretchTransparent(int x, int y, int iName, int width, int height, UINT color)
{
	int result = false;
	
	HBITMAP hBmp = GDI_CreateHBitmapFromImage(iName);

    if (hBmp) {
		HDC hDC, hMemDC;
		BITMAP bm;
		HBITMAP hOldBitmap;

		hDC = modeGDI? hdcGDI : hdcDIB;

		GetObject(hBmp, sizeof(BITMAP), &bm);
		hMemDC		= CreateCompatibleDC(hDC);
		hOldBitmap	= (HBITMAP)SelectObject(hMemDC, hBmp);
		
		// load lpTransparentBlt function
		if (!lpTransparentBlt) {
			if (!hInst_msimg32) {
				hInst_msimg32 = LoadLibrary("msimg32.dll");
			}
			lpTransparentBlt = ((LPTRANSPARENTBLT) GetProcAddress(hInst_msimg32, "TransparentBlt"));
		}

		if (lpTransparentBlt) {
		result = lpTransparentBlt(hDC, x, y, width, height, hMemDC, 0, 0, bm.bmWidth, bm.bmHeight, color);
		}

		SelectObject(hMemDC, hOldBitmap);
		DeleteObject(hBmp);
	}

	return result;
};

#endif
// FMDIB_BITMAP_H