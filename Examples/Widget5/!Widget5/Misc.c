/************************************************************************
 *                                                                      *
 *        File: Misc.c                                                  *
 *                                                                      *
 *     Purpose: Provides miscellanous routines used by more than one    *
 *              file in the program                                     *
 *                                                                      *
 ************************************************************************/

#include "DeskLib:WimpSWIs.h"          /* Low-level WIMP commands         */
#include "DeskLib:Window.h"            /* Window handling automation      */
#include "DeskLib:Icon.h"              /* Icon handling automation        */
#include "DeskLib:Menu.h"              /* Menu create & show support      */
#include "DeskLib:Screen.h"

#include "kernel.h"

#include <stdio.h>

#include "MySWIs.h"

extern void inc_icon(window_handle window, icon_handle icon, int upperlimit)
{
  int value;

  value = Icon_GetInteger(window, icon);

  if(value < upperlimit)
     value++;

  Icon_SetInteger(window, icon, value);
}

extern void dec_icon(window_handle window, icon_handle icon, int lowerlimit)
{
  int value;

  value = Icon_GetInteger(window, icon);

  if(value > lowerlimit)
     value--;

  Icon_SetInteger(window, icon, value);
}

extern void Icon_SetFileType(window_handle window, icon_handle icon, int filetype)
{
 /*
  * set filetype of icon
  */
  icon_block icondata;
  int topbits;
  int middlebits;
  int bottombits;


  Wimp_GetIconState(window, icon, &icondata);
  topbits = filetype >> 8;
  middlebits = (filetype - (topbits << 8)) >> 4;
  bottombits = filetype - (middlebits << 4) - (topbits << 8);
  sprintf(icondata.data.indirecttext.validstring,
          "sfile_%x%x%x", topbits, middlebits, bottombits);


}

extern void Popup_menu(window_handle window, icon_handle icon, menu_ptr menu)
{
  int winx, winy, iconx, icony;
  int block[20];
  _kernel_swi_regs r;

  block[0] = window;
  r.r[1] = (int) block; 
  _kernel_swi( SWI_XOS_Bit | SWI_Wimp_GetWindowState, &r, &r );
  winx = block[1];
  winy = block[4];

  block[0] = window;
  block[1] = icon;
  r.r[1] = (int) block;
  _kernel_swi( SWI_XOS_Bit | SWI_Wimp_GetIconState, &r, &r );
  iconx = block[4];
  icony = block[5];

  Menu_Show(menu, (iconx + winx + 64), (icony + winy));
}

extern void Menu_MakeWarn(menu_ptr menu, int entry, BOOL yesno)
{
 /*
  * makes the entry in the menu send a menuwarn message or not
  * as yesno
  */
  menu_item *item = (menu_item *) (((int) menu) + sizeof(menu_block));

  item = &item[entry];
  item->menuflags.data.notifysub = yesno;
  if(yesno)
     item->submenu.value = 1;
  else
     item->submenu.value = NULL;

}

extern void Window_GetPos(window_handle window, wimp_point *pos)
{
 /*
  * gets the current position of the topleft corner of the window
  */
  window_state state;

  Wimp_GetWindowState(window, &state);

  pos->x = state.openblock.screenrect.min.x;
  pos->y = state.openblock.screenrect.max.y;

}

extern void Window_ShowPos(window_handle window, wimp_point *openpos)
{
 /*
  * Shows the window in the position openpos
  */
  window_state state;
  int width;
  int height;
  wimp_point pos;

  pos.x = openpos->x;
  pos.y = openpos->y;

  Wimp_GetWindowState(window, &state);
  state.openblock.behind = -1;

  width = state.openblock.screenrect.max.x - state.openblock.screenrect.min.x;
  height = state.openblock.screenrect.max.y - state.openblock.screenrect.min.y;

  if (pos.x < 0)  pos.x = 0;
  if (pos.y < 64) pos.y = 64;
  if (pos.x > screen_size.x - 96) pos.x = screen_size.x - 96;
  if (pos.y > screen_size.y - 32) pos.y = screen_size.y - 32;

  state.openblock.screenrect.min.x = pos.x;
  state.openblock.screenrect.max.x = state.openblock.screenrect.min.x + width;

  state.openblock.screenrect.max.y = pos.y;
  state.openblock.screenrect.min.y = state.openblock.screenrect.max.y - height;

  Wimp_OpenWindow(&state.openblock);
}
