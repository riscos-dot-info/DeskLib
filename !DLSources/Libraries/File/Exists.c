/*
    ####             #    #     # #
    #   #            #    #       #          The FreeWare C library for 
    #   #  ##   ###  #  # #     # ###             RISC OS machines
    #   # #  # #     # #  #     # #  #   ___________________________________
    #   # ####  ###  ##   #     # #  #                                      
    #   # #        # # #  #     # #  #    Please refer to the accompanying
    ####   ### ####  #  # ##### # ###    documentation for conditions of use
    ________________________________________________________________________

    File:    File.Exists.c
    Author:  Copyright � 1993,1994 Jason Williams, Jason Howat
    Version: 1.01 (08 Jun 1994)
    Purpose: Checks if a given file exists (can be read)
             Note that directories cannot be read, so will be returned as
             non existent.
*/


#include "DeskLib:File.h"
#include "kernel.h"


extern BOOL File_Exists(char *filename)
{
  _kernel_osfile_block osfile;

  return (_kernel_osfile(17, filename, &osfile) == 1);
}
