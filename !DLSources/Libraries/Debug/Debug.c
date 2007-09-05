/*
 * This file is part of DeskLib, the C library for RISC OS.
 * Please see accompanying documentation for terms of use.
 *
 *       http://www.riscos.info/index.php/DeskLib
 *
 *
 * Module:  Debug
 * File:    Debug.c
 * Author:  Julian Smith
 * Purpose: Debugging functions
 *
 * Version History
 * 10/07/1995: v0.00
 * 31/08/2007: Altered debug system to be part of main library
 *             Debug_Initialise now takes a parameter to determine
 *             what action the debug functions take.
 *
 */

#include <stdlib.h>
#include <stdarg.h>

#define _DeskLib_Debug_BUILD

#include "DebugDefs.h"
#include "DeskLib:Debug.h"


int debug_level = 0;
/*
  Programs can set this to 0-5, and Debug1_Printf statements will come into effect.
*/

dl_debug_type dl_debug__libraryinuse = dl_Debug_UNINITIALISED;

char debug__filename[256] = "";

int Debug__Dummyf( const char *format, ...)
{
  UNUSED( format);
  return 0;
}

void Debug__Assert( const char *expression, char *sourcefile, int line)
{
  Debug_Printf("Assertion '%s' failed at line %i of file '%s'.\n",
                expression, line, sourcefile);
  abort();
}

void Debug_Initialise(dl_debug_type type)
{
  if (dl_debug__libraryinuse == dl_Debug_UNINITIALISED)
  /* Can only initialise once per session */
  {
    dl_debug__libraryinuse = type;

    switch (type)
    {
      case dl_Debug_UNINITIALISED:
        /* In case user initialises with "unititialised" value or Debug_Printf
           is called before dl_Debug_Initialise is - the default is stderr */
        dl_debug__libraryinuse = dl_Debug_STDERR;
        /* No initialisation needed for stderr */
        break;
      case dl_Debug_REPORTER:
        dl_Debug_InitialiseReporter();
        break;
      case dl_Debug_PIPETYPE:
        dl_Debug_InitialisePipetype();
        break;
      case dl_Debug_STDERR:
        /* No initialisation needed */
        break;
      case dl_Debug_UNIQUEFILE:
        dl_Debug_InitialiseUniqueFile();
        break;
      case dl_Debug_UNIQUEPIPE:
        dl_Debug_InitialiseUniquePipe();
        break;
    }
  }
}

int Debug_Printf(const char *format, ...)
{
  char buffer[1024];
  va_list argptr;
  int i;

  va_start(argptr, format);
  i = vsnprintf(buffer, sizeof(buffer), format, argptr);
  va_end(argptr);

  /* If the output string runs over the end of the buffer, it will be
     truncated but not null-terminated, so have to account for that: */
  buffer[sizeof(buffer)-1] = '\0';

  Debug_Print(buffer);

  return i;
}

void Debug_Print(const char *text)
{
  /* Check we've been initialised */
  if (dl_debug__libraryinuse == dl_Debug_UNINITIALISED)
    Debug_Initialise(dl_Debug_UNINITIALISED);

  switch (dl_debug__libraryinuse)
  {
    case dl_Debug_UNINITIALISED:
      /* This case is dealt with above. After calling Debug_Initialise,
         dl_debug__libraryinuse cannot be dl_Debug_UNINITIALISED.
         This case statement is just here to suppress the compiler warning */
      break;
    case dl_Debug_REPORTER:
      dl_Debug_PrintReporter(text);
      break;
    case dl_Debug_PIPETYPE:
     dl_Debug_PrintPipetype(text);
      break;
    case dl_Debug_STDERR:
     dl_Debug_PrintStderr(text);
      break;
    case dl_Debug_UNIQUEFILE:
     dl_Debug_PrintUniqueFile(text);
      break;
    case dl_Debug_UNIQUEPIPE:
    dl_Debug_PrintUniquePipe(text);
      break;
  }
}

