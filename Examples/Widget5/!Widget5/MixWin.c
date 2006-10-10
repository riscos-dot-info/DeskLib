/************************************************************************
 *                                                                      *
 *        File: MixWin.c                                                *
 *                                                                      *
 *     Purpose: Provides user interface to mix two images process       *
 *                                                                      *
 ************************************************************************/

#include "DeskLib:WimpSWIs.h"          /* Low-level WIMP commands         */
#include "DeskLib:Window.h"            /* Window handling automation      */
#include "DeskLib:Core.h"              /* usefull core functions          */
#include "DeskLib:Error.h"             /* Error despatcher                */
#include "DeskLib:Event.h"             /* Event despatcher                */
#include "DeskLib:EventMsg.h"          /* Wimp Message event dispatcher   */
#include "DeskLib:File.h"              /* Low level file handling         */
#include "DeskLib:Msgs.h"              /* Message translation code        */
#include "DeskLib:Resource.h"          /* Handles finding resource files  */
#include "DeskLib:Sound.h"             /* Beep!                           */
#include "DeskLib:Template.h"          /* Template loading and caching    */
#include "DeskLib:Icon.h"              /* Icon handling automation        */
#include "DeskLib:Screen.h"            /* Caching screen info             */
#include "DeskLib:Sprite.h"            /* Sprite handling routines        */
#include "DeskLib:File.h"              /* OS file IO                      */
#include "DeskLib:KeyCodes.h"          /* Codes for wimp returned keys    */
#include "DeskLib:Hourglass.h"         /* control hourglass               */
#include "DeskLib:Coord.h"

#include "SpriteWin.h"
#include "CheckBanks.h"
#include "Process.h"
#include "Widget5.h"
#include "Misc.h"
#include "Configure.h"

#include <stdlib.h>
#include <stdio.h>
#include <string.h>

#define initial_SLIDER  13
#define base_SLIDER 5
#define idletime_SLIDER 0

#define icon_IMAGE1 8
#define icon_IMAGE2 9
#define icon_REALTIME 10

#define default_IMAGE1PERCENT 50

extern window_handle db;

typedef struct {
   int source1;
   int source2;
   int dest;
   int slidericon;
   int arearight;
   int arealeft; /* left + slidersize */
   int ptroffset;
   unsigned int oldidletime;
   window_handle window;
} mix_block;

static BOOL Click_mixwin(event_pollblock *event, void *reference);
static BOOL Null_updateslider(event_pollblock *event, void *reference);
static BOOL Null_checksourcebanks(event_pollblock *event, void *reference);
static BOOL UserDrag_updateslider(event_pollblock *event, void *reference);
static void close_mixwin(BOOL ok, BOOL keepopen, mix_block *block);
BOOL update_imagepercents(window_handle window, int image1percent);
void update_slider(mix_block *block, int image1percent, BOOL copyblock);
void NoMemProcess(void);

/******************************************************************************
 *                            extern routines                                 *
 ******************************************************************************/

extern void MixWin_Init(void)
{
 /*
  * initialisation to be done on startup
  */
 
}

/******************************************************************************/

extern void MixWin_Open(int source1, int source2, int dest, wimp_point *openpos)
{
  mix_block *block;
  char text[20];

 /*
  * allocate memory for range info block and return if fails
  */
  block = malloc(sizeof(mix_block));
  if(block == NULL){
     NoMemProcess();
     return;
  }

 /*
  * initialise window and block
  */
  block->window = Window_Create("MixProcess", template_TITLEMIN);
  block->source1 = source1;
  block->source2 = source2;
  block->dest = dest;

 /*
  * initialise both source image bank numbers
  */
  Msgs_Lookup("txt.image", text, 19);
  Icon_printf(block->window, 1, "%s %i", text, block->source1);
  Icon_printf(block->window, 3, "%s %i", text, block->source2);

  block->slidericon = initial_SLIDER;

 /*
  * setup realtime icon
  */
  Icon_Deselect(block->window, icon_REALTIME);
  if(block->source2 == block->dest)
     Icon_Shade(block->window, icon_REALTIME);
  else
     Icon_SetSelect(block->window, icon_REALTIME, realtime); /* default state */

  Event_Claim(event_NULL, event_ANY, event_ANY, Null_checksourcebanks, block);
  Event_Claim(event_CLICK, block->window, event_ANY, Click_mixwin, block);

  update_imagepercents(block->window, default_IMAGE1PERCENT);
  update_slider(block, default_IMAGE1PERCENT, FALSE);

  Window_ShowPos(block->window, openpos);
  
}

