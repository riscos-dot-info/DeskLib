;-------------------------------------------------------------------------------
; OS_WriteN
;
        GET     macros.h
        GET     regdefs.h
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
