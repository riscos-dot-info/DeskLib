/*
    ####             #    #     # #
    #   #            #    #       #          The FreeWare C library for 
    #   #  ##   ###  #  # #     # ###             RISC OS machines
    #   # #  # #     # #  #     # #  #   ___________________________________
    #   # ####  ###  ##   #     # #  #                                      
    #   # #        # # #  #     # #  #    Please refer to the accompanying
    ####   ### ####  #  # ##### # ###    documentation for conditions of use
    ________________________________________________________________________

    File:    Clear.c.Init
    Author:  Copyright © 1993 Jason Howat
    Version: 1.00 (16 Dec 1993)
    Purpose: Initialise the Clear library.
*/


#include "DeskLib:Clear.h"

#include "ClearDefs.h"


char            *clear__creator;
unsigned        clear__creatorversion;


void Clear_Init(char *creator, unsigned version)
{
  clear__creator = creator;
  clear__creatorversion = version;
}
