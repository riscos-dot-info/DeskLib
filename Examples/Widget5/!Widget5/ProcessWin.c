/************************************************************************
 *                                                                      *
 *        File: ProcessWin.c                                            *
 *                                                                      *
 *     Purpose: Provides user interface to image processing functions   *
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

#include "SpriteWin.h"
#include "CheckBanks.h"
#include "Process.h"
#include "Filter.h"
#include "MixWin.h"
#include "GreyEdWin.h"
#include "Misc.h"

#include <stdlib.h>
#include <stdio.h>

extern window_handle db;

typedef struct {
   iconrange_block source2range;
   iconrange_block destrange;
} common2_block;

/* definitions for 'process' */
#define process_COPY        0
#define process_ADD         1
#define process_AVERAGE     2
#define process_MIX         3
#define process_CONVOLUTION 4
#define process_MEDIAN      5
#define process_GREYEDIT    6
#define process_SCALE       7


/* icon block for dest images */
#define image_MIN 8
#define image_MAX 13

/* icon blocks for twin source windows */
#define sourceimage2_MIN 8
#define sourceimage2_MAX 13
#define desttwinimage_MIN 16
#define desttwinimage_MAX 21

/* icon block for size radios in scale window */
#define size_MIN 16
#define size_MAX 21

/* default values for scale window */
static wimp_point tofit;
static sprite_scalefactors custom;
static int selectedsize;

void NoMemProc(void);
void InvAnchor(void);

void open_copy(int sourceimage);
void open_add(int sourceimage);
void open_average(int sourceimage);
void open_convolution(int sourceimage);
void open_median(int sourceimage);
void open_greyedit(int sourceimage);
void open_scale(int sourceimage);
void open_mix(int sourceimage);

void prepare_processwin(iconrange_block *range);

static BOOL Click_common(event_pollblock *event, void *reference);
static BOOL Click_common2(event_pollblock *event, void *reference);
static BOOL Click_convolution(event_pollblock *event, void *reference);
static BOOL Click_scale(event_pollblock *event, void *reference);

static BOOL Key_convolution(event_pollblock *event, void *reference);
static BOOL Key_scale(event_pollblock *event, void *reference);

static void close_copy(BOOL ok, BOOL keepopen, iconrange_block *range);
static void close_add(BOOL ok, BOOL keepopen, iconrange_block *range);
static void close_average(BOOL ok, BOOL keepopen, iconrange_block *range);
static void close_convolution(BOOL ok, BOOL keepopen, iconrange_block *range);
static void close_median(BOOL ok, BOOL keepopen, iconrange_block *range);
static void close_greyedit(BOOL ok, BOOL keepopen, iconrange_block *range);
static void close_scale(BOOL ok, BOOL keepopen, iconrange_block *range);
static void close_mix(BOOL ok, BOOL keepopen, common2_block *block);

static BOOL Close_window(event_pollblock *event, void *reference);
static BOOL Close_window2(event_pollblock *event, void *reference);

static BOOL Null_closecommon2(event_pollblock *event, void *reference);

/******************************************************************************
 *                            extern routines                                 *
 ******************************************************************************/

extern void ProcessWin_Init(void)
{
 /*
  * initialisation to be done on startup
  */

 /*
  * setup default vaules for scale window
  */
  tofit.x = 1280;
  tofit.y = 1024;

  custom.multiply.x = 1;
  custom.divide.x = 1;
  custom.multiply.y = 1;
  custom.divide.y = 2;

  selectedsize = 17;
 
}

/******************************************************************************/

extern void ProcessWin_Start(int process, int sourceimage)
{

  switch(process)
  {
    case process_COPY:
      open_copy(sourceimage);
      break;

    case process_ADD:
      open_add(sourceimage);
      break;

    case process_AVERAGE:
      open_average(sourceimage);
      break;

    case process_MIX:
      open_mix(sourceimage);
      break;

    case process_CONVOLUTION:
      open_convolution(sourceimage);
      break;

    case process_MEDIAN:
      open_median(sourceimage);
      break;

    case process_GREYEDIT:
      open_greyedit(sourceimage);
      break;
      
    case process_SCALE:
      open_scale(sourceimage);
      break;

    default:
      Error_Report(1, "sel %i   image %i", process, sourceimage);
      break;
  }

}

