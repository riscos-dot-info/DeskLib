/************************************************************************
 *                                                                      *
 *        File: GreyEdWin.c                                             *
 *                                                                      *
 *     Purpose: Provides user interface to edit greylevels process      *
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
#include "DeskLib:Menu.h"              /* Menu create & show support      */
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
#include <math.h>
                        
extern window_handle db;

#define total_GREYBARS 65

typedef struct {
   window_handle window;
   int sourceimage;
   int destimage;
   unsigned int oldidletime;
   char greybarmap[total_GREYBARS];
   char greymap[256];
} greyed_block;

#define idletime_BARS 0

#define icon_IMAGEX       1
#define icon_IMAGEY       3

#define icon_REALTIME     11
#define icon_CLOSE        12
#define icon_MAP          13
#define icon_NORMDRAW     6
#define icon_SMOOTHDRAW   7
#define icon_VSMOOTHDRAW  8
#define icon_HEIGHTDISP   25
#define icon_WIDTHDISP    10
#define icon_BARBACK      14
#define icon_BARMIN       26
#define icon_BARMAX       90
#define icon_MAXHEIGHT    256
#define icon_PROCESSES    5

#define bar_WIDTH 4
#define border_WIDTH 4

static BOOL Click_greyedwin(event_pollblock *event, void *reference);
static BOOL Null_updatebars(event_pollblock *event, void *reference);
static BOOL Null_checksourcebank(event_pollblock *event, void *reference);
static void close_greyedwin(BOOL ok, BOOL keepopen, greyed_block *block);
static BOOL UserDrag_updatebars(event_pollblock *event, void *reference);
static BOOL MenuChoice_processmenu(event_pollblock *event, void *reference);
void NoMemEdit(void);
void bar_setheight(window_handle window, icon_handle icon, int barheight, BOOL redraw);
void map_linear(greyed_block *block);
void map_invert(greyed_block *block);
void map_histeq(greyed_block *block);
void map_gamma(greyed_block *block);
void map_threshold(greyed_block *block);
void map_slce(greyed_block *block);
void refresh_map(window_handle window);
void barmap2map(char (*greybarmap)[], char (*greymap)[]);
void map2barmap(char (*greybarmap)[], char (*greymap)[]);
void addtomaps(greyed_block *block, int greybarno, int barheight);

/* process menu */
static menu_ptr processmenu = NULL;
static menu_ptr gammasubmenu = NULL;
static menu_ptr threshsubmenu = NULL;
static char gammavalue[5];
static char threshvalue[4];
static greyed_block *currentblock = NULL;

/* process menu items */
#define processmenu_LINEAR 0
#define processmenu_INVERT 1
#define processmenu_THRESH 2
#define processmenu_GAMMA  3
#define processmenu_HISTEQ 4
#define processmenu_SLCE   5

/******************************************************************************
 *                            extern routines                                 *
 ******************************************************************************/

extern void GreyEdWin_Init(void)
{
 /*
  * initialisation to be done on startup
  */
  char menudescription[260];
  char menutitle[15];

 /*
  * build menus
  */
  Msgs_Lookup("title.greyproc", menutitle, 15);
  Msgs_Lookup("menu.greyproc", menudescription, 260);
  processmenu = Menu_New(menutitle, menudescription);

  Msgs_Lookup("title.threshval", menutitle, 15);
  threshsubmenu = Menu_New(menutitle,"    ");
  Menu_AddSubMenu(processmenu, processmenu_THRESH, threshsubmenu);
  Menu_MakeWritable(threshsubmenu, 0, threshvalue, 4, "a0-9");

  Msgs_Lookup("title.gammaval", menutitle, 15);
  gammasubmenu = Menu_New(menutitle,"    ");
  Menu_AddSubMenu(processmenu, processmenu_GAMMA, gammasubmenu);
  Menu_MakeWritable(gammasubmenu, 0, gammavalue, 5, "a0-9.");

  Event_Claim(event_MENU, event_ANY, event_ANY, MenuChoice_processmenu, NULL);

 /*
  * default values
  */
  strcpy(threshvalue, "128");
  strcpy(gammavalue, "1.00");
}

