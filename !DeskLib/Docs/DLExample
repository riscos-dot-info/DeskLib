/*
   Example of DeskLib fundamentals
   - creates a task with an iconbar icon and a small
     iconbar menu including a "proginfo" box
*/

#include "DeskLib:Core.h"     /* Core DeskLib definitions etcd    */
#include "DeskLib:Window.h"   /* Window manipulation routines     */
#include "DeskLib:Menu.h"     /* Menu manipulation routines       */
#include "DeskLib:Event.h"    /* Event handling library           */
#include "DeskLib:Template.h" /* For loading templates            */
#include "DeskLib:Handler.h"  /* Provides some default handlers   */
#include "DeskLib:Icon.h"     /* Used to create iconbar icon      */
#include "DeskLib:WimpSWIs.h" /* Used to call Wimp_GetPointerInfo */

/* Function prototypes */
BOOL Click_Handler(event_pollblock *event, void *reference);
BOOL Menu_Handler(event_pollblock *event, void *reference);

/* This can be set anywhere in the program and will cause */
/* it to exit next time the poll loop is called           */
BOOL quit = FALSE;

/* Main program */
int main(void)
{
  /* Set up handles (all defined in Wimp.h) for proginfo */
  /* window and iconbar icon and menu                    */
  window_handle info_window;
  menu_ptr iconbar_menu;

  /* Initialises the DeskLib event handling routines */
  Event_Initialise("DLExample");

  /* Load templates */
  Template_Initialise();
  Template_LoadFile("<DLExample$Dir>.Templates");

  /* Create windows */
  info_window = Window_Create("Info", template_TITLEMIN);

  /* Create iconbar menu */
  iconbar_menu = Menu_New("DLExample", "Info|Help...,Settings...|Quit");

  /* ...and attach proginfo box */
  Menu_AddSubWindow(iconbar_menu, 0, info_window);

  /* Create iconbar icon */
  Icon_BarIcon("!DLExample", iconbar_RIGHT);

  /* Event handlers */
  Event_Claim(event_CLICK, window_ICONBAR, event_ANY, Click_Handler,       iconbar_menu);
  Event_Claim(event_MENU,  event_ANY,      event_ANY, Menu_Handler,        NULL);

  /* Default event handlers */
  Event_Claim(event_CLOSE, event_ANY,      event_ANY, Handler_CloseWindow, NULL);
  Event_Claim(event_OPEN,  event_ANY,      event_ANY, Handler_OpenWindow,  NULL);

  /* Call poll routine until quit */
  while(!quit) Event_Poll();

  return 0;
}

BOOL Click_Handler(event_pollblock *event, void *reference)
{
  /* Only respond is menu button was clicked */
  if (event->data.mouse.button.data.menu)
  {
    /* Display menu (-1 means: display at correct height for an iconbar menu) */
    /* We need to cast the reference back to menu_ptr type                    */
    Menu_Show((menu_ptr) reference, event->data.mouse.pos.x, -1);

    /* Return "true" to indicate we dealt with the event */
    return TRUE;
  }

  /* Return "false" if this event wasn't for us - other registered */
  /* handlers will be called until one returns "true"              */
  return FALSE;
}

BOOL Menu_Handler(event_pollblock *event, void *reference)
{
  /* For use with Wimp_GetPointerInfo */
  mouse_block pointer;

  /* Find which menu item was chosen - the menu items are index from the */
  /* top entry down, starting at 0 (in this case the "Info" entry) and   */
  /* increasing by 1 for each entry                                      */
  switch (event->data.selection[0])
  {
    case 3:
      /* Task will exit at next call to Event_Poll */
      quit = TRUE;
      /* Returning "true" indicates we dealt with the event */
      return TRUE;
      break;
  }

  /* Re-open the menu if the user clicked with adjust */
  Wimp_GetPointerInfo(&pointer);
  if (pointer.button.data.adjust) Menu_ShowLast();

  /* Returning "false" indicates we didn't deal with the event */
  return FALSE;
}
