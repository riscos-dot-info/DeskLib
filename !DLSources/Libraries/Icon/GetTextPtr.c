/*
    ####             #    #     # #
    #   #            #    #       #          The FreeWare C library for 
    #   #  ##   ###  #  # #     # ###             RISC OS machines
    #   # #  # #     # #  #     # #  #   ___________________________________
    #   # ####  ###  ##   #     # #  #                                      
    #   # #        # # #  #     # #  #    Please refer to the accompanying
    ####   ### ####  #  # ##### # ###    documentation for conditions of use
    ________________________________________________________________________

    File:    Icon.GetTextPtr.c
    Author:  Copyright © 1994 Tim Browse
    Version: 1.00 (04 Mar 1994)
    Purpose: Returns a pointer to the indirected text of an icon.
             (It points to the actual text used by the icon; not a copy.)
*/


#include <stdio.h>

#include "DeskLib:Wimp.h"
#include "DeskLib:WimpSWIS.h"
#include "DeskLib:Icon.h"

char *Icon_GetTextPtr(window_handle window, icon_handle icon)
{
  icon_block iconblk;

  Wimp_GetIconState(window, icon, &iconblk);

  return iconblk.data.indirecttext.buffer;
}
