/*
    ####             #    #     # #
    #   #            #    #       #          The FreeWare C library for
    #   #  ##   ###  #  # #     # ###             RISC OS machines
    #   # #  # #     # #  #     # #  #   ___________________________________
    #   # ####  ###  ##   #     # #  #
    #   # #        # # #  #     # #  #    Please refer to the accompanying
    ####   ### ####  #  # ##### # ###    documentation for conditions of use
    ________________________________________________________________________

    File:    Window.GetInfo.c
    Author:  Copyright © 1992 Jason Williams
    Version: 1.00 (19 Mar 1992)
    Purpose: High-level window management functions: Get Window info
*/



#include <string.h>

#include "DeskLib:WimpSWIs.h"
#include "DeskLib:Window.h"


extern void Window_GetInfo(window_handle window, window_info *result)
/* Returns just the window part of the window_info block (strips icon defn.s)
 * The real solution would be to find out how many icons were in the window
 * and then get a flex block big enough for the whole thing and then do it,
 * but you can't read the number of icons until you have read the window_info
 * so it's a bit difficult! As a result, I just use a 4kB block to hold
 * the window definition in.
 *
 *      NOTE THAT THIS WILL CRASH IF A WINDOW HAS TOO MANY ICONS!!!!!
 *      (anything more than about 120 icons will cause undefined results)
 *
 * SeeAlso: Window_GetInfo3()
 */
{

  int         block[1024];           /* 4096 bytes for winfo return block */
  window_info *winfo = (window_info *) &block[0];

  winfo->window = window;
  Wimp_GetWindowInfo(winfo);
  memcpy(result, winfo, sizeof(window_info));
}
