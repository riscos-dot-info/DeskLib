/*
    ####             #    #     # #
    #   #            #    #       #          The FreeWare C library for
    #   #  ##   ###  #  # #     # ###             RISC OS machines
    #   # #  # #     # #  #     # #  #   ___________________________________
    #   # ####  ###  ##   #     # #  #
    #   # #        # # #  #     # #  #    Please refer to the accompanying
    ####   ### ####  #  # ##### # ###    documentation for conditions of use
    ________________________________________________________________________

    File:    Debug.c.uniquefile
    Author:  Julian Smith
    Version: 0.00 (04 Jun 1995)
    Purpose: Provides a set of Debug_ function which send stuff to a unique
             file found using tmpnam().
*/


#include <stdarg.h>
#include <stdio.h>

#undef vsprintf

#include "DeskLib:Error.h"
#include "DeskLib:Str.h"
#include "DeskLib:Debug.h"



static char	debug__filename[ 256] = "";
static FILE	*debug__file;
static BOOL	debug__initialised = FALSE;



#define Debug__EnsureInitialised()	if ( !debug__initialised)	Debug_Initialise()



void	Debug_Initialise( void)
{
debug__initialised = TRUE;
tmpnam( debug__filename);
debug__file = fopen( debug__filename, "w");
if ( !debug__file)
	Error_ReportFatal( 1, "Debug_Initialise can't open output file '%s'\n", debug__filename);

if (setvbuf( debug__file, NULL, _IONBF, 0))		/* Turn off buffering	*/
	Error_Report( 1, error_PLACE "Couldn't turn buffering off for output debug file");
}



int	Debug_Printf( const char *format, ...)
{
va_list	va;
int	i;

Debug__EnsureInitialised();
va_start( va, format);
i = vfprintf( debug__file, format, va);
va_end( va);

return i;
}



void	Debug_Print( const char *text)
{
Debug__EnsureInitialised();
fputs( text, debug__file);
}


