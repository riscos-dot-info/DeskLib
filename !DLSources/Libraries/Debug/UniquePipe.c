/*
    ####             #    #     # #
    #   #            #    #       #          The FreeWare C library for
    #   #  ##   ###  #  # #     # ###             RISC OS machines
    #   # #  # #     # #  #     # #  #   ___________________________________
    #   # ####  ###  ##   #     # #  #
    #   # #        # # #  #     # #  #    Please refer to the accompanying
    ####   ### ####  #  # ##### # ###    documentation for conditions of use
    ________________________________________________________________________

    File:    Debug1.c.uniquepipe
    Author:  Julian Smith
    Version: 0.00 (04 Jun 1995)
             0.10 (31 Aug 2007) Altered to work as part of main library
    Purpose: Provides a set of Debug_ function which send stuff to a unique
             file in a given directory.
*/


#include <stdarg.h>
#include <stdio.h>

/*
31/8/07 - not sure why this #undef was here?
#undef vsprintf
*/

#include "DeskLib:Error.h"

#include "DeskLib:Debug.h"

#include "DeskLib:Str.h"

#include "DebugDefs.h"


#define dl_Debug__ClosePipeFile(f) fclose(f)

extern char debug__filename[256]; /* Set up in Debug.c */



void dl_Debug_InitialiseUniquePipe(void)
{
  snprintf(debug__filename, 256, "Pipe:$.DeskLib.%s", LeafName(tmpnam( NULL)));
}


static FILE *dl_Debug__OpenPipeFile(void)
{
  FILE *file;

  file = fopen(debug__filename, "a");
  if (!file) Error_ReportFatal(1, "Can't open debugging pipe file '%s'");

  return file;
}

void dl_Debug_PrintUniquePipe(const char *text)
{
  FILE *f;
  f = dl_Debug__OpenPipeFile();
  fputs(text, f);
  dl_Debug__ClosePipeFile(f);
}


