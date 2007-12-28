/*
 * This file is part of DeskLib, the C library for RISC OS.
 * Please see accompanying documentation for terms of use.
 *
 *       http://www.riscos.info/index.php/DeskLib
 *
 *
 * Module:  Debug
 * File:    reporter.c
 * Author:  Adam Richardson
 * Purpose: Send debug output to Martin Avison's "Reporter"
 *
 * Version History
 * 18/08/2007: Creation
 *
 */

#include "DeskLib:Debug.h"
#include "DeskLib:SWI.h"
#include "DeskLib:Environment.h"

#include "DebugDefs.h"

extern dl_debug_type dl_debug__libraryinuse; /* Set up in Debug.c */

void dl_Debug_InitialiseReporter(void)
{
  if (!dl_Environment_TaskIsActive("Reporter"))
    /* Reporter is not active, so default to stderr */
    dl_debug__libraryinuse = dl_Debug_STDERR;
}

void dl_Debug_PrintReporter(const char *text)
{
  SWI(1 ,0 , SWI_Reporter_Text0 | XOS_Bit, text);
}


