/*
    ####             #    #     # #
    #   #            #    #       #          The FreeWare C library for 
    #   #  ##   ###  #  # #     # ###             RISC OS machines
    #   # #  # #     # #  #     # #  #   ___________________________________
    #   # ####  ###  ##   #     # #  #                                      
    #   # #        # # #  #     # #  #    Please refer to the accompanying
    ####   ### ####  #  # ##### # ###    documentation for conditions of use
    ________________________________________________________________________

    File:    Clear.c.Free
    Author:  Copyright © 1993,1994 Jason Howat
    Version: 1.01 (13 May 1994)
    Purpose: Release all memory used by a Clear file.
    History: 1.00 (16 Dec 1993)   initial version
             1.01 (13 May 1994)   updated to use Mem_ library
*/


#include <stdlib.h>

#include "DeskLib:Clear.h"
#include "DeskLib:Mem.h"


void Clear_Free(clear_picture *picture)
{
  if(picture->bitmap)
    Mem_Free((void **)&picture->bitmap);
  if(picture->palette)
    Mem_Free((void **)&picture->palette);
  free(picture);
}
