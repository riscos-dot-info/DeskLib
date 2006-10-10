/*
    ####             #    #     # #
    #   #            #    #       #          The FreeWare C library for
    #   #  ##   ###  #  # #     # ###             RISC OS machines
    #   # #  # #     # #  #     # #  #   ___________________________________
    #   # ####  ###  ##   #     # #  #
    #   # #        # # #  #     # #  #    Please refer to the accompanying
    ####   ### ####  #  # ##### # ###    documentation for conditions of use
    ________________________________________________________________________

    File:    Icon.BarIcon.c
    Author:  Copyright © 1992, 1993 Jason Williams
             Corrected by Jason Howat
    Version: 1.02 (14 Jul 1993)
    Purpose: Place icon on the Icon Bar
*/


#include <string.h>

#include "DeskLib:Wimp.h"
#include "DeskLib:WimpSWIs.h"
#include "DeskLib:Sprite.h"
#include "DeskLib:Screen.h"
#include "DeskLib:SWI.h"
#include "DeskLib:Icon.h"

#define ReadModeVar(m, v, r) SWI(3, 3, 0x20035, (m), (v), 0,   0, 0, (r))

extern icon_handle Icon_BarIcon(const char *spritename, window_handle pos)
{
  icon_createblock icreate;
  icon_handle      icon;
  void             *romsprites, *ramsprites;
  sprite_info      sinfo;
  int              xeigfactor;
  int              yeigfactor;

  icreate.window = pos;
  icreate.icondata.flags.value = 0x1700301a;       /* Click-able sprite icon */
  strncpy(icreate.icondata.data.spritename, spritename, sprite_MAXNAME);

  icreate.icondata.workarearect.min.x =
    icreate.icondata.workarearect.min.y = 0;

  Wimp_BaseOfSprites(&romsprites, &ramsprites);
  if (Sprite_ReadInfo(ramsprites, spritename, &sinfo) == NULL)
  {
    /****** previous version used eigfactors for current mode ******/
    ReadModeVar(sinfo.mode, 4, &xeigfactor);
    ReadModeVar(sinfo.mode, 5, &yeigfactor);
    icreate.icondata.workarearect.max.x = sinfo.width  << xeigfactor;
    icreate.icondata.workarearect.max.y = sinfo.height << yeigfactor;
  }
  else
  {
    icreate.icondata.workarearect.max.x =
      icreate.icondata.workarearect.max.y = 68;
  }

  Wimp_CreateIcon(&icreate, &icon);
  return(icon);
}
