;-------------------------------------------------------------------------------
; ColourTrans_InvalidateCache(void);
; Copyright 1993 Shaun Blackmore

        GET     macros.h
        GET     regdefs.h
        GET     swinos.h
;
        PREAMBLE
        STARTCODE ColourTrans_InvalidateCache
;
        STMFD   sp!,{lr}
        SWI     XSWI_ColourTrans_InvalidateCache
        MOVVC   r0, #0
        LDMFD   sp!,{pc}
;
        END
