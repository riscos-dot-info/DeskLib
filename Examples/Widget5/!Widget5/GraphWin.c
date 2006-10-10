/************************************************************************
 *                                                                      *
 *        File: GraphWin.c                                              *
 *                                                                      *
 *     Purpose: Draws graphs for greylevel and cumulative histograms    *
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
#include "Process.h"
#include "Configure.h"

#include <stdlib.h>
#include <stdio.h>
#include <string.h>

extern window_handle db;

/* definitions for 'graph' */
#define graph_GREYHISTOGRAM 0
#define graph_CUMUHISTOGRAM 1

/* icon defs */
#define icon_MAXY       4
#define icon_LABELX     10
#define icon_BAR        36
#define icon_FREQMIN    27
#define icon_FREQMAX    35
#define icon_CUMUPIXMIN 11
#define icon_CUMUPIXMAX 26

/* icon defs for small graph window */
#define icon_SMAXY       4
#define icon_SLABELX     10
#define icon_SBAR        26
#define icon_SFREQMIN    17
#define icon_SFREQMAX    25
#define icon_SCUMUPIXMIN 11
#define icon_SCUMUPIXMAX 16

void open_greyhistogram(int sourceimage);
void open_cumulativehistogram(int sourceimage);
void draw_graph(window_handle window, int (*histogram)[256]);
static BOOL Close_window(event_pollblock *event, void *reference);

/******************************************************************************
 *                            extern routines                                 *
 ******************************************************************************/

extern void GraphWin_Init(void)
{
 /*
  * initialisation to be done on startup
  */
 
}

/******************************************************************************/

extern void GraphWin_Start(int graph, int sourceimage)
{

  switch(graph)
  {
    case graph_GREYHISTOGRAM:
      open_greyhistogram(sourceimage);
      break;

    case graph_CUMUHISTOGRAM:
      open_cumulativehistogram(sourceimage);
      break;

    default:
      Error_Report(1, "graph %i   image %i", graph, sourceimage);
      break;
  }

}

/******************************************************************************
 *                                open handlers                               *
 ******************************************************************************/

void open_greyhistogram(int sourceimage)
{
  window_handle window;
  int histogram[256];
  wimp_point *size;
  char number[5];
  char title[50];
  icon_handle icon;

 /*
  * get size of image
  */
  size = SpriteWin_GetSize(sourceimage);

 /*
  * calculate greylevel histogram
  */
  Process_GreyHistorgram(sourceimage, size, &histogram);

 /*
  * create graph window
  */
  switch(graphtype)
  {
    case graphtype_LARGE:
       window = Window_Create("Graph", template_TITLEMIN);
       break;

    case graphtype_SMALL:
       window = Window_Create("SmallGraph", template_TITLEMIN);
       break;
  }

 /*
  * remove y label that is not needed
  */
  switch(graphtype)
  {
    case graphtype_LARGE:
       for(icon = icon_CUMUPIXMIN; icon <= icon_CUMUPIXMAX; icon++)
           Wimp_DeleteIcon(window, icon);
       break;

    case graphtype_SMALL:
       for(icon = icon_SCUMUPIXMIN; icon <= icon_SCUMUPIXMAX; icon++)
           Wimp_DeleteIcon(window, icon);
       break;
  }

 /*
  * set text for window title
  */
  Msgs_Lookup("txt.hisgra", title, 49);
  sprintf(number, " %i", sourceimage);
  strcat(title, number);
  Window_SetTitle(window, title);

 /*
  * draw bars on graph
  */
  draw_graph(window, &histogram);

 /*
  * install event handlers
  */
  Event_Claim(event_CLOSE, window, event_ANY, Close_window, (void *)window);

 /*
  * display window
  */
  Window_Show(window, open_WHEREVER);

}

/******************************************************************************/