/******************************************************************************/

extern void GreyEdWin_Open(int source, int dest, wimp_point *openpos)
{
  greyed_block *block;
  char text[20];

 /*
  * allocate memory for range info block and return if fails
  */
  block = malloc(sizeof(greyed_block));
  if(block == NULL){
     NoMemEdit();
     return;
  }

 /*
  * initialise window and block
  */
  block->window = Window_Create("GreyEdit", template_TITLEMIN);
  block->sourceimage = source;
  block->destimage = dest;

 /*
  * initialise both source image bank numbers
  */
  Msgs_Lookup("txt.image", text, 19);
  Icon_printf(block->window, icon_IMAGEX, "%s %i", text, block->sourceimage);
  Icon_printf(block->window, icon_IMAGEY, "%s %i", text, block->destimage);

 /*
  * setup realtime and smooth draw icons
  */
  Icon_Deselect(block->window, icon_REALTIME);
  if(block->sourceimage == block->destimage)
     Icon_Shade(block->window, icon_REALTIME);
  else
     Icon_SetSelect(block->window, icon_REALTIME, realtime); /* default state */

  Icon_SetRadios(block->window, icon_NORMDRAW, icon_VSMOOTHDRAW,
                 icon_NORMDRAW + drawtype);

 /*
  * clear position display
  */
  Icon_SetText(block->window, icon_HEIGHTDISP, ""); 
  Icon_SetText(block->window, icon_WIDTHDISP, "");

 /*
  * initialise map to linear
  */
  map_linear(block);

  Event_Claim(event_NULL, event_ANY, event_ANY, Null_checksourcebank, block);
  Event_Claim(event_CLICK, block->window, event_ANY, Click_greyedwin, block);

  Window_ShowPos(block->window, openpos);
  
}

/******************************************************************************
 *                              Click handlers                                *
 ******************************************************************************/

static BOOL Click_greyedwin(event_pollblock *event, void *reference)
{
  greyed_block *block = reference;
  convert_block convert;
  window_state state;
  drag_block dragdata;
  icon_block bicon;

  if(event->data.mouse.icon == icon_BARBACK ||
    ((event->data.mouse.icon >= icon_BARMIN) && (event->data.mouse.icon <= icon_BARMAX))){

    /*
     * start drag of greylevel mapping bars
     */
     block->oldidletime = idletime;
     idletime = idletime_BARS;

     Event_Claim(event_NULL, event_ANY, event_ANY, Null_updatebars, block);
     Event_Claim(event_USERDRAG, event_ANY, event_ANY, UserDrag_updatebars, block);

    /*
     * get background icon position in workarea coords
     */
     Wimp_GetIconState(block->window, icon_BARBACK, &bicon);

    /*
     * setup convert block for workarea to screen coord conversion
     */
     Wimp_GetWindowState(block->window, &state);
     convert.screenrect = state.openblock.screenrect;
     convert.scroll = state.openblock.scroll;
     
    /*
     * setup drag block
     */
     dragdata.window = block->window;
     dragdata.type = drag_INVISIBLE;
     dragdata.screenrect = state.openblock.screenrect;   /* dragged box not used */

    /*
     * set drag area to background icon (screen coords)
     */
     dragdata.parent.min.x = bicon.workarearect.min.x + border_WIDTH;
     dragdata.parent.max.x = bicon.workarearect.max.x - border_WIDTH;
     dragdata.parent.min.y = bicon.workarearect.min.y + border_WIDTH;
     dragdata.parent.max.y = bicon.workarearect.max.y - border_WIDTH;
     Coord_RectToScreen(&dragdata.parent, &convert);

     Wimp_DragBox(&dragdata);

     return(TRUE);
  }

  switch(event->data.mouse.icon)
  {
    case icon_PROCESSES:
       currentblock = block;
       Popup_menu(block->window, icon_PROCESSES, processmenu);
       break;

    case icon_NORMDRAW: case icon_SMOOTHDRAW: case icon_VSMOOTHDRAW:
       Icon_Select(block->window, event->data.mouse.icon);
       break;
    
    case icon_CLOSE:
       close_greyedwin(FALSE, FALSE, block);
       break;

    case icon_MAP: 
       close_greyedwin(TRUE, event->data.mouse.button.data.adjust, block);
       break;
      
  }
  return(TRUE);
}