/******************************************************************************
 *                              Click handlers                                *
 ******************************************************************************/

static BOOL Click_mixwin(event_pollblock *event, void *reference)
{
  mix_block *block = reference;
  convert_block convert;
  window_state  state;
  drag_block    dragdata;
  icon_block    bicon, sicon;
  int ptroffset;

  if(event->data.mouse.icon == block->slidericon){

    /*
     * start drag of slider
     */
     block->oldidletime = idletime;
     idletime = idletime_SLIDER;

     Event_Claim(event_NULL, event_ANY, event_ANY, Null_updateslider, block);
     Event_Claim(event_USERDRAG, event_ANY, event_ANY, UserDrag_updateslider, block); 

    /*
     * get slider and background icon positions in workarea coords
     */
     Wimp_GetIconState(block->window, base_SLIDER, &bicon);
     Wimp_GetIconState(block->window, block->slidericon, &sicon);

    /*
     * setup convert block for workarea to screen coord conversion
     */
     Wimp_GetWindowState(block->window, &state);
     convert.screenrect = state.openblock.screenrect;
     convert.scroll     = state.openblock.scroll;

    /*
     * setup drag block
     */
     dragdata.window    = block->window;
     dragdata.type      = drag_INVISIBLE;
     dragdata.screenrect= state.openblock.screenrect;   /* dragged box not used */

    /*
     * set drag area to be right hand edge of background and
     * lefthand edge + size of slider bar
     */
     dragdata.parent.min.x = bicon.workarearect.min.x + 4 +
        /* size of slider */ (sicon.workarearect.max.x - sicon.workarearect.min.x);
     dragdata.parent.max.x = bicon.workarearect.max.x - 4;
     Coord_RectToScreen(&dragdata.parent, &convert);

    /*
     * move drag area to left by amount pointer is to left of slider bar right edge
     */
     ptroffset = event->data.mouse.pos.x -
                 Coord_XToScreen(sicon.workarearect.max.x, &convert);
     dragdata.parent.min.x += ptroffset;
     dragdata.parent.max.x += ptroffset;

    /*
     * set up area in block
     */
     block->arealeft = dragdata.parent.min.x;
     block->arearight = dragdata.parent.max.x;
     block->ptroffset = ptroffset;

    /*
     * prevent y movement
     */
     dragdata.parent.min.y = event->data.mouse.pos.y;
     dragdata.parent.max.y = event->data.mouse.pos.y;

     Wimp_DragBox(&dragdata);

     return(TRUE);
  }


  switch(event->data.mouse.icon)
  {
    
    case 11: /* Close */
       close_mixwin(FALSE, FALSE, block);
       break;

    case 12: /* Mix */
       close_mixwin(TRUE, event->data.mouse.button.data.adjust, block);
       break;
      
  }
  return(TRUE);
}

/******************************************************************************
 *                                null handlers                               *
 ******************************************************************************/

static BOOL Null_updateslider(event_pollblock *event, void *reference)
{
  mouse_block ptrinfo;
  mix_block *block = reference;
  int image1percent;
  int image2percent;
  int backgroundlength;

  Wimp_GetPointerInfo(&ptrinfo);

 /*
  * calculate percentages of position of pointer in restricted movable area
  */
  backgroundlength = block->arearight - block->arealeft;
  image1percent = ((block->arearight - ptrinfo.pos.x) * 100) / backgroundlength;
  image2percent = ((ptrinfo.pos.x - block->arealeft) * 100) / backgroundlength;

 /*
  * use the value closest to the pointer to calculate the other
  */
  if(image1percent < 50)
     image1percent = 100 - image2percent;

  if(update_imagepercents(block->window, image1percent))
     update_slider(block, image1percent, TRUE);

  return(TRUE);
}

/******************************************************************************/

static BOOL Null_checksourcebanks(event_pollblock *event, void *reference)
{
  mix_block *block = reference;

 /*
  * close process window if any of the source banks become free
  */
  if( SpriteWin_FreeImage(block->source1) || SpriteWin_FreeImage(block->source2) )
     close_mixwin(FALSE, FALSE, block);

  return(FALSE);
}