/******************************************************************************
 *                              Click handlers                                *
 ******************************************************************************/

static BOOL Click_common2(event_pollblock *event, void *reference)
{
  common2_block *block = reference;

  switch(event->data.mouse.icon)
  {
    
    case 22: /* Cancel */
       (block->source2range.close)(FALSE, FALSE, block);
       break;

    case 23: /* Action */
       (block->source2range.close)(TRUE, event->data.mouse.button.data.adjust, block);
       break;
      
  }
  return(TRUE);
}

/******************************************************************************/

static BOOL Click_common(event_pollblock *event, void *reference)
{
  iconrange_block *range = reference;

  switch(event->data.mouse.icon)
  {
    
    case 14: /* Cancel */
       (range->close)(FALSE, FALSE, range);
       break;

    case 15: /* Action */
       (range->close)(TRUE, event->data.mouse.button.data.adjust, range);
       break;
      
  }
  return(TRUE);
}

/******************************************************************************/

static BOOL Click_convolution(event_pollblock *event, void *reference)
{
  iconrange_block *range = reference;

  switch(event->data.mouse.icon)
  {
    
    case 32: /* Cancel */
       close_convolution(FALSE, FALSE, range);
       break;

    case 33: /* Filter */
       close_convolution(TRUE, event->data.mouse.button.data.adjust, range);
       break;
      
  }
  return(TRUE);
}

/******************************************************************************/

static BOOL Click_scale(event_pollblock *event, void *reference)
{
  iconrange_block *range = reference;

  if( (event->data.mouse.icon >= size_MIN) && (event->data.mouse.icon <= size_MAX) ){
     Icon_Select(range->window, event->data.mouse.icon);
     return(TRUE);
  }

  switch(event->data.mouse.icon)
  {
    
    case 31: /* Cancel */
       close_scale(FALSE, FALSE, range);
       break;

    case 32: /* Scale */
       close_scale(TRUE, event->data.mouse.button.data.adjust, range);
       break;
      
  }
  return(TRUE);
}

/******************************************************************************
 *                             keypress handlers                              *
 ******************************************************************************/

static BOOL Key_convolution(event_pollblock *event, void *reference)
{
  iconrange_block *range = reference;

  if(event->data.key.caret.icon == 31){

     if(event->data.key.code == keycode_RETURN){
        close_convolution(TRUE, FALSE, range);
        return(TRUE);
     }

  }

  return(FALSE);

}

/******************************************************************************/

static BOOL Key_scale(event_pollblock *event, void *reference)
{
  iconrange_block *range = reference;

  if(event->data.key.caret.icon == 30){

     if(event->data.key.code == keycode_RETURN){
        close_scale(TRUE, FALSE, range);
        return(TRUE);
     }

  }

  return(FALSE);

}

/******************************************************************************
 *                                open handlers                               *
 ******************************************************************************/

void prepare_processwin(iconrange_block *range)
{
 /*
  * set text in window and start destination bank scanning
  */
  wimp_point *sourcesize;
  char text[20];

 /*
  * imagebank number
  */
  Msgs_Lookup("txt.imbank", text, 19);
  Icon_printf(range->window, 2, "%s %i", text, range->sourceimage);

 /*
  * image bank size
  */
  sourcesize = SpriteWin_GetSize(range->sourceimage);
  Icon_SetInteger(range->window, 3, sourcesize->x);
  Icon_SetInteger(range->window, 5, sourcesize->y);

 /*
  * initialise the available banks and start the scanning procedure
  */
  range->size.x = sourcesize->x;
  range->size.y = sourcesize->y;
  range->min = image_MIN;
  range->max = image_MAX;
  CheckBanks_InstallScanDestination(range);

}

/******************************************************************************/

