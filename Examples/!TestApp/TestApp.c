/*
    ####             #    #     # #
    #   #            #    #       #          The FreeWare C library for 
    #   #  ##   ###  #  # #     # ###             RISC OS machines
    #   # #  # #     # #  #     # #  #   ___________________________________
    #   # ####  ###  ##   #     # #  #                                      
    #   # #        # # #  #     # #  #    Please refer to the accompanying
    ####   ### ####  #  # ##### # ###    documentation for conditions of use
    ________________________________________________________________________

    File:    TestApp.c
    Author:  Copyright © 1992, 1993 Jason Williams
    Version: 1.10 (12 Jul 1993)
    Purpose: Demonstrate  (and test ;-)  some of the main DeskLib application
             base routines. Note that 80% of this file is comment(!) and that
             most of the calls used are overkill for such a small application
*/


#include "DeskLib:WimpSWIs.h"          /* Low-level WIMP commands         */
#include "DeskLib:Window.h"            /* Window handling automation      */

#include "DeskLib:Dialog.h"            /* High-level dialogue windows     */
#include "DeskLib:Error.h"             /* Error despatcher                */
#include "DeskLib:Event.h"             /* Event despatcher                */
#include "DeskLib:EventMsg.h"          /* Wimp Message event dispatcher   */
#include "DeskLib:File.h"              /* Low level file handling         */
#include "DeskLib:GFX.h"               /* Graphics routines (GFX_Wait)    */
#include "DeskLib:Handler.h"           /* Default/example event handlers  */
#include "DeskLib:Hourglass.h"         /* Hourglass module interfaces     */
#include "DeskLib:Icon.h"              /* Icon handling automation        */
#include "DeskLib:Menu.h"              /* Menu create & show support      */
#include "DeskLib:Msgs.h"              /* Message translation code        */
#include "DeskLib:Resource.h"          /* Handles finding resource files  */
#include "DeskLib:Screen.h"            /* Getting screen size info, etc   */
#include "DeskLib:Sound.h"             /* Sound System control            */
#include "DeskLib:Template.h"          /* Template loading and caching    */
#include "DeskLib:Debug.h"             /* Diagnostic output               */
#include <stdio.h>
#include <string.h>

static BOOL     quit      = FALSE;     /* Set to TRUE to quit the app.    */
static menu_ptr mainmenu  = NULL;      /* Pointer to our main menu        */
static menu_ptr item2menu = NULL;      /* Pointer to our main menu        */


/* Defines for the positions at which items appear on our menus.
 * Note that menu items always start at number 0, so item number 2 is
 * actually the THIRD item in the menu.
 */
#define mainmenu_CHANGETITLE 0
#define mainmenu_ITEM1       1
#define mainmenu_ITEM2       2
#define mainmenu_QUIT        3


static BOOL AskUser(char *questiontag)
/*  Tests/demonstrates use of Dialog_ calls to show a query window containing
 *  the given bit of text - the given string is a Msgs_ message tag
 */
{
  char   msg[200];
  dialog dlog;
  BOOL   clickedOK;
  
  dlog = Dialog_Create("query", 0);                 /* Create the window    */

  Msgs_Lookup(questiontag, msg, 196);               /* Fill in the message  */
  Icon_SetText(Dialog_WindowHandle(dlog), 2, msg);

  Dialog_Show(dlog);                                /* Pop up the dialog    */
  Sound_SysBeep();                                  /* And beep 4 attention */

  while (Dialog_WaitForClick(dlog) > 1 || Dialog_Persist(dlog))
    /* Busy wait loop */ ;                          /* Wait for OK/Cancel   */

  /*  Check: Did they click OK?
   *  The other possibilities are: they didn't click any icon, but just closed
   *  the window (in which case LastClicked will be dialog_NOCHOICE), or
   *  they clicked the cancel icon (icon number 1)
   *  We must remember this before calling Dialog_Destroy, as the dialog
   *  structure will be lost upon a destroy.
   */
  clickedOK = (Dialog_LastClicked(dlog) == 0);

  Event_Poll();  /*  Ensure WIMP gets a chance to deselect the icon before we
                  *  kill the window (this is a bug in the RISC OS 3 Wimp)
                  */

  Dialog_Destroy(dlog);              /* Remove the dialog, releasing memory */

  return(clickedOK);
}



static window_handle menuwindow = NULL;