/******************************************************************************
 *                             userdrag handlers                              *
 ******************************************************************************/

static BOOL UserDrag_updateslider(event_pollblock *event, void *reference)
{
  mix_block *block = reference;

  Wimp_DragBox((drag_block *) -1);
  Event_Release(event_USERDRAG, event_ANY, event_ANY, UserDrag_updateslider, block);
  Event_Release(event_NULL, event_ANY, event_ANY, Null_updateslider, block);

 /*
  * if realtime is on then update destination
  */
  if(Icon_GetSelect(block->window, icon_REALTIME))
     close_mixwin(TRUE, TRUE, block);

 /*
  * reset wimp_poll idletime
  */
  idletime = block->oldidletime;

  return(TRUE);
}

/******************************************************************************
 *                               close handlers                               *
 ******************************************************************************/

static void close_mixwin(BOOL ok, BOOL keepopen, mix_block *block)
{
  wimp_point *size;
  int image1percent;

  if(ok == TRUE){

    /*
     * process the image
     */
     size = SpriteWin_GetSize(block->source1);

    /*
     * create destination if not created and return if creation fails
     */
     if( SpriteWin_FreeImage(block->dest) ){

        if( !SpriteWin_Create(size, block->dest) )
           return;
     }

     image1percent = Icon_GetInteger(block->window, icon_IMAGE1);
     Process_Mix(block->source1, block->source2, block->dest, image1percent, size);

    /*
     * refresh destination image
     */
     SpriteWin_RefreshWin(block->dest);
     
  }

  if(keepopen == FALSE){

     Event_Release(event_NULL, event_ANY, event_ANY, Null_checksourcebanks, block);
     Window_Delete(block->window);
     free(block);
  }

}

/******************************************************************************
 *                           menu choice handlers                             *
 ******************************************************************************/

/******************************************************************************
 *                             misc routines                                  *
 ******************************************************************************/

BOOL update_imagepercents(window_handle window, int image1percent)
{
  char text[10];
  BOOL updatedicons;

  updatedicons = FALSE;

 /*
  * update the icons if the numbers have changed
  */
  sprintf(text, "%i%%", image1percent);
  if(strcmp(text, Icon_GetTextPtr(window, icon_IMAGE1)) != 0){
     Icon_SetText(window, icon_IMAGE1, text);
     updatedicons = TRUE;

     sprintf(text, "%i%%", 100 - image1percent);
     Icon_SetText(window, icon_IMAGE2, text);

  }

  return(updatedicons);
}

/******************************************************************************/

void update_slider(mix_block *block, int image1percent, BOOL copyblock)
{
  icon_block bicon;
  icon_createblock sicon;
  int movablelength;
  int righttoedge;
  int sliderlength;
  int backgroundlength;
  window_redrawblock r;

 /*
  * get slider and background icon positions in workarea coords
  */
  Wimp_GetIconState(block->window, base_SLIDER, &bicon);
  Wimp_GetIconState(block->window, block->slidericon, &sicon.icondata);

 /*
  * setup redraw block to remove old slider position
  */
  r.window = block->window;
  r.rect = sicon.icondata.workarearect;
  r.rect.max.x += 4;

 /*
  * calculate new slider position
  */
  sliderlength = sicon.icondata.workarearect.max.x - sicon.icondata.workarearect.min.x;
  backgroundlength = (bicon.workarearect.max.x - 4) - (bicon.workarearect.min.x + 4);
  movablelength = backgroundlength - sliderlength;
                  
  righttoedge = (movablelength * image1percent) / 100;

  sicon.icondata.workarearect.max.x = bicon.workarearect.max.x - 4 - righttoedge;
  sicon.icondata.workarearect.min.x = sicon.icondata.workarearect.max.x - sliderlength;

 /*
  * remove and refresh old slider
  */
  sicon.window = block->window;
  Wimp_DeleteIcon(block->window, block->slidericon);
  Wimp_ForceRedraw(&r);

 /*
  * add and redraw new slider
  */
  Wimp_CreateIcon(&sicon, &(block->slidericon));
  Wimp_SetIconState(block->window, block->slidericon, 0, 0); /* forceredraw slider */
  
}

/******************************************************************************/

void NoMemProcess(void)
{
  char err_msg[128];

  Msgs_Lookup("err.nomemproc", err_msg, 127);
  Error_Report(1,err_msg);

}
