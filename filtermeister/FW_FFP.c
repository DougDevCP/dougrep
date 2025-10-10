///////////////////////////////////////////////////////////////////////////////////
//  File: FW_FFP.c
//
//  Source file of FilterMeisterVS which links image processing code with the 
//  FilterMeister framework
//
//  Include this file in the build process to compile your FFP source code as C
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

#include "PITypes.h"
#include "AfhFM.h"
#include "AfhFMcontext.h"
#include "FW_FFP.h"
#include "resource.h"
#include <stdio.h>
#include <windows.h>
#include <time.h>
#include <direct.h>
#include <math.h>



/****************************************************************************/
// Exception handling routines
/****************************************************************************/

#ifdef NDEBUG

static int show_reg(const char *regname, int regval)
{
    return msgBox(MB_ICONINFORMATION|MB_OKCANCEL,
               "Register Contents", //FilterMeister 
               "%s = %#8.8X", regname, regval);
} /*show_reg*/

static int show_sreg(const char *regname, int regval)
{
    return msgBox(MB_ICONINFORMATION|MB_OKCANCEL,
               "Segment Register Contents", //FilterMeister 
               "%s = %#4.4X", regname, (WORD)regval);
} /*show_sreg*/



static int Eval_Exception(LPEXCEPTION_POINTERS ep)
{
    int n_except;
    char p[256] = "";
    int reply;
    //FLOATING_SAVE_AREA env387;  //don't actually need the Cr0NpxState field, but it's harmless

    static const char *tag_type[] = {"valid", "zero", "special", "EMPTY"};
    static const char *precision_control[] = {"24 bits", "RESERVED", "53 bits", "64 bits"};
    static const char *rounding_control[] = {"round to nearest", "round down", "round up", "chop toward 0"};

    n_except = ep->ExceptionRecord->ExceptionCode;

#if 1 //#if 1
    if ( n_except != STATUS_INTEGER_DIVIDE_BY_ZERO && 
         n_except != STATUS_INTEGER_OVERFLOW &&
         n_except != STATUS_ACCESS_VIOLATION &&
         n_except != STATUS_ILLEGAL_INSTRUCTION &&
         n_except != STATUS_ARRAY_BOUNDS_EXCEEDED &&
         n_except != STATUS_PRIVILEGED_INSTRUCTION &&
         n_except != STATUS_STACK_OVERFLOW &&
         n_except != STATUS_CONTROL_C_EXIT &&
         //n_except != STATUS_BREAKPOINT &&
         //n_except != STATUS_SINGLE_STEP &&
         n_except != STATUS_FLOAT_INVALID_OPERATION &&
         n_except != STATUS_FLOAT_DENORMAL_OPERAND &&
         n_except != STATUS_FLOAT_DIVIDE_BY_ZERO &&
         n_except != STATUS_FLOAT_INEXACT_RESULT &&
         n_except != STATUS_FLOAT_STACK_CHECK &&
         n_except != STATUS_FLOAT_UNDERFLOW &&
         n_except != STATUS_FLOAT_OVERFLOW ){    // Pass on most
         
            return EXCEPTION_CONTINUE_SEARCH;            //  exceptions
        }
#endif

    // abort() raises STATUS_CONTROL_C_EXIT...
    if (n_except == STATUS_CONTROL_C_EXIT)
        return EXCEPTION_EXECUTE_HANDLER;

    // save NDP context and mask exceptions (via implicit finit) so we don't recur
    // if this is a f.p. exception...
    //__asm {
    //    fnsave env387
    //}


    switch (n_except) {
    case STATUS_INTEGER_DIVIDE_BY_ZERO:
        strcpy_s(p,256,"Integer (zero-)divide error!");
        break;
    case STATUS_INTEGER_OVERFLOW:
        strcpy_s(p,256,"Integer overflow!");
        break;
    case STATUS_ACCESS_VIOLATION:
        strcpy_s(p,256,"Memory access violation!");
        break;
    case STATUS_ILLEGAL_INSTRUCTION:
        strcpy_s(p,256,"Illegal instruction!");
        break;
    case STATUS_ARRAY_BOUNDS_EXCEEDED:
        strcpy_s(p,256,"Array bounds exceeded!");
        break;
    case STATUS_PRIVILEGED_INSTRUCTION:
        strcpy_s(p,256,"Privileged instruction!");
        break;
    case STATUS_STACK_OVERFLOW:
        strcpy_s(p,256,"Stack overflow!");
        break;
    case STATUS_CONTROL_C_EXIT:
        strcpy_s(p,256,"Control-C exit!");
        break;
    case STATUS_BREAKPOINT:
        strcpy_s(p,256,"Breakpoint trap!");
        break;
    case STATUS_SINGLE_STEP:
        strcpy_s(p,256,"Single step trap or debug fault!");
        break;
    case STATUS_FLOAT_INVALID_OPERATION:
        strcpy_s(p,256,"Floating-point invalid operation!");
        break;
    case STATUS_FLOAT_DENORMAL_OPERAND:
        strcpy_s(p,256,"Floating-point denormal operand!");
        break;
    case STATUS_FLOAT_DIVIDE_BY_ZERO:
        strcpy_s(p,256,"Floating-point divide by zero!");
        break;
    case STATUS_FLOAT_INEXACT_RESULT:
        strcpy_s(p,256,"Floating-point inexact result!");
        break;
    case STATUS_FLOAT_STACK_CHECK:
        // FP stack check: C1 = 0 ==> overflow, else underflow
        //if (env387.StatusWord & 0x0200)
            strcpy_s(p,256,"Floating-point stack overflow!");
        //else
        //    strcpy_s(p,256,"Floating-point stack underflow!";
        break;
    case STATUS_FLOAT_UNDERFLOW:
        strcpy_s(p,256,"Floating-point underflow!");
        break;
    case STATUS_FLOAT_OVERFLOW:
        strcpy_s(p,256,"Floating-point overflow!");
        break;
    default:
        strcpy_s(p,256,"Unrecognized exception.");
        break;
    }


    reply = msgBox(MB_ICONERROR|MB_OK,
                   "Error Message",
                   "%s\n\n"
                   "",
                   p);

    return EXCEPTION_EXECUTE_HANDLER;

} /*Eval_Exception*/

