/************************************************************************
 *                                                                      *
 *        File: Create.c                                                *
 *                                                                      *
 *     Purpose: Provides window to create a blank image of a certain    *
 *              size in any of the image banks.                         *
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

#include "HackLib:flex.h"              /* Flexable malloc                 */

#include "kernel.h"                    /* clib kernel functions           */
#include "MySWIs.h"                    /* SWI no defs                     */
#include "Create.h"
#include "SpriteWin.h"
#include "Misc.h"
#include "CheckBanks.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

extern window_handle db;

static window_handle create = NULL;

static iconrange_block range;

/* icon numbers for image bank radios */
#define image_MIN 12
#define image_MAX 17

/* max and min image resolutions */
#define res_MIN 1
#define res_MAX 9999

/* default image resolution for create window */
#define def_X 256
#define def_Y 256

static BOOL Click_create(event_pollblock *event, void *reference);
static BOOL Key_create(event_pollblock *event, void *reference);
static BOOL Close_create(event_pollblock *event, void *reference);
void close_create(BOOL ok, BOOL keepopen);

/******************************************************************************
 *                              Click handlers                                *
 ******************************************************************************/

static BOOL Click_create(event_pollblock *event, void *reference)
{

  switch(event->data.mouse.icon)
  {

    case 5: case 8: /* down X , Y */
       if(event->data.mouse.button.data.select)
          dec_icon(create, event->data.mouse.icon - 1, res_MIN);
       if(event->data.mouse.button.data.adjust)
          inc_icon(create, event->data.mouse.icon - 1, res_MAX);
       break;
      
    case 6: case 9: /* up X , Y */
       if(event->data.mouse.button.data.select)
          inc_icon(create, event->data.mouse.icon - 2, res_MAX);
       if(event->data.mouse.button.data.adjust)
          dec_icon(create, event->data.mouse.icon - 2, res_MIN);
       break;
    
    case 18: /* Cancel */
       close_create(FALSE, FALSE);
       break;

    case 19: /* Create */
       close_create(TRUE, event->data.mouse.button.data.adjust);
       break;
      
  }
  return(TRUE);
}

/******************************************************************************
 *                             keypress handlers                              *
 ******************************************************************************/

static BOOL Key_create(event_pollblock *event, void *reference)
{

  if(event->data.key.caret.icon == 7){

     if(event->data.key.code == keycode_RETURN){
        close_create(TRUE, FALSE);
        return(TRUE);
     }

  }

  return(TRUE);

}

/******************************************************************************
 *                                open handlers                               *
 ******************************************************************************/

extern void Create_Open(void)
{
 /*
  * open create new image window on screen
  */

  if(create == NULL){

    /*
     * create the window
     */
     create = Window_Create("Create", template_TITLEMIN);

    /*
     * set default values
     */
     Icon_SetInteger(create, 4, def_X);
     Icon_SetInteger(create, 7, def_Y);

    /*
     * initialise the available banks and start the scanning procedure
     */
     range.window = create;
     range.min = image_MIN;
     range.max = image_MAX;
     CheckBanks_InstallScanFree(&range);

    /*
     * install event handlers
     */
     Event_Claim(event_CLICK, create, event_ANY, Click_create, NULL);

     Event_Claim(event_KEY, create, event_ANY, Key_create, NULL);

     Event_Claim(event_CLOSE, create, event_ANY, Close_create, NULL);
  }

  Window_Show(create, open_WHEREVER);

 /*
  * put caret into X res icon
  * REMINDER: call only works when the window is displayed
  */
  Icon_SetCaret(create, 4);
}


/******************************************************************************
 *                               close handlers                               *
 ******************************************************************************/

void close_create(BOOL ok, BOOL keepopen)
{
  wimp_point res;
  int imageno;

  if(ok == TRUE){

     res.x = Icon_GetInteger(create, 4);
     res.y = Icon_GetInteger(create, 7);

     imageno = image_getselect(&range);

     if(imageno != -1)
        SpriteWin_Create(&res, imageno);
  
  }

  if(keepopen == FALSE){

     CheckBanks_RemoveScanFree(&range);
     Window_Delete(create);
     create = NULL;
  }

}

/******************************************************************************/

static BOOL Close_create(event_pollblock *event, void *reference)
{
  close_create(FALSE, FALSE);

  return(FALSE);
}

/******************************************************************************
 *                           menu choice handlers                             *
 ******************************************************************************/

/******************************************************************************
 *                            extern routines                                 *
 ******************************************************************************/

extern void Create_Init(void)
{
 /*
  * initialisation to be done on startup
  */
 
}

/******************************************************************************
 *                             misc routines                                  *
 ******************************************************************************/

