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
    Author:  Julian Smith, with clever taskwindow code from Paul Field and Cy Booker.
    Version: 0.00 (04 Jun 1995)
    Purpose: Provides a set of Debug_ function which send stuff to a unique
             file in a given directory.
*/


#include <stdarg.h>
#include <stdio.h>

#undef vsprintf

#include "DeskLib:Error.h"
#include "DeskLib:WimpSWIs.h"

#include "DeskLib:Debug.h"

#include "DeskLib:Str.h"

static char	debug_filename[ 256] = "";



#define Debug__EnsureInitialised()	if ( debug_filename[0]==0)	Debug_Initialise()

void	Debug_Initialise( void)
/* Simply redirect stderr to a unique file in pipe:, and	*/
/* open a taskwindow which will *Type out stderr.		*/
/* Aren't taskwindows wonderful?				*/
{
char	command[ 320];
sprintf( debug_filename, "Pipe:$.DeskLib.%s", LeafName( tmpnam( NULL)));
freopen( debug_filename, "w", stderr);
setbuf( stderr, NULL);
sprintf( command, 
	"taskwindow \"type %s\" -wimpslot 16k -name \"Debug output\" -quit", 
	debug_filename
	);

Error_CheckFatal(
	Wimp_StartTask( command)
	);
}



int	Debug_Printf( const char *format, ...)
{
va_list	va;
int	i;
Debug__EnsureInitialised();
va_start(va, format);
i = vfprintf( stderr, format, va);
va_end(va);

return i;
}



void	Debug_Print( const char *text)
{
Debug__EnsureInitialised();
fputs( text, stderr);
}