#endif //#ifdef NDEBUG



int call_FFP(FFP_HANDLER h, INT_PTR p1) //int X86_call_fmf1(BYTE *code, int p1)
{

#ifdef NDEBUG
    __try {
#endif

        switch(h){
            case FFP_CtlDef:
                return CtlDef((FMcontext * const)p1);
            case FFP_OnCtl:
                return OnCtl((FMcontext * const)p1, gFmcp->n, gFmcp->e, gFmcp->previous);
#if ONWINDOWSTART
            case FFP_OnWindowStart:
                return OnWindowStart((FMcontext * const)p1);
#endif
            case FFP_OnFilterStart:
                return OnFilterStart((FMcontext * const)p1);
            case FFP_ForEveryTile:
                return ForEveryTile((FMcontext * const)p1);
            //case FFP_ForEveryRow:
            //    return ForEveryRow((FMcontext * const)p1);
            case FFP_OnFilterEnd:
                return OnFilterEnd((FMcontext * const)p1);
        }

#ifdef NDEBUG
    }
    __except (Eval_Exception(GetExceptionInformation())) { //Eval_Exception(GetExceptionInformation())
        if (!gFmcp->doingProxy) {
            //if running main image, set gResult to abort ALL tiles...            
            gResult = userCanceledErr;
            //ErrorOk("An error occured, so the rendering was aborted.");
            //return userCanceledErr;
        }
        
        //ErrorOk("An error occured, but you can continue now.");
        return userCanceledErr;
    }
#endif

    return true; //((fmf1)code)(p1);

} /*X86_call_fmf1*/



#include "../../_FilterCodeFile.h"
