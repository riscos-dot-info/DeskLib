/*
    ####             #    #     # #
    #   #            #    #       #          The FreeWare C library for
    #   #  ##   ###  #  # #     # ###             RISC OS machines
    #   # #  # #     # #  #     # #  #   ___________________________________
    #   # ####  ###  ##   #     # #  #
    #   # #        # # #  #     # #  #    Please refer to the accompanying
    ####   ### ####  #  # ##### # ###    documentation for conditions of use
    ________________________________________________________________________

    File:    Debug.c.Debug
    Author:  Julian Smith
    Version: 0.00 (10 Jul 1995)
    Purpose: Core Debug things. This file's object file is LibFile-d into 
             every debugging library.
*/

#include <stdlib.h>

#include "DeskLib:Debug.h"

int	debug_level = 0;
/*
Programs can set this to 0-5, and Debug1_Printf statements will
come into effect.
*/


int	Debug__Dummyf( const char *format, ...)
{
UNUSED( format);
return 0;
}





void	Debug__Assert( const char *expression, char *sourcefile, int line)
{
Debug_Printf( 
	"Assertion '%s' failed at line %i of file '%s'.\n", 
	expression, line, sourcefile
	);
abort();
}
