/*

This is some code from my Shell library which uses DeskLib's Print
library to print bits of windows.

It *won't* compile because it uses things from Shell.

However, you should be able to get an idea for how to use the Print_
library from this.

Hopefully, a working example of using the Print library will be included
sometime.

- Julian Smith 17 May 1995

*/


#include <string.h>

#include "DeskLib:Event.h"
#include "DeskLib:WimpSWIs.h"
#include "DeskLib:Hourglass.h"
#include "DeskLib:PDriver.h"
#include "DeskLib:Error.h"
#include "DeskLib:Kbd.h"

#include "Shell.Shell.h"
#include "Shell.FindWind.h"
#include "Shell.Redraw2.h"

#include "DeskLib:Print.h"



static void	ErrorCheck( print_block *print, os_error *error)
	/* Errors occuring during printing have to be dealt with	*/
	/* rather carefully - PDriver_AbortJob *must* be called before	*/
	/* any error is reported, as otherwise the error text will be	*/
	/* intercepted by the printer driver.				*/
{
if ( !error)	return;
PDriver_AbortJob( print->job);
/*Error_Report( 0, Error_PLACE);*/
Error_ReportFatal( error->errnum, error->errmess);
}




static BOOL	Shell__RectPrintFn( print_block *print)
{
print_transformation	matrix;
wimp_point		position;
wimp_rect		rect;
int			more = 0, n;

Shell_convertpoint	convert;
Shell_rectblock		*shellrect = (Shell_rectblock *) print->reference;
Shell_windblock		*shellwind = Shell_FindWindBlock( shellrect->window);

matrix.xx	= 1<<16;	/* This is an identity matrix.	*/
matrix.xy	= 0;
matrix.yy	= 1<<16;
matrix.yx	= 0;

position.x	= 72000*1;	/* This is where bottom-left corner of our rect will appear on	*/
position.y	= 72000*1;	/* the page, relative to bottom-left of page, in millipoints.	*/
				/* Should really find size of page and borders etc.		*/
rect = shellrect->rect;

convert.x = 0;	/* Make all Shell redrawing be relative to window origin		*/
convert.y = 0;	/* because we will specify our rectangle (in PDriver_GiveRectangle)	*/
		/* in coors rel to window origin.					*/

Hourglass_On();

ErrorCheck( print, PDriver_GiveRectangle( 0, &rect, &matrix, &position, 0xffffff00/*backgnd*/));

ErrorCheck( print, PDriver_DrawPage( 1, &rect, 0, NULL, &more, &n));

while( more)	{
	Shell_WindRedraw2( &rect, convert, shellwind);

	ErrorCheck( print, PDriver_GetRectangle( &rect, &more, &n));
	}

Hourglass_Off();

return NOERROR;
}





static void	Shell__PrintResultFn( print_block *print, print_result result)
{
UNUSED( print);

if ( result>256)	{
	os_error	*error = (os_error *) result;
	Error_Report( error->errnum, error->errmess);
	}

else if ( result==print_result_QUEUED)	{
	Error_Report( 0, "Shell rectangle data has been saved to print queue.");
	}

else if ( result!=print_result_OK)
	Error_Report( 0, "Shell rectangle printing error number %i", result);

else Error_Report( 0, "Shell rectangle print finished.");
}



static BOOL	Shell__PrintSaveFn( print_block *print, message_datasaveack *datasaveack)
{
Shell_rectblock *rect = (Shell_rectblock *) print->reference;
if ( rect->saver)	return rect->saver( datasaveack->filename, rect);
return ERROR;
}




void	Shell_PrintRect( Shell_rectblock *rect)
{
BOOL	e;

e = Print_StartPrint(
	Shell__RectPrintFn,
	(rect->saver) ? Shell__PrintSaveFn : NULL,
	Shell__PrintResultFn,
	rect,			/* reference		*/
	rect->filetype,
	rect->size,		/* estimated filesize	*/
	"ShellPrint",		/* leafname		*/
	"Shell-rectangle"	/* Job title.		*/
	);

if (e==ERROR)	Error_Report( 0, "Print_StartPrint failed");
}



static BOOL Shell_PrintRectClickHandler( event_pollblock *event, void *reference)
	/* This fn is called whenever there is a click on any window	*/
{
Shell_rectblock	*r;
mouse_block	ptrinfo;
os_error	error;
int		ret;

UNUSED( reference);
UNUSED( event);

if ( !Kbd_KeyDown( inkey_CTRL)) return FALSE;

Wimp_GetPointerInfo( &ptrinfo);
r = Shell_FindRectBlockFromPtr( &ptrinfo);

if ( !r) return FALSE;		/* Click wasn't on a rect		*/

error.errnum	= 0;
strcpy( error.errmess, "Print this Shell rectangle?");
ret = Wimp_ReportErrorR( &error, 3 /*OK,Cancel*/, event_taskname);
	
if (ret==1)	Shell_PrintRect( r);	/* OK pressed.	*/

return TRUE;
}






void Shell_InitPrintRectClick( void)
{
Event_Claim( event_CLICK, event_ANY, event_ANY, Shell_PrintRectClickHandler, NULL);
return;
}

