/*
    ####             #    #     # #
    #   #            #    #       #          The FreeWare C library for
    #   #  ##   ###  #  # #     # ###             RISC OS machines
    #   # #  # #     # #  #     # #  #   ___________________________________
    #   # ####  ###  ##   #     # #  #
    #   # #        # # #  #     # #  #    Please refer to the accompanying
    ####   ### ####  #  # ##### # ###    documentation for conditions of use
    ________________________________________________________________________

    File:    File.Size.c
    Author:  Copyright © 1993 Jason Williams, 1994 Tony Houghton
    Version: 1.03 (25 Sep 2002)
    Purpose: Reads the size of the given file.
*/

#include "kernel.h"

#include "DeskLib:file.h"

extern int File_Size(const char *filename)
{
  _kernel_osfile_block osfile;
  int object_type = _kernel_osfile(17, filename, &osfile);

  /* Allow both image files and normal files to be "sized" */
  if (object_type == 1 || object_type == 3)
    return (osfile.start);
  /* Otherwise return the error code zero */
  else
    return (0);
}
