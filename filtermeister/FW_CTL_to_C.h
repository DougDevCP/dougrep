///////////////////////////////////////////////////////////////////////////////////
//  File: FW_CTL_to_C.h
//
//  Header file of FilterMeisterVS which translates FilterMeister language 
//  expressions for creating controls to C/C++
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

#include "AfhFMcontext.h"

#ifdef __cplusplus
#define EXTERNC extern "C"
#else
#define EXTERNC
#endif


#define CtlDef int CtlDef (FMcontext * const fmc)
//#define AboutMessage EXTERNC char * AboutMessage(void) // Commented out by Kohan 2020


#define need_tbuf fmc->need_tbuf
#define need_t2buf fmc->need_t2buf
#define need_t3buf fmc->need_t3buf
#define need_t4buf fmc->need_t4buf


#define dialogsize(a,b) setDialogPos(0,-1,-1,a,b);
//#define DragBackground 
#define notitlebar clearDialogStyle(WS_CAPTION);
#define PosClientCENTER setDialogPos(0,-1,-1,-1,-1);
#define noctlscaling lockCtlScaling(true);
#define ctlscaling lockCtlScaling(false);
#define dialogtitle(a) setDialogTitle(a);
#define dialogtheme(a) setDialogTheme(a);
#define dialogimage(a) setDialogImage(a);
#define dialogimagemode(a,b) setDialogImageMode(a,b);
#define tiled setDialogImageMode(DIM_TILED,0);
#define dragbackground setDialogDragMode(1);
#define dialogminmax(a,b,c,d) setDialogMinMax(a,b,c,d);
#define maxbox setDialogStyle(WS_MAXIMIZEBOX);
#define minbox setDialogStyle(WS_MINIMIZEBOX);
#define systemmenu setDialogStyle(WS_SYSMENU);
#define nosystemmenu clearDialogStyle(WS_SYSMENU);
//#define keyevent setDialogEvent(4);
#define sizegrip setDialogSizeGrip(1);

#define color_dialog gParams->gDialogGradientColor1


#undef RGB
#define RGB(a,b,c) fm_RGB(a,b,c)

#undef ctl  // Added by Kohan 2020, to fix conflict with FW_FFP_to_C
#define ctl(a) fmc->n = a;

#define gamma(a) setCtlGamma(fmc->n, a); 

