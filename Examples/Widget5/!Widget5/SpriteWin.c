/************************************************************************
 *                                                                      *
 *        File: SpriteWin.c                                             *
 *                                                                      *
 *     Purpose: Store 256 grey level arbitrary sized images in 6 'image *
 *              banks' and display them in a window using a pixel       *
 *              colour translation table to map the grey levels to the  *
 *              screen palette if required.                             *
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

#include "DeskLib:Dialog2.h"           /* windows on menus made easy      */

#include "flex.h"                      /* Flexible malloc                 */

#include "DeskLib:Pane.h"                      /* Panes with less pain            */

#include "ATrans.h"                    /* My load/save library            */

#include "kernel.h"                    /* clib kernel functions           */
#include "MySWIs.h"                    /* SWI no defs                     */
#include "SpriteWin.h"
#include "Misc.h"
#include "LoadSave.h"
#include "ProcessWin.h"
#include "GraphWin.h"
#include "Widget5.h"
#include "Configure.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

/* debug window */
extern window_handle db;

/* size of palette in sprite */
#define sizeof_PALETTE 2048

/* some common sprite details */
#define Mode 21 /* 256 colours */
#define Colours 256
#define SpriteName "widget5pic"

#define image_OFFSET 2108

/* array to hold 256 grey level palette */
static palette_entry GreyscalePalette[Colours];

/* array to hold image bank info */
static spritewin_block *imageblock[number_IMAGES];

extern int *imageblockadd = (int *) &imageblock[0];

/* menu for image bank window */
static menu_ptr imagewindowmenu = NULL;
static menu_ptr savemenu = NULL;
static menu_ptr processmenu = NULL;
static menu_ptr graphmenu = NULL;

static int currentimageno = 0; /* it's sad isn't it! */

#define imagewindowmenu_IMAGEINFO 0
#define imagewindowmenu_SAVE      1
#define imagewindowmenu_PROCESS   2
#define imagewindowmenu_GRAPH     3
#define imagewindowmenu_ZOOM      4

/* zoom window */
static dialog2_block *zoomwindow;
/* max and min scalefactors for zoom window */
#define scale_MAX 50
#define scale_MIN 1

/* save windows */
#define savemenu_SPRITE  0
#define savemenu_WIDGET5 1
#define savemenu_WIDGET4 2
static dialog2_block *save;
static dialog2_block *savewidget5;

/* image info window */
static dialog2_block *imageinfo;

static BOOL Redraw(event_pollblock *event, void *reference);
static BOOL Click(event_pollblock *event, void *reference);
static BOOL Click_zoomwindow(event_pollblock *event, void *reference);
static BOOL Click_savesprite(event_pollblock *event, void *reference);
static BOOL Click_savewidget4(event_pollblock *event, void *reference);
static BOOL Click_savewidget5(event_pollblock *event, void *reference);
static BOOL Key_savesprite(event_pollblock *event, void *reference);
static BOOL Key_savewidget4(event_pollblock *event, void *reference);
static BOOL Key_savewidget5(event_pollblock *event, void *reference);
static BOOL Key_zoomwindow(event_pollblock *event, void *reference);
static BOOL BuildPixTransTable(event_pollblock *event, void *reference);
static BOOL ModeChange(event_pollblock *event, void *reference);
static BOOL CloseWindow(event_pollblock *event, void *reference);
static BOOL Menuwarn(event_pollblock *event, void *reference);
static BOOL MenuChoice(event_pollblock *event, void *reference);
void build_pixtranstable(spritewin_block *block);
void NotEnoughMem(void);
void DragToSave(void);
void scale_image(int multiplier, int divisor, spritewin_block *block);
void update_screenscale(spritewin_block *block);

/******************************************************************************
 *                            extern routines                                 *
 ******************************************************************************/

