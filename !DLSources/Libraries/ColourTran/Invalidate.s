;-------------------------------------------------------------------------------
; ColourTrans_InvalidateCache(void);
; Copyright 1993 Shaun Blackmore

        GET     ^.h.macros
        GET     ^.h.RegNames
        GET     ^.h.SWInames
;
        PREAMBLE
        STARTCODE ColourTrans_InvalidateCache
;
        STMFD   sp!,{lr}
        SWI     XSWI_ColourTrans_InvalidateCache
        MOVVC   r0, #0
        LDMFD   sp!,{pc}^
;
        END