void prepare_processwin2(common2_block *block)
{
 /*
  * set text in window and start destination bank scanning
  */
  wimp_point *sourcesize;
  char text[20];

 /*
  * imagebank number
  */
  Msgs_Lookup("txt.imbank", text, 19);
  Icon_printf(block->destrange.window, 2, "%s %i", text, block->destrange.sourceimage);

 /*
  * image bank size
  */
  sourcesize = SpriteWin_GetSize(block->destrange.sourceimage);
  Icon_SetInteger(block->destrange.window, 3, sourcesize->x);
  Icon_SetInteger(block->destrange.window, 5, sourcesize->y);

 /*
  * initialise the available banks and start the scanning procedure
  */
  block->source2range.size.x = sourcesize->x;
  block->source2range.size.y = sourcesize->y;
  block->source2range.min = sourceimage2_MIN;
  block->source2range.max = sourceimage2_MAX;
  CheckBanks_InstallScanSource(&(block->source2range));

  block->destrange.size.x = sourcesize->x;
  block->destrange.size.y = sourcesize->y;
  block->destrange.min = desttwinimage_MIN;
  block->destrange.max = desttwinimage_MAX;
  CheckBanks_InstallScanDestination(&(block->destrange));

  Event_Claim(event_NULL, event_ANY, event_ANY, Null_closecommon2, block);

}

/******************************************************************************/

void open_copy(int sourceimage)
{
 /*
  * open copy image window on screen
  */
  iconrange_block *range;
  char text[40];

 /*
  * allocate memory for range info block and return if fails
  */
  range = malloc(sizeof(iconrange_block));
  if(range == NULL){
     NoMemProc();
     return;
  }

 /*
  * create the window
  */
  range->window = Window_Create("Common", template_TITLEMIN);
  Msgs_Lookup("txt.copyt", text, 39);
  Window_SetTitle(range->window, text);
  Msgs_Lookup("txt.copy", text, 39);
  Icon_SetText(range->window, 15, text);

 /*
  * set routine that closes window when source bank is closed
  */
  range->sourceimage = sourceimage;
  range->close = (closewindow_routine)close_copy;

 /*
  * initialise source and destination image frames
  */
  prepare_processwin(range);

 /*
  * install event handlers
  */
  Event_Claim(event_CLICK, range->window, event_ANY, Click_common, range);

  Event_Claim(event_CLOSE, range->window, event_ANY, Close_window, range);
  
  Window_Show(range->window, open_UNDERPOINTER);

}

/******************************************************************************/

void open_add(int sourceimage)
{
 /*
  * open add new image window on screen
  */
  iconrange_block *range;
  char text[40];

 /*
  * allocate memory for range info block and return if fails
  */
  range = malloc(sizeof(iconrange_block));
  if(range == NULL){
     NoMemProc();
     return;
  }

 /*
  * create the window
  */
  range->window = Window_Create("Common", template_TITLEMIN);
  Msgs_Lookup("txt.addt", text, 39);
  Window_SetTitle(range->window, text);
  Msgs_Lookup("txt.add", text, 39);
  Icon_SetText(range->window, 15, text);

 /*
  * set routine that closes window when source bank is closed
  */
  range->sourceimage = sourceimage;
  range->close = (closewindow_routine)close_add;

 /*
  * initialise source and destination image frames
  */
  prepare_processwin(range);

 /*
  * install event handlers
  */
  Event_Claim(event_CLICK, range->window, event_ANY, Click_common, range);

  Event_Claim(event_CLOSE, range->window, event_ANY, Close_window, range);
  
  Window_Show(range->window, open_UNDERPOINTER);

}

/******************************************************************************/

void open_average(int sourceimage)
{
 /*
  * open average image window on screen
  */
  iconrange_block *range;
  char text[40];

 /*
  * allocate memory for range info block and return if fails
  */
  range = malloc(sizeof(iconrange_block));
  if(range == NULL){
     NoMemProc();
     return;
  }

 /*
  * create the window
  */
  range->window = Window_Create("Common", template_TITLEMIN);
  Msgs_Lookup("txt.avrt", text, 39);
  Window_SetTitle(range->window, text);
  Msgs_Lookup("txt.avr", text, 39);
  Icon_SetText(range->window, 15, text);

 /*
  * set routine that closes window when source bank is closed
  */
  range->sourceimage = sourceimage;
  range->close = (closewindow_routine)close_average;

 /*
  * initialise source and destination image frames
  */
  prepare_processwin(range);

 /*
  * install event handlers
  */
  Event_Claim(event_CLICK, range->window, event_ANY, Click_common, range);

  Event_Claim(event_CLOSE, range->window, event_ANY, Close_window, range);
  
  Window_Show(range->window, open_UNDERPOINTER);

}

