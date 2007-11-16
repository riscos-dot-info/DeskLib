/*
    ####             #    #     # #
    #   #            #    #       #          The FreeWare C library for
    #   #  ##   ###  #  # #     # ###             RISC OS machines
    #   # #  # #     # #  #     # #  #   ___________________________________
    #   # ####  ###  ##   #     # #  #
    #   # #        # # #  #     # #  #    Please refer to the accompanying
    ####   ### ####  #  # ##### # ###    documentation for conditions of use
    ________________________________________________________________________

    File:    WAssert.h
    Author:  Copyright © 1993 Mark H. Wilkinson
    Version: 1.00 (05 Sep 1993)
             1.01 (25 Oct 2007) Depreciated
    Purpose: Wimp version of ANSI assert() system, based on RISCOS_Lib
             version presented in Archive magazine 6.12.53 by Remo Biagioni.
*/

/*
   NOTE: This module is deprecated. Use the Debug Module instead
*/


#include <stdlib.h>
#include <string.h>

#include "DeskLib:Event.h"
#include "DeskLib:WimpSWIs.h"
#include "DeskLib:WAssert.h"

/*
   This prototype is here to keep cross-compile happy. The Wassert.h file has
   been moved to the oldinclude directory, which is OK for the RISC OS side
   as that is put on DeskLib$Path. However, for GCCSDK DESKLIB_PATH only
   points to the include directory, so it doesn't find the header file.
*/
#ifndef __dl_wassert_h
extern void __wassert(const char *);
#endif


void __wassert(const char *message)
{
	os_error error;
	error.errnum = 0;
	strncpy(error.errmess, message, sizeof(error.errmess));
	Wimp_ReportError(&error, 1, event_taskname);
	exit(1);
}