static BOOL ButtonClick(event_pollblock *event, void *reference)
/*  Handler attached to all button click events.
 *  If no other handler specifically handles the click, and it was a menu
 *  click, we will pop up our menu in an appropriate position.
 */
{
  int ypos;

  UNUSED( reference);
  
  Debug_Printf( "A mouse-button has been pressed...\n");

  if (event->data.mouse.button.data.menu)             /* is a MENU click */
  {
    if (event->data.mouse.window >= 0)
    {
      ypos = event->data.mouse.pos.y;                /* Over a window    */
      menuwindow = event->data.mouse.window;         /* (remember which) */
      Menu_SetFlags(mainmenu, mainmenu_CHANGETITLE, FALSE, FALSE);
                                                     /* Unshade item 0   */
    }
    else
    {
      ypos = -1;                                     /* Over the iconbar */
      menuwindow = NULL;
      Menu_SetFlags(mainmenu, mainmenu_CHANGETITLE, FALSE, TRUE);
                                                     /* Shade item 0     */
    }

    Menu_Show(mainmenu, event->data.mouse.pos.x, ypos);
  }

  return(TRUE);
}



static BOOL MenuChoice(event_pollblock *event, void *reference)
/*  Handler to take care of the user's menu choices.
 *  For now, we simply beep at practically everything except the 'Quit' option
 */
{
  mouse_block ptr;

  UNUSED( reference);
  
  Debug_Printf( "Menu choice... for item %i\n", event->data.selection[0]);
  
  if (menu_currentopen != mainmenu)        /* Only respond to the mainmenu  */
    return(FALSE);

  switch(event->data.selection[0])
  {
    case mainmenu_CHANGETITLE:             /* Change the window's titlebar  */
      Sound_SysBeep();                     /* Beep to confirm the choice.   */
      if (menuwindow != NULL)
        Window_SetTitle(menuwindow, "Whee! A New title!");
      break;

    case mainmenu_ITEM2:
      switch(event->data.selection[1])
      {
        case 0:
          Menu_SetFlags(item2menu, 0, 1, 0);  /* Tick item 0   */
          Menu_SetFlags(item2menu, 1, 0, 0);  /* Untick item 1 */
          break;

        case 1:
          Menu_SetFlags(item2menu, 0, 0, 0);  /* Untick item 0 */
          Menu_SetFlags(item2menu, 1, 1, 0);  /* Tick item 1   */
          break;

        case -1:                      /* Instead of choosing a submenu item, */
                                      /* user clicked main item directly     */
          
          Debug_Printf( "TestApp is now going to cause a deliberate divide-by-zero error\n");
          
          AskUser("query.click");
          
          #ifdef DeskLib_DEBUG
          Debug_Printf( "Infinity = %i\n", event->data.mouse.window/0);
          /* 
          This deliberately causes a division by zero error (warned by
          cc during compilation) in the debug version of TestApp.
          This will cause a signal to be raised in the debug-version. 
          This signal will be displayed and the program terminated.
          */
          #endif

          break;
      }
      break;

    case mainmenu_QUIT:
      quit = TRUE;
      break;

    default:
      Sound_SysBeep();                     /* Beep to confirm the choice.   */
      break;
  }

  Wimp_GetPointerInfo(&ptr);               /* If ADJUST was used to select, */
  if (ptr.button.data.adjust)              /* then we keep the menu open.   */
    Menu_ShowLast();

  return(TRUE);
}