/******************************************************************************
 *                                null handlers                               *
 ******************************************************************************/

static BOOL Null_updatebars(event_pollblock *event, void *reference)
{
  mouse_block ptrinfo;
  greyed_block *block = reference;
  convert_block convert;
  window_state state;
  icon_block bicon;
  int greybarheight;
  int barheight;
  int greybarnumber;
  char postext[5];
  int bariconnumber;
  int newgreybarheight;
  int across;

  Wimp_GetPointerInfo(&ptrinfo);
  Icon_SetInteger(db, 3, ptrinfo.pos.x);
  Icon_SetInteger(db, 4, ptrinfo.pos.y);


 /*
  * setup convert block for workarea to screen coord conversion
  */
  Wimp_GetWindowState(block->window, &state);
  convert.screenrect = state.openblock.screenrect;
  convert.scroll = state.openblock.scroll;

 /*
  * get background icon position in workarea coords
  */
  Wimp_GetIconState(block->window, icon_BARBACK, &bicon);

 /*
  * convert mouse position to workarea coords
  */
  Coord_PointToWorkArea(&ptrinfo.pos, &convert);

 /*
  * height of bar in workarea terms
  */
  barheight = ptrinfo.pos.y - (bicon.workarearect.min.y + border_WIDTH);

 /*
  * number of greylevel bar (first is 0)
  */
  greybarnumber = (ptrinfo.pos.x - (bicon.workarearect.min.x + border_WIDTH) ) >> 2;
  if( !(greybarnumber < total_GREYBARS) )
     greybarnumber = total_GREYBARS - 1;

 /*
  * update greybarmap
  */
  greybarheight = barheight;
  if(greybarheight > 255)
     greybarheight = 255;

 /*
  * update bar on screen
  */
  bariconnumber = greybarnumber + icon_BARMIN;
  addtomaps(block, greybarnumber, greybarheight);

 /*
  * update height display
  */
  sprintf(postext, "%i", greybarheight);
  if( strcmp(Icon_GetTextPtr(block->window, icon_HEIGHTDISP), postext) != 0 )
     Icon_SetText(block->window, icon_HEIGHTDISP, postext);
  across = greybarnumber << 2;
  if(across > 255)
     across = 255;
  sprintf(postext, "%i", across);
  if( strcmp(Icon_GetTextPtr(block->window, icon_WIDTHDISP), postext) != 0 ) 
     Icon_SetText(block->window, icon_WIDTHDISP, postext);

 /*
  * if smooth draw is on then update both other bars
  */

  if(Icon_GetSelect(block->window, icon_VSMOOTHDRAW)){

     if(bariconnumber > (icon_BARMIN + 2)){
        newgreybarheight = (block->greybarmap[greybarnumber - 3] + greybarheight) >> 1;
        addtomaps(block, greybarnumber - 2, newgreybarheight);
     }

     if(bariconnumber < (icon_BARMAX - 2)){
        newgreybarheight = (block->greybarmap[greybarnumber + 3] + greybarheight) >> 1;
        addtomaps(block, greybarnumber + 2, newgreybarheight);
     }
  }

  if( Icon_GetSelect(block->window, icon_SMOOTHDRAW) ||
      Icon_GetSelect(block->window, icon_VSMOOTHDRAW) ){

     if(bariconnumber > (icon_BARMIN + 1)){
        newgreybarheight = (block->greybarmap[greybarnumber - 2] + greybarheight) >> 1;
        addtomaps(block, greybarnumber - 1, newgreybarheight);
     }

     if(bariconnumber < (icon_BARMAX - 1)){
        newgreybarheight = (block->greybarmap[greybarnumber + 2] + greybarheight) >> 1;
        addtomaps(block, greybarnumber + 1, newgreybarheight);
     }
  }

  return(TRUE);
}