#define bitmap() createCtl(fmc->n,CC_BITMAP,"", -1, -1, -1, -1, SS_BITMAP, 0, 0, 3);
#define imagectl() createCtl(fmc->n,CC_IMAGE,"", -1, -1, -1, -1, SS_OWNERDRAW, 0, 0, 3);
#define combobox() createCtl(fmc->n, CC_COMBOBOX,"", -1, -1, -1, -1, CBS_DROPDOWNLIST, 0, 0, 3);
#define comboboxscroll() createCtl(fmc->n, CC_COMBOBOX,"", -1, -1, -1, -1, CBS_DROPDOWNLIST|WS_VSCROLL, 0, 0, 3);
//#define combobox2(a) createCtl(fmc->n, CC_COMBOBOX,a, -1, -1, -1, -1, CBS_DROPDOWNLIST, 0, 0, 3);
#define statictext() createCtl(fmc->n, CC_STATICTEXT,"", -1, -1, -1, -1, SS_LEFT, 0, 0, 3);
#define pushbutton() createCtl(fmc->n, CC_PUSHBUTTON,"", -1, -1, -1, -1, BS_PUSHBUTTON, 0, 0, 3);
#define groupbox() createCtl(fmc->n, CC_GROUPBOX,"", -1, -1, -1, -1, BS_GROUPBOX, 0, 0, 3);
#define checkbox() createCtl(fmc->n, CC_CHECKBOX,"", -1, -1, -1, -1, BS_AUTOCHECKBOX, 0, 0, 3);
#define standard() createCtl(fmc->n, CC_STANDARD,"", -1, -1, -1, -1, 0, 0, 0, 3);
#define standardvert() createCtl(fmc->n, CC_STANDARD,"", -1, -1, -1, -1, 1, 0, 0, 3);
#define ownerdraw() createCtl(fmc->n, CC_OWNERDRAW,"", -1, -1, -1, -1, BS_OWNERDRAW, 0, 0, 3);
#define frame() createCtl(fmc->n, CC_FRAME,"", -1, -1, -1, -1, SS_ETCHEDFRAME, 0, 0, 3); //SS_BLACKFRAME
#define slider() createCtl(fmc->n, CC_SLIDER,"", -1, -1, -1, -1, 0, 0, 0, 3);
#define tabctl() createCtl(fmc->n, CC_TAB,"", -1, -1, -1, -1, 0, 0, 0, 3);
#define toolbar() createCtl(fmc->n, CC_TOOLBAR,"", -1, -1, -1, -1, 0, 0, 0, 3);
#define radiobutton() createCtl(fmc->n, CC_RADIOBUTTON,"", -1, -1, -1, -1, BS_AUTORADIOBUTTON, 0, 0, 3);
#define listbox() createCtl(fmc->n,CC_LISTBOX,"", -1, -1, -1, -1, WS_HSCROLL|WS_VSCROLL, 0, 0, 3);
//#define listboxcol() createCtl(fmc->n,CC_LISTBOX,"", -1, -1, -1, -1, LBS_MULTICOLUMN, 0, 0, 3);
#define listview() createCtl(fmc->n,CC_LISTVIEW,"", -1, -1, -1, -1, WS_HSCROLL|WS_VSCROLL|LVS_SINGLESEL, 0, 0, 3);
#define listview_multi() createCtl(fmc->n,CC_LISTVIEW,"", -1, -1, -1, -1, WS_HSCROLL|WS_VSCROLL, 0, 0, 3);
#define listbar() createCtl(fmc->n,CC_LISTBAR,"", -1, -1, -1, -1, 0, 0, 0, 3);
#define scrollbar() createCtl(fmc->n,CC_SCROLLBAR,"", -1, -1, -1, -1, 0, 0, 0, 3);
#define edit() createCtl(fmc->n,CC_EDIT,"", -1, -1, -1, -1, 0, 0, 0, 3);
#define edit_autohscroll() createCtl(fmc->n,CC_EDIT,"", -1, -1, -1, -1, ES_AUTOHSCROLL, 0, 0, 3);
#define edit_multi() createCtl(fmc->n,CC_EDIT,"", -1, -1, -1, -1, ES_MULTILINE, 0, 0, 3);

#define previewcheck() fmc->previewCheckbox = fmc->n;
#define mouseover() setCtlProperties(fmc->n, CTP_MOUSEOVER);
#define mousemove() setCtlProperties(fmc->n, CTP_MOUSEMOVE); 
#define drawitem() setCtlProperties(fmc->n, CTP_DRAWITEM); 
#define previewdrag() setCtlProperties(fmc->n, CTP_PREVIEWDRAG); 
//#define vscroll() setCtlStyle(fmc->n, WS_VSCROLL);
//#define hscroll() setCtlStyle(fmc->n, WS_HSCROLL);
#define notify() setCtlStyle(fmc->n, SS_NOTIFY);
#define usetabstops() setCtlStyle(fmc->n, LBS_USETABSTOPS);
#define columnwidth(a) SendMessage(gParams->ctl[fmc->n].hCtl, LB_SETCOLUMNWIDTH , HDBUsToPixels(a), 0); 
#define thumbsize(a) setCtlThumbSize(fmc->n,a);
#define selrange() setCtlStyle(fmc->n, TBS_ENABLESELRANGE);

#define contextmenu() setCtlProperties(fmc->n, CTP_CONTEXTMENU);
#define staticedge() setCtlStyleEx(fmc->n, WS_EX_STATICEDGE);
#define modalframe() setCtlStyleEx(fmc->n, WS_EX_DLGMODALFRAME);
#define center() setCtlStyle(fmc->n, SS_CENTER);
#define clientedge() setCtlStyleEx(fmc->n, WS_EX_CLIENTEDGE);
#define noclientedge() clearCtlStyleEx(fmc->n, WS_EX_CLIENTEDGE);
#define border() setCtlStyle(fmc->n, WS_BORDER);
#define rightalignedtext()  setCtlStyle(fmc->n, SS_RIGHT); //setCtlStyleEx(fmc->n, WS_EX_RIGHT); 
#define leftalignedtext()  setCtlStyle(fmc->n, SS_LEFT); //setCtlStyleEx(fmc->n, WS_EX_RIGHT); 
#define pushlike() setCtlStyle(fmc->n, BS_PUSHLIKE); 
#define etched() setCtlStyle(fmc->n, SS_ETCHEDFRAME);
#define centerimage() setCtlStyle(fmc->n, SS_CENTERIMAGE);
#define dialogcolor(a) setDialogColor(a);
#define multiline() setCtlStyle(fmc->n, BS_MULTILINE);

