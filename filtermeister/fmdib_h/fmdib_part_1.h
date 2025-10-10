///////////////////////////////////////////////////////////////////////////////////
//  File: fmdib.h
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

#ifndef FMDIB_H
#define FMDIB_H

#include <stdio.h>
#include "fmmath.h"
#include "fmdib_core.h"
#include "fmdib_bitmap.h"
#include "fmdib_antialias.h"
#include "fmdib_text.h"

typedef HRESULT (STDAPICALLTYPE *LPGRADIENTFILL) (HDC, PTRIVERTEX, ULONG, PVOID, ULONG, ULONG);
LPGRADIENTFILL lpGradientFill = NULL;

/* FUTURE
	Copy to clipboard/paste from clipboard
	Hardware scaling?
	Buffered canvas with FSAA
		internal:
			GDIstartBuffer(hwnd, width, height);
			GDIendBuffer(hwnd, width, height);
	Pen style                        
		constants
	Brush style
		constants
	More primitives
		triangle -> polygon engine
		bezier
		b-spline
	Native AA algorithms
		Martin's AA line code
*/

//---------------------------------------------------------------------------

// Pen state

HPEN GDIoldPen = NULL;
HPEN GDInewPen = NULL;

UINT GDIcurrentPenColor = 0;

#define GDIpenMode_unassigned	0
#define GDIpenMode_clear		1
#define GDIpenMode_color		2
BYTE GDIpenMode = GDIpenMode_unassigned;

int GDIcurrentPenWidth	= -999;
int GDIpenWidth    		= 1;
int setPenWidth(int width)
{
	GDIpenWidth = max(1, width);
	return true;
}

int GDIcurrentPenStyle	= -999;
int GDIpenStyle			= PS_SOLID;
int setPenStyle(int e)
{
	if (e == PS_SOLID || e == PS_DASH || e == PS_DOT || e == PS_DASHDOT || e == PS_DASHDOTDOT) {
		GDIpenStyle	= e;
	}

	return true;
}

int GDIcurrentPenEndCaps	= -999;
int GDIpenEndCaps			= PS_ENDCAP_ROUND;
int setEndCaps(int e)
{
	if (e == PS_ENDCAP_ROUND || e == PS_ENDCAP_SQUARE || e == PS_ENDCAP_FLAT) {
		GDIpenEndCaps	= e;
	}

	return true;
}

int GDIcurrentPenJoin	= -999;
int GDIpenJoin			= PS_JOIN_ROUND;
int setJoin(int j)
{
	if (j == PS_JOIN_ROUND || j == PS_JOIN_BEVEL || j == PS_JOIN_MITER) {
		GDIpenJoin	= j;
	}

	return true;
}

int GDIcurrentPenInsideFrame	= -999;
int GDIpenInsideFrame	 		= 0;
int setInsideFrame(int e)
{
	GDIpenInsideFrame	= (e? PS_INSIDEFRAME : 0);

	return true;
}

void GDI_setPen(UINT color)
{
	if (GDInewPen) {
		if (GDIpenMode  		!= GDIpenMode_color
		 || color       		!= GDIcurrentPenColor
		 || GDIpenWidth 		!= GDIcurrentPenWidth
		 || GDIpenInsideFrame 	!= GDIcurrentPenInsideFrame
		 || GDIpenJoin 			!= GDIcurrentPenJoin
		 || GDIpenEndCaps 		!= GDIcurrentPenEndCaps
		 || GDIpenStyle 		!= GDIcurrentPenStyle
		 ) {
			
			LOGBRUSH lb;
			
			SelectObject(hdcGDI, GDIoldPen);
			DeleteObject(GDInewPen);

			lb.lbStyle = BS_SOLID;
			lb.lbColor = color;
			lb.lbHatch = (ULONG_PTR)NULL;
			GDInewPen = ExtCreatePen(PS_GEOMETRIC | GDIpenStyle /*PS_SOLID*/ | GDIpenEndCaps
				| GDIpenInsideFrame | GDIpenJoin
				,GDIpenWidth, &lb, 0, NULL);

			GDIoldPen = (HPEN)SelectObject(hdcGDI, GDInewPen);
		}
	} else {
		LOGBRUSH lb;
		lb.lbStyle = BS_SOLID;
		lb.lbColor = color;
		lb.lbHatch = (ULONG_PTR)NULL;
		GDInewPen = ExtCreatePen(PS_GEOMETRIC | GDIpenStyle /*PS_SOLID*/ | GDIpenEndCaps
				| GDIpenInsideFrame | GDIpenJoin
			,GDIpenWidth, &lb, 0, NULL);
		GDIoldPen = (HPEN)SelectObject(hdcGDI, GDInewPen);
	}

	GDIcurrentPenColor 			= color;
	GDIcurrentPenWidth			= GDIpenWidth;
	GDIcurrentPenInsideFrame	= GDIpenInsideFrame;
	GDIcurrentPenEndCaps		= GDIpenEndCaps;
	GDIcurrentPenStyle			= GDIpenStyle;
	GDIcurrentPenJoin			= GDIpenJoin;
	GDIpenMode	   				= GDIpenMode_color;
}