extern void SpriteWin_Init(void)
{
 /*
  * initialisation to be done on startup
  */
  int colour;
  int imageno;
  char menudescription[260];
  char menutitle[15];

 /*
  * initialise 256 level greyscale sprite palette
  */
  for(colour = 0; colour < Colours; colour++){

      GreyscalePalette[colour].data.gcol = 0;
      GreyscalePalette[colour].data.red = colour;
      GreyscalePalette[colour].data.green = colour;
      GreyscalePalette[colour].data.blue = colour;

  }

 /*
  * initialise image blocks so that all images are unused
  */
  for(imageno = 0; imageno < number_IMAGES; imageno++)
     imageblock[imageno] = NULL;

 /*
  * build menus
  */
  Msgs_Lookup("title.image", menutitle, 15);
  Msgs_Lookup("menu.image", menudescription, 260);
  imagewindowmenu = Menu_New(menutitle, menudescription);

  Msgs_Lookup("title.save", menutitle, 15);
  Msgs_Lookup("menu.save", menudescription, 260);
  savemenu = Menu_New(menutitle, menudescription);
  Menu_MakeWarn(imagewindowmenu, imagewindowmenu_SAVE, !fullversion);
  if(fullversion)
     Menu_AddSubMenu(imagewindowmenu, imagewindowmenu_SAVE, savemenu);

  Msgs_Lookup("title.process", menutitle, 15);
  Msgs_Lookup("menu.process", menudescription, 260);
  processmenu = Menu_New(menutitle, menudescription);
  Menu_AddSubMenu(imagewindowmenu, imagewindowmenu_PROCESS, processmenu);

  Msgs_Lookup("title.graph", menutitle, 15);
  Msgs_Lookup("menu.graph", menudescription, 260);
  graphmenu = Menu_New(menutitle, menudescription);
  Menu_AddSubMenu(imagewindowmenu, imagewindowmenu_GRAPH, graphmenu);

  Event_Claim(event_MENU, event_ANY, event_ANY, MenuChoice, NULL);

 /*
  * initialise dialog box structs
  */
  save = Dialog2_CreateDialogBlock( "Save", 42, 42, NULL, NULL, NULL);
  if(fullversion)
     savewidget5 = Dialog2_CreateDialogBlock( "SaveWidget5", 42, 42, NULL, NULL, NULL);
  imageinfo = Dialog2_CreateDialogBlock( "ImageInfo", 42, 42, NULL, NULL, NULL);
  zoomwindow = Dialog2_CreateDialogBlock( "Zoom", 42, 42, NULL, NULL, NULL);
  EventMsg_Claim(message_MENUWARNING, event_ANY, Menuwarn, NULL);

}

/******************************************************************************/

extern void SpriteWin_InitBlock(spritewin_block *block, int imageno)
{

  block->imageno = imageno; /* used for reference only */
  block->spritearea = NULL;
  block->size.x = 0;
  block->size.y = 0;

}

/******************************************************************************/

extern BOOL SpriteWin_FreeImage(int imageno)
{
 /*
  * returns TRUE if the image is free
  */
  return(imageblock[imageno] == NULL);
}

/******************************************************************************/

extern BOOL SpriteWin_SameSize(wimp_point *res, int imageno)
{
 /*
  * returns TRUE if image is same size as 'res'
  */
  return( (imageblock[imageno]->size.x == res->x) &&
          (imageblock[imageno]->size.y == res->y) );
}

/******************************************************************************/

extern BOOL SpriteWin_Identical(int image1, int image2)
{
 /*
  * returns TRUE if both images are the same size
  */
  return( (imageblock[image1]->size.x == imageblock[image2]->size.x) &&
          (imageblock[image1]->size.y == imageblock[image2]->size.y) );

}

/******************************************************************************/

extern wimp_point *SpriteWin_GetSize(int imageno)
{
 /*
  * returns the size of the image 'imageno'
  */

  return( &imageblock[imageno]->size );
}

/******************************************************************************/

extern int SpriteWin_GetPixel(wimp_point *pos, int imageno)
{
 /*
  * returns the grey level value of the pixel at 'pos' in image bank 'imageno'
  */
  return(((char *)imageblock[imageno]->spritearea)[image_OFFSET +
                                  pos->x + (pos->y * imageblock[imageno]->linebytes)]);
 /*
  return (Sprite_ReadPixel(imageblock[imageno].spritearea, SpriteName, pos));
  */
}

/******************************************************************************/

extern void SpriteWin_PutPixel(wimp_point *pos, int pixel, int imageno)
{
 /*
  * sets the grey level of the pixel at 'pos' to 'pixel' in image bank 'imageno'
  */

  ((char *)imageblock[imageno]->spritearea)[image_OFFSET +
                           pos->x + (pos->y * imageblock[imageno]->linebytes)] = pixel;
  
 /*
  Sprite_WritePixel(imageblock[imageno].spritearea, SpriteName, pos, pixel, 0);
  */
}

/******************************************************************************/

extern spritewin_block *SpriteWin_GetImageBlock(int imageno)
{
 /*
  * returns a pointer to the spritewin_block for image bank 'imageno'
  */

  return(imageblock[imageno]);
}

/******************************************************************************/

