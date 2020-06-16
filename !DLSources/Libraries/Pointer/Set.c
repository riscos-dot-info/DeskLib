/*
    ####             #    #     # #
    #   #            #    #       #          The FreeWare C library for
    #   #  ##   ###  #  # #     # ###             RISC OS machines
    #   # #  # #     # #  #     # #  #   ___________________________________
    #   # ####  ###  ##   #     # #  #
    #   # #        # # #  #     # #  #    Please refer to the accompanying
    ####   ### ####  #  # ##### # ###    documentation for conditions of use
    ________________________________________________________________________

    File:    Pointer.Set.c
    Author:  Copyright © 2020 Stephen Fryatt
    Version: 1.00 (16 Jun 2020)
    Purpose: Set the pointer position
*/

#include "DeskLib:Error.h"
#include "DeskLib:Icon.h"
#include "DeskLib:KernelSWIs.h"
#include "DeskLib:Screen.h"
#include "DeskLib:Wimp.h"
#include "DeskLib:WimpSWIs.h"
#include "DeskLib:Pointer.h"


typedef char byte;

static void Lower2Bytes(int integer,byte *byte0,byte *byte1)
                             /* extracts lowest 2 bytes from input integer */
{
 union
      {
       int integer;
       byte bytes[2];
      }
       u;
 u.integer=integer;
 *byte0=u.bytes[0]; /* lowest byte */
 *byte1=u.bytes[1]; /* next lowest byte */
}

os_error *Pointer_SetPosition(wimp_point position)
{
 byte         box[5]={3};
 window_state wstate;
 Lower2Bytes(position.x,&box[1],&box[2]);
 Lower2Bytes(position.y,&box[3],&box[4]);
 return OS_Word(osword_DEFINEPOINTERANDMOUSE,box);
}