/******************************************************************************/

void open_convolution(int sourceimage)
{
 /*
  * open convolute image window on screen
  */
  iconrange_block *range;

 /*
  * allocate memory for range info block and return if fails
  */
  range = malloc(sizeof(iconrange_block));
  if(range == NULL){
     NoMemProc();
     return;
  }

 /*
  * create the window
  */
  range->window = Window_Create("Convolution", template_TITLEMIN);

 /*
  * set routine that closes window when source bank is closed
  */
  range->sourceimage = sourceimage;
  range->close = (closewindow_routine)close_convolution;

 /*
  * initialise source and destination image frames
  */
  prepare_processwin(range);

 /*
  * install filter menu
  */
  Filter_InstallMenu(range->window);

 /*
  * install event handlers
  */
  Event_Claim(event_CLICK, range->window, event_ANY, Click_convolution, range);

  Event_Claim(event_KEY, range->window, event_ANY, Key_convolution, range);

  Event_Claim(event_CLOSE, range->window, event_ANY, Close_window, range);
  
  Window_Show(range->window, open_UNDERPOINTER);

 /*
  * put caret into first number of matrix
  */
  Icon_SetCaret(range->window, 16);

}

/******************************************************************************/

void open_median(int sourceimage)
{
 /*
  * open median filter window on screen
  */
  iconrange_block *range;
  char text[40];

 /*
  * allocate memory for range info block and return if fails
  */
  range = malloc(sizeof(iconrange_block));
  if(range == NULL){
     NoMemProc();
     return;
  }

 /*
  * create the window
  */
  range->window = Window_Create("Common", template_TITLEMIN);
  Msgs_Lookup("txt.medt", text, 39);
  Window_SetTitle(range->window, text);
  Msgs_Lookup("txt.filter", text, 39);
  Icon_SetText(range->window, 15, text);

 /*
  * set routine that closes window when source bank is closed
  */
  range->sourceimage = sourceimage;
  range->close = (closewindow_routine)close_median;

 /*
  * initialise source and destination image frames
  */
  prepare_processwin(range);

 /*
  * install event handlers
  */
  Event_Claim(event_CLICK, range->window, event_ANY, Click_common, range);

  Event_Claim(event_CLOSE, range->window, event_ANY, Close_window, range);
  
  Window_Show(range->window, open_UNDERPOINTER);

}

/******************************************************************************/

void open_greyedit(int sourceimage)
{
 /*
  * open edit image greylevels window
  */
  iconrange_block *range;
  char text[40];

 /*
  * allocate memory for range info block and return if fails
  */
  range = malloc(sizeof(iconrange_block));
  if(range == NULL){
     NoMemProc();
     return;
  }

 /*
  * create the window
  */
  range->window = Window_Create("Common", template_TITLEMIN);
  Msgs_Lookup("txt.greyedt", text, 39);
  Window_SetTitle(range->window, text);
  Msgs_Lookup("txt.start", text, 39);
  Icon_SetText(range->window, 15, text);

 /*
  * set routine that closes window when source bank is closed
  */
  range->sourceimage = sourceimage;
  range->close = (closewindow_routine)close_greyedit;

 /*
  * initialise source and destination image frames
  */
  prepare_processwin(range);

 /*
  * install event handlers
  */
  Event_Claim(event_CLICK, range->window, event_ANY, Click_common, range);

  Event_Claim(event_CLOSE, range->window, event_ANY, Close_window, range);
  
  Window_Show(range->window, open_UNDERPOINTER);

}

/******************************************************************************/

