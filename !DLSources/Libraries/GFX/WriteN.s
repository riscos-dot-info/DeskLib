;-------------------------------------------------------------------------------
; OS_WriteN
;
        GET     Macros.h
        GET     RegDefs.h
        GET     swinos.h
;
        PREAMBLE
        STARTCODE GFX_WriteN
;
        STMFD   sp!,{lr}
        SWI     XSWI_OS_WriteN
        LDMFD   sp!,{pc}
;
        END