int main( void)
{
  window_handle window1, window2, window3;
  icon_handle   baricon;
  char          appname[64];
  char          menudescription[260];


  /*  Tell Resource (and thereby Template, Msgs, etc) where our resource
   *  directory is: "<Tester$Dir>"
   */
  Resource_Initialise("Tester");   /* resources in <Tester$Dir> */


  /*  Load and cache the messages. Find out the application name.
   */
  Msgs_LoadFile("messages");
  Msgs_Lookup("app.name:Tester", appname, 64);


  /*  Initialise the Wimp and Event Manager.
   *  The task name shown on the task display and error reports is set from
   *  the string "appname" fetched previously from the messages file.
   */
  Event_Initialise(appname);
  EventMsg_Initialise();

  Debug_Initialise();
  Debug_InitialiseSignal();
  Debug_Printf( "DeskLib test application started...\n");
  /*  Print some diagnostic iff the debugging version of !TestApp
   *  is being made
   */

  /*  Read and remember the current screen mode information
   *  (width and height, eig-factors, delta-x and delta-y, etc.)
   *  This is needed by the Menu code to get menu widths correct.
   */
  Screen_CacheModeInfo();


  /*  Add a handler function to a "screen mode changing" Wimp message, so
   *  that the information provided by the "Screen" functions is always
   *  correct.
   *  NOTE that the new Handler_ModeChange should also ensure that outline
   *  fonts will work if we change between lo-res and hi-res screen modes.
   */
  EventMsg_Claim(message_MODECHANGE, event_ANY, Handler_ModeChange, NULL);


  /*  Plonk an icon onto the iconbar
   */
  baricon = Icon_BarIcon("!testapp", iconbar_RIGHT);


  /*  Place the Handler_ module skeleton default handlers on all 
   *  Redraw and Open-window request events (Application-wide defaults)
   */
  Event_Claim(event_REDRAW, event_ANY, event_ANY, Handler_NullRedraw, NULL);
  Event_Claim(event_OPEN, event_ANY, event_ANY, Handler_OpenWindow, NULL);


  /*  Load in and cache our window templates from the template file
   *  "<Tester$Dir>.Templates" (Templates utilise the "Resource Directory"),
   *  with support for using outline fonts in the icons if the file specifies
   *  outline fonts.
   */
  Template_Initialise();
  Template_UseOutlineFonts();
  Template_LoadFile("Templates");

  /*  Create and open our main windows from the template "mainwindow".
   *  Open the same window three times (3 copies):
   *  1.  on-screen wherever it was positioned in the template manager,
   *  2.  Centered on screen (screen-mode independent)
   *  3.  Under the pointer (over where the program was double-clicked -note
   *      that the window won't appear until after the hourglass test below
   *      so it may not look like it was opened under the pointer!)
   */
  window1 = Window_CreateAndShow("mainwindow", 0, open_WHEREVER);
  window2 = Window_CreateAndShow("mainwindow", 0, open_CENTERED);
  window3 = Window_CreateAndShow("mainwindow", 0, open_UNDERPOINTER);


  /*  Claim window-close events
   *  Use the Window_Delete() handler rather than the Wimp_CloseWindow() one
   *  so that the window is closed and deleted and all memory and event 
   *  claims are released whenever the user hits the close icon.
   */
  Event_Claim(event_CLOSE, event_ANY, event_ANY, Handler_DeleteWindow, NULL);


  /*  Add the 3-d "OK" button click handler so that the OK button (icon #2)
   *  in the 3-d window indents for 1/3rd of a second whenever it is clicked
   */
  Event_Claim(event_CLICK, window2, 2, Handler_ClickOK, NULL);


  /*  Add the Window_HelpHandler message handler so that help requests
   *  for the 3 main windows and the iconbar icon are automatically handled
   */
  Window_AutoHelp(event_ANY, event_ANY);


  /*  Menu example/test
   *  1. Create two menu structures from messages in the Msgs file.
   *  2. Attach the 'item2menu' as a submenu of the main menu
   *  3. Extend the submenu by adding another two items to it
   *  4. Add a handler to pop up our menu if we get a menu click over
   *     our icon/windows, and another handler to process menu choices.
   */
  Msgs_Lookup("menu.main", menudescription, 260);
  mainmenu  = Menu_New("TestApp", menudescription);

  Msgs_Lookup("menu.item2", menudescription, 260);
  item2menu = Menu_New("SubMenu", menudescription);

  item2menu = Menu_Extend(item2menu, "~Extension 1,~Extension 2");

  Menu_AddSubMenu(mainmenu, 2, item2menu);

  Event_Claim(event_CLICK, event_ANY, event_ANY, ButtonClick, NULL);
  Event_Claim(event_MENU,  event_ANY, event_ANY, MenuChoice,  NULL);


  /*  Test the SWI call interface, and the Hourglass macros by whizzing
   *  through a 0..100 percent count. Also uses GFX_Wait to tie it
   *  to the vsync, so it will take 1 second to go from 0% to 100%
   *
   *  Also test mouse-rectangle restriction
   */

  Pointer_RestrictToWindow(window3);

  {
    int loop;
    Hourglass_Start(1);
    for (loop = 0; loop < 100; loop++)
    {
      Hourglass_Percentage(loop);
      GFX_Wait();
    }
    Hourglass_Off();
  }

  Pointer_Unrestrict();

  /*  Main event handling loop. Let Event_ do all the work for us!
   */
  while (!quit)
    Event_Poll();

  Template_ClearAll();

  /*  Finally, we just exit. The atexit() handlers will clean up after us
   */
  return(0);
}
