/*
    ####             #    #     # #
    #   #            #    #       #          The FreeWare C library for
    #   #  ##   ###  #  # #     # ###             RISC OS machines
    #   # #  # #     # #  #     # #  #   ___________________________________
    #   # ####  ###  ##   #     # #  #
    #   # #        # # #  #     # #  #    Please refer to the accompanying
    ####   ### ####  #  # ##### # ###    documentation for conditions of use
    ________________________________________________________________________

    File:    Sound.h
    Author:  Copyright � 1993 Jason Williams
    Version: 0.01 (28 Mar 1993)
    Purpose: Sound system interfaces
*/

#ifndef __dl_sound_h
#define __dl_sound_h

#ifdef __cplusplus
extern "C" {
#endif

/* Abstract */
/*
  This header provides sound system interfaces.
  In fact, just the one at the moment.
*/

extern void Sound_SysBeep(void);
/*
  Simply writes a character 7 (system beep) to the VDU stream, in order
  to sound a system beep. It sorta goes 'beep', really.
*/

#ifdef __cplusplus
}
#endif


#endif
