/*
    ####             #    #     # #
    #   #            #    #       #          The FreeWare C library for
    #   #  ##   ###  #  # #     # ###             RISC OS machines
    #   # #  # #     # #  #     # #  #   ___________________________________
    #   # ####  ###  ##   #     # #  #
    #   # #        # # #  #     # #  #    Please refer to the accompanying
    ####   ### ####  #  # ##### # ###    documentation for conditions of use
    ________________________________________________________________________

    File:    IncDec.c
    Author:  Copyright © 1994 Lee Atkinson
    Version: 1.00 (12 Oct 1993)
    Purpose: Demonstrate Icon's Increment and Decrement handlers

*/

#include "DeskLib:Core.h"
#include "DeskLib:Window.h"            /* Window handling automation      */
#include "DeskLib:Error.h"             /* Error despatcher                */
#include "DeskLib:Event.h"             /* Event despatcher                */
#include "DeskLib:EventMsg.h"          /* Wimp Message event dispatcher   */
#include "DeskLib:Handler.h"           /* Default/example event handlers  */
#include "DeskLib:Icon.h"              /* Icon handling automation        */
#include "DeskLib:Resource.h"          /* Handles finding resource files  */
#include "DeskLib:Screen.h"            /* Getting screen size info, etc   */
#include "DeskLib:Template.h"          /* Template loading and caching    */

#define TOPTEXT    0
#define TOPINC     1
#define TOPDEC     2
#define BOTTOMTEXT 3
#define BOTTOMINC  5
#define BOTTOMDEC  4

static BOOL quit=       FALSE;
static window_handle    window;
static icon_incdecblock *incdec1;
static icon_incdecblock *incdec2;

static BOOL ByeBye(event_pollblock *event,void *ref)
{
 Icon_ReleaseIncDecHandler(incdec1);
 Icon_ReleaseIncDecHandler(incdec2);
 Window_Delete(window);
 quit=TRUE;
 return TRUE;
}

int main(void)
{
 Resource_InitialisePath("IncDec");
 Event_Initialise("Inc/Decrement Test");
 EventMsg_Initialise();
 Screen_CacheModeInfo();
 EventMsg_Claim(message_MODECHANGE,event_ANY,Handler_ModeChange,NULL);
 Template_Initialise();
 Template_LoadFile("Templates");
 window=Window_Create("main",0);
 Event_Claim(event_CLOSE,window,event_ANY,ByeBye,            NULL);
 Event_Claim(event_OPEN, window,event_ANY,Handler_OpenWindow,NULL);
 incdec1=Icon_InitIncDecHandler(window,TOPTEXT,TOPINC,TOPDEC,TRUE,5,10,100,10);
 incdec2=Icon_InitIncDecHandler(window,BOTTOMTEXT,BOTTOMINC,BOTTOMDEC,FALSE,1,
                                                                  -100,-50,-50);
 Window_Show(window,open_CENTERED);
 while (!quit) Event_Poll();
 return FALSE;
}
