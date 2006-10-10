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
    Purpose: Provides a set of Debug_ function which send stuff to a unique
             file in a given directory.
*/


#include <stdarg.h>
#include <stdio.h>

#undef vsprintf

#include "DeskLib:Error.h"

#include "DeskLib:Debug.h"

#include "DeskLib:Str.h"

static char	debug_filename[ 256] = "";





void	Debug_Initialise( void)
{
sprintf( debug_filename, "Pipe:$.DeskLib.%s", LeafName( tmpnam( NULL)));
}


static FILE	*Debug__OpenPipeFile( void)
{
FILE	*file;
if ( debug_filename[ 0] == 0)	Debug_Initialise();
file = fopen( debug_filename, "a");
if ( !file)	Error_ReportFatal( 0, "Can't open debugging pipe file '%s'");
return file;
}

#define Debug__ClosePipeFile( f)	fclose( f)



int	Debug_Printf( const char *format, ...)
{
va_list	va;
int	i;
FILE	*f;

f = Debug__OpenPipeFile();

va_start(va, format);
i = vfprintf( f, format, va);
va_end(va);

Debug__ClosePipeFile( f);
return i;
}



void	Debug_Print( const char *text)
{
FILE	*f;
f = Debug__OpenPipeFile();
fputs( text, f);
Debug__ClosePipeFile( f);
}


