
#include "DeskLib:WimpSWIs.h"          /* Low-level WIMP commands         */
#include "DeskLib:Window.h"            /* Window handling automation      */
#include "DeskLib:Dialog.h"            /* High-level dialogue windows     */
#include "DeskLib:DragASpr.h"          /* DragASprite veneers             */
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
#include "DeskLib:Pane.h"              /* Pane window handling            */
#include "DeskLib:Resource.h"          /* Handles finding resource files  */
#include "DeskLib:Screen.h"            /* Getting screen size info, etc   */
#include "DeskLib:Sound.h"             /* Sound System control            */
#include "DeskLib:Template.h"          /* Template loading and caching    */
#include "DeskLib:Sprite.h"

/*
 * ANSI includes
 *---------------
 */

#include <stdio.h>
#include <string.h>

BOOL quit = FALSE;
window_handle window;
int sblck[2560];
char name [] ="ampy";
sprite_areainfo *sblock = (sprite_areainfo *) sblck;

BOOL CloseWin(event_pollblock *, void *);
BOOL Redraw(event_pollblock *, void *);

int main(void)
{
  Resource_Initialise("SpriteTest");
  Event_Initialise("Sprite Test");
  EventMsg_Initialise();
  Screen_CacheModeInfo();
  EventMsg_Claim(message_MODECHANGE, event_ANY, Handler_ModeChange, NULL);
  Template_Initialise();
  Template_LoadFile("Templates");
  sblock->areasize = 10240;
  sblock->numsprites = 0;
  sblock->firstoffset = 16;
  sblock->freeoffset = 16;
  Sprite_Load((sprite_area)sblock, "<SpriteTest$Dir>.me_bw");
  Event_Claim(event_CLOSE, event_ANY, event_ANY, CloseWin, NULL);
  Event_Claim(event_REDRAW, event_ANY, event_ANY, Redraw, NULL);
  Event_Claim(event_OPEN, event_ANY, event_ANY, Handler_OpenWindow, NULL);
  window = Window_CreateAndShow("stest", 12, open_WHEREVER);

  while (!quit)
    Event_Poll();

  return(0);
}

BOOL CloseWin(event_pollblock *e, void *r)
{
  quit = TRUE;
  return(TRUE);
}

/* Sprite at 120,-340 */

BOOL Redraw(event_pollblock *e, void *r)
{
  window_redrawblock redraw;
  BOOL more;
  wimp_point c;

  c.x=120;
  c.y=-320;

  redraw.window = e->data.openblock.window;
  Wimp_RedrawWindow(&redraw, &more);

  while (more)
  {
    Sprite_WimpPlot((sprite_area)sblock, name, &c, (convert_block *)&redraw.rect,0);
    Wimp_GetRectangle(&redraw, &more);
  }

  return(TRUE);
}

