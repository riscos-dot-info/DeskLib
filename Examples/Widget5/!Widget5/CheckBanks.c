/************************************************************************
 *                                                                      *
 *        File: CheckBanks.c                                            *
 *                                                                      *
 *     Purpose: Provides routines to keep the state of the image bank   *
 *              icons updated.                                          *
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
#include "Configure.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

extern window_handle db;


static BOOL Null_updatefreebanks(event_pollblock *event, void *reference);
static BOOL Null_updatedestbanks(event_pollblock *event, void *reference);
static BOOL Null_updatesourcebanks(event_pollblock *event, void *reference);
void update_freebanks(iconrange_block *range);
void select_freebanks(iconrange_block *range);
void update_destbanks(iconrange_block *range);
void select_destbanks(iconrange_block *range);
void update_sourcebanks(iconrange_block *range);
void image_select(iconrange_block *range, int imageno);
void image_deselect(iconrange_block *range, int imageno);
void image_shade(iconrange_block *range, int imageno);
void image_unshade(iconrange_block *range, int imageno);
void image_setshade(iconrange_block *range, int imageno, BOOL shaded);
BOOL image_getshade(iconrange_block *range, int imageno);


/******************************************************************************
 *                            extern routines                                 *
 ******************************************************************************/

extern void CheckBanks_Init(void)
{
 /*
  * initialisation to be done on startup
  */
 
}

/******************************************************************************/

extern void CheckBanks_InstallScanFree(iconrange_block *range)
{
 /*
  * install routines to update image bank radios to make used banks greyed out
  */
 
 /*
  * initialise the available banks
  */
  update_freebanks(range);

 /*
  * preselect first free bank
  */
  select_freebanks(range);

  Event_Claim(event_NULL, event_ANY, event_ANY, Null_updatefreebanks, range);

}

/******************************************************************************/

extern void CheckBanks_InstallScanDestination(iconrange_block *range)
{
 /*
  * install routines to update image bank radios to make unusable banks greyed out
  */
 
 /*
  * initialise the available banks
  */
  update_destbanks(range);

 /*
  * preselect first free bank
  */
  select_destbanks(range);

  Event_Claim(event_NULL, event_ANY, event_ANY, Null_updatedestbanks, range);

}

/******************************************************************************/

extern void CheckBanks_InstallScanSource(iconrange_block *range)
{
 /*
  * install routines to update image bank radios to greyed out unusable input
  * images
  */
 
 /*
  * initialise the available banks
  */
  update_sourcebanks(range);

  Event_Claim(event_NULL, event_ANY, event_ANY, Null_updatesourcebanks, range);

}

/******************************************************************************/

extern void CheckBanks_RemoveScanFree(iconrange_block *range)
{

  Event_Release(event_NULL, event_ANY, event_ANY, Null_updatefreebanks, range);

}

/******************************************************************************/

extern void CheckBanks_RemoveScanDestination(iconrange_block *range)
{

  Event_Release(event_NULL, event_ANY, event_ANY, Null_updatedestbanks, range);

}

/******************************************************************************/

extern void CheckBanks_RemoveScanSource(iconrange_block *range)
{

  Event_Release(event_NULL, event_ANY, event_ANY, Null_updatesourcebanks, range);

}

/******************************************************************************
 *                            update routines                                 *
 ******************************************************************************/

void update_freebanks(iconrange_block *range)
{
  int imageno;
  BOOL shade;
  BOOL freebank;
  int freeimageno;

  freebank = FALSE;
  freeimageno = 0;

 /*
  * grey out used banks
  */
  for(imageno = 0; imageno < number_IMAGES; imageno++){

      shade = !SpriteWin_FreeImage(imageno);
      if(shade != image_getshade(range, imageno))
         image_setshade(range, imageno, shade);

      if(!shade){
         freebank = TRUE;
         freeimageno = imageno;
      }

  }

 /*
  * select another free bank if current one has been used
  */
  imageno = image_getselect(range);

  if(imageno != -1){

     if(image_getshade(range, imageno)){
        image_deselect(range, imageno);
        select_freebanks(range);
     }

  }
  else if(freebank)
    image_select(range, freeimageno);

}

/******************************************************************************/

