/*
    ####             #    #     # #
    #   #            #    #       #          The FreeWare C library for 
    #   #  ##   ###  #  # #     # ###             RISC OS machines
    #   # #  # #     # #  #     # #  #   ___________________________________
    #   # ####  ###  ##   #     # #  #                                      
    #   # #        # # #  #     # #  #    Please refer to the accompanying
    ####   ### ####  #  # ##### # ###    documentation for conditions of use
    ________________________________________________________________________

    File:    Menu.Warn.c
    Author:  Copyright © 1993 Ainsley Pereira
    Version: 1.00 (25 Aug 1993)
    Purpose: Attaches a handler to be called when pointer is about to open a
             submenu.
*/

#include "DeskLib:Wimp.h"
#include "DeskLib:Event.h"
#include "DeskLib:EventMsg.h"
#include "DeskLib:Menu.h"


extern void Menu_Warn(menu_ptr menu, int entry, BOOL yesno,
                      event_handler handler, void *reference)
{
  menu_item *item = (menu_item *) (((int) menu) + sizeof(menu_block));

  item = &item[entry];
  item->menuflags.data.notifysub = yesno;
  if (yesno)
    EventMsg_Claim(message_MENUWARNING, event_ANY, handler, reference);
  else
    EventMsg_Release(message_MENUWARNING, event_ANY, handler);
}
