/*
    ####             #    #     # #
    #   #            #    #       #          The FreeWare C library for
    #   #  ##   ###  #  # #     # ###             RISC OS machines
    #   # #  # #     # #  #     # #  #   ___________________________________
    #   # ####  ###  ##   #     # #  #
    #   # #        # # #  #     # #  #    Please refer to the accompanying
    ####   ### ####  #  # ##### # ###    documentation for conditions of use
    ________________________________________________________________________

    File:    Url.Launch.c
    Author:  Copyright © 2002 Antony Sidwell
    Version: 1.00 (4 July 2002)
    Purpose: Handle Url launching using the Event & EventMsg bits of DeskLib
*/

/*
 * About Url.Launch.c
 * ------------------
 * To launch an URL, we follow a simple series of steps:
 * 1) Broadcast an ANT request
 * 2) If that isn't claimed by anyone, do the Acorn URI_Dispatch
 *    which first broadcasts and then tries to load an app to deal with it
 * 3) If that fails, try and load an app using the ANT loading method.
 * 4) If that fails, call the failure function, which has a hook for giving
 *    user feedback.
 */

#include "DeskLib:Url.h"

/* ANT url broadcast wimp message number */  
#define message_ANTOPENURL (message_action) 0x4AF80

/* Acorn URO messages */
#define message_URI_MPROCESS (message_action) 0x4E382
#define message_URI_MRETURNRESULT (message_action) 0x4E383

/* Function prototypes for internal functions */
static BOOL Url_ReturnMessage_h(event_pollblock *event, void *param);

/* Internal data structure */
typedef struct
{
	char *url;
	urllaunch_handler *success;
	urllaunch_handler *failure;
} url_info_blk;

/* ---------------------------------------------------------------------
 * The URL launching has succeeded.
 * --------------------------------------------------------------------- */
static BOOL Url_Succeeded(event_pollblock *event, void *param)
{
	url_info_blk *block = (url_info_blk *) param;
	UNUSED(event);

	/* Release the various events we claimed to measuere success */
	EventMsg_Release(message_URI_MRETURNRESULT, event_ANY, Url_ReturnMessage_h);
	EventMsg_Release(message_ANTOPENURL, event_ANY, Url_ReturnMessage_h);
	Event_Release(event_NULL, event_ANY, event_ANY, Url_Succeeded, param);

	if (block->success != NULL)
	{
		block->success(block->url);
	}

	if (block->url != NULL) free(block->url);
	free(block);

	return (TRUE);
}

/* ---------------------------------------------------------------------
 * The URL launching has failed.
 * --------------------------------------------------------------------- */
static void Url_Failed(url_info_blk *block)
{
	/* Release the various events we claimed to measuere success */
	EventMsg_Release(message_URI_MRETURNRESULT, event_ANY, Url_ReturnMessage_h);
	EventMsg_Release(message_ANTOPENURL, event_ANY, Url_ReturnMessage_h);
	Event_Release(event_NULL, event_ANY, event_ANY, Url_Succeeded, block);

	/* If there was a "failure" function given, call it now. */
	if (block->failure != NULL) block->failure(block->url);

	/* Free our copy of the URL we were calling */
	if (block->url != NULL) free(block->url);

	/* And the data block containing it and the callbacks */
	free(block);

	return;
}

/* ---------------------------------------------------------------------
 * Broadcast an URL using the ANT protocol.
 *
 * Format of the message is simple - the URL makes up the "data" section
 * of the message.  This is only suitable for shortish URLs, less than
 * 235 (?) characters.
 * --------------------------------------------------------------------- */
static int Url_ANTBroadcast(const char *url)
{
	message_block msg;

	/* We don't bother trying for longer URLs */
	if (strlen(url) > 235) return (-1);

	/* Prepare the wimp message header */
	msg.header.size = ((sizeof(msg.header) + strlen(url) + 4) & ~3);
	msg.header.yourref = 0;
	msg.header.action = message_ANTOPENURL;

	/* Write the URL string into the "data" part of the message */
	msg.data.bytes[0] = 0;
	strncat(msg.data.bytes, url, sizeof(msg.data.bytes) - 1);

	/* Attempt to broadcast the message, return an error if we fail */
	if (Wimp_SendMessage(event_SENDWANTACK, &msg, 0 , 0)) return (-1);

	return (0);
}

/* ---------------------------------------------------------------------
 * Uses the ANT load thingy, where we try to launch a given application
 * for this type of URL.
 * --------------------------------------------------------------------- */
