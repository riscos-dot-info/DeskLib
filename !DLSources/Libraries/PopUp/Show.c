/*
    ####             #    #     # #
    #   #            #    #       #          The FreeWare C library for 
    #   #  ##   ###  #  # #     # ###             RISC OS machines
    #   # #  # #     # #  #     # #  #   ___________________________________
    #   # ####  ###  ##   #     # #  #                                      
    #   # #        # # #  #     # #  #    Please refer to the accompanying
    ####   ### ####  #  # ##### # ###    documentation for conditions of use
    ________________________________________________________________________

    File:    PopUp.Show.c
    Author:  Copyright © 1993 Jason Williams
    Version: 1.00 (20 May 1993)
    Purpose: High(er)-level support for opening PopUps
*/


#include <string.h>

#include "DeskLib:WimpSWIs.h"
#include "DeskLib:PopUp.h"

static popup_handle DoShow(popup_block *block, char *name,
                           popup_data *definition)
{
  strcpy(block->header.name, name);
  memcpy(&block->data, definition, sizeof(popup_data));
  block->header.reserved1 = block->header.reserved2 =
                            block->header.reserved3 = 0;
  return(PopUp_Open(block));
}


extern popup_handle PopUp_ShowPtr(char *name, BOOL isstatic,
                                  popup_data *definition)
{
  popup_block temp;
  mouse_block ptr;

  Wimp_GetPointerInfo(&ptr);
  temp.header.openpos.x = ptr.pos.x - 64;
  temp.header.openpos.y = ptr.pos.y + 64;
  if (isstatic)
    temp.header.flags = popup_STATIC;
  else
    temp.header.flags = popup_STANDALONE;
  return(DoShow(&temp, name, definition));
}


extern popup_handle PopUp_ShowMenuLeaf(char *name, popup_data *definition,
                                       message_menuwarn *msg)
{
  popup_block temp;

  temp.header.openpos = msg->openpos;
  temp.header.flags = popup_MENULEAF;
  return(DoShow(&temp, name, definition));
}