extern BOOL SpriteWin_AddImageBlock(spritewin_block *block, int imageno)
{
 /*
  * adds the image bank info block 'block' to the list of image banks
  * returns FALSE if bank in use
  */

  if(imageblock[imageno] != NULL)
     return(FALSE);

  imageblock[imageno] = block;
  block->imageno = imageno;

  return(TRUE);

}

/******************************************************************************/

extern void SpriteWin_RefreshWin(int imageno)
{
 /*
  * refreshes the window of the image bank 'imageno'
  */
  window_state windowstate;

  if(imageblock[imageno] == NULL)
     return;

 /*
  * refresh window (the sad way)
  */
  Wimp_GetWindowState(imageblock[imageno]->window, &windowstate);
  Wimp_CloseWindow(imageblock[imageno]->window);
  Wimp_OpenWindow(&windowstate.openblock);

}

/******************************************************************************/

extern void SpriteWin_RemoveImageBlock(int imageno)
{
 /*
  * removes the image bank info block 'block' from the list of image banks
  */

  imageblock[imageno] = NULL;

}

/******************************************************************************/

extern void SpriteWin_CloseSaveWidget5(void)
{
  Dialog2_CloseDialog(savewidget5);
}

/******************************************************************************/

extern void SpriteWin_CloseSave(void)
{
  Dialog2_CloseDialog(save);
}

/******************************************************************************
 *                                open handlers                               *
 ******************************************************************************/

extern BOOL SpriteWin_Create(wimp_point *res, int imageno)
{
 /*
  * creates image at size 'res' in image bank 'imageno' and displays the window
  * exits quietly if the image bank is already used
  */
  spritewin_block *block;

 /*
  * return if bank in use
  */
  if(imageblock[imageno] != NULL)
     return(FALSE);

 /*
  * allocate memory for image bank info block
  * return FALSE if fails
  */
  block = malloc(sizeof(spritewin_block));
  if(block == NULL){
     NotEnoughMem();
     return(FALSE);
  }
  SpriteWin_InitBlock(block, imageno);

 /*
  * add image bank info block to list
  */
  SpriteWin_AddImageBlock(block, imageno);
  
 /*
  * create image but if fails the deallocate memory for image bank info block
  */
  if( !SpriteWin_CreateImage(block, res) ){
     SpriteWin_RemoveImageBlock(imageno);
     free(block);
     return(FALSE);
  }

  SpriteWin_CreateWindow(block);

  return(TRUE);
}

/******************************************************************************/

extern BOOL SpriteWin_CreateImage(spritewin_block *block, wimp_point *res)
{
 /*
  * creates image in image bank info block 'block'
  * returns FALSE if fails
  */
  int size;
  sprite_header *header;
  palette_entry *palette;
  int colour;
  char *image;
  int imagesize;

 /*
  * return if image already used
  */
  if(block->spritearea != NULL)
     return(FALSE);

 /*
  * work out the memory required for the sprite area
  * total memory is 'size' bytes for sprite area
  * 'imagesize' is size of image pixel data in bytes
  */
  size = Sprite_MemorySize(res->x, res->y, Mode, sprite_HASNOMASKPAL);
  imagesize = size - sizeof(sprite_header);
  size += sizeof(sprite_areainfo); /* add sprite area header size */
  size += sizeof_PALETTE; /* add 256 colour palette size */

 /*
  * allocate memory for sprite area
  */
  flex_alloc((flex_ptr) &block->spritearea, size);
  if(block->spritearea == NULL){
     NotEnoughMem();
     return(FALSE);
  }

 /*
  * initialise sprite area header for no sprites
  */
  (block->spritearea)->areasize = size;
  (block->spritearea)->numsprites = 0;
  (block->spritearea)->firstoffset = 16;
  (block->spritearea)->freeoffset = 16;

 /*
  * create blank sprite with no palette in sprite area
  */
  Sprite_Create(block->spritearea, SpriteName, sprite_HASNOMASKPAL, res->x, res->y, Mode);

 /*
  * 'header' points to start of sprite header
  * 'palette' points to start of sprite palette
  * 'image' points to start of image pixel data
  */
  header = (void *) ((char *)block->spritearea + (block->spritearea)->firstoffset);
  palette = (void *) ((char *)header + sizeof(sprite_header));
  image = (void *) ((char *)palette + sizeof_PALETTE);

 /*
  * insert 256 entry palette into sprite
  */
  memmove(image, image - sizeof_PALETTE, imagesize);

  header->imageoffset += sizeof_PALETTE;
  header->maskoffset += sizeof_PALETTE;
  header->offset_next += sizeof_PALETTE;
  block->spritearea->freeoffset += sizeof_PALETTE;

 /*
  * set palette to 256 level greyscale
  */
  for(colour = 0; colour < Colours; colour++){

      palette->value = GreyscalePalette[colour].value;
      palette++;
      palette->value = GreyscalePalette[colour].value;
      palette++;
  }

 /*
  * initialise rest of image bank info block
  * scale is set to 1:1
  * plot position in window and set so that image is lined up with edges
  */
  block->scale.multiply.x = 1;
  block->scale.multiply.y = 1;
  block->scale.divide.x = 1;
  block->scale.divide.y = 1;
  update_screenscale(block);
  block->plotcoord.x = 0;
  block->plotcoord.y = -(res->y * 2);
  block->size = *res;
  block->header =  (char *) header - (char *) block->spritearea;
  block->image =  (char *) image - (char *) block->spritearea;
  block->linebytes = (header->width + 1) * 4;

  return(TRUE);
}

