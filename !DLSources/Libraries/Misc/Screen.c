/*
    ####             #    #     # #
    #   #            #    #       #          The FreeWare C library for
    #   #  ##   ###  #  # #     # ###             RISC OS machines
    #   # #  # #     # #  #     # #  #   ___________________________________
    #   # ####  ###  ##   #     # #  #
    #   # #        # # #  #     # #  #    Please refer to the accompanying
    ####   ### ####  #  # ##### # ###    documentation for conditions of use
    ________________________________________________________________________

    File:    Screen.c
    Author:  Copyright © 1992 Jason Williams
    Version: 1.00 (23 Mar 1992)
    Purpose: Screen functions (read size, eig factors, etc.)
*/

#include "DeskLib:Wimp.h"
#include "DeskLib:SWI.h"
#include "DeskLib:Screen.h"

int        screen_mode  = -1;
wimp_point screen_size;
wimp_point screen_eig;
wimp_point screen_delta;
int        screen_bpp;


#define ReadModeVar(m, v, r) SWI(3, 3, 0x20035, (m), (v), 0,   0, 0, (r))
#define SWI_XOS_Byte 0x20006

extern BOOL Screen_CacheModeInfo(void)
{
  int oldmode = screen_mode;

  SWI(1, 3, SWI_XOS_Byte, 135,   NULL, NULL, &screen_mode);

  if (oldmode == screen_mode)
    return(FALSE);

  ReadModeVar(-1, 9, &screen_bpp);
  screen_bpp = 1 << screen_bpp;

  ReadModeVar(-1, 4, &screen_eig.x);
  ReadModeVar(-1, 5, &screen_eig.y);

  screen_delta.x = 1 << screen_eig.x;
  screen_delta.y = 1 << screen_eig.y;

  ReadModeVar(-1, 11, &screen_size.x);
  screen_size.x = (screen_size.x + 1) << screen_eig.x;

  ReadModeVar(-1, 12, &screen_size.y);
  screen_size.y = (screen_size.y + 1) << screen_eig.y;

  return(TRUE);
}
