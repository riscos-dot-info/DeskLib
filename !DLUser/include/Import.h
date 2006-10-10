/*
    ####             #    #     # #
    #   #            #    #       #          The FreeWare C library for
    #   #  ##   ###  #  # #     # ###             RISC OS machines
    #   # #  # #     # #  #     # #  #   ___________________________________
    #   # ####  ###  ##   #     # #  #
    #   # #        # # #  #     # #  #    Please refer to the accompanying
    ####   ### ####  #  # ##### # ###    documentation for conditions of use
    ________________________________________________________________________

    File:    Import.h
    Author:  Copyright © 1995 Julian Smith
    Version: 1.00 (17 Jan 1995)
    Purpose: File import handling.
*/


#ifndef __Import_h
#define __Import_h

#ifndef __dl_wimp_h
#include "DeskLib:Wimp.h"
#endif


typedef void (*import_fileimporter)( message_dataload *data, void *reference);
/*
This function-type should load data from the file specified in 'data'.

'reference' is the reference originally passed to
Import_RegisterFileLoader.
*/

typedef void *import_ramimporter;
/*
This will be changed sometime to be a function-type for importing using
the RAM transfer protocol.
*/

BOOL	Import_RegisterFileLoader( 
		import_fileimporter	fn, 
		import_ramimporter	ramfn,
		window_handle		window, 
		icon_handle		icon, 
		void			*reference
		);
/*
This registers an internal function (using Event_Claim) which is called
whenver a file is dropped onto window-icon, and does all the
Wimp_SendMessage stuff involved in the wimp file transfer protocol.

'ramfn' is ignored at the moment - use NULL. Eventuall the Import
library will support ram-transfer.

You supply a function 'fn' which will be called to do the actual file-loading, 
using the information in the message_dataload structure.
'fn' will also be given 'reference' as passed in the original call to 
Import_RegisterFileLoader, which can be used to give extra information.

NB message_dataload is defined in DeskLib:Wimp.h and has the following
fields:
	window_handle window;
	icon_handle   icon;
	wimp_point    pos;
	int           estsize;
	int           filetype;
	char          filename[212];
	
The window, icon and pos are the mouse info for where the file was dropped.
filetype and filename are as given by whoever is giving us the file.

'fn' should *not* call Wimp_Poll, as it is called in the middle of
the file-transfer protocol.
*/


#endif

