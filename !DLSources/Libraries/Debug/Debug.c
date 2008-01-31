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

debug_type debug__libraryinuse = debug_UNINITIALISED;

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

void Debug_Initialise(debug_type type)
{
  if (debug__libraryinuse == debug_UNINITIALISED)
  /* Can only initialise once per session */
  {
    debug__libraryinuse = type;

    switch (type)
    {
      case debug_UNINITIALISED:
        /* In case user initialises with "unititialised" value or Debug_Printf
           is called before Debug_Initialise is - the default is stderr */
        debug__libraryinuse = debug_STDERR;
        /* No initialisation needed for stderr */
        break;
      case debug_REPORTER:
        Debug_InitialiseReporter();
        break;
      case debug_PIPETYPE:
        Debug_InitialisePipetype();
        break;
      case debug_STDERR:
        /* No initialisation needed */
        break;
      case debug_UNIQUEFILE:
        Debug_InitialiseUniqueFile();
        break;
      case debug_UNIQUEPIPE:
        Debug_InitialiseUniquePipe();
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
  if (debug__libraryinuse == debug_UNINITIALISED)
    Debug_Initialise(debug_UNINITIALISED);

  switch (debug__libraryinuse)
  {
    case debug_UNINITIALISED:
      /* This case is dealt with above. After calling Debug_Initialise,
         debug__libraryinuse cannot be Debug_UNINITIALISED.
         This case statement is just here to suppress the compiler warning */
      break;
    case debug_REPORTER:
      Debug_PrintReporter(text);
      break;
    case debug_PIPETYPE:
      Debug_PrintPipetype(text);
      break;
    case debug_STDERR:
      Debug_PrintStderr(text);
      break;
    case debug_UNIQUEFILE:
      Debug_PrintUniqueFile(text);
      break;
    case debug_UNIQUEPIPE:
      Debug_PrintUniquePipe(text);
      break;
  }
}

