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
        SWI     SWI_ColourTrans_InvalidateCache + XOS_Bit
        MOVVC   r0, #0
        LDMFD   sp!,{pc}
;
        END