/******************************************************************************/

static BOOL Null_checksourcebank(event_pollblock *event, void *reference)
{
  greyed_block *block = reference;

 /*
  * close process window if the source bank become free
  */
  if( SpriteWin_FreeImage(block->sourceimage) )
     close_greyedwin(FALSE, FALSE, block);

  return(FALSE);
}

/******************************************************************************
 *                             userdrag handlers                              *
 ******************************************************************************/

static BOOL UserDrag_updatebars(event_pollblock *event, void *reference)
{
  greyed_block *block = reference;

  Wimp_DragBox((drag_block *) -1);
  Event_Release(event_USERDRAG, event_ANY, event_ANY, UserDrag_updatebars, block);
  Event_Release(event_NULL, event_ANY, event_ANY, Null_updatebars, block);

 /*
  * if realtime is on then update destination
  */
  if(Icon_GetSelect(block->window, icon_REALTIME))
     close_greyedwin(TRUE, TRUE, block);

 /*
  * reset wimp_poll idletime
  */
  idletime = block->oldidletime;

 /*
  * clear position display
  */
  Icon_SetText(block->window, icon_HEIGHTDISP, "");
  Icon_SetText(block->window, icon_WIDTHDISP, "");


  if(Icon_GetSelect(block->window, icon_REALTIME))
     close_greyedwin(TRUE, TRUE, block);

  return(TRUE);
}

/******************************************************************************
 *                               close handlers                               *
 ******************************************************************************/

static void close_greyedwin(BOOL ok, BOOL keepopen, greyed_block *block)
{
  wimp_point *size;

  if(ok == TRUE){

    /*
     * process the image
     */
     size = SpriteWin_GetSize(block->sourceimage);

    /*
     * create destination if not created and return if creation fails
     */
     if( SpriteWin_FreeImage(block->destimage) ){

        if( !SpriteWin_Create(size, block->destimage) )
           return;
     }

     Process_GreyMap(block->sourceimage, block->destimage, size, &(block->greymap));
     
    /*
     * refresh destination image
     */
     SpriteWin_RefreshWin(block->destimage);
     
  }

  if(keepopen == FALSE){

     Event_Release(event_NULL, event_ANY, event_ANY, Null_checksourcebank, block);
     Window_Delete(block->window);
     free(block);
  }

}

/******************************************************************************
 *                           menu choice handlers                             *
 ******************************************************************************/

static BOOL MenuChoice_processmenu(event_pollblock *event, void *reference)
{
  mouse_block ptr;

  if (menu_currentopen != processmenu) /* Only respond to the processmenu */
    return(FALSE);


  switch(event->data.selection[0])
  {
     case processmenu_LINEAR:
        map_linear(currentblock);
        break;

     case processmenu_INVERT:
        map_invert(currentblock);
        break;

     case processmenu_THRESH:
        map_threshold(currentblock);
        break;
        
     case processmenu_GAMMA:
        map_gamma(currentblock);
        break;

     case processmenu_HISTEQ:
        map_histeq(currentblock);
        break;

     case processmenu_SLCE:
        map_slce(currentblock);
        break;

  }

  if(Icon_GetSelect(currentblock->window, icon_REALTIME))
     close_greyedwin(TRUE, TRUE, currentblock);


  Wimp_GetPointerInfo(&ptr);               /* If ADJUST was used to select, */
  if (ptr.button.data.adjust)              /* then we keep the menu open.   */
    Menu_ShowLast();

  return(TRUE);
}

/******************************************************************************
 *                             misc routines                                  *
 ******************************************************************************/

void NoMemEdit(void)
{
  char err_msg[128];

  Msgs_Lookup("err.nomemproc", err_msg, 127);
  Error_Report(1,err_msg);

}

/******************************************************************************/