/******************************************************************************/

extern void SpriteWin_CreateWindow(spritewin_block *block)
{
 /*
  * create window for image bank info block 'block'
  */
  window_info windowinfo;
  wimp_box newworkarea;
  char windowtitle[10];

 /*
  * create window, set size
  */
  block->window = Window_Create("Image", template_TITLEMIN);
  Window_GetInfo(block->window, &windowinfo);
  newworkarea = windowinfo.block.workarearect;
  newworkarea.min.y = 0 - (block->size.y * 2);
  newworkarea.max.x = block->size.x * 2;
  Wimp_SetExtent(block->window, &newworkarea);

 /*
  * set window title to be "Image X" where X is the image bank number
  */
  sprintf(windowtitle, "Image %i", block->imageno);
  Window_SetTitle(block->window, windowtitle);

 /*
  * initialise pixel translation table for current screen palette
  */
  build_pixtranstable(block);

 /*
  * install event handlers and display window
  */
  Event_Claim(event_REDRAW, block->window, event_ANY, Redraw, block);
  Event_Claim(event_CLOSE, block->window, event_ANY, CloseWindow, block);
  Event_Claim(event_CLICK, block->window, event_ANY, Click, block);
  EventMsg_Claim(message_PALETTECHANGE, block->window, BuildPixTransTable, block);
  EventMsg_Claim(message_MODECHANGE, block->window, BuildPixTransTable, block);
  EventMsg_Claim(message_MODECHANGE, block->window, ModeChange, block);
  Window_Show(block->window, open_WHEREVER);

}

/******************************************************************************
 *                               close handlers                               *
 ******************************************************************************/

static BOOL CloseWindow(event_pollblock *event, void *reference)
{
 /*
  * called when window is closed to delete the window, free the memory used to
  * store the image and set the image bank as unused
  */
  spritewin_block *block = reference;

  Window_Delete(block->window);

  SpriteWin_RemoveImageBlock(block->imageno);

  flex_free((flex_ptr) &block->spritearea);

  free(block);
  
  return(TRUE);
}

/******************************************************************************
 *                              Click handlers                                *
 ******************************************************************************/

static BOOL Click(event_pollblock *event, void *reference)
{
  spritewin_block *block = reference;
  char menutitle[10];

  currentimageno = block->imageno;

  if(event->data.mouse.button.data.menu){
    /*
     * set menu title to reflect image number of image bank
     */
     sprintf(menutitle, "Image %i", block->imageno);
     strncpy(imagewindowmenu->title, menutitle, wimp_MAXNAME);

    /*
     * release old menuwarn event handler
     */
     EventMsg_Release(message_MENUWARNING, event_ANY, Menuwarn);

    /*
     * install new menuwarn event handler with correct pointer to image bank info block
     */
     EventMsg_Claim(message_MENUWARNING, event_ANY, Menuwarn, block);

    /*
     * grey out save widget4 if it is not a 256x256 or 512x512 image
     */
     if( ((block->size.x == 256) && (block->size.y == 256)) ||
         ((block->size.x == 512) && (block->size.y == 512)) )
        Menu_SetFlags(savemenu, savemenu_WIDGET4, 0, 0); /* not shaded */
     else
        Menu_SetFlags(savemenu, savemenu_WIDGET4, 0, 1); /* shaded */

     Menu_Show(imagewindowmenu, event->data.mouse.pos.x,event->data.mouse.pos.y);
  }


/*
  if(event->data.mouse.button.data.select)

  if(event->data.mouse.button.data.adjust)
*/

  return(TRUE);
}

/******************************************************************************/

