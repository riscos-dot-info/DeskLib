/*
    ####             #    #     # #
    #   #            #    #       #          The FreeWare C library for 
    #   #  ##   ###  #  # #     # ###             RISC OS machines
    #   # #  # #     # #  #     # #  #   ___________________________________
    #   # ####  ###  ##   #     # #  #                                      
    #   # #        # # #  #     # #  #    Please refer to the accompanying
    ####   ### ####  #  # ##### # ###    documentation for conditions of use
    ________________________________________________________________________

    File:    Clear.c.Create
    Author:  Copyright © 1993,1994 Jason Howat
    Version: 1.01 (13 May 1994)
    Purpose: Allocate memory for a Clear file.
    History: 1.00 (16 Dec 1993)   initial version
             1.01 (13 May 1994)   updated to use Mem_ library
*/


#include <stdlib.h>
#include "DeskLib:Clear.h"
#include "DeskLib:Mem.h"

#include "DeskLib:Clear.h"
#include "ClearDefs.h"




clear_picture *Clear_Create(unsigned width, unsigned height, unsigned bpp)
{
  clear_picture *temp;
  unsigned      bitmap_size;
  

  if((temp = malloc(sizeof(clear_picture))) == NULL)
    return NULL;

  temp->creator = clear__creator;
  temp->creatorversion = clear__creatorversion;
  temp->width = width;
  temp->height = height;
  temp->bpp = bpp;
  if(bpp > 8)
  {
    temp->palette = NULL;
    bitmap_size = 3 * width * height;
  }
  else
  {
    if(!Mem_Alloc((void **)&temp->palette, sizeof(palette_entry) * (1 << bpp)))
    {
      free(temp);
      return NULL;
    }

    bitmap_size = width * height;
  }

  if(!Mem_Alloc((void **)&temp->bitmap, bitmap_size))
  {
    if(temp->palette)
      Mem_Free((void **)temp->palette);
    free(temp);
  }

  return temp;
}