void update_destbanks(iconrange_block *range)
{
  int imageno;
  BOOL shade;
  BOOL destbank;
  int destimageno;

  destbank = FALSE;
  destimageno = 0;

 /*
  * grey out used banks
  */
  for(imageno = 0; imageno < number_IMAGES; imageno++){

      switch(destinationbanktype)
      {
        case use_FREE:
          shade = !SpriteWin_FreeImage(imageno);
          if(imageno == range->sourceimage){ /* on source image bank */

             if(!shade){ /* source image bank is free so close window */
                if(range->close != NULL)
                   (range->close)(FALSE, FALSE, range);
             }
          }
          break;

        case use_FREEORSAME:
          shade = !SpriteWin_FreeImage(imageno);

          if(imageno == range->sourceimage){ /* on source image bank */

             if(!shade){ /* source image bank is free so close window */
                if(range->close != NULL)
                   (range->close)(FALSE, FALSE, range);
             }
          }
          else if(shade) /* image is in use but maybe same size */
             shade = !SpriteWin_SameSize(&range->size, imageno);
          break;

        default:
          shade = TRUE;
          break;
      }

      if(shade != image_getshade(range, imageno))
         image_setshade(range, imageno, shade);

      if(!shade){
         destbank = TRUE;
         destimageno = imageno;
      }

  }

 /*
  * select another free bank if current one has been used
  */
  imageno = image_getselect(range);

  if(imageno != -1){

     if(image_getshade(range, imageno)){
        image_deselect(range, imageno);
        select_destbanks(range);
     }

  }
  else if(destbank)
    image_select(range, destimageno);

}

/******************************************************************************/

void update_sourcebanks(iconrange_block *range)
{
  int selectedimage;
  int imageno;
  BOOL shade;

  selectedimage = image_getselect(range);

 /*
  * grey out unusable banks and deselect if unusable is selected
  */
  for(imageno = 0; imageno < number_IMAGES; imageno++){

      shade = TRUE;

      if(imageno != range->sourceimage){ /* only try to use banks other than the ps */

         if(!SpriteWin_FreeImage(imageno))
            shade = !SpriteWin_SameSize(&range->size, imageno);
      }

      if(shade != image_getshade(range, imageno))
         image_setshade(range, imageno, shade);

      if(shade && (imageno == selectedimage) )
         image_deselect(range, imageno);

  }

}

/******************************************************************************/

static BOOL Null_updatefreebanks(event_pollblock *event, void *reference)
{

  update_freebanks((iconrange_block *)reference);
  
  return(FALSE);

}

/******************************************************************************/

static BOOL Null_updatedestbanks(event_pollblock *event, void *reference)
{
             
  update_destbanks((iconrange_block *)reference);
  
  return(FALSE);

}

/******************************************************************************/

static BOOL Null_updatesourcebanks(event_pollblock *event, void *reference)
{
             
  update_sourcebanks((iconrange_block *)reference);
  
  return(FALSE);

}

/******************************************************************************
 *                            select routines                                 *
 ******************************************************************************/

void select_freebanks(iconrange_block *range)
{
  int imageno;

  imageno = 0;
  while((imageno < number_IMAGES) && (image_getshade(range, imageno)))
     imageno++;
  if(imageno != number_IMAGES)
     image_select(range, imageno);
}

/******************************************************************************/

void select_destbanks(iconrange_block *range)
{
  int imageno;

  imageno = 0;
  while((imageno < number_IMAGES) && (image_getshade(range, imageno)))
     imageno++;
  if(imageno != number_IMAGES)
     image_select(range, imageno);
}

/******************************************************************************/

void image_select(iconrange_block *range, int imageno)
{
  Icon_SetRadios(range->window, range->min, range->max, range->min + imageno);
}

/******************************************************************************/

void image_deselect(iconrange_block *range, int imageno)
{
  Icon_Deselect(range->window, range->min + imageno);
}

/******************************************************************************/

extern int image_getselect(iconrange_block *range)
{
 /*
  * returns image bank selected or -1 if none are selected
  */
  int icon;

  icon = Icon_WhichRadio(range->window, range->min, range->max);

 /*
  * check to see if the last icon is selected or none
  */
  if(icon == range->max){
     if(!Icon_GetSelect(range->window, icon))
        return(-1);
  }

  return(icon - range->min);
}

/******************************************************************************/

void image_shade(iconrange_block *range, int imageno)
{
  Icon_Shade(range->window, range->min + imageno);
}

/******************************************************************************/

void image_unshade(iconrange_block *range, int imageno)
{
  Icon_Unshade(range->window, range->min + imageno);
}

/******************************************************************************/

void image_setshade(iconrange_block *range, int imageno, BOOL shaded)
{
  Icon_SetShade(range->window, range->min + imageno, shaded);
}

/******************************************************************************/

BOOL image_getshade(iconrange_block *range, int imageno)
{
 /*
  * returns TRUE if icon is shaded
  */
  return(Icon_GetShade(range->window, range->min + imageno));
}




