/************************************************************************
 *                                                                      *
 *        File: Filter.c                                                *
 *                                                                      *
 *     Purpose: Builds filter menu for convolution window from Messages *
 *              file and fills matrix on window with numbers on menu    *
 *              selections.                                             *
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
#include "DeskLib:Icon.h"              /* Icon handling automation        */
#include "DeskLib:Menu.h"              /* Menu create & show support      */
#include "DeskLib:Screen.h"            /* Caching screen info             */
#include "DeskLib:Sprite.h"            /* Sprite handling routines        */
#include "DeskLib:File.h"              /* OS file IO                      */
#include "DeskLib:KeyCodes.h"          /* Codes for wimp returned keys    */

#include "flex.h"                      /* Flexible malloc                 */

#include "kernel.h"                    /* clib kernel functions           */
#include "MySWIs.h"                    /* SWI no defs                     */
#include "Filter.h"
#include "Misc.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

extern window_handle db;

/* filter menu */
static menu_ptr filtermenu = NULL;

/* window handle of window menu was pop-uped from */
static window_handle currentwindow;

/* icon defs */
#define icon_POPUP      27
#define icon_FILTERNAME 26
#define icon_MATRIXMIN  16
#define icon_MATRIXMAX  24
#define icon_ANCHORX    29
#define icon_ANCHORY    31

/* number of entries in filter definition */
#define filter_LENGTH 11

static BOOL Click_popup(event_pollblock *event, void *reference);
static BOOL MenuChoice_filtermenu(event_pollblock *event, void *reference);
static BOOL Key_matrix(event_pollblock *event, void *reference);
void select_filter(window_handle window, int selection);
void NoFilter(void);

/******************************************************************************
 *                            extern routines                                 *
 ******************************************************************************/

extern void Filter_Init(void)
{
 /*
  * initialisation to be done on startup
  */
  char menudescription[260];
  char menutitle[15];

 /*
  * build filter menu
  */
  Msgs_Lookup("title.filter", menutitle, 15);
  Msgs_Lookup("menu.filter", menudescription, 260);
  filtermenu = Menu_New(menutitle, menudescription);

 /*
  * attatch menu selection handler
  */
  Event_Claim(event_MENU, event_ANY, event_ANY, MenuChoice_filtermenu, NULL);

}

/******************************************************************************/

extern void Filter_InstallMenu(window_handle window)
{
 /*
  * installs pop-up menu on convolution window 'window'
  */

  Event_Claim(event_CLICK, window, icon_POPUP, Click_popup, (void *)window);
  Event_Claim(event_KEY, window, event_ANY, Key_matrix, (void *)window);

 /*
  * initialise window with first filter
  */
  select_filter(window, 0);

}

/******************************************************************************/

extern void Filter_RemoveMenu(window_handle window)
{
 /*
  * removes pop-up menu from convolution window 'window'
  */

  Event_Release(event_CLICK, window, icon_POPUP, Click_popup, (void *)window);
  Event_Release(event_KEY, window, event_ANY, Key_matrix, (void *)window);

}

/******************************************************************************/

extern void Filter_GetMatrix(window_handle window,
                             wimp_point *anchor, int (*matrix)[3][3])
{
 /*
  * this fills 'anchor' and 'matrix' with the values from 'window'
  */
  int x, y;

  anchor->x = Icon_GetInteger(window, icon_ANCHORX);
  anchor->y = Icon_GetInteger(window, icon_ANCHORY);

  for(y = 0; y < 3; y++){

     for(x = 0; x < 3; x++){

         (*matrix)[x][y] = Icon_GetInteger(window, icon_MATRIXMIN + x + (3 * y));
     }

  }

}

/******************************************************************************
 *                              Click handlers                                *
 ******************************************************************************/

static BOOL Click_popup(event_pollblock *event, void *reference)
{
  window_handle window = (window_handle) reference;


  if(event->data.mouse.icon == icon_POPUP){ /* the popup menu icon */

     Popup_menu(window, icon_POPUP, filtermenu);
     currentwindow = window;
     return(TRUE);

  }

  return(FALSE);
}

/******************************************************************************
 *                           menu choice handlers                             *
 ******************************************************************************/

static BOOL MenuChoice_filtermenu(event_pollblock *event, void *reference)
{
  mouse_block ptr;

  if (menu_currentopen != filtermenu)        /* Only respond to filtermenu  */
    return(FALSE);

 /*
  * update window with new filter
  */
  select_filter(currentwindow, event->data.selection[0]);

  Wimp_GetPointerInfo(&ptr);               /* If ADJUST was used to select, */
  if (ptr.button.data.adjust)              /* then we keep the menu open.   */
    Menu_ShowLast();

  return(TRUE);
}

/******************************************************************************
 *                             keypress handlers                              *
 ******************************************************************************/

static BOOL Key_matrix(event_pollblock *event, void *reference)
{
 /*
  * update the filter name to user defined if new number are typed into the matrix
  */
  window_handle window = (window_handle) reference;

  char text[64];
  
  if( ((event->data.key.caret.icon >= icon_MATRIXMIN) &&
       (event->data.key.caret.icon <= icon_MATRIXMAX)) ||
      (event->data.key.caret.icon == icon_ANCHORX) ||
      (event->data.key.caret.icon == icon_ANCHORY) ){

      if( ((event->data.key.code >= '0') && (event->data.key.code <= '9')) ||
           (event->data.key.code == '-') || (event->data.key.code == keycode_DELETE) ||
           (event->data.key.code == keycode_BACKSPACE) ){

         Msgs_Lookup("txt.userdef", text, 63);

         if( strcmp(Icon_GetTextPtr(window, icon_FILTERNAME), text) != 0 ){

            Icon_SetText(window, icon_FILTERNAME, text);

         }
         
      }
      
  }

  return(FALSE);

}

/******************************************************************************
 *                             misc routines                                  *
 ******************************************************************************/

void select_filter(window_handle window, int selection)
{
 /*
  * updates convolution window with new filter name and filter matrix
  */
  int entry;
  char list[100];
  char number[10];
  char lookup[20];
  int index_list, index_number;

  Icon_SetText(window, icon_FILTERNAME, Menu_GetText(filtermenu, selection) );


 /*
  * calculate tag for selected filter entry
  */
  sprintf(lookup, "filter.%i", selection);


 /*
  * lookup filter entries, abort with error if entry not found
  */
  if(!Msgs_Lookup(lookup , list, 95)){
     NoFilter();
     return;
  }

 /*
  * place filter into icons
  */
  index_list = 0;
  for(entry = 0; entry < filter_LENGTH; entry++){

      index_number = 0;
      while((list[index_list] >= 0x20) && (list[index_list] != ',')){
         if(list[index_list] != ':')
            number[index_number++] = list[index_list++];
         else
            index_list++;
      }

      number[index_number] = 0;
      index_list++;

      switch(entry)
      {
        case 0: /* x anchor */
          Icon_SetInteger(window, icon_ANCHORX, atoi(number));
          break;

        case 1: /* y anchor */
          Icon_SetInteger(window, icon_ANCHORY, atoi(number));
          break;

        default:
          Icon_SetInteger(window, icon_MATRIXMIN + entry - 2, atoi(number));
          break;
      }
  }
   
}

/******************************************************************************/

void NoFilter(void)
{
  char err_msg[128];

  Msgs_Lookup("err.nofilter", err_msg, 127);
  Error_Report(1,err_msg);

}
