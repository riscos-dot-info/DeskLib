/*
    ####             #    #     # #
    #   #            #    #       #          The FreeWare C library for 
    #   #  ##   ###  #  # #     # ###             RISC OS machines
    #   # #  # #     # #  #     # #  #   ___________________________________
    #   # ####  ###  ##   #     # #  #                                      
    #   # #        # # #  #     # #  #    Please refer to the accompanying
    ####   ### ####  #  # ##### # ###    documentation for conditions of use
    ________________________________________________________________________

    File:    Clear.c.Save
    Author:  Copyright © 1993 Jason Howat
    Version: 1.00 (16 Dec 1993)
    Purpose: Save a Clear file to disk.
*/


#include <string.h>

#include "DeskLib:Clear.h"
#include "DeskLib:File.h"
#include "DeskLib:Wimp.h"


void Clear_Save(clear_picture *picture, char *filename)
{
  file_handle   out;
  unsigned      colours,
                entry;
  unsigned      bitmap_size;
  palette_entry *palette;


  if((out = File_Open(filename, file_WRITE)) == 0)
    return;

  File_SetType(filename, 0x690);

  File_WriteBytes(out, picture->creator, strlen(picture->creator) + 1);

  File_Write32(out, picture->creatorversion);
  File_Write32(out, picture->width);
  File_Write32(out, picture->height);
  File_Write32(out, picture->bpp);

  if(picture->bpp <= 8)
  {
    colours = 1 << picture->bpp;
    palette = picture->palette;
    for(entry = 0 ; entry < colours ; entry++)
    {
      File_Write8(out, palette[entry].data.red);
      File_Write8(out, palette[entry].data.green);
      File_Write8(out, palette[entry].data.blue);
    }

    bitmap_size = picture->width * picture->height;
  }
  else
    bitmap_size = 3 * picture->width * picture->height;

  File_WriteBytes(out, picture->bitmap, bitmap_size);

  File_Close(out);
}
