///////////////////////////////////////////////////////////////////////////////////
//  File: fmdib_antialias.h
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

#ifndef FMDIB_ANTIALIAS_H
#define FMDIB_ANTIALIAS_H

//---------------------------------------------------------------------------

int setThinLineAA(int start_x, int start_y, int end_x, int end_y, UINT color) {
	double xx, yy, slope, alfa;
	int x, y;
 
	int left = min(start_x, end_x), right = max(start_x, end_x);
	int top = min(start_y, end_y), bottom = max(start_y, end_y);
 
	if (modeGDI) {
		BitBlt(hdcDIB, 0, 0, GDIcanvasWidth, GDIcanvasHeight, hdcGDI, 0, 0, SRCCOPY);
		GdiFlush();
		modeGDI = false;
	}
 
	if (left == right && top == bottom) {
		GDI_dibSetPixel(left, top, color);
	} else if (bottom - top > right - left) {
		color = (color >> 16) | (color & 0x0000FF00) | ((color << 16) & 0x00FF0000);
		slope = 1.0 / (float)(end_y - start_y);
		for (y = top; y <= bottom; ++y) {
			xx = slope * ((end_y - y) * start_x + (y - start_y) * end_x);
			x = (int)floor(xx);
			alfa = x + 1 - xx;
			GDI_dibMixPixel_BGR(x, y, color, alfa);
			GDI_dibMixPixel_BGR(x + 1, y, color, 1. - alfa);
		}
	} else {
		color = (color >> 16) | (color & 0x0000FF00) | ((color << 16) & 0x00FF0000); 
		slope = 1.0 / (float)(end_x - start_x);
		for (x = left; x <= right; ++x) {
			yy = slope * ((end_x - x) * start_y + (x - start_x) * end_y);
			y = (int)floor(yy);
			alfa = y + 1 - yy;
			GDI_dibMixPixel_BGR(x, y, color, alfa);
			GDI_dibMixPixel_BGR(x, y + 1, color, 1. - alfa);
		}
	}
 
	return true;
}

//---------------------------------------------------------------------------

int setThinEllipseAA(int start_x, int start_y, int end_x, int end_y, UINT color) {		//NEW!
	int left 	= min(start_x, end_x);
	int right 	= max(start_x, end_x);
	int top 	= min(start_y, end_y);
	int bottom 	= max(start_y, end_y);

	if (modeGDI) {
		BitBlt(hdcDIB, 0, 0, GDIcanvasWidth, GDIcanvasHeight, hdcGDI, 0, 0, SRCCOPY);
		GdiFlush();
		modeGDI = false;
	}

	if (left == right && top == bottom) {
		GDI_dibSetPixel(left, top, color);
	} else {
		int x, y;
		double xx, yy, slope, alfa, delta;
		double aa = .5 * (right - left), bb = .5 * (bottom - top); // semi axes;
		double aa2 = aa * aa, bb2 = bb * bb;
		double xxc = .5 * (left + right), yyc = .5 * (top + bottom); // center coordinates
		double xx45 = aa2 / sqrt(aa2 + bb2), yy45 = bb2 / sqrt(aa2 + bb2);

		color = (color >> 16) | (color & 0x0000FF00) | ((color << 16) & 0x00FF0000);

		if (bb > 0) {
			slope = aa / bb;
			for (y = (int)floor(yyc - yy45); y < yyc + yy45 + 1; ++y) {
				delta = slope * sqrt(bb2 - (y - yyc) * (y - yyc));

				xx = xxc + delta; // right arc
				x = (int)floor(xx);
				alfa = x + 1 - xx;
				GDI_dibMixPixel_BGR(x, y, color, alfa);
				GDI_dibMixPixel_BGR(x + 1, y, color, 1. - alfa);

				xx = xxc - delta; // left arc
				x = (int)floor(xx);
				alfa = x + 1 - xx;
				GDI_dibMixPixel_BGR(x, y, color, alfa);
				GDI_dibMixPixel_BGR(x + 1, y, color, 1. - alfa);
			} // next y
		} // end if bb > 0

		if (aa > 0) {
			slope = bb / aa;
			for (x = (int)floor(xxc - xx45); x < xxc + xx45 + 1; ++x) {
				delta = slope * sqrt(aa2 - (x - xxc) * (x - xxc));

				yy = yyc + delta; // bottom arc
				y = (int)floor(yy);
				alfa = y + 1 - yy;
				GDI_dibMixPixel_BGR(x, y, color, alfa);
				GDI_dibMixPixel_BGR(x, y + 1, color, 1. - alfa);

				yy = yyc - delta; // top arc
				y = (int)floor(yy);
				alfa = y + 1 - yy;
				GDI_dibMixPixel_BGR(x, y, color, alfa);
				GDI_dibMixPixel_BGR(x, y + 1, color, 1. - alfa);
			} // next x
		} // end if aa > 0
	} // end draw ellipse

	return true;
} // end draw antialiased

//---------------------------------------------------------------------------

#endif
// FMDIB_ANTIALIAS_H