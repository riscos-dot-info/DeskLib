/*
    ####             #    #     # #
    #   #            #    #       #          The FreeWare C library for 
    #   #  ##   ###  #  # #     # ###             RISC OS machines
    #   # #  # #     # #  #     # #  #   ___________________________________
    #   # ####  ###  ##   #     # #  #                                      
    #   # #        # # #  #     # #  #    Please refer to the accompanying
    ####   ### ####  #  # ##### # ###    documentation for conditions of use
    ________________________________________________________________________

    File:    Sprite.MemSize.c
    Author:  Copyright © 1993 Tom Kirby-Green, Jason Howat
    Version: 1.01 (02 Sep 1995)
    Purpose: Calculate the size in bytes of a sprite with the given parameters
    Mods:    02 Sep 1995 JH Uses mode variable Log2BPC (bytes per chr) instead 
                            of Log2BPP (bits per pixel), to work properly with 
                            'double pixel' modes such as mode 4.
*/


#include "DeskLib:Sprite.h"
#include "DeskLib:SWI.h"


#define XOS_ReadModeVariable 0x20035

/* Macro to round a number of BITS up to a multiple of words */
#define ROUND32(value) (((value) + 31) & ~31)


extern int Sprite_MemorySize(int width, int height, int mode,
                             spritemem_flags flags)
{
  int ncolours;          /* Maximum logical colour: 1, 3, 15 or 63    */
  int log2bpp;           /* Log base 2 bits per pixel, i.e. 2^log2bpp */
  int bpp;               /* Bits per pixel                            */
  int bit_strlen;        /* Bit string length                         */
  int size;              /* Memory requirements                       */

  SWI(2, 3, XOS_ReadModeVariable,             /* Get Log2BPC for mode */
                 mode, 10,                    /* (used to be Log2BPP  */
       /* TO */  NULL, NULL, &log2bpp );

  bpp = 1 << log2bpp;                /* Work out bits per pixel       */
  bit_strlen = ROUND32(width * bpp); /* Initial bit string length,    *
                                      * rounded up to nearest word    */

  bit_strlen >>= 3;                  /* Convert bits to bytes (/= 8)  */
  size = bit_strlen * height;        /* Multiply by number of columns */

  if (flags & sprite_HASMASK)        /* Mask is just another chunk of */
    size *= 2;                       /* same size as the sprite data  */
                                     
  size += sizeof( sprite_header);    /* Add on 44 bytes for header    */

  if (flags & sprite_HASPAL)         /* Add on size of palette        */
  {
    SWI(2, 3, XOS_ReadModeVariable,
                   mode, 3,
         /* TO */  NULL, NULL, &ncolours);
    size += ( ncolours + 1 ) * ( sizeof( palette_entry ) * 2);
  }

  return(size);
}
