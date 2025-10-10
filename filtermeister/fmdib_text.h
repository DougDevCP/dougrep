///////////////////////////////////////////////////////////////////////////////////
//  File: fmdib_text.h
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

#ifndef FMDIB_TEXT_H
#define FMDIB_TEXT_H

HFONT GDInewFont = NULL;
HFONT GDIoldFont = NULL;

int GDIfontAngle = 0;
int GDIfontSize = 8;

//---------------------------------------------------------------------------

static void GDI_destroyFont()
{
	if (GDInewFont) {
		SelectObject(hdcGDI, GDIoldFont);
		DeleteObject(GDInewFont);
		GDInewFont = NULL;
	}
}

//---------------------------------------------------------------------------

HFONT setFont(int size, int angle, int bold, int italic, char * fontname)
{
	if (hbmGDI || modeDirect) {
		SetBkMode(hdcGDI, TRANSPARENT);
		SetTextAlign(hdcGDI, TA_CENTER);

		GDI_destroyFont();

		// 2008-11-13 mwvdlee: Escapement and orientation should be set to same value.
		GDInewFont = CreateFont(size, 0, angle, angle, (bold? 700 : 400), italic,
							false, false,
							ANSI_CHARSET, OUT_DEFAULT_PRECIS,
							CLIP_DEFAULT_PRECIS, ANTIALIASED_QUALITY, DEFAULT_PITCH,
							(LPCTSTR)fontname);
		GDIoldFont = (HFONT)SelectObject(hdcGDI, GDInewFont);
		GDIfontSize = size;
		GDIfontAngle = angle;

		return GDInewFont; //true;
	}

    return false;
}

//---------------------------------------------------------------------------

int setText(int x, int y, UINT color, int alignment, char * text)
{
 if ((hbmDIB && hbmGDI) || modeDirect) {
  double d, m;
  char * start, * end;
 
  if (!modeGDI) {
   modeGDI = true;
   BitBlt(hdcGDI, 0, 0, GDIcanvasWidth, GDIcanvasHeight, hdcDIB, 0, 0, SRCCOPY);
  }
 
  SetTextColor(hdcGDI, color);

  if (alignment == -1) alignment = TA_CENTER | TA_BOTTOM | TA_NOUPDATECP;
  SetTextAlign(hdcGDI, alignment);

  d = (double)GDIfontAngle * M_PI / 1800.;
  m = 0.;
 
  start = (char *)text;
  end = start;
 
  while (*end != 0) {
   m += (double)GDIfontSize;
 
   end = start;
   while (*end != 0 && *end != '\n' && *end != '\r') {
    ++end;
   }
 
   TextOut(hdcGDI, x + (int)(fr2x(d, m)), y + (int)(fr2y(d, m)), start, (int)(end - start));
 
   if (*end == '\r' && *(end + 1) == '\n') {
    ++end;
   }
   start = end + 1;
  }
 
  return true;
 }
 
 return false;
};


//Does not work perfectly yet !!!
int setTextRect(UINT color, int alignment, char * text, RECT rect)
{
 if ((hbmDIB && hbmGDI) || modeDirect) {
 
  if (!modeGDI) {
   modeGDI = true;
   BitBlt(hdcGDI, 0, 0, GDIcanvasWidth, GDIcanvasHeight, hdcDIB, 0, 0, SRCCOPY);
  }
 
  SetTextColor(hdcGDI, color);

  if (alignment == -1) alignment = 0; //DT_VCENTER;
  SetBkMode(hdcGDI,TRANSPARENT);

  DrawText(hdcGDI, text, (int)strlen(text), &rect, alignment);
  
  return true;
 }
 
 return false;
};


int drawText (int x, int y, char* text){
    
    //return setText(x, y, RGB(0,0,0), TA_CENTER, text);
    
    if ((hbmDIB && hbmGDI) || modeDirect) {
        
        if (!modeGDI) {
            modeGDI = true;
            BitBlt(hdcGDI, 0, 0, GDIcanvasWidth, GDIcanvasHeight, hdcDIB, 0, 0, SRCCOPY);
        }

        //SetTextColor(hdcGDI, color);
        SetBkMode(hdcGDI, TRANSPARENT);

        SetTextAlign(hdcGDI, TA_CENTER);

        TextOut(hdcGDI, x, y, (LPCTSTR)text, (int)strlen ((LPCTSTR)text) );

        return true;
     }
     
     return false;
}

 
//---------------------------------------------------------------------------
 
int setTextv(int x, int y, UINT color, int alignment, char * lpFmt, ...)
{
	char szBuffer[1024];
	va_list ap;
	// va_start(ap, (LPCTSTR)lpFmt);
	va_start(ap, lpFmt);
	_vsnprintf(szBuffer, sizeof(szBuffer), (LPCTSTR)lpFmt, ap);
	szBuffer[sizeof(szBuffer) - 1] = 0;   //ensure NUL-terminated
	va_end(ap);
 
#ifdef BCB
	return setText(x, y, color, alignment, (int)szBuffer);
#else
	return setText(x, y, color, alignment, (char *)formatString(szBuffer));
#endif
} /* setTextv */

//---------------------------------------------------------------------------

#endif
// FMDIB_TEXT_H