void open_scale(int sourceimage)
{
 /*
  * open scale image window on screen
  */
  iconrange_block *range;

 /*
  * allocate memory for range info block and return if fails
  */
  range = malloc(sizeof(iconrange_block));
  if(range == NULL){
     NoMemProc();
     return;
  }

 /*
  * create the window
  */
  range->window = Window_Create("Scale", template_TITLEMIN);

 /*
  * set routine that closes window when source bank is closed
  */
  range->sourceimage = sourceimage;
  range->close = (closewindow_routine)close_convolution;

 /*
  * initialise source and destination image frames
  */
  prepare_processwin(range);

 /*
  * set destination size to -1x-1 so that it cannot find windows of the same size
  */
  range->size.x = -1;
  range->size.y = -1;

 /*
  * initialise the default values
  */
  Icon_SetRadios(range->window, size_MIN, size_MAX, selectedsize);
  Icon_SetInteger(range->window, 22, tofit.x);
  Icon_SetInteger(range->window, 24, tofit.y);
  Icon_SetInteger(range->window, 25, custom.multiply.x);
  Icon_SetInteger(range->window, 27, custom.divide.x);
  Icon_SetInteger(range->window, 28, custom.multiply.y);
  Icon_SetInteger(range->window, 30, custom.divide.x);


 /*
  * install event handlers
  */
  Event_Claim(event_CLICK, range->window, event_ANY, Click_scale, range);

  Event_Claim(event_KEY, range->window, event_ANY, Key_scale, range);

  Event_Claim(event_CLOSE, range->window, event_ANY, Close_window, range);
  
  Window_Show(range->window, open_UNDERPOINTER);

 /*
  * put caret into first icon
  */
  Icon_SetCaret(range->window, 22);

}

/******************************************************************************/

void open_mix(int sourceimage)
{
 /*
  * open grey edit image window on screen
  */
  common2_block *block;
  char text[40];

 /*
  * allocate memory for range info block and return if fails
  */
  block = malloc(sizeof(common2_block));
  if(block == NULL){
     NoMemProc();
     return;
  }

 /*
  * create the window
  */
  block->source2range.window = Window_Create("Common2", template_TITLEMIN);
  block->destrange.window = block->source2range.window;
  Msgs_Lookup("txt.mixt", text, 39);
  Window_SetTitle(block->destrange.window, text);
  Msgs_Lookup("txt.start", text, 39);
  Icon_SetText(block->destrange.window, 23, text);

 /*
  * set routine that closes window when source bank is closed
  */
  block->source2range.sourceimage = sourceimage;
  block->destrange.sourceimage = sourceimage;
  block->source2range.close = (closewindow_routine)close_mix;
  block->destrange.close = NULL; /*(closewindow_routine)close_mix;*/

 /*
  * initialise source 1 + 2 and destination image frames
  */
  prepare_processwin2(block);

 /*
  * install event handlers
  */
  Event_Claim(event_CLICK, block->destrange.window, event_ANY, Click_common2, block);

  Event_Claim(event_CLOSE, block->destrange.window, event_ANY, Close_window2, block);
  
  Window_Show(block->destrange.window, open_UNDERPOINTER);

}

/******************************************************************************
 *                               close handlers                               *
 ******************************************************************************/

static void close_copy(BOOL ok, BOOL keepopen, iconrange_block *range)
{
  int destimage;
  wimp_point *size;

  if(ok == TRUE){

     destimage = image_getselect(range);

    /*
     * process the image
     */
     if(destimage != -1){
        size = SpriteWin_GetSize(range->sourceimage);

       /*
        * create destination if not created and return if creation fails
        */
        if( SpriteWin_FreeImage(destimage) ){

           if( !SpriteWin_Create(size, destimage) )
              return;
        }

        Process_Copy(range->sourceimage, destimage, size);

       /*
        * refresh destination image
        */
        SpriteWin_RefreshWin(destimage);
     }
  
  }

  if(keepopen == FALSE){

     CheckBanks_RemoveScanDestination(range);
     Window_Delete(range->window);
     free(range);
  }

}

/******************************************************************************/

static void close_add(BOOL ok, BOOL keepopen, iconrange_block *range)
{
  int destimage;
  wimp_point *size;

  if(ok == TRUE){

     destimage = image_getselect(range);

    /*
     * process the image
     */
     if(destimage != -1){
        size = SpriteWin_GetSize(range->sourceimage);

       /*
        * create destination if not created and return if creation fails
        */
        if( SpriteWin_FreeImage(destimage) ){

           if( !SpriteWin_Create(size, destimage) )
              return;
        }

        Process_Add(range->sourceimage, destimage, size);

       /*
        * refresh destination image
        */
        SpriteWin_RefreshWin(destimage);
     }
  
  }

  if(keepopen == FALSE){

     CheckBanks_RemoveScanDestination(range);
     Window_Delete(range->window);
     free(range);
  }

}

