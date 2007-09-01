/*
 * This file is part of DeskLib, the C library for RISC OS.
 * Please see accompanying documentation for terms of use.
 *
 *       http://www.riscos.info/index.php/DeskLib
 *
 *
 * Module:  Debug
 * File:    DebugDefs.h
 * Author:  Adam Richardson
 * Purpose: Function prototypes for separate debugging methods
 *
 * Version History
 * 18/08/2007: Creation
 *
 */

#ifndef __Debug_Defs_h
#define __Debug_Defs_h

#include <stdio.h>

void Debug__Assert(const char *expression, char *sourcefile, int line);

void dl_Debug_InitialisePipetype(void);
void dl_Debug_PrintPipetype(const char *text);

void dl_Debug_InitialiseReporter(void);
void dl_Debug_PrintReporter(const char *text);

void dl_Debug_PrintStderr(const char *text);

void dl_Debug_InitialiseUniqueFile(void);
void dl_Debug_PrintUniqueFile(const char *text);

void dl_Debug_InitialiseUniquePipe(void);
static FILE *dl_Debug__OpenPipeFile(void);
void dl_Debug_PrintUniquePipe(const char *text);

#endif