static BOOL Click_zoomwindow(event_pollblock *event, void *reference)
{
  spritewin_block *block = reference;

  switch(event->data.mouse.icon)
  {
    case 0: /* up multiplier */
       if(event->data.mouse.button.data.select)
          inc_icon(zoomwindow->window, 4, scale_MAX);
       if(event->data.mouse.button.data.adjust)
          dec_icon(zoomwindow->window, 4, scale_MIN);
       break;

    case 1: /* down multiplier */
       if(event->data.mouse.button.data.select)
          dec_icon(zoomwindow->window, 4, scale_MIN);
       if(event->data.mouse.button.data.adjust)
          inc_icon(zoomwindow->window, 4, scale_MAX);
       break;

    case 2: /* up divisor */
       if(event->data.mouse.button.data.select)
          inc_icon(zoomwindow->window, 5, scale_MAX);
       if(event->data.mouse.button.data.adjust)
          dec_icon(zoomwindow->window, 5, scale_MIN);
       break;

    case 3: /* down divisor */
       if(event->data.mouse.button.data.select)
          dec_icon(zoomwindow->window, 5, scale_MIN);
       if(event->data.mouse.button.data.adjust)
          inc_icon(zoomwindow->window, 5, scale_MAX);
       break;
  }

  if((event->data.mouse.icon >= 0) && (event->data.mouse.icon <= 3))
       scale_image(Icon_GetInteger(zoomwindow->window, 4),
                   Icon_GetInteger(zoomwindow->window, 5), block);
     
  return(TRUE);
}

/******************************************************************************/

static BOOL Click_savesprite(event_pollblock *event, void *reference)
{
  spritewin_block *block = reference;

  switch(event->data.mouse.icon)
  {
    case 0: /* file icon */
      if(event->data.mouse.button.value == button_DRAGSELECT){
         LoadSave_SetSaveBank(block->imageno, 256);
         ATrans_startdrag(Icon_GetTextPtr(save->window, 1),
                          filetype_SPRITE, 1024, save->window, 0, usericonsprites);
         }
      break;
  
    case 2: /* save */
      if(strchr(Icon_GetTextPtr(save->window, 1), '.') == 0)
         DragToSave();
      else{
         LoadSave_SetSaveBank(block->imageno, 256);
         LoadSave_savefile(Icon_GetTextPtr(save->window, 1), filetype_SPRITE, TRUE);
        if(!event->data.mouse.button.data.adjust)
           Dialog2_CloseDialog(save);
      }
      break;

    case 3: /* cancel */
      Dialog2_CloseDialog(save);
      break;
  }

     
  return(TRUE);
}

/******************************************************************************/

static BOOL Click_savewidget4(event_pollblock *event, void *reference)
{
  spritewin_block *block = reference;

  switch(event->data.mouse.icon)
  {
    case 0: /* file icon */
      if(event->data.mouse.button.value == button_DRAGSELECT){
         LoadSave_SetSaveBank(block->imageno, 256);
         ATrans_startdrag(Icon_GetTextPtr(save->window, 1),
                          filetype_WIDGET4, 1024, save->window, 0, usericonsprites);
         }
      break;
  
    case 2: /* save */
      if(strchr(Icon_GetTextPtr(save->window, 1), '.') == 0)
         DragToSave();
      else{
         LoadSave_SetSaveBank(block->imageno, 256);
         LoadSave_savefile(Icon_GetTextPtr(save->window, 1), filetype_WIDGET4, TRUE);
        if(!event->data.mouse.button.data.adjust)
           Dialog2_CloseDialog(save);
      }
      break;

    case 3: /* cancel */
      Dialog2_CloseDialog(save);
      break;
  }

     
  return(TRUE);
}

/******************************************************************************/

static BOOL Click_savewidget5(event_pollblock *event, void *reference)
{
  spritewin_block *block = reference;
  int numbergreys = 1;

  switch(event->data.mouse.icon)
  {
    case 0: /* file icon */
      if(event->data.mouse.button.value == button_DRAGSELECT){
         if(Icon_GetSelect(savewidget5->window, 4))
            numbergreys = 128;
         if(Icon_GetSelect(savewidget5->window, 5))
            numbergreys = 256;
         LoadSave_SetSaveBank(block->imageno, numbergreys);
         ATrans_startdrag(Icon_GetTextPtr(savewidget5->window, 1),
                          filetype_WIDGET5, 1024, savewidget5->window, 0,usericonsprites);
         }
      break;
  
    case 2: /* save */
      if(strchr(Icon_GetTextPtr(savewidget5->window, 1), '.') == 0)
         DragToSave();
      else{
         if(Icon_GetSelect(savewidget5->window, 4))
            numbergreys = 128;
         if(Icon_GetSelect(savewidget5->window, 5))
            numbergreys = 256;
         LoadSave_SetSaveBank(block->imageno, numbergreys);
         LoadSave_savefile(Icon_GetTextPtr(savewidget5->window, 1),filetype_WIDGET5,TRUE);
         if(!event->data.mouse.button.data.adjust)
            Dialog2_CloseDialog(savewidget5);
      }
      break;

    case 3: /* cancel */
      Dialog2_CloseDialog(savewidget5);
      break;

    case 4: case 5:
      Icon_Select(savewidget5->window, event->data.mouse.icon);
      break;

  }
     
  return(TRUE);
}