static BOOL Url_ANTLoad(const char *url)
{
	char *command_line = NULL;
	char *colon;

	/* Find the protocol (method) delimiter */
	colon = strchr(url, ':');

	/* We've not found a ':', so we can't identify the protocol */
	if (colon == NULL) return (FALSE);

	/* Work out how long the command line should be, allocate space */
	command_line = malloc(14 + (colon - url) + 1 + strlen(url) + 1);

	/* We couldn't allocate memory, so return failure */
	if (command_line == NULL) return (FALSE);

	/* Build a system variable to read the contents of */
	strcpy(command_line, "Alias$URLOpen_");

	/* Append the "method" */
	strncat(command_line, url, colon - url);

	/* Terminate the string */
	command_line[14 + (colon - url)] = 0;

	/* If we can't read the contents of the variable we can't launch any application */
	if (getenv(command_line) == NULL) return (FALSE);

	/* Add a space to our string */
	strcat(command_line, " ");

	/* Write the rest of the URL into the buffer */
	strcat(command_line, url);

	/* Try and Launch the task, passing the URL as parameter */
	if (Wimp_StartTask(command_line + 6) != NULL)
	{
		/* Failure to start the task */
		return (FALSE);
	}

	return (TRUE);
}

/* ---------------------------------------------------------------------
 * Launch an URL using the Acorn URI protocol.
 * --------------------------------------------------------------------- */
static void Url_URIDispatch(const char *url)
{
    int flags;

	/* Dispatch the SWI, asking to be notified of the result with a message */ 
	if (SWI(3, 1, SWI_URI_Dispatch, 1, url, event_taskhandle, &flags) != NULL || flags & 1)
	{
		/* If we have failed, move straight onto the "antload" technique */
		Url_ANTLoad(url);
	}

	return;
}

/* ---------------------------------------------------------------------
 * This is called when we get a return from our ANT broadcast message,
 * or URI message
 * --------------------------------------------------------------------- */
static BOOL Url_ReturnMessage_h(event_pollblock *event, void *param)
{
	url_info_blk 	 *block = (url_info_blk *) param;

	/* This has come back from the Acorn system */
	if (event->data.message.header.action == message_URI_MRETURNRESULT)
	{
		/* Check the flags to see if the URL was claimed */
		if ((event->data.message.data.words[0] & 0x01) == 0)
		{
			 /* url was claimed - we can stop now */
			Url_Succeeded(NULL, param);
			return (TRUE);
		}

		/* failed AcornURI. Try ANT launch */
		if (Url_ANTLoad(block->url) == FALSE)
		{
			Url_Failed(block);
		}
		else
		{
			Url_Succeeded(NULL, block);
		}

		/* we haven't acknowledged the returnresult message, so the URI
		 * handler task will automatically free the URI */
	}
	else if (event->data.message.header.action == message_ANTOPENURL)
	{
		/* If this was a response to the ant broadcast */
		if (event->type == event_USERMESSAGEACK)
		{
			/* Try the acorn method instead */
			Url_URIDispatch(block->url);
			return (TRUE);
		}
	}

	return (FALSE);
}

/* ---------------------------------------------------------------------
 * Our "entry point" for the URL launch routines.
 * You can (optionally) supply a function to be called if an URL couldn't
 * be fetched.
 * --------------------------------------------------------------------- */
extern void Url_Launch(const char *url, urllaunch_handler *failure, urllaunch_handler *success)
{
	url_info_blk *block = NULL;

	block = malloc(sizeof(url_info_blk));
	if (block == NULL)
	{
		failure(url);
		return;
	}

	block->failure = failure;
	block->success = success;
	block->url = malloc(strlen(url) + 1);

	/* Take a copy of the URL */
	if (block->url == NULL)
	{
		Url_Failed(block);
        return;
	}
	else
	{
		strcpy(block->url, url);
	}

	/* Claim these message before we've sent anything out */
	EventMsg_Claim(message_URI_MRETURNRESULT, event_ANY, Url_ReturnMessage_h, block);
	EventMsg_Claim(message_ANTOPENURL, event_ANY, Url_ReturnMessage_h, block);

	/* This is in case the original ANT broadcast succeeds! */
	Event_Claim(event_NULL, event_ANY, event_ANY, Url_Succeeded, block);

	/* If this fails, we give an immediate report of problems */
	if (Url_ANTBroadcast(url) == -1) Url_Failed(block);

	return;
}
