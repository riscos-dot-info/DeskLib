;   ####             #    #     # #
;   #   #            #    #       #          The FreeWare C library for
;   #   #  ##   ###  #  # #     # ###             RISC OS machines
;   #   # #  # #     # #  #     # #  #   ___________________________________
;   #   # ####  ###  ##   #     # #  #
;   #   # #        # # #  #     # #  #    Please refer to the accompanying
;   ####   ### ####  #  # ##### # ###    documentation for conditions of use
;   ________________________________________________________________________
;
;   File:    Sound.SysBeep.s
;   Author:  Copyright © 1993 Jason Williams
;   Version: 1.00 (28 Mar 1992)
;   Purpose: Does a VDU 7 in order to sound a system beep

  GET     ^.h.regdefs
  GET     ^.h.swinos
  GET     ^.h.macros


  PREAMBLE
  STARTCODE Sound_SysBeep

    STMFD   sp!, {lr}
    SWI     OS_WriteI + 7 + XOS_Bit
    LDMFD   sp!, {pc}^

  END
