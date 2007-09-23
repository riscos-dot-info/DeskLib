/*
 * This file is part of DeskLib, the C library for RISC OS.
 * Please see accompanying documentation for terms of use.
 *
 *       http://www.riscos.info/index.php/DeskLib
 *
 *
 * Module:  Handler
 * File:    MenuClick.c
 * Author:  Adam Richardson
 * Purpose: Handler for use by dl_Shell_Initialise
 *
 * Version History
 * 16/09/2007: Creation
 *
 */

#include "DeskLib:Wimp.h"
#include "DeskLib:Icon.h"
#include "DeskLib:Handler.h"
#include "DeskLib:Menu.h"

BOOL dl_Handler__MenuClick(event_pollblock *event, void *reference)
{
  if (event->data.mouse.button.data.menu) /* Only claim menu clicks */
  {
    if (event->data.mouse.window == window_ICONBAR)
      Menu_Show((menu_ptr) reference, event->data.mouse.pos.x, -1);
    else
      Menu_Show((menu_ptr) reference, event->data.mouse.pos.x, event->data.mouse.pos.y);

    return TRUE;
  }
  return FALSE;
}