/******************************************************************************
 *                             keypress handlers                              *
 ******************************************************************************/

static BOOL Key_zoomwindow(event_pollblock *event, void *reference)
{
  spritewin_block *block = reference;
 
  if(event->data.key.caret.icon == 5){

     if(event->data.key.code == keycode_RETURN){

        scale_image(Icon_GetInteger(zoomwindow->window, 4),
                    Icon_GetInteger(zoomwindow->window, 5), block);

     }

  }

  return(TRUE);

}

/******************************************************************************/

static BOOL Key_savesprite(event_pollblock *event, void *reference)
{
  spritewin_block *block = reference;
 
  if(event->data.key.caret.icon == 1){

     if(event->data.key.code == keycode_RETURN){

        if(strchr(Icon_GetTextPtr(save->window, 1), '.') == 0)
           DragToSave();
        else{
           LoadSave_SetSaveBank(block->imageno, 256);
           LoadSave_savefile(Icon_GetTextPtr(save->window, 1), filetype_SPRITE, TRUE);
           Dialog2_CloseDialog(save);
        }

     }

  }

  return(TRUE);

}

/******************************************************************************/

static BOOL Key_savewidget5(event_pollblock *event, void *reference)
{
  spritewin_block *block = reference;
  int numbergreys;
 
  if(event->data.key.caret.icon == 1){

     if(event->data.key.code == keycode_RETURN){

        if(strchr(Icon_GetTextPtr(savewidget5->window, 1), '.') == 0)
           DragToSave();
        else{
           if(Icon_GetSelect(savewidget5->window, 4))
              numbergreys = 128;
           if(Icon_GetSelect(savewidget5->window, 5))
              numbergreys = 256;
           LoadSave_SetSaveBank(block->imageno, numbergreys);
         LoadSave_savefile(Icon_GetTextPtr(savewidget5->window, 1),filetype_WIDGET5,TRUE);
           Dialog2_CloseDialog(savewidget5);
        }

     }

  }

  return(TRUE);

}

/******************************************************************************/

static BOOL Key_savewidget4(event_pollblock *event, void *reference)
{
  spritewin_block *block = reference;
 
  if(event->data.key.caret.icon == 1){

     if(event->data.key.code == keycode_RETURN){

        if(strchr(Icon_GetTextPtr(save->window, 1), '.') == 0)
           DragToSave();
        else{
           LoadSave_SetSaveBank(block->imageno, 256);
           LoadSave_savefile(Icon_GetTextPtr(save->window, 1), filetype_WIDGET4, TRUE);
           Dialog2_CloseDialog(save);
        }

        Icon_SetCaret(zoomwindow->window, 4);
     }

  }

  return(TRUE);

}

/******************************************************************************
 *                            MenuWarn handlers                               *
 ******************************************************************************/

