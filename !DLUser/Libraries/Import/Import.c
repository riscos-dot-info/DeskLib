/*
    ####             #    #     # #
    #   #            #    #       #          The FreeWare C library for
    #   #  ##   ###  #  # #     # ###             RISC OS machines
    #   # #  # #     # #  #     # #  #   ___________________________________
    #   # ####  ###  ##   #     # #  #
    #   # #        # # #  #     # #  #    Please refer to the accompanying
    ####   ### ####  #  # ##### # ###    documentation for conditions of use
    ________________________________________________________________________

    File:    Import.Import.c
    Author:  Copyright © 1995 Julian Smith
    Version: 1.00 (17 Jan 1995)
    Purpose: File import handling.
*/


#include <stdlib.h>
#include <string.h>

#include "DeskLib:WimpSWIs.h"
#include "DeskLib:Error.h"
#include "DeskLib:Event.h"
#include "DeskLib:File.h"

#include "DeskLib:Import.h"




typedef struct	{
	import_fileimporter	fn;		/* Function which loads the file.	*/
	import_ramimporter      ramfn;		/* Function which ram-transfers (NIY)	*/
	void			*reference;	/* Reference to pass to 'fn'.		*/
	unsigned int		ourref;		/* wimp message 'ourref' for the last	*/
						/* message that we have sent.		*/
	}
	import_block;




static BOOL	Import__DataLoadHandler( event_pollblock *event, void *reference)
{
import_block	*import	= (import_block *) reference;


if ( event->data.message.header.action != message_DATALOAD)	return FALSE;

if ( import->ourref && ( event->data.message.header.yourref != import->ourref))	return FALSE;
	/* 
	import->ourref != 0 means that we have just received a 
	message_DATASAVE and replied with a message_DATASAVEACK, so 
	we are expecting a messsage_DATALOAD which is a reply to our 
	message_DATASAVEACK. This messsage_DATALOAD doesn't have 
	a matching 'yourref' so we return.
	*/

import->fn( &event->data.message.data.dataload, import->reference);
	/* Call the function which does the actual loading.	*/


/* Now finish the file-transfer protocol...	*/

if ( import->ourref)	{
	/* 
	This DATALOAD is a bona-fide response to our previous 
	message_DATASAVEACK, so the file will be <Wimp$Scrap>. Hence
	we should delete <Wimp$Scrap> 'cos we've just loaded it.
	*/

	import->ourref = 0;
		/* Clear things ready for the next message_DATALOAD	*/
	
	if ( strcmp( event->data.message.data.dataload.filename, "<Wimp$Scrap>"))	{
		Error_Report( 
			0, 
			"Scrapfile transfer, but filename is '%s' instead of "
			"'<Wimp$Scrap>' - not deleting file", 
			event->data.message.data.dataload.filename
			);
		}
		
	else File_Delete( "<Wimp$Scrap>");
	}

/* Tell whoever is giving us the file that we have loaded it.	*/
	{
	message_block	reply	= event->data.message;
	
	reply.header.action	= message_DATALOADACK;
	reply.header.yourref	= event->data.message.header.myref;
	
	Wimp_SendMessage( event_SEND, &reply, event->data.message.header.sender, 0);
	}

return TRUE;
}






static BOOL	Import__DataSaveHandler( event_pollblock *event, void *reference)
{
import_block	*import	= (import_block *) reference;
message_block	reply;


if ( event->data.message.header.action != message_DATASAVE)	return FALSE;

/* 
We need to tell the sender of this message)DATASAVE to do the transfer
via  <Wimp$Scrap>. We also tell them that we aren't the filer, so their
file  won't be 'safe' after we've loaded it.
*/

reply				= event->data.message;
reply.header.action		= message_DATASAVEACK;
reply.header.yourref		= event->data.message.header.myref;
reply.data.datasaveack.estsize	= -1;	/* = Unsafe	*/

strcpy( reply.data.datasaveack.filename, "<Wimp$Scrap>");
	/* Should really check that <Wimp$Scrap> exists.	*/

reply.header.size = sizeof( message_block);
	/* 
	We've changed the message size by strcpy-ing into ...filename.
	Should really calculate the correct length here, not just use the
	maximum possible.
	*/

Wimp_SendMessage( event_SEND, &reply, reply.header.sender, 0);

import->ourref	= reply.header.myref;
	/* 
	So we can check that a subsequent message_DATALOAD is a response to
	this.
	*/

return TRUE;
}






BOOL	Import_RegisterFileLoader( 
		import_fileimporter	fn, 
		import_ramimporter	ramfn,
		window_handle		window, 
		icon_handle		icon, 
		void			*reference
		)
{
import_block	*import;

import = malloc( sizeof( import_block));
if ( !import)	{
	Error_OutOfMemory( FALSE, "import block");
	return ERROR;
	}

import->fn		= fn;
import->ramfn		= ramfn;
import->reference	= reference;
import->ourref		= 0;	/* Only non-zero for DATASAVE transfer.	*/

Event_Claim( event_USERMESSAGE, window, icon, Import__DataLoadHandler, import);
Event_Claim( event_USERMESSAGERECORDED, window, icon, Import__DataLoadHandler, import);
Event_Claim( event_USERMESSAGE, window, icon, Import__DataSaveHandler, import);
Event_Claim( event_USERMESSAGERECORDED, window, icon, Import__DataSaveHandler, import);

return NOERROR;

}

