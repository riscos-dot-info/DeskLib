/*
    ####             #    #     # #
    #   #            #    #       #          The FreeWare C library for
    #   #  ##   ###  #  # #     # ###             RISC OS machines
    #   # #  # #     # #  #     # #  #   ___________________________________
    #   # ####  ###  ##   #     # #  #
    #   # #        # # #  #     # #  #    Please refer to the accompanying
    ####   ### ####  #  # ##### # ###    documentation for conditions of use
    ________________________________________________________________________

    File:    Slider !RunImage
    Author:  Jason Williams (Hacked together from !IncDec)
             Julian Smith - typedef-d slider_updateproc
    Version: 1.01 (Apr 1995)
    Purpose: Demonstrate Sliders

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

#include "DeskLib:Slider.h"



static BOOL quit=       FALSE;
static window_handle    window;

static slider_info MySliderInfo;

typedef int (*slider_updateproc)( void *, void *);

static BOOL ByeBye(event_pollblock *event,void *ref)
{
 Window_Delete(window);
 quit=TRUE;
 return TRUE;
}



static BOOL RedrawWindow(event_pollblock *event, void *reference)
/*  Called to redraw our window. We don't have anything to draw
 *  except the slider, which we do by calling Slider_Redraw
 */
{
  window_redrawblock redraw;
  BOOL more;

  redraw.window = event->data.openblock.window;
  Wimp_RedrawWindow(&redraw, &more);

  while (more)
  {
    Slider_Redraw(&MySliderInfo, &redraw.cliprect);
    Wimp_GetRectangle(&redraw, &more);
  }

  return(TRUE);
}


static int SliderHasBeenUpdated(slider_info *info, void *reference)
/* Called whenever the slider is updated during a drag. This provides
 * an active display as you drag.
 */
{
  int currentval = Slider_ReadValue(info);

  Icon_SetInteger(window, 3, currentval);

  return(0);
}



static BOOL IconClicked(event_pollblock *event, void *reference)
{
  if (!event->data.mouse.button.data.menu)               /* Not a MENU click */
  {
    int newvalue, closed = FALSE;
    if (Slider_Drag(&MySliderInfo, &closed, &newvalue, NULL) == NULL)
    {
      /*  If the drag was successful, and the window has not been closed
       *  in the meantime, then show the end-of-drag value in the other
       *  icon in the window.
       */
      if (!closed)
        Icon_SetInteger(window, 4, newvalue);
    }

    return(TRUE);
  }

  return(FALSE);
}




int main(void)
{
 Resource_InitialisePath("Slider");
 Event_Initialise("Slider Test");
 EventMsg_Initialise();

 Screen_CacheModeInfo();

 EventMsg_Claim(message_MODECHANGE,event_ANY,Handler_ModeChange,NULL);

 Template_Initialise();
 Template_LoadFile("Templates");
 window=Window_Create("main",0);

 MySliderInfo.window = window;
 MySliderInfo.icon   = 0;
 MySliderInfo.value  = 300;
 MySliderInfo.limits.min = 30;
 MySliderInfo.limits.max = 976;
 MySliderInfo.colour.foreground = 11;
 MySliderInfo.colour.background = 0;
 MySliderInfo.border.x = MySliderInfo.border.y = 12;
 MySliderInfo.knob.spritearea = 0;
 MySliderInfo.knob.sprite = 0;
 MySliderInfo.flags.vertical = FALSE;
 MySliderInfo.flags.rgb = FALSE;
 MySliderInfo.flags.dragging = FALSE;
 MySliderInfo.flags.clickstop = FALSE;
 MySliderInfo.flags.reserved = 0;
 MySliderInfo.update = SliderHasBeenUpdated;
 MySliderInfo.reference = NULL;
                             
 Icon_SetText(window, 3, "-");
 Icon_SetText(window, 4, "-");
 
 Slider_SetValue(&MySliderInfo, 375, NULL, NULL);

 Event_Claim(event_CLOSE,window,event_ANY,ByeBye,            NULL);
 Event_Claim(event_OPEN, window,event_ANY,Handler_OpenWindow,NULL);
 Event_Claim(event_REDRAW, window,event_ANY,RedrawWindow,NULL);
 Event_Claim(event_CLICK, window,event_ANY,IconClicked,NULL);

 Window_Show(window,open_CENTERED);

 while (!quit)
   Event_Poll();

 return FALSE;
}