void open_cumulativehistogram(int sourceimage)
{
  window_handle window;
  int greyhist[256];
  int cuhist[256];
  wimp_point *size;
  char number[5];
  char title[50];
  icon_handle icon;

 /*
  * get size of image
  */
  size = SpriteWin_GetSize(sourceimage);

 /*
  * calculate greylevel histogram
  */
  Process_GreyHistorgram(sourceimage, size, &greyhist);

 /*
  * calculate cumulative grey level histogram
  */
  Process_CumulativeHistorgram(&greyhist, &cuhist);

 /*
  * create graph window
  */
  switch(graphtype)
  {
    case graphtype_LARGE:
       window = Window_Create("Graph", template_TITLEMIN);
       break;

    case graphtype_SMALL:
       window = Window_Create("SmallGraph", template_TITLEMIN);
       break;
  }

 /*
  * remove y label that is not needed
  */
  switch(graphtype)
  {
    case graphtype_LARGE:
       for(icon = icon_FREQMIN; icon <= icon_FREQMAX; icon++)
           Wimp_DeleteIcon(window, icon);
       break;

    case graphtype_SMALL:
       for(icon = icon_SFREQMIN; icon <= icon_SFREQMAX; icon++)
           Wimp_DeleteIcon(window, icon);
       break;
  }

 /*
  * set text for window title
  */
  Msgs_Lookup("txt.cugra", title, 49);
  sprintf(number, " %i", sourceimage);
  strcat(title, number);
  Window_SetTitle(window, title);

 /*
  * draw bars on graph
  */
  draw_graph(window, &cuhist);

 /*
  * install event handlers
  */
  Event_Claim(event_CLOSE, window, event_ANY, Close_window, (void *)window);

 /*
  * display window
  */
  Window_Show(window, open_WHEREVER);

}

/******************************************************************************
 *                               close handlers                               *
 ******************************************************************************/

static BOOL Close_window(event_pollblock *event, void *reference)
{
  window_handle window = (window_handle) reference;

  Window_Delete(window);

  return(TRUE);
}

/******************************************************************************
 *                             misc routines                                  *
 ******************************************************************************/

void draw_graph(window_handle window, int (*histogram)[256])
{
 /*
  * draws graph on window of histogram in array 'histogram'
  */
  icon_createblock icreate;
  icon_handle icon;
  int loop;
  int maxheight;
  float heightscale;
  int barheight;

  icon_handle baricon;
  icon_handle ylabel;
  int horizbarsep;

  switch(graphtype)
  {
    case graphtype_LARGE:
       baricon = icon_BAR;
       ylabel = icon_MAXY;
       horizbarsep = 4;
       break;

    case graphtype_SMALL:
       baricon = icon_SBAR;
       ylabel = icon_SMAXY;
       horizbarsep = 2;
       break;
  }

 /*
  * setup icon block for first bar and remove guide bar from window
  */
  icreate.window = window;
  Wimp_GetIconState(window, baricon, &icreate.icondata);
  icreate.icondata.workarearect.min.x = icreate.icondata.workarearect.max.x - 2;
  Wimp_DeleteIcon(window, baricon);

 /*
  * find maximum height for bars
  */
  maxheight = 0;
  for(loop = 0; loop < 256; loop++)
      if( (*histogram)[loop] > maxheight )
         maxheight = (*histogram)[loop];

 /*
  * set max y label
  */
  Icon_SetInteger(window, ylabel, maxheight);

 /*
  * calculate scaling factor for height of bars
  */
  heightscale = ( (float) icreate.icondata.workarearect.max.y -
                  (float) icreate.icondata.workarearect.min.y) / (float) maxheight;
 /*
  * add bars for histogram
  */
  for(loop = 0; loop < 256; loop++){

     barheight = (int) ( (float) (*histogram)[loop] * heightscale );

     if(barheight > 0){ /* no bar if zero */

        if(barheight < 4) /* min size of bar */
           barheight = 4;

        icreate.icondata.workarearect.max.y = icreate.icondata.workarearect.min.y +
                                              barheight;
         
        Wimp_CreateIcon(&icreate, &icon);
     }

     icreate.icondata.workarearect.min.x += horizbarsep;
     icreate.icondata.workarearect.max.x += horizbarsep;

  }

}