#define actionpreview() setCtlAction(fmc->n,CA_PREVIEW);
#define actionnone() setCtlAction(fmc->n,CA_NONE);
#define actioncancel() setCtlAction(fmc->n,CA_CANCEL);
#define actionapply() setCtlAction(fmc->n,CA_APPLY);
#define actionabout() setCtlAction(fmc->n,CA_ABOUT);
#define action(a) setCtlAction(fmc->n,a);

#define theme(a) setCtlTheme(fmc->n,a);


#define text(a) setCtlText(fmc->n,a);
#define textw(a) setCtlTextW(fmc->n,a);
#define label(a) setCtlLabel(fmc->n,a);
#define image(a) setCtlImage(fmc->n,a,'B');
#define icon(a,b) setToolIcon(fmc->n,a,b);

#define val(a) setCtlVal(fmc->n,a);
#define divisor(a) setCtlDivisor(fmc->n,a);
#define page(a) setCtlPageSize(fmc->n,a);
#define defval(a) setCtlDefVal(fmc->n,a);
#define pos(a,b) setCtlPos(fmc->n,a,b,-1,-1);
#define editpos(a,b) setCtlEditPos(fmc->n,a,b); // by Ognen Genchev
#define defpos(a,b) setCtlDefPos(fmc->n,a,b,-1,-1);
#define pos2(a,b) setCtlPos(fmc->n,a,b,-1,-1);setCtlDefPos(fmc->n,a,b,-1,-1);
#define size(a,b) setCtlPos(fmc->n,-1,-1,a,b);
#define defsize(a,b) setCtlDefPos(fmc->n,-1,-1,a,b);
#define size2(a,b) setCtlPos(fmc->n,-1,-1,a,b);setCtlDefPos(fmc->n,-1,-1,a,b);
#define pixelsize(a,b) setCtlPixelPos(fmc->n,-1,-1,a,b);
#define color(a) setCtlColor(fmc->n,a);
#define syscolor(a) setCtlSysColor(fmc->n,a);
#define textcolor(a) setCtlFontColor(fmc->n,a);
#define fontcolor(a) setCtlFontColor(fmc->n,a);
#define buddycolor(a) setCtlBuddyColor(fmc->n,a);

#define tooltip(a) setCtlToolTip(fmc->n,a,0);
#define range(a,b) setCtlRange(fmc->n,a,b);
#define tab(a,b) setCtlTab(fmc->n,a,b);
#define deftab(a,b) setCtlDefTab(fmc->n,a,b);
#define tab2(a,b) setCtlTab(fmc->n,a,b);setCtlDefTab(fmc->n,a,b);
#define mate(a) setCtlMate(fmc->n,a);
#define scripting(a) setCtlScripting(fmc->n,a);
#define font(a) setCtlFont(fmc->n,a);

#define order(a) setCtlOrder(fmc->n,a);

#define image(a) setCtlImage(fmc->n,a,'B');
#define anchor(a) setCtlAnchor(fmc->n,a);
#define editsize(a,b) setCtlEditSize(fmc->n,a,b);
#define editstaticedge() clearCtlBuddyStyle(fmc->n,1,WS_BORDER);setCtlBuddyStyleEx (fmc->n,1,WS_EX_STATICEDGE);
#define editclientedge() clearCtlBuddyStyle(fmc->n,1,WS_BORDER);setCtlBuddyStyleEx (fmc->n,1,WS_EX_CLIENTEDGE);
#define maxsize(a,b) setCtlBuddyMaxSize(fmc->n,a,b);

#define invisible() enableCtl(fmc->n,0);
#define disabled() enableCtl(fmc->n,1);

