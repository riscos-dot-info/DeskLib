/*
    ####             #    #     # #
    #   #            #    #       #          The FreeWare C library for 
    #   #  ##   ###  #  # #     # ###             RISC OS machines
    #   # #  # #     # #  #     # #  #   ___________________________________
    #   # ####  ###  ##   #     # #  #                                      
    #   # #        # # #  #     # #  #    Please refer to the accompanying
    ####   ### ####  #  # ##### # ###    documentation for conditions of use
    ________________________________________________________________________

    File:    TextFile.ReadToD.c
    Author:  Copyright © 1992 Jason Williams
    Version: 1.00 (07 Apr 1992)
    Purpose: Generic textfile-handling routines
*/


#include <stdio.h>

#include "DeskLib:Core.h"
#include "DeskLib:TextFile.h"

#undef getc
#undef feof

extern void TextFile_ReadToDelimiter(FILE *infile, char delimiter,
                                     char *line, int maxlength)
{
  int index = 0;
  char thischar = 0;

  TextFile_SkipBlanks(infile);

  while (!feof(infile) && index < maxlength - 1)
  {
    thischar = getc(infile);
    if (thischar == delimiter)
      break;

    line[index++] = thischar;
  }
  line[index] = '\0';
}