static BOOL Menuwarn(event_pollblock *event, void *reference)
{
  spritewin_block *block = reference;

  if(menu_currentopen != imagewindowmenu)
     return(FALSE);

  switch(event->data.message.data.menuwarn.selection[0])
  {

    case imagewindowmenu_IMAGEINFO:
      Dialog2_OpenDialogMenuLeaf(event, imageinfo);
      Icon_SetInteger(imageinfo->window, 3, block->size.x);
      Icon_SetInteger(imageinfo->window, 4, block->size.y);
      Icon_printf(imageinfo->window, 5, "%i K",
                          flex_size((flex_ptr) &block->spritearea) / 1024);
      return(TRUE);
      break;

    case imagewindowmenu_SAVE:

      if(fullversion){

         switch(event->data.message.data.menuwarn.selection[1])
         {
           case savemenu_SPRITE:
             Dialog2_OpenDialogMenuLeaf(event, save);
             Icon_SetText(save->window, 1, "Sprite");
             Icon_SetFileType(save->window, 0, filetype_SPRITE);
             Event_Claim(event_CLICK, save->window, event_ANY, Click_savesprite, block);
             Event_Claim(event_KEY, save->window, event_ANY, Key_savesprite, block);
             return(TRUE);
             break;
                                                                    
           case savemenu_WIDGET5:
             Dialog2_OpenDialogMenuLeaf(event, savewidget5);
             Icon_Select(savewidget5->window, 5); /* default to 256 colours */
           Event_Claim(event_CLICK,savewidget5->window,event_ANY,Click_savewidget5,block);
             Event_Claim(event_KEY, savewidget5->window, event_ANY,Key_savewidget5,block);
             return(TRUE);
             break;

           case savemenu_WIDGET4:
             Dialog2_OpenDialogMenuLeaf(event, save);
             Icon_SetText(save->window, 1, "Widget4");
             Icon_SetFileType(save->window, 0, filetype_WIDGET4);
             Event_Claim(event_CLICK, save->window, event_ANY, Click_savewidget4, block);
             Event_Claim(event_KEY, save->window, event_ANY, Key_savewidget4, block);
             return(TRUE);
             break;
         }
      }

      else{ /* just open savesprite instead of save type menu */
         Dialog2_OpenDialogMenuLeaf(event, save);
         Icon_SetText(save->window, 1, "Sprite");
         Icon_SetFileType(save->window, 0, filetype_SPRITE);
         Event_Claim(event_CLICK, save->window, event_ANY, Click_savesprite, block);
         Event_Claim(event_KEY, save->window, event_ANY, Key_savesprite, block);
         return(TRUE);
      }
      break;

    case imagewindowmenu_ZOOM:
      Dialog2_OpenDialogMenuLeaf(event, zoomwindow);
      Event_Claim(event_CLICK, zoomwindow->window, event_ANY, Click_zoomwindow, block);
      Event_Claim(event_KEY, zoomwindow->window, event_ANY, Key_zoomwindow, block);
      Icon_SetInteger(zoomwindow->window, 4, block->scale.multiply.x);
      Icon_SetInteger(zoomwindow->window, 5, block->scale.divide.x);
      Icon_SetCaret(zoomwindow->window, 4);
      return(TRUE);
      break;
      
  }

  return(FALSE);
}

/******************************************************************************
 *                           menu choice handlers                             *
 ******************************************************************************/

static BOOL MenuChoice(event_pollblock *event, void *reference)
{
  mouse_block ptr;

  if (menu_currentopen != imagewindowmenu) /* Only respond to the imagewindowmenu */
    return(FALSE);


  switch(event->data.selection[0])
  {
     case imagewindowmenu_PROCESS:
        if(event->data.selection[1] != -1)
           ProcessWin_Start(event->data.selection[1], currentimageno);
        break;

     case imagewindowmenu_GRAPH:
        if(event->data.selection[1] != -1)
           GraphWin_Start(event->data.selection[1], currentimageno);
        break;

  }

  Wimp_GetPointerInfo(&ptr);               /* If ADJUST was used to select, */
  if (ptr.button.data.adjust)              /* then we keep the menu open.   */
    Menu_ShowLast();

  return(TRUE);
}

/******************************************************************************
 *                        palette change handlers                             *
 ******************************************************************************/

static BOOL BuildPixTransTable(event_pollblock *event, void *reference)
{
  /*
   * called on palette change to recalculate pixel translation table
   */

   build_pixtranstable((spritewin_block *)reference);

   return(FALSE);
}

/******************************************************************************/

void build_pixtranstable(spritewin_block *block)
{
 /*
  * calculates pixel translation table for current screen palette
  * code taken from PRMs Vol. 1 Page 834
  */
  sprite_header *header;
  palette_entry *palette;
  int q;
  _kernel_swi_regs r;

  header = (void *) ((char *)block->spritearea + (block->spritearea)->firstoffset);

  if(header->imageoffset == 44){

     palette = 0;
  }
  else{

     palette = (palette_entry *)&GreyscalePalette;
  }

  for(q = 0; q < 256; q++){

     block->pixtransbuffer[q] = q;

  }

  if(header->imageoffset == 44 + 2048){

     for(q = 0; q < 256; q++){

         r.r[0] = (int)GreyscalePalette[q].value;
         Error_Check((os_error *)
                 _kernel_swi( SWI_XOS_Bit | SWI_ColourTrans_ReturnColourNumber, &r, &r ));
         block->pixtransbuffer[q] = r.r[0];
     }
  }
  else{

     r.r[0] = Mode;
     r.r[1] = (int)palette;
     r.r[2] = -1;
     r.r[3] = -1;
     r.r[4] = (int)&block->pixtransbuffer;
     Error_Check((os_error *)
                 _kernel_swi( SWI_XOS_Bit | SWI_ColourTrans_SelectTable, &r, &r ));
  }

  block->pixtranstable = (char *)-1;
  for(q = 0; q < 256; q++){

      if(block->pixtransbuffer[q] != q)
         block->pixtranstable = (char *)&block->pixtransbuffer;
  }
     
}

