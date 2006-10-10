/************************************************************************
 *                                                                      *
 *        File: PaneTests.c                                             *
 *                                                                      *
 *     Purpose: Tests Pane2 library                                     *
 *                                                                      *
 ************************************************************************/

#include "DeskLib:WimpSWIs.h"          /* Low-level WIMP commands         */
#include "DeskLib:Window.h"            /* Window handling automation      */
#include "DeskLib:Core.h"              /* usefull core functions          */
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
#include "DeskLib:Slider.h"            /* "Easy" slider handling          */
#include "DeskLib:Kbd.h"               /* Read hotkeys                    */
#include "DeskLib:Time.h"              /* "TIME"                          */
#include "DeskLib:KernelSWIs.h"        /* OS_Byte                         */

#include "DeskLib:Pane2.h"

#include "kernel.h"
#include "MySwis.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

/******************************************************************************/

static BOOL CloseEventHandler(event_pollblock *event, void *reference)
{
  
  Pane2_Delete(event->data.words[0]);

  return(TRUE);
}

/******************************************************************************/

static BOOL ClickEventHandler(event_pollblock *event, void *reference)
{

  switch(event->data.mouse.icon)
  {
    case 1: case 2:
      Pane2_Delete(event->data.mouse.window);
      break;
  }

  return(TRUE);
}

/******************************************************************************/

static BOOL ClickMain1EventHandler(event_pollblock *event, void *reference)
{
  int flags;

  switch(event->data.mouse.icon)
  {
    case 1:
      flags = Pane2_SetFlags(event->data.mouse.window, 1, -1); /* read flags */

      if(Icon_GetSelect(event->data.mouse.window, 1))/* turnonresize */
         flags = flags | pane2_VRESIZE;

      else{ /* turn off resize */
         flags = flags | pane2_VRESIZE;
         flags = flags ^ pane2_VRESIZE;
      }

      Pane2_SetFlags(event->data.mouse.window, 1, flags); /* set new flags */

      Pane2_Show(event->data.mouse.window, open_WHEREVER); /* refresh size etc... */
      break;
  }

  return(TRUE);
}

/******************************************************************************/

static BOOL ClickPane4bEventHandler(event_pollblock *event, void *reference)
{

  switch(event->data.mouse.icon)
  {
    case 2:
      Pane2_DeletePane((window_handle) reference, 2);
      break;
  }

  return(TRUE);
}

/******************************************************************************/

extern void PaneTests_Init(void)
{


}

/******************************************************************************/

extern void PaneTests_MenuSelection(int selection)
{
  window_handle main;
  window_handle pane;
  wimp_point zerooffset = {0, 0};
  wimp_point paneoffset;
  wimp_point panesize;

  switch(selection)
  {
     case 1:
       /*
        * Test 1
        * Top left of pane is attached to top left of main window usings
        * offsets in template file
        */
        main = Window_Create("Main1", template_TITLEMIN);

        Pane2_AddMain(main);

        pane = Window_Create("Pane1", template_TITLEMIN);

        Pane2_AddPane(main, pane, NULL, NULL,
                      pane2_FIXED|pane2_MAINTOP|pane2_PANETOP);

        Event_Claim(event_CLICK, main, event_ANY, ClickMain1EventHandler, NULL); 

        Event_Claim(event_CLOSE, main, event_ANY, CloseEventHandler, NULL);

        Pane2_Show(main, open_WHEREVER);

        break;

     case 2:
       /*
        * Test 2
        * Top left of first pane is attached to top left of main window using a supplied
        * offset of 0,0 and the pane is resized when the main window is resized.
        * Bottom left of second pane is attached to bottom left of main window using a
        * supplied offset of 0,0 and the pane is resized and scrolled horizontally with
        * the main window.
        */
        main = Pane2_CreateAndAddMain("Main2", template_TITLEMIN);

        Pane2_CreateAndAddPane("Pane2a", template_TITLEMIN, main, &zerooffset, NULL,
                    pane2_FIXED|pane2_MAINTOP|pane2_PANETOP|pane2_HRESIZE|pane2_HSTRETCH);

        Pane2_CreateAndAddPane("Pane2b", template_TITLEMIN, main, &zerooffset, NULL,
                               pane2_FIXED|pane2_HRESIZE|pane2_HSCROLL|pane2_HSTRETCH);

        Event_Claim(event_CLOSE, main, event_ANY, CloseEventHandler, NULL);

        Pane2_Show(main, open_WHEREVER);

        break;

     case 3:
       /*
        * Test 3
        * Pane is sized and positioned according to icon 0 on the main window
        * The pane is linked to the main window forming a commonly used options
        * type window
        */
        main = Pane2_CreateAndAddMain("Main3", template_TITLEMIN);

        Pane2_ReadIcon(main, 0, "Pane3", &paneoffset, &panesize);
        Pane2_CreateAndAddPane("Pane3", template_TITLEMIN, main, &paneoffset, &panesize,
                               pane2_FIXED|pane2_MAINTOP|pane2_PANETOP);

        Event_Claim(event_CLICK, main, event_ANY, ClickEventHandler, NULL);
        Event_Claim(event_CLOSE, main, event_ANY, CloseEventHandler, NULL);

        Pane2_Show(main, open_WHEREVER);
        
        break;

     case 4:
       /*
        * Test 4
        * !ArtWork style movable panes
        */
        main = Pane2_CreateAndAddMain("Main4", template_TITLEMIN);

        Pane2_CreateAndAddPane("Pane4a", template_TITLEMIN, main, NULL, NULL,
                               pane2_MAINTOP|pane2_PANETOP);

        pane = Pane2_CreateAndAddPane("Pane4b", template_TITLEMIN, main, NULL, NULL,
                                      pane2_PANETOP);

        Event_Claim(event_CLICK, pane, event_ANY, ClickPane4bEventHandler, (void *)main); 

        Event_Claim(event_CLOSE, main, event_ANY, CloseEventHandler, NULL);

        Pane2_Show(main, open_WHEREVER);
        
        break;

  }
}
