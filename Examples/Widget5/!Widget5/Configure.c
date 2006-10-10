/************************************************************************
 *                                                                      *
 *        File: Configure.c                                             *
 *                                                                      *
 *     Purpose: Configuration options for program.                      *
 *              Routines to load and save options and display window to *
 *              change options.                                         *
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
#include "DeskLib:KernelSWIs.h"        /* OS_Byte                         */

#include "flex.h"                      /* Flexible malloc                 */

#include "kernel.h"                    /* clib kernel functions           */
#include "MySWIs.h"                    /* SWI no defs                     */
#include "Configure.h"
#include "Misc.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

extern window_handle db;

static window_handle configure = NULL;

#define size_256x256 0
#define size_512x512 1
extern wimp_point widget4size = 0;

extern int destinationbanktype = 0;
extern int graphtype = 0;
extern BOOL fullversion = FALSE;
extern int drawtype = 0;
extern BOOL realtime = FALSE;

/* icon numbers for radios */
#define process_MIN 2
#define process_MAX 3

#define format_MIN 6
#define format_MAX 7

#define graph_MIN 13 /* large graph */
#define graph_MAX 14 /* small graph */

#define drawtype_MIN 17
#define drawtype_MID 18
#define drawtype_MAX 19

#define icon_REALTIME 22

static BOOL Click_configure(event_pollblock *event, void *reference);
static BOOL Close_configure(event_pollblock *event, void *reference);
void close_configure(BOOL ok, BOOL keepopen);
void load_options(void);
void save_options(void);

/******************************************************************************
 *                              Click handlers                                *
 ******************************************************************************/

static BOOL Click_configure(event_pollblock *event, void *reference)
{

  switch(event->data.mouse.icon)
  {

    case process_MIN: case process_MAX:
      Icon_Select(configure, event->data.mouse.icon);
      break;

    case format_MIN: case format_MAX:
      Icon_Select(configure, event->data.mouse.icon);
      break;

    case graph_MIN: case graph_MAX:
      Icon_Select(configure, event->data.mouse.icon);
      break;

    case drawtype_MIN: case drawtype_MID: case drawtype_MAX:
      Icon_Select(configure, event->data.mouse.icon);
      break;

    case 8: /* Save */
      close_configure(TRUE, event->data.mouse.button.data.adjust);
      save_options();
      break;

    case 9: /* Cancel */
      close_configure(FALSE, FALSE);
      break;

    case 10: /* Change */
      close_configure(TRUE, event->data.mouse.button.data.adjust);
      break;
      
  }
  return(TRUE);
}

/******************************************************************************
 *                                open handlers                               *
 ******************************************************************************/

extern void Configure_Open(void)
{
 /*
  * open configure window on screen
  */

  if(configure == NULL){

    /*
     * create the window
     */
     configure = Window_Create("Configure", template_TITLEMIN);

    /*
     * process destination
     */
     Icon_SetRadios(configure, process_MIN, process_MAX, process_MIN+destinationbanktype);


    /*
     * widget 4 load format
     */
     if( (widget4size.x == 256) && (widget4size.y == 256) )
        Icon_SetRadios(configure, format_MIN, format_MAX, format_MIN);
     else
        Icon_SetRadios(configure, format_MIN, format_MAX, format_MAX);

    /*
     * graph size
     */
     Icon_SetRadios(configure, graph_MIN, graph_MAX, graph_MIN + graphtype);

    /*
     * drawing greymap type
     */
     Icon_SetRadios(configure, drawtype_MIN, drawtype_MAX, drawtype_MIN + drawtype);

    /*
     * real-time updates
     */
     Icon_SetSelect(configure, icon_REALTIME, realtime);

    /*
     * install event handlers
     */
     Event_Claim(event_CLICK, configure, event_ANY, Click_configure, NULL);

     Event_Claim(event_CLOSE, configure, event_ANY, Close_configure, NULL);
  }

  Window_Show(configure, open_WHEREVER);

}


/******************************************************************************
 *                               close handlers                               *
 ******************************************************************************/

void close_configure(BOOL ok, BOOL keepopen)
{

  if(ok == TRUE){

    /*
     * process destination
     */
     destinationbanktype = Icon_WhichRadio(configure,process_MIN,process_MAX)-process_MIN;

    /*
     * graph size
     */
     graphtype = Icon_WhichRadio(configure, graph_MIN, graph_MAX) - graph_MIN;

    /*
     * draw greymap type
     */
     drawtype = Icon_WhichRadio(configure, drawtype_MIN, drawtype_MAX) - drawtype_MIN;

    /*
     * real-time updates
     */
     realtime = Icon_GetSelect(configure, icon_REALTIME);

    /*
     * widget 4 load format
     */
     if((Icon_WhichRadio(configure, format_MIN, format_MAX)-format_MIN) == size_256x256){
        widget4size.x = 256;
        widget4size.y = 256;
     }
     else{
        widget4size.x = 512;
        widget4size.y = 512;
     }

  }

  if(keepopen == FALSE){

     Window_Delete(configure);
     configure = NULL;
  }

}

/******************************************************************************/

static BOOL Close_configure(event_pollblock *event, void *reference)
{
  close_configure(FALSE, FALSE);

  return(FALSE);
}

/******************************************************************************
 *                           menu choice handlers                             *
 ******************************************************************************/

/******************************************************************************
 *                            extern routines                                 *
 ******************************************************************************/

extern void Configure_Init(void)
{
 /*
  * initialisation to be done on startup
  */
  char buffer[20];
  window_block *block;

  load_options();

  fullversion = FALSE;

  if(OS_ReadVarVal("Widget5$Options", buffer, 20)){
     if(strcmp(buffer, "X") == 0)
        fullversion = TRUE;
  }

  /* mod the configure window template to hide the Widget4 file size
   * options if not on full version mode
   */
  if(!fullversion){

     block = Template_Find("Configure");

     block->screenrect.max.y = block->screenrect.max.y - 188;
     block->scroll.y =  block->scroll.x - 188;
     block->minsize.y = block->minsize.y - 188;
  }
}

/******************************************************************************
 *                             misc routines                                  *
 ******************************************************************************/

void load_options(void)
{
  char filename[60];
  file_handle channel;
  int temp;

  strcpy(filename, resource_pathname);
  strcat(filename, "Choices");

  if(File_Exists(filename) == FALSE)
     save_options();

  channel = File_Open(filename, file_READ);

  destinationbanktype = File_ReadInt(channel);

  temp = File_ReadInt(channel);
  if(temp == size_256x256){
     widget4size.x = 256;
     widget4size.y = 256;
  }
  else{
     widget4size.x = 512;
     widget4size.y = 512;
  }

  graphtype = File_ReadInt(channel);

  drawtype = File_ReadInt(channel);

  realtime = File_ReadInt(channel);

  File_Close(channel);

}

void save_options(void)
{
  char filename[60];
  file_handle channel;

  strcpy(filename, resource_pathname);
  strcat(filename, "Choices");

  channel = File_Open(filename, file_WRITE);

  File_WriteInt(channel, destinationbanktype);

  if( (widget4size.x == 256) && (widget4size.y == 256) )
     File_WriteInt(channel, size_256x256);
  else
     File_WriteInt(channel, size_512x512);

  File_WriteInt(channel, graphtype);

  File_WriteInt(channel, drawtype);

  File_WriteInt(channel, realtime);

  File_Close(channel);

  File_SetType(filename, filetype_TEXT);

}