/******************************************************************************/

static void close_average(BOOL ok, BOOL keepopen, iconrange_block *range)
{
  int destimage;
  wimp_point *size;

  if(ok == TRUE){

     destimage = image_getselect(range);

    /*
     * process the image
     */
     if(destimage != -1){
        size = SpriteWin_GetSize(range->sourceimage);

       /*
        * create destination if not created and return if creation fails
        */
        if( SpriteWin_FreeImage(destimage) ){

           if( !SpriteWin_Create(size, destimage) )
              return;
        }

        Process_Average(range->sourceimage, destimage, size);

       /*
        * refresh destination image
        */
        SpriteWin_RefreshWin(destimage);
     }
  
  }

  if(keepopen == FALSE){

     CheckBanks_RemoveScanDestination(range);
     Window_Delete(range->window);
     free(range);
  }

}

/******************************************************************************/

static void close_convolution(BOOL ok, BOOL keepopen, iconrange_block *range)
{
  int destimage;
  wimp_point *size;
  wimp_point anchor;
  int filter[3][3];

  if(ok == TRUE){

     destimage = image_getselect(range);

    /*
     * process the image
     */
     if(destimage != -1){
        size = SpriteWin_GetSize(range->sourceimage);

        Filter_GetMatrix(range->window, &anchor, &filter);

       /*
        * check anchor point is valid
        */
        if( (anchor.x < -1) || (anchor.x > 1) || (anchor.y < -1) || (anchor.y > 1) ){
           InvAnchor();
           return;
        }

       /*
        * create destination if not created and return if creation fails
        */
        if( SpriteWin_FreeImage(destimage) ){

           if( !SpriteWin_Create(size, destimage) )
              return;
        }
        
        Process_Convolve(range->sourceimage, destimage, size, &anchor, &filter, TRUE);

       /*
        * refresh destination image
        */
        SpriteWin_RefreshWin(destimage);
     }
  
  }

  if(keepopen == FALSE){

     CheckBanks_RemoveScanDestination(range);
     Window_Delete(range->window);
     free(range);
  }

}

/******************************************************************************/

static void close_median(BOOL ok, BOOL keepopen, iconrange_block *range)
{
  int destimage;
  wimp_point *size;

  if(ok == TRUE){

     destimage = image_getselect(range);

    /*
     * process the image
     */
     if(destimage != -1){
        size = SpriteWin_GetSize(range->sourceimage);

       /*
        * create destination if not created and return if creation fails
        */
        if( SpriteWin_FreeImage(destimage) ){

           if( !SpriteWin_Create(size, destimage) )
              return;
        }

        Process_Median(range->sourceimage, destimage, size);

       /*
        * refresh destination image
        */
        SpriteWin_RefreshWin(destimage);
     }
  
  }

  if(keepopen == FALSE){

     CheckBanks_RemoveScanDestination(range);
     Window_Delete(range->window);
     free(range);
  }

}

/******************************************************************************/

static void close_greyedit(BOOL ok, BOOL keepopen, iconrange_block *range)
{
  int destimage;
  wimp_point openpos;

  if(ok == TRUE){

     destimage = image_getselect(range);

    /*
     * process the image
     */
     if(destimage != -1){

       /*
        * open process window
        */

        Window_GetPos(range->window, &openpos);
        GreyEdWin_Open(range->sourceimage, destimage, &openpos);

     }
  
  }

  if(keepopen == FALSE){

     CheckBanks_RemoveScanDestination(range);
     Window_Delete(range->window);
     free(range);
  }

}

/******************************************************************************/