void bar_setheight(window_handle window, icon_handle icon, int barheight, BOOL redraw)
{
  icon_createblock icreate;
  icon_handle iconhandle;
  window_redrawblock r;

  icreate.window = window;
  Wimp_GetIconState(window, icon, &icreate.icondata);
  Wimp_DeleteIcon(window, icon);

 /*
  * setup redraw block to redraw difference between both icons
  */
  r.window = window;
  r.rect = icreate.icondata.workarearect;

  if(barheight < 4)
     barheight = 4;
  if(barheight == 255)
     barheight = 256;
  icreate.icondata.workarearect.max.y = icreate.icondata.workarearect.min.y + barheight;
  Wimp_CreateIcon(&icreate, &iconhandle);

  if(redraw){

    if(r.rect.max.y > icreate.icondata.workarearect.max.y){ /* icon is shorter */

        r.rect.min.y = icreate.icondata.workarearect.max.y;
     }
     else{ /* icon is longer */

        r.rect.min.y = r.rect.max.y;
        r.rect.max.y = icreate.icondata.workarearect.max.y;
     }
     Wimp_ForceRedraw(&r);
  }

}

/******************************************************************************/

void map_linear(greyed_block *block)
{
  int bar;
  int greylevel;

 /*
  * make greymap linear
  */
  for(greylevel = 0; greylevel < 256; greylevel++)
      block->greymap[greylevel] = greylevel;

 /*
  * update greybarmap
  */
  map2barmap(&(block->greybarmap), &(block->greymap));


 /*
  * update bars on screen
  */
  for(bar = 0; bar < total_GREYBARS ; bar++)
      bar_setheight(block->window, bar + icon_BARMIN, block->greybarmap[bar], FALSE);

 /*
  * refresh bars on screen
  */
  refresh_map(block->window);
  
}

/******************************************************************************/

void map_invert(greyed_block *block)
{
  int bar;
  int greylevel;

 /*
  * make greymap inverted linear
  */
  for(greylevel = 0; greylevel < 256; greylevel++)
      block->greymap[greylevel] = 255 - greylevel;

 /*
  * update greybarmap
  */
  map2barmap(&(block->greybarmap), &(block->greymap));


 /*
  * update bars on screen
  */
  for(bar = 0; bar < total_GREYBARS ; bar++)
      bar_setheight(block->window, bar + icon_BARMIN, block->greybarmap[bar], FALSE);

 /*
  * refresh bars on screen
  */
  refresh_map(block->window);
  
}

/******************************************************************************/

void map_histeq(greyed_block *block)
{
 /*
  * make greymap histogram equalized
  */
  wimp_point *size;
  int bar;

  size = SpriteWin_GetSize(block->sourceimage);

  Process_HistogramEq(block->sourceimage, size, &(block->greymap));

 /*
  * update greybarmap
  */
  map2barmap(&(block->greybarmap), &(block->greymap));


 /*
  * update bars on screen
  */
  for(bar = 0; bar < total_GREYBARS ; bar++)
      bar_setheight(block->window, bar + icon_BARMIN, block->greybarmap[bar], FALSE);

 /*
  * refresh bars on screen
  */
  refresh_map(block->window);

}

/******************************************************************************/

void map_gamma(greyed_block *block)
{
 /*
  * produce gamma corrected greymap
  */
  int bar;
  int greylevel;
  int newgrey;
  double gamma;
  double factor;

  factor = 1.0 / 255.0;

  gamma = atof(gammavalue);

  if(gamma == 0){

     Menu_SetText(gammasubmenu, 0, "0.00");

     for(greylevel = 0; greylevel < 256; greylevel++)
         block->greymap[greylevel] = 255;

  }
  else{

     block->greymap[0] = 0;

     for(greylevel = 1; greylevel < 256; greylevel++){

         newgrey = (int) ( ( pow((double) greylevel * factor, gamma) * 255.0 ) + 0.5 );

         if(newgrey > 255)
            newgrey = 255;
         block->greymap[greylevel] = newgrey;
     }
  }

 /*
  * update greybarmap
  */
  map2barmap(&(block->greybarmap), &(block->greymap));


 /*
  * update bars on screen
  */
  for(bar = 0; bar < total_GREYBARS ; bar++)
      bar_setheight(block->window, bar + icon_BARMIN, block->greybarmap[bar], FALSE);

 /*
  * refresh bars on screen
  */
  refresh_map(block->window);
  

}

