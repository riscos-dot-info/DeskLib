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
    Author:  Copyright © 1993,1994 Jason Williams, Jason Howat
    Version: 1.02 (25 Sep 2002) (AJPS)
    Purpose: Checks if the file exists at all, and if it can be termed a
             "file" - directories are excluded, image files are not.
*/


#include "DeskLib:File.h"
#include "kernel.h"


extern BOOL File_Exists(char *filename)
{   
	unsigned type;
  
	/* Get the file information */  
	SWI(2, 1, SWI_OS_File, 17, filename, &type);
	
    /* 1 => normal file, 3 => image file */
	return ((type & 1) ? TRUE : FALSE);

}