static void close_scale(BOOL ok, BOOL keepopen, iconrange_block *range)
{
  int destimage;
  wimp_point *sourcesize;
  wimp_point destsize;
  wimp_point *actdestsize;

 /*
  * update the default values
  */
  selectedsize = Icon_WhichRadio(range->window, size_MIN, size_MAX);
  tofit.x = Icon_GetInteger(range->window, 22);
  tofit.y = Icon_GetInteger(range->window, 24);
  custom.multiply.x = Icon_GetInteger(range->window, 25); 
  custom.divide.x = Icon_GetInteger(range->window, 27);
  custom.multiply.y = Icon_GetInteger(range->window, 28);
  custom.divide.y = Icon_GetInteger(range->window, 30);

  if(ok == TRUE){

     destimage = image_getselect(range);

    /*
     * process the image
     */
     if(destimage != -1){
        sourcesize = SpriteWin_GetSize(range->sourceimage);

       /*
        * calculate the size of the destination image
        */
        switch( Icon_WhichRadio(range->window, size_MIN, size_MAX) - size_MIN)
        {
          case 0: /* 400% */    
            destsize.x = sourcesize->x * 4;
            destsize.y = sourcesize->y * 4;
            break;

          case 1: /* 200% */     
            destsize.x = sourcesize->x * 2;
            destsize.y = sourcesize->y * 2;
            break;

          case 2: /* 50% */     
            destsize.x = sourcesize->x / 2;
            destsize.y = sourcesize->y / 2;
            break;

          case 3: /* 25% */    
            destsize.x = sourcesize->x / 4;
            destsize.y = sourcesize->y / 4;
            break;

          case 4: /* to fit */  
            destsize.x = tofit.x;
            destsize.y = tofit.y;
            break;

          case 5: /* custom */  
            destsize.x = (sourcesize->x * custom.multiply.x) / custom.divide.x;
            destsize.y = (sourcesize->y * custom.multiply.y) / custom.divide.y;
            break;
        }
            
       /*
        * create destination and return if creation fails
        */
        if( SpriteWin_FreeImage(destimage) ){

           if( !SpriteWin_Create(&destsize, destimage) )
              return;
        }
        actdestsize = SpriteWin_GetSize(destimage);
        Process_Scale(range->sourceimage, destimage, sourcesize, actdestsize);

       /*
        * refresh destination image
        */
        SpriteWin_RefreshWin(destimage);
     }
  
  }

  if(keepopen == FALSE){

     CheckBanks_RemoveScanDestination(range);
     Window_Delete(range->window);
     free(range);
  }

}

/******************************************************************************/

static void close_mix(BOOL ok, BOOL keepopen, common2_block *block)
{
  int source2image;
  int destimage;
  BOOL oktokill;
  wimp_point openpos;

  oktokill = TRUE;

  if(ok == TRUE){

     source2image = image_getselect(&(block->source2range));
     destimage = image_getselect(&(block->destrange));

     if( (source2image != -1) && (destimage != -1) ){

       /*
        * open process window
        */

        Window_GetPos(block->destrange.window, &openpos);
        MixWin_Open(block->destrange.sourceimage, source2image, destimage, &openpos);
     }
     else
        oktokill = FALSE;

  }

 /*
  * kill window
  */

  if(oktokill){

     CheckBanks_RemoveScanSource(&(block->source2range));
     CheckBanks_RemoveScanDestination(&(block->destrange));
     Event_Release(event_NULL, event_ANY, event_ANY, Null_closecommon2, block);
     Window_Delete(block->destrange.window);
     free(block);
  }

}

/******************************************************************************/

static BOOL Close_window(event_pollblock *event, void *reference)
{
  iconrange_block *range = reference;

  (range->close)(FALSE, FALSE, range);

  return(FALSE);
}

/******************************************************************************/

static BOOL Close_window2(event_pollblock *event, void *reference)
{
  common2_block *block = reference;

  (block->source2range.close)(FALSE, FALSE, block);

  return(FALSE);
}

/******************************************************************************
 *                             Null handlers                                  *
 ******************************************************************************/

static BOOL Null_closecommon2(event_pollblock *event, void *reference)
{
  common2_block *block = reference;
  
  if(SpriteWin_FreeImage(block->source2range.sourceimage))
     (block->source2range.close)(FALSE, FALSE, block);

  return(FALSE);
}

/******************************************************************************
 *                             misc routines                                  *
 ******************************************************************************/

void NoMemProc(void)
{
  char err_msg[128];

  Msgs_Lookup("err.nomemproc", err_msg, 127);
  Error_Report(1,err_msg);

}

/******************************************************************************/

void InvAnchor(void)
{
  char err_msg[128];

  Msgs_Lookup("err.invanch", err_msg, 127);
  Error_Report(1,err_msg);

}