/******************************************************************************/

void map_threshold(greyed_block *block)
{
 /*
  * make greymap thresholded
  */
  int greylevel;
  int bar;
  int threshold;

  threshold = atoi(threshvalue);

  if(threshold == 0)
     Menu_SetText(threshsubmenu, 0, "0");

  if(threshold > 255){
     threshold = 255;
     Menu_SetText(threshsubmenu, 0, "255");
  }

  for(greylevel = 0; greylevel < threshold; greylevel++)
      block->greymap[greylevel] = 0;

  for(greylevel = threshold; greylevel < 256; greylevel++)
      block->greymap[greylevel] = 255;

 /*
  * update greybarmap
  */
  map2barmap(&(block->greybarmap), &(block->greymap));


 /*
  * update bars on screen
  */
  for(bar = 0; bar < total_GREYBARS ; bar++)
      bar_setheight(block->window, bar + icon_BARMIN, block->greybarmap[bar], FALSE);

 /*
  * refresh bars on screen
  */
  refresh_map(block->window);

}

/******************************************************************************/

void map_slce(greyed_block *block)
{
 /*
  * make SLCE greymap
  */
  wimp_point *size;
  int bar;

  size = SpriteWin_GetSize(block->sourceimage);

 /*
  * calculate SLCE greymap
  */
  Process_SLCE(block->sourceimage, size, &(block->greymap));

 /*
  * update greybarmap
  */
  map2barmap(&(block->greybarmap), &(block->greymap));


 /*
  * update bars on screen
  */
  for(bar = 0; bar < total_GREYBARS ; bar++)
      bar_setheight(block->window, bar + icon_BARMIN, block->greybarmap[bar], FALSE);

 /*
  * refresh bars on screen
  */
  refresh_map(block->window);

}

/******************************************************************************/

void refresh_map(window_handle window)
{
  window_redrawblock r;
  icon_block iconinfo;

  Wimp_GetIconState(window, icon_BARBACK, &iconinfo);

  r.window = window;
  r.rect.min.x = iconinfo.workarearect.min.x + border_WIDTH;
  r.rect.max.x = iconinfo.workarearect.max.x - border_WIDTH;
  r.rect.min.y = iconinfo.workarearect.min.y + border_WIDTH;
  r.rect.max.y = iconinfo.workarearect.max.y - border_WIDTH;
  Wimp_ForceRedraw(&r);

}

/******************************************************************************/

void barmap2map(char (*greybarmap)[], char (*greymap)[])
{
 /*
  * calculate greymap from greybarmap
  */
  int greylevel;
  int barlevel;

 /*
  * up to 251 they are 4 apart
  */
  for(greylevel = 0; greylevel < 252; greylevel += 4){

     barlevel = greylevel >> 2;

     (*greymap)[greylevel]     = (*greybarmap)[barlevel];

     (*greymap)[greylevel + 1] = ( (3 * (*greybarmap)[barlevel    ]) >> 2 ) +
                                 ( (    (*greybarmap)[barlevel + 1]) >> 2 );

     (*greymap)[greylevel + 2] = ( (    (*greybarmap)[barlevel    ]) >> 1 ) +
                                 ( (    (*greybarmap)[barlevel + 1]) >> 1 );

     (*greymap)[greylevel + 3] = ( (    (*greybarmap)[barlevel    ]) >> 2 ) +
                                 ( (3 * (*greybarmap)[barlevel + 1]) >> 2 );

  }

 /*
  * 252 and over they are 3 apart
  */
  greylevel = 252;
  barlevel = greylevel >> 2;

  (*greymap)[greylevel]     = (*greybarmap)[barlevel];

  (*greymap)[greylevel + 1] = ( (2 * (*greybarmap)[barlevel    ]) / 3 ) +
                              ( (    (*greybarmap)[barlevel + 1]) / 3 );

  (*greymap)[greylevel + 2] = ( (    (*greybarmap)[barlevel    ]) / 3 ) +
                              ( (2 * (*greybarmap)[barlevel + 1]) / 3 );

  (*greymap)[greylevel + 3] = (*greybarmap)[barlevel + 1];

}

