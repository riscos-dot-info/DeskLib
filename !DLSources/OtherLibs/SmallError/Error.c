/*
    ####             #    #     # #
    #   #            #    #       #          The FreeWare C library for
    #   #  ##   ###  #  # #     # ###             RISC OS machines
    #   # #  # #     # #  #     # #  #   ___________________________________
    #   # ####  ###  ##   #     # #  #
    #   # #        # # #  #     # #  #    Please refer to the accompanying
    ####   ### ####  #  # ##### # ###    documentation for conditions of use
    ________________________________________________________________________

    File:    SmError.Error.c
    Author:  Julian Smith
    Version: 0.00 (22 May 1995)
    Purpose: Centralised error handling functions
             This outputs error messages with fprintf( stderr), for use
             with non-desktop programs.

*/


#include <stdarg.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>

#include "DeskLib:Error.h"
#include "DeskLib:Event.h"



extern void Error_Report( int errornum, char *report, ...)
{
va_list	va;

UNUSED( errornum);

fprintf( stderr, "\nError from '%s':\n", event_taskname);

va_start( va, report);
vfprintf( stderr, report, va);
va_end( va);

}



extern void Error_ReportFatal( int errornum, char *report, ...)
{
va_list	va;
char	errmess[256];

va_start( va, report);
vsprintf( errmess, report, va);
va_end( va);

Error_Report( 
	errornum,
	"%s has suffered a fatal internal error (%s). Must quit immediately",
	event_taskname,	errmess
	);
exit(1);
}



extern void Error_ReportInternal( int errornum, char *report, ...)
{
va_list	va;
char	errmess[256];

va_start( va, report);
vsprintf( errmess, report, va);
va_end( va);

Error_Report( errornum, errmess);
}




extern void Error_ReportFatalInternal( int errornum, char *report, ...)
{
va_list	va;
char	errmess[256];

va_start( va, report);
vsprintf( errmess, report, va);
va_end( va);

Error_ReportFatal( errornum, errmess);
}




extern BOOL Error_Check( os_error *error)
{
if ( error != NULL)	{
	Error_Report( error->errnum, error->errmess);
	return TRUE;
	}

return FALSE;
}



extern void Error_CheckFatal(os_error *error)
{
if (error != NULL)	Error_ReportFatal( error->errnum, error->errmess);
}



extern BOOL Error_OutOfMemory( BOOL fatal, char *place)
{
if ( fatal)	Error_ReportFatal( 0, "Unable to get enough memory for the %s", place);

else		Error_Report( 0, "Unable to get enough memory for the %s", place);

return FALSE;
/*
Always returns FALSE so can return FALSE from your
own function at the same time as reporting the error
*/
}