void GDI_clearPen()
{
	if (GDInewPen) {
		if (GDIpenMode  != GDIpenMode_clear) {
			SelectObject(hdcGDI, GDIoldPen);
			DeleteObject(GDInewPen);

			GDInewPen = CreatePen(PS_NULL, 0, 0);
			GDIoldPen = (HPEN)SelectObject(hdcGDI, GDInewPen);
		}
	} else {
		GDInewPen = CreatePen(PS_NULL, 0, 0);
		GDIoldPen = (HPEN)SelectObject(hdcGDI, GDInewPen);
	}

	GDIpenMode	   = GDIpenMode_color;
}

void GDI_destroyPen()
{
	if (GDInewPen) {
		SelectObject(hdcGDI, GDIoldPen);
		DeleteObject(GDInewPen);
		GDInewPen = NULL;
	}
}

//---------------------------------------------------------------------------
// Brush state

HBRUSH GDIoldBrush = NULL;
HBRUSH GDInewBrush = NULL;
UINT GDInewBrushColor = 0;
LOGBRUSH GDIlogBrush;

#define GDIbrushMode_unassigned	0
#define GDIbrushMode_clear		1
#define GDIbrushMode_color		2
BYTE GDIbrushMode = GDIbrushMode_unassigned;

void GDI_setBrush(UINT color)
{
	if (GDInewBrush) {
		if (GDIbrushMode	!= GDIbrushMode_color
		 || color			!= GDInewBrushColor) {
			SelectObject(hdcGDI, GDIoldBrush);
			DeleteObject(GDInewBrush);

			GDInewBrush = CreateSolidBrush(color);
			GDIoldBrush = (HBRUSH)SelectObject(hdcGDI, GDInewBrush);
		}
	} else {
		GDInewBrush = CreateSolidBrush(color);
		GDIoldBrush = (HBRUSH)SelectObject(hdcGDI, GDInewBrush);
	}

	GDInewBrushColor = color;
	GDIbrushMode	 = GDIbrushMode_color;
}

void GDI_clearBrush()
{
	if (GDInewBrush) {
		if (GDIbrushMode != GDIbrushMode_clear) {
			SelectObject(hdcGDI, GDIoldBrush);
			DeleteObject(GDInewBrush);

			GDIlogBrush.lbStyle = BS_NULL;
			GDInewBrush = CreateBrushIndirect(&GDIlogBrush);
			GDIoldBrush = (HBRUSH)SelectObject(hdcGDI, GDInewBrush);
		}
	} else {
		GDIlogBrush.lbStyle = BS_NULL;
		GDInewBrush = CreateBrushIndirect(&GDIlogBrush);
		GDIoldBrush = (HBRUSH)SelectObject(hdcGDI, GDInewBrush);
	}

	GDIbrushMode	 = GDIbrushMode_clear;
}

void GDI_destroyBrush()
{
	if (GDInewBrush) {
		SelectObject(hdcGDI, GDIoldBrush);
		DeleteObject(GDInewBrush);
		GDInewBrush = NULL;
	}
}

//---------------------------------------------------------------------------

int setRectFill(int left, int top, int right, int bottom, UINT color)
{
	if ((hbmDIB && hbmGDI) || modeDirect) {
		RECT rect = { left, top, right, bottom };

		if (!modeGDI) {
            modeGDI = true;
            BitBlt(hdcGDI, 0, 0, GDIcanvasWidth, GDIcanvasHeight, hdcDIB, 0, 0, SRCCOPY);
        }
        
		GDI_setBrush(color);

		if (GDIsuperSampling>1){
			rect.left *= GDIsuperSampling;
			rect.top *= GDIsuperSampling;
			rect.right *= GDIsuperSampling;
			rect.bottom *= GDIsuperSampling;
		}
		
        return FillRect(hdcGDI, &rect, GDInewBrush);
    }

	return false;
};

//---------------------------------------------------------------------------

//requires msimg32.lib! Works on Win 98+ and WinNT 2000+
int setRectGradient(int left, int top, int right, int bottom, UINT color_TL, UINT color_BR, int horizontal)
{
	if ((hbmDIB && hbmGDI) || modeDirect) {
		TRIVERTEX vertex[2];
		GRADIENT_RECT gRect;

		vertex[0].x     = left;
		vertex[0].y     = top;
		vertex[0].Red   = GetRValue(color_TL) << 8;
		vertex[0].Green = GetGValue(color_TL) << 8;
		vertex[0].Blue  = GetBValue(color_TL) << 8;
		vertex[0].Alpha = 0x0000;

		vertex[1].x     = right;
		vertex[1].y     = bottom; 
		vertex[1].Red   = GetRValue(color_BR) << 8;
		vertex[1].Green = GetGValue(color_BR) << 8;
		vertex[1].Blue  = GetBValue(color_BR) << 8;
		vertex[1].Alpha = 0x0000;

		gRect.UpperLeft  = 0;
		gRect.LowerRight = 1;

		// load gradientfill function