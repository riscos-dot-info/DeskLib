/*
    ####             #    #     # #
    #   #            #    #       #          The FreeWare C library for
    #   #  ##   ###  #  # #     # ###             RISC OS machines
    #   # #  # #     # #  #     # #  #   ___________________________________
    #   # ####  ###  ##   #     # #  #
    #   # #        # # #  #     # #  #    Please refer to the accompanying
    ####   ### ####  #  # ##### # ###    documentation for conditions of use
    ________________________________________________________________________

    File:    Debug1.c.Debug1
    Author:  Julian Smith
    Version: 0.00 (04 Jun 1995)
    Purpose: Provides a set of Debug_ function which send stuff to stderr
*/


#include <stdarg.h>
#include <stdio.h>

#undef vsprintf

#include "DeskLib:Debug.h"


void	Debug_Initialise( void)
{
/* We're only a simple library...	*/
}


int	Debug_Printf( const char *format, ...)
{
va_list	va;
int	i;
va_start(va, format);
i = vfprintf( stderr, format, va);
va_end(va);
return i;
}


void	Debug_Print( const char *text)
{
fputs( text, stderr);
}


