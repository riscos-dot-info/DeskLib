;-------------------------------------------------------------------------------
; OS_Write0
;
        GET     Macros.h
        GET     RegDefs.h
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
