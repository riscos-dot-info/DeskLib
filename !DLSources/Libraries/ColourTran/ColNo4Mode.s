;-------------------------------------------------------------------------------
; int ColourTrans_ReturnColourNumberForMode(int pal_entry, int mode, int pal);
; Copyright 1993 Jason Howat


        GET     macros.h
        GET     ^.h.RegNames
        GET     ^.h.SWInames
;
        PREAMBLE
        STARTCODE ColourTrans_ReturnColourNumberForMode
;
        STMFD   sp!,{lr}
        SWI     XSWI_ColourTrans_ReturnColourNumberForMode
        MOVVS   r0, #-1
        LDMFD   sp!,{pc}
;
        END