/******************************************************************************
 *                          mode change handlers                              *
 ******************************************************************************/

static BOOL ModeChange(event_pollblock *event, void *reference)
{
  spritewin_block *block = reference;

  update_screenscale(block);

  return(FALSE);
}

/******************************************************************************
 *                            redraw handlers                                 *
 ******************************************************************************/

static BOOL Redraw(event_pollblock *event, void *reference)
{
 /*
  * called to redraw the image window
  */
  window_redrawblock redraw;
  BOOL more;
  spritewin_block *block = reference;
  _kernel_swi_regs r;

  redraw.window = event->data.openblock.window;
  Wimp_RedrawWindow(&redraw, &more);

  while (more)
  {

    r.r[0] = 52 + 256;
    r.r[1] = (int)block->spritearea;
    r.r[2] = (int)SpriteName;
    r.r[3] = (block->plotcoord.x - redraw.scroll.x) + redraw.rect.min.x;
    r.r[4] = (block->plotcoord.y - redraw.scroll.y) + redraw.rect.max.y;
    r.r[5] = 0;
    r.r[6] = (int)&block->screenscale;
    r.r[7] = (int)block->pixtranstable;
    
    Error_Check((os_error *)
              _kernel_swi( SWI_XOS_Bit | SWI_OS_SpriteOp , &r, &r ));
                                                        
    Wimp_GetRectangle(&redraw, &more);
  }

  return(TRUE);
}

/******************************************************************************
 *                             misc routines                                  *
 ******************************************************************************/

void NotEnoughMem(void)
{
  char err_msg[128];

  Msgs_Lookup("err.nomemspr", err_msg, 127);
  Error_Report(1,err_msg);

}

/******************************************************************************/

void DragToSave(void)
{
  char msg[128];

  Msgs_Lookup("msg.noleaf", msg, 127);
  Error_Report(1,msg);

}

/******************************************************************************/

void scale_image(int multiplier, int divisor, spritewin_block *block)
{
  window_info windowinfo;
  wimp_box newworkarea;
  window_state windowstate;
  
 /*
  * change scaleing factors for image
  */
  block->scale.multiply.x = multiplier;
  block->scale.multiply.y = multiplier;
  block->scale.divide.x = divisor;
  block->scale.divide.y = divisor;
  update_screenscale(block);

 /*
  * change window area to reflect new image size
  */
  Window_GetInfo(block->window, &windowinfo);
  newworkarea = windowinfo.block.workarearect;
  newworkarea.min.y = 0 - ( (block->size.y * 2 * multiplier) / divisor );
  newworkarea.max.x = (block->size.x * 2 * multiplier ) / divisor;
  Wimp_SetExtent(block->window, &newworkarea);

 /*
  * change plot position in window to reflect new image size
  */
  block->plotcoord.x = 0;
  block->plotcoord.y = -( (block->size.y * 2 * multiplier) / divisor );

 /*
  * refresh image and window size (the sad way)
  */
  Wimp_GetWindowState(block->window, &windowstate);
  Wimp_CloseWindow(block->window);
  Wimp_OpenWindow(&windowstate.openblock);
  
}

/******************************************************************************/

void update_screenscale(spritewin_block *block)
{
  Screen_CacheModeInfo();

  switch(screen_eig.x)
  {
    case 0:
      block->screenscale.multiply.x = block->scale.multiply.x * 2;
      block->screenscale.divide.x = block->scale.divide.x;
      break;

    case 1:
      block->screenscale.multiply.x = block->scale.multiply.x;
      block->screenscale.divide.x = block->scale.divide.x;
      break;

    case 2:
      block->screenscale.multiply.x = block->scale.multiply.x;
      block->screenscale.divide.x = block->scale.divide.x * 2;
      break;
  }

  switch(screen_eig.y)
  {
    case 0:
      block->screenscale.multiply.y = block->scale.multiply.y * 2;
      block->screenscale.divide.y = block->scale.divide.y;
      break;

    case 1:
      block->screenscale.multiply.y = block->scale.multiply.y;
      block->screenscale.divide.y = block->scale.divide.y;
      break;

    case 2:
      block->screenscale.multiply.y = block->scale.multiply.y;
      block->screenscale.divide.y = block->scale.divide.y * 2;
      break;
  }

}

