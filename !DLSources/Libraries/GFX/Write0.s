;-------------------------------------------------------------------------------
; OS_Write0
;
        GET     macros.h
        GET     RegNames.h
        GET     swinos.h
;
        PREAMBLE
        STARTCODE GFX_Write0
;
        STMFD   sp!,{lr}
        SWI     XSWI_OS_Write0
        LDMFD   sp!,{pc}
;
        END