/******************************************************************************/

void map2barmap(char (*greybarmap)[], char (*greymap)[])
{
 /*
  * convert greymap to greybarmap
  */
  int bar;

 /*
  * all bars exept last are 4 apart
  */
  for(bar = 0; bar < (total_GREYBARS - 1); bar++){

      (*greybarmap)[bar] = (*greymap)[bar << 2];
  }

 /*
  * last bar is 3 apart
  */
  bar = total_GREYBARS - 1;
  (*greybarmap)[bar] = (*greymap)[255];

}

/******************************************************************************/

void addtomaps(greyed_block *block, int greybarno, int barheight)
{
 /*
  * adds value to greybarmap, barmap and to bar on screen
  */
  int greylevel;

 /*
  * put barheight into greybarmap
  */
  block->greybarmap[greybarno] = barheight;

 /*
  * calculate central greylevel in greymap and place in central value
  */
  greylevel = greybarno << 2;
  block->greymap[greylevel] = barheight;

 /*
  * up to 251 they are 4 apart
  */

  /* do 3 above greybar */
  if( (greybarno >= 0) && (greybarno < (total_GREYBARS - 2)) ){

     block->greymap[greylevel + 1] = ( (3 * block->greybarmap[greybarno    ]) >> 2 ) +
                                     ( (    block->greybarmap[greybarno + 1]) >> 2 );

     block->greymap[greylevel + 2] = ( (    block->greybarmap[greybarno    ]) >> 1 ) +
                                     ( (    block->greybarmap[greybarno + 1]) >> 1 );

     block->greymap[greylevel + 3] = ( (    block->greybarmap[greybarno    ]) >> 2 ) +
                                     ( (3 * block->greybarmap[greybarno + 1]) >> 2 );

  }

  /* do 3 below greybar */
  if( (greybarno > 0) && (greybarno <= (total_GREYBARS - 2)) ){

     block->greymap[greylevel - 3] = ( (3 * block->greybarmap[greybarno - 1]) >> 2 ) +
                                     ( (    block->greybarmap[greybarno    ]) >> 2 );

     block->greymap[greylevel - 2] = ( (    block->greybarmap[greybarno - 1]) >> 1 ) +
                                     ( (    block->greybarmap[greybarno    ]) >> 1 );

     block->greymap[greylevel - 1] = ( (    block->greybarmap[greybarno - 1]) >> 2 ) +
                                     ( (3 * block->greybarmap[greybarno    ]) >> 2 );

  }

  /* do 2 above second from last greybar */
  if(greybarno == total_GREYBARS - 2){

     block->greymap[greylevel + 1] = ( (2 * block->greybarmap[greybarno    ]) / 3 ) +
                                     ( (    block->greybarmap[greybarno + 1]) / 3 );

     block->greymap[greylevel + 2] = ( (    block->greybarmap[greybarno    ]) / 3 ) +
                                     ( (2 * block->greybarmap[greybarno + 1]) / 3 );

  }

  /* do 2 below top greybar */
  if(greybarno == total_GREYBARS - 1){

     block->greymap[greylevel - 2] = ( (2 * block->greybarmap[greybarno - 1]) / 3 ) +
                                     ( (    block->greybarmap[greybarno    ]) / 3 );

     block->greymap[greylevel - 1] = ( (    block->greybarmap[greybarno - 1]) / 3 ) +
                                     ( (2 * block->greybarmap[greybarno    ]) / 3 );

  }

  bar_setheight(block->window, greybarno + icon_BARMIN, barheight, TRUE);
}
